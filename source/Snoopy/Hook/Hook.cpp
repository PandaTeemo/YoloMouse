#include <Snoopy/Hook/Hook.hpp>

namespace Snoopy
{
    // public
    //-------------------------------------------------------------------------
    Hook::Hook( void* target_address, x86::HookFunction hook_function, Placement placement ):
        _enabled        (false),
        _target_buffer  (CODE_BUFFER_SIZE),
        _target_address (target_address),
        _hook_function  (hook_function),
        _placement      (placement)
    {
    }

    Hook::~Hook()
    {
        // free trampoline memory
        if(_trampoline.GetSourceAddress())
            VirtualFree(_trampoline.GetSourceAddress(), 0, MEM_RELEASE);
    }

    //-------------------------------------------------------------------------
    Bool Hook::Init()
    {
        // init target
        if(!_InitTarget(_target_address))
            return false;

        // int target backup
        if(!_InitTargetBackup())
            return false;

        // init trampoline
        if(!_InitTrampoline(_hook_function))
            return false;

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Hook::Enable()
    {
        DWORD   old_permissions;
        BOOL    status;

        // fail if enabled
        if(_enabled)
            return false;

        // enable write permissions
        status = VirtualProtect(_target.GetSourceAddress(), _target_backup.GetCodeLength(), PAGE_EXECUTE_READWRITE, &old_permissions);
        if(!status)
            return false;

        // reset target
        _target.Empty();

        // replace target with jump to trampoline
        _target.OpJmpRel(_trampoline.GetSourceAddress());

        // pad with nops
        while( _target.GetCodeLength() < _target_backup.GetCodeLength() )
            _target.OpNop();

        // set enabled
        _enabled = true;

        return true;
    }

    Bool Hook::Disable()
    {
        // fail if disabled
        if( !_enabled )
            return false;

        // reset target
        _target.Empty();

        // restore from backup
        Tools::MemCopy(_target.GetSourceAddress(), _target_backup.GetCodeMemory(), _target_backup.GetCodeLength());

        // clear enabled
        _enabled = false;

        return true;
    }

    // private
    //-------------------------------------------------------------------------
    Bool Hook::_InitTarget( void* target_address )
    {
        // set address
        _target.SetSourceAddress(target_address);

        // set buffer
        _target.SetCodeBuffer(ByteArray(reinterpret_cast<Byte*>(target_address), CODE_BUFFER_SIZE));

        // reset
        _target.Empty();

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Hook::_InitTargetBackup()
    {
        x86::Operation op;

        // set address
        _target_backup.SetSourceAddress(_target.GetSourceAddress());

        // set buffer
        _target_backup.SetCodeBuffer(_target_buffer);

        // reset
        _target_backup.Empty();

        // read operations until minimum met
        while( _target_backup.GetCodeLength() < TARGET_MIN_SIZE )
        {
            // read operation
            if(!_target_backup.Read(op))
                return false;
        }

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Hook::_InitTrampoline( void* hook_address )
    {
        // allocate memory
		Byte* address = _AllocTrampolineAddress();
        if( address == NULL )
            return false;

        // update address
        _trampoline.SetSourceAddress(address);

        // set buffer
        _trampoline.SetCodeBuffer(ByteArray(address, CODE_BUFFER_SIZE));

        // clear
        _trampoline.Empty();

        // relocate target (hook after)
        if( _placement == AFTER && !_trampoline.Relocate(_target_backup))
            return false;

    #if CPU_64
        // push hook arguments
        _trampoline.OpPushReg(x86::REGISTER_R9);
        _trampoline.OpPushReg(x86::REGISTER_R8);
        _trampoline.OpPushReg(x86::REGISTER_DX);
        _trampoline.OpPushReg(x86::REGISTER_CX);
        _trampoline.OpPushReg(x86::REGISTER_AX);
        _trampoline.OpMovRegReg(x86::REGISTER_CX, x86::REGISTER_SP);

        // set hook function address
        _trampoline.OpMovRegImm(x86::REGISTER_AX, (Native)hook_address);

        // call hook function
        _trampoline.OpCallReg(x86::REGISTER_AX);

        // pop hook arguments
        _trampoline.OpPopReg(x86::REGISTER_AX);
        _trampoline.OpPopReg(x86::REGISTER_CX);
        _trampoline.OpPopReg(x86::REGISTER_DX);
        _trampoline.OpPopReg(x86::REGISTER_R8);
        _trampoline.OpPopReg(x86::REGISTER_R9);
    #else
        // push hook arguments
        _trampoline.OpPushReg(x86::REGISTER_SP);

        // call hook function
        _trampoline.OpCallRel(hook_address);

        // pop hook arguments
        _trampoline.OpPopReg(x86::REGISTER_SP);
    #endif

        // relocate target (hook before)
        if( _placement == BEFORE && !_trampoline.Relocate(_target_backup))
            return false;

        // jump back to after target
        _trampoline.OpJmpRel(reinterpret_cast<Byte*>(_target.GetSourceAddress()) + _target_backup.GetCodeLength());

        return true;
    }

    //-------------------------------------------------------------------------
    Byte* Hook::_AllocTrampolineAddress()
    {
        Byte*                       top;
        MEMORY_BASIC_INFORMATION    mbi;
        SYSTEM_INFO                 si;

        // get system infos
        GetSystemInfo(&si);

        // start from the target address. we want to be as close to this as possible to keep branch instructions simple.
        //top = (Byte*)si.lpMaximumApplicationAddress;
        top = (Byte*)_target_address;
        
        // iterate regions down until something available
        for( ; top >= si.lpMinimumApplicationAddress; top -= si.dwAllocationGranularity )
        {
            // query
            if( VirtualQuery(top, &mbi, sizeof(mbi)) )
            {
                // align
                top = (Byte*)mbi.BaseAddress;

                // if free
                if(mbi.State & MEM_FREE)
                {
                    // attempt alloc
		            Byte* memory = (Byte*)VirtualAlloc(top, CODE_BUFFER_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE);

                    // return on success)
                    if( memory )
                        return memory;
                }
            }
        }

        return NULL;
    }
}

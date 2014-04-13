#include <Hooks/Hook.hpp>

namespace Hooks
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
        if(_trampoline.GetAddress())
            VirtualFree(_trampoline.GetAddress(), 0, MEM_RELEASE);
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
        xassert(!_enabled);
        DWORD   old_permissions;
        BOOL    status;

        // enable write permissions
        status = VirtualProtect(_target.GetAddress(), _target_backup.GetCodeLength(), PAGE_EXECUTE_READWRITE, &old_permissions);
        if(!status)
            return false;

        // reset target
        _target.Empty();

        // replace target with jump to trampoline
        _target.OpJump(_trampoline.GetAddress());

        // pad with nops
        while( _target.GetCodeLength() < _target_backup.GetCodeLength() )
            _target.OpNop();

        // set enabled
        _enabled = true;

        return true;
    }

    void Hook::Disable()
    {
        xassert(_enabled);

        // reset target
        _target.Empty();

        // restore from backup
        Tools::MemCopy(_target.GetAddress(), _target_backup.GetCodeMemory(), _target_backup.GetCodeLength());

        // clear enabled
        _enabled = false;
    }

    // private
    //-------------------------------------------------------------------------
    Bool Hook::_InitTarget( void* target_address )
    {
        // set address
        _target.SetAddress(target_address);

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
        _target_backup.SetAddress(_target.GetAddress());

        // set buffer
        _target_backup.SetCodeBuffer(_target_buffer);

        // reset
        _target_backup.Empty();

        // read operations until minimum met
        while( _target_backup.GetCodeLength() < TARGET_MIN )
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
        _trampoline.SetAddress(address);

        // set buffer
        _trampoline.SetCodeBuffer(ByteArray(address, CODE_BUFFER_SIZE));

        // clear
        _trampoline.Empty();

        // relocate target (hook after)
        if( _placement == AFTER && !_trampoline.Relocate(_target_backup))
            return false;

        // push all registers
        _trampoline.OpPushAd();

        // add call to hook function
        _trampoline.OpCall(hook_address);

        // pop all registers
        _trampoline.OpPopAd();

        // relocate target (hook before)
        if( _placement == BEFORE && !_trampoline.Relocate(_target_backup))
            return false;

        // jump back to after target
        _trampoline.OpJump(reinterpret_cast<Byte*>(_target.GetAddress()) + _target_backup.GetCodeLength());

        return true;
    }

    //-------------------------------------------------------------------------
    Byte* Hook::_AllocTrampolineAddress()
    {
        Byte*                       top;
        MEMORY_BASIC_INFORMATION    mbi;
        SYSTEM_INFO                 si;

        // get systemo infos
        GetSystemInfo(&si);

        // from the top
        top = (Byte*)si.lpMaximumApplicationAddress;
            
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

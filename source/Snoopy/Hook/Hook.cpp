#include <Snoopy/Hook/Hook.hpp>

namespace Snoopy
{
    // public
    //-------------------------------------------------------------------------
    Hook::Hook():
        _enabled        (false),
        _initialized    (false),
        _target_buffer  (CODE_BUFFER_SIZE)
    {
    }

    Hook::~Hook()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool Hook::IsInitialized() const
    {
        return _initialized;
    }

    Bool Hook::IsEnabled() const
    {
        return _enabled;
    }

    //-------------------------------------------------------------------------
    Bool Hook::Initialize( void* target_address, x86::HookFunction hook_function )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _target_address = target_address;
        _hook_function = hook_function;

        // initialize target
        if(!_InitializeTarget(_target_address))
            return false;

        // intialize target backup
        if(!_InitializeTargetBackup())
            return false;

        // initialize trampoline
        if(!_InitializeTrampoline(_hook_function))
            return false;

        // set initialized
        _initialized = true;

        return true;
    }

    void Hook::Shutdown()
    {
        ASSERT( IsInitialized() );

        // free trampoline memory
        if(_trampoline.GetSourceAddress())
            VirtualFree(_trampoline.GetSourceAddress(), 0, MEM_RELEASE);

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool Hook::Enable()
    {
        ASSERT( !IsEnabled() );

        DWORD   old_permissions;
        BOOL    status;

        // enable write permissions
        status = VirtualProtect(_target.GetSourceAddress(), _target_backup.GetCodeLength(), PAGE_EXECUTE_READWRITE, &old_permissions);
        if(!status)
            return false;

        // reset target
        _target.Empty();

        // replace target with jump to trampoline
        if( !_target.OpJmpRel( _trampoline.GetSourceAddress() ) )
            return false;

        // pad with nops
        while( _target.GetCodeLength() < _target_backup.GetCodeLength() )
            _target.OpNop();

        // set enabled
        _enabled = true;

        return true;
    }

    Bool Hook::Disable()
    {
        ASSERT( IsEnabled() );

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
    Bool Hook::_InitializeTarget( void* target_address )
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
    Bool Hook::_InitializeTargetBackup()
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
    Bool Hook::_InitializeTrampoline( void* hook_address )
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

    #if CPU_64
        // push target arguments
        _trampoline.OpPushReg(x86::REGISTER_R9);
        _trampoline.OpPushReg(x86::REGISTER_R8);
        _trampoline.OpPushReg(x86::REGISTER_DX);
        _trampoline.OpPushReg(x86::REGISTER_CX);
        _trampoline.OpPushReg(x86::REGISTER_AX);

        /* 
            RCX is the argument to our hook function. it points to the stack containing
            the arguments (CX,DX,R8,R9) being passed to the target function. this allows
            hook function to modify these arguments as they get popped back to registers
            before theyre passed to the target.

            note that AX is actually the first value seen by hook argument array. this
            should be ignored and exists to be consistent with 32bit calling impl.
        */
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
        // push stack pointer
        _trampoline.OpPushReg(x86::REGISTER_SP);

        // call hook function
        if( !_trampoline.OpCallRel( hook_address ) )
            return false;

        /*
            pop stack pointer into unused register, (or can just increment it by 4) since
            calling function allowed to use that piece of memory. plus it makes no sense
            to pop a stack pointer.
        */
        _trampoline.OpPopReg(x86::REGISTER_AX);
    #endif

        // relocate target
        if( !_trampoline.Relocate(_target_backup))
            return false;

        // jump back to after target
        if( !_trampoline.OpJmpRel(reinterpret_cast<Byte*>(_target.GetSourceAddress()) + _target_backup.GetCodeLength()) )
            return false;

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

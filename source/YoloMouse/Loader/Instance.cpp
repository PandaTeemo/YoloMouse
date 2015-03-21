#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Loader/Instance.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    Instance::Instance():
        _process_id  (0),
        _process     (NULL),
        _wait_handle (NULL)
    {
    }

    Instance::~Instance()
    {
    }

    //-------------------------------------------------------------------------
    DWORD Instance::GetProcessId() const
    {
        return _process_id;
    }

    //-------------------------------------------------------------------------
    Bool Instance::Load( DWORD process_id )
    {
        xassert(_process_id == 0);

        // open process with all access privileges required by the tools that will use it (dont use PROCESS_ALL_ACCESS)
        _process = OpenProcess(
            PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_CREATE_THREAD|PROCESS_VM_READ|PROCESS_VM_WRITE|SYNCHRONIZE,
            FALSE, process_id);
        if( _process == NULL )
            return false;

        // choose inject dll
        const Char* inject_dll = _ChooseInjectDll();
        if( inject_dll != NULL )
        {
            // set notify name
            _injector.SetNotifyName(INJECT_NOTIFY_FUNCTION);

            // load
            if( _injector.Load(_process, inject_dll) )
            {
                // notify init
                if( Notify(NOTIFY_INIT) )
                {
                    // register process exit handler
                    if( RegisterWaitForSingleObject(&_wait_handle, _process, _ProcessExitHandler, this, INFINITE, WT_EXECUTEONLYONCE) )
                    {
                        _process_id = process_id;
                        return true;
                    }
                }
            }

            // unload
            Unload();
        }

        return false;
    }

    Bool Instance::Unload()
    {
        // free wait handle
        if( _wait_handle )
            UnregisterWait(_wait_handle);

        // unload injected dll
        _injector.Unload();

        // close process handle
        if( _process )
            CloseHandle(_process);

        // reset
        _process_id = 0;
        _process = NULL;
        _wait_handle = NULL;

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Instance::Notify( NotifyId id, Byte8 parameter )
    {
        NotifyMessage m;

        // build message
        m.id = id;
        m.parameter = parameter;

        // call remote notify handler
        return _injector.CallNotify(&m, sizeof(m));
    }

    // private
    //-------------------------------------------------------------------------
    const CHAR* Instance::_ChooseInjectDll()
    {
        // get process bitness
        Bitness bitness = SystemTools::GetProcessBitness(_process);

        // choose dll
        if( bitness == BITNESS_32 )
            return PATH_DLL32;
        if( bitness == BITNESS_64 )
            return PATH_DLL64;

        return NULL;
    }

    //-------------------------------------------------------------------------
    VOID CALLBACK Instance::_ProcessExitHandler( _In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired )
    {
        Instance* instance = reinterpret_cast<Instance*>(lpParameter);
        xassert(instance);

        // unload
        instance->Unload();
    }
}

#pragma once
#include <Snoopy/Inject/Injector.hpp>
#include <YoloMouse/Share/NotifyMessage.hpp>

namespace YoloMouse
{
    /**/
    class Instance
    {
    public:
        /**/
        Instance();
        ~Instance();

        /**/
        DWORD GetProcessId() const;

        /**/
        Bool Load( DWORD process_id );
        Bool Unload();

        /**/
        Bool Notify( NotifyId id, Byte8 parameter=0 );

    private:
        /**/
        const CHAR* _ChooseInjectDll();

        /**/
        static VOID CALLBACK _ProcessExitHandler( _In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired );

        // fields
        DWORD       _process_id;
        HANDLE      _process;
        HANDLE      _wait_handle;
        Injector    _injector;
    };
}

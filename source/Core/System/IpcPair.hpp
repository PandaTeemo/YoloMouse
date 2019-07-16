#include <Core/Root.hpp>
#include <windows.h>

namespace Core
{
    /**/
    class IpcPair
    {
    public:
        // constants
        static constexpr ULong IPC_NAME_LIMIT = 64;

        /**/
        struct IListener
        {
            /**/
            virtual void OnRecv( const void* message ) = 0;
        };

        /**/
        IpcPair();
        ~IpcPair();

        /**/
        Bool Initialize( const Char* name, ULong size, IListener& listener );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        Index GetInstance() const;

        /**/
        Bool Send( const void* message, ULong size, ULong wait_ms=0 );

    private:
        // types
        struct MemoryEntry
        {
            Byte4   lock;
            Byte    data[1];
        };

        /**/
        Bool _Initialize( const Char* name, ULong size, IListener& listener );
        void _Shutdown();

        /**/
        static VOID CALLBACK _EventWakeHandler( _In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired );

        // fields: parameters
        ULong           _size;
        IListener*      _listener;
        // fields: state
        Index           _instance;
        HANDLE          _hmap;
        HANDLE          _hwait;
        HANDLE          _hevent[2];
        MemoryEntry*    _memory[2];
    };
}

#include <Core/System/IpcPair.hpp>
#include <YoloMouse/Share/Ipc/IpcMessage.hpp>

namespace Yolomouse
{
    /**/
    class IpcMessenger
    {
    public:
        /**/
        struct IListener
        {
            /**/
            virtual void OnRecv( const IpcMessage& message ) = 0;
        };

        // constants
        static const ULong SEND_TIMEOUT = 100; // ms

        /**/
        Bool Initialize( ULong process_id, IListener& listener );
        void Shutdown();

        /**/
        Bool Send( const IpcMessage& message, ULong size, ULong timeout=SEND_TIMEOUT );

    private:
        // constants
        static constexpr Char* IPC_NAME_FORMAT = "YoloIpc.%u";

        // fields
        IpcPair _ipc;
    };
}

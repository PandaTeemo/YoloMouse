#include <YoloMouse/Share/Ipc/IpcMessenger.hpp>
#include <stdio.h>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    Bool IpcMessenger::Initialize( ULong process_id, IListener& listener )
    {
        Char temp_name[IpcPair::IPC_NAME_LIMIT];

        // build name
        sprintf_s( temp_name, sizeof(temp_name), IPC_NAME_FORMAT, process_id );

        // determine max size required
        ULong size = static_cast<ULong>(Tools::Max( sizeof( LoadIpcMessage ), sizeof( SetCursorIpcMessage ) ));

        // initialize ipc pair
        //WARN: IListener ghetto casting IpcPair::IListener
        return _ipc.Initialize( temp_name, size, reinterpret_cast<IpcPair::IListener&>(listener) );
    }

    void IpcMessenger::Shutdown()
    {
        // shutdown ipc pair
        _ipc.Shutdown();
    }

    //-------------------------------------------------------------------------
    Bool IpcMessenger::Send( const IpcMessage& message, ULong size, ULong timeout )
    {
        return _ipc.Send( &message, size, timeout );
    }
}

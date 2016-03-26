#include <YoloMouse/Share/SharedState.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    SharedState::SharedState():
        _host   (false),
        _shared (IPC_MEMORY_NAME)
    {
    }

    //-------------------------------------------------------------------------
    Bool SharedState::Open( Bool host )
    {
        // open shared memory
        if(!_shared.Open(host))
            return false;

        // update state
        _host = host;

        // initialize log
        if( host )
            _shared->log.Intialize();

        return true;
    }

    void SharedState::Close()
    {
        // close shared memory
        _shared.Close();
    }

    //-------------------------------------------------------------------------
    SharedLog& SharedState::GetLog() const
    {
        return _shared->log;
    }

    //-------------------------------------------------------------------------
    PathString& SharedState::EditPath()
    {
        return _shared->path;
    }
}

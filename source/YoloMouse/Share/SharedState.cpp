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

        return true;
    }

    void SharedState::Close()
    {
        // close shared memory
        _shared.Close();
    }

    //-------------------------------------------------------------------------
    PathString& SharedState::EditPath()
    {
        return _shared->path;
    }
}

#include <Core/System/SystemTools.hpp>
#include <YoloMouse/Loader/Core/App.hpp>
#include <YoloMouse/Loader/Inject/InjectSession.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <Snoopy/Inject/Injector.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    InjectSession::InjectSession():
        _process_id (INVALID_ID),
        _loaded     (false)
    {
    }

    InjectSession::~InjectSession()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool InjectSession::Initialize( Id process_id )
    {
        ASSERT( !IsInitialized() );

        // initialize ipc
        if( !_ipc.Initialize( process_id, *this ) )
            return false;

        // set fields
        _process_id = process_id;

        return true;
    }

    void InjectSession::Shutdown()
    {
        ASSERT( IsInitialized() );
        ASSERT( !IsLoaded() );

        // shutdown ipc
        _ipc.Shutdown();

        // reset fields
        _process_id = INVALID_ID;
    }

    //-------------------------------------------------------------------------
    Bool InjectSession::IsInitialized() const
    {
        return _process_id != INVALID_ID;
    }

    Bool InjectSession::IsLoaded() const
    {
        return _loaded;
    }

    //-------------------------------------------------------------------------
    Id InjectSession::GetProcessId() const
    {
        return _process_id;
    }

    //-------------------------------------------------------------------------
    Bool InjectSession::Load( const PathString& bindings_path )
    {
        ASSERT( !IsLoaded() );
        ASSERT( IsInitialized() );

        // open process with necessary privileges required by the tools that will use it
        HANDLE process = OpenProcess(PROCESS_VM_OPERATION|PROCESS_CREATE_THREAD|PROCESS_VM_WRITE, FALSE, _process_id);
        if( process != NULL )
        {
            // choose inject dll
            const Char* inject_dll = _ChooseInjectDll(process);
            if( inject_dll != nullptr )
            {
                // inject dll into process
                if( _injector.Load( process, inject_dll ) )
                {
                    // tell dll to load
                    if( _SendLoad( bindings_path ) )
                    {
                        // set loaded state
                        _loaded = true;
                    }
                    else
                        _injector.Unload();
                }
                else
                    LOG("InjectSession.Load.Injector.Load: %s", inject_dll);
            }
            else
                LOG("InjectSession.ChooseInjectDll");

            // close process handle
            CloseHandle(process);
        }
        else
            LOG("InjectSession.Load.OpenProcess");

        return _loaded;
    }

    void InjectSession::Unload()
    {
        ASSERT( IsLoaded() );

        // tell dll to exit (ignore fail)
        _SendExit();

        // unload injected dll
        _injector.Unload();

        // reset loaded state
        _loaded = false;
    }

    //-------------------------------------------------------------------------
    Bool InjectSession::SendSetCursor( CursorType type, CursorId id, CursorVariation variation, CursorSize size_delta )
    {
        SetCursorIpcMessage message;

        // build message
        message.request = IPC_REQUEST_SET_CURSOR;
        message.type = type;
        message.id = id;
        message.variation = variation;
        message.size_delta = size_delta;

        // send message
        return _ipc.Send( message, sizeof(message) );
    }

    Bool InjectSession::SendSetDefaultCursor()
    {
        SetCursorIpcMessage message;

        // build message
        message.request = IPC_REQUEST_SET_DEFAULT_CURSOR;

        // send message
        return _ipc.Send( message, sizeof(message) );
    }

    Bool InjectSession::SendResetCursor()
    {
        IpcMessage message;

        // build message
        message.request = IPC_REQUEST_RESET_CURSOR;

        // send message
        return _ipc.Send( message, sizeof(message) );
    }

    Bool InjectSession::SendRefreshCursor()
    {
        IpcMessage message;

        // build message
        message.request = IPC_REQUEST_REFRESH_CURSOR;

        // send message
        return _ipc.Send( message, sizeof(message) );
    }

    // private
    //-------------------------------------------------------------------------
    Bool InjectSession::_SendLoad( const PathString& bindings_path )
    {
        LoadIpcMessage  message;
        App&            app = App::Instance();

        // build message
        message.request = IPC_REQUEST_LOAD;
        message.host_path = app.GetHostPath();
        message.log_path = app.GetLogPath();
        message.bindings_path = bindings_path;

        // send message
        return _ipc.Send( message, sizeof(message) );
    }

    Bool InjectSession::_SendExit()
    {
        IpcMessage message;

        // build message
        message.request = IPC_REQUEST_EXIT;

        // send message
        return _ipc.Send( message, sizeof(message) );
    }

    //-------------------------------------------------------------------------
    const CHAR* InjectSession::_ChooseInjectDll( HANDLE process )
    {
        // get process bitness
        Bitness bitness = SystemTools::GetProcessBitness(process);

        // choose dll
        if( bitness == BITNESS_32 )
            return PATH_DLL32;
        if( bitness == BITNESS_64 )
            return PATH_DLL64;

        return NULL;
    }

    //-------------------------------------------------------------------------
    void InjectSession::_OnRecvCursorChanging( const OnCursorChangingIpcMessage& message )
    {
        InjectSessionEvent event;

        // build event
        event.id = InjectSessionEvent::CURSOR_CHANGING;
        event.u.cursor_changing = message.binding;

        // notify
        events.Notify(event);
    }

    void InjectSession::_OnRecvCursorShowing( const OnCursorShowingIpcMessage& message )
    {
        InjectSessionEvent event;

        // build event
        event.id = InjectSessionEvent::CURSOR_SHOWING0;
        event.u.cursor_showing = message.showing;

        // notify
        events.Notify(event);
    }

    void InjectSession::OnRecv( const IpcMessage& message )
    {
        // handle request from injected target
        switch( message.request )
        {
        case IPC_REQUEST_ON_CURSOR_CHANGING:
            _OnRecvCursorChanging( reinterpret_cast<const OnCursorChangingIpcMessage&>(message) );
            break;
        case IPC_REQUEST_ON_CURSOR_SHOWING:
            _OnRecvCursorShowing( reinterpret_cast<const OnCursorShowingIpcMessage&>(message) );
            break;
        }
    }
}

#pragma once
#include <Core/Support/EventDispatcher.hpp>
#include <Snoopy/Inject/Injector.hpp>
#include <YoloMouse/Share/Ipc/IpcMessenger.hpp>
#include <YoloMouse/Loader/Events/InjectSessionEvent.hpp>

namespace Yolomouse
{
    /**/
    class InjectSession:
        public IpcMessenger::IListener
    {
    public:
        /**/
        InjectSession();
        ~InjectSession();

        /**/
        Bool Initialize( Id process_id );
        void Shutdown();

        /**/
        Bool IsInitialized() const;
        Bool IsLoaded() const;

        /**/
        Id GetProcessId() const;

        /**/
        Bool Load( const PathString& bindings_path );
        void Unload();

        /**/
        Bool SendSetCursor( CursorType type, CursorId id, CursorVariation variation, CursorSize size_delta );
        Bool SendSetDefaultCursor();
        Bool SendResetCursor();
        Bool SendRefreshCursor();

        // events
        EventDispatcher<InjectSessionEvent> events;

    private:
        /**/
        Bool _SendLoad( const PathString& bindings_path );
        Bool _SendExit();

        /**/
        static const CHAR* _ChooseInjectDll( HANDLE process );

        /**/
        void _OnRecvCursorChanging( const OnCursorChangingIpcMessage& message );
        void _OnRecvCursorShowing( const OnCursorShowingIpcMessage& message );
        void OnRecv( const IpcMessage& message );

        // fields: parameters
        Id                  _process_id;
        // fields: state
        Bool                _loaded;
        // fields: objects
        Snoopy::Injector    _injector;
        IpcMessenger        _ipc;
    };
}

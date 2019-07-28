#pragma once
#include <YoloMouse/Loader/Events/InjectSessionEvent.hpp>
#include <YoloMouse/Loader/Inject/InjectSession.hpp>
#include <YoloMouse/Share/Cursor/CursorBindings.hpp>

namespace Yolomouse
{
    /**/
    class Target:
        public EventListener<InjectSessionEvent>
    {
    public:
        /**/
        Target();
        ~Target();

        /**/
        Bool IsInitialized() const;
        Bool IsStarted() const;
        Bool IsRestricted() const;

        /**/
        Id GetProcessId() const;

        /**/
        Bool SetCursor( const CursorInfo& updates, CursorUpdateFlags flags );
        Bool SetDefaultCursor();
        Bool ResetCursor();

    private:
        friend class TargetController;

        // types
        struct IEventHandler
        {
            virtual void OnTargetShutdown( Id process_id ) = 0;
        };

        /**/
        Bool Initialize( Id process_id, Bool require_configured, IEventHandler& event_handler );
        void Shutdown();

        /**/
        Bool Start();
        void Stop();

        /**/
        void OnHover( HWND hwnd );
        void OnHoverOut();

    private:
        // constants
        static constexpr ULong HACK_VISIBILITY_TIMEOUT = 300; // ms

        /**/
        Bool _InitializeProcessListener();
        void _ShutdownProcessListener();

        /**/
        static Bool _IsValidCursor( const CursorInfo& properties );

        /**/
        Bool _UpdateRestrictedBinding( const CursorInfo& updates, CursorUpdateFlags flags );
        void _UpdateCursor();

        /**/
        Bool _BuildBindingsPath();

        /**/
        void _OnInjectedCursorChanging( const CursorInfo& info );
        void _OnInjectedCursorShowing( Bool showing );
        Bool _OnEvent( const InjectSessionEvent& event );

        /**/
        static VOID CALLBACK _OnProcessExit( _In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired );

        /**/
        static void _BuildDefaultCursor( CursorInfo& info );

        // fields: parameters
        Id                      _process_id;
        IEventHandler*          _event_handler;
        // fields: info
        PathString              _bindings_path;
        // fields: state
        Bool                    _initialized;
        Bool                    _started;
        HWND                    _hover_hwnd;
        Bool                    _showing;
        CursorInfo              _active_cursor;
        // fields: objects
        CursorBindings          _restricted_bindings;
        InjectSession           _inject_session;
        HANDLE                  _process;
        HANDLE                  _wait_handle;
    };
}

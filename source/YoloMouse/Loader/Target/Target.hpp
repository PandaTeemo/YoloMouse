#pragma once
#include <YoloMouse/Loader/Events/InjectSessionEvent.hpp>
#include <YoloMouse/Loader/Inject/InjectSession.hpp>
#include <YoloMouse/Share/Bindings/CursorBindings.hpp>

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

        /**/
        Id GetProcessId() const;

        /**/
        Bool ActionSetCursor( CursorType type, CursorId id );
        Bool ActionSetDefaultCursor();
        Bool ActionResetCursor();
        Bool ActionSetCursorSize( Long size_index_delta );

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
        void _UpdateRestrictedBinding( CursorType type, CursorId id, CursorVariation variation, CursorSize size_delta );
        void _UpdateCursor();

        /**/
        Bool _BuildBindingsPath();

        /**/
        void _OnInjectedCursorChanging( const CursorBindings::Binding& binding );
        void _OnInjectedCursorShowing( Bool showing );
        Bool _OnEvent( const InjectSessionEvent& event );

        /**/
        static VOID CALLBACK _OnProcessExit( _In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired );

        /**/
        static void _BuildDefaultOverlayBinding( CursorBindings::Binding& binding );

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
        CursorBindings::Binding _binding;
        // fields: objects
        CursorBindings          _restricted_bindings;
        InjectSession           _inject_session;
        HANDLE                  _process;
        HANDLE                  _wait_handle;
    };
}

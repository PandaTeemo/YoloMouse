/*
    this is a software cursor overlay implementation as an alternative to the more intrusive injected
    implementation in order to avoid the wrath of anticheats.
*/
#pragma once
#include <Core/Math/Vector2.hpp>
#include <Core/Support/EventDispatcher.hpp>
#include <Core/Support/Singleton.hpp>
#include <Core/UI/Window.hpp>
#include <YoloMouse/Loader/Events/OverlayEvent.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/BasicCursor.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/IOverlayCursor.hpp>
#include <YoloMouse/Loader/Overlay/Mouse/MousePositionMonitor.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Enums.hpp>

namespace Yolomouse
{
    /**/
    class Overlay:
        public Singleton<Overlay>,
        public EventListener<WindowEvent>
    {
    public:
        /**/
        Overlay();
        ~Overlay();

        /**/
        Bool Initialize( HINSTANCE hinstance );
        void Shutdown();

        /**/
        Bool IsInitialized() const;
        Bool IsStarted() const;
        Bool IsCursorInstalled( CursorId id ) const;

        /**/
        RenderContext&  GetRenderContext();
        Window&         GetWindow();

        /**/
        Bool SetCursor( const CursorInfo& info );
        Bool SetCursorIterated( CursorInfo& info );
        void SetCursorHidden();

        /**/
        Bool InstallCursor( CursorId id, IOverlayCursor& cursor );
        Bool UninstallCursor( CursorId id );

        /**/
        Bool Start();
        void Stop();

        // events
        EventDispatcher<OverlayEvent> events;

    private:
        // enums
        enum: Bits
        {
            PRE_FRAME_EVENT_RESIZE =        BIT(0),
        };
        typedef Bits PreFrameEvents;
        enum: Bits
        {
            IN_FRAME_EVENT_SET_CURSOR =     BIT(0),
            IN_FRAME_EVENT_HIDE_CURSOR =    BIT(1)
        };
        typedef Bits InFrameEvents;

        // aliases
        typedef FlatArray<IOverlayCursor*, CURSOR_ID_COUNT> CursorTable;

        // constants
        const ULong _THREAD_STACK_SIZE = KILOBYTES(64);

        /**/
        Bool _Initialize( HINSTANCE hinstance );
        Bool _InitializeWindow();
        Bool _InitializeInput();
        Bool _InitializeThread();

        /**/
        void _Shutdown();
        void _ShutdownWindow();
        void _ShutdownInput();
        void _ShutdownThread();

        /**/
        void _ProcessPreFrameEvents();
        void _ProcessInFrameEvents();
        void _FrameResize( const Vector2l& size );
        void _FrameLoop();

        /**/
        void _UpdateHoverState( const Vector2l& cursor_position );

        /**/
        IOverlayCursor* _LoadCursor( const CursorInfo& info );
        IOverlayCursor* _LoadCursorIterated( CursorInfo& info );

        /**/
        void _OnDisplayChange( ULong width, ULong height );
        Bool _OnEvent( const WindowEvent& event );

        /**/
        static DWORD WINAPI _ThreadProcedure( _In_ LPVOID lpParameter );

        // fields: parameters
        CursorTable                 _cursors;
        IOverlayCursor*             _active_cursor;
        // fields: state
        Window                      _window;
        MousePositionMonitor        _mouse;
        HANDLE                      _thread;
        Bool                        _initialized;
        Bool                        _started;
        Bool                        _active;
        BasicCursor                 _basic_cursor;
        HWND                        _hover_hwnd;
        RenderContext               _render_context;
        // fields: local events
        PreFrameEvents              _pre_frame_events;
        InFrameEvents               _in_frame_events;
        Vector2l                    _resize_event;
        IOverlayCursor*             _cursor_event;
    };
}

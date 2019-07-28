#include <YoloMouse/Loader/Overlay/Overlay.hpp>
#include <YoloMouse/Share/Cursor/CursorTools.hpp>
#include <dwmapi.h>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        constexpr ULong TOPMOST_UPDATE_COUNT =  8;              // frames
        constexpr ULong TOPMOST_ROUTINE_COUNT = 200;            // frames
    }

    // public
    //-------------------------------------------------------------------------
    Overlay::Overlay():
        // fields: parameters
        _active_cursor          (nullptr),
        // fields: state
        _thread                 (NULL),
        _initialized            (false),
        _started                (false),
        _active                 (false),
        _hover_hwnd             (NULL),
        _pre_frame_events       (0),
        _in_frame_events        (0),
        _resize_event           (0, 0)
    {
        _cursors.Zero();
    }

    Overlay::~Overlay()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool Overlay::Initialize( HINSTANCE hinstance )
    {
        ASSERT( !IsInitialized() );

        // initialize else undo failings
        if( !_Initialize( hinstance ) )
        {
            _Shutdown();
            return false;
        }

        // set initialized
        _initialized = true;

        return true;
    }

    void Overlay::Shutdown()
    {
        ASSERT( IsInitialized() );
        ASSERT( !IsStarted() );

        // shutdown
        _Shutdown();

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool Overlay::IsInitialized() const
    {
        return _initialized;
    }

    Bool Overlay::IsStarted() const
    {
        return _started;
    }

    Bool Overlay::IsCursorInstalled( CursorId id ) const
    {
        ASSERT( id < CURSOR_ID_COUNT );
        return _cursors[id] != nullptr;
    }

    //-------------------------------------------------------------------------
    RenderContext& Overlay::GetRenderContext()
    {
        return _render_context;
    }

    Window& Overlay::GetWindow()
    {
        return _window;
    }

    //-------------------------------------------------------------------------
    Bool Overlay::SetCursor( const CursorInfo& info )
    {
        // load requested cursor
        IOverlayCursor* cursor = _LoadCursor( info );
        if( cursor == nullptr )
            return false;

        // set update event
        _cursor_event = cursor;
        _in_frame_events |= IN_FRAME_EVENT_SET_CURSOR;

        return true;
    }

    Bool Overlay::SetCursorIterated( CursorInfo& info )
    {
        // load requested cursor
        IOverlayCursor* cursor = _LoadCursorIterated( info );
        if( cursor == nullptr )
            return false;

        // set update event
        _cursor_event = cursor;
        _in_frame_events |= IN_FRAME_EVENT_SET_CURSOR;

        return true;
    }

    void Overlay::SetCursorHidden()
    {
        // activate
        _in_frame_events |= IN_FRAME_EVENT_HIDE_CURSOR;
    }

    //-------------------------------------------------------------------------
    Bool Overlay::InstallCursor( CursorId id, IOverlayCursor& cursor )
    {
        ASSERT( IsInitialized() );
        ASSERT( _cursors[id] == nullptr );

        // initialize cursor
        if( !cursor.Initialize({ _render_context, _window.GetAspectRatio() }) )
            return false;

        // add cursor to table
        _cursors[id] = &cursor;

        return true;
    }

    Bool Overlay::UninstallCursor( CursorId id )
    {
        ASSERT( IsInitialized() );

        // get cursor from table
        IOverlayCursor*& cursor = _cursors[id];
        if( cursor == nullptr )
            return false;

        // shutdown cursor
        cursor->Shutdown();

        // remove from table
        cursor = nullptr;

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Overlay::Start()
    {
        ASSERT( !IsStarted() );

        // initialize thread
        if( !_InitializeThread() )
            return false;

        // set started
        _started = true;

    #ifdef BUILD_DEBUG
        CursorInfo cursor = { CURSOR_TYPE_OVERLAY, 0, 0, 10 };
        SetCursorIterated( cursor );
    #endif

        return true;
    }

    void Overlay::Stop()
    {
        ASSERT( IsStarted() );

        // shutdown thread
        _ShutdownThread();

        // reset started
        _started = false;
    }

    // private
    //-------------------------------------------------------------------------
    Bool Overlay::_Initialize( HINSTANCE hinstance )
    {
        // initial size is desktop resolution
        Vector2l size(GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ));

        // initialize window
        if( !_window.Initialize({hinstance, size, OVERLAY_CLASS, OVERLAY_NAME, Window::OPTION_OVERLAY}) )
            return false;

        // initialize render context
        if( !_render_context.Initialize(_window.GetHandle(), _window.GetSize()) )
            return false;

        // initialize mouse
        if( !_mouse.Initialize(_window) )
            return false;

        // initialize basic cursor
        if( !_basic_cursor.Initialize({ _render_context, _window.GetAspectRatio() }) )
            return false;

        // register events
        _window.events.Add( *this );

        return true;
    }

    Bool Overlay::_InitializeThread()
    {
        // create thread for run method
        _thread = CreateThread( NULL, _THREAD_STACK_SIZE, _ThreadProcedure, this, 0, NULL );
        if( _thread == NULL )
            return false;

        // raise priority. ignore fails
        SetThreadPriority( _thread, THREAD_PRIORITY_HIGHEST );

        // set active state
        _active = true;

        return true;
    }

    //-------------------------------------------------------------------------
    void Overlay::_Shutdown()
    {
        // unregister events
        _window.events.Remove( *this );

        // shutdown basic cursor
        _basic_cursor.Shutdown();

        // shutdown mouse
        _mouse.Shutdown();

        // shutdown render context
        _render_context.Shutdown();

        // shutdown window
        _window.Shutdown();
    }

    void Overlay::_ShutdownThread()
    {
        // if active
        if( _active )
        {
            // reset active state
            _active = false;

            // wait for thread to complete
            WaitForSingleObject( _thread, 4000 );
        }
    }

    //-------------------------------------------------------------------------
    void Overlay::_ProcessPreFrameEvents()
    {
        // if pending pre frame events exist
        if( _pre_frame_events != 0 )
        {
            // handle resize
            if( _pre_frame_events & PRE_FRAME_EVENT_RESIZE )
                _FrameResize( _resize_event );

            // reset pre frame events
            _pre_frame_events = 0;
        }
    }

    void Overlay::_ProcessInFrameEvents()
    {
        // if pending in frame events exist
        if( _in_frame_events != 0 )
        {
            // if cursor hide, reset active cursor
            if( _in_frame_events & IN_FRAME_EVENT_HIDE_CURSOR )
                _active_cursor = nullptr;
            // if cursor set, if set new cursor
            if( _in_frame_events & IN_FRAME_EVENT_SET_CURSOR )
                _active_cursor = _cursor_event;

            // reset in frame events
            _in_frame_events = 0;
        }
    }

    void Overlay::_FrameResize( const Vector2l& size )
    {
        // update window size
        _window.SetSize(size);

        // resize render context
        _render_context.Resize(size);

        // notify cursors
        _basic_cursor.OnResize( { _window.GetAspectRatio() } );
        for( IOverlayCursor* cursor : _cursors )
            if( cursor != nullptr )
                cursor->OnResize( { _window.GetAspectRatio() } );
    }

    void Overlay::_FrameLoop()
    {
        Vector2l windows_position;
        Vector2l adjusted_position;
        Vector2f nds_position;

        // run frame loop
        while(_active)
        {
            Bool idle = true;

            // process pre frame events
            _ProcessPreFrameEvents();

            // being render session
            _render_context.RenderBegin();

            // get mouse monitor cursor position
            if( _mouse.GetCursorPosition(windows_position, adjusted_position, nds_position) )
            {
                // update hover state using windows cursor position
                _UpdateHoverState(windows_position);

                // process in frame events
                _ProcessInFrameEvents();

                // if active cursor exists
                if( _active_cursor != nullptr )
                {
                    // update cursor
                    _active_cursor->Update({ nds_position });

                    // draw cursor
                    _active_cursor->Draw();

                    // set render
                    idle = false;
                }
            }

            // complete render session
            _render_context.RenderComplete(idle);
        }
    }

    //-------------------------------------------------------------------------
    void Overlay::_UpdateHoverState( const Vector2l& windows_position )
    {
        POINT point;

        // convert last cursor position to point
        point.x = windows_position.x;
        point.y = windows_position.y;

        // get window at cursor position
        _hover_hwnd = WindowFromPoint(point);

        // notify
        events.Notify( {OverlayEvent::WINDOW_HOVER, _hover_hwnd } );
    }

    //-------------------------------------------------------------------------
    IOverlayCursor* Overlay::_LoadCursor( const CursorInfo& info )
    {
        // by cursor type
        switch( info.type )
        {
        case CURSOR_TYPE_BASIC:
            // set basic cursor
            if( _basic_cursor.SetCursor( info.id, info.variation, info.size ) )
                return &_basic_cursor;

            return false;

        case CURSOR_TYPE_OVERLAY:
            {
                // get installed cursor
                IOverlayCursor* cursor = _cursors[info.id];

                // if installed set cursor
                if( cursor != nullptr && cursor->SetCursor( info.id, info.variation, info.size ) )
                    return cursor;

                return false;
            }

        default:
            return false;
        }
    }

    IOverlayCursor* Overlay::_LoadCursorIterated( CursorInfo& info )
    {
        // for each cursor id with rotation
        for( CursorId idi = 0; idi < CURSOR_ID_COUNT; info.id = (info.id + 1) % CURSOR_ID_COUNT, ++idi )
        {
            // for each cursor variation with rotation
            for( CursorVariation variationi = 0; variationi < CURSOR_VARIATION_COUNT; info.variation = (info.variation + 1) % CURSOR_VARIATION_COUNT, ++variationi )
            {
                // load cursor
                IOverlayCursor* cursor = _LoadCursor( info );

                // return if loaded
                if( cursor != nullptr )
                    return cursor;
            }

            info.variation = 0;
        }

        return nullptr;
    }

    //-------------------------------------------------------------------------
    void Overlay::_OnDisplayChange( ULong width, ULong height )
    {
        // send pending resize event
        _resize_event.Set( width, height );
        _pre_frame_events |= PRE_FRAME_EVENT_RESIZE;
    }

    Bool Overlay::_OnEvent( const WindowEvent& event )
    {
        switch( event.msg )
        {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return true;

        case WM_DISPLAYCHANGE:
            _OnDisplayChange(LOWORD(event.lparam), HIWORD(event.lparam));
            return true;
        }

        return false;
    }

    //-------------------------------------------------------------------------
    DWORD WINAPI Overlay::_ThreadProcedure( _In_ LPVOID lpParameter )
    {
        // parameter is overlay window
        Overlay* overlay_window = reinterpret_cast<Overlay*>(lpParameter);

        // run
        overlay_window->_FrameLoop();

        return 0;
    }
}

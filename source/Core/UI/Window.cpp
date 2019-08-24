#include <Core/UI/Window.hpp>
#include <dwmapi.h>

namespace Core
{
    // public
    //-------------------------------------------------------------------------
    Window::Window():
        // fields: parameters
        _hinstance  (NULL),
        // fields: state
        _hwnd       (NULL),
        _size       (0, 0)
    {
    }

    Window::~Window()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool Window::Initialize( const InitializeDef& def )
    {
        ASSERT( !IsInitialized() );
        WNDCLASS wclass = {};

        // set fields
        _hinstance = def.hinstance;
        _class_name = def.class_name;
        _size = def.size;
        _SizeTweak();
        _aspect_ratio = static_cast<Float>(_size.x) / static_cast<Float>(_size.y);

        // build window class
        wclass.style = CS_OWNDC;
        wclass.lpfnWndProc = _WindowProcedure;
        wclass.cbClsExtra = 0;
        wclass.cbWndExtra = 0;
        wclass.hInstance = _hinstance;
        wclass.hIcon = LoadIcon( nullptr, IDI_APPLICATION );
        wclass.hCursor = LoadCursor( nullptr, IDC_CROSS );
        wclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wclass.lpszMenuName = nullptr;
        wclass.lpszClassName = _class_name;

        // register window class
        RegisterClass(&wclass);

        // create window
        _hwnd = CreateWindowEx(
            0,                          // optional window styles.
            def.class_name,              // window class
            def.title,                  // window text
            WS_POPUP,                   // window style
            0, 0, _size.x, _size.y,     // size and position
            NULL,                       // parent window    
            NULL,                       // menu
            _hinstance,                 // instance handle
            NULL                        // additional application data
        );

        // fail if 
        if( _hwnd == NULL )
            return false;

        // associate this object with window
        SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)this);

        // if overlay option
        if( def.options & OPTION_OVERLAY )
        {
            const MARGINS frame_extend_margin = {-1,-1,-1,-1};

            // set overlay styles
	        SetWindowLongPtr(_hwnd, GWL_STYLE, WS_VISIBLE); // not quite same as ShowWindow
	        SetWindowLongPtr(_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST|WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE);
            DwmExtendFrameIntoClientArea(_hwnd, &frame_extend_margin );

            // set timer for topmost refresh
            SetTimer( _hwnd, 0, TOPMOST_REFRESH_TIMER, NULL );
        }

        return true;
    }

    void Window::Shutdown()
    {
        ASSERT( IsInitialized() );

        // destroy window
        DestroyWindow(_hwnd);

        // unregister class
        UnregisterClass(_class_name, _hinstance);

        // reset fields
        _hwnd = nullptr;
    }

    //-------------------------------------------------------------------------
    Bool Window::IsInitialized() const
    {
        return _hwnd != NULL;
    }

    //-------------------------------------------------------------------------
    HWND Window::GetHandle() const
    {
        return _hwnd;
    }

    Vector2l Window::GetSize() const
    {
        return _size;
    }

    Float Window::GetAspectRatio() const
    {
        return _aspect_ratio;
    }

    //-------------------------------------------------------------------------
    void Window::SetSize( const Vector2l& size )
    {
        // set new size
        _size = size;
        _SizeTweak();

        // update aspect ratio
        _aspect_ratio = static_cast<Float>(_size.x) / static_cast<Float>(_size.y);

        // update window size
        //TODO5: some flags r overlay specific
        SetWindowPos( _hwnd, HWND_TOPMOST, 0, 0, _size.x, _size.y, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOREDRAW );
    }

    // private
    //-------------------------------------------------------------------------
    void Window::_SetTop()
    {
        // update our topmost state (reposition Z order)
        SetWindowPos( _hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOREDRAW );
    }

    //-------------------------------------------------------------------------
    void Window::_SizeTweak()
    {
        // this is ghetto way to prevent taskbar hiding if topmost window. maybe windows thinks we're trying to be completely fullscreen?
        _size -= 1;
    }

    //-------------------------------------------------------------------------
    LRESULT CALLBACK Window::_WindowProcedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
    {
        // handle msg
        switch( msg )
        {
        // internal events
        //TODO2: disable WM_WINDOWPOSCHANGED for now, can be performance danger if another app competing for topmost
        //case WM_WINDOWPOSCHANGED:
        /*
            TODO3: find way to avoid updating zorder every timer interval. WM_WINDOWPOSCHANGED works fine in most
            cases, except entering fullscreen GW2 which produces no window proc events at all. SetWinEventHook
            doesnt provide anything either and win event hook would be too intrusive.
        */
        case WM_TIMER:
            ((Window*)GetWindowLongPtr( hwnd, GWLP_USERDATA ))->_SetTop();
            return 0;

        // if supported event
        case WM_INPUT:
        case WM_DESTROY:
        case WM_DISPLAYCHANGE:
        case WM_SETTINGCHANGE:
            {
                WindowEvent event;

                // get window instance
                Window* window = (Window*)GetWindowLongPtr( hwnd, GWLP_USERDATA );

                // build event
                event.window = window;
                event.msg = msg;
                event.wparam = wparam;
                event.lparam = lparam;

                // notify
                if( window->events.Notify(event) )
                    return 0;
            }
            break;
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

#include <YoloMouse/Loader/Target/Support/CursorVisibilityHacker.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        const Char* HOOK_DLL =      "user32.dll";
        const Char* HOOK_FUNCTION = "ShowCursor";
    }

    // public
    //-------------------------------------------------------------------------
    CursorVisibilityHacker::CursorVisibilityHacker():
        _initialized    (false),
        _hook_dll       (NULL),
        _hook_proc      (NULL)
    {
    }

    CursorVisibilityHacker::~CursorVisibilityHacker()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool CursorVisibilityHacker::Initialize()
    {
        // load hook library
        _hook_dll = LoadLibraryA( HOOK_DLL );
        if( _hook_dll != NULL )
        {
            // get hook procedure
            _hook_proc = GetProcAddress( _hook_dll, HOOK_FUNCTION );
            if( _hook_proc != NULL )
            {
                // set initialized
                _initialized = true;
                return true;
            }
        }

        // undo
        Shutdown();

        return false;
    }

    void CursorVisibilityHacker::Shutdown()
    {
        // if hook library loaded
        if( _hook_dll != NULL )
        {
            // free hook library
            FreeLibrary( _hook_dll );

            // reset fields
            _hook_dll = NULL;
            _hook_proc = NULL;
        }

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool CursorVisibilityHacker::IsInitialized() const
    {
        return _initialized;
    }

    //-------------------------------------------------------------------------
    Bool CursorVisibilityHacker::Show( HWND hwnd, ULong timeout_ms )
    {
        DWORD process_id;

        // check if already visible
        if( _IsCursorVisible() )
            return true;

        // get thread id of hwnd
        DWORD hwnd_thread_id = GetWindowThreadProcessId( hwnd, &process_id );
        if( hwnd_thread_id == 0 )
            return false;

        // install winevent hook
        HWINEVENTHOOK hwineventhook = SetWinEventHook(EVENT_MIN, EVENT_MAX, _hook_dll, reinterpret_cast<WINEVENTPROC>(_hook_proc), process_id, hwnd_thread_id, WINEVENT_INCONTEXT );
        if( hwineventhook == NULL )
            return false;

        // run trigger loop to show cursor
        Bool status = _TriggerLoop( true, timeout_ms );

        // uninstall winevent hook
        UnhookWinEvent( hwineventhook );

        return status;
    }

    Bool CursorVisibilityHacker::Hide( HWND hwnd, ULong timeout_ms )
    {
        DWORD process_id;

        // check if already hidden
        if( !_IsCursorVisible() )
            return true;

        // get thread id of hwnd
        DWORD hwnd_thread_id = GetWindowThreadProcessId( hwnd, &process_id );
        if( hwnd_thread_id == 0 )
            return false;

        // install windows hook
        HHOOK hhook = SetWindowsHookEx( WH_CALLWNDPROCRET, reinterpret_cast<HOOKPROC>(_hook_proc), _hook_dll, hwnd_thread_id );
        if( hhook == NULL )
            return false;

        // run trigger loop to hide cursor
        Bool status = _TriggerLoop( false, timeout_ms );

        // uninstall windows hook
        UnhookWindowsHookEx( hhook );

        return status;
    }

    // private
    //-------------------------------------------------------------------------
    Bool CursorVisibilityHacker::_IsCursorVisible()
    {
        CURSORINFO cursor_info = {};
        cursor_info.cbSize = sizeof( CURSORINFO );

        // check cursor visibility state
        return GetCursorInfo( &cursor_info ) && cursor_info.flags != 0;
    }

    //-------------------------------------------------------------------------
    Bool CursorVisibilityHacker::_TriggerLoop( Bool visibility, ULong timeout_ms )
    {
        // for each millisecond
        for( ULong t = 0; t < timeout_ms; ++t )
        {
            INPUT input = {};

            // test visibility
            if( _IsCursorVisible() == visibility )
                return true;

            // build trigger input using tiny mouse movement
            input.type = INPUT_MOUSE;
            input.mi.dx = (t % 2) == 0 ? -1 : 1;
            input.mi.dy = (t % 2) == 0 ? -1 : 1;
            input.mi.time = GetCurrentTime();
            input.mi.dwFlags = MOUSEEVENTF_MOVE;

            // send trigger input
            SendInput( 1, &input, sizeof( input ) );

            // wait 1 ms
            Sleep( 1 );
        }

        return false;
    }
}

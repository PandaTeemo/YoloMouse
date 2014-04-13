#pragma once
#include <Core/Support/Singleton.hpp>
#include <YoloMouse/Dll/CursorBindings.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedState.hpp>

namespace YoloMouse
{
    /**/
    class CursorHook
    {
    private:
        // state
        static Bool             _active;
        static CursorBindings   _bindings;
        static HCURSOR          _last_cursor;
        static HCURSOR          _replace_cursor;

        // input
        static Char             _target_id[STRING_PATH_SIZE];
        static Bool             _assign_ready;
        static Index            _assign_index;
        static Bool             _refresh_ready;

        // objects
        static SharedState&     _state;
        static Hook             _hook_setcursor;
        static Hook             _hook_getcursor;

    public:
        /**/
        static void Load( HWND hwnd );
        static void Unload();

        /**/
        static void Assign( HWND hwnd, Index cursor_index );

        /**/
        static void Refresh( HWND hwnd );

    private:
        /**/
        static Bool _OnSetCursorAssign( HCURSOR hcursor, Index cursor_index );
        static Bool _OnSetCursorChange( HCURSOR hcursor );

        /**/
        static VOID HOOK_CALL _OnSetCursor( x86::Registers registers );

        /* may need this in the future */
        //static VOID HOOK_CALL _OnGetCursor( volatile x86::Registers registers );
    };
}

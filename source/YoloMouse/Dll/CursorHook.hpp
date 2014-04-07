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

        // objects
        static Hook             _hook;
        static SharedState&     _state;

    public:
        /**/
        static void Load( HWND hwnd );
        static void Unload();

        /**/
        static void Assign( HWND hwnd, Index cursor_index );

    private:
        static void _RefreshCursor( HWND hwnd );

        /**/
        static Bool _OnSetCursorAssign( HCURSOR hcursor, Index cursor_index );
        static Bool _OnSetCursorChange( HCURSOR hcursor );

        /**/
        static VOID HOOK_CALL _OnSetCursor( x86::Registers registers );
    };
}

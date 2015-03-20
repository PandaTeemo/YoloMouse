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
        enum Method
        {
            METHOD_SETCURSOR,
            METHOD_SETCLASSLONG,
        };

        // state
        static Bool             _active;
        static CursorBindings   _bindings;
        static HCURSOR          _last_cursor;
        static HCURSOR          _replace_cursor;
        static Method           _method;

        // input
        static WCHAR            _target_id[STRING_PATH_SIZE];
        static Bool             _assign_ready;
        static Index            _assign_index;
        static Bool             _refresh_ready;

        // objects
        static SharedState&     _state;
        static Hook             _hook_setcursor;
        static Hook             _hook_setclasslonga;
        static Hook             _hook_setclasslongw;

    public:
        /**/
        static Bool Load();
        static void Unload();

        /**/
        static Bool Assign( Index cursor_index );

        /**/
        static Bool Refresh();

    private:
        /**/
        static Bool _LoadHooks();
        static void _UnloadHooks();

        /**/
        static Bool _OnCursorAssign( HCURSOR hcursor, Index cursor_index );
        static Bool _OnCursorChanging( HCURSOR hcursor );
        static Bool _OnCursorEvent( HCURSOR& new_cursor, HCURSOR old_cursor );

        /**/
        static VOID HOOK_CALL _OnSetCursor( Native* arguments );
        static VOID HOOK_CALL _OnSetClassLong( Native* arguments );
    };
}

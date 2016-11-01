#pragma once
#include <Core/Support/Singleton.hpp>
#include <YoloMouse/Dll/CursorBindings.hpp>
#include <YoloMouse/Dll/CursorVault.hpp>
#include <YoloMouse/Dll/HandleCache.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedState.hpp>

namespace YoloMouse
{
    /**/
    class App
    {

    public:
        /**/
        static Bool Load();
        static void Unload();

        /**/
        static Bool UpdateCursor( Index resource_index );
        static Bool UpdateSize( Long size_index_delta );
        static Bool UpdateDefault();

        /**/
        static Bool Refresh();

    private:
        /**/
        static Bool _LoadHooks();
        static void _UnloadHooks();

        /**/
        static void _LoadCursors();
        static void _UnloadCursors();

        /**/
        static Bool _OnUpdateGroup( HCURSOR hcursor );
        static Bool _OnUpdateSize( HCURSOR hcursor );
        static void _OnUpdateDefault();

        /**/
        static Bool _OnCursorChanging( HCURSOR hcursor );
        static void _OnCursorHook( HCURSOR& new_cursor, HCURSOR old_cursor );

        /**/
        static VOID HOOK_CALL _OnHookSetCursor( Native* arguments );
        static VOID HOOK_CALL _OnHookSetClassLong( Native* arguments );

        // fields: state
        static Bool             _active;
        static CursorBindings   _bindings;
        static CursorVault      _vault;
        static HandleCache      _cache;
        static HCURSOR          _last_cursor;
        static HCURSOR          _replace_cursor;
        static CursorBindings::Binding* _current_binding;
        static PathString       _target_id;
        static Bool             _refresh_ready;

        // fields: input
        static Index            _update_group;
        static Long             _update_size;
        static Bool             _update_default;

        // fields: hooks
        static Hook             _hook_setcursor;
        static Hook             _hook_setclasslonga;
        static Hook             _hook_setclasslongw;
    };
}

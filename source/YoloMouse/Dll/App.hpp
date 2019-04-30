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
        static Bool UpdatePreset( Index preset_major_index );
        static Bool UpdateSize( Long size_index_delta );

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
        static Bool _LoadCursorByBinding( const CursorBindings::Binding& binding, HCURSOR hcursor=NULL );
        static void _UnloadCursorByBinding( const CursorBindings::Binding& binding );
        static Bool _GetCursorByBinding( HCURSOR& hcursor, const CursorBindings::Binding& binding );

        /**/
        static Bool _OnUpdatePreset( HCURSOR hcursor );
        static Bool _OnUpdateSize( HCURSOR hcursor );

        /**/
        static Bool _OnCursorChanging( HCURSOR hcursor );
        static void _OnCursorHook( HCURSOR& new_cursor, HCURSOR old_cursor );

        /**/
        static VOID HOOK_CALL _OnHookSetCursor( Native* arguments );
        static VOID HOOK_CALL _OnHookSetClassLong( Native* arguments );

        static HCURSOR  _GetClassCursor();
        static void     _SetClassCursor( Native value );
        static void     _SaveClassCursor();
        static void     _RestoreClassCursor();

        // fields: state
        static Bool             _active;
        static HWND             _hwnd;
        static CursorBindings   _bindings;
        static CursorVault      _vault;
        static HandleCache      _cache;
        static HCURSOR          _last_cursor;
        static HCURSOR          _replace_cursor;
        static CursorBindings::Binding* _current_binding;
        static PathString       _target_id;
        static Bool             _refresh_ready;
        static Bool             _setclasslong_self;
        static Native           _classlong_save;

        // fields: input
        static Index            _update_preset_major_index;
        static Long             _update_size_delta;

        // fields: hooks
        static Hook             _hook_setcursor;
        static Hook             _hook_setclasslonga;
        static Hook             _hook_setclasslongw;
    };
}

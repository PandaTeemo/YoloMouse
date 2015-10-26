#include <YoloMouse/Dll/App.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

namespace YoloMouse
{
    // fields
    //-------------------------------------------------------------------------
    Bool            App::_active             (false);
    CursorBindings  App::_bindings;
    CursorVault     App::_vault;
    HandleCache     App::_cache;
    HCURSOR         App::_last_cursor        (NULL);
    HCURSOR         App::_replace_cursor     (NULL);
    App::Method     App::_method             (App::METHOD_SETCURSOR);
    PathString      App::_target_id;
    Bool            App::_refresh_ready      (false);

    Index           App::_update_group       (INVALID_INDEX);
    Long            App::_update_size        (0);

    Hook            App::_hook_setcursor     (SetCursor, App::_OnHookSetCursor, Hook::BEFORE);
#if CPU_64
    Hook            App::_hook_setclasslonga (SetClassLongPtrA, App::_OnHookSetClassLong, Hook::BEFORE);
    Hook            App::_hook_setclasslongw (SetClassLongPtrW, App::_OnHookSetClassLong, Hook::BEFORE);
#else
    Hook            App::_hook_setclasslonga (SetClassLongA, App::_OnHookSetClassLong, Hook::BEFORE);
    Hook            App::_hook_setclasslongw (SetClassLongW, App::_OnHookSetClassLong, Hook::BEFORE);
#endif

     // public
    //-------------------------------------------------------------------------
    Bool App::Load()
    {
        SharedState& state = SharedState::Instance();

        // if not already active
        if( _active )
            return true;

        // build id string
        if( !SharedTools::BuildTargetId(_target_id, COUNT(_target_id), GetCurrentProcess()) )
            return false;

        // load state
        if( !state.Open(false) )
            return false;

        // load hooks
        if( !_LoadHooks() )
            return false;

        // load cursors
        _LoadCursors();

        // activate
        _active = true;

        // refresh cursor
        Refresh();

        return true;
    }

    void App::Unload()
    {
        SharedState& state = SharedState::Instance();

        // ignore if already inactive
        if( !_active )
            return;

        // unload cursors
        _UnloadCursors();

        // unload hooks
        _UnloadHooks();

        // unload state
        state.Close();

        // reset
        _update_size = 0;
        _update_group = INVALID_INDEX;
        _active = false;
    }

    //-------------------------------------------------------------------------
    Bool App::UpdateCursor( Index group_index )
    {
        // require active
        if( !_active )
            return false;

        // mark for update
        _update_group = group_index;

        // refresh cursor
        Refresh();

        return true;
    }

    Bool App::UpdateSize( Long size_index_delta )
    {
        // require active
        if( !_active )
            return false;

        // mark for update
        _update_size = size_index_delta;

        // refresh cursor
        Refresh();

        return true;
    }

    //-------------------------------------------------------------------------
    Bool App::Refresh()
    {
        DWORD process_id;

        // require active
        if( !_active )
            return false;

        // get last cursor
        HCURSOR refresh_cursor = _last_cursor;

        // get active window
        HWND hwnd = GetForegroundWindow();

        // get thread and process id of this window
        DWORD hwnd_thread_id = GetWindowThreadProcessId(hwnd, &process_id);

        // require active window belongs to this process. otherwise dont bother refreshing.
        if( process_id != GetCurrentProcessId() )
            return false;

        // get thread id of this thread (the one Loader's CreateRemoteThread created).
        DWORD current_thread_id = GetCurrentThreadId();

        // attach to window thread. this is to make GetCursor and SetCursor work properly
        if( AttachThreadInput(hwnd_thread_id, current_thread_id, TRUE) )
        {
            // if does not exist
            if( refresh_cursor == NULL )
            {
                // get active windows cursor
                refresh_cursor = GetCursor();

                // cannot be yolomouse cursor
                if( _vault.HasCursor(refresh_cursor) )
                {
                    AttachThreadInput(hwnd_thread_id, current_thread_id, FALSE);
                    return false;
                }
            }

            // set refresh state
            _refresh_ready = true;

            // refresh according to method
            if( _method == METHOD_SETCLASSLONG )
            {
                // set current cursor to force update
            #if CPU_64
                SetClassLongPtrA(hwnd, GCLP_HCURSOR, (LONG_PTR)refresh_cursor);
            #else
                SetClassLongA(hwnd, GCL_HCURSOR, (LONG)refresh_cursor);
            #endif
            }
            else
            {
                // set current cursor to force update
                SetCursor(refresh_cursor);

                // then trigger application to call SetCursor with its own cursor
                PostMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            }

            // detach from window thread
            AttachThreadInput(hwnd_thread_id, current_thread_id, FALSE);
        }

        return true;
    }

    // private
    //-------------------------------------------------------------------------
    Bool App::_LoadHooks()
    {
        // init hooks
        if( !_hook_setcursor.Init() )
            return false;
        if( !_hook_setclasslonga.Init() )
            return false;
        if( !_hook_setclasslongw.Init() )
            return false;

        // enable hooks
        if( !_hook_setcursor.Enable() )
            return false;
        if( !_hook_setclasslonga.Enable() )
            return false;
        if( !_hook_setclasslongw.Enable() )
            return false;

        return true;
    }

    void App::_UnloadHooks()
    {
        // disable hooks
        _hook_setclasslongw.Disable();
        _hook_setclasslonga.Disable();
        _hook_setcursor.Disable();
    }

    //-------------------------------------------------------------------------
    void App::_LoadCursors()
    {
        const CursorBindings::MapTable& map = _bindings.GetMap();

        // load cursor map from file
        _bindings.Load(_target_id);

        // for each map entry
        for( CursorBindings::MapIterator cm = map.Begin(); cm != map.End(); ++cm )
        {
            // load cursor
            if( cm->bitmap_hash != 0 )
                _vault.Load(cm->resource_index, cm->size_index);
        }
    }

    void App::_UnloadCursors()
    {
        // unload cursors
        _vault.UnloadAll();
    }

    //-------------------------------------------------------------------------
    Bool App::_OnUpdateGroup( HCURSOR hcursor )
    {
        Index size_index = CURSOR_SIZE_DEFAULT;
        Index group_index = _update_group;
        Index last_resource_index = INVALID_INDEX;

        // clear state
        _update_group = INVALID_INDEX;

        // cannot be yolomouse cursor
        if( _vault.HasCursor(hcursor) )
            return false;

        // get hash of cursor
        Hash cursor_hash = _cache.GetHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
            return false;

        // get cursor binding
        CursorBindings::Binding* binding = _bindings.GetBinding(cursor_hash);

        // remove previous binding
        if( binding )
        {
            // save last resource index
            last_resource_index = binding->resource_index;

            // unload cursor
            _vault.Unload(last_resource_index, binding->size_index);

            // save size index
            size_index = binding->size_index;

            // remove binding
            _bindings.Remove(*binding);
        }

        // add new binding unless explicitly removing
        if( group_index != CURSOR_SPECIAL_REMOVE && group_index < CURSOR_GROUP_COUNT )
        {
            Index cursor_index = 0;
            Index resource_index = INVALID_INDEX;

            // if has previous resource index and same group iterate that cursor index
            if( last_resource_index != INVALID_INDEX && group_index == (last_resource_index / CURSOR_GROUP_SIZE) )
                cursor_index = (last_resource_index + 1) % CURSOR_GROUP_SIZE;

            // load attempt loop
            for( Index load_attempt = 0; load_attempt < CURSOR_GROUP_SIZE; load_attempt++, cursor_index = (cursor_index + 1) % CURSOR_GROUP_SIZE )
            {
                // calculate resource index
                resource_index = (group_index * CURSOR_GROUP_SIZE) + cursor_index;

                // if valid index load cursor
                if( resource_index < CURSOR_RESOURCE_LIMIT && _vault.Load(resource_index, size_index) )
                    break;
            }

            // fail if nothing loaded
            if( resource_index == INVALID_INDEX )
                return false;

            // add binding
            if( _bindings.Add(cursor_hash, resource_index, size_index) == NULL )
                return false;
        }

        // save cursor map to file
        _bindings.Save(_target_id);

        return true;
    }

    Bool App::_OnUpdateSize( HCURSOR hcursor )
    {
        Long size_index_delta = _update_size;

        // clear state
        _update_size = 0;

        // cannot be yolomouse cursor
        if( _vault.HasCursor(hcursor) )
            return false;

        // get hash of cursor
        Hash cursor_hash = _cache.GetHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
            return false;

        // get cursor binding
        CursorBindings::Binding* binding = _bindings.GetBinding(cursor_hash);

        // require binding
        if( binding == NULL )
            return false;

        // unload previous cursor
        _vault.Unload(binding->resource_index, binding->size_index);

        // calculate new size index
        Index size_index = Tools::Clamp<Long>(binding->size_index + size_index_delta, 0, CURSOR_SIZE_COUNT - 1);
        
        // load cursor with new size
        if( !_vault.Load(binding->resource_index, size_index) )
            return false;

        // update binding size index
        binding->size_index = size_index;

        // save cursor map to file
        _bindings.Save(_target_id);

        return true;
    }

    //-------------------------------------------------------------------------
    Bool App::_OnCursorChanging( HCURSOR hcursor )
    {
        // cannot be yolomouse cursor
        if( _vault.HasCursor(hcursor) )
            return false;

        // get hash of cursor
        Hash cursor_hash = _cache.GetHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
            return false;

        // reset replacement cursor
        _replace_cursor = NULL;

        // get cursor binding
        const CursorBindings::Binding* binding = _bindings.GetBinding(cursor_hash);

        // if has binding
        if( binding )
        {
            // update replacement cursor
            _replace_cursor = _vault.GetCursor(binding->resource_index, binding->size_index);
        }

        return true;
    }

    //-------------------------------------------------------------------------
    void App::_OnCursorHook( HCURSOR& new_cursor, HCURSOR old_cursor )
    {
        // ignore null cursor
        if( old_cursor == NULL )
            return;

        // if cursor changing set refresh state
        if( old_cursor != _last_cursor )
            _refresh_ready = true;

        // set new last cursor
        _last_cursor = old_cursor;

        // if updating new cursor
        if( _update_group != INVALID_INDEX )
        {
            // handle cursor update
            if(!_OnUpdateGroup(old_cursor))
                return;

            // set refresh state
            _refresh_ready = true;
        }

        // if updating size
        if( _update_size != 0 )
        {
            // handle cursor size
            if(!_OnUpdateSize(old_cursor))
                return;

            // set refresh state
            _refresh_ready = true;
        }

        // if refreshing cursor
        if( _refresh_ready )
        {
            // clear refresh state
            _refresh_ready = false;

            // handle cursor change
            if(!_OnCursorChanging(old_cursor))
                return;
        }

        // fail if no replacement
        if( _replace_cursor == NULL )
            return;
        
        // return replacement cursor
        new_cursor = _replace_cursor;
    }

    //-------------------------------------------------------------------------
    VOID HOOK_CALL App::_OnHookSetCursor( Native* arguments )
    {
        // update cursor
        _OnCursorHook((HCURSOR&)arguments[1], (HCURSOR)arguments[1]);
    }

    VOID HOOK_CALL App::_OnHookSetClassLong( Native* arguments )
    {
        // if changing cursor
    #if CPU_64
        if((int)arguments[2] == GCLP_HCURSOR)
    #else
        if((int)arguments[2] == GCL_HCURSOR)
    #endif
        {
            // change method
            _method = METHOD_SETCLASSLONG;

            // update cursor
            _OnCursorHook((HCURSOR&)arguments[3], (HCURSOR)arguments[3]);
        }
    }
}

#include <YoloMouse/Dll/App.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <Core/Windows/SystemTools.hpp>

namespace YoloMouse
{
    // fields
    //-------------------------------------------------------------------------
    Bool            App::_active                (false);
    HWND            App::_hwnd =                NULL;
    CursorBindings  App::_bindings;
    CursorVault     App::_vault;
    HandleCache     App::_cache;
    HCURSOR         App::_last_cursor           (NULL);
    HCURSOR         App::_replace_cursor        (NULL);
    CursorBindings::Binding* App::_current_binding(NULL);
    PathString      App::_target_id;
    Bool            App::_refresh_ready         (false);
    Native          App::_classlong_original =  0;
    Native          App::_classlong_last =      0;

    Index           App::_update_group          (INVALID_INDEX);
    Long            App::_update_size           (0);
    Bool            App::_update_default        (false);

    Hook            App::_hook_setcursor        (SetCursor, App::_OnHookSetCursor, Hook::BEFORE);
#if CPU_64
    Hook            App::_hook_setclasslonga    (SetClassLongPtrA, App::_OnHookSetClassLong, Hook::BEFORE);
    Hook            App::_hook_setclasslongw    (SetClassLongPtrW, App::_OnHookSetClassLong, Hook::BEFORE);
#else
    Hook            App::_hook_setclasslonga    (SetClassLongA, App::_OnHookSetClassLong, Hook::BEFORE);
    Hook            App::_hook_setclasslongw    (SetClassLongW, App::_OnHookSetClassLong, Hook::BEFORE);
#endif

     // public
    //-------------------------------------------------------------------------
    Bool App::Load()
    {
        SharedState& state = SharedState::Instance();

        // if not already active
        if( _active )
            return true;

        // load state
        if( !state.Open(false) )
            return false;

        // build id string
        if( !SharedTools::BuildTargetId(_target_id, COUNT(_target_id), GetCurrentProcess()) )
        {
            elog("DllApp.Load.BuildTargetId");
            return false;
        }

        // backup class long value
        _SaveCursorClassLong();

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

        // restore original classlong
        _RestoreCursorClassLong();

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
        {
            elog("DllApp.UpdateCursor.NotActive");
            return false;
        }

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
        {
            elog("DllApp.UpdateSize.NotActive");
            return false;
        }

        // mark for update
        _update_size = size_index_delta;

        // refresh cursor
        Refresh();

        return true;
    }

    Bool App::UpdateDefault()
    {
        // require active
        if( !_active )
        {
            elog("DllApp.UpdateDefault.NotActive");
            return false;
        }

        // mark for update
        _update_default = true;

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
        {
            elog("DllApp.Refresh.NotActive");
            return false;
        }

        // get last cursor
        HCURSOR refresh_cursor = _last_cursor;

        // get target window
        HWND hwnd = SystemTools::GetFocusWindow();
        if( hwnd == NULL )
        {
            elog("DllApp.Refresh.GetFocusWindow");
            return false;
        }

        // get thread and process id of this window
        DWORD hwnd_thread_id = GetWindowThreadProcessId(hwnd, &process_id);

        // require active window belongs to this process. otherwise dont bother refreshing.
        if( process_id != GetCurrentProcessId() )
        {
            elog("DllApp.Refresh.WindowNotCurrentProcess");
            return false;
        }

        // update active window
        _hwnd = hwnd;

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
                    elog("DllApp.Refresh.IsYoloCursor");
                    return false;
                }
            }

            // set refresh state
            _refresh_ready = true;

            // set current cursor to force update
            SetCursor( refresh_cursor );

            // then trigger application to call SetCursor with its own cursor
            PostMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));

            // detach from window thread
            AttachThreadInput(hwnd_thread_id, current_thread_id, FALSE);
        }
        else
            elog("DllApp.Refresh.AttachThreadInput");

        return true;
    }

    // private
    //-------------------------------------------------------------------------
    Bool App::_LoadHooks()
    {
        // init hooks
        if( !_hook_setcursor.Init() )
        {
            elog("DllApp.LoadHooks.Init.SetCursor");
            return false;
        }
        if( !_hook_setclasslonga.Init() )
        {
            elog("DllApp.LoadHooks.Init.SetClassLongA");
            return false;
        }
        if( !_hook_setclasslongw.Init() )
        {
            elog("DllApp.LoadHooks.Init.SetClassLongW");
            return false;
        }

        // enable hooks
        if( !_hook_setcursor.Enable() )
        {
            elog("DllApp.LoadHooks.Enable.SetCursor");
            return false;
        }
        if( !_hook_setclasslonga.Enable() )
        {
            elog("DllApp.LoadHooks.Enable.SetClassLongA");
            return false;
        }
        if( !_hook_setclasslongw.Enable() )
        {
            elog("DllApp.LoadHooks.Enable.SetClassLongW");
            return false;
        }

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

        // get default
        CursorBindings::Binding& default_binding = _bindings.EditDefault();
        if( default_binding.Isvalid() )
        {
            // load default cursor
            _vault.Load(default_binding.resource_index, default_binding.size_index);
        }

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
        Index size_index =          CURSOR_INDEX_DEFAULT;
        Index group_index =         _update_group;
        Index last_resource_index = INVALID_INDEX;

        // clear state
        _update_group = INVALID_INDEX;

        // cannot be yolomouse cursor
        if( _vault.HasCursor(hcursor) )
        {
            elog("DllApp.OnUpdateGroup.IsYoloCursor");
            return false;
        }

        // get hash of cursor
        Hash cursor_hash = _cache.GetHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
        {
            elog("DllApp.OnUpdateGroup.InvalidHash");
            return false;
        }

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

        // add new binding if not explicitly removing
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
            {
                elog("DllApp.OnUpdateGroup.NothingLoaded");
                return false;
            }

            // add binding
            if( _bindings.Add(cursor_hash, resource_index, size_index) == NULL )
            {
                elog("DllApp.OnUpdateGroup.AddBinding");
                return false;
            }
        }
        // else if removing
        else
        {
            // if no previous binding
            if( binding == NULL )
            {
                // update default (to clear it)
                _update_default = true;
            }
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
        {
            elog("DllApp.OnUpdateSize.IsYoloCursor");
            return false;
        }

        // get hash of cursor
        Hash cursor_hash = _cache.GetHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
        {
            elog("DllApp.OnUpdateSize.InvalidHash");
            return false;
        }

        // get cursor binding
        CursorBindings::Binding* binding = _bindings.GetBinding(cursor_hash);

        // require binding
        if( binding == NULL )
            return false;

        // unload previous cursor
        _vault.Unload(binding->resource_index, binding->size_index);

        // calculate new size index
        Index size_index = Tools::Clamp<Long>(binding->size_index + size_index_delta, 0, CURSOR_INDEX_COUNT - 1);
        
        // load cursor with new size
        if( !_vault.Load(binding->resource_index, size_index) )
            return false;

        // update binding size index
        binding->size_index = size_index;

        // save cursor map to file
        _bindings.Save(_target_id);

        return true;
    }

    void App::_OnUpdateDefault()
    {
        // clear state
        _update_default = false;

        // get default binding
        CursorBindings::Binding& default_binding = _bindings.EditDefault();

        // if valid unload that cursor
        if( default_binding.Isvalid() )
            _vault.Unload(default_binding.resource_index, default_binding.size_index);

        // if has current binding make that the default
        if( _current_binding )
            default_binding = *_current_binding;
        // else clear default binding
        else
            default_binding = CursorBindings::Binding();

        // if valid load that cursor
        if( default_binding.Isvalid() )
            _vault.Load(default_binding.resource_index, default_binding.size_index);

        // save cursor map to file
        _bindings.Save(_target_id);
    }

    //-------------------------------------------------------------------------
    Bool App::_OnCursorChanging( HCURSOR hcursor )
    {
        // cannot be yolomouse cursor
        if( _vault.HasCursor(hcursor) )
        {
            elog("DllApp.OnCursorChanging.IsYoloCursor");
            return false;
        }

        // get hash of cursor
        Hash cursor_hash = _cache.GetHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
        {
            elog("DllApp.OnCursorChanging.InvalidHash");
            return false;
        }

        // reset replacement cursor
        _replace_cursor = NULL;

        // get cursor binding
        _current_binding = _bindings.GetBinding(cursor_hash);

        // if has binding
        if( _current_binding )
        {
            // update replacement cursor
            _replace_cursor = _vault.GetCursor(_current_binding->resource_index, _current_binding->size_index);
        }
        // else try default
        else
        {
            // get default
            CursorBindings::Binding& default_binding = _bindings.EditDefault();

            // replace if valid
            if( default_binding.Isvalid() )
                _replace_cursor = _vault.GetCursor(default_binding.resource_index, default_binding.size_index);
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

        // clear last cursor
        _last_cursor = NULL;

        // if updating new cursor
        if( _update_group != INVALID_INDEX )
        {
            // handle cursor group
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

        // if updating default
        if( _update_default )
        {
            // handle cursor default
            _OnUpdateDefault();
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
        
        // set new last cursor
        _last_cursor = old_cursor;

        // return replacement cursor
        new_cursor = _replace_cursor;
    }

    //-------------------------------------------------------------------------
    VOID HOOK_CALL App::_OnHookSetCursor( Native* arguments )
    {
        // backup class long value
        _SaveCursorClassLong();

        // update cursor using setclasslong method first
        _SetCursorClassLong( arguments[1] );

        // if replacement cursor was set pass it out to setcursor method
        if( _replace_cursor != NULL )
            arguments[1] = (Native)_replace_cursor;
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
            // update cursor
            _OnCursorHook((HCURSOR&)arguments[3], (HCURSOR)arguments[3]);
        }
    }

    //-------------------------------------------------------------------------
    void App::_SetCursorClassLong( Native value )
    {
        // set class long value
    #if CPU_64
        SetClassLongPtrA( _hwnd, GCLP_HCURSOR, (LONG_PTR)value );
    #else
        SetClassLongA( _hwnd, GCL_HCURSOR, (LONG)value );
    #endif

        // set last class long value
        _classlong_last = value;
    }

    void App::_SaveCursorClassLong()
    {
        Native current_value;

        // get current class long value
    #if CPU_64
        current_value = (Native)GetClassLongPtrA( _hwnd, GCLP_HCURSOR );
    #else
        current_value = (Native)GetClassLongA( _hwnd, GCL_HCURSOR );
    #endif

        // if not last then update as original value
        if( current_value != _classlong_last )
            _classlong_original = current_value;
    }

    void App::_RestoreCursorClassLong()
    {
        // if any changes made
        if( _classlong_last )
        {
            // get current original class long
            _SaveCursorClassLong();

            // restore original class long
            _SetCursorClassLong( _classlong_original );
        }
    }
}

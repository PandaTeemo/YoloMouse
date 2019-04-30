#include <YoloMouse/Dll/App.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <Core/Windows/SystemTools.hpp>

namespace YoloMouse
{
    // fields
    //-------------------------------------------------------------------------
    Bool            App::_active =                      false;
    HWND            App::_hwnd =                        NULL;
    CursorBindings  App::_bindings;
    CursorVault     App::_vault;
    HandleCache     App::_cache;
    HCURSOR         App::_last_cursor =                 NULL;
    HCURSOR         App::_replace_cursor =              NULL;
    CursorBindings::Binding* App::_current_binding =    NULL;
    PathString      App::_target_id;
    Bool            App::_refresh_ready =               false;
    Bool            App::_setclasslong_self =           false;
    Native          App::_classlong_save =              INVALID_HANDLE;

    Index           App::_update_preset_major_index =   INVALID_INDEX;
    Long            App::_update_size_delta =           0;

    Hook            App::_hook_setcursor                (SetCursor, App::_OnHookSetCursor, Hook::BEFORE);
#if CPU_64
    Hook            App::_hook_setclasslonga            (SetClassLongPtrA, App::_OnHookSetClassLong, Hook::BEFORE);
    Hook            App::_hook_setclasslongw            (SetClassLongPtrW, App::_OnHookSetClassLong, Hook::BEFORE);
#else
    Hook            App::_hook_setclasslonga            (SetClassLongA, App::_OnHookSetClassLong, Hook::BEFORE);
    Hook            App::_hook_setclasslongw            (SetClassLongW, App::_OnHookSetClassLong, Hook::BEFORE);
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

        // restore original class cursor
        _RestoreClassCursor();

        // unload state
        state.Close();

        // reset fields
        _update_size_delta = 0;
        _update_preset_major_index = INVALID_INDEX;
        _active = false;
    }

    //-------------------------------------------------------------------------
    Bool App::UpdatePreset( Index preset_major_index )
    {
        // require active
        if( !_active )
        {
            elog("DllApp.UpdateCursor.NotActive");
            return false;
        }

        // mark for update
        _update_preset_major_index = preset_major_index;

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

        // mark size index delta update
        _update_size_delta = size_index_delta;

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

        // get focus window
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
            // if refresh cursor does not exist
            if( refresh_cursor == NULL )
            {
                // get current application class cursor
                refresh_cursor = _GetClassCursor();
                
                // if does not exist, try global windows cursor (error prone)
                if( refresh_cursor == NULL )
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
            PostMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));

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
        // load cursor map from file
        _bindings.Load(_target_id);

        // for each binding
        for( CursorBindings::Binding& binding: _bindings.GetBindings() )
        {
            // load preset cursor
            if( binding.cursor_hash != 0 )
                _vault.LoadPreset(binding.preset_index, binding.size_index);
        }

        // identity cursors will be loaded on demand
    }

    void App::_UnloadCursors()
    {
        // unload all cursors
        _vault.UnloadAll();
    }

    //-------------------------------------------------------------------------
    Bool App::_LoadCursorByBinding( const CursorBindings::Binding& binding, HCURSOR hcursor )
    {
        // by resource type
        switch( binding.resource_type )
        {
        case RESOURCE_IDENTITY:
            // load identity cursor
            return _vault.LoadIdentity(binding.cursor_hash, binding.size_index, hcursor);
        case RESOURCE_PRESET:
            // load preset cursor
            return _vault.LoadPreset(binding.preset_index, binding.size_index);
        default:
            return false;
        }
    }

    void App::_UnloadCursorByBinding( const CursorBindings::Binding& binding )
    {
        // by resource type
        switch( binding.resource_type )
        {
        case RESOURCE_IDENTITY:
            // unload identity cursor
            _vault.UnloadIdentity(binding.cursor_hash, binding.size_index);
            break;
        case RESOURCE_PRESET:
            // unload preset cursor
            _vault.UnloadPreset(binding.preset_index, binding.size_index);
            break;
        default:;
        }
    }

    Bool App::_GetCursorByBinding( HCURSOR& hcursor, const CursorBindings::Binding& binding )
    {
        // by resource type
        switch( binding.resource_type )
        {
        case RESOURCE_IDENTITY:
            // get identity cursor
            hcursor = _vault.GetIdentity(binding.cursor_hash, binding.size_index);
            return hcursor != NULL;
        case RESOURCE_PRESET:
            // get preset cursor
            hcursor = _vault.GetPreset(binding.preset_index, binding.size_index);
            return hcursor != NULL;
        default:
            return false;
        }
    }

    //-------------------------------------------------------------------------
    Bool App::_OnUpdatePreset( HCURSOR hcursor )
    {
        Index size_index =          CURSOR_SIZE_INDEX_DEFAULT;
        Index preset_major_index =  _update_preset_major_index;
        Index last_preset_index =   INVALID_INDEX;

        // clear state
        _update_preset_major_index = INVALID_INDEX;

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
        CursorBindings::Binding* binding = _bindings.GetBindingByHash(cursor_hash);

        // remove previous binding
        if( binding )
        {
            // save last preset index
            last_preset_index = binding->preset_index;

            // unload previous cursor
            _UnloadCursorByBinding( *binding );

            // keep last size index
            size_index = binding->size_index;

            // remove binding
            _bindings.Remove(*binding);
        }

        // add new binding if not explicitly removing
        if( preset_major_index != CURSOR_SPECIAL_REMOVE && preset_major_index < CURSOR_RESOURCE_PRESET_MAJOR_COUNT )
        {
            CursorBindings::Binding binding;
            Index                   preset_minor_index = 0;
            Index                   preset_index = INVALID_INDEX;
            Bool                    loaded = false;

            // if has previous preset index and same major index iterate minor index
            if( last_preset_index != INVALID_INDEX && preset_major_index == (last_preset_index / CURSOR_RESOURCE_PRESET_MINOR_COUNT) )
                preset_minor_index = (last_preset_index + 1) % CURSOR_RESOURCE_PRESET_MINOR_COUNT;

            // load attempt loop
            for( Index load_attempt = 0; load_attempt < CURSOR_RESOURCE_PRESET_MINOR_COUNT; load_attempt++, preset_minor_index = (preset_minor_index + 1) % CURSOR_RESOURCE_PRESET_MINOR_COUNT )
            {
                // calculate preset index
                preset_index = (preset_major_index * CURSOR_RESOURCE_PRESET_MINOR_COUNT) + preset_minor_index;

                // load if valid preset index
                if( preset_index < CURSOR_RESOURCE_PRESET_COUNT && _vault.LoadPreset( preset_index, size_index ) )
                {
                    loaded = true;
                    break;
                }
            }

            // fail if nothing loaded
            if( !loaded )
            {
                elog("DllApp.OnUpdateGroup.NothingLoaded");
                return false;
            }

            // create binding
            binding.cursor_hash = cursor_hash;
            binding.size_index = size_index;
            binding.resource_type = RESOURCE_PRESET;
            binding.preset_index = preset_index;

            // add binding
            if( _bindings.Add(binding) == NULL )
            {
                elog("DllApp.OnUpdateGroup.AddBinding");
                return false;
            }
        }

        // save cursor bindings
        _bindings.Save(_target_id);

        return true;
    }

    Bool App::_OnUpdateSize( HCURSOR hcursor )
    {
        Long  size_index_delta = _update_size_delta;

        // clear size state
        _update_size_delta = 0;

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
        CursorBindings::Binding* binding = _bindings.GetBindingByHash(cursor_hash);

        // if has binding
        if( binding != NULL )
        {
            // unload previous cursor
            _UnloadCursorByBinding( *binding );
        }
        // create identity binding (bind current application cursor for resizing)
        else
        {
            CursorBindings::Binding identity_binding;

            // create identity binding
            identity_binding.cursor_hash = cursor_hash;
            identity_binding.resource_type = RESOURCE_IDENTITY;

            // select initial size index as nearest current cursor's size
            identity_binding.size_index = SharedTools::CursorSizeToSizeIndex( SharedTools::CursorToSize( hcursor ) );

            // add identity binding
            binding = _bindings.Add( identity_binding );
            if( binding == NULL )
            {
                elog("DllApp.OnUpdateSize.AddBinding");
                return false;
            }
        }

        // calculate new size index
        binding->size_index = Tools::Clamp<Long>(binding->size_index + size_index_delta, 0, CURSOR_SIZE_INDEX_COUNT - 1);

        // load cursor with new size
        if( !_LoadCursorByBinding(*binding, hcursor) )
            return false;

        // save cursor map to file
        _bindings.Save(_target_id);

        return true;
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
        _current_binding = _bindings.GetBindingByHash(cursor_hash);

        // fail if no binding
        if( _current_binding == nullptr )
            return false;

        // success if get replacement cursor from binding
        if( _GetCursorByBinding( _replace_cursor, *_current_binding ) )
            return true;

        // if identity binding
        if( _current_binding->resource_type == RESOURCE_IDENTITY )
        {
            // if failed and identity cursor, then attempt to load and try again
            if( !_LoadCursorByBinding(*_current_binding, hcursor) )
                return false;

            // try again
            return _GetCursorByBinding( _replace_cursor, *_current_binding );
        }

        return false;
    }

    //-------------------------------------------------------------------------
    void App::_OnCursorHook( HCURSOR& new_cursor, HCURSOR old_cursor )
    {
        // ignore null cursor
        if( old_cursor == NULL )
        {
            _replace_cursor = NULL;
            return;
        }

        // if cursor changing set refresh state
        if( old_cursor != _last_cursor )
            _refresh_ready = true;

        // clear last cursor
        _last_cursor = NULL;

        // if updating new cursor
        if( _update_preset_major_index != INVALID_INDEX )
        {
            // handle cursor preset update
            if(!_OnUpdatePreset(old_cursor))
                return;

            // set refresh state
            _refresh_ready = true;
        }

        // if updating size
        if( _update_size_delta != 0 )
        {
            // handle cursor size update
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
        
        // set new last cursor
        _last_cursor = old_cursor;

        // return replacement cursor
        new_cursor = _replace_cursor;
    }

    //-------------------------------------------------------------------------
    VOID HOOK_CALL App::_OnHookSetCursor( Native* arguments )
    {
        // update cursor using class method first
        _SetClassCursor( arguments[1] );

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
            // backup class cursor value if not yet saved. yes do it here ;)
            if( _classlong_save == INVALID_HANDLE )
                _SaveClassCursor();

            // if not self calling this, save classlong value to be restored on exit
            if( !_setclasslong_self )
                _classlong_save = static_cast<Native>(arguments[3]);

            // update cursor
            _OnCursorHook((HCURSOR&)arguments[3], (HCURSOR)arguments[3]);
        }
    }

    //-------------------------------------------------------------------------
    HCURSOR App::_GetClassCursor()
    {
        // save current class long value
    #if CPU_64
        return (HCURSOR)GetClassLongPtrA( _hwnd, GCLP_HCURSOR );
    #else
        return (HCURSOR)GetClassLongA( _hwnd, GCL_HCURSOR );
    #endif
    }

    void App::_SetClassCursor( Native value )
    {
        // set self caller state
        _setclasslong_self = true;

        // set class long value
    #if CPU_64
        SetClassLongPtrA( _hwnd, GCLP_HCURSOR, (LONG_PTR)value );
    #else
        SetClassLongA( _hwnd, GCL_HCURSOR, (LONG)value );
    #endif

        // reset self caller state
        _setclasslong_self = false;
    }

    void App::_SaveClassCursor()
    {
        // save current class cursor
        _classlong_save = reinterpret_cast<Native>(_GetClassCursor());
    }

    void App::_RestoreClassCursor()
    {
        // if any changes made
        if( _classlong_save != INVALID_HANDLE )
        {
            // restore original class long
            _SetClassCursor( _classlong_save );
        }
    }
}

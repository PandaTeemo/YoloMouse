#include <Core/System/Debug.hpp>
#include <Core/System/SystemTools.hpp>
#include <Core/Windows/WindowTools.hpp>
#include <YoloMouse/Dll/Core/App.hpp>
#include <YoloMouse/Share/Cursor/CursorBindingsSerializer.hpp>
#include <YoloMouse/Share/Cursor/CursorTools.hpp>

namespace Yolomouse
{
     // local
    //-------------------------------------------------------------------------
    namespace
    {
        // dll unloader
        //---------------------------------------------------------------------
        DWORD WINAPI _DllUnloader( _In_ LPVOID lpParameter )
        {
            // run
            FreeLibraryAndExitThread( reinterpret_cast<HINSTANCE>(lpParameter), 0 );
            return 0;
        }
    }

    // public
    //-------------------------------------------------------------------------
    App::App():
        _hdll                   (NULL),
        _initialized            (false),
        _loaded                 (false),
        _update_size_delta      (0)
    {
        _bindings_path.Zero();
    }

    App::~App()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool App::IsInitialized() const
    {
        return _initialized;
    }

    Bool App::IsLoaded() const
    {
        return _loaded;
    }

    //-------------------------------------------------------------------------
    Bool App::Initialize( HINSTANCE hdll )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _hdll = hdll;

        // initialize ipc
        if( !_ipc.Initialize( GetCurrentProcessId(), *this ) )
            return false;

        // set initialized
        _initialized = true;

        return true;
    }

    void App::Shutdown()
    {
        ASSERT( IsInitialized() );
        ASSERT( !IsLoaded() );

        // shutdown ipc
        _ipc.Shutdown();

        // reset fields
        _hdll = NULL;
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool App::Load( const PathString& host_path, const PathString& log_path, const PathString& bindings_path )
    {
        ASSERT( IsInitialized() );
        ASSERT( !IsLoaded() );

        // initialize debug
        Debug::Initialize( log_path, false );
        LOG2( "DllApp.Load" );

        // set fields
        _bindings_path = bindings_path;

        // initialized cursor vault
        _cursor_vault.Initialize( host_path );

        // load bindings
        _LoadBindings();

        // load hooks
        if( !_LoadHooks() )
            return false;

        // set loaded
        _loaded = true;

        // refresh current cursor
        _RefreshCurrentCursor();

        return true;
    }

    void App::Unload()
    {
        ASSERT( IsLoaded() );

        // unload hooks
        _UnloadHooks();

        // shutdown cursor vault
        _cursor_vault.Shutdown();

        // restore class cursors
        _RestoreClassCursors();

        // shutdown debug
        LOG2( "DllApp.Unload" );
        Debug::Shutdown();

        // reset loaded
        _loaded = false;
    }

    // private: implement:IpcMessenger::IListener
    //-------------------------------------------------------------------------
    void App::OnRecv( const IpcMessage& message )
    {
        LOG2( "DllApp.OnRecv: %u", message.request );

        // handle ipc request
        switch( message.request )
        {
        case IPC_REQUEST_LOAD:
            _OnRequestLoad(reinterpret_cast<const LoadIpcMessage&>(message));
            break;
        case IPC_REQUEST_EXIT:
            _OnRequestExit();
            break;
        case IPC_REQUEST_SET_CURSOR:
            _OnRequestSetCursor(reinterpret_cast<const SetCursorIpcMessage&>(message));
            break;
        case IPC_REQUEST_SET_DEFAULT_CURSOR:
            _OnRequestSetDefaultCursor();
            break;
        case IPC_REQUEST_RESET_CURSOR:
            _OnRequestResetCursor();
            break;
        case IPC_REQUEST_REFRESH_CURSOR:
            _RefreshCurrentCursor();
            break;
        }
    }

    // private
    //-------------------------------------------------------------------------
    void App::_LoadBindings()
    {
        // load cursor map from file (can fail)
        CursorBindingsSerializer::Load( _bindings, _bindings_path );
    }

    Bool App::_LoadHooks()
    {
        // initialize hooks
        if( !_hook_setcursor.Initialize(::SetCursor, _OnHookSetCursor) )
        {
            LOG("DllApp.LoadHooks.Init.SetCursor");
            return false;
        }
    #if CPU_64
        if( !_hook_setclasslonga.Initialize(::SetClassLongPtrA, _OnHookSetClassLong) )
    #else
        if( !_hook_setclasslonga.Initialize(::SetClassLongA, _OnHookSetClassLong) )
    #endif
        {
            LOG("DllApp.LoadHooks.Init.SetClassLongA");
            return false;
        }
    #if CPU_64
        if( !_hook_setclasslongw.Initialize(::SetClassLongPtrW, _OnHookSetClassLong) )
    #else
        if( !_hook_setclasslongw.Initialize(::SetClassLongW, _OnHookSetClassLong) )
    #endif
        {
            LOG("DllApp.LoadHooks.Init.SetClassLongW");
            return false;
        }
        if( !_hook_showcursor.Initialize(::ShowCursor, _OnHookShowCursor) )
        {
            LOG("DllApp.LoadHooks.Init.ShowCursor");
            return false;
        }

        // enable hooks
        if( !_hook_setcursor.Enable() )
        {
            LOG("DllApp.LoadHooks.Enable.SetCursor");
            return false;
        }
        if( !_hook_setclasslonga.Enable() )
        {
            LOG("DllApp.LoadHooks.Enable.SetClassLongA");
            return false;
        }
        if( !_hook_setclasslongw.Enable() )
        {
            LOG("DllApp.LoadHooks.Enable.SetClassLongW");
            return false;
        }
        if( !_hook_showcursor.Enable() )
        {
            LOG("DllApp.LoadHooks.Enable.ShowCursor");
            return false;
        }

        return true;
    }

    //-------------------------------------------------------------------------
    void App::_UnloadHooks()
    {
        // disable hooks
        _hook_showcursor.Disable();
        _hook_setclasslongw.Disable();
        _hook_setclasslonga.Disable();
        _hook_setcursor.Disable();

        // shutdown hooks
        _hook_showcursor.Shutdown();
        _hook_setclasslongw.Shutdown();
        _hook_setclasslonga.Shutdown();
        _hook_setcursor.Shutdown();
    }

    //-------------------------------------------------------------------------
    void App::_RefreshCursor( HCURSOR hcursor, HWND hwnd )
    {
        DWORD process_id;

        // update via set class cursor
        _SetClassCursor( hwnd, hcursor, CLASSLONG_NINDEX_REFRESH );

        // also update via setcursor
        DWORD hwnd_thread_id = GetWindowThreadProcessId(hwnd, &process_id);
        DWORD current_thread_id = GetCurrentThreadId();
        if( hwnd_thread_id != 0 && current_thread_id != 0 )
        {
            AttachThreadInput( hwnd_thread_id, current_thread_id, TRUE );
            SetCursor( hcursor );
            AttachThreadInput(hwnd_thread_id, current_thread_id, FALSE);
        }

        // this should get cursor class to update immediately instead of waiting for mouse move
        //PostMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    }

    void App::_RefreshCurrentCursor()
    {
        HCURSOR hcursor;
        HWND    hwnd;

        // get current application cursor and refresh it
        if( _GetCurrentCursor( hcursor, hwnd ) && hcursor != NULL )
            _RefreshCursor( hcursor, hwnd );
    }

    //-------------------------------------------------------------------------
    CursorInfo* App::_UpdateCursorBinding( HCURSOR hcursor, Hash cursor_hash, const CursorInfo& updates, CursorUpdateFlags flags )
    {
        // get existing cursor binding info
        CursorInfo* info = _bindings.GetBinding(cursor_hash);

        // if does not exist
        if( info == nullptr )
        {
            // create
            info = _bindings.CreateBinding( cursor_hash );

            // fail if full
            if( info == nullptr )
            {
                LOG( "DllApp.UpdateCursorBinding.Full" );
                return nullptr;
            }

            // if creating through resize
            if( flags & ( CURSOR_UPDATE_INCREMENT_SIZE | CURSOR_UPDATE_DECREMENT_SIZE ) )
            {
                // set clone type
                info->type = CURSOR_TYPE_CLONE;

                // assign a default size nearest original cursor size
                info->size = CursorTools::SizeToId( CursorTools::HandleToSize( hcursor ) );
            }
            else
            {
                // require valid type
                if( updates.type == CURSOR_TYPE_INVALID )
                    return nullptr;

                // set default id, variation, size
                info->id = 0;
                info->variation = 0;
                info->size = CURSOR_SIZE_DEFAULT;
            }
        }

        // patch bindings with updates
        CursorTools::PatchProperties(*info, updates, flags);

        // validate cursor info
        if( !info->IsValid() )
        {
            LOG("DllApp.UpdateCursorBinding.InvalidBinding");
            _bindings.RemoveBinding(cursor_hash);
            return nullptr;
        }

        // save cursor bindings (can fail)
        CursorBindingsSerializer::Save( _bindings, _bindings_path );

        // log
    #if CPU_64
        LOG2( "DllApp.UpdateCursorBinding: HANDLE:%I64x HASH:%I64x BINDING:T=%d,I=%d,V=%d,S=%d", hcursor, cursor_hash, info->type, info->id, info->variation, info->size );
    #else
        LOG2( "DllApp.UpdateCursorBinding: HANDLE:%x HASH:%x BINDING:T=%d,I=%d,V=%d,S=%d", hcursor, cursor_hash, info->type, info->id, info->variation, info->size );
    #endif

        return info;
    }

    //-------------------------------------------------------------------------
    HCURSOR App::_LoadBoundCursor( HCURSOR hcursor, Hash cursor_hash, CursorInfo& info )
    {
        // by type
        switch( info.type )
        {
        case CURSOR_TYPE_BASIC:
            // load basic cursor
            return _LoadBoundBasicCursor( info );
        case CURSOR_TYPE_CLONE:
            // load clone cursor
            return _LoadBoundCloneCursor( hcursor, cursor_hash, info );
        default:
            return NULL;
        }
    }

    /*
        loads given binding otherwise iterates binding until load succeeds or fails when iteration exhausted
    */
    HCURSOR App::_LoadBoundBasicCursor( CursorInfo& info )
    {
        // for each cursor id with rotation
        for( CursorId idi = 0; idi < CURSOR_ID_COUNT; info.id = (info.id + 1) % CURSOR_ID_COUNT, ++idi )
        {
            // for each cursor variation with rotation
            for( CursorVariation variationi = 0; variationi < CURSOR_VARIATION_COUNT; info.variation = (info.variation + 1) % CURSOR_VARIATION_COUNT, ++variationi )
            {
                // load vault cursor
                HCURSOR hcursor = _cursor_vault.LoadBasic( info.id, info.variation, info.size );
                if( hcursor != NULL )
                    return hcursor;
            }

            info.variation = 0;
        }

        return NULL;
    }

    HCURSOR App::_LoadBoundCloneCursor( HCURSOR hcursor, Hash cursor_hash, CursorInfo& info )
    {
        // load clone from vault
        HCURSOR cloned_hcursor = _cursor_vault.LoadClone( hcursor, cursor_hash, info.size );
        if( cloned_hcursor != NULL )
            return cloned_hcursor;

        return NULL;

    }

    void App::_UnloadBoundCursor( Hash cursor_hash, const CursorInfo& info )
    {
        // by resource type
        switch( info.type )
        {
        case CURSOR_TYPE_BASIC:
            // unload basic cursor
            _cursor_vault.UnloadBasic(info.id, info.variation, info.size);
            break;
        case CURSOR_TYPE_CLONE:
            // unload clone cursor
            _cursor_vault.UnloadClone(cursor_hash, info.size);
            break;
        default:;
        }
    }

    HCURSOR App::_GetBoundCursor( Hash cursor_hash, const CursorInfo& info )
    {
        // by type
        switch( info.type )
        {
        case CURSOR_TYPE_BASIC:
            // get basic cursor
            return _cursor_vault.GetBasic(info.id, info.variation, info.size);
        case CURSOR_TYPE_CLONE:
            // unload clone cursor
            return _cursor_vault.GetClone(cursor_hash, info.size);
        default:
            return NULL;
        }
    }

    //-------------------------------------------------------------------------
    Bool App::_GetCurrentCursor( HCURSOR& hcursor, HWND& hwnd )
    {
        // get focus window
        hwnd = WindowTools::GetFocusWindow();
        if( hwnd == NULL )
        {
            LOG("DllApp.Refresh.GetFocusWindow");
            return false;
        }

        // get thread and process id of this window
        DWORD process_id = 0;
        DWORD hwnd_thread_id = GetWindowThreadProcessId( hwnd, &process_id );

        // require window belongs to this process
        if( process_id != GetCurrentProcessId() )
        {
            LOG( "DllApp.Refresh.WindowNotCurrentProcess" );
            return false;
        }

        // get hwnd class atom value
        DWORD class_atom = GetClassLongA( hwnd, GCW_ATOM );

        // return last cursor captured by hooks for this window (usually exists)
        WindowCursorEntry* entry = _window_cursor_map.Get( class_atom );
        if( entry != NULL )
        {
            hcursor = entry->last_cursor;
            return true;
        }

        // get current application class cursor
        hcursor = _GetClassCursor(hwnd);
        
        // if does not exist, try global windows cursor (error prone)
        if( hcursor == NULL )
        {
            // get thread id of this thread
            DWORD current_thread_id = GetCurrentThreadId();

            // attach to window thread. this is to make GetCursor work properly
            if( !AttachThreadInput( hwnd_thread_id, current_thread_id, TRUE ) )
            {
                LOG( "DllApp.Refresh.AttachThreadInput" );
                return false;
            }

            // get current windows cursor
            hcursor = GetCursor();

            // detach from window thread
            AttachThreadInput( hwnd_thread_id, current_thread_id, FALSE );
        }

        // fail if no cursor exists
        if( hcursor == NULL )
        {
            LOG("DllApp.Refresh.NullCursor");
            return false;
        }

        // cannot be yolomouse cursor. this shouldnt happen unless app calls GetClassLong
        // after YM used SetClassLong to change cursor and later uses that cursor as its own.
        if( _cursor_vault.HasCursor(hcursor) )
        {
            LOG("DllApp.Refresh.IsYoloCursor");
            return false;
        }

        return true;
    }

    HCURSOR App::_GetClassCursor( HWND hwnd )
    {
        // save current class long value
    #if CPU_64
        return (HCURSOR)GetClassLongPtrA( hwnd, GCLP_HCURSOR );
    #else
        return (HCURSOR)GetClassLongA( hwnd, GCL_HCURSOR );
    #endif
    }

    void App::_SetClassCursor( HWND hwnd, HCURSOR hcursor, ClassLongNIndex nindex_override )
    {
        // set class long value
    #if CPU_64
        SetClassLongPtrA( hwnd, nindex_override == CLASSLONG_NINDEX_DEFAULT ? GCLP_HCURSOR : nindex_override, (LONG_PTR)hcursor );
    #else
        SetClassLongA( hwnd, nindex_override == CLASSLONG_NINDEX_DEFAULT ? GCL_HCURSOR : nindex_override, (LONG)hcursor );
    #endif
    }

    void App::_RestoreClassCursors()
    {
        // if any changes made
        for( WindowCursorMap::KeyValue& kv : _window_cursor_map )
        {
            const WindowCursorEntry& entry = kv.value;

            // if set, restore original class long
            if( entry.original_class_set )
                _SetClassCursor( entry.hwnd, entry.original_class_cursor, CLASSLONG_NINDEX_DEFAULT );

            // log
        #if CPU_64
            LOG2( "DllApp.RestoreClassCursors: %I64x %I64x", kv.key, kv.value.original_class_cursor );
        #else
            LOG2( "DllApp.RestoreClassCursors: %x %x", kv.key, kv.value.original_class_cursor );
        #endif
        }
    }

    // cursor event handlers
    //-------------------------------------------------------------------------
    Bool App::_NotifyCursorChanging( const CursorInfo& info )
    {
        OnCursorChangingIpcMessage message;

        // build message
        message.request = IPC_REQUEST_ON_CURSOR_CHANGING;
        message.info = info;

        // send message. use small timeout
        return _ipc.Send( message, sizeof( message ), 2 );
    }

    Bool App::_NotifyCursorShowing( Bool showing )
    {
        OnCursorShowingIpcMessage message;

        // build message
        message.request = IPC_REQUEST_ON_CURSOR_SHOWING;
        message.showing = showing;

        // send message. use small timeout
        return _ipc.Send( message, sizeof( message ), 2 );
    }

    // event handlers
    //-------------------------------------------------------------------------
    void App::_OnRequestLoad( const LoadIpcMessage& message )
    {
        // load self
        if( !IsLoaded() )
            Load(message.host_path, message.log_path, message.bindings_path);
    }

    void App::_OnRequestExit()
    {
        // unload self
        if( IsLoaded() )
            Unload();

        // trigger exit of this dll
        CreateThread(NULL, 0, _DllUnloader, _hdll, 0, NULL);
    }

    void App::_OnRequestSetCursor( const SetCursorIpcMessage& message )
    {
        HCURSOR hcursor;
        HWND    hwnd;

        // get current application cursor
        if( _GetCurrentCursor( hcursor, hwnd ) && hcursor != NULL )
        {
            // get hash of cursor
            Hash cursor_hash = _cache.GetHash(hcursor);
            if( cursor_hash != 0 )
            {
                // update cursor binding
                CursorInfo* binding = _UpdateCursorBinding( hcursor, cursor_hash, message.properties, message.flags );

                // if binding updated
                if( binding != nullptr )
                {
                    // refresh cursor
                    _RefreshCursor(hcursor, hwnd);
                }
            }
        }
    }

    void App::_OnRequestSetDefaultCursor()
    {
        HCURSOR hcursor;
        HWND    hwnd;

        // get current application cursor
        if( _GetCurrentCursor( hcursor, hwnd ) && hcursor != NULL )
        {
            // get hash of cursor
            Hash cursor_hash = _cache.GetHash(hcursor);
            if( cursor_hash != 0 )
            {
                // get associated binding
                CursorInfo* binding = _bindings.GetBinding( cursor_hash );

                // if binding exists
                if( binding != nullptr )
                {
                    // only supported default types
                    switch( binding->type )
                    {
                    case CURSOR_TYPE_BASIC:
                    case CURSOR_TYPE_OVERLAY:
                        {
                            // get current default binding
                            CursorInfo& default_binding = _bindings.GetDefaultBinding();

                            // if default binding active, unload previous default cursor
                            if( default_binding.IsValid() )
                                _UnloadBoundCursor(cursor_hash, default_binding);

                            // make current binding the new default
                            default_binding = *binding;

                            // save cursor bindings (can fail)
                            CursorBindingsSerializer::Save( _bindings, _bindings_path );
                        }
                        break;
                    default:;
                    }
                }
            }
        }
    }
    
    void App::_OnRequestResetCursor()
    {
        HCURSOR hcursor;
        HWND    hwnd;

        // get current application cursor
        if( _GetCurrentCursor( hcursor, hwnd ) && hcursor != NULL )
        {
            // get hash of cursor
            Hash cursor_hash = _cache.GetHash(hcursor);
            if( cursor_hash != 0 )
            {
                // get associated binding
                CursorInfo* binding = _bindings.GetBinding( cursor_hash );

                // if exists
                if( binding != nullptr )
                {
                    // unload associated cursor
                    _UnloadBoundCursor(cursor_hash, *binding);

                    // remove binding
                    _bindings.RemoveBinding(cursor_hash);
                }
                // else reset default binding
                else
                    _bindings.GetDefaultBinding() = CursorInfo();

                // refresh cursor
                _RefreshCursor(hcursor, hwnd);

                // save cursor bindings (can fail)
                CursorBindingsSerializer::Save( _bindings, _bindings_path );
            }
        }
    }

    // cursor event handlers
    //-------------------------------------------------------------------------
    void App::_OnInjectedCursorChanging( HCURSOR& out_cursor, HCURSOR in_cursor )
    {
        // get default cursor binding
        CursorInfo* info = &_bindings.GetDefaultBinding();

        // get hash of app cursor
        Hash cursor_hash = _cache.GetHash(in_cursor);

        // if valid hash
        if( cursor_hash != 0 )
        {
            // check for assigned binding
            CursorInfo* assigned_info = _bindings.GetBinding( cursor_hash );

            // if found, use assigned binding instead
            if( assigned_info != nullptr )
                info = assigned_info;
        }

        // handle by binding type
        switch( info->type )
        {
        case CURSOR_TYPE_OVERLAY:
            // if overlay, hide app cursor by using hidden cursor (do not use 0, SetClassLong doesnt like it)
            // and let loader handle overlay cursor display
            out_cursor = _cursor_vault.GetHidden();
            break;

        case CURSOR_TYPE_BASIC:
        case CURSOR_TYPE_CLONE:
            {
                // get matching vault cursor
                HCURSOR vault_cursor = _GetBoundCursor( cursor_hash, *info );

                // if exists or loaded successfully, make new app cursor
                if( vault_cursor != NULL || (vault_cursor = _LoadBoundCursor( in_cursor, cursor_hash, *info )) != NULL )
                    out_cursor = vault_cursor;
            }
            break;
        }

        // notify loader cursor changing
        _NotifyCursorChanging( *info );

        // log
    #if CPU_64
        LOG2( "DllApp.OnCursorChanging: UPDATED:%u HASH:%I64x BINDING:T=%d,I=%d,V=%d,S=%d", out_cursor != in_cursor, cursor_hash, info->type, info->id, info->variation, info->size );
    #else
        LOG2( "DllApp.OnCursorChanging: UPDATED:%u HASH:%x BINDING:T=%d,I=%d,V=%d,S=%d", out_cursor != in_cursor, cursor_hash, info->type, info->id, info->variation, info->size );
    #endif
    }

    void App::_OnInjectedCursorShowing( Bool showing )
    {
        // log
        LOG2( "DllApp.OnCursorShowing: %u", showing );

        // notify loader cursor showing status
        _NotifyCursorShowing( showing );
    }

    // hooks
    //-------------------------------------------------------------------------
    VOID HOOK_CALL App::_OnHookSetCursor( volatile Native* arguments )
    {
        // in arguments
        Native a1 = arguments[1];

        // log
    #if CPU_64
        LOG2( "DllApp.OnHookSetCursor: %I64x", a1 );
    #else
        LOG2( "DllApp.OnHookSetCursor: %x", a1 );
    #endif

        // get app
        App& app = App::Instance();

        // update cursor through class long method. hwnd parameter is reused to get hcursor value.
        app._SetClassCursor( reinterpret_cast<HWND>(&a1), reinterpret_cast<HCURSOR>(a1), CLASSLONG_NINDEX_SETCURSOR );

        // out arguments
        arguments[1] = a1;
    }

    VOID HOOK_CALL App::_OnHookSetClassLong( volatile Native* arguments )
    {
        // in arguments
        Native a1 = arguments[1];
        Native a2 = arguments[2];
        Native a3 = arguments[3];

        // get nIndex argument
        int nIndex = static_cast<int>(a2);

        // log
    #if CPU_64
        LOG2( "DllApp.OnHookSetClassLong: %I64x %d %I64x", a1, a2, a3 );
    #else
        LOG2( "DllApp.OnHookSetClassLong: %x %d %x", a1, a2, a3 );
    #endif

        // handle by nIndex
        switch( nIndex )
        {
        case CLASSLONG_NINDEX_REFRESH:
        case CLASSLONG_NINDEX_SETCURSOR:
    #if CPU_64
        case GCLP_HCURSOR:
    #else
        case GCL_HCURSOR:
    #endif
            {
                WindowCursorEntry* window_cursor_entry = nullptr;
                HCURSOR*           phcursor_setcursor = nullptr;
                App&               app = App::Instance();

                // get hwnd argument
                HWND hwnd = reinterpret_cast<HWND>(a1);

                // get cursor argument
                HCURSOR hcursor = reinterpret_cast<HCURSOR>(a3);

                // if refresh or setcursor request
                if( nIndex == CLASSLONG_NINDEX_REFRESH || nIndex == CLASSLONG_NINDEX_SETCURSOR )
                {
                    // set correct nIndex arguments
                #if CPU_64
                    a2 = GCLP_HCURSOR;
                #else
                    a2 = GCL_HCURSOR;
                #endif

                    // if we're being called from setcursor hook, arg 1 is a pointer to HCURSOR so actual HWND
                    // doesnt exist. instead get active window as current HWND. This may not always be valid but
                    // its best effort since SetCursor doesn't provide HWND parameter.
                    if( nIndex == CLASSLONG_NINDEX_SETCURSOR )
                    {
                        phcursor_setcursor = reinterpret_cast<HCURSOR*>(a1);
                        hwnd = GetActiveWindow();
                        a1 = reinterpret_cast<Native>(hwnd);
                    }

                    // if hwnd specified 
                    if( hwnd != NULL )
                    {
                        // access window cursor entry for this window class
                        window_cursor_entry = &app._window_cursor_map.Set( GetClassLongA( hwnd, GCW_ATOM ) );

                        // if original class cursor not set, save now using current application class cursor
                        if( !window_cursor_entry->original_class_set )
                        {
                            window_cursor_entry->original_class_set = true;
                            window_cursor_entry->original_class_cursor = app._GetClassCursor( hwnd );
                        }
                    }
                }
                // else if regular call with valid hwnd
                else if( hwnd != NULL )
                {
                    // access window cursor entry for this window class
                    window_cursor_entry = &app._window_cursor_map.Set( GetClassLongA( hwnd, GCW_ATOM ) );

                    // update original class cursor for this window class
                    window_cursor_entry->original_class_set = true;
                    window_cursor_entry->original_class_cursor = hcursor;
                }

                // update last seen hwnd and cursor
                if( window_cursor_entry != nullptr )
                {
                    window_cursor_entry->hwnd = hwnd;
                    window_cursor_entry->last_cursor = hcursor;
                }

                // handle cursor change
                app._OnInjectedCursorChanging( hcursor, hcursor );
                a3 = reinterpret_cast<Native>(hcursor);

                // if setcursor request, reply with updated cursor using HWND argument
                if( nIndex == CLASSLONG_NINDEX_SETCURSOR )
                    *phcursor_setcursor = hcursor;
            }
            break;

        default:;
        }

        // out arguments
        arguments[1] = a1;
        arguments[2] = a2;
        arguments[3] = a3;
    }

    VOID HOOK_CALL App::_OnHookShowCursor( volatile Native* arguments )
    {
        // in arguments
        Native a1 = arguments[1];

        // log
        LOG2( "DllApp.OnHookShowCursor: %u", a1 );

        // get app
        App& app = App::Instance();

        // handle cursor showing state
        app._OnInjectedCursorShowing( a1 == TRUE );
    }
}

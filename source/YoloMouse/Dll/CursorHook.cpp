#include <YoloMouse/Dll/CursorHook.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

namespace YoloMouse
{
    // private
    //-------------------------------------------------------------------------
    Bool                CursorHook::_active             (false);
    CursorBindings      CursorHook::_bindings;
    HCURSOR             CursorHook::_last_cursor        (NULL);
    HCURSOR             CursorHook::_replace_cursor     (NULL);
    CursorHook::Method  CursorHook::_method             (CursorHook::METHOD_SETCURSOR);

    WCHAR               CursorHook::_target_id          [STRING_PATH_SIZE];
    Bool                CursorHook::_assign_ready       (false);
    Index               CursorHook::_assign_index       (INVALID_INDEX);
    Bool                CursorHook::_refresh_ready      (false);

    SharedState&        CursorHook::_state =            SharedState::Instance();
    Hook                CursorHook::_hook_setcursor     (SetCursor, CursorHook::_OnSetCursor, Hook::BEFORE);
#ifdef _WIN64
    Hook                CursorHook::_hook_setclasslonga (SetClassLongPtrA, CursorHook::_OnSetClassLong, Hook::BEFORE);
    Hook                CursorHook::_hook_setclasslongw (SetClassLongPtrW, CursorHook::_OnSetClassLong, Hook::BEFORE);
#else
    Hook                CursorHook::_hook_setclasslonga (SetClassLongA, CursorHook::_OnSetClassLong, Hook::BEFORE);
    Hook                CursorHook::_hook_setclasslongw (SetClassLongW, CursorHook::_OnSetClassLong, Hook::BEFORE);
#endif

     // public
    //-------------------------------------------------------------------------
    Bool CursorHook::Load()
    {
        DWORD process_id = GetCurrentProcessId();

        // if not already active
        if( _active )
            return true;

        // build id string
        if( !SharedTools::BuildTargetId(_target_id, COUNT(_target_id), process_id) )
            return false;

        // load state
        if( !_state.Open(false) )
            return false;

        // load hooks
        if( !_LoadHooks() )
            return false;

        // load cursor map from file
        _bindings.Load(_target_id);

        // activate
        _active = true;

        // refresh cursor
        Refresh();

        return true;
    }

    void CursorHook::Unload()
    {
        // disable hook
        if( !_active )
            return;

        // unload state
        _state.Close();

        // unload  hooks
        _UnloadHooks();

        // deactivate
        _assign_index = INVALID_INDEX;
        _assign_ready = false;
        _active = false;
    }

    //-------------------------------------------------------------------------
    Bool CursorHook::Assign( Index cursor_index )
    {
        // require active
        if( !_active )
            return false;

        // mark for update
        _assign_index = cursor_index;
        _assign_ready = true;

        // refresh cursor
        Refresh();

        return true;
    }

    //-------------------------------------------------------------------------
    Bool CursorHook::Refresh()
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
                if( _state.FindCursor(refresh_cursor) != INVALID_INDEX )
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
            #ifdef _WIN64
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
    Bool CursorHook::_LoadHooks()
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

    void CursorHook::_UnloadHooks()
    {
        // disable hooks
        _hook_setclasslongw.Disable();
        _hook_setclasslonga.Disable();
        _hook_setcursor.Disable();
    }

    //-------------------------------------------------------------------------
    Bool CursorHook::_OnCursorAssign( HCURSOR hcursor, Index cursor_index )
    {
        // cannot be yolomouse cursor
        if( _state.FindCursor(hcursor) != INVALID_INDEX )
            return false;

        // calculate hash of cursor
        Hash cursor_hash = SharedTools::CalculateCursorHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
            return false;

        // find cursor mapping
        Index mapping_index = _bindings.Find(cursor_hash);

        // if removing
        if( cursor_index == INVALID_INDEX )
        {
            // require mapping exist
            if( mapping_index == INVALID_INDEX )
                return false;

            // remove mapping
            _bindings.Remove(mapping_index);
        }
        // else if adding
        else
        {
            // require cursor exists
            if( _state.GetCursor(cursor_index) == NULL )
                return false;

            // remove mapping if already exists
            if( mapping_index != INVALID_INDEX )
                _bindings.Remove(mapping_index);
     
            // add mapping
            if( _bindings.Add(cursor_hash, cursor_index) == INVALID_INDEX )
                return false;
        }

        // save cursor map to file
        _bindings.Save(_target_id);

        return true;
    }

    //-------------------------------------------------------------------------
    Bool CursorHook::_OnCursorChanging( HCURSOR hcursor )
    {
        // cannot be yolomouse cursor
        if( _state.FindCursor(hcursor) != INVALID_INDEX )
            return false;

        // calculate hash of cursor
        Hash cursor_hash = SharedTools::CalculateCursorHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
            return false;

        // reset replacement cursor
        _replace_cursor = NULL;

        // find cursor mapping
        Index mapping_index = _bindings.Find(cursor_hash);

        // if found
        if( mapping_index != INVALID_INDEX )
        {
            // update replacement cursor
            _replace_cursor = _state.GetCursor(_bindings.Get(mapping_index)._index);
        }

        return true;
    }

    //-------------------------------------------------------------------------
    Bool CursorHook::_OnCursorEvent( HCURSOR& new_cursor, HCURSOR old_cursor )
    {
        // ignore null cursor
        if( old_cursor == NULL )
            return false;

        // if cursor changing set refresh state
        if( old_cursor != _last_cursor )
            _refresh_ready = true;

        // set new last cursor
        _last_cursor = old_cursor;

        // if assigning new cursor
        if( _assign_ready )
        {
            // clear assign state
            _assign_ready = false;

            // handle cursor assign
            if(!_OnCursorAssign(old_cursor, _assign_index))
                return false;

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
                return false;
        }

        // fail if no replacement
        if( _replace_cursor == NULL )
            return false;
        
        // return replacement cursor
        new_cursor = _replace_cursor;
        return true;
    }
    //-------------------------------------------------------------------------
    VOID HOOK_CALL CursorHook::_OnSetCursor( Native* arguments )
    {
        // update cursor
        _OnCursorEvent((HCURSOR&)arguments[1], (HCURSOR)arguments[1]);
    }

    VOID HOOK_CALL CursorHook::_OnSetClassLong( Native* arguments )
    {
        // if changing cursor
    #ifdef _WIN64
        if((int)arguments[2] == GCLP_HCURSOR)
    #else
        if((int)arguments[2] == GCL_HCURSOR)
    #endif
        {
            // change method
            _method = METHOD_SETCLASSLONG;

            // update cursor
            _OnCursorEvent((HCURSOR&)arguments[3], (HCURSOR)arguments[3]);
        }
    }
}

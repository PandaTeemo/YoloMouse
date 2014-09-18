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
    Hook                CursorHook::_hook_setclasslong  (SetClassLongPtrA, CursorHook::_OnSetClassLong, Hook::BEFORE);
#else
    Hook                CursorHook::_hook_setclasslong  (SetClassLongA, CursorHook::_OnSetClassLong, Hook::BEFORE);
#endif

     // public
    //-------------------------------------------------------------------------
    void CursorHook::Load( HWND hwnd )
    {
        // if not already active
        if( _active )
            return;

        // load state
        if( !_state.Open(false) )
            return;

        // enable hooks
        if( !_hook_setcursor.Init() )
            return;

        // activate
        _active = true;

        // build id string
        if( !SharedTools::BuildTargetId(_target_id, COUNT(_target_id), hwnd) )
            return;

        // enable hooks
        if(!_hook_setcursor.Enable() )
            return;

        // load cursor map from file
        _bindings.Load(_target_id);

        // enable optional hooks
        if(_hook_setclasslong.Init())
            _hook_setclasslong.Enable();

        // refresh cursor
        Refresh(hwnd);
    }

    void CursorHook::Unload()
    {
        // disable hook
        if( !_active )
            return;

        // unload state
        _state.Close();

        // disable hooks
        _hook_setclasslong.Disable();
        _hook_setcursor.Disable();

        // deactivate
        _assign_index = INVALID_INDEX;
        _assign_ready = false;
        _active = false;
    }

    //-------------------------------------------------------------------------
    void CursorHook::Assign( HWND hwnd, Index cursor_index )
    {
        xassert(_active);

        // mark for update
        _assign_index = cursor_index;
        _assign_ready = true;

        // refresh cursor
        Refresh(hwnd);
    }

    //-------------------------------------------------------------------------
    void CursorHook::Refresh( HWND hwnd )
    {
        // get last cursor
        HCURSOR refresh_cursor = _last_cursor;

        // get current and window threads
        DWORD hwnd_thread_id = GetWindowThreadProcessId(hwnd, 0);
        DWORD current_thread_id = GetCurrentThreadId();

        // attach to window thread. this is to make GetCursor and SetCursor work properly
        AttachThreadInput(hwnd_thread_id, current_thread_id, TRUE);

        // if does not exist
        if( refresh_cursor == NULL )
        {
            // get active windows cursor
            refresh_cursor = GetCursor();

            // cannot be yolomouse cursor
            if( _state.FindCursor(refresh_cursor) != INVALID_INDEX )
            {
                AttachThreadInput(hwnd_thread_id, current_thread_id, FALSE);
                return;
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
            SendMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
        }

        // detach from window thread
        AttachThreadInput(hwnd_thread_id, current_thread_id, FALSE);
    }

    // private
    //-------------------------------------------------------------------------
    HCURSOR CursorHook::_AdaptCursor( HCURSOR from )
    {
        // adapt null to special empty cursor
        if(from == NULL)
            return CURSOR_SPECIAL_EMPTY;
        // else use given
        else
            return from;
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
        // adapt cursor
        old_cursor = _AdaptCursor(old_cursor);

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
        xassert(_active);

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

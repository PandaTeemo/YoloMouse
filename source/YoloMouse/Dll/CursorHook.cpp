#include <YoloMouse/Dll/CursorHook.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

namespace YoloMouse
{
    // private
    //-------------------------------------------------------------------------
    Bool            CursorHook::_active         (false);
    CursorBindings  CursorHook::_bindings;
    HCURSOR         CursorHook::_last_cursor    (NULL);
    HCURSOR         CursorHook::_replace_cursor (NULL);

    WCHAR           CursorHook::_target_id      [STRING_PATH_SIZE];
    Bool            CursorHook::_assign_ready   (false);
    Index           CursorHook::_assign_index   (INVALID_INDEX);
    Bool            CursorHook::_refresh_ready  (false);

    SharedState&    CursorHook::_state =        SharedState::Instance();
    Hook            CursorHook::_hook_setcursor (SetCursor, CursorHook::_OnSetCursor, Hook::BEFORE);
    //Hook          CursorHook::_hook_getcursor (GetCursor, CursorHook::_OnGetCursor, Hook::AFTER);

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
        if( !_hook_setcursor.Init() )//|| !_hook_getcursor.Init())
            return;

        // activate
        _active = true;

        // build id string
        if( !SharedTools::BuildTargetId(_target_id, COUNT(_target_id), hwnd) )
            return;

        // enable hooks
        if(!_hook_setcursor.Enable() )//|| !_hook_getcursor.Enable())
            return;

        // load cursor map from file
        _bindings.Load(_target_id);

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

        // disable hook
        _hook_setcursor.Disable();
        //_hook_getcursor.Disable();

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

        // if does not exist
        if( refresh_cursor == NULL )
        {
            // get active windows cursor
            refresh_cursor = GetCursor();

            // cannot be yolomouse cursor
            if( _state.FindCursor(refresh_cursor) != INVALID_INDEX )
                return;
        }

        // set refresh state
        _refresh_ready = true;
            
        // set current cursor to force update
        SetCursor(refresh_cursor);

        // then trigger application to call SetCursor with its own cursor
        PostMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
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
    Bool CursorHook::_OnSetCursorAssign( HCURSOR hcursor, Index cursor_index )
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
    Bool CursorHook::_OnSetCursorChange( HCURSOR hcursor )
    {
        // cannot be yolomouse cursor
        if( _state.FindCursor(hcursor) != INVALID_INDEX )
            return false;

        // calculate hash of cursor
        Hash cursor_hash = SharedTools::CalculateCursorHash(hcursor);

        // fail if invalid
        if( cursor_hash == 0 )
            return false;

        // reset replace cursor
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
    VOID HOOK_CALL CursorHook::_OnSetCursor( x86::Registers registers )
    {
        xassert(_active);
        HCURSOR hcursor = *(HCURSOR*)(registers.esp + 4);

        // adapt cursor
        hcursor = _AdaptCursor(hcursor);

        // if cursor changing set refresh state
        if( hcursor != _last_cursor )
            _refresh_ready = true;

        // set new last cursor
        _last_cursor = hcursor;

        // if assigning new cursor
        if( _assign_ready )
        {
            // clear assign state
            _assign_ready = false;

            // handle cursor assign
            if(!_OnSetCursorAssign(hcursor, _assign_index))
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
            if(!_OnSetCursorChange(hcursor))
                return;
        }

        // replace hcursor parameter before call to SetCursor
        if( _replace_cursor )
            *(HCURSOR*)(registers.esp + 4) = _replace_cursor;
    }
    /*
    VOID HOOK_CALL CursorHook::_OnGetCursor( volatile x86::Registers registers )
    {
        // replace the return value of GetCursor with last called parameter to SetCursor
        if( _last_cursor != NULL )
            registers.eax = (Byte4)_last_cursor;
    }
    */
}

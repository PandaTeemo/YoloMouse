#pragma once
#include <Core/Container/Array.hpp>
#include <Core/Windows/ShellUi.hpp>

namespace Core
{
    // local: types
    //------------------------------------------------------------------------
    typedef FixedArray<ShellUi::IListener*, 4>   ListenerCollection;
    typedef ListenerCollection::Iterator    ListenerIterator;


    // local: enum
    //------------------------------------------------------------------------
    enum
    {
        ID_TRAYICON =   1,
        ID_EXIT =       1000,
        ID_SEPARATOR =  1001,
        ID_USER =       1010,
    };

    enum
    {
        WMAPP_TRAYICON  = WM_APP,
    };

    
    // local: data
    //------------------------------------------------------------------------
    static ListenerCollection   _listeners;
    static const WCHAR*         _name = L"";
    static HWND                 _hwnd = NULL;
    static HMENU                _menu = NULL;
    static HINSTANCE            _hinstance = NULL;
    static Id                   _icon_id = INVALID_ID;


    // local: ui
    //------------------------------------------------------------------------
    static void _ShellIconCreate()
    {
        NOTIFYICONDATA nid = {0};

        // systray icon
        nid.hIcon = (_icon_id == INVALID_ID) ?
            LoadIcon( NULL, IDI_HAND ) :
            (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(_icon_id), IMAGE_ICON, 16, 16, 0);

        // other parameters
        nid.cbSize              = sizeof( nid );
        nid.hWnd                = _hwnd;
        nid.uID                 = ID_TRAYICON;
        nid.uFlags              = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage    = WMAPP_TRAYICON;

        // tooltip
        wcscpy_s( nid.szTip, COUNT(nid.szTip), _name );

        // create notify icon
        Shell_NotifyIcon( NIM_ADD, &nid );
    }

    static void _ShellIconDestroy()
    {
        NOTIFYICONDATA nid = {0};

        // delete systray icon
        nid.cbSize  = sizeof( nid );
        nid.hWnd    = _hwnd;
        nid.uID     = ID_TRAYICON;

        Shell_NotifyIcon( NIM_DELETE, &nid );
    }

    // local: wndproc
    //------------------------------------------------------------------------
    static void _OnMenuDestroy( HWND hwnd )
    {
        // destroy menu
        DestroyMenu( _menu );

        // delete shell icon
        _ShellIconDestroy();
    }

    //------------------------------------------------------------------------
    static void _OnMenuOpen( HWND hwnd )
    {
        HMENU   menu = NULL;
        WORD    command;
        POINT   curpos;

        // set focus
        SetForegroundWindow( hwnd );

        // get position for popup menu
        GetCursorPos(&curpos);

        // show popup menu
        command = TrackPopupMenu(
            _menu,
            TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY | TPM_BOTTOMALIGN,
            curpos.x, curpos.y,
            0,
            hwnd,
            NULL );

        // forward return command
        SendMessage( hwnd, WM_COMMAND, command, 0 );
    }

    static Bool _OnMenuUserOption( WORD wid )
    {
        MENUITEMINFO mi;

        // init
        mi.cbSize = sizeof(mi);
        mi.fMask = MIIM_STATE;

        // get option state
        if(GetMenuItemInfo(_menu, wid, FALSE, &mi))
        {
            // notify
            for( ListenerIterator l = _listeners.Begin(); l != _listeners.End(); ++l )
            {
                // get current state
                Bool enabled = (mi.fState & MFS_CHECKED) != 0;

                // if callback handled
                if((*l)->OnMenuOption(wid - ID_USER, enabled))
                    return true;
            }
        }

        return false;
    }

    //------------------------------------------------------------------------
    static Bool _OnCommandProc( HWND hwnd, WORD wid, HWND htcl )
    {
        // if exit
        if( wid == ID_EXIT )
        {
            PostMessage( hwnd, WM_CLOSE, 0, 0 );
            return true;
        }
        // else if user option
        else if( wid >= ID_USER )
            return _OnMenuUserOption( wid );

        return false;
    }

    //-------------------------------------------------------------------------
    static LRESULT CALLBACK _WndProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
    {
        switch(message)
        {
        case WMAPP_TRAYICON:
            SetForegroundWindow( hwnd );
            switch( lparam )
            {
            case WM_RBUTTONDOWN:
            case WM_LBUTTONDOWN:
                _OnMenuOpen(hwnd);
                return 0;
            }
            return 0;

        case WM_COMMAND:
            if(_OnCommandProc( hwnd, LOWORD(wparam), (HWND)lparam ))
                return 0;
            break;

        case WM_CLOSE:
            _OnMenuDestroy(hwnd);
            PostQuitMessage(0);
            return 0;

        case WM_DESTROY:
            return 0;

        default:
            // notify
            for( ListenerIterator l = _listeners.Begin(); l != _listeners.End(); ++l )
                if((*l)->OnMessage(hwnd, message, wparam, lparam))
                    return 0;
        }

        return DefWindowProc( hwnd, message, wparam, lparam );
    }

    // public
    //--------------------------------------------------------------------------
    ShellUi::ShellUi()
    {
    }

    void ShellUi::AddMenu()
    {
        xassert(_hwnd);

        // create shell icon
        _ShellIconCreate();

        // create popup menu
        _menu = CreatePopupMenu();

        // add exit option
        InsertMenu( _menu, 0, MF_BYPOSITION | MF_STRING, ID_EXIT, L"E&xit" );
        SetMenuDefaultItem( _menu, ID_EXIT, FALSE );
    }

    void ShellUi::AddMenuBreak()
    {
        // add separator
        InsertMenu( _menu, 0, MF_BYPOSITION | MF_SEPARATOR, ID_SEPARATOR, NULL );
    }

    void ShellUi::AddMenuOption( Id id, const WCHAR* name, Bool enabled )
    {
        // add exit option
        InsertMenu( _menu, 0, MF_BYPOSITION | MF_STRING | (enabled ? MF_CHECKED : MF_UNCHECKED), id + ID_USER, name );
    }

    //--------------------------------------------------------------------------
    void ShellUi::HideMenu()
    {
        xassert(_menu);

        // destroy shell icon
        _ShellIconDestroy();
    }

    //--------------------------------------------------------------------------
    HWND ShellUi::GetHwnd()
    {
        return _hwnd;
    }

    //--------------------------------------------------------------------------
    void ShellUi::SetIcon( Id icon_id )
    {
        _icon_id = icon_id;
    }

    void ShellUi::SetName( const WCHAR* name )
    {
        _name = name;
    }

    void ShellUi::SetMenuOption( Id id, Bool enabled )
    {
        MENUITEMINFO mi;

        // init
        mi.cbSize = sizeof(mi);
        mi.fMask = MIIM_STATE;
        mi.fState = enabled ? MFS_CHECKED : MFS_UNCHECKED;

        // update option state
        SetMenuItemInfo(_menu, id + ID_USER, FALSE, &mi);
    }

    //--------------------------------------------------------------------------
    void ShellUi::AddListener( IListener& listener )
    {
        _listeners.Add(&listener);
    }

    void ShellUi::RemoveListener( IListener& listener )
    {
        //TODO
    }

    //--------------------------------------------------------------------------
    Bool ShellUi::Start()
    {
        WNDCLASSEX wclass;

        // get instance
        _hinstance = (HINSTANCE)GetModuleHandle(NULL);

        // register window class
        wclass.cbSize = sizeof(wclass);
        wclass.style = 0;
        wclass.lpfnWndProc = _WndProc;
        wclass.cbClsExtra = 0;
        wclass.cbWndExtra = 0;
        wclass.hInstance = _hinstance;
        wclass.hCursor = LoadCursor( NULL, IDC_ARROW );
        wclass.hbrBackground = NULL;
        wclass.lpszMenuName = NULL;
        wclass.lpszClassName = _name;

        // application icon
        wclass.hIcon = wclass.hIconSm = (_icon_id == INVALID_ID) ?
            LoadIcon( NULL, IDI_HAND ) :
            (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(_icon_id), IMAGE_ICON, 16, 16, 0);

        // register class
        RegisterClassEx(&wclass);

        // create window
        _hwnd = CreateWindow(
            wclass.lpszClassName,
            L"",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            _hinstance,
            NULL );

        return (_hwnd != NULL);
    }

    void ShellUi::Stop()
    {
        // destroy window
        DestroyWindow(_hwnd);

        // unregister class
        UnregisterClass(_name, _hinstance);
    }

    //--------------------------------------------------------------------------
    void ShellUi::Run()
    {
        MSG msg;

        // run loop
        while(GetMessage(&msg, NULL, 0, 0 ) > 0)
        { 
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }

    void ShellUi::Exit()
    {
        PostMessage( _hwnd, WM_CLOSE, 0, 0 );
    }
}

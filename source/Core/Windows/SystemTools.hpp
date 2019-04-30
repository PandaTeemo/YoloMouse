#pragma once
#include <Core/Constants.hpp>

namespace Core
{
    /**/
    class SystemTools
    {
    public:
        /**/
        static Bitness GetProcessBitness( HANDLE process );

        /**/
        static HWND GetFocusWindow();

        /**/
        static Bool EnableAutoStart( const WCHAR* name, const WCHAR* path, Bool enable );

        /**/
        static Bool GetProcessDirectory( WCHAR* path, ULong limit );

        /**/
        static HANDLE OpenDebugPrivileges();
        static void CloseDebugPrivileges( HANDLE handle );

        /**/
        static Bool TestGameWindow( HWND hwnd );

        /**/
        static Bool IsValidGameWindowSize( const SIZE& size );

        /**/
        static Bool TestFramesUpdating( const RECT& region, HWND hwnd, ULong test_count=1 );

        /**/
        static SIZE RectToSize( const RECT& rect );

    private:
        /**/
        static Bool _TestFramesUpdating( const RECT& region, HDC hdc );
    };
}

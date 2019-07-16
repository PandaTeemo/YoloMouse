#pragma once
#include <Core/Container/String.hpp>
#include <Windows.h>

namespace Core
{
    /**/
    class WindowTools
    {
    public:
        /**/
        static void RunWindowLoop();

        /**/
        static HWND GetFocusWindow();

        /**/
        static Bool TestGameWindow( HWND hwnd );

        /**/
        static Bool IsValidGameWindowSize( const SIZE& size );

        /**/
        static Bool TestFramesUpdating( const RECT& region, HWND hwnd, ULong test_count=1 );

        /**/
        static SIZE RectToSize( const RECT& rect );

        /**/
        static void MessagePopup( const Char* title, Bool error, const Char* format, ... );

    private:
        /**/
        static Bool _TestFramesUpdating( const RECT& region, HDC hdc );
    };
}

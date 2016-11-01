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
        static OsVersion GetOsVersion();

        /**/
        static Bool EnableAutoStart( const WCHAR* name, const WCHAR* path, Bool enable );

        /**/
        static Bool GetProcessDirectory( WCHAR* path, ULong limit );

        /**/
        static HANDLE OpenDebugPrivileges();
        static void CloseDebugPrivileges( HANDLE handle );
    };
}

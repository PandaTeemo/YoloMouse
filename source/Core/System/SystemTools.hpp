#pragma once
#include <Core/Constants.hpp>
#include <Core/Types.hpp>

namespace Core
{
    /**/
    class SystemTools
    {
    public:
        /**/
        static Bitness  GetProcessBitness( HANDLE process );
        static Bool     GetProcessDirectory( PathString& path );
        static UHuge    GetTickTime();
        static UHuge    GetTickFrequency();

        /**/
        static Bool AccessUserPath( PathString& path, const WCHAR* app_name );

        /**/
        static Bool EnableAutoStart( const WCHAR* name, const PathString& path, Bool enable );

        /**/
        static HANDLE OpenDebugPrivileges();
        static void   CloseDebugPrivileges( HANDLE handle );
    };
}

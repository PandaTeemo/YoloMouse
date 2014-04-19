#pragma once
#include <Core/Constants.hpp>

namespace Core
{
    /**/
    class SystemTools
    {
    public:
        /**/
        static Bitness GetProcessBitness( DWORD process_id );

        /**/
        static OsVersion GetOsVersion();

        /**/
        static Bool EnableAutoStart( const WCHAR* name, const WCHAR* path, Bool enable );

        /**/
        static Bool GetProcessDirectory( WCHAR* path, ULong limit );
        static Bool GetProcessDirectory( DWORD process_id, WCHAR* path, ULong limit );
    };
}

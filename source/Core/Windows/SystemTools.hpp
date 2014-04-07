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
        static Bool EnableAutoStart( const Char* name, const Char* path, Bool enable );

        /**/
        static Bool GetProcessDirectory( Char* path, ULong limit );
        static Bool GetProcessDirectory( DWORD process_id, Char* path, ULong limit );
    };
}

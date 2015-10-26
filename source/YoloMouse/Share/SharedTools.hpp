#pragma once
#include <YoloMouse/Share/Root.hpp>

namespace YoloMouse
{
    /**/
    class SharedTools
    {
    public:
        /**/
        static Bool BuildTargetId( WCHAR* target_id, ULong limit, HANDLE process );

        /**/
        static Bool BuildSavePath( WCHAR* path, ULong limit, const WCHAR* name );

        /**/
        static void ErrorMessage( const Char* message );
    };
}

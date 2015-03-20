#pragma once
#include <YoloMouse/Share/Root.hpp>

namespace YoloMouse
{
    /**/
    class SharedTools
    {
    public:
        /**/
        static Hash CalculateCursorHash( HCURSOR hcursor );

        /**/
        static Bool BuildTargetId( WCHAR* target_id, ULong limit, DWORD process_id );

        /**/
        static Bool BuildSavePath( WCHAR* path, ULong limit, const WCHAR* name );

        /**/
        static void ErrorMessage( const Char* message );
    };
}

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
        static Bool BuildUserPath( WCHAR* path, ULong limit, const WCHAR* name, const WCHAR* extension, HANDLE process );

        /**/
        static void MessagePopup( Bool error, const Char* format, ... );

        /**/
        static Index CursorSizeToSizeIndex( ULong size );
        static ULong CursorToSize( HCURSOR hcursor );
    };
}

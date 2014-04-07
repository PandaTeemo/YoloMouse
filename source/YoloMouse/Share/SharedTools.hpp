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
        static Bool BuildTargetId( Char* target_id, ULong limit, HWND hwnd );

        /**/
        static void BuildTargetSavePath( Char* path, ULong limit, const Char* target_id );
    };
}

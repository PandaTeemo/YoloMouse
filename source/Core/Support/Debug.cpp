#include <Core/Constants.hpp>
#include <Core/Support/Debug.hpp>
#include <stdio.h>
#include <stdlib.h>

namespace Core
{
    // public
    //-------------------------------------------------------------------------
    void Debug::Console( const Char* format, ... )
    {
        static Bool open = false;
        va_list vargs;

        // only once
        if(!open)
        {
            FILE* file;

            AllocConsole();
            freopen_s(&file, "conin$", "r", stdin);
            freopen_s(&file, "conout$", "w", stdout);
            freopen_s(&file, "conout$", "w", stderr);
            open = true;
        }

        xassert(format != NULL);
        va_start(vargs, format);
        vfprintf(stdout, format, vargs); fputs("", stdout);
        va_end(vargs);
    }
}

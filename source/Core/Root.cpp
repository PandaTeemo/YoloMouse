#pragma once
#include <Core/Root.hpp>
#include <Core/Support/Tools.hpp>
#include <stdio.h>

namespace Core
{
    //------------------------------------------------------------------------
    void LogFile( const Char* format, ... )
    {
        FILE* file;

        if(format == NULL)
        {
            if(fopen_s(&file, LOG_PATH, "w") == 0)
                fclose(file);
        }
        else if(fopen_s(&file, LOG_PATH, "at") == 0)
        {
            va_list args;
            va_start(args, format);
            vfprintf(file, format, args);
            va_end(args);
            fclose(file);
        }
    }

    void LogAssert( const char* message, const char *file, unsigned line )
    {
        // log
        LogFile("%s @ %s:%u\n", message, file, line);

        // the end!
        abort();
    }

    void LogConsole( const Char* format, ... )
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

    //------------------------------------------------------------------------
    void ExceptionMessage( const char* message, const char *file, unsigned line )
    {
        static Char buffer[0x100];

        // path length
        ULong file_length = (ULong)strlen(file);

        // create message
        sprintf_s(buffer, sizeof(buffer),
            "%s\n\n...%s:%u",
            message,
            file + file_length - Tools::Min<ULong>(file_length, 20),
            line);

        // throw message
        throw buffer;
    }
}

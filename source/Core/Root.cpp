#pragma once
#include <Core/Root.hpp>
#include <Core/Support/Debug.hpp>
#include <Core/Support/Tools.hpp>
#include <stdio.h>

namespace Core
{
    //------------------------------------------------------------------------
    void LogAssert( const char* message, const char *file, unsigned line )
    {
        // log
        Debug::Log("%s @ %s:%u\n", message, file, line);

        // the end!
        abort();
    }

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

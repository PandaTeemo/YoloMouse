#include <Core/Root.hpp>
#include <Core/System/Debug.hpp>
#include <stdio.h>

namespace Core
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        static const ULong LOG_STRING_LIMIT = 1024;
    }

    // logging
    //-------------------------------------------------------------------------
    void DebugLog( const Char* format, ... )
    {
        va_list vargs;
        Char    message[LOG_STRING_LIMIT];
        ASSERT(format != nullptr);

        // build log message
        va_start(vargs, format);
        vsnprintf(message, COUNT(message), format, vargs);
        va_end(vargs);

        // log message
        Debug::Log(String(message));
    }

    // failure
    //-------------------------------------------------------------------------
    void DebugFatal( const Char* format, ... )
    {
        va_list vargs;
        Char    message[LOG_STRING_LIMIT];
        ASSERT(format != nullptr);

        // build log message
        va_start(vargs, format);
        vsnprintf(message, COUNT(message), format, vargs);
        va_end(vargs);

        // log message
        Debug::Log(String(message));

        // exit
        Debug::Exit();
    }

    void DebugAssert( const Char* message, const Char *file, ULong line )
    {
        DebugFatal( "ASSERT:%s (%s:%u)", message, file, line );
    }
}

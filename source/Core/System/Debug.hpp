#pragma once
#include <Core/Container/String.hpp>
#include <Core/Types.hpp>
#include <stdio.h>

namespace Core
{
    /**/
    class Debug
    {
    public:
        /**/
        static void Initialize( const PathString& log_path, Bool host );
        static void Shutdown();

        /**/
        static void Exit();

        /**/
        static void Log( const String& message );
        static void LogFile( const String& message );
        static void LogConsole( const String& message );

    private:
        /**/
        static void _InitializeFileLog( const PathString& log_path, Bool host );
        static void _InitializeConsoleLog();

        // fields
        static PathString _log_path;
    };
}

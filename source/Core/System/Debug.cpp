#include <Core/System/Debug.hpp>
#include <io.h>

namespace Core
{
    // staitcs
    //-------------------------------------------------------------------------
    PathString Debug::_log_path;

    // public
    //-------------------------------------------------------------------------
    void Debug::Initialize( const PathString& log_path, Bool host )
    {
    #ifdef OPTION_DEBUG_LOG_FILE
        // initialize file logging
        _InitializeFileLog(log_path, host);
    #endif
    #ifdef OPTION_DEBUG_LOG_CONSOLE
        // initialize console logging
        _InitializeConsoleLog();
    #endif
    }

    void Debug::Shutdown()
    {
    #ifdef OPTION_DEBUG_LOG_CONSOLE
        // shutdown console logging
        FreeConsole();
    #endif
    }

    //-------------------------------------------------------------------------
    void Debug::Exit()
    {
        // abort/exit/terminate (which one!)
        abort();
    }

    //-------------------------------------------------------------------------
    void Debug::Log( const String& message )
    {
        // log message
    #ifdef OPTION_DEBUG_LOG_FILE
        Debug::LogFile(String(message));
    #endif
    #ifdef OPTION_DEBUG_LOG_CONSOLE
        Debug::LogConsole(String(message));
    #endif
    }

    void Debug::LogFile( const String& message )
    {
        FILE* file;

        // open log file, write, and close. this is ghetto way to allow parallel writes.
        if( _log_path[0] && _wfopen_s(&file, _log_path.GetMemory(), L"at") == 0 )
        {
            fwrite( message.GetMemory(), 1, message.GetCount(), file );
            fputc( '\n', file );
            fclose(file);
        }
    }

    void Debug::LogConsole( const String& message )
    {
        fwrite( message.GetMemory(), 1, message.GetCount(), stdout );
        fputc( '\n', stdout );
    }

    // private
    //-------------------------------------------------------------------------
    void Debug::_InitializeFileLog( const PathString& log_path, Bool host )
    {
        // set fields
        _log_path = log_path;

        // if host, delete existing log file
        if( host )
            _wunlink(_log_path.GetMemory());
    }

    void Debug::_InitializeConsoleLog()
    {
        FILE* file;

        // allocate console
        AllocConsole();

        // associate standard streams with console
        freopen_s(&file, "conin$", "r", stdin);
        freopen_s(&file, "conout$", "w", stdout);
        freopen_s(&file, "conout$", "w", stderr);
    }
}

#include <YoloMouse/Loader/App.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <Core/Windows/SystemTools.hpp>

namespace YoloMouse
{
    // types
    //-------------------------------------------------------------------------
    enum ExitStatus
    {
        EXIT_NORMAL,
        EXIT_ELEVATE,
        EXIT_ERROR,
        EXIT_PLATFORM,
    };

    // main
    //-------------------------------------------------------------------------
    static ExitStatus Main()
    {
        App app;

        // start
        app.Start();

        // run
        app.Run();

        // stop
        app.Stop();

        // return exit status
        return app.GetElevate() ? EXIT_ELEVATE : EXIT_NORMAL;
    }

    // debugging
    //-------------------------------------------------------------------------
    void _UnitTest()
    {
    }
}

// platform
//-----------------------------------------------------------------------------
int WINAPI WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance, 
    LPSTR       lpCmdLine,
    int         iCmdShow)
{
    using namespace Core;

    //YoloMouse::_UnitTest(); return 0;

    // restart loop
    while( true )
    {
        // default exit status
        int status = YoloMouse::EXIT_NORMAL;

        // create duplicate instance prevention mutex
        HANDLE instance_mutex = CreateMutex( NULL, TRUE, YoloMouse::IPC_MUTEX_NAME );

        // if failed to create
        if( instance_mutex == NULL )
            status = YoloMouse::EXIT_PLATFORM + 0;
        else
        {
            // if duplicate instance
            if( GetLastError() == ERROR_ALREADY_EXISTS )
                status = YoloMouse::EXIT_PLATFORM + 2;
            // else good to go
            else
            {
                PathString path;

                // ensure working directory is that of the main executable
                if(SystemTools::GetProcessDirectory(path, COUNT(path)) && SetCurrentDirectory(path))
                {
                    // run main
                    try
                    {
                        status = YoloMouse::Main();
                    }
                    // catch eggs
                    catch( const Char* error )
                    {
                        YoloMouse::SharedTools::ErrorMessage(error);
                        status = YoloMouse::EXIT_PLATFORM + 1;
                    }
                }
                // path change failed
                else
                    status = YoloMouse::EXIT_PLATFORM + 4;
            }

            // cleanup
            CloseHandle( instance_mutex );
        }

        // if elevate requested relaunch as administrator
        if( status == YoloMouse::EXIT_ELEVATE )
        {
            // relaunch self as administrator. return value over 32 indicates success
            if( (Long)ShellExecute(NULL, L"runas", YoloMouse::PATH_LOADER, L"", NULL, SW_SHOWNORMAL) > 32 )
                return 0;

            // else restart current process
        }
        // normal exit
        else
            return status;
    }
}

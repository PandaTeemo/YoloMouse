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
        EXIT_PLATFORM_MUTEX,
        EXIT_PLATFORM_DUPLICATE,
        EXIT_PLATFORM_MAIN,
        EXIT_PLATFORM_PATH,
        EXIT_PLATFORM_RUNASADMIN,
    };

    // main
    //-------------------------------------------------------------------------
    static ExitStatus Main()
    {
        App         app;
        ExitStatus  status;

        // run main
        try
        {
            // start
            app.Start();

            // run
            app.Run();

            // stop
            app.Stop();

            // normal or elevated exit
            status = app.GetElevate() ? EXIT_ELEVATE : EXIT_NORMAL;
        }
        // catch eggs
        catch( const Char* error )
        {
            // show error message
            SharedTools::MessagePopup(true, error);

            // stop
            app.Stop();

            // error exit
            status = EXIT_PLATFORM_MAIN;
        }

        return status;
    }

    // unit testing area
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
    using namespace YoloMouse;

    //YoloMouse::_UnitTest(); return 0;

    // restart loop
    while( true )
    {
        // default exit status
        int status = EXIT_NORMAL;

        // create duplicate instance prevention mutex
        HANDLE instance_mutex = CreateMutex( NULL, TRUE, IPC_MUTEX_NAME );

        // if failed to create
        if( instance_mutex == NULL )
            status = EXIT_PLATFORM_MUTEX;
        else
        {
            // if duplicate instance
            if( GetLastError() == ERROR_ALREADY_EXISTS )
                status = EXIT_PLATFORM_DUPLICATE;
            // else good to go
            else
            {
                PathString path;

                // ensure working directory is that of the main executable
                if(SystemTools::GetProcessDirectory(path, COUNT(path)) && SetCurrentDirectory(path))
                {
                    // run main
                    status = Main();
                }
                // path change failed
                else
                    status = EXIT_PLATFORM_PATH;
            }

            // cleanup
            CloseHandle( instance_mutex );
        }

        // if elevate requested relaunch as administrator
        if( status == EXIT_ELEVATE )
        {
            // relaunch self as administrator. return value over 32 indicates success
            if( (Long)(Huge)ShellExecute(NULL, L"runas", PATH_LOADER, L"", NULL, SW_SHOWNORMAL) > 32 )
                return 0;
            else
            {
                SharedTools::MessagePopup(true, "Run As Administrator Failed");
                return EXIT_PLATFORM_RUNASADMIN;
            }
        }
        // normal exit
        else
        {
            // report exit error except duplicate
            if( status && status != EXIT_PLATFORM_DUPLICATE )
                SharedTools::MessagePopup(true, "Start Error: %d", status);

            return status;
        }
    }
}

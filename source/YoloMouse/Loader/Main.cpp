#include <YoloMouse/Loader/Core/App.hpp>
#include <Core/System/SystemTools.hpp>
#include <Core/Windows/WindowTools.hpp>

namespace Yolomouse
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
    static ExitStatus Main( HINSTANCE hinstance )
    {
        App&        app = App::Instance();
        ExitStatus  status = EXIT_NORMAL;

        // initialize app
        if( app.Initialize( hinstance ) )
        {
            // run window loop
            WindowTools::RunWindowLoop();

            // shutdown app
            app.Shutdown();
        }
        // else error
        else
            status = EXIT_ERROR;

        // return status if error
        if( status  )
            return status;

        // if elevated requested, return elevate status
        if( app.GetElevate() )
            return EXIT_ELEVATE;

        return EXIT_NORMAL;
    }

    // temporary unit testing area
    //-------------------------------------------------------------------------
    void _UnitTest( HINSTANCE hInstance )
    {
        /*
        Overlay& w = Overlay::Instance();
        w.Initialize( hInstance );
        w.Run();
        w.Shutdown();
        */
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
    using namespace Yolomouse;

    //YoloMouse::_UnitTest(hInstance); return 0;

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
                if( SystemTools::GetProcessDirectory(path) && SetCurrentDirectory(path.GetMemory()) )
                {
                    // raise our priority to improve overlay cursor accuracy (allow fail)
                    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

                    // run main
                    status = Main(hInstance);
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
                WindowTools::MessagePopup(APP_NAMEC, true, "Run As Administrator Failed");
                return EXIT_PLATFORM_RUNASADMIN;
            }
        }
        // normal exit
        else
        {
            // report exit error except duplicate
            if( status && status != EXIT_PLATFORM_DUPLICATE )
                WindowTools::MessagePopup(APP_NAMEC, true, "Start Error: %d", status);

            return status;
        }
    }
}

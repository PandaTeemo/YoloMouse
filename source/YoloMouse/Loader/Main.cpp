#include <YoloMouse/Loader/App.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <Core/Windows/SystemTools.hpp>

namespace YoloMouse
{
    // main
    //-------------------------------------------------------------------------
    static void Main()
    {
        App app;

        // start
        app.Start();

        // run
        app.Run();

        // stop
        app.Stop();
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
    int status;

    // create duplicate instance prevention mutex
    HANDLE instance_mutex = CreateMutex( NULL, TRUE, YoloMouse::IPC_MUTEX_NAME );

    // if failed to create
    if( instance_mutex == NULL )
        status = 3;
    else
    {
        // if duplicate instance
        if( GetLastError() == ERROR_ALREADY_EXISTS )
            status = 2;
        // else good to go
        else
        {
            WCHAR path[STRING_PATH_SIZE];

            // ensure working directory is that of the main executable
            if(SystemTools::GetProcessDirectory(path, COUNT(path)) && SetCurrentDirectory(path))
            {
                // run main
                try
                {
                    YoloMouse::Main();
                    status = 0;
                }
                // catch eggs
                catch( const Char* error )
                {
                    YoloMouse::SharedTools::ErrorMessage(error);
                    status = 1;
                }
            }
            // path change failed
            else
                status = 4;
        }

        // cleanup
        CloseHandle( instance_mutex );
    }

    return status;
}

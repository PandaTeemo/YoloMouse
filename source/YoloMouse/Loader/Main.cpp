#include <YoloMouse/Loader/App.hpp>
#include <Core/Windows/SystemTools.hpp>

namespace YoloMouse
{
    // testing
    //-------------------------------------------------------------------------
    /*
    static VOID HOOK_CALL _OnSleep( x86::Registers registers )
    {
    }

    static void _test()
    {
        Hooks::Hook hook(Sleep,_OnSleep);
        hook.Init();
        hook.Enable();
        Sleep(100);
        hook.Disable();
    }
    //*/

    // main
    //-------------------------------------------------------------------------
    static void Main()
    {
        //_test(); return;
        //Debug::Log(NULL); system("del Settings.ini"); system("del /Q Save\\*.*");

        App& app = App::Instance();

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
            Char path[STRING_PATH_SIZE];

            // ensure working directory is that of the main executable
            if(SystemTools::GetProcessDirectory(path, sizeof(path)) && SetCurrentDirectory(path))
            {
                // run main
                try
                {
                    YoloMouse::Main();
                    status = 0;
                }
                // catch eggs
                catch( const Core::Char* error )
                {
                    MessageBox(NULL, error, YoloMouse::APP_NAME, MB_OK|MB_ICONERROR);
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

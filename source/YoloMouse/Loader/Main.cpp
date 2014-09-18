#include <YoloMouse/Loader/App.hpp>
#include <Core/Windows/SystemTools.hpp>

#include <Snoopy/Inject/Injector.hpp>

namespace YoloMouse
{
    // testing
    //-------------------------------------------------------------------------
    /*
    static VOID HOOK_CALL _OnHookSetCursor( HCURSOR cursor )
    {
    }

    static VOID HOOK_CALL _OnHookSleep( Native* arguments )
    {
        xlog("TEST!\n");
        //arguments[1] = 100;
    }

    static void _test1()
    {
        //Snoopy::Hook hook(Sleep, (x86::HookFunction)_OnHookSleep);
        Snoopy::Hook hook(SetClassLongPtrA, (x86::HookFunction)_OnHookSleep);
        hook.Init();
        hook.Enable();
        SetClassLongPtrA(0, -12, 123);
        //Sleep(2000);
        hook.Disable();
    }
    //*/

    // main
    //-------------------------------------------------------------------------
    static void Main()
    {
        //xlog(NULL); system("del /Q C:\\Users\\Administrator\\AppData\\Local\\YoloMouse\\*.*");
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
    //YoloMouse::_test1(); return 0;
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
                    MessageBoxA(NULL, error, YoloMouse::APP_NAMEC, MB_OK|MB_ICONERROR);
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

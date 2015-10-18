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

    // debugging
    //-------------------------------------------------------------------------
    void _UnitTest()
    {
        /*
        //Byte xx[] = {0xFF, 0x25, 0x1E, 0x00, 0x81, 0x71};         //32: jmp absolute
        //Byte xx[] = {0xFF, 0x25, 0x90, 0xEF, 0x5C, 0x00};         //64: jmp relative
        //Byte xx[] = {0xB8, 0x9F, 0x20, 0x07, 0x00};               //64: mov reg constant
        Byte xx[] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x08};   //64: 2 regular instructions
        
        Byte buff[20] = {0};
        Snoopy::x86::Assembly zz;
        ByteArray ba(xx, sizeof(xx));
        ByteArray bzz(buff, sizeof(buff));
        zz.SetSourceAddress(0);
        zz.SetCodeBuffer(ba);
        Snoopy::x86::Assembly aa;
        aa.SetCodeBuffer(bzz);
        aa.Empty();

        aa.Relocate(zz);
        return;
        //*/
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

    //YoloMouse::_UnitTest(); return 0;

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

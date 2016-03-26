#include <YoloMouse/Dll/App.hpp>
#include <YoloMouse/Share/NotifyMessage.hpp>
using namespace YoloMouse;

// exports
//-----------------------------------------------------------------------------
void __declspec(dllexport)
YoloNotify( void* arg )
{
    NotifyMessage& m = *reinterpret_cast<NotifyMessage*>(arg);

    // handle notify
    switch(m.id)
    {
    case NOTIFY_INIT:
        App::Load();
        break;

    case NOTIFY_SETCURSOR:
        App::UpdateCursor(static_cast<Index>(m.parameter));
        break;

    case NOTIFY_SETSIZE:
        App::UpdateSize(static_cast<Long>(m.parameter));
        break;

    case NOTIFY_SETDEFAULT:
        App::UpdateDefault();
        break;

    case NOTIFY_REFRESH:
        App::Refresh();
        break;
    }
}

// platform
//-----------------------------------------------------------------------------
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    // dll requests
    switch(fdwReason)
    {
    case DLL_PROCESS_DETACH:
        App::Unload();
        break;
    }

    return TRUE;
}

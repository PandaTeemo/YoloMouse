#include <YoloMouse/Dll/CursorHook.hpp>
#include <YoloMouse/Share/NotifyMessage.hpp>
using namespace YoloMouse;

// exports
//-----------------------------------------------------------------------------
void __declspec(dllexport)
YoloNotify( void* arg )
{
    NotifyMessage&  m = *reinterpret_cast<NotifyMessage*>(arg);
    HWND            hwnd = reinterpret_cast<HWND>(m.hwnd);

    // handle notify
    switch(m.id)
    {
    case NOTIFY_INIT:
        CursorHook::Load(hwnd);
        break;

    case NOTIFY_ASSIGN:
        CursorHook::Assign(hwnd, static_cast<Index>(m.parameter));
        break;

    case NOTIFY_REFRESH:
        CursorHook::Refresh(hwnd);
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
        CursorHook::Unload();
        break;
    }

    return TRUE;
}

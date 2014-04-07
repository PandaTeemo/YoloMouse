#include <YoloMouse/Dll/CursorHook.hpp>

// exports
//-----------------------------------------------------------------------------
LRESULT __declspec(dllexport) CALLBACK
OnHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    CWPSTRUCT& m = *(CWPSTRUCT*)lParam;

    // intercept messages
    switch(m.message)
    {
    case YoloMouse::WMYOLOMOUSE_INIT:
        YoloMouse::CursorHook::Load(m.hwnd);
        return 0;

    case YoloMouse::WMYOLOMOUSE_ASSIGN:
        YoloMouse::CursorHook::Assign(m.hwnd, (Core::Index)m.wParam);
        return 0;
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// platform
//-----------------------------------------------------------------------------
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    // dll requests
    switch(fdwReason)
    {
    case DLL_PROCESS_DETACH:
        YoloMouse::CursorHook::Unload();
        break;
    }

    return TRUE;
}

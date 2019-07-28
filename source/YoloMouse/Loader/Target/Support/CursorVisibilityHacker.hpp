/*
    this is a complete hack to control cursor visibility of another window/process.

    you'd think hiding an app's cursor would be simple (or maybe it is and i missed something) but it requires 
    injecting into the process. instead i roll with a windows hooks exploit whereby i provide the hook an  
    already loaded  dll (user32.dll) and a pointer to ShowCursor(bool) instead of a legit hook callback. this hook 
    is then  externally triggered such that ShowCursor gets the first argument of a hook routine (nCode) such that 
    the  value is 0 aka FALSE.
*/
#pragma once
#include <Core/Support/Singleton.hpp>
#include <YoloMouse/Share/Root.hpp>
#include <windows.h>

namespace Yolomouse
{
    /**/
    class CursorVisibilityHacker:
        public Singleton<CursorVisibilityHacker>
    {
    public:
        /**/
        CursorVisibilityHacker();
        ~CursorVisibilityHacker();

        /**/
        Bool Initialize();
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        Bool Show( HWND hwnd, ULong timeout_ms );
        Bool Hide( HWND hwnd, ULong timeout_ms );

    private:
        /**/
        static Bool _IsCursorVisible();

        /**/
        Bool _TriggerLoop( Bool visibility, ULong timeout_ms );

        // fields: state
        Bool        _initialized;
        HMODULE     _hook_dll;
        void*       _hook_proc;
    };
}

/*
    this is a complete hack to control cursor visibility of another window/process.
    it uses winhooks and existing 
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

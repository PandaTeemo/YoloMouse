#pragma once
#include <Core/Container/String.hpp>
#include <Core/Support/Singleton.hpp>

namespace Core
{
    /**/
    class ShellUi:
        public Singleton<ShellUi>
    {
    public:
        /**/
        struct IListener
        {
            virtual Bool OnMessage( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam ) { return false; }
            virtual Bool OnMenuOption( Id id, Bool& enabled ) { return false; }
        };

    public:
        /**/
        ShellUi();

        /**/
        HWND GetHwnd();

        /**/
        void AddMenu();
        void AddMenuOption( Id id, const Char* name, Bool enabled );

        /**/
        void HideMenu();

        /**/
        void SetIcon( Id icon_id );
        void SetName( const String& name );

        /**/
        void AddListener( IListener& listener );
        void RemoveListener( IListener& listener );

        /**/
        Bool Start();
        void Stop();

        /**/
        void Run();
    };
}

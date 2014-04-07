#pragma once
#include <Core/Support/Singleton.hpp>

namespace Core
{
    /**/
    class SystemMonitor:
        public Singleton<SystemMonitor>
    {
    public:
        struct IListener
        {
            virtual void OnWindowCreate( HWND hwnd ) {}
            virtual void OnWindowDestroy( HWND hwnd ) {}
            virtual void OnWindowFocus( HWND hwnd ) {}
        };

    public:
        /**/
        void SetListener( IListener* listener=NULL );

        /**/
        Bool Start();
        void Stop();
    };
}

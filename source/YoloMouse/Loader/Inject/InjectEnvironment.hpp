#pragma once
#include <Core/Support/Singleton.hpp>
#include <Snoopy/Inject/Injector.hpp>
#include <YoloMouse/Share/Root.hpp>
#include <windows.h>

namespace Yolomouse
{
    /**/
    class InjectEnvironment:
        public Singleton<InjectEnvironment>
    {
    public:
        /**/
        InjectEnvironment();
        ~InjectEnvironment();

        /**/
        void Initialize();
        void Shutdown();

        /**/
        Bool IsInitialized() const;

    private:
        /**/
        Bool _InitializePrivileges();

        /**/
        void _ShutdownPrivileges();

        // fields
        HANDLE _hprivileges;
    };
}

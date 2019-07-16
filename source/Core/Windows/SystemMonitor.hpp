#pragma once
#include <Core/Events/SystemMonitorEvent.hpp>
#include <Core/Support/Singleton.hpp>
#include <Core/Support/EventDispatcher.hpp>

namespace Core
{
    /**/
    class SystemMonitor:
        public Singleton<SystemMonitor>
    {
    public:
        // enums
        enum EventId: Id
        {
            WINDOW_FOREGROUND,
            WINDOW_ZORDER,
        };

        /**/
        SystemMonitor();
        ~SystemMonitor();

        /**/
        Bool Initialize();
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        // events
        EventDispatcher<SystemMonitorEvent> events;

    private:
        // fields
        HWINEVENTHOOK _handle;
    };
}

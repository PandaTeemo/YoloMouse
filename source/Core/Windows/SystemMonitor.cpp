#include <Core/Windows/SystemMonitor.hpp>

namespace Core
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // event handler
        //---------------------------------------------------------------------
        static VOID CALLBACK _WinEvent(
            HWINEVENTHOOK hWinEventHook,
            DWORD         event,
            HWND          hwnd,
            LONG          idObject,
            LONG          idChild,
            DWORD         idEventThread,
            DWORD         dwmsEventTime)
        {
            if( idObject == OBJID_WINDOW && idChild == CHILDID_SELF )
            {
                SystemMonitor& system_monitor = SystemMonitor::Instance();

                switch(event)
                {
                case EVENT_SYSTEM_FOREGROUND:
                    system_monitor.events.Notify( { SystemMonitorEvent::WINDOW_FOREGROUND, hwnd } );
                /*
                    system_monitor.events.Notify( { SystemMonitorEvent::WINDOW_ZORDER, hwnd } );
                    break;

                case EVENT_OBJECT_REORDER:
                case EVENT_OBJECT_LOCATIONCHANGE:
                case EVENT_OBJECT_PARENTCHANGE:
                    system_monitor.events.Notify( { SystemMonitorEvent::WINDOW_ZORDER, hwnd } );
                    break;
                */
                }
            }
        }
    }

    // public
    //-------------------------------------------------------------------------
    SystemMonitor::SystemMonitor():
        _handle  (NULL)
    {
    }

    SystemMonitor::~SystemMonitor()
    {
    }

    //-------------------------------------------------------------------------
    Bool SystemMonitor::Initialize()
    {
        // set window event hook
        _handle = SetWinEventHook( EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, _WinEvent, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS );

        return _handle != NULL;
    }

    void SystemMonitor::Shutdown()
    {
        // if window event hook set
        if( _handle != NULL )
        {
            // destroy window event hook
            UnhookWinEvent(_handle);

            // clear state
            _handle = NULL;
        }
    }

    //-------------------------------------------------------------------------
    Bool SystemMonitor::IsInitialized() const
    {
        return _handle != NULL;
    }
}

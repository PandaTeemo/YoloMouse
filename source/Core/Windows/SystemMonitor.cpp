#pragma once
#include <Core/Windows/SystemMonitor.hpp>

namespace Core
{
    // local: state
    //-------------------------------------------------------------------------
    static SystemMonitor::IListener  _dummy_listener;
    static SystemMonitor::IListener* _listener = &_dummy_listener;

    // local: handler
    //-------------------------------------------------------------------------
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
            switch(event)
            {
            case EVENT_SYSTEM_FOREGROUND:
                _listener->OnWindowFocus(hwnd);
                break;
            }
        }
    }

    // public
    //-------------------------------------------------------------------------
    void SystemMonitor::SetListener( IListener* listener )
    {
        _listener = listener ? listener : &_dummy_listener;
    }

    //-------------------------------------------------------------------------
    Bool SystemMonitor::Start()
    {
        return 
            SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, _WinEvent, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS) != 0;
    }

    //-------------------------------------------------------------------------
    void SystemMonitor::Stop()
    {
    }
}

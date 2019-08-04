#pragma once
#include <YoloMouse/Share/Root.hpp>

namespace Yolomouse
{
    // enums
    //-------------------------------------------------------------------------
    enum IpcRequest: Byte
    {
        IPC_REQUEST_INVALID,
        IPC_REQUEST_LOAD,
        IPC_REQUEST_EXIT,
        IPC_REQUEST_SET_CURSOR,
        IPC_REQUEST_SET_DEFAULT_CURSOR,
        IPC_REQUEST_RESET_CURSOR,
        IPC_REQUEST_REFRESH_CURSOR,
        IPC_REQUEST_ON_CURSOR_CHANGING,
        IPC_REQUEST_ON_CURSOR_SHOWING,
    };

    enum
    {
        SETTING_CURSORKEY_BASIC,
        SETTING_CURSORKEY_OVERLAY,
        SETTING_CURSORKEY_VARIATION,
        SETTING_CURSORKEY_DEFAULT,
        SETTING_CURSORKEY_RESET,
        SETTING_CURSORKEY_SMALLER,
        SETTING_CURSORKEY_LARGER,
        SETTING_AUTOSTART,
        SETTING_REDUCEOVERLAYLAG,
        SETTING_SHOWMENU,
        SETTING_COUNT
    };

    enum
    {
        // info
        MENU_OPTION_ABOUT,
        MENU_OPTION_ERRORS,
        // options
        MENU_OPTION_AUTOSTART,
        MENU_OPTION_REDUCEOVERLAYLAG,
        // etc
        MENU_OPTION_RUNASADMIN,
        MENU_OPTION_SETTINGSFOLDER,

        MENU_OPTION_COUNT
    };
}

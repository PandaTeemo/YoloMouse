#pragma once
#include <YoloMouse/Share/Root.hpp>

namespace Yolomouse
{
    // enums
    //-------------------------------------------------------------------------
    enum CursorType: Byte
    {
        CURSOR_TYPE_BASIC,      // one of preinstalled (or user provided) cur/ani files
        CURSOR_TYPE_CLONE,      // clones existing application cursor, used to modding that cursor like resize
        CURSOR_TYPE_OVERLAY,    // a directx overlay cursor, this has a little lag despite methods to reduce it, but its much prettier!
        CURSOR_TYPE_INVALID
    };

    typedef Byte CursorId;
    enum: Byte
    {
        CURSOR_ID_COUNT =           10,
        CURSOR_ID_INVALID
    };

    typedef Byte CursorVariation;
    enum: Byte
    {
        CURSOR_VARIATION_COUNT =    10,
        CURSOR_VARIATION_INVALID
    };

    typedef Char CursorSize;
    enum: Char
    {
        CURSOR_SIZE_ORIGINAL =      0,
        CURSOR_SIZE_DEFAULT =       7,
        CURSOR_SIZE_COUNT =         16,
        CURSOR_SIZE_INVALID
    };

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
        SETTING_CURSORKEY_1,
        SETTING_CURSORKEY_2,
        SETTING_CURSORKEY_3,
        SETTING_CURSORKEY_4,
        SETTING_CURSORKEY_5,
        SETTING_CURSORKEY_6,
        SETTING_CURSORKEY_7,
        SETTING_CURSORKEY_8,
        SETTING_CURSORKEY_9,
        SETTING_CURSORKEY_OVERLAY_1,
        SETTING_CURSORKEY_OVERLAY_2,
        SETTING_CURSORKEY_OVERLAY_3,
        SETTING_CURSORKEY_OVERLAY_4,
        SETTING_CURSORKEY_OVERLAY_5,
        SETTING_CURSORKEY_OVERLAY_6,
        SETTING_CURSORKEY_OVERLAY_7,
        SETTING_CURSORKEY_OVERLAY_8,
        SETTING_CURSORKEY_OVERLAY_9,
        SETTING_CURSORKEY_DEFAULT,
        SETTING_CURSORKEY_RESET,
        SETTING_CURSORKEY_SMALLER,
        SETTING_CURSORKEY_LARGER,
        SETTING_CURSORKEY_DEFAULT2,
        SETTING_CURSORKEY_RESET2,
        SETTING_CURSORKEY_SMALLER2,
        SETTING_CURSORKEY_LARGER2,
        SETTING_GAMESONLY,
        SETTING_AUTOSTART,
        SETTING_SHOWMENU,
        SETTING_COUNT
    };

    enum
    {
        // info
        MENU_OPTION_ABOUT,
        MENU_OPTION_ERRORS,
        // options
        //MENU_OPTION_GAMESONLY,
        MENU_OPTION_AUTOSTART,
        // etc
        MENU_OPTION_RUNASADMIN,
        MENU_OPTION_SETTINGSFOLDER,

        MENU_OPTION_COUNT
    };
}

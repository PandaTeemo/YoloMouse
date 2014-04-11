#pragma once
#include <Core/Support/Settings.hpp>
#include <YoloMouse/Share/Root.hpp>

namespace YoloMouse
{
    // enums
    //-------------------------------------------------------------------------
    enum
    {
        WMYOLOMOUSE_INIT =      WM_APP + 0,
        WMYOLOMOUSE_ASSIGN =    WM_APP + 1,
        WMYOLOMOUSE_REFRESH =   WM_APP + 2,
    };

    enum
    {
        // these should match SHARED_CURSOR_LIMIT
        SETTING_CURSORKEY_1,
        SETTING_CURSORKEY_2,
        SETTING_CURSORKEY_3,
        SETTING_CURSORKEY_4,
        SETTING_CURSORKEY_5,
        SETTING_CURSORKEY_6,
        SETTING_CURSORKEY_7,
        SETTING_CURSORKEY_8,
        SETTING_CURSORKEY_9,
        SETTING_CURSORKEY_RESET,
        SETTING_CURSORKEY_SMALLER,
        SETTING_CURSORKEY_LARGER,
        SETTING_CURSORSIZE,
        SETTING_AUTOSTART,
        SETTING_SHOWMENU,
    };

    enum
    {
        MENU_OPTION_AUTOSTART,
        MENU_OPTION_SHOWMENU,
        MENU_OPTION_SMALLER,
        MENU_OPTION_LARGER,
        MENU_OPTION_COUNT
    };

    enum CursorSize
    {
        CURSOR_SIZE_TINY,
        CURSOR_SIZE_SMALL,
        CURSOR_SIZE_MEDIUM,
        CURSOR_SIZE_LARGE,
        CURSOR_SIZE_HUGE,
        CURSOR_SIZE_COUNT,
    };

    // numeric
    //-------------------------------------------------------------------------
    static const ULong CURSOR_MAPPING_LIMIT =       10;
    static const ULong LOADER_ACTIVE_LIMIT =        100;
    static const ULong SHARED_CURSOR_LIMIT =        9;

    // strings
    //-------------------------------------------------------------------------
    extern const Char* APP_MENU_STRINGS[];
    static const Char* APP_NAME =                   "YoloMouse";

    static const Char* PATH_LOADER =                "YoloMouse.exe";
    static const Char* PATH_DLL32 =                 "Dll32.dll";
    static const Char* PATH_SETTINGS =              "Settings.ini";
    static const Char* PATH_CURSORS =               "Cursors";
    extern const Char* PATH_CURSORS_SIZE[];
    static const Char* PATH_SAVE =                  "Save";

    static const Char* IPC_MUTEX_NAME =             "YoloMouseMutex";
    static const Char* IPC_MEMORY_NAME =            "YoloMouseMemory";

    // settings
    //-------------------------------------------------------------------------
    extern Settings::KeyValueCollection SETTINGS_ITEMS;
}

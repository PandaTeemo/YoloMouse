#pragma once
#include <Core/Support/Settings.hpp>
#include <YoloMouse/Share/Root.hpp>

namespace YoloMouse
{
    // enums
    //-------------------------------------------------------------------------
    enum NotifyId
    {
        NOTIFY_INIT,
        NOTIFY_ASSIGN,
        NOTIFY_REFRESH,
    };

    enum
    {
        // number of SETTING_CURSORKEY_# should match SHARED_CURSOR_LIMIT
        SETTING_CURSORKEY_1,
        SETTING_CURSORKEY_2,
        SETTING_CURSORKEY_3,
        SETTING_CURSORKEY_4,
        SETTING_CURSORKEY_5,
        SETTING_CURSORKEY_6,
        SETTING_CURSORKEY_7,
        SETTING_CURSORKEY_8,
        SETTING_CURSORKEY_9,
        SETTING_CURSORKEY_10,
        SETTING_CURSORKEY_11,
        SETTING_CURSORKEY_12,
        SETTING_CURSORKEY_13,
        SETTING_CURSORKEY_14,
        SETTING_CURSORKEY_15,
        SETTING_CURSORKEY_16,
        SETTING_CURSORKEY_17,
        SETTING_CURSORKEY_18,
        SETTING_CURSORKEY_19,
        SETTING_CURSORKEY_20,
        SETTING_CURSORKEY_21,
        SETTING_CURSORKEY_22,
        SETTING_CURSORKEY_23,
        SETTING_CURSORKEY_24,
        SETTING_CURSORKEY_25,
        SETTING_CURSORKEY_26,
        SETTING_CURSORKEY_27,
        SETTING_CURSORKEY_28,
        SETTING_CURSORKEY_29,
        SETTING_CURSORKEY_30,
        SETTING_CURSORKEY_31,
        SETTING_CURSORKEY_32,
        SETTING_CURSORKEY_33,
        SETTING_CURSORKEY_34,
        SETTING_CURSORKEY_35,
        SETTING_CURSORKEY_RESET,
        SETTING_CURSORKEY_SMALLER,
        SETTING_CURSORKEY_LARGER,
        SETTING_CURSORSIZE,
        SETTING_AUTOSTART,
    };

    enum
    {
        MENU_OPTION_AUTOSTART,
        MENU_OPTION_ABOUT,
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
    static const ULong CURSOR_MAPPING_LIMIT =       200;
    static const ULong LOADER_TARGET_LIMIT =        20;
    static const ULong SHARED_CURSOR_LIMIT =        35;

    // strings
    //-------------------------------------------------------------------------
    extern const WCHAR* APP_MENU_STRINGS[];
    static const WCHAR* APP_NAME =                  L"YoloMouse";
    static const Char*  APP_NAMEC =                 "YoloMouse";
    static const Char*  APP_ABOUT =                 "Version 0.5.5\nBy HaPpY :)";

    static const WCHAR* PATH_LOADER =               L"YoloMouse.exe";
    static const CHAR*  PATH_DLL32 =                "Yolo32.dll";
    static const CHAR*  PATH_DLL64 =                "Yolo64.dll";
    static const WCHAR* PATH_SETTINGS_NAME =        L"Settings";
    static const WCHAR* PATH_CURSORS =              L"Cursors";
    extern const WCHAR* PATH_CURSORS_SIZE[];

    static const CHAR*  INJECT_NOTIFY_FUNCTION =    "YoloNotify";

    static const WCHAR* IPC_MUTEX_NAME =            L"YoloMouseMutex";
    static const WCHAR* IPC_MEMORY_NAME =           L"YoloMouseMemory";

    // settings
    //-------------------------------------------------------------------------
    extern Settings::KeyValueCollection SETTINGS_ITEMS;
}

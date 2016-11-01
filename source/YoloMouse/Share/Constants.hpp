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
        NOTIFY_SETCURSOR,
        NOTIFY_SETSIZE,
        NOTIFY_SETDEFAULT,
        NOTIFY_REFRESH,
    };

    enum
    {
        SETTING_GROUPKEY_1,
        SETTING_GROUPKEY_2,
        SETTING_GROUPKEY_3,
        SETTING_GROUPKEY_4,
        SETTING_GROUPKEY_5,
        SETTING_GROUPKEY_6,
        SETTING_GROUPKEY_7,
        SETTING_GROUPKEY_8,
        SETTING_GROUPKEY_9,
        SETTING_GROUPKEY_RESET,
        SETTING_SIZEKEY_SMALLER,
        SETTING_SIZEKEY_LARGER,
        SETTING_DEFAULTKEY,
        SETTING_AUTOSTART,
        SETTING_SHOWMENU,
    };

    enum
    {
        MENU_OPTION_RUNASADMIN,
        MENU_OPTION_AUTOSTART,
        MENU_OPTION_SETTINGS,
        MENU_OPTION_ERRORS,
        MENU_OPTION_ABOUT,
        MENU_OPTION_COUNT
    };

    enum
    {
        CURSOR_SIZE_ORIGINAL,
        CURSOR_SIZE_16,
        CURSOR_SIZE_24,
        CURSOR_SIZE_32,
        CURSOR_SIZE_48,
        CURSOR_SIZE_64,
        CURSOR_SIZE_80,
        CURSOR_SIZE_96,
        CURSOR_SIZE_112,
        CURSOR_SIZE_128,
        CURSOR_SIZE_COUNT,

        CURSOR_SIZE_DEFAULT = CURSOR_SIZE_64
    };

    // numeric
    //-------------------------------------------------------------------------
    static const ULong APP_VERSION[] =              { 0, 8, 0 };
    static const ULong APP_NAME_LIMIT =             64;
    static const ULong LOG_MEMORY_LIMIT =           KILOBYTES(8);
    static const ULong LOADER_TARGET_LIMIT =        20;
    static const ULong CURSOR_BINDING_LIMIT =       100;
    static const ULong CURSOR_GROUP_SIZE =          10;
    static const ULong CURSOR_GROUP_COUNT =         9;
    static const ULong CURSOR_RESOURCE_LIMIT =      CURSOR_GROUP_COUNT * CURSOR_GROUP_SIZE;
    static const Index CURSOR_SPECIAL_REMOVE =      9999;
    static const ULong CURSOR_CACHE_LIMIT =         50;

    // strings
    //-------------------------------------------------------------------------
    extern const WCHAR* APP_MENU_STRINGS[];
    static const WCHAR* APP_NAME =                  L"YoloMouse";
    static const Char*  APP_NAMEC =                 "YoloMouse";

    static const WCHAR* PATH_LOADER =               L"YoloMouse.exe";
    static const CHAR*  PATH_DLL32 =                "Yolo32.dll";
    static const CHAR*  PATH_DLL64 =                "Yolo64.dll";
    static const WCHAR* PATH_SETTINGS_NAME =        L"Settings";
    static const WCHAR* PATH_CURSORS_CUSTOM =       L"Cursors";
    static const WCHAR* PATH_CURSORS_DEFAULT =      L"Cursors\\Default";
    static const WCHAR* PATH_ERRORS =               L"errors";
    static const WCHAR* EXTENSION_INI =             L"ini";
    static const WCHAR* EXTENSION_LOG =             L"txt";
    static const WCHAR* EXTENSION_STATIC =          L"cur";
    static const WCHAR* EXTENSION_ANIMATED =        L"ani";

    static const CHAR*  INJECT_NOTIFY_FUNCTION =    "YoloNotify";

    static const WCHAR* IPC_MUTEX_NAME =            L"YoloMouseMutex";
    static const WCHAR* IPC_MEMORY_NAME =           L"YoloMouseMemory";

    static const Char*  TEXT_ABOUT =                "Version %u.%u.%u %u-Bit\nBy HaPpY :)";
    static const Char*  TEXT_NOERRORS =             "No errors found... Yay! :D";

    // tables
    //-------------------------------------------------------------------------
    extern Settings::KeyValueCollection SETTINGS_ITEMS;
    extern const ULong                  CURSOR_SIZE_TABLE[CURSOR_SIZE_COUNT];
}

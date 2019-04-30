#pragma once
#include <Core/Support/Settings.hpp>
#include <YoloMouse/Share/Enums.hpp>

namespace YoloMouse
{
    // numeric
    //-------------------------------------------------------------------------
    static const ULong APP_VERSION[] =                      { 0, 10, 0 };
    static const ULong APP_NAME_LIMIT =                     64;
    static const ULong LOG_MEMORY_LIMIT =                   KILOBYTES(8);
    static const ULong LOADER_TARGET_LIMIT =                20;
    static const ULong CURSOR_BINDING_LIMIT =               100;
    static const ULong CURSOR_SIZE_INDEX_ORIGINAL =         0;
    static const ULong CURSOR_SIZE_INDEX_DEFAULT =          7;
    static const ULong CURSOR_SIZE_INDEX_COUNT =            16;
    static const ULong CURSOR_RESOURCE_PRESET_MINOR_COUNT = 10;
    static const ULong CURSOR_RESOURCE_PRESET_MAJOR_COUNT = 9;
    static const ULong CURSOR_RESOURCE_PRESET_COUNT =       CURSOR_RESOURCE_PRESET_MINOR_COUNT* CURSOR_RESOURCE_PRESET_MAJOR_COUNT;
    static const ULong CURSOR_RESOURCE_IDENTITY_LIMIT =     50;
    static const Index CURSOR_SPECIAL_REMOVE =              9999;
    static const ULong CURSOR_CACHE_LIMIT =                 50;
    static const ULong INI_LINE_LIMIT =                     256;

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
    static const WCHAR* EXTENSION_STATIC_CURSOR =   L"cur";
    static const WCHAR* EXTENSION_ANIMATED_CURSOR = L"ani";

    static const CHAR*  INJECT_NOTIFY_FUNCTION =    "YoloNotify";

    static const WCHAR* IPC_MUTEX_NAME =            L"YoloMouseMutex";
    static const WCHAR* IPC_MEMORY_NAME =           L"YoloMouseMemory";

    static const Char*  TEXT_ABOUT =                "Version %u.%u.%u %u-Bit\nBy HaPpY :)";
    static const Char*  TEXT_NOERRORS =             "No errors found... Yay! :D";

    // tables
    //-------------------------------------------------------------------------
    extern Settings::KeyValueCollection SETTINGS_ITEMS;
    extern const ULong                  CURSOR_SIZE_TABLE[CURSOR_SIZE_INDEX_COUNT];
    extern const ULong                  CURSOR_SIZE_TABLE_V_0_8_3[10];
}

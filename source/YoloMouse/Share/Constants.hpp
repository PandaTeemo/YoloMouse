#pragma once
#include <Core/Support/Settings.hpp>
#include <YoloMouse/Share/Enums.hpp>

namespace Yolomouse
{
    // numeric
    //-------------------------------------------------------------------------
    static const ULong APP_VERSION[] =              { 0, 11, 1 };
    static const ULong CURSOR_CACHE_LIMIT =         50;

    // strings
    //-------------------------------------------------------------------------
    extern const WCHAR* APP_MENU_STRINGS[];
    static const WCHAR* APP_NAME =                  L"YoloMouse";
    static const Char*  APP_NAMEC =                 "YoloMouse";
    static const WCHAR* OVERLAY_NAME =              L"YoloCursorOverlay";
    static const WCHAR* OVERLAY_CLASS =             L"YoloCursorClass";

    static const WCHAR* PATH_LOADER =               L"YoloMouse.exe";
    static const CHAR*  PATH_DLL32 =                "Yolo32.dll";
    static const CHAR*  PATH_DLL64 =                "Yolo64.dll";
    static const WCHAR* PATH_LOG_NAME =             L"log.txt";
    static const WCHAR* PATH_SETTINGS =             L"Settings.ini";
    static const WCHAR* PATH_CURSORS_CUSTOM =       L"Cursors";
    static const WCHAR* PATH_CURSORS_DEFAULT =      L"Cursors\\Default";
    static const WCHAR* EXTENSION_INI =             L"ini";
    static const WCHAR* EXTENSION_STATIC_CURSOR =   L"cur";
    static const WCHAR* EXTENSION_ANIMATED_CURSOR = L"ani";

    static const WCHAR* IPC_MUTEX_NAME =            L"YoloMouseMutex";
    static const WCHAR* IPC_MEMORY_NAME =           L"YoloMouseMemory";

    static const Char*  TEXT_ABOUT =                "Version %u.%u.%u %u-Bit\nBy HaPpY :)";
    static const Char*  TEXT_NOLOG =                "No log :P";

    // tables
    //-------------------------------------------------------------------------
    extern Settings::KeyValueCollection SETTINGS_ITEMS;
}

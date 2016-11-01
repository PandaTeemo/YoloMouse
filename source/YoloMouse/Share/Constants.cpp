#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    //------------------------------------------------------------------------
    const WCHAR* APP_MENU_STRINGS[MENU_OPTION_COUNT] =
    {
        L"Run as administrator",    //MENU_OPTION_RUNASADMIN
        L"Start with Windows",      //MENU_OPTION_AUTOSTART
        L"Open settings folder",    //MENU_OPTION_SETTINGS
        L"Errors",                  //MENU_OPTION_ERRORS
        L"About",                   //MENU_OPTION_ABOUT
    };

    const ULong CURSOR_SIZE_TABLE[CURSOR_SIZE_COUNT] =
    {
        0,
        16,
        24,
        32,
        48,
        64,
        80,
        96,
        112,
        128,
    };

    //------------------------------------------------------------------------
    static Settings::KeyValue _settings[] =
    {
        { "CursorKey1",         "CONTROL ALT 1" },
        { "CursorKey2",         "CONTROL ALT 2" },
        { "CursorKey3",         "CONTROL ALT 3" },
        { "CursorKey4",         "CONTROL ALT 4" },
        { "CursorKey5",         "CONTROL ALT 5" },
        { "CursorKey6",         "CONTROL ALT 6" },
        { "CursorKey7",         "CONTROL ALT 7" },
        { "CursorKey8",         "CONTROL ALT 8" },
        { "CursorKey9",         "CONTROL ALT 9" },
        { "CursorKeyReset",     "CONTROL ALT 0" },
        { "CursorKeySmaller",   "CONTROL ALT -" },
        { "CursorKeyLarger",    "CONTROL ALT =" },
        { "CursorKeyDefault",   "CONTROL ALT D" },
        { "AutoStart",          "1" },
        { "ShowMenu",           "1" },
    };
    Settings::KeyValueCollection SETTINGS_ITEMS(_settings, COUNT(_settings));
}

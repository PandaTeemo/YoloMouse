#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    //------------------------------------------------------------------------
    const WCHAR* APP_MENU_STRINGS[MENU_OPTION_COUNT] =
    {
        // info
        L"About",                   //MENU_OPTION_ABOUT
        L"Errors",                  //MENU_OPTION_ERRORS
        // settings
        L"Games only",              //MENU_OPTION_GAMESONLY
        L"Start with Windows",      //MENU_OPTION_AUTOSTART
        // etc
        L"Run as Administrator",    //MENU_OPTION_RUNASADMIN
        L"Open settings folder",    //MENU_OPTION_SETTINGSFOLDER
    };

    const ULong CURSOR_SIZE_TABLE[CURSOR_SIZE_INDEX_COUNT] =
    {
        0,
        16,
        24,
        32,
        40,
        48,
        56,
        64,
        72,
        80,
        88,
        96,
        104,
        112,
        120,
        128,
    };

    const ULong CURSOR_SIZE_TABLE_V_0_8_3[] =
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
    static Settings::KeyValue _settings[SETTING_COUNT] =
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
        { "GamesOnly",          "1" },
        { "AutoStart",          "1" },
        { "ShowMenu",           "1" },
    };
    Settings::KeyValueCollection SETTINGS_ITEMS(_settings, SETTING_COUNT);
}

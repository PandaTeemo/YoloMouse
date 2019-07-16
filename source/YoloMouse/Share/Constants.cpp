#include <YoloMouse/Share/Constants.hpp>

namespace Yolomouse
{
    //------------------------------------------------------------------------
    const WCHAR* APP_MENU_STRINGS[MENU_OPTION_COUNT] =
    {
        // info
        L"About",                   //MENU_OPTION_ABOUT
        L"Errors",                  //MENU_OPTION_ERRORS
        // settings
        //L"Games only",              //MENU_OPTION_GAMESONLY
        L"Start with Windows",      //MENU_OPTION_AUTOSTART
        // etc
        L"Run as Administrator",    //MENU_OPTION_RUNASADMIN
        L"Open settings folder",    //MENU_OPTION_SETTINGSFOLDER
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
        { "CursorKeyOverlay1",  "CONTROL SHIFT 1" },
        { "CursorKeyOverlay2",  "CONTROL SHIFT 2" },
        { "CursorKeyOverlay3",  "CONTROL SHIFT 3" },
        { "CursorKeyOverlay4",  "CONTROL SHIFT 4" },
        { "CursorKeyOverlay5",  "CONTROL SHIFT 5" },
        { "CursorKeyOverlay6",  "CONTROL SHIFT 6" },
        { "CursorKeyOverlay7",  "CONTROL SHIFT 7" },
        { "CursorKeyOverlay8",  "CONTROL SHIFT 8" },
        { "CursorKeyOverlay9",  "CONTROL SHIFT 9" },
        { "CursorKeyDefault",   "CONTROL ALT D" },
        { "CursorKeyReset",     "CONTROL ALT 0" },
        { "CursorKeySmaller",   "CONTROL ALT -" },
        { "CursorKeyLarger",    "CONTROL ALT =" },
        { "CursorKeyDefault2",  "CONTROL SHIFT D" },
        { "CursorKeyReset2",    "CONTROL SHIFT 0" },
        { "CursorKeySmaller2",  "CONTROL SHIFT -" },
        { "CursorKeyLarger2",   "CONTROL SHIFT =" },
        { "GamesOnly",          "1" },
        { "AutoStart",          "1" },
        { "ShowMenu",           "1" },
    };
    Settings::KeyValueCollection SETTINGS_ITEMS(_settings, SETTING_COUNT);
}

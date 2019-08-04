#include <YoloMouse/Share/Constants.hpp>

namespace Yolomouse
{
    //------------------------------------------------------------------------
    const WCHAR* APP_MENU_STRINGS[MENU_OPTION_COUNT] =
    {
        // info
        L"About",                       //MENU_OPTION_ABOUT
        L"Errors",                      //MENU_OPTION_ERRORS
        // settings
        L"Start with Windows",          //MENU_OPTION_AUTOSTART
        L"Reduce overlay pointer lag",  //MENU_OPTION_REDUCEOVERLAYLAG
        // etc
        L"Run as Administrator",        //MENU_OPTION_RUNASADMIN
        L"Open settings folder",        //MENU_OPTION_SETTINGSFOLDER
    };

    //------------------------------------------------------------------------
    static Settings::KeyValue _settings[SETTING_COUNT] =
    {
        { "CursorKeyBasic",     "CONTROL ALT 1" },
        { "CursorKeyOverlay",   "CONTROL ALT 2" },
        { "CursorKeyVariation", "CONTROL ALT 3" },
        { "CursorKeyDefault",   "CONTROL ALT D" },
        { "CursorKeyReset",     "CONTROL ALT 0" },
        { "CursorKeySmaller",   "CONTROL ALT -" },
        { "CursorKeyLarger",    "CONTROL ALT =" },
        { "AutoStart",          "1" },
        { "ReduceOverlayLag",   "0" },
        { "ShowMenu",           "1" },
    };
    Settings::KeyValueCollection SETTINGS_ITEMS(_settings, SETTING_COUNT);
}

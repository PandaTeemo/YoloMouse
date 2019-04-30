#pragma once
#include <YoloMouse/Share/Root.hpp>

namespace YoloMouse
{
    // enums
    //-------------------------------------------------------------------------
    enum NotifyId
    {
        NOTIFY_INIT,
        NOTIFY_UPDATEPRESET,
        NOTIFY_UPDATESIZE,
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
        MENU_OPTION_GAMESONLY,
        MENU_OPTION_AUTOSTART,
        // etc
        MENU_OPTION_RUNASADMIN,
        MENU_OPTION_SETTINGSFOLDER,

        MENU_OPTION_COUNT
    };

    enum ResourceType: Char
    {
        RESOURCE_PRESET =   'p',
        RESOURCE_IDENTITY = 'i',
        RESOURCE_UNKNOWN =  '?',
    };
}

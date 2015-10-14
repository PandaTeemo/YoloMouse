#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    //------------------------------------------------------------------------
    const WCHAR* APP_MENU_STRINGS[MENU_OPTION_COUNT] =
    {
        L"Start with Windows",
        L"About",
    };

    const WCHAR* PATH_CURSORS_SIZE[CURSOR_SIZE_COUNT] =
    {
        L"Tiny",
        L"Small",
        L"Medium",
        L"Large",
        L"Huge",
    };

    //------------------------------------------------------------------------
    static Settings::KeyValue _settings[] =
    {
        { "CursorKey1",         "CONTROL ALT SHIFT 1" },
        { "CursorKey2",         "CONTROL ALT SHIFT 2" },
        { "CursorKey3",         "CONTROL ALT SHIFT 3" },
        { "CursorKey4",         "CONTROL ALT SHIFT 4" },
        { "CursorKey5",         "CONTROL ALT SHIFT 5" },
        { "CursorKey6",         "CONTROL ALT SHIFT 6" },
        { "CursorKey7",         "CONTROL ALT SHIFT 7" },
        { "CursorKey8",         "CONTROL ALT SHIFT 8" },
        { "CursorKey9",         "CONTROL ALT SHIFT 9" },
        { "CursorKey10",        "CONTROL ALT SHIFT Q" },
        { "CursorKey11",        "CONTROL ALT SHIFT W" },
        { "CursorKey12",        "CONTROL ALT SHIFT E" },
        { "CursorKey13",        "CONTROL ALT SHIFT R" },
        { "CursorKey14",        "CONTROL ALT SHIFT T" },
        { "CursorKey15",        "CONTROL ALT SHIFT Y" },
        { "CursorKey16",        "CONTROL ALT SHIFT U" },
        { "CursorKey17",        "CONTROL ALT SHIFT I" },
        { "CursorKey18",        "CONTROL ALT SHIFT O" },
        { "CursorKey19",        "CONTROL ALT SHIFT P" },
        { "CursorKey20",        "CONTROL ALT SHIFT A" },
        { "CursorKey21",        "CONTROL ALT SHIFT S" },
        { "CursorKey22",        "CONTROL ALT SHIFT D" },
        { "CursorKey23",        "CONTROL ALT SHIFT F" },
        { "CursorKey24",        "CONTROL ALT SHIFT G" },
        { "CursorKey25",        "CONTROL ALT SHIFT H" },
        { "CursorKey26",        "CONTROL ALT SHIFT J" },
        { "CursorKey27",        "CONTROL ALT SHIFT K" },
        { "CursorKey28",        "CONTROL ALT SHIFT L" },
        { "CursorKey29",        "CONTROL ALT SHIFT Z" },
        { "CursorKey30",        "CONTROL ALT SHIFT X" },
        { "CursorKey31",        "CONTROL ALT SHIFT C" },
        { "CursorKey32",        "CONTROL ALT SHIFT V" },
        { "CursorKey33",        "CONTROL ALT SHIFT B" },
        { "CursorKey34",        "CONTROL ALT SHIFT N" },
        { "CursorKey35",        "CONTROL ALT SHIFT M" },
        { "CursorKeyReset",     "CONTROL ALT SHIFT 0" },
        { "CursorKeySmaller",   "CONTROL ALT SHIFT -" },
        { "CursorKeyLarger",    "CONTROL ALT SHIFT =" },
        { "CursorSize",         "2" },
        { "AutoStart",          "1" },
    };
    Settings::KeyValueCollection SETTINGS_ITEMS(_settings, COUNT(_settings));
}

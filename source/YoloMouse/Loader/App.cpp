#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Loader/App.hpp>
#include <YoloMouse/Loader/Resource/resource.h>
#include <YoloMouse/Share/Constants.hpp>

#include <psapi.h>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    App::App():
        _state          (SharedState::Instance()),
        _ui             (ShellUi::Instance()),
        _input_monitor  (_ui),
        _system_monitor (SystemMonitor::Instance()),
        _settings       (SETTINGS_ITEMS, PATH_SETTINGS)
    {
        // init ui
        _ui.SetName(APP_NAME);
        _ui.SetIcon(IDI_ICONAPP);
    }

    //-------------------------------------------------------------------------
    void App::Start()
    {
        // start settings
        _StartSettings();

        // start shared state
        _StartState();

        // start loader
        _StartLoader();

        // start ui
        _StartUi();

        // start options
        _StartOptions();

        // start system monitoring
        _StartSystem();

        // start input monitoring
        _StartInput();
    }

    void App::Stop()
    {
        // stop input monitoring
        _StopInput();

        // stop system monitoring
        _StopSystem();

        // stop options
        _StartOptions();

        // stop ui
        _StopUi();

        // stop loader
        _StopLoader();

        // stop state
        _StopState();

        // stop settings
        _StopSettings();
    }

    //-------------------------------------------------------------------------
    void App::Run()
    {
        // run ui
        ShellUi::Instance().Run();
    }

    // private
    //-------------------------------------------------------------------------
    void App::_StartInput()
    {
        const Settings::KeyValueCollection& kvs = _settings.GetCollection();

        // register events
        _input_monitor.SetListener(this);

        // for each cursor key
        for( Id id = SETTING_CURSORKEY_1; id <= SETTING_CURSORKEY_LARGE; id++ )
        {
            // create combo
            _input_monitor.CreateCombo(id, _settings.Get(id));
        }

        // start input monitor
        eggs(_input_monitor.Start());
    }

    void App::_StartLoader()
    {
        // create target save directory
        CreateDirectory(PATH_SAVE, NULL);

        // start loader
        eggs(_loader.Start());
    }

    void App::_StartOptions()
    {
        // enable autostart
        if( _settings.GetBoolean(SETTING_AUTOSTART) )
            _OptionAutoStart(true);

        // set cursor size
        _OptionCursorSize(_settings.GetNumber(SETTING_CURSORSIZE));
    }

    void App::_StartSettings()
    {
        // load settings
        _settings.Load();
    }

    void App::_StartState()
    {
        Char path[STRING_PATH_SIZE];

        // open shared cursor table as host
        eggs(_state.Open(true));

        // set path
        eggs(GetCurrentDirectory(sizeof(path), path) != 0);
        _state.SetPath(path);
    }

    void App::_StartSystem()
    {
        // register events
        _system_monitor.SetListener(this);

        // start monitors
        eggs(_system_monitor.Start());
    }

    void App::_StartUi()
    {
        // start ui
        eggs(_ui.Start());

        // if showmenu enabled
        if( _settings.GetBoolean(SETTING_SHOWMENU) )
        {
            // get cursor size setting
            Long size = _settings.GetNumber(SETTING_CURSORSIZE);

            // add menu
            _ui.AddMenu();

            // add menu options
            _ui.AddMenuBreak();
            _ui.AddMenuOption(MENU_OPTION_AUTOSTART,    APP_MENU_STRINGS[MENU_OPTION_AUTOSTART],   _settings.GetBoolean(SETTING_AUTOSTART));
            _ui.AddMenuOption(MENU_OPTION_SHOWMENU,     APP_MENU_STRINGS[MENU_OPTION_SHOWMENU],    true);
            _ui.AddMenuBreak();
            _ui.AddMenuOption(MENU_OPTION_SIZE_SMALL,   APP_MENU_STRINGS[MENU_OPTION_SIZE_SMALL],  size == CURSOR_SIZE_SMALL);
            _ui.AddMenuOption(MENU_OPTION_SIZE_MEDIUM,  APP_MENU_STRINGS[MENU_OPTION_SIZE_MEDIUM], size == CURSOR_SIZE_MEDIUM);
            _ui.AddMenuOption(MENU_OPTION_SIZE_LARGE,   APP_MENU_STRINGS[MENU_OPTION_SIZE_LARGE],  size == CURSOR_SIZE_LARGE);
        }

        // register events
        _ui.AddListener(*this);
    }

    //-------------------------------------------------------------------------
    void App::_StopInput()
    {
        // clear events
        _input_monitor.SetListener();

        // stop system events
        _input_monitor.Stop();
    }

    void App::_StopLoader()
    {
        _loader.Stop();
    }

    void App::_StopOptions()
    {
    }

    void App::_StopSettings()
    {
    }

    void App::_StopState()
    {
        // close shared cursor table
        _state.Close();
    }

    void App::_StopSystem()
    {
        // clear events
        _system_monitor.SetListener();

        // stop system events
        _system_monitor.Stop();
    }

    void App::_StopUi()
    {
        // clear events
        _ui.RemoveListener(*this);

        // stop ui
        _ui.Stop();
    }

    //-------------------------------------------------------------------------
    Bool App::_OptionAutoStart( Bool enable )
    {
        Char path[STRING_PATH_SIZE];

        // get exec path
        if(GetFullPathName(PATH_LOADER, sizeof(path), path, NULL))
        {
            // update settings
            _settings.SetBoolean(SETTING_AUTOSTART, enable);
            _settings.Save();

            // enable or disable autostart
            SystemTools::EnableAutoStart( APP_NAME, path, enable );

            return true;
        }

        return false;
    }

    Bool App::_OptionHideMenu()
    {
        // update settings
        _settings.SetBoolean(SETTING_SHOWMENU, false);
        _settings.Save();

        // hide menu
        _ui.HideMenu();

        return true;
    }

    Bool App::_OptionCursorSize( ULong size )
    {
        // check
        if( size >= CURSOR_SIZE_COUNT )
            return false;

        // update cursor size
        _settings.SetNumber(SETTING_CURSORSIZE, size);
        _state.SetCursorSize(static_cast<CursorSize>(size));

        // get target window
        HWND hwnd = Loader::GetActiveTarget();
        if( hwnd )
        {
            // require target is loaded
            if(_loader.IsLoaded(hwnd))
            {
                // notify target to refresh
                _loader.NotifyRefresh(hwnd);
            }
        }

        return true;
    }

    //-------------------------------------------------------------------------
    Bool App::_ReplaceCursor( Index cursor_index )
    {
        // get target window
        HWND hwnd = Loader::GetActiveTarget();
        if( hwnd == NULL )
            return false;

        // load target if not already loaded
        if(!_loader.IsLoaded(hwnd) && !_loader.Load(hwnd))
            return false;

        // notify target
        _loader.NotifyAssign(hwnd, cursor_index);

        return true;
    }

    //-------------------------------------------------------------------------
    void App::OnKeyCombo( Id combo_id )
    {
        // change cursor
        if( combo_id >= SETTING_CURSORKEY_1 && combo_id <= SETTING_CURSORKEY_9 )
            _ReplaceCursor(combo_id - SETTING_CURSORKEY_1);
        // reset cursor
        else if( combo_id == SETTING_CURSORKEY_RESET )
            _ReplaceCursor(INVALID_INDEX);
        // change size
        else if( combo_id >= SETTING_CURSORKEY_SMALL && combo_id <= SETTING_CURSORKEY_LARGE )
            _OptionCursorSize(combo_id - SETTING_CURSORKEY_SMALL);
    }

    //-------------------------------------------------------------------------
    void App::OnWindowCreate( HWND hwnd )
    {
    }

    void App::OnWindowDestroy( HWND hwnd )
    {
        // unload
        _loader.Unload(hwnd);
    }

    void App::OnWindowFocus( HWND hwnd )
    {
        // if already loaded
        if( _loader.IsLoaded(hwnd) )
        {
            // notify target to refresh
            _loader.NotifyRefresh(hwnd);
        }
        else
        {
            // load target if configured for yolomouse
            if( _loader.IsConfigured(hwnd) )
                _loader.Load(hwnd);
        }
    }

    //-------------------------------------------------------------------------
    Bool App::OnMenuOption( Id id, Bool enabled )
    {
        switch(id)
        {
        // auto start
        case MENU_OPTION_AUTOSTART:
            // toggle auto start
            if(_OptionAutoStart(!enabled))
                _ui.SetMenuOption(id, !enabled);
            return true;

        // show menu
        case MENU_OPTION_SHOWMENU:
            if(enabled)
            {
                _OptionHideMenu();
                _ui.SetMenuOption(id, false);
            }
            return true;

        // cursor size
        case MENU_OPTION_SIZE_SMALL:
        case MENU_OPTION_SIZE_MEDIUM:
        case MENU_OPTION_SIZE_LARGE:
            _OptionCursorSize(id - MENU_OPTION_SIZE_SMALL);
            _ui.SetMenuOption(MENU_OPTION_SIZE_SMALL, id == MENU_OPTION_SIZE_SMALL);
            _ui.SetMenuOption(MENU_OPTION_SIZE_MEDIUM, id == MENU_OPTION_SIZE_MEDIUM);
            _ui.SetMenuOption(MENU_OPTION_SIZE_LARGE, id == MENU_OPTION_SIZE_LARGE);
            return true;

        default:
            return false;
        }
    }
}

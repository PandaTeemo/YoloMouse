#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Loader/App.hpp>
#include <YoloMouse/Loader/Resource/resource.h>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <io.h>
#include <Shlobj.h>
#include <stdio.h>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    App::App():
        _state          (SharedState::Instance()),
        _ui             (ShellUi::Instance()),
        _input_monitor  (_ui),
        _system_monitor (SystemMonitor::Instance()),
        _settings       (SETTINGS_ITEMS),
        _elevate        (false),
        _games_only     (false)
    {
        // init ui
        _ui.SetName(APP_NAME);
        _ui.SetIcon(IDI_ICONAPP);
    }

    //-------------------------------------------------------------------------
    Bool App::GetElevate() const
    {
        return _elevate;
    }

    //-------------------------------------------------------------------------
    void App::Start()
    {
        // start shared state
        _StartState();

        // start settings
        _StartSettings();

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
        _StopOptions();

        // stop ui
        _StopUi();

        // stop settings
        _StopSettings();

        // stop shared state
        _StopState();
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
        for( Id id = SETTING_GROUPKEY_1; id <= SETTING_SIZEKEY_LARGER; id++ )
        {
            String format = _settings.Get(id);

            // create combo
            if( !_input_monitor.CreateCombo(id, format) )
                elog("App.StartInput.CreateCombo: %d:%s", id, format.GetZ());
        }

        // start input monitor
        eggs(_input_monitor.Start());
    }

    void App::_StartOptions()
    {
        // enable autostart
        if( _settings.GetBoolean(SETTING_AUTOSTART) )
            _OptionAutoStart(true, false);

        // update "games only" option
        _OptionGamesOnly(_settings.GetBoolean(SETTING_GAMESONLY), false);
    }

    void App::_StartSettings()
    {
        PathString settings_path;

        // set settings path
        if( SharedTools::BuildUserPath(settings_path, COUNT(settings_path), PATH_SETTINGS_NAME, EXTENSION_INI, NULL) )
            _settings.SetPath(settings_path);
        else
            elog("App.StartSettings.BuildUserPath: %s", Tools::WToCString(PATH_SETTINGS_NAME));

        // load settings
        if( !_settings.Load() )
            elog("App.StartSettings.Load: %s", Tools::WToCString(settings_path));
    }

    void App::_StartState()
    {
        // open shared cursor table as host
        eggs(_state.Open(true));

        // update state path
        eggs(GetCurrentDirectory(sizeof(PathString), _state.EditPath()) > 0);
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
            // add menu
            _ui.AddMenu();

            // add menu break
            _ui.AddMenuBreak();
            // add show settings
            _ui.AddMenuOption(MENU_OPTION_SETTINGSFOLDER, APP_MENU_STRINGS[MENU_OPTION_SETTINGSFOLDER], false);
            // add run-as-administrator option if not already admin
            if( !IsUserAnAdmin() )
                _ui.AddMenuOption(MENU_OPTION_RUNASADMIN, APP_MENU_STRINGS[MENU_OPTION_RUNASADMIN], false);

            // add menu break
            _ui.AddMenuBreak();
            // add autostart option
            _ui.AddMenuOption(MENU_OPTION_AUTOSTART, APP_MENU_STRINGS[MENU_OPTION_AUTOSTART], _settings.GetBoolean(SETTING_AUTOSTART));
            // add games only option
            _ui.AddMenuOption(MENU_OPTION_GAMESONLY, APP_MENU_STRINGS[MENU_OPTION_GAMESONLY], _settings.GetBoolean(SETTING_GAMESONLY));

            // add menu break
            _ui.AddMenuBreak();
            // add debug log
            _ui.AddMenuOption(MENU_OPTION_ERRORS, APP_MENU_STRINGS[MENU_OPTION_ERRORS], false);
            // add about dialog
            _ui.AddMenuOption(MENU_OPTION_ABOUT, APP_MENU_STRINGS[MENU_OPTION_ABOUT], false);
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
    void App::_OptionAbout()
    {
        MediumString about_text;

        // generate about text
        about_text.Format( TEXT_ABOUT,
            APP_VERSION[0],         // version major
            APP_VERSION[1],         // version minor
            APP_VERSION[2],         // version patch
            CPU_64 ? 64 : 32    // bitness
        );

        // show about dialog
        SharedTools::MessagePopup(false, about_text.GetMemory());
    }

    void App::_OptionErrors()
    {
        FILE*       file;
        PathString  error_path;
        SharedLog&  log = SharedState::Instance().GetLog();

        // if empty
        if( log.IsEmpty() )
        {
            SharedTools::MessagePopup(false, TEXT_NOERRORS);
            return;
        }

        // get error path
        if( !SharedTools::BuildUserPath(error_path, COUNT(error_path), PATH_ERRORS, EXTENSION_LOG, NULL) )
            return;

        // open file
        if( _wfopen_s(&file, error_path, L"wt") == 0 )
        {
            SharedLog&  log = SharedState::Instance().GetLog();
            Index       index = INVALID_INDEX;
            MaxString   line;

            // read memory log and write to file
            while( log.Read(index, line) )
                fprintf(file, "%s\n", line.GetZ());

            // close file
            fclose(file);

            // open in default text editor
            ShellExecute(NULL, L"open", error_path, L"", NULL, SW_SHOWNORMAL);
        }
    }

    void App::_OptionGamesOnly( Bool enable, Bool save )
    {
        // update games only state
        _games_only = enable;

        // update settings
        if( save )
        {
            _settings.SetBoolean(SETTING_GAMESONLY, enable);
            if( !_settings.Save() )
                elog("App.OptionGamesOnly.Save");
        }
    }

    Bool App::_OptionAutoStart( Bool enable, Bool save )
    {
        PathString path;

        // get exec path
        if(GetFullPathName(PATH_LOADER, COUNT(path), path, NULL))
        {
            // update settings
            if( save )
            {
                _settings.SetBoolean(SETTING_AUTOSTART, enable);
                if( !_settings.Save() )
                    elog("App.OptionAutoStart.Save");
            }

            // enable or disable autostart
            if( !SystemTools::EnableAutoStart( APP_NAME, path, enable ) )
            {
                elog("App.OptionAutoStart.EnableAutoStart: %s", Tools::WToCString(path));
                return false;
            }

            return true;
        }
        else
            elog("App.OptionAutoStart.GetFullPathName");

        return false;
    }

    Bool App::_OptionRunAsAdmin()
    {
        // exit current process
        ShellUi::Instance().Exit();

        // set elevate state
        _elevate = true;

        return true;
    }

    void App::_OptionSettingsFolder()
    {
        PathString settings_path;

        // get settings path
        if( !SharedTools::BuildUserPath(settings_path, COUNT(settings_path), NULL, NULL, NULL) )
            return;

        // open in default text editor
        ShellExecute(NULL, L"open", settings_path, L"", NULL, SW_SHOWNORMAL);
    }

    //-------------------------------------------------------------------------
    Bool App::_AssignCursor( Index group_index )
    {
        // access active instance
        Instance* instance = _AccessCurrentInstance();
        if( instance == NULL )
            return false;

        // notify instance to assign
        return instance->Notify(NOTIFY_UPDATEPRESET, group_index);
    }

    Bool App::_AssignSize( Long size_index_delta )
    {
        // access active instance
        Instance* instance = _AccessCurrentInstance();
        if( instance == NULL )
            return false;

        // notify instance to assign
        return instance->Notify(NOTIFY_UPDATESIZE, size_index_delta);
    }

    //-------------------------------------------------------------------------
    Instance* App::_AccessCurrentInstance()
    {
        DWORD process_id = 0;

        // get focus window
        HWND hwnd = SystemTools::GetFocusWindow();
        if( hwnd == NULL )
        {
            elog("App.AccessCurrentInstance.GetFocusWindow");
            return NULL;
        }

        // get its process id
        if( GetWindowThreadProcessId(hwnd, &process_id) == 0 )
        {
            elog("App.AccessCurrentInstance.GetWindowThreadProcessId");
            return NULL;
        }

        // find instance if one exists
        Instance* instance = _instance_manager.Find(process_id);
        if( instance == NULL )
        {
            // if games only option enabled
            if( _games_only )
            {
                // fail if game window test fails
                if( !SystemTools::TestGameWindow( hwnd ) )
                {
                    elog("App.AccessCurrentInstance.TestGameWindow");
                    return NULL;
                }
            }

            // allocate new instance
            instance = _instance_manager.Load(process_id);
        }

        return instance;
    }

    //-------------------------------------------------------------------------
    void App::OnKeyCombo( Id combo_id )
    {
        // change cursor
        if( combo_id >= SETTING_GROUPKEY_1 && combo_id <= SETTING_GROUPKEY_9 )
            _AssignCursor(combo_id - SETTING_GROUPKEY_1);
        // reset cursor
        else if( combo_id == SETTING_GROUPKEY_RESET )
            _AssignCursor(CURSOR_SPECIAL_REMOVE);
        // change size
        else if( combo_id >= SETTING_SIZEKEY_SMALLER && combo_id <= SETTING_SIZEKEY_LARGER )
            _AssignSize(combo_id == SETTING_SIZEKEY_SMALLER ? -1 : 1);
    }

    //-------------------------------------------------------------------------
    void App::OnWindowFocus( HWND hwnd )
    {
        try
        {
            DWORD process_id = 0;

            // get process id of window
            if( GetWindowThreadProcessId(hwnd, &process_id) )
            {
                // find instance
                Instance* instance = _instance_manager.Find(process_id);
                if( instance != NULL )
                {
                    // notify target to refresh
                    instance->Notify(NOTIFY_REFRESH);
                }
                else
                {
                    // load instance if configured for yolomouse
                    if( _instance_manager.IsConfigured(process_id) )
                        _instance_manager.Load(process_id);
                }
            }
        }
        // catch eggs
        catch( const Char* error )
        {
            SharedTools::MessagePopup(true, error);
        }
    }

    //-------------------------------------------------------------------------
    Bool App::OnMenuOption( Id id, Bool enabled )
    {
        switch(id)
        {
        // show menu
        case MENU_OPTION_ABOUT:
            _OptionAbout();
            return true;

        // show debug log
        case MENU_OPTION_ERRORS:
            _OptionErrors();
            return true;

        // games only
        case MENU_OPTION_GAMESONLY:
            // toggle games only
            _OptionGamesOnly( !enabled, true );
            _ui.SetMenuOption(MENU_OPTION_GAMESONLY, !enabled);
            return true;

        // auto start
        case MENU_OPTION_AUTOSTART:
            // toggle auto start
            if( _OptionAutoStart(!enabled, true) )
                _ui.SetMenuOption(MENU_OPTION_AUTOSTART, !enabled);
            return true;

        // run as administrator
        case MENU_OPTION_RUNASADMIN:
            _OptionRunAsAdmin();
            return true;

        // open settings folder
        case MENU_OPTION_SETTINGSFOLDER:
            _OptionSettingsFolder();
            return true;

        default:
            return false;
        }
    }
}

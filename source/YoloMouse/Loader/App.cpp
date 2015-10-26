#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Loader/App.hpp>
#include <YoloMouse/Loader/Resource/resource.h>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

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
        _settings       (SETTINGS_ITEMS)
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

        // stop shared state
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
        for( Id id = SETTING_GROUPKEY_1; id <= SETTING_SIZEKEY_LARGER; id++ )
        {
            // create combo
            _input_monitor.CreateCombo(id, _settings.Get(id));
        }

        // start input monitor
        eggs(_input_monitor.Start());
    }

    void App::_StartOptions()
    {
        // enable autostart
        if( _settings.GetBoolean(SETTING_AUTOSTART) )
            _OptionAutoStart(true, false);
    }

    void App::_StartSettings()
    {
        PathString settings_path;

        // set settings path
        if(SharedTools::BuildSavePath(settings_path, COUNT(settings_path), PATH_SETTINGS_NAME))
            _settings.SetPath(settings_path);

        // load settings
        _settings.Load();
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

        // add menu
        _ui.AddMenu();

        // add menu options
        _ui.AddMenuBreak();
        _ui.AddMenuOption(MENU_OPTION_AUTOSTART,    APP_MENU_STRINGS[MENU_OPTION_AUTOSTART],    _settings.GetBoolean(SETTING_AUTOSTART));
        _ui.AddMenuOption(MENU_OPTION_ABOUT,        APP_MENU_STRINGS[MENU_OPTION_ABOUT],        false);

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
                _settings.Save();
            }

            // enable or disable autostart
            SystemTools::EnableAutoStart( APP_NAME, path, enable );

            return true;
        }

        return false;
    }

    //-------------------------------------------------------------------------
    Bool App::_AssignCursor( Index group_index )
    {
        // access active instance
        Instance* instance = _AccessCurrentInstance();
        if( instance == NULL )
            return false;

        // notify instance to assign
        return instance->Notify(NOTIFY_SETCURSOR, group_index);
    }

    Bool App::_AssignSize( Long size_index_delta )
    {
        // access active instance
        Instance* instance = _AccessCurrentInstance();
        if( instance == NULL )
            return false;

        // notify instance to assign
        return instance->Notify(NOTIFY_SETSIZE, size_index_delta);
    }

    //-------------------------------------------------------------------------
    Instance* App::_AccessCurrentInstance()
    {
        DWORD process_id = 0;

        // get active window
        HWND hwnd = GetForegroundWindow();
        if( hwnd == NULL )
            return NULL;

        // get its process id
        if( GetWindowThreadProcessId(hwnd, &process_id) == 0 )
            return NULL;

        // find instance if one exists
        Instance* instance = _instance_manager.Find(process_id);
        if( instance == NULL )
        {
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
            SharedTools::ErrorMessage(error);
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
            if(_OptionAutoStart(!enabled, true))
                _ui.SetMenuOption(id, !enabled);
            return true;

        // show menu
        case MENU_OPTION_ABOUT:
            MessageBoxA(NULL, APP_ABOUT, APP_NAMEC, MB_OK|MB_ICONINFORMATION);
            return true;

        default:
            return false;
        }
    }
}

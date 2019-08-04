#include <Core/System/Debug.hpp>
#include <Core/System/SystemTools.hpp>
#include <Core/Windows/WindowTools.hpp>
#include <Core/Windows/SystemMonitor.hpp>
#include <YoloMouse/Loader/Core/App.hpp>
#include <YoloMouse/Loader/Resource/resource.h>
#include <YoloMouse/Loader/Target/TargetController.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <io.h>
#include <Shlobj.h>
#include <stdio.h>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    App::App():
        _elevate            (false),
        _initialized        (false),
        _ui                 (ShellUi::Instance()),
        _input_monitor      (_ui),
        _settings           (SETTINGS_ITEMS)
    {
        _host_path.Zero();
        _user_path.Zero();
    }

    App::~App()
    {
    }

    //-------------------------------------------------------------------------
    Bool App::Initialize( HINSTANCE hinstance )
    {
        // init ui
        _ui.SetName(APP_NAME);
        _ui.SetIcon(IDI_ICONAPP);

        // initialize paths
        if( !_InitializePaths() )
            LOG("App.Initialize.Paths");
        else
        {
            // initialize debug
            Debug::Initialize( _log_path, true );

            // initialize system monitor
            if( !SystemMonitor::Instance().Initialize() )
                LOG("TargetController.Initialize.SystemMonitor");

            // initialize settings
            else if( !_InitializeSettings() )
                LOG( "App.Initialize.Settings" );

            // initialize ui
            else if( !_InitializeUi() )
                LOG( "App.Initialize.Ui" );

            // initialize options
            else if( !_InitializeOptions() )
                LOG( "App.Initialize.Options" );

            // initialize input monitoring
            else if( !_InitializeInput() )
                LOG( "App.Initialize.Input" );

            // initialize overlay
            else if( !_InitializeOverlay(hinstance) )
                LOG( "App.Initialize.Overlay" );

            // initialize target controller
            else if( !TargetController::Instance().Initialize() )
                LOG( "App.Initialize.TargetController" );

            // success
            else
            {
                // set initialized
                _initialized = true;
                return true;
            }
        }

        // undo
        Shutdown();

        return false;
    }

    void App::Shutdown()
    {
        // if system monitor initialized
        if( SystemMonitor::Instance().IsInitialized() )
        {
            // if ui started
            if( _ui.IsStarted() )
            {
                // if input monitoring started
                if( _input_monitor.IsStarted() )
                {
                    // if overlay initialized
                    if( Overlay::Instance().IsInitialized() )
                    {
                        // if target controller initialized
                        if( TargetController::Instance().IsInitialized() )
                        {
                            // shutdown target controller
                            TargetController::Instance().Shutdown();

                            // reset initialized
                            _initialized = false;
                        }

                        // shutdown overlay
                        _ShutdownOverlay();
                    }

                    // shutdown input monitoring
                    _ShutdownInput();
                }

                // shutdown ui
                _ShutdownUi();
            }

            // shutdown system monitor
            SystemMonitor::Instance().Shutdown();
        }

        // shutdown debug
        Debug::Shutdown();
    }

    //-------------------------------------------------------------------------
    Bool App::IsInitialized() const
    {
        return _initialized;
    }

    Bool App::IsElevated() const
    {
        return IsUserAnAdmin() == TRUE;
    }

    //-------------------------------------------------------------------------
    Bool App::GetElevate() const
    {
        return _elevate;
    }

    const PathString& App::GetHostPath() const
    {
        return _host_path;
    }

    const PathString& App::GetUserPath() const
    {
        return _user_path;
    }

    const PathString& App::GetLogPath() const
    {
        return _log_path;
    }

    // impl
    //-------------------------------------------------------------------------
    void App::OnKeyCombo( Id combo_id )
    {
        Target* target;

        // access current target
        if( TargetController::Instance().AccessTarget( target, IsElevated() ) )
        {
            // handle by combo id
            switch (combo_id)
            {
            // set/increment basic cursor
            case SETTING_CURSORKEY_BASIC:
                target->SetCursor({CURSOR_TYPE_BASIC}, CURSOR_UPDATE_INCREMENT_ID);
                break;
            // set/increment overlay cursor
            case SETTING_CURSORKEY_OVERLAY:
                target->SetCursor({CURSOR_TYPE_OVERLAY}, CURSOR_UPDATE_INCREMENT_ID);
                break;
            // increment current cursor variation
            case SETTING_CURSORKEY_VARIATION:
                target->SetCursor({}, CURSOR_UPDATE_INCREMENT_VARIATION);
                break;
            // reset cursor
            case SETTING_CURSORKEY_RESET:
                target->ResetCursor();
                break;
            // set default cursor
            case SETTING_CURSORKEY_DEFAULT:
                target->SetDefaultCursor();
                break;
            // decrement size
            case SETTING_CURSORKEY_SMALLER:
                target->SetCursor({}, CURSOR_UPDATE_DECREMENT_SIZE);
                break;
            // increment size
            case SETTING_CURSORKEY_LARGER:
                target->SetCursor({}, CURSOR_UPDATE_INCREMENT_SIZE);
                break;
            default:;
            }
        }
        else
            LOG("App.OnKeyCombo.AccessTarget");
    }

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

        // auto start
        case MENU_OPTION_AUTOSTART:
            // toggle auto start
            if( _OptionAutoStart(!enabled, true) )
                _ui.SetMenuOption(MENU_OPTION_AUTOSTART, !enabled);
            return true;

        // reduce overlay lag
        case MENU_OPTION_REDUCEOVERLAYLAG:
            // toggle reduce overlay lag
            _OptionReduceOverlayLag( !enabled, true );
            _ui.SetMenuOption(MENU_OPTION_REDUCEOVERLAYLAG, !enabled);
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

    // private
    //-------------------------------------------------------------------------
    Bool App::_InitializePaths()
    {
        // get current yolomouse path
        if( GetCurrentDirectory( _host_path.GetLimit(), _host_path.EditMemory() ) == 0 )
            return false;

        // get/create user path
        if( !SystemTools::AccessUserPath( _user_path, APP_NAME ) )
            return false;

        // create log file path
        if( swprintf_s( _log_path.EditMemory(), _log_path.GetLimit(), L"%s\\%s", _user_path.GetMemory(), PATH_LOG_NAME ) <= 0 )
            return false;

        return true;
    }

    Bool App::_InitializeSettings()
    {
        PathString settings_path;

        // create settings file path
        if( swprintf_s( settings_path.EditMemory(), settings_path.GetLimit(), L"%s\\%s", _user_path.GetMemory(), PATH_SETTINGS ) <= 0 )
            return false;

        // load settings
        _settings.SetPath(settings_path);
        if( !_settings.Load() )
        {
            LOG("App.StartSettings.Load: %s", Tools::WToCString(settings_path.GetMemory()));
            return false;
        }

        return true;
    }

    Bool App::_InitializeUi()
    {
        // start ui
        if( !_ui.Start() )
            return false;

        // if showmenu enabled
        if( _settings.GetBoolean(SETTING_SHOWMENU) )
        {
            // add menu
            _ui.AddMenu();

            // add menu break
            _ui.AddMenuBreak();
            // add show settings
            _ui.AddMenuOption(MENU_OPTION_SETTINGSFOLDER, APP_MENU_STRINGS[MENU_OPTION_SETTINGSFOLDER], false);
            // add run-as-administrator option if not already elevated/admin
            if( !IsElevated() )
                _ui.AddMenuOption(MENU_OPTION_RUNASADMIN, APP_MENU_STRINGS[MENU_OPTION_RUNASADMIN], false);

            // add menu break
            _ui.AddMenuBreak();
            // add reduce overlay lag option
            _ui.AddMenuOption(MENU_OPTION_REDUCEOVERLAYLAG, APP_MENU_STRINGS[MENU_OPTION_REDUCEOVERLAYLAG], _settings.GetBoolean(SETTING_REDUCEOVERLAYLAG));
            // add autostart option
            _ui.AddMenuOption(MENU_OPTION_AUTOSTART, APP_MENU_STRINGS[MENU_OPTION_AUTOSTART], _settings.GetBoolean(SETTING_AUTOSTART));

            // add menu break
            _ui.AddMenuBreak();
            // add debug log
            _ui.AddMenuOption(MENU_OPTION_ERRORS, APP_MENU_STRINGS[MENU_OPTION_ERRORS], false);
            // add about dialog
            _ui.AddMenuOption(MENU_OPTION_ABOUT, APP_MENU_STRINGS[MENU_OPTION_ABOUT], false);
        }

        // register events
        _ui.AddListener(*this);

        return true;
    }

    Bool App::_InitializeInput()
    {
        const Settings::KeyValueCollection& kvs = _settings.GetCollection();

        // for each cursor key
        for( Id id = SETTING_CURSORKEY_BASIC; id <= SETTING_CURSORKEY_LARGER; id++ )
        {
            String format = _settings.Get(id);

            // create combo
            if( !_input_monitor.CreateCombo(id, format) )
                LOG("App.StartInput.CreateCombo: %d:%s", id, format.GetZ());
        }

        // start input monitor
        if( !_input_monitor.Start(*this) )
            return false;

        return true;
    }

    Bool App::_InitializeOptions()
    {
        // enable autostart
        if( _settings.GetBoolean(SETTING_AUTOSTART) )
            _OptionAutoStart(true, false);

        // update "reduce overlay lag" option
        _OptionReduceOverlayLag(_settings.GetBoolean(SETTING_REDUCEOVERLAYLAG), false);

        return true;
    }

    Bool App::_InitializeOverlay( HINSTANCE hinstance )
    {
        // get overlay
        Overlay& overlay = Overlay::Instance();

        // initialize overlay
        if( !overlay.Initialize( hinstance ) )
            LOG( "App.InitializeOverlay.Initialize" );
        else
        {
            // initialize overlay cursors
            _overlay_cursor_vault.Initialize();

            // install overlay cursors
            _InstallOverlayCursors();

            // start overlay
            if( !overlay.Start() )
                LOG( "App.InitializeOverlay.Start" );
            // success
            else
                return true;
        }

        // undo
        _ShutdownOverlay();

        return false;
    }

    void App::_InstallOverlayCursors()
    {
        // get overlay
        Overlay& overlay = Overlay::Instance();
        const OverlayCursorVault::CursorTable& cursors = _overlay_cursor_vault.GetCursors();

        // add cursors to overlay
        for( Index cursor_id = 0; cursor_id < cursors.GetCount(); ++cursor_id )
        {
            IOverlayCursor* cursor = cursors[cursor_id];
            if( cursor != nullptr )
                overlay.InstallCursor( static_cast<CursorId>(cursor_id), *cursor );
        }
    }

    //-------------------------------------------------------------------------
    void App::_ShutdownUi()
    {
        // clear events
        _ui.RemoveListener(*this);

        // stop ui
        _ui.Stop();
    }

    void App::_ShutdownInput()
    {
        // stop system events
        _input_monitor.Stop();
    }

    void App::_ShutdownOverlay()
    {
        // get overlay
        Overlay& overlay = Overlay::Instance();

        // if overlay initialized
        if( overlay.IsInitialized() )
        {
            // if overlay cursors initialized
            if( _overlay_cursor_vault.IsInitialized() )
            {
                // if overlay started
                if( overlay.IsStarted() )
                    overlay.Stop();

                // uninstall overlay cursors
                _UninstallOverlayCursors();

                // shutdown overlay cursors
                _overlay_cursor_vault.Shutdown();
            }

            // shutdown overlay
            overlay.Shutdown();
        }
    }

    void App::_UninstallOverlayCursors()
    {
        // get overlay
        Overlay& overlay = Overlay::Instance();
        const OverlayCursorVault::CursorTable& cursors = _overlay_cursor_vault.GetCursors();

        // add cursors to overlay
        for( Index cursor_id = 0; cursor_id < cursors.GetCount(); ++cursor_id )
        {
            IOverlayCursor* cursor = cursors[cursor_id];
            if( cursor != nullptr )
                overlay.UninstallCursor( static_cast<CursorId>(cursor_id) );
        }
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
        WindowTools::MessagePopup(APP_NAMEC, false, about_text.GetMemory());
    }

    void App::_OptionErrors()
    {
        // if log exists
        if( Tools::DoesFileExist( _log_path.GetMemory() ) )
        {
            // open in default text editor
            ShellExecute(NULL, L"open", _log_path.GetMemory(), L"", NULL, SW_SHOWNORMAL);
        }
        // else show message about no log file existing
        else
            WindowTools::MessagePopup(APP_NAMEC, false, TEXT_NOLOG);
    }

    Bool App::_OptionAutoStart( Bool enable, Bool save )
    {
        PathString path;

        // get exec path
        if(GetFullPathName(PATH_LOADER, path.GetLimit(), path.EditMemory(), NULL))
        {
            // update settings
            if( save )
            {
                _settings.SetBoolean(SETTING_AUTOSTART, enable);
                if( !_settings.Save() )
                    LOG("App.OptionAutoStart.Save");
            }

            // enable or disable autostart
            if( !SystemTools::EnableAutoStart( APP_NAME, path, enable ) )
            {
                LOG("App.OptionAutoStart.EnableAutoStart: %s", Tools::WToCString(path.GetMemory()));
                return false;
            }

            return true;
        }
        else
            LOG("App.OptionAutoStart.GetFullPathName");

        return false;
    }

    void App::_OptionReduceOverlayLag( Bool enable, Bool save )
    {
        // update games only state
        Overlay::Instance().SetReduceLatency(enable);

        // update settings
        if( save )
        {
            _settings.SetBoolean(SETTING_REDUCEOVERLAYLAG, enable);
            if( !_settings.Save() )
                LOG("App.ReduceOverlayLag.Save");
        }
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
        // open user path in file explorer
        ShellExecute(NULL, L"open", _user_path.GetMemory(), L"", NULL, SW_SHOWNORMAL);
    }
}

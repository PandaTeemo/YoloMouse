#pragma once
#include <Core/Support/Settings.hpp>
#include <Core/Support/Singleton.hpp>
#include <Core/Windows/InputMonitor.hpp>
#include <Core/Windows/ShellUi.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/OverlayCursorVault.hpp>

namespace Yolomouse
{
    /**/
    class App:
        public Singleton<App>,
        public InputMonitor::IListener,
        public ShellUi::IListener
    {
    public:
        /**/
        App();
        ~App();

        /**/
        Bool Initialize( HINSTANCE hinstance );
        void Shutdown();

        /**/
        Bool IsInitialized() const;
        Bool IsElevated() const;

        /**/
        Bool              GetElevate() const;
        const PathString& GetHostPath() const;
        const PathString& GetUserPath() const;
        const PathString& GetLogPath() const;

    private:
        // impl: InputMonitor::IListener, ShellUi::IListener
        /**/
        void OnKeyCombo( Id combo_id );
        Bool OnMenuOption( Id id, Bool enabled );

    private:
        /**/
        Bool _InitializePaths();
        Bool _InitializeSettings();
        Bool _InitializeUi();
        Bool _InitializeInput();
        Bool _InitializeOptions();
        Bool _InitializeOverlay( HINSTANCE hinstance );
        void _InstallOverlayCursors();

        /**/
        void _ShutdownUi();
        void _ShutdownInput();
        void _ShutdownOverlay();
        void _UninstallOverlayCursors();

        /**/
        void _OptionAbout();
        void _OptionErrors();
        Bool _OptionAutoStart( Bool enable, Bool save );
        void _OptionReduceOverlayLag( Bool enable, Bool save );
        Bool _OptionRunAsAdmin();
        void _OptionSettingsFolder();

        // fields: parameters
        Bool        _elevate;

        // fields: state
        Bool        _initialized;

        // fields: info
        PathString  _host_path;
        PathString  _user_path;
        PathString  _log_path;

        // fields: objects
        OverlayCursorVault  _overlay_cursor_vault;
        ShellUi&            _ui;
        InputMonitor        _input_monitor;
        Settings            _settings;
    };
}

#pragma once
#include <Core/Support/Settings.hpp>
#include <Core/Windows/InputMonitor.hpp>
#include <Core/Windows/ShellUi.hpp>
#include <Core/Windows/SystemMonitor.hpp>
#include <YoloMouse/Share/SharedState.hpp>
#include <YoloMouse/Loader/InstanceManager.hpp>

namespace YoloMouse
{
    /**/
    class App:
        public InputMonitor::IListener,
        public SystemMonitor::IListener,
        public ShellUi::IListener
    {
    private:
        InstanceManager _instance_manager;
        SharedState&    _state;

        ShellUi&        _ui;
        InputMonitor    _input_monitor;
        SystemMonitor&  _system_monitor;
        Settings        _settings;

    public:
        /**/
        App();

        /**/
        void Start();
        void Stop();

        /**/
        void Run();

    private:
        /**/
        void _StartInput();
        void _StartOptions();
        void _StartSettings();
        void _StartState();
        void _StartSystem();
        void _StartUi();

        /**/
        void _StopInput();
        void _StopOptions();
        void _StopSettings();
        void _StopState();
        void _StopSystem();
        void _StopUi();

        /**/
        Bool _OptionAutoStart( Bool enable, Bool save );

        /**/
        Bool _AssignCursor( Index cursor_index );
        Bool _AssignSize( ULong size );

        /**/
        Instance* _AccessCurrentInstance();

        /**/
        void OnKeyCombo( Id combo_id );

        /**/
        void OnWindowFocus( HWND hwnd );

        /**/
        Bool OnMenuOption( Id id, Bool enabled );
    };
}

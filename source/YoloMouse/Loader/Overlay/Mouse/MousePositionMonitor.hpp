/*
    attempts to reduce cursor positioning latency. ideally should be called right before vblank.

    the main drawback to a software cursor is it falls behind the system/hardware cursor which has 
    privileged access to the a special place in display hardware thats independent of current
    framerate.

    by only using GetCursorPos the lag is very noticeable. instead this should wait for as clonse to vblank 
    as possible to get cursor position + whatever corresponding display of this position. the idea is that 
    at this point we have the most current update from GetCursorPos plus relative/mickey coordinates from raw
    input. relative coordinates are also adjusted for system mouse speed and acceleration.
                
    its as close to hardware impl as i could come up with without diving into kernel land. its good 
    enough for most gamers, save the FPS crowd, who dont use a cursor anyway :)
*/
#pragma once
#include <Core/Events/SystemMonitorEvent.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Support/EventDispatcher.hpp>
#include <Core/UI/Window.hpp>
#include <YoloMouse/Loader/Events/OverlayEvent.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/BasicCursor.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/IOverlayCursor.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Enums.hpp>

namespace Yolomouse
{
    /**/
    class MousePositionMonitor:
        public EventListener<WindowEvent>
    {
    public:
        /**/
        MousePositionMonitor();
        ~MousePositionMonitor();

        /**/
        Bool Initialize( Window& window );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        Bool GetCursorPosition( Vector2l& windows_position, Vector2f& nds_position );

        /**/
        void SetImprovedPrecision( Bool enabled );

    private:
        // constants
        static constexpr Float MICKEY_MULTIPLIER_DAMPENER = 0.7f;

        /**/
        void _UpdateMouseSettings();

        /**/
        void _OnRawInput( HRAWINPUT hrawinput );

        /**/
        Bool _OnEvent( const WindowEvent& event );

        // fields: parameters
        Window*         _window;
        Bool            _option_improved_precision;
        // fields: state
        Bool            _initialized;
        Float           _mickey_multiplier;
        Vector2l        _cursor_position_delta;
        HWND            _hover_hwnd;
        // fields: input
        RAWINPUTDEVICE  _rawinput_mouse;
    };
}

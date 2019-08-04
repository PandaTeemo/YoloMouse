#include <YoloMouse/Loader/Overlay/Mouse/MousePositionMonitor.hpp>

namespace Yolomouse
{
  // public
    //-------------------------------------------------------------------------
    MousePositionMonitor::MousePositionMonitor():
        // fields: parameters
        _window                     (nullptr),
        _option_improved_precision  (false),
        // fields: state
        _mickey_multiplier          (0),
        _initialized                (false),
        _cursor_position_delta      (0,0),
        _hover_hwnd                 (NULL)
    {
    }

    MousePositionMonitor::~MousePositionMonitor()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool MousePositionMonitor::Initialize( Window& window )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _window = &window;

        // define raw input mouse device
        _rawinput_mouse.usUsagePage =    0x01;
        _rawinput_mouse.usUsage =        0x02;
        _rawinput_mouse.dwFlags =        RIDEV_INPUTSINK;
        _rawinput_mouse.hwndTarget =     _window->GetHandle();

        // register raw input
        if( !RegisterRawInputDevices( &_rawinput_mouse, 1, sizeof( _rawinput_mouse ) ) )
            return false;

        // update mouse settings
        _UpdateMouseSettings();

        // register events
        _window->events.Add( *this );

        // set initialized
        _initialized = true;

        return true;
    }

    void MousePositionMonitor::Shutdown()
    {
        ASSERT( IsInitialized() );

        // unregister events
        _window->events.Remove( *this );

        // define raw input mouse device disable
        _rawinput_mouse.dwFlags = RIDEV_REMOVE;

        // unregister raw input
        RegisterRawInputDevices( &_rawinput_mouse, 1, sizeof( _rawinput_mouse ) );

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool MousePositionMonitor::IsInitialized() const
    {
        return _initialized;
    }

    //-------------------------------------------------------------------------
    Bool MousePositionMonitor::GetCursorPosition( Vector2l& windows_position, Vector2f& nds_position )
    {
        POINT       point;
        Vector2l    adjusted_position;
        Vector2l    size = _window->GetSize();

        // get cursor position (1 frame behind)
        if( !GetCursorPos( &point ) )
            return false;

        // convert point to windows position
        windows_position.Set( point.x, point.y );

        // if rawinput mickeys are to be included to improve precision
        if( _option_improved_precision )
        {
            // adjust by cursor position delta from raw input adjusted by mickey multiplier (depends on windows mouse settings)
            adjusted_position = windows_position + ( _cursor_position_delta.Cast<Float>() * _mickey_multiplier ).Cast<Long>();
        }
        // else use base window position
        else
            adjusted_position = windows_position;

        // convert to NDS coordinates given current resolution
        nds_position.x = (static_cast<Float>(adjusted_position.x) / static_cast<Float>(size.y)) - (_window->GetAspectRatio() * 0.5f);
        nds_position.y = (static_cast<Float>(adjusted_position.y) / static_cast<Float>(size.y)) - 0.5f;

        // reset cursor position delta
        _cursor_position_delta.Set(0);

        return true;
    }

    //-------------------------------------------------------------------------
    void MousePositionMonitor::SetImprovedPrecision( Bool enabled )
    {
        _option_improved_precision = enabled;
    }

    // private
    //-------------------------------------------------------------------------
    void MousePositionMonitor::_UpdateMouseSettings()
    {
        // types
        const struct SpeedTableEntry
        {
            Float normal;
            Float enhanced;
        };

        // constants
        static const ULong           SPEED_TABLE_COUNT = 20;
        static const SpeedTableEntry SPEED_TABLE[SPEED_TABLE_COUNT] = {
            {1.0f/32.0f, 0.1f},
            {1.0f/16.0f, 0.2f},
            {1.0f/8.0f,  0.3f},
            {2.0f/8.0f,  0.4f},
            {3.0f/8.0f,  0.5f},
            {4.0f/8.0f,  0.6f},
            {5.0f/8.0f,  0.7f},
            {6.0f/8.0f,  0.8f},
            {7.0f/8.0f,  0.9f},
            {1.0f,       1.0f},
            {1.25f,      1.1f},
            {1.50f,      1.2f},
            {1.75f,      1.3f},
            {2.00f,      1.4f},
            {2.25f,      1.5f},
            {2.50f,      1.6f},
            {2.75f,      1.7f},
            {3.00f,      1.8f},
            {3.25f,      1.9f},
            {3.50f,      2.0f},
        };

        // locals
        int mouse_speed = 10;
        int mouse_settings[3] = { 0 };

        // get current mouse speed
        SystemParametersInfoA( SPI_GETMOUSESPEED, 0, &mouse_speed, 0 );

        // get acceleration
        SystemParametersInfoA( SPI_GETMOUSE, 0, mouse_settings, 0 );

        // calculate mickey multiplier
        if( mouse_speed >= 1 && mouse_speed <= SPEED_TABLE_COUNT )
        {
            // get speed table entry given mouse speed
            const SpeedTableEntry& entry = SPEED_TABLE[mouse_speed - 1];

            // if acceleration enabled
            if( mouse_settings[2] )
            {
                //TODO4 acceleration more complicated, read following to improve.
                // https://www.esreality.com/index.php?a=post&id=1945096
                // for now increase dampening by 2x
                _mickey_multiplier = entry.enhanced * MICKEY_MULTIPLIER_DAMPENER * 0.5f;
            }
            // else use normal factor
            else
                _mickey_multiplier = entry.normal * MICKEY_MULTIPLIER_DAMPENER;
        }
        // else use default mickey multiplier
        else
            _mickey_multiplier = 1.0f * MICKEY_MULTIPLIER_DAMPENER;
    }

    //-------------------------------------------------------------------------
    void MousePositionMonitor::_OnRawInput( HRAWINPUT hrawinput )
    {
        RAWINPUT ri;
        UINT     ri_size = sizeof(ri);

        // read raw input
        if( GetRawInputData( hrawinput, RID_INPUT, &ri, &ri_size, sizeof( RAWINPUTHEADER ) ) >= 0 )
        {
            // read by type
            if(ri.header.dwType == RIM_TYPEMOUSE)
            {
                // update cursor position delta
                _cursor_position_delta.x += ri.data.mouse.lLastX;
                _cursor_position_delta.y += ri.data.mouse.lLastY;
            }
        }
    }

    //-------------------------------------------------------------------------
    Bool MousePositionMonitor::_OnEvent( const WindowEvent& event )
    {
        switch( event.msg )
        {
        case WM_INPUT:
            _OnRawInput( (HRAWINPUT)event.lparam );
            return true;

        case WM_SETTINGCHANGE:
            // if user updated mouse settings
            if( (UINT)event.wparam == SPI_SETMOUSE )
                _UpdateMouseSettings();
            return true;
        }

        return false;
    }
}

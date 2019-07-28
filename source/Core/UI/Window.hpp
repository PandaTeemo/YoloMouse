#pragma once
#include <Core/Events/WindowEvent.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Support/EventDispatcher.hpp>
#include <windows.h>

namespace Core
{
    /**/
    class Window
    {
    public:
        // enums
        enum: Bits
        {
            OPTION_OVERLAY = BIT( 0 )
        };

        // types
        struct InitializeDef
        {
            HINSTANCE       hinstance;
            Vector2l        size;
            const WCHAR*    class_name;
            const WCHAR*    title;
            Bits            options;
        };

        /**/
        Window();
        ~Window();

        /**/
        Bool Initialize( const InitializeDef& def );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        HWND            GetHandle() const;
        const Vector2l& GetSize() const;
        Float           GetAspectRatio() const;

        /**/
        void SetSize( const Vector2l& size );

        // events
        EventDispatcher<WindowEvent> events;

    private:
        // constants
        static constexpr ULong TOPMOST_REFRESH_TIMER = 1000; // ms

        /**/
        void _SetTop();

        /**/
        static LRESULT CALLBACK _WindowProcedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

        // fields: parameters
        HINSTANCE                   _hinstance;
        const WCHAR*                _class_name;
        // fields: state
        HWND                        _hwnd;
        Vector2l                    _size;
        Float                       _aspect_ratio;
    };
}

//TODO3: dump d3d crap into sep class, maybe mousing details too
/*
    this is a software cursor overlay implementation as an alternative to the more intrusive injected
    implementation in order to avoid the wrath of anticheats.

    solving cursor latency
        the main drawback to a software cursor such as this one is it falls behind the system/hardware cursor
        which has privileged access to the a special place in display hardware thats independent of current
        framerate.

        by only using GetCursorPos and vsyncing renders the lag will be very noticeable. instead we wait for
        vsync/blank manually without letting present do it. the idea is that during vblank we have the most 
        current update from GetCursorPos plus relative/mickey coordinates from raw input. relative coordinates
        are also adjusted for system mouse speed and acceleration.
                
        its as close to hardware impl as i could come up with without diving into kernel land. its good 
        enough for most gamers, save the FPS crowd, who dont have a cursor anyway :)

    hiding old game cursor
        you'd think this would be simple, but it requires injecting into the process. instead i roll with a
        windows hooks exploit whereby i provide the hook an already loaded dll (user32.dll) and a pointer
        to ShowCursor(bool) instead of a legit hook callback. this hook is then externally triggered such
        that ShowCursor gets the first argument of a hook routine (nCode) such that the value is 0 aka FALSE.
*/
#pragma once
#include <Core/Events/SystemMonitorEvent.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Vector4.hpp>
#include <Core/Support/EventDispatcher.hpp>
#include <Core/Support/Singleton.hpp>
#include <YoloMouse/Loader/Events/OverlayEvent.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/IOverlayCursor.hpp>
#include <YoloMouse/Loader/Overlay/Shaders/ShaderTypes.hpp>
#include <YoloMouse/Loader/Overlay/Support/RenderTimingController.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Enums.hpp>
#include <dxgi1_2.h>
#include <d3d11.h>

namespace Yolomouse
{
    /**/
    class Overlay:
        public Singleton<Overlay>,
        public EventListener<SystemMonitorEvent>
    {
    public:
        /**/
        Overlay();
        ~Overlay();

        /**/
        Bool Initialize( HINSTANCE hinstance );
        void Shutdown();

        /**/
        Bool IsInitialized() const;
        Bool IsStarted() const;
        Bool IsCursorInstalled( CursorId id ) const;

        /**/
        ID3D11Device&        GetDevice() const;
        ID3D11DeviceContext& GetDeviceContext() const;

        /**/
        void ShowCursor( CursorId id, CursorVariation variation, CursorSize size );
        void HideCursor();

        /**/
        Bool InstallCursor( CursorId id, IOverlayCursor& cursor );
        Bool UninstallCursor( CursorId id );

        /**/
        Bool Start();
        void Stop();

        // events
        EventDispatcher<OverlayEvent> events;

    private:
        // enums
        enum: Bits
        {
            FRAME_EVENT_RESIZE = BIT(0)
        };
        typedef Bits FrameEvents;

        // constants
        static constexpr Float DEFAULT_REFRESH_RATE =  1.0f / 59.94f;  // hz
        static constexpr Float FRAME_PROCESSING_TIME = 1.5f;           // ms
        static constexpr ULong TOPMOST_UPDATE_COUNT =  8;              // frames

        // aliases
        typedef FlatArray<IOverlayCursor*, CURSOR_ID_COUNT> CursorTable;

        // constants
        const ULong _THREAD_STACK_SIZE = KILOBYTES(64);

        /**/
        Bool _Initialize( HINSTANCE hinstance );
        Bool _InitializeWindow();
        Bool _InitializeD3d();
        Bool _InitializeD3dBuffer( ID3D11Buffer*& buffer, D3D11_USAGE usage, ULong size, UINT BindFlags );
        Bool _InitializeD3dDepthStencilView();
        Bool _InitializeD3dRenderTargetView();
        void _InitializeView();
        Bool _InitializeInput();
        Bool _InitializeThread();

        /**/
        void _Shutdown();
        void _ShutdownWindow();
        void _ShutdownD3d();
        void _ShutdownD3dDepthStencilView();
        void _ShutdownD3dRenderTargetView();
        void _ShutdownInput();
        void _ShutdownThread();

        /**/
        void _ProcessFrameEvents();
        Bool _FrameResize( const Vector2l& size );
        void _FrameLoop();

        /**/
        void _UpdateMouseSettings();
        void _UpdateHoverState( const Vector2l& cursor_position );
        void _UpdateTopmost();
        void _UpdateScene( IOverlayCursor& cursor, const Vector2f& nds_position );

        /**/
        void _Render( const IOverlayCursor* cursor );

        /**/
        Bool _CalculateCursorPosition( Vector2l& windows_position, Vector2l& adjusted_position, Vector2f& nds_position );

        /**/
        void _OnWindowZOrder( HWND hwnd );
        void _OnRawInput( HRAWINPUT hrawinput );
        void _OnDisplayChange( ULong width, ULong height );
        Bool _OnEvent( const SystemMonitorEvent& event );

        /**/
        static LRESULT CALLBACK _WindowProcedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
        static DWORD WINAPI     _ThreadProcedure( _In_ LPVOID lpParameter );

        // fields: parameters
        CursorTable                 _cursors;
        IOverlayCursor*             _active_cursor;
        // fields: windows
        HINSTANCE                   _hinstance;
        HWND                        _hwnd;
        Vector2l                    _size;
        HANDLE                      _thread;
        Float                       _mickey_multiplier;
        // fields: state
        Bool                        _initialized;
        Bool                        _started;
        Bool                        _active;
        Vector2l                    _cursor_position_delta;
        HWND                        _hover_hwnd;
        ULong                       _topmost_counter;
        RenderTimingController      _render_timing_controller;
        FrameEvents                 _frame_events;
        Vector2l                    _frame_resize;
        // fields: d3d state
        ID3D11Device*               _device;
        ID3D11DeviceContext*        _device_context;
        IDXGIFactory2*              _factory;
        IDXGISwapChain1*            _swapchain;
        IDXGIOutput*                _swapchain_output;
        ID3D11DepthStencilState*    _depthstencil_state;
        ID3D11Texture2D*            _depthstencil_buffer;
        ID3D11DepthStencilView*     _depthstencil_view;
        ID3D11BlendState*           _blend_state;
        ID3D11RenderTargetView*     _render_target_view;
        ID3D11VertexShader*         _vertex_shader;
        ID3D11PixelShader*          _pixel_shader;
        ID3D11InputLayout*          _shader_input_layout;
        ID3D11Buffer*               _vs_constant_buffer;
        VertexShaderConstantValue   _vs_constant_value;
        ID3D11Buffer*               _ps_constant_buffer;
        PixelShaderConstantValue    _ps_constant_value;
        ID3D11RasterizerState*      _rasterizer_state;
        // fields: d3d view
        Float                       _aspect_ratio;
        // fields: input
        RAWINPUTDEVICE              _rawinput_mouse;
    };
}

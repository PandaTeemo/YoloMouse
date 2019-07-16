#include <Core/Math/Math.hpp>
#include <Core/Math/Transform3.hpp>
#include <Core/System/SystemTools.hpp>
#include <Core/Windows/SystemMonitor.hpp>
#include <YoloMouse/Loader/Overlay/Overlay.hpp>
#include <YoloMouse/Loader/Overlay/Shaders/Default/PixelShader.hpp>
#include <YoloMouse/Loader/Overlay/Shaders/Default/VertexShader.hpp>
#include <dwmapi.h>
#include <math.h>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    Overlay::Overlay():
        // fields: parameters
        _active_cursor          (nullptr),
        // fields: window
        _hinstance              (NULL),
        _hwnd                   (NULL),
        _size                   (0, 0),
        _thread                 (NULL),
        _mickey_multiplier      (0),
        // fields: state
        _initialized            (false),
        _started                (false),
        _active                 (false),
        _cursor_position_delta  (0,0),
        _topmost_counter        (0),
        _hover_hwnd             (NULL),
        _frame_events           (0),
        _frame_resize           (0, 0),
        // fields: d3d state
        _device                 (nullptr),
        _device_context         (nullptr),
        _factory                (nullptr),
        _swapchain              (nullptr),
        _swapchain_output       (nullptr),
        _depthstencil_state     (nullptr),
        _depthstencil_buffer    (nullptr),
        _depthstencil_view      (nullptr),
        _blend_state            (nullptr),
        _render_target_view     (nullptr),
        _vertex_shader          (nullptr),
        _pixel_shader           (nullptr),
        _shader_input_layout    (nullptr),
        _vs_constant_buffer     (nullptr),
        _ps_constant_buffer     (nullptr),
        _rasterizer_state       (nullptr),
        // fields: d3d scene
        _aspect_ratio           (0)
    {
        _cursors.Zero();
    }

    Overlay::~Overlay()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool Overlay::Initialize( HINSTANCE hinstance )
    {
        ASSERT( !IsInitialized() );

        // initialize else undo failings
        if( !_Initialize( hinstance ) )
        {
            _Shutdown();
            return false;
        }

        // set initialized
        _initialized = true;

        return true;
    }

    void Overlay::Shutdown()
    {
        ASSERT( IsInitialized() );
        ASSERT( !IsStarted() );

        // shutdown
        _Shutdown();

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool Overlay::IsInitialized() const
    {
        return _initialized;
    }

    Bool Overlay::IsStarted() const
    {
        return _started;
    }

    Bool Overlay::IsCursorInstalled( CursorId id ) const
    {
        ASSERT( id != CURSOR_ID_INVALID );
        return _cursors[id] != nullptr;
    }

    //-------------------------------------------------------------------------
    ID3D11Device& Overlay::GetDevice() const
    {
        ASSERT( IsInitialized() );
        return *_device;
    }

    ID3D11DeviceContext& Overlay::GetDeviceContext() const
    {
        ASSERT( IsInitialized() );
        return *_device_context;
    }

    //-------------------------------------------------------------------------
    void Overlay::ShowCursor( CursorId id, CursorVariation variation, CursorSize size )
    {
        ASSERT( id != CURSOR_ID_INVALID );

        // get specified cursor
        IOverlayCursor* cursor = _cursors[id];

        // if valid, update cursor settings
        if( cursor != nullptr )
        {
            // update cursor settings
            cursor->SetCursorVariation( variation );
            cursor->SetCursorSize( size );

            // set new active cursor
            _active_cursor = cursor;
        }
    }

    void Overlay::HideCursor()
    {
        // reset active cursor cursor
        _active_cursor = nullptr;
    }

    //-------------------------------------------------------------------------
    Bool Overlay::InstallCursor( CursorId id, IOverlayCursor& cursor )
    {
        ASSERT( IsInitialized() );
        ASSERT( _cursors[id] == nullptr );

        // initialize cursor
        if( !cursor.Initialize({
                *_device,
                *_device_context,
                _aspect_ratio
            }) )
            return false;

        // add cursor to table
        _cursors[id] = &cursor;

        return true;
    }

    Bool Overlay::UninstallCursor( CursorId id )
    {
        ASSERT( IsInitialized() );

        // get cursor from table
        IOverlayCursor*& cursor = _cursors[id];
        if( cursor == nullptr )
            return false;

        // shutdown cursor
        cursor->Shutdown();

        // remove from table
        cursor = nullptr;

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Overlay::Start()
    {
        ASSERT( !IsStarted() );

        // initialize thread
        if( !_InitializeThread() )
            return false;

        // register events
        SystemMonitor::Instance().events.Add( *this );

        // set started
        _started = true;

        return true;
    }

    void Overlay::Stop()
    {
        ASSERT( IsStarted() );

        // unregister events
        SystemMonitor::Instance().events.Remove( *this );

        // shutdown thread
        _ShutdownThread();

        // reset started
        _started = false;
    }

    // private
    //-------------------------------------------------------------------------
    Bool Overlay::_Initialize( HINSTANCE hinstance )
    {
        // set fields
        _hinstance = hinstance;

        // get current resolution
        _size.Set(GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ));

        // update mouse settings
        _UpdateMouseSettings();

        // initialize window
        if( !_InitializeWindow() )
            return false;

        // initialize direct3d
        if( !_InitializeD3d() )
            return false;

        // initialize render timing controller
        _render_timing_controller.SetResolution( _size );
        _render_timing_controller.Initialize( *_swapchain_output );

        // initialize view
        _InitializeView();

        // initialize input
        if( !_InitializeInput() )
            return false;

        return true;
    }

    Bool Overlay::_InitializeWindow()
    {
        WNDCLASS        wclass = {};
        const MARGINS   frame_extend_margin = {-1,-1,-1,-1};

        // build window class
        wclass.style = CS_OWNDC;
        wclass.lpfnWndProc = _WindowProcedure;
        wclass.cbClsExtra = 0;
        wclass.cbWndExtra = 0;
        wclass.hInstance = _hinstance;
        wclass.hIcon = LoadIcon( nullptr, IDI_APPLICATION );
        wclass.hCursor = LoadCursor( nullptr, IDC_CROSS );
        wclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wclass.lpszMenuName = nullptr;
        wclass.lpszClassName = OVERLAY_CLASS;

        // register window class
        RegisterClass(&wclass);

        // create window
        _hwnd = CreateWindowEx(
            0,                          // optional window styles.
            OVERLAY_CLASS,              // window class
            OVERLAY_NAME,               // window text
            WS_POPUP,                   // window style
            0, 0, _size.x, _size.y,     // size and position
            NULL,                       // parent window    
            NULL,                       // menu
            _hinstance,                 // instance handle
            NULL                        // additional application data
        );

        // fail if 
        if( _hwnd == NULL )
        {
            LOG( "Overlay.CreateWindowEx" );
            return false;
        }

        // set overlay styles
	    SetWindowLongPtr(_hwnd, GWL_STYLE, WS_VISIBLE); // not quite same as ShowWindow
	    SetWindowLongPtr(_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST|WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE);
        DwmExtendFrameIntoClientArea(_hwnd, &frame_extend_margin );
        ShowWindow(_hwnd, SW_MAXIMIZE);
        
        return true;
    }

    Bool Overlay::_InitializeD3d()
    {
        // driver types supported
        const D3D_DRIVER_TYPE DRIVER_TYPES[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };

        // feature levels supported
        const D3D_FEATURE_LEVEL FEATURE_LEVELS[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_1
        };

        // shader layout
        D3D11_INPUT_ELEMENT_DESC SHADER_LAYOUT[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            //{"UV",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        // locals
        DXGI_SWAP_CHAIN_DESC1           swapchain_desc = {};
        IDXGIDevice*                    dxgi_device = nullptr;
        IDXGIAdapter*                   dxgi_adapter = nullptr;
        D3D11_DEPTH_STENCIL_DESC        depthstencil_desc = {};
        D3D11_BLEND_DESC                blend_desc = {};
        D3D11_RASTERIZER_DESC           rasterizer_desc = {};
        HRESULT                         hresult;

        // for each device in order of ideal first
        for( UINT i = 0; i < ARRAYSIZE( DRIVER_TYPES ); ++i )
        {
            D3D_FEATURE_LEVEL feature_level;

            // attempt to create d3d device
            hresult = D3D11CreateDevice(
                nullptr,
                DRIVER_TYPES[i],
                nullptr,
                0,
                FEATURE_LEVELS,
                ARRAYSIZE( FEATURE_LEVELS ),
                D3D11_SDK_VERSION,
                &_device,
                &feature_level,
                &_device_context );

            // choose this device
            if( SUCCEEDED( hresult ) )
                break;
        }

        // fail if no devices created
        if( FAILED( hresult ) )
        {
            LOG( "Overlay.D3D11CreateDevice" );
            return false;
        }

        // get dxgi factory
        hresult = _device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
        if (FAILED(hresult))
        {
            LOG( "Overlay.dxgi_device.QueryInterface" );
            return false;
        }
        hresult = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgi_adapter));
        dxgi_device->Release();
        if (FAILED(hresult))
        {
            LOG( "Overlay.dxgi_device.GetParent" );
            return false;
        }
        hresult = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&_factory));
        dxgi_adapter->Release();
        dxgi_adapter = nullptr;
        if (FAILED(hresult))
        {
            LOG( "Overlay.dxgi_adapter.GetParent" );
            return false;
        }

        // describe swapchain
        swapchain_desc.SwapEffect =            DXGI_SWAP_EFFECT_DISCARD;
        swapchain_desc.BufferCount =           1;
        swapchain_desc.Width =                 _size.x;
        swapchain_desc.Height =                _size.y;
        swapchain_desc.Format =                DXGI_FORMAT_B8G8R8A8_UNORM;
        swapchain_desc.BufferUsage =           DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.SampleDesc.Count =      1;
        swapchain_desc.SampleDesc.Quality =    0;

        // create swapchain
        hresult = _factory->CreateSwapChainForHwnd(_device, _hwnd, &swapchain_desc, nullptr, nullptr, &_swapchain);
        if (FAILED(hresult))
        {
            LOG( "Overlay.dxgi_factory.CreateSwapChainForHwnd" );
            return false;
        }

        // get swapchain output
        hresult = _swapchain->GetContainingOutput( &_swapchain_output );
        if (FAILED(hresult))
        {
            LOG( "Overlay.swapchain.GetContainingOutput" );
            return false;
        }

        // describe depth buffer (disable for now)
        depthstencil_desc.DepthEnable = false;
        depthstencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthstencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

        // describe blend state
        blend_desc.RenderTarget[0].BlendEnable = TRUE;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

        // create blend state
        if( (hresult = _device->CreateBlendState(&blend_desc, &_blend_state)) != S_OK )
        {
            LOG( "Overlay.device.CreateBlendState" );
            return false;
        }

        // set blend state
        _device_context->OMSetBlendState(_blend_state, NULL, 0xffffffff);

        // create depth stencil state
        if( (hresult = _device->CreateDepthStencilState(&depthstencil_desc, &_depthstencil_state)) != S_OK )
        {
            LOG( "Overlay.device.CreateDepthStencilState" );
            return false;
        }

        // set depth stencil state
        _device_context->OMSetDepthStencilState(_depthstencil_state, 0);

        // initialize depth stencil view
        if( !_InitializeD3dDepthStencilView() )
            return false;

        // initialize render view
        if( !_InitializeD3dRenderTargetView() )
            return false;

        // create vertex shader
        if( (hresult = _device->CreateVertexShader(g_VS, ARRAYSIZE(g_VS), nullptr, &_vertex_shader)) != S_OK )
        {
            LOG( "Overlay.device.CreateVertexShader" );
            return false;
        }

        // create pixel shader
        if( (hresult = _device->CreatePixelShader(g_PS, ARRAYSIZE(g_PS), nullptr, &_pixel_shader)) != S_OK )
        {
            LOG( "Overlay.device.CreatePixelShader" );
            return false;
        }

        // create shader layout
        if( (hresult = _device->CreateInputLayout(SHADER_LAYOUT, ARRAYSIZE(SHADER_LAYOUT), g_VS, ARRAYSIZE(g_VS), &_shader_input_layout)) != S_OK )
        {
            LOG( "Overlay.device.CreateDepthStencilView" );
            return false;
        }

        // create vertex shader object constant buffer
        if( !_InitializeD3dBuffer( _vs_constant_buffer, D3D11_USAGE_DEFAULT, sizeof( VertexShaderConstantValue ), D3D11_BIND_CONSTANT_BUFFER ) )
            return false;
        _device_context->VSSetConstantBuffers(0, 1, &_vs_constant_buffer);
 
        // create pixel shader object constant buffer
        if( !_InitializeD3dBuffer( _ps_constant_buffer, D3D11_USAGE_DEFAULT, sizeof( PixelShaderConstantValue ), D3D11_BIND_CONSTANT_BUFFER ) )
            return false;
        _device_context->PSSetConstantBuffers(0, 1, &_ps_constant_buffer);
 
        // set layout
        _device_context->IASetInputLayout(_shader_input_layout);

        // set shaders
        _device_context->VSSetShader(_vertex_shader, 0, 0);
        _device_context->PSSetShader(_pixel_shader, 0, 0);

        // set topology
        _device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
        // describe rasterizer state
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        //rasterizer_desc.CullMode = D3D11_CULL_BACK;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        //rasterizer_desc.AntialiasedLineEnable = TRUE;
        //rasterizer_desc.MultisampleEnable = TRUE;

        // create rasterizer state
        if( (hresult = _device->CreateRasterizerState(&rasterizer_desc, &_rasterizer_state)) != S_OK )
            return false;

        // set rasterizer state
        _device_context->RSSetState(_rasterizer_state);

        return true;
    }

    Bool Overlay::_InitializeD3dBuffer( ID3D11Buffer*& buffer, D3D11_USAGE usage, ULong size, UINT BindFlags )
    {
        D3D11_BUFFER_DESC               buffer_desc = {};

        // describe shader constant buffer
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.ByteWidth = sizeof(VertexShaderConstantValue);
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffer_desc.CPUAccessFlags = 0;
        buffer_desc.MiscFlags = 0;

        // create shader constant buffer
        return _device->CreateBuffer( &buffer_desc, NULL, &buffer ) == S_OK;
    }

    Bool Overlay::_InitializeD3dDepthStencilView()
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC   depthstencil_view_desc = {};
        D3D11_TEXTURE2D_DESC            depthstencil_texture = {};
        HRESULT                         hresult;

        // set depth stencil state
        _device_context->OMSetDepthStencilState(_depthstencil_state, 0);

        // describe depth stencil view
        depthstencil_view_desc.Format =             DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthstencil_view_desc.ViewDimension =      D3D11_DSV_DIMENSION_TEXTURE2D;
        depthstencil_view_desc.Texture2D.MipSlice = 0;

        // describe depth stencil texture
        depthstencil_texture.Width =                _size.x;
        depthstencil_texture.Height =               _size.y;
        depthstencil_texture.MipLevels =            1;
        depthstencil_texture.ArraySize =            1;
        depthstencil_texture.Format =               DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthstencil_texture.SampleDesc.Count =     1;
        depthstencil_texture.SampleDesc.Quality =   0;
        depthstencil_texture.Usage =                D3D11_USAGE_DEFAULT;
        depthstencil_texture.BindFlags =            D3D11_BIND_DEPTH_STENCIL;
        depthstencil_texture.CPUAccessFlags =       0;
        depthstencil_texture.MiscFlags =            0;
 
        // create depth/stencil view
        if( (hresult = _device->CreateTexture2D(&depthstencil_texture, NULL, &_depthstencil_buffer)) != S_OK )
        {
            LOG( "Overlay.device.CreateTexture2D" );
            return false;
        }
        if( (hresult = _device->CreateDepthStencilView(_depthstencil_buffer, &depthstencil_view_desc, &_depthstencil_view)) != S_OK )
        {
            LOG( "Overlay.device.CreateDepthStencilView" );
            return false;
        }

        return true;
    }

    Bool Overlay::_InitializeD3dRenderTargetView()
    {
        ID3D11Texture2D* back_buffer;
        HRESULT          hresult;

        // get back buffer
        if( (hresult = _swapchain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&back_buffer )) != S_OK )
        {
            LOG( "Overlay.swapchain.GetBuffer" );
            return false;
        }

        // create render target view
        if( _device->CreateRenderTargetView( back_buffer, NULL, &_render_target_view ) != S_OK )
        {
            LOG( "Overlay.device.CreateRenderTargetView" );
            return false;
        }

        // release buffer
        back_buffer->Release();

        // set render target
        _device_context->OMSetRenderTargets( 1, &_render_target_view, _depthstencil_view );

        return true;
    }

    void Overlay::_InitializeView()
    {
        D3D11_VIEWPORT viewport = {};

        // update aspect ratio
        _aspect_ratio = static_cast<Float>(_size.x) / static_cast<Float>(_size.y);

        // create viewport
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = static_cast<Float>(_size.x);
        viewport.Height = static_cast<Float>(_size.y);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
 
        // set viewport
        _device_context->RSSetViewports(1, &viewport);
    }

    Bool Overlay::_InitializeInput()
    {
        // define raw input mouse device
        _rawinput_mouse.usUsagePage =    0x01;
        _rawinput_mouse.usUsage =        0x02;
        _rawinput_mouse.dwFlags =        RIDEV_INPUTSINK;
        _rawinput_mouse.hwndTarget =     _hwnd;

        // register raw input
        return RegisterRawInputDevices( &_rawinput_mouse, 1, sizeof( _rawinput_mouse ) ) == TRUE;
    }

    Bool Overlay::_InitializeThread()
    {
        // create thread for run method
        _thread = CreateThread( NULL, _THREAD_STACK_SIZE, _ThreadProcedure, this, 0, NULL );
        if( _thread == NULL )
            return false;

        // raise priority. ignore fails
        SetThreadPriority( _thread, THREAD_PRIORITY_HIGHEST );

        // set active state
        _active = true;

        return true;
    }

    //-------------------------------------------------------------------------
    void Overlay::_Shutdown()
    {
        // shutdown input
        _ShutdownInput();

        // shutdown render timing controller
        if( _render_timing_controller.IsInitialized() )
            _render_timing_controller.Shutdown();

        // shutdown direct3d
        _ShutdownD3d();

        // shutdown window
        _ShutdownWindow();
    }

    void Overlay::_ShutdownWindow()
    {
        // destroy window
        DestroyWindow(_hwnd);

        // unregister class
        UnregisterClass(OVERLAY_CLASS, _hinstance);

        // reset fields
        _hwnd = nullptr;
    }

    void Overlay::_ShutdownD3d()
    {
        // shutdown rasterizer state
        if( _rasterizer_state )
        {
            _rasterizer_state->Release();
            _rasterizer_state = nullptr;
        }

        // shutdown pixel shader constant buffer
        if( _ps_constant_buffer )
        {
            _ps_constant_buffer->Release();
            _ps_constant_buffer = nullptr;
        }

        // shutdown vertex shader constant buffer
        if( _vs_constant_buffer )
        {
            _vs_constant_buffer->Release();
            _vs_constant_buffer = nullptr;
        }

        // shutdown shader input layout
        if( _shader_input_layout )
        {
            _shader_input_layout->Release();
            _shader_input_layout = nullptr;
        }

        // shutdown pixel shader
        if( _pixel_shader )
        {
            _pixel_shader->Release();
            _pixel_shader = nullptr;
        }

        // shutdown vertex shader
        if( _vertex_shader )
        {
            _vertex_shader->Release();
            _vertex_shader = nullptr;
        }

        // shutdown render target view
        _ShutdownD3dRenderTargetView();

        // shutdown depth stencil view
        _ShutdownD3dDepthStencilView();

        // shutdown blend state
        if( _blend_state )
        {
            _blend_state->Release();
            _blend_state = nullptr;
        }

        // shutdown depthstencil state
        if( _depthstencil_state )
        {
            _depthstencil_state->Release();
            _depthstencil_state = nullptr;
        }

        // shutdown swapchain output
        if( _swapchain_output )
        {
            _swapchain_output->Release();
            _swapchain_output = nullptr;
        }

        // shutdown swapchain
        if( _swapchain )
        {
            _swapchain->Release();
            _swapchain = nullptr;
        }

        // shutdown dxgi factory
        if( _factory )
        {
            _factory->Release();
            _factory = nullptr;
        }

        // shutdown device context
        if( _device_context )
        {
            _device_context->Release();
            _device_context = nullptr;
        }

        // shutdown device
        if( _device )
        {
            _device->Release();
            _device = nullptr;
        }
    }

    void Overlay::_ShutdownD3dDepthStencilView()
    {
        // shutdown depthstencil view
        if( _depthstencil_view )
        {
            _depthstencil_view->Release();
            _depthstencil_view = nullptr;
        }

        // shutdown depthstencil buffer
        if( _depthstencil_buffer )
        {
            _depthstencil_buffer->Release();
            _depthstencil_buffer = nullptr;
        }
    }

    void Overlay::_ShutdownD3dRenderTargetView()
    {
        // shutdown render target view
        if( _render_target_view )
        {
            _render_target_view->Release();
            _render_target_view = nullptr;
        }
    }

    void Overlay::_ShutdownInput()
    {
        // define raw input mouse device disable
        _rawinput_mouse.dwFlags = RIDEV_REMOVE;

        // unregister raw input
        RegisterRawInputDevices( &_rawinput_mouse, 1, sizeof( _rawinput_mouse ) );
    }

    void Overlay::_ShutdownThread()
    {
        // if active
        if( _active )
        {
            // reset active state
            _active = false;

            // wait for thread to complete
            WaitForSingleObject( _thread, 2000 );
        }
    }

    //-------------------------------------------------------------------------
    void Overlay::_ProcessFrameEvents()
    {
        // if pending frame events exist
        if( _frame_events != 0 )
        {
            // handle resize
            if( _frame_events & FRAME_EVENT_RESIZE )
                _FrameResize( _frame_resize );

            // reset frame events
            _frame_events = 0;
        }
    }

    Bool Overlay::_FrameResize( const Vector2l& size )
    {
        HRESULT hresult;

        // set new size
        _size = size;

        // update window size
        SetWindowPos( _hwnd, HWND_TOPMOST, 0, 0, _size.x, _size.y, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOREDRAW );

        // update render timing controller
        _render_timing_controller.SetResolution( size );

        // shutdown render target view
        _ShutdownD3dRenderTargetView();

        // shutdown depth stencil view
        _ShutdownD3dDepthStencilView();

        // resize swapchain
        //hresult = _swapchain->ResizeBuffers(1, size.x, size.y, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        hresult = _swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hresult))
        {
            LOG( "Overlay.ResizeSwapchain.ResizeBuffers" );
            return false;
        }

        // reinitialize depth stencil view
        if( !_InitializeD3dDepthStencilView() )
            return false;

        // reinitialize render target view
        if( !_InitializeD3dRenderTargetView() )
            return false;

        // reinitialize view
        _InitializeView();

        // notify cursors
        for( IOverlayCursor* cursor : _cursors )
            if( cursor != nullptr )
                cursor->SetSceneSize( { _aspect_ratio } );

        return true;
    }

    void Overlay::_FrameLoop()
    {
        DEBUG( UHuge tick_frequency = SystemTools::GetTickFrequency(); )
        Vector2l windows_position;
        Vector2l adjusted_position;
        Vector2f nds_position;
        Bool     cleared = false;

        // run frame loop
        while(_active)
        {
            DEBUG( UHuge begin_ticks = SystemTools::GetTickTime(); )

            // process pending frame events
            _ProcessFrameEvents();

            /*
                wait for vblank then wait as close to next vblank as possible while still allowing time for processing and
                rendering as controlled by render timing controller. the idea is to capture the most accurate cursor positioning
                and display it as close to when hardware cursor is rendered (exactly at vblank) in order to reduce cursor lag.
                
                this is a best effort approach without diving into kernel land. its good enough for most gamers, except the
                FPS crowd, who dont use a cursor anyway :)
            */
            _swapchain_output->WaitForVBlank();

            // begin render timing controller
            _render_timing_controller.Begin();

            // calculate cursor position
            if( _CalculateCursorPosition(windows_position, adjusted_position, nds_position) )
            {
                // update hover state using windows cursor position
                _UpdateHoverState(windows_position);

                // update topmost state
                _UpdateTopmost();

                // get active cursor
                IOverlayCursor* cursor = _active_cursor;

                // if active cursor exists
                if( cursor != nullptr )
                {
                    // update scene
                    _UpdateScene( *cursor, nds_position );

                    // render scene
                    _Render( cursor );

                    // reset cleared
                    cleared = false;
                }
                // if not cleared
                else if( !cleared )
                {
                    // clear last render
                    _Render(nullptr);

                    // set cleared
                    cleared = true;
                }
            }

            // end render timing controller
            _render_timing_controller.End();

            // log
            //LOG2( "Overlay.Run.FrameTime %f", static_cast<Float>(SystemTools::GetTickTime() - begin_ticks) / static_cast<Float>(tick_frequency) );
        }
    }

    //-------------------------------------------------------------------------
    void Overlay::_UpdateMouseSettings()
    {
        // types
        const struct SpeedTableEntry
        {
            Float normal;
            Float enhanced;
        };

        // constants
        static const Float           MULTIPLIER_DAMPENER = 0.7f;
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
                //TODO4 acceleration more complicated, read following to improve
                // https://www.esreality.com/index.php?a=post&id=1945096
                _mickey_multiplier = entry.enhanced * MULTIPLIER_DAMPENER;
            }
            // else use normal factor
            else
                _mickey_multiplier = entry.normal * MULTIPLIER_DAMPENER;
        }
        // else use default mickey multiplier
        else
            _mickey_multiplier = 1.0f * MULTIPLIER_DAMPENER;
    }

    void Overlay::_UpdateHoverState( const Vector2l& windows_position )
    {
        POINT point;

        // convert last cursor position to point
        point.x = windows_position.x;
        point.y = windows_position.y;

        // get window at cursor position
        HWND hwnd = WindowFromPoint(point);

        // if different from current hover window
        if( hwnd != _hover_hwnd )
        {
            // set topmost counter to trigger topmost update
            _topmost_counter = TOPMOST_UPDATE_COUNT;

            // update hover window
            _hover_hwnd = hwnd;
        }

        // notify
        events.Notify( {OverlayEvent::WINDOW_HOVER, hwnd } );
    }

    /*
        this counter based method to delay updating our Z order is to avoid performance issues with
        another process trying to do the same.
    */
    void Overlay::_UpdateTopmost()
    {
        // if topmost counter active, decrement, and if reached 0
        if( _topmost_counter > 0 && --_topmost_counter == 0 )
        {
            // update our topmost state (reposition Z order)
            SetWindowPos( _hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );
        }
    }

    void Overlay::_UpdateScene( IOverlayCursor& cursor, const Vector2f& nds_position )
    {
        // update cursor scene
        cursor.Update({
            _render_timing_controller.GetFrameTime(),
            nds_position,
            _vs_constant_value,
            _ps_constant_value
        });
    }

    //-------------------------------------------------------------------------
    void Overlay::_Render( const IOverlayCursor* cursor )
    {
        const Float CLEAR_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        // clear backbuffer
        _device_context->ClearRenderTargetView(_render_target_view, CLEAR_COLOR);
 
        // clear depth/stencil buffers
        _device_context->ClearDepthStencilView(_depthstencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
 
        // sync shader objects to gpu
        _device_context->UpdateSubresource(_vs_constant_buffer, 0, NULL, &_vs_constant_value, 0, 0);
        _device_context->UpdateSubresource(_ps_constant_buffer, 0, NULL, &_ps_constant_value, 0, 0);
            
        // render cursor, if specified
        if( cursor != nullptr )
            cursor->Render();

        // render swapchain buffers to screen
        _swapchain->Present(0, 0);
    }

    //-------------------------------------------------------------------------
    Bool Overlay::_CalculateCursorPosition( Vector2l& windows_position, Vector2l& adjusted_position, Vector2f& nds_position )
    {
        POINT point;

        // get cursor position (1 frame behind)
        if( !GetCursorPos( &point ) )
            return false;

        // convert point to windows position
        windows_position.x = point.x;
        windows_position.y = point.y;

        // adjust by cursor position delta from raw input adjusted by mickey multiplier (depends on windows mouse settings)
        adjusted_position.x = windows_position.x + static_cast<Long>(static_cast<Float>(_cursor_position_delta.x) * _mickey_multiplier);
        adjusted_position.y = windows_position.y + static_cast<Long>(static_cast<Float>(_cursor_position_delta.y) * _mickey_multiplier);

        // convert to NDS coordinates given current resolution
        nds_position.x = (static_cast<Float>(adjusted_position.x) / static_cast<Float>(_size.y)) - (_aspect_ratio * 0.5f);
        nds_position.y = (static_cast<Float>(adjusted_position.y) / static_cast<Float>(_size.y)) - 0.5f;

        // reset cursor position delta
        _cursor_position_delta.Set(0);

        return true;
    }

    //-------------------------------------------------------------------------
    void Overlay::_OnWindowZOrder( HWND hwnd )
    {
        // process if current hover window
        if( hwnd == _hover_hwnd )
        {
            // set topmost counter to trigger topmost update
            _topmost_counter = TOPMOST_UPDATE_COUNT;
        }
    }

    void Overlay::_OnRawInput( HRAWINPUT hrawinput )
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

    void Overlay::_OnDisplayChange( ULong width, ULong height )
    {
        // send pending resize event
        _frame_events |= FRAME_EVENT_RESIZE;
        _frame_resize.Set( width, height );
    }

    Bool Overlay::_OnEvent( const SystemMonitorEvent& event )
    {
        switch( event.id )
        {
        case SystemMonitorEvent::WINDOW_ZORDER:
            _OnWindowZOrder(event.hwnd);
            return true;
        default:
            return false;
        }
    }

    //-------------------------------------------------------------------------
    LRESULT CALLBACK Overlay::_WindowProcedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
    {
        switch( msg )
        {
        case WM_INPUT:
            Overlay::Instance()._OnRawInput( (HRAWINPUT)lparam );
            return 0;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;

        case WM_DISPLAYCHANGE:
            Overlay::Instance()._OnDisplayChange(LOWORD(lparam), HIWORD(lparam));
            break;

        case WM_SETTINGCHANGE:
            // if user updated mouse settings
            if( (UINT)wparam == SPI_SETMOUSE )
                Overlay::Instance()._UpdateMouseSettings();
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    /**/
    DWORD WINAPI Overlay::_ThreadProcedure( _In_ LPVOID lpParameter )
    {
        // parameter is overlay window
        Overlay* overlay_window = reinterpret_cast<Overlay*>(lpParameter);

        // run
        overlay_window->_FrameLoop();

        return 0;
    }
}

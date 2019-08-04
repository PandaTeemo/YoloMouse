#include <Core/System/SystemTools.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Shaders/Default/PixelShader.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Shaders/Default/VertexShader.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        static const Float CLEAR_COLOR[4] = { 0, 0, 0, 0 };
    }

    // public
    //-------------------------------------------------------------------------
    RenderContext::RenderContext():
        // fields: window
        _size                   (0, 0),
        // fields: state
        _initialized            (false),
        _started                (false),
        _cleared                (false),
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
        _sampler                (nullptr),
        _shader_input_layout    (nullptr),
        _vs_constant_buffer     (nullptr),
        _ps_constant_buffer     (nullptr),
        _rasterizer_state       (nullptr)
    {
    }

    RenderContext::~RenderContext()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool RenderContext::Initialize( HWND hwnd, const Vector2l& size )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _size = size;

        // initialize direct3d
        if( !_InitializeD3d(hwnd) )
            return false;

        // initialize blending
        if( !_InitializeBlending() )
            return false;

        // initialize view
        _InitializeView();

        // initialize render timing controller
        _render_timing_controller.Initialize( *_swapchain_output, _size );

        // set initialized
        _initialized = true;

        return true;
    }

    void RenderContext::Shutdown()
    {
        ASSERT( IsInitialized() );
        ASSERT( !IsStarted() );

        // shutdown render timing controller
        if( _render_timing_controller.IsInitialized() )
            _render_timing_controller.Shutdown();

        // shutdown blending
        _ShutdownBlending();

        // shutdown direct3d
        _ShutdownD3d();

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool RenderContext::IsInitialized() const
    {
        return _initialized;
    }

    Bool RenderContext::IsStarted() const
    {
        return _started;
    }

    //-------------------------------------------------------------------------
    ID3D11Device& RenderContext::GetDevice() const
    {
        ASSERT( IsInitialized() );
        return *_device;
    }

    ID3D11DeviceContext& RenderContext::GetDeviceContext() const
    {
        ASSERT( IsInitialized() );
        return *_device_context;
    }

    const RenderTimingController& RenderContext::GetRenderTimingController() const
    {
        return _render_timing_controller;
    }

    const Vector2l& RenderContext::GetSize() const
    {
        return _size;
    }

    //-------------------------------------------------------------------------
    void RenderContext::SetReduceLatency( Bool enabled )
    {
        // set render timing controller fill time option
        _render_timing_controller.SetFillTime( enabled );
    }

    //-------------------------------------------------------------------------
    Bool RenderContext::Resize( const Vector2l& size )
    {
        ASSERT( IsInitialized() );
        HRESULT hresult;

        // set new size
        _size = size;

        // shutdown render timing controller
        _render_timing_controller.Shutdown();

        // shutdown render target view
        _ShutdownD3dRenderTargetView();

        // shutdown depth stencil view
        _ShutdownD3dDepthStencilView();

        // resize swapchain
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

        // reinitialize render timing controller
        _render_timing_controller.Initialize( *_swapchain_output, _size );

        return true;
    }

    //-------------------------------------------------------------------------
    void RenderContext::CommitShaderContants( const VertexShaderConstantValue& vs, const PixelShaderConstantValue& ps )
    {
        // commit vs and ps constant values to gpu
        _device_context->UpdateSubresource(_vs_constant_buffer, 0, NULL, &vs, 0, 0);
        _device_context->UpdateSubresource(_ps_constant_buffer, 0, NULL, &ps, 0, 0);
    }

    //-------------------------------------------------------------------------
    void RenderContext::RenderBegin()
    {
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

        // set blend state
        _device_context->OMSetBlendState(_blend_state, NULL, 0xffffffff);
    }

    void RenderContext::RenderComplete( Bool idle )
    {
        // if idle requested
        if( idle )
        {
            // if not yet cleared
            if( !_cleared )
            {
                // render again to clear
                _Render();

                // set cleared
                _cleared = true;
            }
        }
        else
        {
            // standard render
            _Render();

            // reset cleared
            _cleared = false;
        }

        // end render timing controller
        _render_timing_controller.End();
    }

    // private
    //-------------------------------------------------------------------------
    Bool RenderContext::_InitializeD3d( HWND hwnd )
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
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_1
        };

        // shader layout
        D3D11_INPUT_ELEMENT_DESC SHADER_LAYOUT[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0}, // 12
            {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}, // 16
            {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}, // 12
            {"UV",       0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0}, // 8
        };

        // locals
        DXGI_SWAP_CHAIN_DESC1           swapchain_desc = {};
        IDXGIDevice*                    dxgi_device = nullptr;
        IDXGIAdapter*                   dxgi_adapter = nullptr;
        D3D11_DEPTH_STENCIL_DESC        depthstencil_desc = {};
        D3D11_SAMPLER_DESC              sampler_desc = {};
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
                D3D11_CREATE_DEVICE_SINGLETHREADED|D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY,
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
        swapchain_desc.SwapEffect =             DXGI_SWAP_EFFECT_DISCARD;
        swapchain_desc.BufferCount =            1;
        swapchain_desc.Width =                  _size.x;
        swapchain_desc.Height =                 _size.y;
        swapchain_desc.Format =                 DXGI_FORMAT_B8G8R8A8_UNORM;
        swapchain_desc.BufferUsage =            DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.SampleDesc.Count =       4;
        swapchain_desc.SampleDesc.Quality =     0;
        //swapchain_desc.AlphaMode =              DXGI_ALPHA_MODE_STRAIGHT;
        swapchain_desc.Flags =                  DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // create swapchain
        hresult = _factory->CreateSwapChainForHwnd(_device, hwnd, &swapchain_desc, nullptr, nullptr, &_swapchain);
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
        depthstencil_desc.DepthEnable = FALSE;
        depthstencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthstencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

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

        // describe sampler
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler_desc.MinLOD = 0;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

        // create sampler
        if( (hresult = _device->CreateSamplerState( &sampler_desc, &_sampler )) != S_OK )
            return false;
        _device_context->PSSetSamplers(0, 1, &_sampler);

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
        rasterizer_desc.CullMode = D3D11_CULL_NONE;

        // create rasterizer state
        if( (hresult = _device->CreateRasterizerState(&rasterizer_desc, &_rasterizer_state)) != S_OK )
            return false;

        // set rasterizer state
        _device_context->RSSetState(_rasterizer_state);

        return true;
    }

    Bool RenderContext::_InitializeD3dBuffer( ID3D11Buffer*& buffer, D3D11_USAGE usage, ULong size, UINT BindFlags )
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

    Bool RenderContext::_InitializeD3dDepthStencilView()
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC   depthstencil_view_desc = {};
        D3D11_TEXTURE2D_DESC            depthstencil_texture = {};
        HRESULT                         hresult;

        // set depth stencil state
        _device_context->OMSetDepthStencilState(_depthstencil_state, 0);

        // describe depth stencil view
        depthstencil_view_desc.Format =             DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthstencil_view_desc.ViewDimension =      D3D11_DSV_DIMENSION_TEXTURE2DMS; // to get antialiasing working
        depthstencil_view_desc.Texture2D.MipSlice = 0;

        // describe depth stencil texture
        depthstencil_texture.Width =                _size.x;
        depthstencil_texture.Height =               _size.y;
        depthstencil_texture.MipLevels =            1;
        depthstencil_texture.ArraySize =            1;
        depthstencil_texture.Format =               DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthstencil_texture.SampleDesc.Count =     4;
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

    Bool RenderContext::_InitializeD3dRenderTargetView()
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

    Bool RenderContext::_InitializeBlending()
    {
        D3D11_BLEND_DESC blend_desc = {};
        HRESULT          hresult;

        // describe blend state
        blend_desc.RenderTarget[0].BlendEnable = TRUE;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;

        // create blend state
        if( (hresult = _device->CreateBlendState(&blend_desc, &_blend_state)) != S_OK )
        {
            LOG( "BaseCursor.InitializeBlending.CreateBlendState" );
            return false;
        }

        return true;
    }

    void RenderContext::_InitializeView()
    {
        D3D11_VIEWPORT viewport = {};

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

    //-------------------------------------------------------------------------
    void RenderContext::_ShutdownD3d()
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

        // shutdown sampler
        if( _sampler )
        {
            _sampler->Release();
            _sampler = nullptr;
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

    void RenderContext::_ShutdownD3dDepthStencilView()
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

    void RenderContext::_ShutdownBlending()
    {
        // shutdown blend state
        if( _blend_state )
        {
            _blend_state->Release();
            _blend_state = nullptr;
        }
    }

    void RenderContext::_ShutdownD3dRenderTargetView()
    {
        // shutdown render target view
        if( _render_target_view )
        {
            _render_target_view->Release();
            _render_target_view = nullptr;
        }
    }

    //-------------------------------------------------------------------------
    void RenderContext::_Render()
    {
        // render swapchain buffers to screen. syncing to 1 vblank reduces visual jitter despite
        // manually waiting on vblank and keeping fps at monitor hz.
        _swapchain->Present( 1, 0 );

        // clear backbuffer
        _device_context->ClearRenderTargetView( _render_target_view, CLEAR_COLOR );

        // clear depth/stencil buffers
        _device_context->ClearDepthStencilView( _depthstencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
    }
}

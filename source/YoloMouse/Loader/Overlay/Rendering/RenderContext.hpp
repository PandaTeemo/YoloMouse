#pragma once
#include <Core/Math/Vector2.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Shaders/ShaderTypes.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Support/RenderTimingController.hpp>
#include <dxgi1_2.h>
#include <d3d11.h>

namespace Yolomouse
{
    /**/
    class RenderContext
    {
    public:
        /**/
        RenderContext();
        ~RenderContext();

        /**/
        Bool Initialize( HWND hwnd, const Vector2l& size );
        void Shutdown();

        /**/
        Bool IsInitialized() const;
        Bool IsStarted() const;

        /**/
        ID3D11Device&                   GetDevice() const;
        ID3D11DeviceContext&            GetDeviceContext() const;
        const RenderTimingController&   GetRenderTimingController() const;
        const Vector2l&                 GetSize() const;

        /**/
        void SetReduceLatency( Bool enabled );

        /**/
        Bool Resize( const Vector2l& size );

        /**/
        void CommitShaderContants( const VertexShaderConstantValue& vs, const PixelShaderConstantValue& ps );

        /**/
        void RenderBegin();
        void RenderComplete( Bool idle=false );

    private:
        /**/
        Bool _InitializeD3d( HWND hwnd );
        Bool _InitializeD3dBuffer( ID3D11Buffer*& buffer, D3D11_USAGE usage, ULong size, UINT BindFlags );
        Bool _InitializeD3dDepthStencilView();
        Bool _InitializeD3dRenderTargetView();
        Bool _InitializeBlending();
        void _InitializeView();

        /**/
        void _ShutdownD3d();
        void _ShutdownD3dDepthStencilView();
        void _ShutdownBlending();
        void _ShutdownD3dRenderTargetView();

        /**/
        void _Render();

        // fields: parameters
        Vector2l                    _size;
        // fields: state
        Bool                        _initialized;
        Bool                        _started;
        Bool                        _cleared;
        RenderTimingController      _render_timing_controller;
        // fields: state
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
        ID3D11SamplerState*         _sampler;
        ID3D11InputLayout*          _shader_input_layout;
        ID3D11Buffer*               _vs_constant_buffer;
        ID3D11Buffer*               _ps_constant_buffer;
        ID3D11RasterizerState*      _rasterizer_state;
    };
}

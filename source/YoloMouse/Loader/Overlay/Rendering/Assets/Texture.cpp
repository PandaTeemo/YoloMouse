#include <YoloMouse/Loader/Overlay/Rendering/Assets/Texture.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    Texture::Texture():
        _render_context (nullptr),
        _texture        (nullptr),
        _texture_view   (nullptr)
    {
    }

    Texture::~Texture()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool Texture::Initialize( const InitializeDef& def )
    {
        ASSERT( !IsInitialized() );
        D3D11_TEXTURE2D_DESC    texture_desc = {};
        D3D11_SUBRESOURCE_DATA  subresource_data = {};
        ID3D11Device&           device = def.render_context.GetDevice();

        // set fields
        _render_context = &def.render_context;

        // define texture
	    texture_desc.Width  = def.size.x;
	    texture_desc.Height = def.size.y;
	    texture_desc.MipLevels = 1;
	    texture_desc.ArraySize = 1;
        texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	    texture_desc.SampleDesc.Count = 1;
	    texture_desc.SampleDesc.Quality = 0;
	    texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
	    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	    texture_desc.CPUAccessFlags = 0;
	    texture_desc.MiscFlags = 0;

        // define subresource data
	    subresource_data.pSysMem = def.pixel_data;
	    subresource_data.SysMemPitch = def.size.x * 4;

        // create texture
        if( device.CreateTexture2D( &texture_desc, &subresource_data, &_texture ) == S_OK )
        {
	        D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};

            // define shader resource view desc
	        shader_resource_view_desc.Format = texture_desc.Format;
	        shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	        shader_resource_view_desc.Texture2D.MipLevels = -1;
	        shader_resource_view_desc.Texture2D.MostDetailedMip = 0;

            // create texture view
            if( device.CreateShaderResourceView( _texture, &shader_resource_view_desc, &_texture_view ) == S_OK )
                return true;
        }

    	return false;
    }

    void Texture::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown texture view
        if( _texture_view )
        {
            _texture_view->Release();
            _texture_view = nullptr;
        }

        // shutdown texture 
        if( _texture )
        {
            _texture->Release();
            _texture = nullptr;
        }

        // reset fields
        _render_context = nullptr;
    }

    //-------------------------------------------------------------------------
    Bool Texture::IsInitialized() const
    {
        return _render_context != nullptr;
    }

    //-------------------------------------------------------------------------
    void Texture::Draw() const
    {
        ID3D11DeviceContext& device_context = _render_context->GetDeviceContext();

        // bind texture to pixel shader
        device_context.PSSetShaderResources(0, 1, &_texture_view);
    }
}

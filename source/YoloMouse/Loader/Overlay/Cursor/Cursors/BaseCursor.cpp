#include <Core/Math/Math.hpp>
#include <Core/Math/Transform3.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/BaseCursor.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        const Vector4f VARIATION_TABLE[CURSOR_VARIATION_COUNT] =
        {
            {0,     0.76f, 1,     1},
            {1,     0,     0.58f, 1},
            {0,     1,     0.42f, 1},
            {1,     0.95f, 0,     1},
            {1,     0.57f, 0,     1},
            {1,     0,     0,     1},
            {0,     0.36f, 1,     1},
            {0.63f, 0,     1,     1},
            {0.78f, 0.78f, 0.78f, 1},
            {1,     1,     1,     1},
        };
    }

    // public
    //-------------------------------------------------------------------------
    Bool BaseCursor::IsInitialized() const
    {
        return _render_context != nullptr;
    }

    Bool BaseCursor::IsTextureInitialized() const
    {
        return _texture_view != nullptr;
    }

    Bool BaseCursor::IsGeometryInitialized() const
    {
        return _shape_vertex_buffer != nullptr;
    }

    //-------------------------------------------------------------------------
    CursorId BaseCursor::GetId() const
    {
        return _id;
    }

    CursorVariation BaseCursor::GetVariation() const
    {
        return _variation;
    }

    CursorSize BaseCursor::GetSize() const
    {
        return _size;
    }

    // protected
    //-------------------------------------------------------------------------
    BaseCursor::BaseCursor():
        _render_context         (nullptr),
        _aspect_ratio           (0),
        _id                     (0),
        _variation              (0),
        _size                   (CURSOR_SIZE_DEFAULT),
        _cursor_scale           (static_cast<Float>(CURSOR_SIZE_DEFAULT) * SIZEID_TO_SCALE),
        _auto_scale             (true),
        _projection_matrix      (Matrix4f::IDENTITY()),
        _blend_state            (nullptr),
        _shape_index_count      (0),
        _shape_index_buffer     (nullptr),
        _shape_vertex_buffer    (nullptr),
        _texture                (nullptr),
        _texture_view           (nullptr)
    {
    }

    BaseCursor::~BaseCursor()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool BaseCursor::_InitializeBlending( const BlendingDef& def )
    {
        D3D11_BLEND_DESC blend_desc = {};
        HRESULT          hresult;

        // describe blend state
        blend_desc.RenderTarget[0].BlendEnable = TRUE;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	    blend_desc.RenderTarget[0].SrcBlend = def.src;
	    blend_desc.RenderTarget[0].DestBlend = def.dest;
	    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

        // create blend state
        if( (hresult = _render_context->GetDevice().CreateBlendState(&blend_desc, &_blend_state)) != S_OK )
        {
            LOG( "BaseCursor.InitializeBlending.CreateBlendState" );
            return false;
        }

        // set blend state
        _render_context->GetDeviceContext().OMSetBlendState(_blend_state, NULL, 0xffffffff);

        return true;
    }

    Bool BaseCursor::_InitializeGeometry( const GeometryDef& def )
    {
        D3D11_BUFFER_DESC       index_buffer_desc = {};
        D3D11_BUFFER_DESC       vertex_buffer_desc = {};
        D3D11_SUBRESOURCE_DATA  subresource_data = {};
        HRESULT                 hresult;
        ID3D11Device&           device = _render_context->GetDevice();

        // save index count
        _shape_index_count = def.indices.GetCount() * 3;

        // describe index buffer
        index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        index_buffer_desc.ByteWidth = sizeof(Index) * _shape_index_count;
        index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
 
        // create index buffer
        subresource_data.pSysMem = def.indices.GetMemory();
        if( (hresult = device.CreateBuffer(&index_buffer_desc, &subresource_data, &_shape_index_buffer)) != S_OK )
            return false;
 
        // describe vertex buffer
        vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        vertex_buffer_desc.ByteWidth = sizeof(ShaderVertex) * def.vertices.GetCount();
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_buffer_desc.CPUAccessFlags = 0;
        vertex_buffer_desc.MiscFlags = 0;
 
        // create vertex buffer
        subresource_data.pSysMem = def.vertices.GetMemory();
        if( (hresult = device.CreateBuffer(&vertex_buffer_desc, &subresource_data, &_shape_vertex_buffer)) != S_OK )
            return false;
 
        return true;
    }

    Bool BaseCursor::_InitializeTexture( const TextureDef& def )
    {
        D3D11_TEXTURE2D_DESC            texture2d_desc = {};
        D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
        HRESULT                         hresult;
        D3D11_SUBRESOURCE_DATA          subresource_data = {};
        ID3D11Device&                   device = _render_context->GetDevice();

        // describe subresource data
        subresource_data.pSysMem = def.pixels;
        subresource_data.SysMemPitch = sizeof(Byte4) * def.size.x;

        // describe texture
        texture2d_desc.Width = def.size.x;
        texture2d_desc.Height = def.size.y;
        texture2d_desc.MipLevels = texture2d_desc.ArraySize = 1;
        texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture2d_desc.SampleDesc.Count = 1;
        texture2d_desc.Usage = D3D11_USAGE_DYNAMIC;//D3D11_USAGE_IMMUTABLE
        texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texture2d_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        texture2d_desc.MiscFlags = 0;

        // create texture
        if( (hresult = device.CreateTexture2D( &texture2d_desc, &subresource_data, &_texture )) != S_OK )
            return false;

	    // describe shader resource view
	    shader_resource_view_desc.Format = texture2d_desc.Format;
	    shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	    shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
	    shader_resource_view_desc.Texture2D.MipLevels = -1;

        // create texture view
        if( (hresult = device.CreateShaderResourceView(_texture, &shader_resource_view_desc, &_texture_view)) != S_OK )
            return false;

        return true;
    }

    //-------------------------------------------------------------------------
    void BaseCursor::_ShutdownBlending()
    {
        // shutdown blend state
        if( _blend_state )
        {
            _blend_state->Release();
            _blend_state = nullptr;
        }
    }

    void BaseCursor::_ShutdownGeometry()
    {
        // shutdown shape vertex buffer
        if( _shape_vertex_buffer )
        {
            _shape_vertex_buffer->Release();
            _shape_vertex_buffer = nullptr;
        }

        // shutdown shape index buffer
        if( _shape_index_buffer )
        {
            _shape_index_buffer->Release();
            _shape_index_buffer = nullptr;
        }
    }

    void BaseCursor::_ShutdownTexture()
    {
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
    }

    //-------------------------------------------------------------------------
    Bool BaseCursor::SetCursor( CursorId id, CursorVariation variation, CursorSize size )
    {
        ASSERT( id < CURSOR_ID_COUNT );
        ASSERT( variation < CURSOR_VARIATION_COUNT );
        ASSERT( size >= CURSOR_SIZE_MIN && size < CURSOR_SIZE_COUNT );

        // update id, variation, size
        _id = id;
        _variation = variation;
        _size = size;

        // update cursor scale
        _cursor_scale = static_cast<Float>(size) * SIZEID_TO_SCALE;

        return true;
    }
    
    void BaseCursor::_SetAutoScale( Bool enable )
    {
        _auto_scale = enable;
    }

    //-------------------------------------------------------------------------
    void BaseCursor::_CalculateFaceNormals( Array<ShaderVertex>& vertices, const Array<Index3>& indices )
    {
        // for each index
        for( Index i = 0; i < indices.GetCount(); ++i )
        {
            const Index3& index = indices[i];
            Index i0 = index.a;
            Index i1 = index.b;
            Index i2 = index.c;
            Vector3f v10 = vertices[i1].position - vertices[i0].position;
            Vector3f v20 = vertices[i2].position - vertices[i0].position;
            Vector3f normal = v10.CrossProduct( v20 ).Normal();
            vertices[i0].normal = normal;
            vertices[i1].normal = normal;
            vertices[i2].normal = normal;
        }
    }

    // impl:IOverlayCursor
    //-------------------------------------------------------------------------
    Bool BaseCursor::Initialize( const InitializeDef& def )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _render_context = &def.render_context;
        _aspect_ratio = def.aspect_ratio;

        // initialize camera
        _InitializeCamera();

        // notify initialize subclass
        if( !_OnInitialize() )
            return false;

        return true;
    }

    void BaseCursor::Shutdown()
    {
        ASSERT( IsInitialized() );

        // notify shutdown subclass
        _OnShutdown();

        // reset fields
        _render_context = nullptr;
    }

    //-------------------------------------------------------------------------
    void BaseCursor::Update( const UpdateDef& def )
    {
        Transform3f transform =  Transform3f::IDENTITY();
        Matrix4f    mscale =     Matrix4f::IDENTITY();
        Matrix4f    mtransform = Matrix4f::IDENTITY();
        
        // notify update
        UpdateDef2  sup_def = {
            _render_context->GetRenderTimingController().GetFrameTime(),
            _ps_constant_value.light_vector,
            transform.orientation
        };
        _OnUpdate( sup_def );

        // calculate model matrix at origin and without scale
        transform.ToMatrix4(_vs_constant_value.model);

        // create transform
        transform.translation.x = def.cursor_position.x;
        transform.translation.y = def.cursor_position.y;
        transform.ToMatrix4(mtransform);

        // create scale matrix
        if( _auto_scale )
            mscale.Scale( _cursor_scale, _cursor_scale, _cursor_scale );

        // create mvp and transpose cuz directx sucks, vulkan rules! xD
        _vs_constant_value.mvp = (_projection_matrix * mtransform * mscale).Transpose();

        // update variation color based on variation
        _vs_constant_value.variation_color = VARIATION_TABLE[_variation];

        // update texturing state
        _ps_constant_value.texturing = IsTextureInitialized();

        // commit constant values
        _render_context->CommitShaderContants(_vs_constant_value, _ps_constant_value);
    }

    //-------------------------------------------------------------------------
    void BaseCursor::Draw() const
    {
        // if geometry initialized
        if( IsGeometryInitialized() )
        {
            ID3D11DeviceContext& device_context = _render_context->GetDeviceContext();

            // if texture initialized
            if( IsTextureInitialized() )
            {
                // bind texture to pixel shader
                device_context.PSSetShaderResources(0, 1, &_texture_view);
            }

            // set index buffer
            device_context.IASetIndexBuffer(_shape_index_buffer, DXGI_FORMAT_R32_UINT, 0);

            // set vertex buffer
            UINT stride = sizeof(ShaderVertex);
            UINT offset = 0;
            device_context.IASetVertexBuffers(0, 1, &_shape_vertex_buffer, &stride, &offset);

            // draw shape
            device_context.DrawIndexed(_shape_index_count, 0, 0);
        }
    }

    //-------------------------------------------------------------------------
    void BaseCursor::OnResize( const ResizeDef& def )
    {
        // update fields
        _aspect_ratio = def.aspect_ratio;

        // reinitialize camera
        _InitializeCamera();
    }

    // private
    //-------------------------------------------------------------------------
    void BaseCursor::_InitializeCamera()
    {
        // calculoate projection matrix
        _CalculateOrthoProjection();
    }

    //-------------------------------------------------------------------------
    void BaseCursor::_CalculateOrthoProjection()
    {
        // calculate x,y cells. scale to NDC (normalized device coordinates)
        Float xx = 2.0f / _aspect_ratio;
        Float yy = 2.0f;

        // update projection matrix
        _projection_matrix.Set(
            xx,  0,   0,   0,
            0,  -yy,  0,   0,
            0,   0,   1,   0,
            0,   0,   0,   1
        );
    }
}

#include <YoloMouse/Loader/Overlay/Rendering/Assets/Mesh.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    Mesh::Mesh():
        _render_context (nullptr),
        _index_count    (0),
        _index_buffer   (nullptr),
        _vertex_buffer  (nullptr)
    {
    }

    Mesh::~Mesh()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool Mesh::Initialize( const InitializeDef& def )
    {
        ASSERT( !IsInitialized() );
        D3D11_BUFFER_DESC       index_buffer_desc = {};
        D3D11_BUFFER_DESC       vertex_buffer_desc = {};
        D3D11_SUBRESOURCE_DATA  subresource_data = {};
        HRESULT                 hresult;

        // set fields
        _render_context = &def.render_context;

        // save index count
        _index_count = def.indices.GetCount() * 3;

        // describe index buffer
        index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        index_buffer_desc.ByteWidth = sizeof(Index) * _index_count;
        index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
 
        // create index buffer
        subresource_data.pSysMem = def.indices.GetMemory();
        if( (hresult = _render_context->GetDevice().CreateBuffer(&index_buffer_desc, &subresource_data, &_index_buffer)) != S_OK )
            return false;
 
        // describe vertex buffer
        vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        vertex_buffer_desc.ByteWidth = sizeof(ShaderVertex) * def.vertices.GetCount();
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_buffer_desc.CPUAccessFlags = 0;
        vertex_buffer_desc.MiscFlags = 0;
 
        // create vertex buffer
        subresource_data.pSysMem = def.vertices.GetMemory();
        if( (hresult = _render_context->GetDevice().CreateBuffer(&vertex_buffer_desc, &subresource_data, &_vertex_buffer)) != S_OK )
            return false;
 
        return true;
    }

    void Mesh::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown vertex buffer
        if( _vertex_buffer )
        {
            _vertex_buffer->Release();
            _vertex_buffer = nullptr;
        }

        // shutdown index buffer
        if( _index_buffer )
        {
            _index_buffer->Release();
            _index_buffer = nullptr;
        }

        // reset fields
        _render_context = nullptr;
    }

    //-------------------------------------------------------------------------
    Bool Mesh::IsInitialized() const
    {
        return _render_context != nullptr;
    }

    //-------------------------------------------------------------------------
    void Mesh::Draw() const
    {
        ID3D11DeviceContext& device_context = _render_context->GetDeviceContext();

        // set index buffer
        device_context.IASetIndexBuffer(_index_buffer, DXGI_FORMAT_R32_UINT, 0);

        // set vertex buffer
        UINT stride = sizeof(ShaderVertex);
        UINT offset = 0;
        device_context.IASetVertexBuffers(0, 1, &_vertex_buffer, &stride, &offset);

        // draw shape
        device_context.DrawIndexed(_index_count, 0, 0);
    }
}

#include <Core/Math/Math.hpp>
#include <Core/Math/Transform3.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/ArrowCursor.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        const Float    ROTATIONS_PER_SECOND = 0.2f;
        const Vector4f VARIATION_TABLE[CURSOR_VARIATION_COUNT] =
        {
            {0,     0.76f, 1,     0.6f},
            {1,     0,     0.58f, 0.6f},
            {0,     1,     0.42f, 0.6f},
            {1,     0.95f, 0,     0.6f},
            {1,     0.57f, 0,     0.6f},
            {1,     0,     0,     0.6f},
            {0,     0.36f, 1,     0.6f},
            {0.63f, 0,     1,     0.6f},
            {0.78f, 0.78f, 0.78f, 0.6f},
            {1,     1,     1,     0.6f},
        };
    }

    // public
    //-------------------------------------------------------------------------
    ArrowCursor::ArrowCursor():
        _device                 (nullptr),
        _device_context         (nullptr),
        _aspect_ratio           (0),
        _variation              (0),
        _cursor_scale           (0.03f),
        _projection_matrix      (Matrix4f::IDENTITY()),
        _shape_index_buffer     (nullptr),
        _shape_vertex_buffer    (nullptr),
        _shape_rotater          (0)
    {
    }

    ArrowCursor::~ArrowCursor()
    {
        ASSERT( IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool ArrowCursor::IsInitialized() const
    {
        return _device_context != nullptr;
    }

    //-------------------------------------------------------------------------
    Bool ArrowCursor::Initialize( const InitializeDef& def )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _device = &def.device;
        _device_context = &def.device_context;
        _aspect_ratio = def.aspect_ratio;

        // set default size
        SetCursorSize( CURSOR_SIZE_DEFAULT );

        // initialize scene
        _InitializeScene();

        // initialize geometry
        if( !_InitializeGeometry() )
            return false;

        return true;
    }

    void ArrowCursor::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown geometry
        _ShutdownGeometry();

        // shutdown scene
        _ShutdownScene();

        // reset fields
        _device = nullptr;
        _device_context = nullptr;

    }

    //-------------------------------------------------------------------------
    void ArrowCursor::SetSceneSize( const ResizeDef& def )
    {
        // shutdown scene
        _ShutdownScene();

        // update fields
        _aspect_ratio = def.aspect_ratio;

        // reinitialize scene
        _InitializeScene();
    }

    void ArrowCursor::SetCursorVariation( CursorVariation variation )
    {
        ASSERT( variation < CURSOR_VARIATION_COUNT );

        // update variation
        _variation = variation;
    }

    void ArrowCursor::SetCursorSize( CursorSize size )
    {
        ASSERT( size < CURSOR_SIZE_COUNT );

        // do not support original size
        if( size != CURSOR_SIZE_ORIGINAL )
        {
            // update cursor scale
            _cursor_scale = static_cast<Float>(size) * SIZEID_TO_SCALE;
        }
    }

    //-------------------------------------------------------------------------
    void ArrowCursor::Update( const UpdateDef& def )
    {
        Transform3f transform =     Transform3f::IDENTITY();
        Matrix4f    mscale =        Matrix4f::IDENTITY();
        Matrix4f    mtransform =    Matrix4f::IDENTITY();

        // create shape transform
        transform.translation.x = def.cursor_position.x;
        transform.translation.y = def.cursor_position.y;
        //transform.translation.z = -2;
        transform.orientation = Quaternionf( Math<Float>::PI / 4.0f, -Math<Float>::PI / 8.0f, _shape_rotater );
        transform.ToMatrix4(mtransform);

        // create scale matrix
        mscale.Scale( _cursor_scale, _cursor_scale, _cursor_scale );

        // create model matrix
        def.vs_constant_value.model = mtransform * mscale;

        // and transpose cuz directx sucks, vulkan rules! xD
        def.vs_constant_value.mvp = (_projection_matrix * def.vs_constant_value.model).Transpose();
        //*/

        transform.translation = Vector3f::ZERO();
        transform.ToMatrix4(def.vs_constant_value.model);

        // iterate rotator
        _shape_rotater += ROTATIONS_PER_SECOND * def.frame_time * Math<Float>::PI2;
            
            //0.02f;

        if( _shape_rotater > Math<Float>::PI2 )
            _shape_rotater -= Math<Float>::PI2;



        def.ps_constant_value.light_color = VARIATION_TABLE[_variation];
    }

    //-------------------------------------------------------------------------
    void ArrowCursor::Render() const
    {
        // draw cursor shape
        _device_context->DrawIndexed(12 * 3, 0, 0);
    }

    // private
    //-------------------------------------------------------------------------
    void ArrowCursor::_InitializeScene()
    {
        // calculoate projection matrix
        _CalculateOrthoProjection();
    }

    Bool ArrowCursor::_InitializeGeometry()
    {
        D3D11_BUFFER_DESC       index_buffer_desc = {};
        D3D11_BUFFER_DESC       vertex_buffer_desc = {};
        D3D11_SUBRESOURCE_DATA  subresource_data = {};
        HRESULT                 hresult;

        // define cursor shape vertices
        ShaderVertex vertices[] =
        {
            {{ 0.0f,  0.0f,   0.0f}, {}, {1,0,1,1}},
            {{ 3.0f,  0.866f,-0.5f}, {}, {1,0,1,1}},
            {{ 3.0f, -0.866f,-0.5f}, {}, {1,0,1,1}},

            {{ 0.0f,  0.0f,   0.0f}, {}, {0,1,1,1}},
            {{ 3.0f, -0.866f,-0.5f}, {}, {0,1,1,1}},
            {{ 3.0f,  0.0f,   1.0f}, {}, {0,1,1,1}},

            {{ 0.0f,  0.0f,   0.0f}, {}, {1,1,0,1}},
            {{ 3.0f,  0.0f,   1.0f}, {}, {1,1,0,1}},
            {{ 3.0f,  0.866f,-0.5f}, {}, {1,1,0,1}},

            {{ 3.0f,  0.0f,   1.0f}, {}, {0,1,1,1}},
            {{ 3.0f, -0.866f,-0.5f}, {}, {0,1,1,1}},
            {{ 3.0f,  0.866f,-0.5f}, {}, {0,1,1,1}},
        };

        // define cursor shape indices
        const Index indices[] =
        {
            0, 1, 2,
            3, 4, 5,
            6, 7, 8,
            9, 10, 11,
        };

        //!!!! make funk
        for( int ii = 0; ii < COUNT( vertices ); ++ii )
        {
            vertices[ii].color = Vector4f( 0, 0.7f, 1, 1 );
            vertices[ii].color.w = 0.7f;
        }
        for( int ii = 0; ii < COUNT( indices ); ii+=3 )
        {
            Vector3f v1 = vertices[indices[ii + 1]].position - vertices[indices[ii + 0]].position;
            Vector3f v2 = vertices[indices[ii + 2]].position - vertices[indices[ii + 0]].position;
            Vector3f normal = v1.CrossProduct(v2).Normal();
            vertices[indices[ii + 0]].normal = normal;
            vertices[indices[ii + 1]].normal = normal;
            vertices[indices[ii + 2]].normal = normal;
        }
        //!!!!!!!!!!

        // describe index buffer
        index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        index_buffer_desc.ByteWidth = sizeof(Index) * COUNT(indices) * 3;
        index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
 
        // describe index buffer subresource data
        subresource_data.pSysMem = indices;
        if( (hresult = _device->CreateBuffer(&index_buffer_desc, &subresource_data, &_shape_index_buffer)) != S_OK )
            return false;
 
        // set index buffer
        _device_context->IASetIndexBuffer(_shape_index_buffer, DXGI_FORMAT_R32_UINT, 0);
 
        // describe vertex buffer
        vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        vertex_buffer_desc.ByteWidth = sizeof(ShaderVertex) * COUNT(vertices);
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_buffer_desc.CPUAccessFlags = 0;
        vertex_buffer_desc.MiscFlags = 0;
 
        // describe vertex buffer subresource data
        subresource_data.pSysMem = vertices;
        if( (hresult = _device->CreateBuffer(&vertex_buffer_desc, &subresource_data, &_shape_vertex_buffer)) != S_OK )
            return false;
 
        // set vertex buffer
        UINT stride = sizeof(ShaderVertex);
        UINT offset = 0;
        _device_context->IASetVertexBuffers(0, 1, &_shape_vertex_buffer, &stride, &offset);
 
        return true;
    }

    //-------------------------------------------------------------------------
    void ArrowCursor::_ShutdownScene()
    {
    }

    void ArrowCursor::_ShutdownGeometry()
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

    //-------------------------------------------------------------------------
    void ArrowCursor::_CalculateOrthoProjection()
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

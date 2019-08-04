#include <Core/Math/Math.hpp>
#include <Core/Math/Transform3.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/BaseCursor.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Support/RenderTools.hpp>

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
            {0.5f,  0.5f,  0.5f,  1},
            {1,     1,     1,     1},
        };
    }

    // public
    //-------------------------------------------------------------------------
    BaseCursor::BaseCursor():
        _render_context     (nullptr),
        _id                 (0),
        _variation          (0),
        _size               (CURSOR_SIZE_DEFAULT),
        _cursor_scale       (static_cast<Float>(CURSOR_SIZE_DEFAULT) * SIZEID_TO_SCALE),
        _auto_scale         (true),
        _projection_matrix  (Matrix4f::IDENTITY())
    {
    }

    BaseCursor::~BaseCursor()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool BaseCursor::Initialize( RenderContext& render_context )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _render_context = &render_context;

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
    Bool BaseCursor::IsInitialized() const
    {
        return _render_context != nullptr;
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

    void BaseCursor::SetAspectRatio( Float aspect_ratio )
    {
        // update projection matrix
        RenderTools::BuildOrthoProjectionMatrix( _projection_matrix, aspect_ratio );
    }
    
    //-------------------------------------------------------------------------
    void BaseCursor::Draw( const Vector2f& position )
    {
        // if mesh initialized
        if( _mesh.IsInitialized() )
        {
            Transform3f                 transform =  Transform3f::IDENTITY();
            Matrix4f                    mscale =     Matrix4f::IDENTITY();
            Matrix4f                    mtransform = Matrix4f::IDENTITY();
            VertexShaderConstantValue   vs_constant_value;
            PixelShaderConstantValue    ps_constant_value;
            ID3D11DeviceContext&        device_context = _render_context->GetDeviceContext();
        
            // notify base for updates
            UpdateDef sup_def = {
                _render_context->GetRenderTimingController().GetFrameTime(),
                ps_constant_value.light_vector,
                transform.orientation
            };
            _OnUpdate( sup_def );

            // calculate model matrix at origin and without scale
            transform.ToMatrix4(vs_constant_value.model);

            // create transform
            transform.translation.x = position.x;
            transform.translation.y = position.y;
            transform.ToMatrix4(mtransform);

            // create scale matrix
            if( _auto_scale )
                mscale.Scale( _cursor_scale, _cursor_scale, _cursor_scale );

            // create mvp and transpose cuz directx sucks, vulkan rules! xD
            vs_constant_value.mvp = (_projection_matrix * mtransform * mscale).Transpose();

            // update variation color based on variation
            vs_constant_value.variation_color = VARIATION_TABLE[_variation];

            // if texture initialized
            if( _texture.IsInitialized() )
            {
                // draw texture
                _texture.Draw();

                // set texture type
                ps_constant_value.type = PIXELSHADERTYPE_UNLIT_TEXTURE;
            }
            // else set basic type
            else
                ps_constant_value.type = PIXELSHADERTYPE_LIT_BASIC;

            // commit constant values
            _render_context->CommitShaderContants(vs_constant_value, ps_constant_value);

            // draw mesh
            _mesh.Draw();
        }
    }

    // protected
    //-------------------------------------------------------------------------
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
            Index i0 = index.x;
            Index i1 = index.y;
            Index i2 = index.z;
            Vector3f v10 = vertices[i1].position - vertices[i0].position;
            Vector3f v20 = vertices[i2].position - vertices[i0].position;
            Vector3f normal = v10.CrossProduct( v20 ).Normal();
            vertices[i0].normal = normal;
            vertices[i1].normal = normal;
            vertices[i2].normal = normal;
        }
    }
}

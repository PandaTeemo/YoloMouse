#include <Core/Math/Transform3.hpp>
#include <YoloMouse/Loader/Overlay/Text/TextMesh.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Support/RenderTools.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    TextMesh::TextMesh()
    {
    }

    TextMesh::~TextMesh()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    void TextMesh::Initialize( RenderContext& render_context, const FontAtlas& atlas )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _render_context = &render_context;
        _atlas = &atlas;
    }

    void TextMesh::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown mesh if used
        if( _mesh.IsInitialized() )
            _mesh.Shutdown();

        // reset fields
        _render_context = nullptr;
        _atlas = nullptr;
    }

    //-------------------------------------------------------------------------
    Bool TextMesh::IsInitialized() const
    {
        return _render_context != nullptr;
    }

    //-------------------------------------------------------------------------
    Bool TextMesh::SetText( Float& total_width, const String& text, Float height )
    {
        ASSERT( IsInitialized() );

        // shutdown previous mesh
        if( _mesh.IsInitialized() )
            _mesh.Shutdown();

        // build text mesh
        return _BuildTextMesh( total_width, text, height );
    }

    void TextMesh::SetAspectRatio( Float aspect_ratio )
    {
        // update projection matrix
        RenderTools::BuildOrthoProjectionMatrix( _projection_matrix, aspect_ratio );
    }

    //-------------------------------------------------------------------------
    void TextMesh::Draw( const Vector2f& position, const Vector4f& color ) const
    {
        Transform3f                 transform =  Transform3f::IDENTITY();
        Matrix4f                    mtransform = Matrix4f::IDENTITY();
        VertexShaderConstantValue   vs_constant_value;
        PixelShaderConstantValue    ps_constant_value;
        ID3D11DeviceContext&        device_context = _render_context->GetDeviceContext();
        
        // set model matrix
        vs_constant_value.model = Matrix4f::IDENTITY();

        // create transform
        transform.translation.x = position.x;
        transform.translation.y = position.y;
        transform.ToMatrix4(mtransform);

        // create mvp and transpose cuz directx sucks, vulkan rules! xD
        vs_constant_value.mvp = (_projection_matrix * mtransform).Transpose();
        vs_constant_value.variation_color = color;

        // set texturing state
        ps_constant_value.type = PIXELSHADERTYPE_UNLIT_TEXTURE;

        // commit constant values
        _render_context->CommitShaderContants(vs_constant_value, ps_constant_value);

        // draw texture
        _atlas->GetTexture().Draw();

        // draw mesh
        _mesh.Draw();
    }

    // private
    //-------------------------------------------------------------------------
    Bool TextMesh::_BuildTextMesh( Float& total_width, const String& text, Float height )
    {
        /* shows font atlas
        Float s = 0.4f;
        ShaderVertex VERTICES[] = {
            { { 0, 0, 0 }, {1,0,0,1}, {0,0,1}, {0,0} },
            { { s, 0, 0 }, {1,1,0,1}, {0,0,1}, {1,0} },
            { { s, s, 0 }, {1,0,1,1}, {0,0,1}, {1,1} },
            { { 0, s, 0 }, {0,1,1,1}, {0,0,1}, {0,1} },
        };
        Index3 INDICES[] = {
            { 0, 1, 2 },
            { 0, 2, 3 },
        };
        return _mesh.Initialize( {*_render_context, Array<Index3>( INDICES, 2 ), Array<ShaderVertex>( VERTICES, 4 )} );
        //*/
        SimpleArray<ShaderVertex>   vertices;
        SimpleArray<Index3>         indices;
        ULong                       n = text.GetCount();
        Float                       z = 0;
        const Vector3f              normal(0,0,1);

        // preallocate vertices and indices
        indices.SetCount(n * 2);
        vertices.SetCount(n * 4);

        // set initial total width
        total_width = 0;

        // for each character
        for( Index ci = 0, vi = 0, ii = 0; ci < text.GetCount(); ++ci, vi += 4, ii += 2 )
        {
            // get font glyph
            const FontAtlas::Glyph* glyph = _atlas->GetGlyph( text[ci] );

            // ignore if not found
            if( glyph != nullptr )
            {
                ShaderVertex& v0 = vertices[vi];
                ShaderVertex& v1 = vertices[vi + 1];
                ShaderVertex& v2 = vertices[vi + 2];
                ShaderVertex& v3 = vertices[vi + 3];
                Index3&       i0 = indices[ii];
                Index3&       i1 = indices[ii + 1];

                // get character width 
                Float width = (glyph->size.x / glyph->size.y) * height;

                // build positions
                v0.position.Set( total_width, 0, z );
                v1.position.Set( total_width, height, z );
                v2.position.Set( total_width + width, height, z );
                v3.position.Set( total_width + width, 0, z );

                // build normals
                v0.normal = normal;
                v1.normal = normal;
                v2.normal = normal;
                v3.normal = normal;

                // build uvs
                v0.uv = glyph->uv_min;
                v1.uv.Set( glyph->uv_min.x, glyph->uv_max.y );
                v2.uv = glyph->uv_max;
                v3.uv.Set( glyph->uv_max.x, glyph->uv_min.y );

                // build variable color
                v0.color = v1.color = v2.color = v3.color = Vector4f( 0, 0, 0, 1 );

                // build indices
                i0.Set( vi, vi + 2, vi + 1 );
                i1.Set( vi, vi + 3, vi + 2 );

                // increment display offset
                total_width += width;
            }
        }

        // initialize mesh
        return _mesh.Initialize( {*_render_context, indices, vertices} );
    }
}

#pragma once
#include <Core/Container/String.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Assets/Mesh.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>
#include <YoloMouse/Loader/Overlay/Text/FontAtlas.hpp>

namespace Yolomouse
{
    /**/
    class TextMesh
    {
    public:
        /**/
        TextMesh();
        ~TextMesh();

        /**/
        void Initialize( RenderContext& render_context, const FontAtlas& atlas );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        Bool SetText( Float& total_width, const String& text, Float height );
        void SetAspectRatio( Float aspect_ratio );

        /**/
        void Draw( const Vector2f& position, const Vector4f& color ) const;

    private:
        /**/
        Bool _BuildTextMesh( Float& total_width, const String& text, Float height );

        // fields: parameters
        RenderContext*      _render_context;
        const FontAtlas*    _atlas;
        // fields: state
        Mesh                _mesh;
        Matrix4f            _projection_matrix;
    };
}

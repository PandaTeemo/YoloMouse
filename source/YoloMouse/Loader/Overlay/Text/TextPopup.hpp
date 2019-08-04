#pragma once
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>
#include <YoloMouse/Loader/Overlay/Text/TextMesh.hpp>

namespace Yolomouse
{
    /**/
    class TextPopup
    {
    public:
        /**/
        TextPopup();
        ~TextPopup();

        /**/
        Bool Initialize( RenderContext& render_context );
        void Shutdown();

        /**/
        Bool IsInitialized() const;
        Bool IsActive() const;

        /**/
        Bool SetText( const String& text, const Vector2f& position, ULong timeout );
        void SetAspectRatio( Float aspect_ratio );

        /**/
        void Draw();

    private:
        // enums
        enum State: Byte
        {
            STATE_IDLE,
            STATE_VISIBLE,
            STATE_FADING
        };
        /**/
        Bool _InitializeBackgrounMesh( const Vector2f& size );

        /**/
        void _DrawPopup() const;
        void _DrawBackgrounMesh( const Vector2f& position, const Vector4f& color ) const;

        // fields: parameters
        RenderContext*  _render_context;
        Vector2f        _position;
        ULong           _timeout;
        // fields: state
        State           _state;
        Float           _fade;
        Float           _message_width;
        Vector2f        _message_position;
        Vector2f        _header_position;
        //TODO2: put into view/scene constantbuffer and call for whole sene
        Matrix4f        _projection_matrix;
        // fields: objects
        FontAtlas       _font_atlas;
        TextMesh        _message_mesh;
        TextMesh        _header_mesh;
        Mesh            _background_mesh;
    };
}

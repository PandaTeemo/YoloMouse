#include <Core/Math/Transform3.hpp>
#include <Core/System/SystemTools.hpp>
#include <YoloMouse/Loader/Overlay/Text/TextPopup.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Support/RenderTools.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        static constexpr WChar*     FONT_FAMILY =       L"Arial";
        static constexpr Float      MESSAGE_SIZE =      0.02f;
        static constexpr Char*      HEADER_TEXT =       "Yolomouse";
        static constexpr Float      HEADER_SIZE =       0.01f;
        static constexpr Float      MESSAGE_OFFSET =    0.005f;
        static const     Vector2f   HEADER_PADDING      ( 0.003f, 0.002f );
        static const     Vector2f   BACKGROUND_PADDING  ( 0.03f, 0.017f );
        static const     Vector4f   BACKGROUND_COLOR    ( 0, 0, 0, 0.5f );
        static const     Vector4f   MESSAGE_COLOR       ( 1, 1, 1, 1 );
        static const     Vector4f   HEADER_COLOR        ( 0, 0.8f, 1, 1 );
        static constexpr Float      FADE_RATE =         0.5f;
    }

    // public
    //-------------------------------------------------------------------------
    TextPopup::TextPopup():
        _render_context     (nullptr),
        _position           (0,0),
        _timeout            (0),
        _state              (STATE_IDLE),
        _fade               (0),
        _message_width      (0),
        _message_position   (0,0),
        _header_position    (0,0),
        _projection_matrix  (Matrix4f::IDENTITY())
    {
    }

    TextPopup::~TextPopup()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool TextPopup::Initialize( RenderContext& render_context )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _render_context = &render_context;

        // initialize font atlas
        if( !_font_atlas.Initialize( render_context, FONT_FAMILY ) )
            return false;

        // initialize text mesh
        _message_mesh.Initialize( render_context, _font_atlas );

        // initialize header mesh
        _header_mesh.Initialize( render_context, _font_atlas );

        return true;
    }

    void TextPopup::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown header mesh
        if( _header_mesh.IsInitialized() )
            _header_mesh.Shutdown();

        // shutdown message mesh
        if( _message_mesh.IsInitialized() )
            _message_mesh.Shutdown();

        // shutdown background mesh
        if( _background_mesh.IsInitialized() )
            _background_mesh.Shutdown();

        // shutdown font atlas
        if( _font_atlas.IsInitialized() )
            _font_atlas.Shutdown();

        // reset fields
        _render_context = nullptr;
    }

    //-------------------------------------------------------------------------
    Bool TextPopup::IsInitialized() const
    {
        return _render_context != nullptr;
    }

    Bool TextPopup::IsActive() const
    {
        return _state != STATE_IDLE;
    }

    //-------------------------------------------------------------------------
    Bool TextPopup::SetText( const String& text, const Vector2f& position, ULong timeout )
    {
        ASSERT( IsInitialized() );
        Float fdummy;

        // set fields
        _position = position;
        _timeout = SystemTools::GetTimeMs() + timeout;

        // update build header mesh
        if( !_header_mesh.SetText( fdummy, HEADER_TEXT, HEADER_SIZE ) )
            return false;

        // update build text mesh
        if( !_message_mesh.SetText( _message_width, text, MESSAGE_SIZE ) )
            return false;

        // shutdown previous background mesh
        if( _background_mesh.IsInitialized() )
            _background_mesh.Shutdown();

        // initialize background mesh
        if( !_InitializeBackgrounMesh( {_message_width + BACKGROUND_PADDING.x, MESSAGE_SIZE + BACKGROUND_PADDING.y} ) )
            return false;

        // calculate message display position
        _message_position.Set(position.x - _message_width / 2.0f, position.y + MESSAGE_OFFSET );

        // calculate header display position
        _header_position.Set(position.x - (_message_width + BACKGROUND_PADDING.x) / 2.0f + HEADER_PADDING.x, position.y - BACKGROUND_PADDING.y / 2.0f + HEADER_PADDING.y);

        // set initial fade
        _fade = 1;

        // enter visible state
        _state = STATE_VISIBLE;

        return true;
    }

    void TextPopup::SetAspectRatio( Float aspect_ratio )
    {
        // update message aspect ratio
        _message_mesh.SetAspectRatio( aspect_ratio );

        // update header aspect ratio
        _header_mesh.SetAspectRatio( aspect_ratio );

        // update projection matrix
        RenderTools::BuildOrthoProjectionMatrix( _projection_matrix, aspect_ratio );
    }

    //-------------------------------------------------------------------------
    void TextPopup::Draw()
    {
        // by state
        switch( _state )
        {
        case STATE_IDLE:
            break;

        case STATE_VISIBLE:
            // if timeout expired, enter fading state
            if( SystemTools::GetTimeMs() > _timeout )
                _state = STATE_FADING;

            // draw popup
            _DrawPopup();
            break;

        case STATE_FADING:
            // fade relative to fame time
            _fade -= _render_context->GetRenderTimingController().GetFrameTime() * FADE_RATE;

            // if still fading, draw popup, else enter idle state
            if( _fade > 0 )
                _DrawPopup();
            else
                _state = STATE_IDLE;
            break;
        }
    }

    // private
    //-------------------------------------------------------------------------
    Bool TextPopup::_InitializeBackgrounMesh( const Vector2f& size )
    {
        Float w = size.x / 2;
        Float h = size.y / 2;

        ShaderVertex vertices[] = {
            { { -w, -h, 0 }, { 0, 0, 0, 1 }, {0,0,1}, {0,0} },
            { {  w, -h, 0 }, { 0, 0, 0, 1 }, {0,0,1}, {1,0} },
            { {  w,  h, 0 }, { 0, 0, 0, 1 }, {0,0,1}, {1,1} },
            { { -w,  h, 0 }, { 0, 0, 0, 1 }, {0,0,1}, {0,1} },
        };
        Index3 indices[] = {
            { 0, 1, 2 },
            { 0, 2, 3 },
        };

        // initialize mesh
        return _background_mesh.Initialize( {
            *_render_context,
            Array<Index3>(indices, 2),
            Array<ShaderVertex>(vertices, 4)} );
    }

    //-------------------------------------------------------------------------
    void TextPopup::_DrawPopup() const
    {
        Vector4f fade_color(1, 1, 1, _fade);

        // draw background mesh
        _DrawBackgrounMesh(_position, BACKGROUND_COLOR * fade_color);

        // draw header text
        _header_mesh.Draw(_header_position, HEADER_COLOR * fade_color);

        // draw message text
        _message_mesh.Draw(_message_position, MESSAGE_COLOR * fade_color);
    }

    void TextPopup::_DrawBackgrounMesh( const Vector2f& position, const Vector4f& color ) const
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
        transform.translation.y = position.y + MESSAGE_SIZE / 2.0f;
        transform.ToMatrix4(mtransform);

        // create mvp and transpose cuz directx sucks, vulkan rules! xD
        vs_constant_value.mvp = (_projection_matrix * mtransform).Transpose();
        vs_constant_value.variation_color = color;

        // set texturing state
        ps_constant_value.type = PIXELSHADERTYPE_UNLIT_BASIC;

        // commit constant values
        _render_context->CommitShaderContants(vs_constant_value, ps_constant_value);

        // draw mesh
        _background_mesh.Draw();
    }
}

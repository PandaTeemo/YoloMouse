#pragma once
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/IOverlayCursor.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Types.hpp>

namespace Yolomouse
{
    /**/
    class BaseCursor:
        public IOverlayCursor
    {
    public:
        // types
        struct BlendingDef
        {
            D3D11_BLEND src;
            D3D11_BLEND dest;
        };

        struct GeometryDef
        {
            Array<ShaderVertex> vertices;
            Array<Index3>       indices;
        };

        struct TextureDef
        {
            Vector2l    size;
            void*       pixels;
        };

        struct UpdateDef2
        {
            Float        frame_time;
            Vector3f&    light_vector;
            Quaternionf& orientation;
        };

        struct DrawDef
        {
            Index begin;
            Index end;
        };

        /**/
        BaseCursor();
        ~BaseCursor();

        // impl:IOverlayCursor
        /**/
        Bool Initialize( const InitializeDef& def );
        void Shutdown();

        /**/
        Bool IsInitialized() const;
        Bool IsTextureInitialized() const;
        Bool IsGeometryInitialized() const;

        /**/
        CursorId        GetId() const;
        CursorVariation GetVariation() const;
        CursorSize      GetSize() const;

        /**/
        virtual Bool SetCursor( CursorId id, CursorVariation variation, CursorSize size );

        /**/
        void Update( const UpdateDef& def );

        /**/
        void Draw() const;

        /**/
        void OnResize( const ResizeDef& def );

    protected:
        /**/
        virtual Bool _OnInitialize() = 0;
        Bool _InitializeBlending( const BlendingDef& def );
        Bool _InitializeGeometry( const GeometryDef& def );
        Bool _InitializeTexture( const TextureDef& def );

        /**/
        virtual void _OnShutdown() = 0;
        void _ShutdownBlending();
        void _ShutdownGeometry();
        void _ShutdownTexture();

        /**/
        virtual void _OnUpdate( UpdateDef2& def ) = 0;

        /**/
        void _SetAutoScale( Bool enable );

        /**/
        void _CalculateFaceNormals( Array<ShaderVertex>& vertices, const Array<Index3>& indices );

    private:
        // constants
        // SIZEID_TO_SCALE * SizeId = cursor size relative to resolution height
        static constexpr Float SIZEID_TO_SCALE = 0.0032f;

        /**/
        void _InitializeCamera();

        /**/
        void _CalculateOrthoProjection();

        // fields: parameters
        RenderContext*              _render_context;
        Float                       _aspect_ratio;
        CursorId                    _id;
        CursorVariation             _variation;
        CursorSize                  _size;
        Float                       _cursor_scale;
        Bool                        _auto_scale;
        // fields: state
        ID3D11BlendState*           _blend_state;
        Index                       _shape_index_count;
        ID3D11Buffer*               _shape_index_buffer;
        ID3D11Buffer*               _shape_vertex_buffer;
        ID3D11Texture2D*            _texture;
        ID3D11ShaderResourceView*   _texture_view;
        Matrix4f                    _projection_matrix;
        VertexShaderConstantValue   _vs_constant_value;
        PixelShaderConstantValue    _ps_constant_value;
   };
}
#pragma once
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/IOverlayCursor.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Assets/Mesh.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Assets/Texture.hpp>

namespace Yolomouse
{
    /**/
    class BaseCursor:
        public IOverlayCursor
    {
    public:
        // types
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

        struct UpdateDef
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

        /**/
        Bool Initialize( RenderContext& render_context );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        CursorId        GetId() const;
        CursorVariation GetVariation() const;
        CursorSize      GetSize() const;

        /**/
        virtual Bool SetCursor( CursorId id, CursorVariation variation, CursorSize size );
        void         SetAspectRatio( Float aspect_ratio );

        /**/
        void Draw( const Vector2f& position );

    protected:
        /**/
        virtual Bool _OnInitialize() = 0;
        virtual void _OnShutdown() = 0;
        virtual void _OnUpdate( UpdateDef& def ) = 0;

        /**/
        void _SetAutoScale( Bool enable );

        /**/
        void _CalculateFaceNormals( Array<ShaderVertex>& vertices, const Array<Index3>& indices );

        // fields
        Mesh            _mesh;
        Texture         _texture;
        RenderContext*  _render_context;
 
    private:
        // constants
        // SIZEID_TO_SCALE * SizeId = cursor size relative to resolution height
        static constexpr Float SIZEID_TO_SCALE = 0.0032f;

        // fields: parameters
        CursorId                    _id;
        CursorVariation             _variation;
        CursorSize                  _size;
        Float                       _cursor_scale;
        Bool                        _auto_scale;
        // fields: state
        Matrix4f                    _projection_matrix;
   };
}

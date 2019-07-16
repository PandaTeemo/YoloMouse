#pragma once
#include <Core/Math/Matrix4.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/IOverlayCursor.hpp>

namespace Yolomouse
{
    /**/
    class ArrowCursor:
        public IOverlayCursor
    {
    public:
        /**/
        ArrowCursor();
        ~ArrowCursor();

        /**/
        // impl:IOverlayCursor
        Bool IsInitialized() const;

    private:
        // impl:IOverlayCursor
        /**/
        Bool Initialize( const InitializeDef& def );
        void Shutdown();

        /**/
        void SetSceneSize( const ResizeDef& def );
        void SetCursorVariation( CursorVariation variation );
        void SetCursorSize( CursorSize size );

        /**/
        void Update( const UpdateDef& def );

        /**/
        void Render() const;

    private:
        // constants
        // SIZEID_TO_SCALE * SizeId = cursor size relative to resolution height
        static constexpr Float SIZEID_TO_SCALE = 0.0032f;

        /**/
        void _InitializeScene();
        Bool _InitializeGeometry();

        /**/
        void _ShutdownScene();
        void _ShutdownGeometry();

        /**/
        void _CalculateOrthoProjection();

        // fields: parameters
        ID3D11Device*           _device;
        ID3D11DeviceContext*    _device_context;
        Float                   _aspect_ratio;
        CursorVariation         _variation;
        Float                   _cursor_scale;
        // fields: state
        Matrix4f                _projection_matrix;
        ID3D11Buffer*           _shape_index_buffer;
        ID3D11Buffer*           _shape_vertex_buffer;
        Float                   _shape_rotater;
    };
}
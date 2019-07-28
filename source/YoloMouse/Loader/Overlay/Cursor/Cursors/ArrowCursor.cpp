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
        const Float     ROTATION_SPEED = 0.13f;   // rotations/sec
        const Vector3f  LIGHT_VECTOR = Vector3f(0,-1,0).Normal();

        // vertices
        ShaderVertex VERTEX_DATA[] =
        {
            {{ 0.0f,  0.0f,   0.0f}, {0,0,0,1}},
            {{ 3.2f,  0.866f,-0.5f}, {0,0,0,0.5f}},
            {{ 3.2f, -0.866f,-0.5f}, {0,0,0,0.5f}},

            {{ 0.0f,  0.0f,   0.0f}, {0,0,0,1}},
            {{ 3.2f, -0.866f,-0.5f}, {0,0,0,0.5f}},
            {{ 3.2f,  0.0f,   1.0f}, {0,0,0,0.5f}},

            {{ 0.0f,  0.0f,   0.0f}, {0,0,0,1}},
            {{ 3.2f,  0.0f,   1.0f}, {0,0,0,0.5f}},
            {{ 3.2f,  0.866f,-0.5f}, {0,0,0,0.5f}},

            {{ 3.2f,  0.0f,   1.0f}, {0,0,0,0.5f}},
            {{ 3.2f, -0.866f,-0.5f}, {0,0,0,0.5f}},
            {{ 3.2f,  0.866f,-0.5f}, {0,0,0,0.5f}},
        };
        Array<ShaderVertex> VERTICES( VERTEX_DATA, COUNT(VERTEX_DATA) );

        // indices
        Index3 INDEX_DATA[] =
        {
            { 0, 1, 2 },
            { 3, 4, 5 },
            { 6, 7, 8 },
            { 9, 10, 11 }
        };
        Array<Index3> INDICES( INDEX_DATA, COUNT(INDEX_DATA) );
    }

    // public
    //-------------------------------------------------------------------------
    ArrowCursor::ArrowCursor():
        _rotater(0)
    {
    }

    ArrowCursor::~ArrowCursor()
    {
    }

    //-------------------------------------------------------------------------
    Bool ArrowCursor::_OnInitialize()
    {
        // calculate face normals
        _CalculateFaceNormals( VERTICES, INDICES );

        // initialize blending
        if( !_InitializeBlending({D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA}) )
            return false;

        // initialize geometry
        if( !_InitializeGeometry({VERTICES, INDICES}) )
            return false;

        return true;
    }

    void ArrowCursor::_OnShutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown geometry
        _ShutdownGeometry();

        // shutdown blending
        _ShutdownBlending();
    }

    void ArrowCursor::_OnUpdate( UpdateDef2& def )
    {
        // update light vector
        def.light_vector = LIGHT_VECTOR;

        // update transform
        def.orientation = Quaternionf( Math<Float>::PI / 4.0f, -Math<Float>::PI / 8.0f, _rotater );

        // iterate rotator
        _rotater += ROTATION_SPEED * def.frame_time * Math<Float>::PI2;
        if( _rotater > Math<Float>::PI2 )
            _rotater -= Math<Float>::PI2;
    }
}

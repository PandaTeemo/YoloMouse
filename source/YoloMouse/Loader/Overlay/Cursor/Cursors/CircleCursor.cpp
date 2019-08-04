#include <Core/Math/Math.hpp>
#include <Core/Math/Transform3.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/CircleCursor.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        const Float ROTATION_SPEED =    0.06f;  // rotations/sec
        const ULong SEGMENT_COUNT =     32;
        const Float RING_RADIUS0 =      1.10f;
        const Float RING_RADIUS1 =      1.30f;
        const Float RING_HEIGHT =       0.8f;
        const Float CENTER_RADIUS =     0.12f;
        const Float CENTER_HEIGHT =     0.4f;
        const Float LIGHT_HEIGHT =      1.2f;

        const ULong VERTEX_COUNT =      (SEGMENT_COUNT * 3) + 1 + 12;    // 3x per ring segment, 1 for origin, 12 for center
        const ULong INDEX_COUNT =       SEGMENT_COUNT * 3 + 4;           // 3x per ring segment, 4 for center
    }

    // public
    //-------------------------------------------------------------------------
    CircleCursor::CircleCursor():
        _rotater(0)
    {
    }

    CircleCursor::~CircleCursor()
    {
    }

    //-------------------------------------------------------------------------
    Bool CircleCursor::_OnInitialize()
    {
        // initialize mesh
        if( !_InitializeMesh() )
            return false;

        return true;
    }

    void CircleCursor::_OnShutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown mesh
        _mesh.Shutdown();
    }

    void CircleCursor::_OnUpdate( UpdateDef& def )
    {
        // calculate rotation
        Float ux = Math<Float>::Cos(_rotater);
        Float uy = Math<Float>::Sin(_rotater);

        // update light vector
        def.light_vector = Vector3f(ux, uy, LIGHT_HEIGHT).Normal();

        // iterate rotator
        _rotater += ROTATION_SPEED * def.frame_time * Math<Float>::PI2;
        if( _rotater > Math<Float>::PI2 )
            _rotater -= Math<Float>::PI2;
    }

    // private
    //-------------------------------------------------------------------------
    Bool CircleCursor::_InitializeMesh()
    {
        // constants
        static const ULong VINDEX_RING_ORIGIN =    SEGMENT_COUNT * 3;
        static const ULong VINDEX_CENTER_BEGIN =   VINDEX_RING_ORIGIN + 1;
        static const ULong INDEX_CENTER_BEGIN =    SEGMENT_COUNT * 3;
        static const Vector2f CENTER_OFFSETS[4] =
        {
            { 1,  0},
            { 0,  1},
            {-1,  0},
            { 0, -1},
        };

        // buffers
        ShaderVertex vertex_data[VERTEX_COUNT];
        Index3       index_data[INDEX_COUNT];

        // set ring origin vertex
        ShaderVertex& v_ring_origin = vertex_data[VINDEX_RING_ORIGIN];
        v_ring_origin.position.Set( 0, 0, 0 );
        v_ring_origin.color.Set( 0, 0, 0, 0 );
        v_ring_origin.normal.Set( 0, 0, 1 );

        // for each ring segment
        for( Index i = 0; i < SEGMENT_COUNT; ++i )
        {
            Index         vi = i * 3;
            Index         ii = i * 3;
            ShaderVertex& v0a = vertex_data[vi];
            ShaderVertex& v0b = vertex_data[vi + 1];
            ShaderVertex& v1 = vertex_data[vi + 2];
            Index3&       i0 = index_data[ii];
            Index3&       i1 = index_data[ii + 1];
            Index3&       i2 = index_data[ii + 2];
            Float         t = static_cast<Float>(i) * Math<Float>::PI2 / static_cast<Float>(SEGMENT_COUNT);
            Float         ux = Math<Float>::Cos(t);
            Float         uy = Math<Float>::Sin(t);

            // create ring0 and 1 vertices
            v0a.position.Set( ux * RING_RADIUS0, uy * RING_RADIUS0, RING_HEIGHT );
            v0a.color.Set( 0, 0, 0, 0.3f );
            v0a.normal = Vector3f( -ux * RING_HEIGHT * 4, -uy * RING_HEIGHT * 4, RING_RADIUS0 ).Normal();
            v0b = v0a;
            v0b.color.Set( 0, 0, 0, 1 );
            v0b.normal = Vector3f( ux * RING_HEIGHT, uy * RING_HEIGHT, RING_RADIUS1-RING_RADIUS0 ).Normal();
            v1.position.Set( ux * RING_RADIUS1, uy * RING_RADIUS1, 0 );
            v1.color.Set( 0, 0, 0, 1 );
            v1.normal = v0b.normal;

            // create polys
            i0.x = VINDEX_RING_ORIGIN;
            i0.y = vi;
            i0.z = i == (SEGMENT_COUNT - 1) ? 0 : vi + 3;

            i1.x = vi + 1;
            i1.y = vi + 2;
            i1.z = i0.z + 2;

            i2.x = vi + 1;
            i2.y = i0.z + 2;
            i2.z = i0.z + 1;
        }

        // for each center vertex
        for( Index i = 0; i < 4; ++i )
        {
            Index         vi = VINDEX_CENTER_BEGIN + (i * 3);
            Index         ii = INDEX_CENTER_BEGIN + i;
            ShaderVertex& v0 = vertex_data[vi];
            ShaderVertex& v1 = vertex_data[vi + 1];
            ShaderVertex& v2 = vertex_data[vi + 2];
            Index3&       i0 = index_data[ii];

            // get offsets
            const Vector2f off0 = CENTER_OFFSETS[i];
            const Vector2f off1 = CENTER_OFFSETS[(i + 1) % 4];

            // create vertices
            v0.position.Set( off0.x * CENTER_RADIUS, off0.y * CENTER_RADIUS, 0 );
            v1.position.Set( off1.x * CENTER_RADIUS, off1.y * CENTER_RADIUS, 0 );
            v2.position.Set( 0, 0, CENTER_HEIGHT );
            v0.color = v1.color = v2.color = Vector4f( 1, 1, 1, 1 );
            v0.normal = v1.normal = v2.normal = Vector3f(off0.x * CENTER_HEIGHT, off0.y * CENTER_HEIGHT, CENTER_RADIUS).Normal();

            // create poly
            i0.x = vi;
            i0.y = vi + 1;
            i0.z = vi + 2;
        }

        // initialize geometry
        return BaseCursor::_mesh.Initialize({
            *_render_context,
            Array<Index3>      (index_data, INDEX_COUNT),
            Array<ShaderVertex>(vertex_data, VERTEX_COUNT)
        });
    }
}

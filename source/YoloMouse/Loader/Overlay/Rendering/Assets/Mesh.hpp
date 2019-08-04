#pragma once
#include <YoloMouse/Loader/Overlay/Rendering/Types.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>

namespace Yolomouse
{
    /**/
    class Mesh
    {
    public:
        // types
        struct InitializeDef
        {
            RenderContext&      render_context;
            Array<Index3>       indices;
            Array<ShaderVertex> vertices;
        };

        /**/
        Mesh();
        ~Mesh();

        /**/
        Bool Initialize( const InitializeDef& def );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        void Draw() const;

    private:
        // fields
        RenderContext*  _render_context;
        Index           _index_count;
        ID3D11Buffer*   _index_buffer;
        ID3D11Buffer*   _vertex_buffer;
    };
}

#pragma once
#include <YoloMouse/Loader/Overlay/Rendering/Types.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>

namespace Yolomouse
{
    /**/
    class Texture
    {
    public:
        // types
        struct InitializeDef
        {
            RenderContext&  render_context;
            const void*     pixel_data;
            Vector2l        size;
        };

        /**/
        Texture();
        ~Texture();

        /**/
        Bool Initialize( const InitializeDef& def );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        void Draw() const;

    private:
        // fields
        RenderContext*              _render_context;
        ID3D11Texture2D*            _texture;
        ID3D11ShaderResourceView*   _texture_view;
    };
}

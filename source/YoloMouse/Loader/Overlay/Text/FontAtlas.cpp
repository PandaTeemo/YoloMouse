//TODO5: SDF support
#include <Core/Support/Tools.hpp>
#include <YoloMouse/Loader/Overlay/Text/FontAtlas.hpp>
#include <YoloMouse/Loader/Overlay/Text/FontMapBuilder.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        static constexpr ULong  MAP_SIZE =      512;    // must be power of 2 or the universe will implode
        static constexpr Float  FONT_SIZE =     64.0f;
        static constexpr Char   CHARACTERS[] =  " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,-=+[]";
    }

    // public
    //-------------------------------------------------------------------------
    FontAtlas::FontAtlas():
        _render_context (nullptr)
    {
    }

    FontAtlas::~FontAtlas()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool FontAtlas::Initialize( RenderContext& render_context, const WChar* font_name )
    {
        ASSERT( !IsInitialized() );
        FontMapBuilder  font_map_builder;
        Bool            status = false;

        // set fields
        _render_context = &render_context;

        // initialize font map builder
        if( font_map_builder.Initialize( font_name, FONT_SIZE, MAP_SIZE ) )
        {
            Vector2f    offset( 1, 1 );
            Float       row_height = 0;
            Float       fmap_size = static_cast<Float>( MAP_SIZE );

            // for each character
            for( const Char* character = CHARACTERS; *character; ++character )
            {
                // get glyph entry
                Glyph& glyph = _glyphs[*character];

                // get character sizing
                if( font_map_builder.GetCharacterSize( glyph.size, *character ) )
                {
                    // update row height
                    row_height = Tools::Max( row_height, glyph.size.y );
                    
                    // if offset over map size
                    if( ( offset.x + glyph.size.x ) > fmap_size )
                    {
                        // reset x offset to 1 (instead of 0 to avoid overlap)
                        offset.x = 1;

                        // update y offset + padding to avoid overlap
                        offset.y += row_height + 1;

                        // reset row height
                        row_height = 0;
                    }

                    // write character to map and get its size
                    if( font_map_builder.WriteCharacter( offset, *character ) )
                    {
                        // update glyph entry
                        glyph.active = true;
                        glyph.offset = offset;
                        glyph.uv_min = offset / fmap_size;
                        glyph.uv_max = (offset + glyph.size) / fmap_size;

                        // update x offset + padding to avoid overlap
                        offset.x += glyph.size.x + 4;
                    }
                }
            }

            // build directx map
            if( font_map_builder.ToDxMap( _texture, *_render_context ) )
                status = true;

            // shutdown builder
            font_map_builder.Shutdown();
        }

        return status;

        return true;
    }

    void FontAtlas::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown texture
        _texture.Shutdown();

        // reset fields
        _render_context = nullptr;
    }

    //-------------------------------------------------------------------------
    Bool FontAtlas::IsInitialized() const
    {
        return _render_context != nullptr;
    }

    //-------------------------------------------------------------------------
    const FontAtlas::Glyph* FontAtlas::GetGlyph( Char character ) const
    {
        ASSERT( character >= 0 && character < 0x80 );

        // get glyph
        const Glyph& glyph = _glyphs[character];

        // if active
        if( glyph.active )
            return &glyph;

        return nullptr;
    }

    const Texture& FontAtlas::GetTexture() const
    {
        return _texture;
    }
}

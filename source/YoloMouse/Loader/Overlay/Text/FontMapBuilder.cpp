#include <YoloMouse/Loader/Overlay/Text/FontMapBuilder.hpp>

namespace Yolomouse
{
    // public
    //---------------------------------------------------------------------
    FontMapBuilder::FontMapBuilder():
        _map_size       (0),
        _token          (NULL),
        _graphics       (nullptr),
        _font           (nullptr),
        _bitmap         (nullptr),
        _string_format  (nullptr)
    {
    }

    /**/
    FontMapBuilder::~FontMapBuilder()
    {
        ASSERT( !IsInitialized() );
    }

    /**/
    Bool FontMapBuilder::Initialize( const WChar* font_name, Float font_size, ULong map_size )
    {
        ASSERT( !IsInitialized() );

        Gdiplus::GdiplusStartupInput     startup_input(NULL, TRUE, TRUE);
	    Gdiplus::GdiplusStartupOutput    startup_output;

        // set fields
        _map_size = map_size;

	    // initialize gdi+
        if( Gdiplus::GdiplusStartup( &_token, &startup_input, &startup_output ) == Gdiplus::Status::Ok )
        {
            Gdiplus::CharacterRange range( 0, 1 );

            // create font
            _font = new Gdiplus::Font( font_name, font_size, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );

            // create bitmap
            _bitmap = new Gdiplus::Bitmap( map_size, map_size, PixelFormat32bppARGB );

            // create graphics
            _graphics = new Gdiplus::Graphics( _bitmap );
            _graphics->SetTextRenderingHint( Gdiplus::TextRenderingHintAntiAlias );
            _graphics->Clear( Gdiplus::Color( 255, 0, 0, 0 ) );
            _graphics->SetPageUnit( Gdiplus::UnitPixel );

            // create string format
            _string_format = new Gdiplus::StringFormat(Gdiplus::StringFormat::GenericTypographic());
            _string_format->SetFormatFlags( Gdiplus::StringFormatFlagsMeasureTrailingSpaces );
            _string_format->SetMeasurableCharacterRanges( 1, &range );

            // set status ok
            return true;
        }

        return false;
    }

    void FontMapBuilder::Shutdown()
    {
        ASSERT( IsInitialized() );

        // if gdi+ initialized
        if( _token != NULL )
        {
            // free objects
            delete _string_format;
            delete _graphics;
            delete _bitmap;
            delete _font;

            // shutdown gdi+
            Gdiplus::GdiplusShutdown(_token);
            _token = NULL;
        }
    }

    /**/
    Bool FontMapBuilder::IsInitialized() const
    {
        return _token != NULL;
    }

    /**/
    Bool FontMapBuilder::GetCharacterSize( Vector2f& size, WChar character )
    {
        Gdiplus::RectF  layout( 0, 0, 1000, 1000 );
        Gdiplus::RectF  region_rect;
        Gdiplus::Region region;
        WCHAR           string[1] = { character };

        // get region from single character
        if( _graphics->MeasureCharacterRanges( string, 1, _font, layout, _string_format, 1, &region ) != Gdiplus::Status::Ok )
            return false;

        // extract size from region
        if( region.GetBounds( &region_rect, _graphics ) != Gdiplus::Status::Ok )
            return false;

        // return size
        size.Set( region_rect.Width, region_rect.Height );

        return true;
    }

    /**/
    Bool FontMapBuilder::WriteCharacter( const Vector2f& offset, WChar character )
    {
        WCHAR               string[1] = { character };
	    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 255, 255));

        // write character to bitmap
        return _graphics->DrawString(string, 1, _font, Gdiplus::PointF(offset.x, offset.y), _string_format, &brush) == Gdiplus::Status::Ok;
    }

    /**/
    Bool FontMapBuilder::ToDxMap( Texture& texture, RenderContext& render_context )
    {
	    Gdiplus::BitmapData bitmap_data;
        Bool                status = false;

        // get bitmap memory access
        if( _bitmap->LockBits( &Gdiplus::Rect( 0, 0, _map_size, _map_size ), Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmap_data ) == Gdiplus::Status::Ok )
        {
            D3D11_TEXTURE2D_DESC    texture_desc = {};
            D3D11_SUBRESOURCE_DATA  subresource_data = {};
            Byte*                   pixels_begin = reinterpret_cast<Byte*>( bitmap_data.Scan0 );
            Byte*                   pixels_end = pixels_begin + (_map_size * _map_size * 4);

            // tweak pixel alphas cuz microshit sux. background must be BGRA=000000ff and foreground BGRA=ffffffff
            for( Byte* pixel = pixels_begin; pixel < pixels_end; pixel += 4 )
            {
                // if not fully opaque
                if( pixel[0] != 0xff )
                {
                    // make alpha one of RGB values and rest fully opaque
                    pixel[3] = pixel[0];
                    pixel[0] = pixel[1] = pixel[2] = 0xff;
                }
            }

            // initialize texture
            status = texture.Initialize( { render_context, pixels_begin, Vector2l(_map_size) } );
        }

        // unget bitmap memory access
	    _bitmap->UnlockBits(&bitmap_data);  

	    return status;
    }
}

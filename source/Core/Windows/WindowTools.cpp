#include <Core/Windows/WindowTools.hpp>
#include <stdio.h>

namespace Core
{
    // public
    //--------------------------------------------------------------------------
    void WindowTools::RunWindowLoop()
    {
        MSG msg;

        // run loop
        while(GetMessage(&msg, NULL, 0, 0 ) > 0)
        { 
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }

    //-------------------------------------------------------------------------
    HWND WindowTools::GetFocusWindow()
    {
        POINT point;

        // get cursor position
        if( !GetCursorPos( &point ) )
            return NULL;

        // get window at cursor position
        return WindowFromPoint(point);
    }

    //-------------------------------------------------------------------------
    Bool WindowTools::TestGameWindow( HWND hwnd )
    {
        // constants
        static const ULong TEST_FRAMES_UPDATING_COUNT = 3;

        /*
            ghetto attempt at determining if a window belongs to a game :)
            TODO: find better way!
        */
        RECT client_region = { 0 };

        // get required window client region
        if( !GetClientRect( hwnd, &client_region ) )
            return false;

        // cannot be desktop window
        if( GetDesktopWindow() == hwnd )
            return false;

        // require a valid minimum game window size
        if( !IsValidGameWindowSize( RectToSize( client_region ) ) )
            return false;

        // require frames are updating. this will false positive with various apps
        // like running video players and screen savers but oh well /shrug
        if( !TestFramesUpdating( client_region, hwnd, TEST_FRAMES_UPDATING_COUNT ) )
            return false;

        return true;
    }

    //-------------------------------------------------------------------------
    Bool WindowTools::IsValidGameWindowSize( const SIZE& size )
    {
        // require min width/height
        return size.cx >= GAME_WINDOW_MIN_WIDTH && size.cy >= GAME_WINDOW_MIN_HEIGHT;
    }

    //-------------------------------------------------------------------------
    Bool WindowTools::TestFramesUpdating( const RECT& region, HWND hwnd, ULong test_count )
    {
        ULong success_count = 0;

        // use desktop window with screen coords (to avoid some anticheats such as with WoW)
        RECT  desktop_region = region;
        HWND  desktop_hwnd = GetDesktopWindow();

        // map region to desktop, if 0 result, then fullscreen app
        if( MapWindowPoints( hwnd, desktop_hwnd, (LPPOINT)&desktop_region, 2 ) == 0 )
        {
            // success if fullscreen app is foreground window suggesting dedicated fullscreen game
            //TODO: find better way to determine dedicated fullscreen app
            return GetForegroundWindow() == hwnd;
        }
        HDC desktop_hdc = GetDC(desktop_hwnd);

        // test frames updating by desktop hdc
        for( Index i = 0; i < test_count; ++i )
            success_count += _TestFramesUpdating( desktop_region, desktop_hdc );

        // release hdc
        ReleaseDC( desktop_hwnd, desktop_hdc );

        // succes if all test successful
        return success_count == test_count;
    }

    //-------------------------------------------------------------------------
    SIZE WindowTools::RectToSize( const RECT& rect )
    {
        SIZE size;

        size.cx = rect.right - rect.left;
        size.cy = rect.bottom - rect.top;

        return size;
    };

    //-------------------------------------------------------------------------
    Bool WindowTools::ReadHBitmapPixels( Byte4*& pixels, Vector2l& size, HBITMAP hbitmap )
    {
        BITMAP           bitmap_info;
        BITMAPINFOHEADER bmi = {};
        Bool             status = false;

        // get bitmap info
        if( GetObject( hbitmap, sizeof( BITMAP ), &bitmap_info ) == 0 )
            return false;

        // create a device context same as the screen
        HDC hdc = CreateCompatibleDC(NULL);
        if( hdc == NULL )
            return false;

        // select the hbitmap into that device context
        HBITMAP old_bitmap = (HBITMAP)SelectObject(hdc, hbitmap);
        if( old_bitmap != NULL )
        {
            // determine size and count
            size.Set(bitmap_info.bmWidth, abs(bitmap_info.bmHeight) );
            ULong pixel_count = size.x * size.y;

            // set bitmap info for extracting pixels from hbitmap
            bmi.biSize = sizeof(BITMAPINFOHEADER);
            bmi.biPlanes = 1;
            bmi.biBitCount = 32;
            bmi.biWidth = size.x;
            bmi.biHeight = -size.y;
            bmi.biCompression = BI_RGB;
            bmi.biSizeImage = 0;

            // allocate pixel data
            pixels = new Byte4[pixel_count];

            // get pixel data from bitmap
            if( GetDIBits(hdc, hbitmap, 0, size.y, pixels, (BITMAPINFO*)&bmi, DIB_RGB_COLORS) != 0 )
            {
                // swap pixels from BGRA to RGBA
                for (Index i = 0; i < pixel_count; ++i)
                {
                    Byte* rgba = reinterpret_cast<Byte*>(pixels + i);
                    Tools::Swap(rgba[0], rgba[2]);
                }

                // success
                status = true;
            }
            else
                delete[] pixels;
        }

        // cleanup resources
        SelectObject(hdc, old_bitmap);
        DeleteDC(hdc);

        return status;
    }

    //-------------------------------------------------------------------------
    void WindowTools::MessagePopup( const Char* title, Bool error, const Char* format, ... )
    {
        va_list args;
        Char    message[STRING_MAX_SIZE];

        // only once
        ASSERT(format != NULL);
        va_start(args, format);
        int count = vsprintf_s(message, sizeof(message), format, args);
        va_end(args);

        // show message
        MessageBoxA(NULL, message, title, MB_OK|(error ? MB_ICONERROR : MB_ICONINFORMATION));
    }

    //private
    //-------------------------------------------------------------------------
    Bool WindowTools::_TestFramesUpdating( const RECT& region, HDC hdc )
    {
        // constants
        static const ULong TEST_FRAMES_UPDATING_DELAY = 20; //ms
        static const ULong TEST_FRAMES_UPDATING_ITERATIONS = 10;
        static const Long  TEST_PIXEL_OFFSET = 5;

        // types
        struct TestPixel
        {
            POINT       position;
            COLORREF    color;
        };

        // choose 4 pixels in region corners
        TestPixel test_pixels[] = {
            { { region.left + TEST_PIXEL_OFFSET, region.bottom - TEST_PIXEL_OFFSET } },
            { { region.left + TEST_PIXEL_OFFSET, region.top + TEST_PIXEL_OFFSET } },
            { { region.right - TEST_PIXEL_OFFSET, region.bottom - TEST_PIXEL_OFFSET } },
            { { region.right - TEST_PIXEL_OFFSET, region.top + TEST_PIXEL_OFFSET } },
        };
        ULong valid_count = 0;

        // initialize pixels
        for( Index i = 0; i < COUNT( test_pixels ); ++i )
        {
            // get test pixel
            TestPixel& test_pixel = test_pixels[i];

            // get/save current pixel color in test pixel entry
            test_pixel.color = GetPixel(hdc, test_pixel.position.x, test_pixel.position.y);

            // skip if invalid
            if( test_pixel.color == CLR_INVALID )
                continue;

            // change it a little by flipping lower color component bits
            test_pixel.color ^= 0x00010101;

            // apply new color
            SetPixel(hdc, test_pixel.position.x, test_pixel.position.y, test_pixel.color);

            // increment valid count
            ++valid_count;
        }

        // fail if no valid pixels
        if( valid_count == 0 )
            return false;

        // test pixels for changes
        for( Index i = 0; i < TEST_FRAMES_UPDATING_ITERATIONS; ++i )
        {
            // track total pixels changed
            ULong change_count = 0;

            // let frames update
            Sleep( TEST_FRAMES_UPDATING_DELAY );

            // for each test pixel
            for( Index j = 0; j < COUNT( test_pixels ); ++j )
            {
                // get test pixel
                const TestPixel& test_pixel = test_pixels[j];

                // if valid
                if( test_pixel.color != CLR_INVALID )
                {
                    // get current pixel color
                    COLORREF current_color = GetPixel( hdc, test_pixel.position.x, test_pixel.position.y );

                    // if changed, increment change count
                    if( current_color != test_pixel.color )
                        ++change_count;
                }
            }

            // success if all valid pixels changed
            if( change_count == valid_count )
                return true;
            // else fail if some, but none, changed
            else if( change_count != 0 )
                return false;
        }

        return false;
    }
}

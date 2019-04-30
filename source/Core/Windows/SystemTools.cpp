#include <Core/Windows/SystemTools.hpp>
#include <Core/Support/Tools.hpp>
#include <psapi.h>
#include <stdio.h>

namespace Core
{
    // local
    namespace
    {
        // from https://support.microsoft.com/en-us/kb/131065
        //---------------------------------------------------------------------
        BOOL _SetPrivileges( HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege )
        {
            TOKEN_PRIVILEGES tp = {0};
            LUID             luid;
            TOKEN_PRIVILEGES tpPrevious;
            DWORD            cbPrevious = sizeof(TOKEN_PRIVILEGES);

            if(!LookupPrivilegeValue( NULL, Privilege, &luid )) 
                return FALSE;

            // first pass. get current privilege setting
            tp.PrivilegeCount           = 1;
            tp.Privileges[0].Luid       = luid;
            tp.Privileges[0].Attributes = 0;

            AdjustTokenPrivileges(
                hToken,
                FALSE,
                &tp,
                sizeof(TOKEN_PRIVILEGES),
                &tpPrevious,
                &cbPrevious);
            if (GetLastError() != ERROR_SUCCESS)
                return FALSE;

            // second pass. set privilege based on previous setting
            tpPrevious.PrivilegeCount       = 1;
            tpPrevious.Privileges[0].Luid   = luid;

            if(bEnablePrivilege)
                tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
            else
                tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);

            AdjustTokenPrivileges(
                hToken,
                FALSE,
                &tpPrevious,
                cbPrevious,
                NULL,
                NULL);
            if( GetLastError() != ERROR_SUCCESS )
                return false;

            return true;
        }
    }


    // public
    //-------------------------------------------------------------------------
    Bitness SystemTools::GetProcessBitness( HANDLE process )
    {
        SYSTEM_INFO system_info;

        // check if everything is 32bit
        GetNativeSystemInfo(&system_info);

        // select cpu architecture
        switch( system_info.wProcessorArchitecture )
        {
        // 64bit
        case PROCESSOR_ARCHITECTURE_AMD64:
        case PROCESSOR_ARCHITECTURE_IA64:
            {
                BOOL is_wow64;

                // get wow64 (emulated 32) status
                if( process==NULL || !IsWow64Process(process, &is_wow64) )
                    return BITNESS_UNKNOWN;

                // if emulated then 32 else 64
                return is_wow64 ? BITNESS_32 : BITNESS_64;
            }

        // 32bit
        case PROCESSOR_ARCHITECTURE_INTEL:
            return BITNESS_32;
        }

        return BITNESS_UNKNOWN;
    }

    //-------------------------------------------------------------------------
    HWND SystemTools::GetFocusWindow()
    {
        POINT point;

        // get cursor position
        if( !GetCursorPos( &point ) )
            return NULL;

        // get window at cursor position
        return WindowFromPoint(point);
    }

    //-------------------------------------------------------------------------
    Bool SystemTools::EnableAutoStart( const WCHAR* name, const WCHAR* path, Bool enable )
    {
        const WCHAR* REGISTRY_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        Bool status = false;
        HKEY hkey;

        // if enabling
        if(enable)
        {
            // open/create key
            if(RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkey, NULL) == ERROR_SUCCESS)
            {
                // set value
                if(RegSetValueEx (hkey, name, 0, REG_SZ, (Byte*)path, (DWORD)(wcslen(path) * sizeof(WCHAR))) == ERROR_SUCCESS)
                    status = true;

                // close key
                RegCloseKey(hkey);
            }
        }
        // open key
        else if( RegOpenKey(HKEY_CURRENT_USER, REGISTRY_PATH, &hkey) == ERROR_SUCCESS )
        {
            // delete value
            if( RegDeleteValue(hkey, name) == ERROR_SUCCESS )
                status = true;

            // close key
            RegCloseKey(hkey);
        }

        return status;
    }

    //-------------------------------------------------------------------------
    Bool SystemTools::GetProcessDirectory( WCHAR* path, ULong limit )
    {
        if( GetModuleFileName( NULL, path, limit ) == 0 )
            return false;

        Tools::StripFileName(path);
        return true;
    }

    // from https://support.microsoft.com/en-us/kb/131065
    //-------------------------------------------------------------------------
    HANDLE SystemTools::OpenDebugPrivileges()
    {
        HANDLE token;

        // open thread access token
        if( !OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &token) )
        {
            if( GetLastError() == ERROR_NO_TOKEN )
            {
                if( !ImpersonateSelf(SecurityImpersonation) )
                    return NULL;

                if( !OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &token) )
                    return NULL;
            }
            else
                return NULL;
        }

        // enable SeDebugPrivilege
        if(!_SetPrivileges(token, SE_DEBUG_NAME, TRUE))
        {
            // close token handle
            CloseHandle(token);

            // indicate failure
            return NULL;
        }

        return token;
    }

    void SystemTools::CloseDebugPrivileges( HANDLE handle )
    {
        // disable SeDebugPrivilege
        _SetPrivileges(handle, SE_DEBUG_NAME, FALSE);

        // close token handle
        CloseHandle(handle);
    }

    //-------------------------------------------------------------------------
    Bool SystemTools::TestGameWindow( HWND hwnd )
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
    Bool SystemTools::IsValidGameWindowSize( const SIZE& size )
    {
        // require min width/height
        return size.cx >= GAME_WINDOW_MIN_WIDTH && size.cy >= GAME_WINDOW_MIN_HEIGHT;
    }

    //-------------------------------------------------------------------------
    Bool SystemTools::TestFramesUpdating( const RECT& region, HWND hwnd, ULong test_count )
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
    SIZE SystemTools::RectToSize( const RECT& rect )
    {
        SIZE size;

        size.cx = rect.right - rect.left;
        size.cy = rect.bottom - rect.top;

        return size;
    };

    //private
    //-------------------------------------------------------------------------
    Bool SystemTools::_TestFramesUpdating( const RECT& region, HDC hdc )
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

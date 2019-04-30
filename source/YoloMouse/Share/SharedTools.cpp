#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <YoloMouse/Share/SharedState.hpp>
#include <Psapi.h>
#include <Shlobj.h>
#include <stdio.h>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    Bool SharedTools::BuildTargetId( WCHAR* target_id, ULong limit, HANDLE process )
    {
        static const ULong SLASH_LIMIT = 4;
        Bool        status = false;
        PathString  path = {0};

        // get executable path and build target id string
        if( GetProcessImageFileName(process, path, COUNT(path)) == 0 )
        {
            elog("SharedTools.BuildTargetId.GetProcessImageFileName");
            return false;
        }

        ULong   length = (ULong)wcslen(path);
        ULong   slashes = 0;
        WCHAR*  end = length + path;
        WCHAR   c;

        // state to supress path numbers. this helps to maintain a common
        // id for targets with versioned directories.
        Bool supress_numbers = false;

        // for each character from the end of the path
        do
        {
            // get next character
            c = *--end;

            // if not alphanumeric
            if( (c < 'a' || c > 'z') &&
                (c < 'A' || c > 'Z') &&
                (supress_numbers || c < '0' || c > '9') )
            {
                // replace with _
                *end = '_';
            }

            // if slash
            if( (c == '\\' || c == '/') )
            {
                supress_numbers = true;
                slashes++;
            }
        }
        while( end > path && slashes < SLASH_LIMIT );

        // copy starting at end to target id
        wcscpy_s(target_id, limit, end + 1);

        return true;
    }

    //-------------------------------------------------------------------------
    Bool SharedTools::BuildUserPath( WCHAR* path, ULong limit, const WCHAR* name, const WCHAR* extension, HANDLE process )
    {
        PathString  save_path;
        PathString  wpath;
        HANDLE      token = NULL;
        Bool        status = false;

        // if process specified open process token
        if( process && !OpenProcessToken(process, TOKEN_QUERY|TOKEN_IMPERSONATE|TOKEN_DUPLICATE, &token) )
        {
            elog("SharedTools.BuildUserPath.OpenProcessToken");
            return false;
        }

        // get appdata folder
        if( SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, token, SHGFP_TYPE_CURRENT, wpath) == S_OK )
        {
            // build save path
            eggs(swprintf_s(save_path, COUNT(save_path), L"%s\\%s", wpath, APP_NAME) > 0);

            // ensure save path exists
            CreateDirectory(save_path, NULL);

            // build save path including file
            if( name != NULL && extension != NULL )
            {
                eggs( swprintf_s( path, limit, L"%s\\%s.%s",
                    save_path,
                    name,
                    extension ) > 0 );
            }
            // else use just path
            else
                wcsncpy_s( path, limit, save_path, COUNT(save_path) );

            status = true;
        }
        else
            elog("SharedTools.BuildUserPath.SHGetFolderPath: %s", Tools::WToCString(name));

        // close optional process token
        if( token )
            CloseHandle(token);

        return true;
    }

    //-------------------------------------------------------------------------
    void SharedTools::MessagePopup( Bool error, const Char* format, ... )
    {
        va_list args;
        Char    message[STRING_MAX_SIZE];

        // only once
        xassert(format != NULL);
        va_start(args, format);
        int count = vsprintf_s(message, sizeof(message), format, args);
        va_end(args);

        // show message
        MessageBoxA(NULL, message, APP_NAMEC, MB_OK|(error ? MB_ICONERROR : MB_ICONINFORMATION));
    }

    //-------------------------------------------------------------------------
    Index SharedTools::CursorSizeToSizeIndex( ULong size )
    {
        // if original size
        if( size == CURSOR_SIZE_INDEX_ORIGINAL )
            return 0;

        // if old table (size is index)
        if( size < CURSOR_SIZE_TABLE[1] )
        {
            // get size from old table
            if( size < COUNT( CURSOR_SIZE_TABLE_V_0_8_3 ) )
                size = CURSOR_SIZE_TABLE_V_0_8_3[size];
            // else return default index
            else
                return CURSOR_SIZE_INDEX_DEFAULT;
        }

        // locate size index nearest requested size
        for( Index i = 0; i < CURSOR_SIZE_INDEX_COUNT; ++i )
            if( size <= CURSOR_SIZE_TABLE[i] )
                return i;

        // use default
        return CURSOR_SIZE_INDEX_DEFAULT;
    }

    ULong SharedTools::CursorToSize( HCURSOR hcursor )
    {
        ICONINFO icon_info;
        BITMAP   bitmap = { 0 };
 
        // get base icon info
        if( GetIconInfo(hcursor, &icon_info) == FALSE )
            return 0;

        // select bitmap handle to get bitmap from
        HBITMAP hbitmap = icon_info.hbmColor ? icon_info.hbmColor : icon_info.hbmMask;

        // if either bitmap handle exists get base icon bitmap object
        if( hbitmap != NULL )
            GetObject( hbitmap, sizeof( BITMAP ), &bitmap );

        // cleanup temporary resources
        if( icon_info.hbmColor != NULL )
            DeleteObject(icon_info.hbmColor);
        if( icon_info.hbmMask != NULL )
            DeleteObject(icon_info.hbmMask);

        // return size as bitmap height (will be 0 if failed)
        return bitmap.bmHeight;
    }
}

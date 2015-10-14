#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <YoloMouse/Share/SharedState.hpp>
#include <Psapi.h>
#include <Shlobj.h>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    Hash SharedTools::CalculateCursorHash( HCURSOR hcursor )
    {
        static const ULong HASH_LIMIT = KILOBYTES(8);
    
        ICONINFO    iconinfo;
        LONG        count;
        Byte        buffer[HASH_LIMIT];

        // require valid
        if( hcursor == NULL )
            return 0;

        // get icon info
        if( GetIconInfo(hcursor, &iconinfo) == FALSE )
            return 0;

        // get icon bitmap buffer
        count = GetBitmapBits( iconinfo.hbmColor ? iconinfo.hbmColor : iconinfo.hbmMask, sizeof(buffer), buffer );

        // iconinfo cleanup 
        if( iconinfo.hbmColor )
            DeleteObject(iconinfo.hbmColor);
        if( iconinfo.hbmMask )
            DeleteObject(iconinfo.hbmMask);

        // fail if no bits read
        if(count == 0)
            return 0;

        // generate hash
        return Tools::Fnv164Hash(buffer, count);
    }

    //-------------------------------------------------------------------------
    Bool SharedTools::BuildTargetId( WCHAR* target_id, ULong limit, HANDLE process )
    {
        static const ULong SLASH_LIMIT = 4;
        Bool status = false;
        WCHAR path[STRING_PATH_SIZE] = {0};

        // get executable path and build target id string
        if( GetProcessImageFileName(process, path, COUNT(path)) == 0 )
            return false;

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
    Bool SharedTools::BuildSavePath( WCHAR* path, ULong limit, const WCHAR* name )
    {
        WCHAR   save_path[STRING_PATH_SIZE];
        WCHAR   wpath[MAX_PATH];

        // get appdata folder
        if( SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, wpath) != S_OK )
            return false;

        // build save path
        eggs(swprintf_s(save_path, COUNT(save_path), L"%s\\%s", wpath, APP_NAME) > 0);

        // ensure save path exists
        CreateDirectory(save_path, NULL);

        // build save path
        eggs(swprintf_s(path, limit, L"%s\\%s.ini",
            save_path,
            name) > 0);

        return true;
    }

    //-------------------------------------------------------------------------
    void SharedTools::ErrorMessage( const Char* message )
    {
        MessageBoxA(NULL, message, APP_NAMEC, MB_OK|MB_ICONERROR);
    }
}

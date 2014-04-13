#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <YoloMouse/Share/SharedState.hpp>
#include <Psapi.h>

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
    Bool SharedTools::BuildTargetId( Char* target_id, ULong limit, HWND hwnd )
    {
        static const ULong SLASH_LIMIT = 4;
        Bool    status = false;
        DWORD   process_id;

        // get process id
        GetWindowThreadProcessId(hwnd, &process_id);

        // get process
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, process_id);
        if(process)
        {
            Char path[STRING_PATH_SIZE] = {0};

            // get executable path and build target id string
            if(GetProcessImageFileName(process, path, sizeof(path)) > 0)
            {
                ULong   length = strlen(path);
                ULong   slashes = 0;
                Char*   end = length + path;
                Char    c;

                // for each character from the end of the path
                do
                {
                    // get next character
                    c = *--end;

                    // if not alphanumeric
                    if( (c < 'a' || c > 'z') &&
                        (c < 'A' || c > 'Z') &&
                        (c < '0' || c > '9') )
                    {
                        // replace with _
                        *end = '_';
                    }

                    // if slash
                    if( (c == '\\' || c == '/') )
                        slashes++;
                }
                while( end > path && slashes < SLASH_LIMIT );

                // copy starting at end to target id
                strcpy_s(target_id, limit, end + 1);

                status = true;
            }

            // close process
            CloseHandle(process);
        }

        return status;
    }

    //-------------------------------------------------------------------------
    void SharedTools::BuildTargetSavePath( Char* path, ULong limit, const Char* target_id )
    {
        // build save path
        eggs(sprintf_s(path, limit, "%s\\%s\\%s.ini",
            SharedState::Instance().GetPath(),
            PATH_SAVE,
            target_id) > 0);
    }
}

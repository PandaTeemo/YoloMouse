#include <Core/Constants.hpp>
#include <Core/Support/Tools.hpp>
#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Share/SharedState.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <Psapi.h>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    SharedState::SharedState():
        _memory(IPC_MEMORY_NAME)
    {
    }

    //-------------------------------------------------------------------------
    Bool SharedState::Open( Bool host )
    {
        // open shared memory
        if(!_memory.Open(host))
            return false;

        // if host
        if(host)
        {
            // reset state
            _memory->path[0] = 0;
            _memory->cursors.Zero();
            _memory->size = CURSOR_SIZE_MEDIUM;

            // load cursors
            _LoadCursors();
        }

        // update state
        _host = host;

        return true;
    }

    void SharedState::Close()
    {
        // if host
        if(_host)
        {
            // unload cursors
            _UnloadCursors();
        }

        // close shared memory
        _memory.Close();
    }

    //-------------------------------------------------------------------------
    const Char* SharedState::GetPath() const
    {
        return _memory->path;
    }

    HCURSOR SharedState::GetCursor( Index cursor_index )
    {
        return _memory->cursors[_memory->size][cursor_index];
    }

    CursorSize SharedState::GetCursorSize() const
    {
        return _memory->size;
    }

    //-------------------------------------------------------------------------
    void SharedState::SetPath( const Char* path )
    {
        strcpy_s(_memory->path, sizeof(_memory->path), path);
    }

    void SharedState::SetCursorSize( CursorSize size )
    {
        _memory->size = size;
    }

    //-------------------------------------------------------------------------
    Index SharedState::FindCursor( HCURSOR hcursor )
    {
        CursorArray& cursors = _memory->cursors[_memory->size];

        // for each cursor
        for( Index i = 0; i < cursors.GetCount(); ++i )
            if(hcursor == cursors[i])
                return i;

        return INVALID_INDEX;
    }

    // private
    //-------------------------------------------------------------------------
    void SharedState::_LoadCursors()
    {
        static const Char* EXTENSIONS[] = { "ani", "cur" };

        // for each size
        for( Index size_index = 0; size_index < CURSOR_SIZE_COUNT; size_index++ )
        {
            CursorArray& cursors = _memory->cursors[size_index];

            // for each potential cursor index
            for( Index cursor_index = 0; cursor_index < cursors.GetCount(); ++cursor_index )
            {
                Char path[STRING_PATH_SIZE];

                // for each extension
                for( Index extension_index = 0; extension_index < COUNT(EXTENSIONS); ++extension_index )
                {
                    UINT loadimage_flags = LR_LOADFROMFILE|LR_SHARED;
    
                    // make cursor path
                    sprintf_s(path, sizeof(path), "%s/%s/%u.%s",
                        PATH_CURSORS,
                        PATH_CURSORS_SIZE[size_index],
                        cursor_index + 1,
                        EXTENSIONS[extension_index]);

                    // windows versions older than vista require size confirmed to 32x32
                    if( SystemTools::GetOsVersion() < OSVERSION_WINVISTA )
                        loadimage_flags |= LR_DEFAULTSIZE;

                    // load shared cursor
                    cursors[cursor_index] = (HCURSOR)LoadImage(NULL, path, IMAGE_CURSOR, 0, 0, loadimage_flags);

                    // break if successful
                    if( cursors[cursor_index] )
                        break;
                }
            }
        }
    }

    void SharedState::_UnloadCursors()
    {
        // do nothing. docs say dont destroy shared cursors
    }
}

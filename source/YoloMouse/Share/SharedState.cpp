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
        _host   (false),
        _shared (IPC_MEMORY_NAME)
    {
    }

    //-------------------------------------------------------------------------
    Bool SharedState::Open( Bool host )
    {
        // open shared memory
        if(!_shared.Open(host))
            return false;

        // if host
        if(host)
        {
            // reset state
            _shared->cursors.Zero();
            _shared->size = CURSOR_SIZE_MEDIUM;

            // load cursors
            _LoadCursors();
        }
        // else local
        else
        {
            // reset state
            _client.cursors.Zero();

            // cache cursors
            _CacheCursors();
        }

        // update state
        _host = host;

        return true;
    }

    void SharedState::Close()
    {
        // free cursors
        _FreeCursors();

        // close shared memory
        _shared.Close();
    }

    //-------------------------------------------------------------------------
    HCURSOR SharedState::GetCursor( Index cursor_index )
    {
        Long         base = _shared->size;
        CursorTable& cursors = _host ? _shared->cursors : _client.cursors;

        // find cursor nearest cursor index
        for( Long offset = 0; offset < CURSOR_SIZE_COUNT; offset++ )
        {
            Byte8 smaller = cursors[Tools::Max<Long>(base - offset, 0)][cursor_index];
            Byte8 larger = cursors[Tools::Min<Long>(base + offset, CURSOR_SIZE_COUNT - 1)][cursor_index];

            // choose larger before smaller
            if( larger )
                return reinterpret_cast<HCURSOR>(larger);
            if( smaller )
                return reinterpret_cast<HCURSOR>(smaller);
        }

        return NULL;
    }

    CursorSize SharedState::GetCursorSize() const
    {
        return _shared->size;
    }

    //-------------------------------------------------------------------------
    void SharedState::SetCursorSize( CursorSize size )
    {
        _shared->size = size;
    }

    //-------------------------------------------------------------------------
    Index SharedState::FindCursor( HCURSOR hcursor )
    {
        CursorArray& cursors = _shared->cursors[_shared->size];

        // for each cursor
        for( Index i = 0; i < cursors.GetCount(); ++i )
            if(hcursor == reinterpret_cast<HCURSOR>(cursors[i]))
                return i;

        return INVALID_INDEX;
    }

    // private
    //-------------------------------------------------------------------------
    HCURSOR SharedState::_LoadCursor( Index cursor_index, const WCHAR* base_path )
    {
        static const WCHAR* EXTENSIONS[] = { L"ani", L"cur" };

        // for each extension
        for( Index extension_index = 0; extension_index < COUNT(EXTENSIONS); ++extension_index )
        {
            UINT    loadimage_flags = LR_LOADFROMFILE|LR_SHARED;
            WCHAR   full_path[STRING_PATH_SIZE];
    
            // make cursor path
            swprintf_s(full_path, COUNT(full_path), L"%s/%u.%s",
                base_path,
                cursor_index + 1,
                EXTENSIONS[extension_index]);

            // windows versions older than vista require size confirmed to 32x32
            if( SystemTools::GetOsVersion() < OSVERSION_WINVISTA )
                loadimage_flags |= LR_DEFAULTSIZE;

            // load shared cursor
            HCURSOR hcursor = reinterpret_cast<HCURSOR>(LoadImage(NULL, full_path, IMAGE_CURSOR, 0, 0, loadimage_flags));

            // break if successful
            if( hcursor )
                return hcursor;
        }

        return NULL;
    }

    void SharedState::_LoadCursors()
    {
        FlatArray<Bool, SHARED_CURSOR_LIMIT> _user_loaded;

        // init
        _user_loaded.Zero();

        // user cursors: for each cursor index
        for( Index cursor_index = 0; cursor_index < SHARED_CURSOR_LIMIT; ++cursor_index )
        {
            // load user cursors into medium slot
            Byte8 cursor =
                _shared->cursors[CURSOR_SIZE_MEDIUM][cursor_index] = 
                reinterpret_cast<Byte8>(_LoadCursor(cursor_index, PATH_CURSORS));

            // mark user loaded (to prevent loading default cursors)
            _user_loaded[cursor_index] = (cursor != 0);
        }

        // default cursors: for each size
        for( Index size_index = 0; size_index < CURSOR_SIZE_COUNT; size_index++ )
        {
            CursorArray& cursors = _shared->cursors[size_index];

            // for each cursor index
            for( Index cursor_index = 0; cursor_index < SHARED_CURSOR_LIMIT; ++cursor_index )
            {
                // if not already user cursor
                if( !_user_loaded[cursor_index] )
                {
                    WCHAR base_path[STRING_PATH_SIZE];

                    // make cursor path
                    swprintf_s(base_path, COUNT(base_path), L"%s/%s", PATH_CURSORS, PATH_CURSORS_SIZE[size_index]);

                    // load cursor
                    cursors[cursor_index] = reinterpret_cast<Byte8>(_LoadCursor(cursor_index, base_path));
                }
            }
        }
    }

    //-------------------------------------------------------------------------
    void SharedState::_CacheCursors()
    {
        // for each size
        for( Index size_index = 0; size_index < CURSOR_SIZE_COUNT; size_index++ )
        {
            // for each cursor index
            for( Index cursor_index = 0; cursor_index < SHARED_CURSOR_LIMIT; ++cursor_index )
            {
                // get shared cursor
                HCURSOR cursor = reinterpret_cast<HCURSOR>(_shared->cursors[size_index][cursor_index]);

                // if exists create cached copy
                if( cursor )
                    _client.cursors[size_index][cursor_index] = reinterpret_cast<Byte8>(CopyImage(cursor, IMAGE_CURSOR, 0, 0, 0));
            }
        }
    }

    //-------------------------------------------------------------------------
    void SharedState::_FreeCursors()
    {
        // if host
        if( _host )
        {
            // do nothing. docs say dont destroy shared cursors
        }
        else
        {
            // for each size
            for( Index size_index = 0; size_index < CURSOR_SIZE_COUNT; size_index++ )
            {
                // for each cursor index
                for( Index cursor_index = 0; cursor_index < SHARED_CURSOR_LIMIT; ++cursor_index )
                {
                    // get cached cursor
                    HCURSOR cursor = reinterpret_cast<HCURSOR>(_client.cursors[size_index][cursor_index]);

                    // if exists destroy
                    if( cursor )
                        DestroyCursor(cursor);
                }
            }
        }
    }
}

#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Dll/CursorVault.hpp>
#include <stdio.h>

namespace YoloMouse
{
    // CursorResource
    //-------------------------------------------------------------------------
    CursorVault::CursorResource::CursorResource():
        handle      (NULL),
        referenced  (0)
    {
    }

    // CacheEntry
    //-------------------------------------------------------------------------
    CursorVault::CacheEntry::CacheEntry():
        state           (RESOURCE_NONE),
        resizable       (true),
        width           (0),
        height          (0),
        loadimage_flags (0)
    {
    }

    // public
    //-------------------------------------------------------------------------
    CursorVault::CursorVault():
        _state(SharedState::Instance())
    {
    }

    CursorVault::~CursorVault()
    {
    }

    //-------------------------------------------------------------------------
    Bool CursorVault::Load( Index resource_index, Index size_index )
    {
        // get entry
        CacheEntry& entry = _table[resource_index];

        // if not ready
        if( entry.state != RESOURCE_READY )
        {
            // fail if already attempted
            if( entry.state == RESOURCE_FAILED )
                return false;

            // initialize cache
            if( !_CacheInit( entry, resource_index ) )
                return false;
        }

        // load into cache
        return _CacheLoad( entry, size_index );
    }

    //-------------------------------------------------------------------------
    void CursorVault::Unload( Index resource_index, Index size_index )
    {
        // get entry
        CacheEntry& entry = _table[resource_index];

        // unload from cache
        _CacheUnload(entry, size_index);
    }

    void CursorVault::UnloadAll()
    {
        // for each cursor
        for( Index i = 0; i < _table.GetCount(); ++i )
        {
            // for each handle by size
            for( Index j = 0; j < CURSOR_INDEX_COUNT; ++j )
            {
                // unload
                Unload(i, j);
            }
        }
    }

    //-------------------------------------------------------------------------
    HCURSOR CursorVault::GetCursor( Index resource_index, Index size_index )
    {
        // get entry
        CacheEntry& entry = _table[resource_index];

        // return cursor handle if any
        return entry.resizable ? entry.resources[size_index].handle : entry.resources[0].handle;
    }

    //-------------------------------------------------------------------------
    Bool CursorVault::HasCursor( HCURSOR hcursor )
    {
        // for each cursor
        for( Index i = 0; i < _table.GetCount(); ++i )
        {
            const ResourceTable& resources = _table[i].resources;

            // for each size
            for( Index j = 0; j < CURSOR_INDEX_COUNT; ++j )
            {
                // success if found
                if( hcursor == resources[j].handle )
                    return true;
            }
        }

        return false;
    }

    // private
    //-------------------------------------------------------------------------
    Bool CursorVault::_CacheInit( CacheEntry& entry, Index resource_index )
    {
        static const WCHAR* EXTENSIONS[] = { EXTENSION_ANIMATED, EXTENSION_STATIC };
        static const WCHAR* SEARCH_PATHS[] = { PATH_CURSORS_CUSTOM, PATH_CURSORS_DEFAULT };

        // for each search path
        for( Index searchpath_index = 0; searchpath_index < COUNT(SEARCH_PATHS); ++searchpath_index )
        {
            const WCHAR* search_path = SEARCH_PATHS[searchpath_index];

            // for each extension
            for( Index extension_index = 0; extension_index < COUNT(EXTENSIONS); ++extension_index )
            {
                HCURSOR hcursor = NULL;
                Bool    resizable = (extension_index != 0);

                // load flags
                entry.loadimage_flags = LR_LOADFROMFILE;

                // build cursor path
                swprintf_s(entry.path, COUNT(entry.path), L"%s\\%s\\%u.%s",
                    _state.EditPath(),
                    search_path,
                    resource_index,
                    EXTENSIONS[extension_index]);

                // load cursor
                hcursor = reinterpret_cast<HCURSOR>(LoadImage(NULL, entry.path, IMAGE_CURSOR, 0, 0, entry.loadimage_flags));
                if( hcursor )
                {
                    ICONINFO iconinfo = {0};
                    BITMAP   bmpinfo = {0};

                    // get icon info
                    if( GetIconInfo(hcursor, &iconinfo) == TRUE && GetObject(iconinfo.hbmMask, sizeof(BITMAP), &bmpinfo) != 0 )
                    {
                        // build entry
                        entry.resizable =   resizable;
                        entry.width =       bmpinfo.bmWidth;
                        entry.height =      abs(bmpinfo.bmHeight) / (iconinfo.hbmColor==NULL ? 2 : 1);
                        entry.state =       RESOURCE_READY;

                        // cleanup 
                        if( iconinfo.hbmColor )
                            DeleteObject(iconinfo.hbmColor);
                        if( iconinfo.hbmMask )
                            DeleteObject(iconinfo.hbmMask);
                        DestroyCursor(hcursor);

                        return true;
                    }
                }
            }
        }

        // mark failed to avoid attempting again
        entry.state = RESOURCE_FAILED;

        elog("CursorVault.CacheInit.NotFound: %d", resource_index);
        return false;
    }

    Bool CursorVault::_CacheLoad( CacheEntry& entry, Index size_index )
    {
        CursorResource& resource = entry.resources[entry.resizable ? size_index : 0];

        // if not referenced
        if( resource.referenced == 0 )
        {
            ULong width;
            ULong height;

            // if resizable
            if( entry.resizable && size_index != CURSOR_INDEX_ORIGINAL )
            {
                xassert(entry.width > 0 && entry.height > 0);

                // get size from table
                width = height = CURSOR_SIZE_TABLE[size_index];
            }
            // else use original size
            else
            {
                width = 0;
                height = 0;
            }

            // load cursor
            resource.handle = reinterpret_cast<HCURSOR>(LoadImage(NULL, entry.path, IMAGE_CURSOR, width, height, entry.loadimage_flags));
            if( resource.handle == NULL )
            {
                elog("CursorVault.CacheLoad.LoadImage: %s %d %d %x", Tools::WToCString(entry.path), width, height, entry.loadimage_flags);
                return false;
            }
        }

        // increment referenced
        resource.referenced++;

        return true;
    }

    void CursorVault::_CacheUnload( CacheEntry& entry, Index size_index )
    {
        CursorResource& resource = entry.resources[entry.resizable ? size_index : 0];

        // if referenced
        if( resource.referenced > 0 )
        {
            // decrement referenced and free if 0
            if( --resource.referenced == 0 )
            {
                // must be loaded
                xassert(resource.handle != NULL);

                // free cursor
                DestroyCursor(resource.handle);

                // reset
                resource.handle = NULL;
            }
        }
    }
}

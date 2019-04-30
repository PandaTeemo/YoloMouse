#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Dll/CursorVault.hpp>
#include <stdio.h>

namespace YoloMouse
{
    // CursorCache
    //-------------------------------------------------------------------------
    CursorVault::CursorCache::CursorCache():
        hcursor     (NULL),
        referenced  (0)
    {
    }

    // PresetEntry
    //-------------------------------------------------------------------------
    CursorVault::PresetEntry::PresetEntry():
        state           (RESOURCE_NONE),
        resizable       (true),
        width           (0),
        height          (0),
        loadimage_flags (0)
    {
    }

    // IdentityEntry
    //-------------------------------------------------------------------------
    CursorVault::IdentityEntry::IdentityEntry():
        cursor_hash(0)
    {
    }

    //-------------------------------------------------------------------------
    Bool CursorVault::IdentityEntry::operator==( const Hash& cursor_hash_ ) const
    {
        return cursor_hash == cursor_hash_;
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
    Bool CursorVault::HasCursor( HCURSOR hcursor )
    {
        // for each preset entry
        for( const PresetEntry& entry: _preset_table )
        {
            // for each cache entry
            for( const CursorCache& cache: entry.cache_table )
            {
                // success if found
                if( hcursor == cache.hcursor )
                    return true;
            }
        }

        // for each identity entry
        for( const IdentityEntry& entry: _identity_map )
        {
            // for each cache entry
            for( const CursorCache& cache: entry.cache_table )
            {
                // success if found
                if( hcursor == cache.hcursor )
                    return true;
            }
        }

        return false;
    }

    //-------------------------------------------------------------------------
    HCURSOR CursorVault::GetIdentity( Hash cursor_hash, Index size_index )
    {
        // find existing entry
        IdentityEntry* entry = _identity_map.Find<Hash>(cursor_hash);

        // return cursor handle if exists else null
        return entry != nullptr ? entry->cache_table[size_index].hcursor : nullptr;
    }

    HCURSOR CursorVault::GetPreset( Index preset_index, Index size_index )
    {
        // get entry
        PresetEntry& entry = _preset_table[preset_index];

        // return cursor handle if any
        return entry.resizable ? entry.cache_table[size_index].hcursor : entry.cache_table[0].hcursor;
    }

    //-------------------------------------------------------------------------
    Bool CursorVault::LoadIdentity( Hash cursor_hash, Index size_index, HCURSOR hcursor )
    {
        xassert( cursor_hash != 0 );
        xassert( size_index < CURSOR_SIZE_INDEX_COUNT );
        xassert( hcursor != NULL );

        // do use original size index
        if( size_index == CURSOR_SIZE_INDEX_ORIGINAL )
            return false;

        // find existing entry
        IdentityEntry* entry = _identity_map.Find<Hash>(cursor_hash);

        // if does not exist
        if( entry == nullptr )
        {
            // fail if full
            if( _identity_map.IsFull() )
            {
                elog("CursorVault.LoadIdentity.Full");
                return false;
            }

            // add new entry
            entry = _identity_map.Add();

            // add entry
            entry->cursor_hash = cursor_hash;
        }

        // load entry
        return _LoadIdentity( *entry, size_index, hcursor );
    }

    Bool CursorVault::LoadPreset( Index preset_index, Index size_index )
    {
        // get entry
        PresetEntry& entry = _preset_table[preset_index];

        // if not ready
        if( entry.state != RESOURCE_READY )
        {
            // fail if already attempted
            if( entry.state == RESOURCE_FAILED )
                return false;

            // initialize preset
            if( !_InitPreset( entry, preset_index ) )
                return false;
        }

        // load preset
        return _LoadPreset( entry, size_index );
    }

    //-------------------------------------------------------------------------
    void CursorVault::UnloadIdentity( Hash cursor_hash, Index size_index )
    {
        xassert( cursor_hash != 0 );
        xassert( size_index < CURSOR_SIZE_INDEX_COUNT );

        // find existing entry
        IdentityEntry* entry = _identity_map.Find<Hash>(cursor_hash);

        // if found
        if( entry != nullptr )
        {
            // unload identity entry
            _UnloadIdentity( *entry, size_index );
        }
    }

    void CursorVault::UnloadPreset( Index preset_index, Index size_index )
    {
        xassert( preset_index < CURSOR_RESOURCE_PRESET_COUNT );
        xassert( size_index < CURSOR_SIZE_INDEX_COUNT );

        // unload preset
        _UnloadPreset( _preset_table[preset_index], size_index );
    }

    void CursorVault::UnloadAll()
    {
        // for each identity entry
        for( IdentityEntry& entry: _identity_map )
        {
            // for each size index, unload cursor cache
            for( Index size_index = 0; size_index < CURSOR_SIZE_INDEX_COUNT; ++size_index )
                _UnloadIdentity( entry, size_index );
        }

        // for each preset entry
        for( PresetEntry& entry: _preset_table )
        {
            // for each size index, unload cursor cache
            for( Index size_index = 0; size_index < CURSOR_SIZE_INDEX_COUNT; ++size_index )
                _UnloadPreset( entry, size_index );
        }
    }

    // private
    //-------------------------------------------------------------------------
    Bool CursorVault::_LoadIdentity( IdentityEntry& entry, Index size_index, HCURSOR hcursor )
    {
        Bool status = false;

        // get cache by size index
        CursorCache& cache = entry.cache_table[size_index];

        // if not referenced
        if( cache.referenced == 0 )
        {
            ICONINFO ii_base;
            BITMAP   bitmap_base;

            // get base icon info
            if( GetIconInfo(hcursor, &ii_base) == FALSE )
            {
                elog("CursorVault.LoadIdentity.GetIconInfo");
                return false;
            }

            // select bitmap handle to get bitmap from
            HBITMAP hbitmap = ii_base.hbmColor ? ii_base.hbmColor : ii_base.hbmMask;

            // get base icon bitmap object
            if( hbitmap == NULL || !GetObject( hbitmap, sizeof( BITMAP ), &bitmap_base ) )
                elog( "CursorVault.LoadIdentity.GetBitmapDimensionEx" );
            else
            {
                // require width/height over sane minimum
                if( bitmap_base.bmWidth < 2 || bitmap_base.bmHeight < 2 )
                    elog( "CursorVault.LoadIdentity.SizeTooSmall" );
                else
                {
                    ULong width;
                    ULong height;

                    // determine target width/height
                    height = CURSOR_SIZE_TABLE[size_index];

                    // if same, use height for width (typical case, avoids float scaling errors)
                    if( bitmap_base.bmWidth == bitmap_base.bmHeight )
                        width = height;
                    // else scale
                    else
                        width = static_cast<ULong>((static_cast<Float>(bitmap_base.bmWidth) / static_cast<Float>(bitmap_base.bmHeight)) * static_cast<Float>(height));

                    // fail if width or height is 0
                    if( width == 0 || height == 0 )
                        elog( "CursorVault.LoadIdentity.ResizeTooSmall" );
                    else
                    {
                        ICONINFO ii_new = { 0 };

                        // resize hotspot
                        ii_new.xHotspot = static_cast<DWORD>((static_cast<Float>(ii_base.xHotspot) / static_cast<Float>(bitmap_base.bmWidth)) * static_cast<Float>(width));
                        ii_new.yHotspot = static_cast<DWORD>((static_cast<Float>(ii_base.yHotspot) / static_cast<Float>(bitmap_base.bmHeight)) * static_cast<Float>(height));

                        // scale mask+color bitmap
                        if( ii_base.hbmMask != NULL )
                            ii_new.hbmMask = (HBITMAP)CopyImage( ii_base.hbmMask, IMAGE_BITMAP, width, height, 0 );
                        if( ii_base.hbmColor != NULL )
                            ii_new.hbmColor = (HBITMAP)CopyImage( ii_base.hbmColor, IMAGE_BITMAP, width, height, 0 );

                        // creating cursor not icon
                        ii_new.fIcon = FALSE;

                        // create scaled cursor
                        cache.hcursor = ::CreateIconIndirect( &ii_new );

                        // require new cursor created
                        if( cache.hcursor == NULL )
                            elog( "CursorVault.LoadIdentity.CreateIconIndirect" );
                        else
                            status = cache.hcursor != NULL;

                        // cleanup new icon info
                        if( ii_new.hbmMask != NULL )
                            DeleteObject( ii_new.hbmMask );
                        if( ii_new.hbmColor != NULL )
                            DeleteObject( ii_new.hbmColor );
                    }
                }
            }

            // cleanup base icon info
            if( ii_base.hbmColor != NULL )
                DeleteObject( ii_base.hbmColor );
            if( ii_base.hbmMask != NULL )
                DeleteObject( ii_base.hbmMask );
        }

        // increment referenced
        cache.referenced++;

        return status;
    }

    void CursorVault::_UnloadIdentity( IdentityEntry& entry, Index size_index )
    {
        // get cache by size index
        CursorCache& cache = entry.cache_table[size_index];

        // if referenced
        if( cache.referenced > 0 )
        {
            // decrement referenced and unload if 0
            if( --cache.referenced == 0 )
                _UnloadCache( cache );
        }
    }

    //-------------------------------------------------------------------------
    Bool CursorVault::_InitPreset( PresetEntry& entry, Index resource_index )
    {
        static const WCHAR* EXTENSIONS[] = { EXTENSION_ANIMATED_CURSOR, EXTENSION_STATIC_CURSOR };
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

                        // cleanup temporary resources
                        if( iconinfo.hbmColor != NULL )
                            DeleteObject(iconinfo.hbmColor);
                        if( iconinfo.hbmMask != NULL )
                            DeleteObject(iconinfo.hbmMask);
                        DestroyCursor(hcursor);

                        return true;
                    }
                    // cleanup on fail
                    else
                    {
                        DestroyCursor(hcursor);
                    }
                }
            }
        }

        // mark failed to avoid attempting again
        entry.state = RESOURCE_FAILED;

        elog("CursorVault.CacheInit.NotFound: %d", resource_index);
        return false;
    }

    Bool CursorVault::_LoadPreset( PresetEntry& entry, Index size_index )
    {
        CursorCache& cache = entry.cache_table[entry.resizable ? size_index : 0];

        // if not referenced
        if( cache.referenced == 0 )
        {
            ULong width;
            ULong height;

            // if resizable
            if( entry.resizable && size_index != CURSOR_SIZE_INDEX_ORIGINAL )
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
            cache.hcursor = reinterpret_cast<HCURSOR>(LoadImage(NULL, entry.path, IMAGE_CURSOR, width, height, entry.loadimage_flags));
            if( cache.hcursor == NULL )
            {
                elog("CursorVault.CacheLoad.LoadImage: %s %d %d %x", Tools::WToCString(entry.path), width, height, entry.loadimage_flags);
                return false;
            }
        }

        // increment referenced
        cache.referenced++;

        return true;
    }

    void CursorVault::_UnloadPreset( PresetEntry& entry, Index size_index )
    {
        // unload from cache
        CursorCache& cache = entry.cache_table[entry.resizable ? size_index : 0];

        // if referenced
        if( cache.referenced > 0 )
        {
            // decrement referenced and unload if 0
            if( --cache.referenced == 0 )
                _UnloadCache( cache );
        }
    }

    //-------------------------------------------------------------------------
    void CursorVault::_UnloadCache( CursorCache& cache )
    {
        // must be loaded
        xassert(cache.hcursor != NULL);

        // free cursor
        DestroyCursor(cache.hcursor);

        // reset
        cache.hcursor = NULL;
    }
}

#include <Core/System/SystemTools.hpp>
#include <YoloMouse/Share/Cursor/CursorVault.hpp>
#include <YoloMouse/Share/Cursor/CursorTools.hpp>
#include <stdio.h>

namespace Yolomouse
{
    // CursorCache
    //-------------------------------------------------------------------------
    CursorVault::CursorCache::CursorCache():
        hcursor     (NULL),
        referenced  (0)
    {
    }

    CursorVault::CursorCache::~CursorCache()
    {
        ASSERT( hcursor == NULL );
        ASSERT( referenced == 0 );
    }

    // BasicEntry
    //-------------------------------------------------------------------------
    CursorVault::BasicEntry::BasicEntry():
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
        _initialized    (false),
        _hidden_cursor  (NULL)
    {
        _host_path.Zero();
    }

    CursorVault::~CursorVault()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    void CursorVault::Initialize( const PathString& host_path )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _host_path = host_path;

        // initialize hidden cursor
        _InitializeHiddenCursor();

        // set initialized
        _initialized = true;
    }

    void CursorVault::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown hidden cursor
        _ShutdownHiddenCursor();

        // shutdown caches
        _ShutdownCaches();

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool CursorVault::IsInitialized() const
    {
        return _initialized;
    }

    //-------------------------------------------------------------------------
    HCURSOR CursorVault::GetBasic( CursorId id, CursorVariation variation, CursorSize size )
    {
        // get entry
        BasicEntry& entry = _basic_table[id][variation];

        // return cursor handle if exists
        return entry.cache_table[size].hcursor;
    }

    HCURSOR CursorVault::GetClone( Hash cursor_hash, CursorSize size )
    {
        ASSERT( size >= CURSOR_SIZE_MIN && size < CURSOR_SIZE_COUNT );

        // get entry
        CloneEntry* entry = _clone_map.Get( cursor_hash );

        // if found, return cursor handle
        if( entry != nullptr )
            return entry->cache_table[size].hcursor;

        return nullptr;
    }

    HCURSOR CursorVault::GetHidden()
    {
        return _hidden_cursor;
    }

    //-------------------------------------------------------------------------
    Bool CursorVault::HasCursor( HCURSOR hcursor )
    {
        // if hidden cursor
        if( hcursor == _hidden_cursor )
            return true;

        // for each preset entry
        for( const BasicEntry& entry: _basic_table )
        {
            // for each cache entry
            for( const CursorCache& cache: entry.cache_table )
            {
                // success if found
                if( hcursor == cache.hcursor )
                    return true;
            }
        }

        // for each clone entry
        for( const CloneMap::KeyValue& kv: _clone_map )
        {
            // for each cache entry
            for( const CursorCache& cache: kv.value.cache_table )
            {
                // success if found
                if( hcursor == cache.hcursor )
                    return true;
            }
        }

        return false;
    }

    //-------------------------------------------------------------------------
    HCURSOR CursorVault::LoadBasic( CursorId id, CursorVariation variation, CursorSize size )
    {
        // get entry
        BasicEntry& entry = _basic_table[id][variation];

        // if not ready
        if( entry.state != RESOURCE_READY )
        {
            // fail if already attempted
            if( entry.state == RESOURCE_FAILED )
                return NULL;

            // preload basic entry
            if( !_PreLoadBasic( entry, id, variation ) )
                return NULL;
        }

        // load basic cursor for given size
        return _LoadBasic( entry, size );
    }

    HCURSOR CursorVault::LoadClone( HCURSOR hcursor, Hash cursor_hash, CursorSize size )
    {
        ASSERT( hcursor != NULL );
        ASSERT( cursor_hash != 0 );
        ASSERT( size >= CURSOR_SIZE_MIN && size < CURSOR_SIZE_COUNT );

        // access entry
        CloneEntry& entry = _clone_map.Set(cursor_hash);

        // load
        return _LoadClone( entry, hcursor, size );
    }

    //-------------------------------------------------------------------------
    void CursorVault::UnloadBasic( CursorId id, CursorVariation variation, CursorSize size )
    {
        ASSERT( id < CURSOR_ID_COUNT );
        ASSERT( variation < CURSOR_VARIATION_COUNT );
        ASSERT( size >= CURSOR_SIZE_MIN && size < CURSOR_SIZE_COUNT );

        // unload basic
        _UnloadBasic( _basic_table[id][variation], size );
    }

    void CursorVault::UnloadClone( Hash cursor_hash, CursorSize size )
    {
        ASSERT( cursor_hash != 0 );
        ASSERT( size >= CURSOR_SIZE_MIN && size < CURSOR_SIZE_COUNT );

        // get entry
        CloneEntry* entry = _clone_map.Get( cursor_hash );

        // if found, unload clone
        if( entry != nullptr )
            _UnloadClone( *entry, size );
    }

    // private
    //-------------------------------------------------------------------------
    void CursorVault::_InitializeHiddenCursor()
    {
        // constants
        static const ULong CURSOR_SIZE = 16;
        static const ULong BUFFER_SIZE = (CURSOR_SIZE * CURSOR_SIZE) / 8;

        // locals
        BYTE _and_bytes[BUFFER_SIZE];
        BYTE _xor_bytes[BUFFER_SIZE];

        // fill pixel buffers with fully transparent bits/values
        Tools::MemSet( _and_bytes, 0xff, COUNT( _and_bytes ) ); 
        Tools::MemSet( _xor_bytes, 0x00, COUNT( _xor_bytes ) ); 

        // create hidden cursor
        _hidden_cursor = CreateCursor( GetModuleHandle( NULL ), 0, 0, CURSOR_SIZE, CURSOR_SIZE, _and_bytes, _xor_bytes );
    }

    //-------------------------------------------------------------------------
    void CursorVault::_ShutdownHiddenCursor()
    {
        // if initialized
        if( _hidden_cursor != NULL )
        {
            // free cursor
            DestroyCursor(_hidden_cursor);

            // reset
            _hidden_cursor = NULL;
        }
    }

    void CursorVault::_ShutdownCaches()
    {
        // for each basic entry
        for( BasicEntry& entry: _basic_table )
        {
            // unload each cache entry
            for( CursorCache& cache: entry.cache_table )
                if( cache.hcursor != NULL )
                    _ShutdownCache( cache );
        }

        // for each clone entry
        for( const CloneMap::KeyValue& kv: _clone_map )
        {
            // unload each cache entry
            for( CursorCache& cache: kv.value.cache_table )
                if( cache.hcursor != NULL )
                    _ShutdownCache( cache );
        }
    }

    //-------------------------------------------------------------------------
    Bool CursorVault::_PreLoadBasic( BasicEntry& entry, CursorId id, CursorVariation variation )
    {
        static const WCHAR* EXTENSIONS[] = { EXTENSION_ANIMATED_CURSOR, EXTENSION_STATIC_CURSOR };
        static const WCHAR* SEARCH_PATHS[] = { PATH_CURSORS_CUSTOM, PATH_CURSORS_DEFAULT };
        Index index;

        // get index from id/variation
        if( !CursorTools::IdVariationToIndex( index, id, variation ) )
            return false;

        // for each search path
        for( Index searchpath_index = 0; searchpath_index < COUNT(SEARCH_PATHS); ++searchpath_index )
        {
            const WCHAR* search_path = SEARCH_PATHS[searchpath_index];

            // for each extension
            for( Index extension_index = 0; extension_index < COUNT(EXTENSIONS); ++extension_index )
            {
                HCURSOR hcursor = NULL;

                // load flags
                entry.resizable = (extension_index != 0);

                // build cursor path
                swprintf_s(entry.path.EditMemory(), entry.path.GetCount(), L"%s\\%s\\%u.%s", _host_path.GetMemory(), search_path, index, EXTENSIONS[extension_index]);

                // load basic entry
                if( _PreLoadBasicEntry( entry ) )
                    return true;
            }
        }

        // mark failed to avoid attempting again
        entry.state = RESOURCE_FAILED;

        LOG("CursorVault.CacheInit.NotFound: %u", index);
        return false;
    }

    Bool CursorVault::_PreLoadBasicEntry( BasicEntry& entry )
    {
        ICONINFO iconinfo = {0};
        BITMAP   bmpinfo = {0};
        Bool     status = false;

        // load flags
        entry.loadimage_flags = LR_LOADFROMFILE;

        // load cursor
        HCURSOR hcursor = reinterpret_cast<HCURSOR>(LoadImage(NULL, entry.path.GetMemory(), IMAGE_CURSOR, 0, 0, entry.loadimage_flags));
        if( hcursor == NULL )
            return false;

        // get icon info
        if( GetIconInfo(hcursor, &iconinfo) == TRUE && GetObject(iconinfo.hbmMask, sizeof(BITMAP), &bmpinfo) != 0 )
        {
            // update entry
            entry.width =   bmpinfo.bmWidth;
            entry.height =  abs(bmpinfo.bmHeight) / (iconinfo.hbmColor==NULL ? 2 : 1);
            entry.state =   RESOURCE_READY;

            // cleanup temporary resources
            if( iconinfo.hbmColor != NULL )
                DeleteObject(iconinfo.hbmColor);
            if( iconinfo.hbmMask != NULL )
                DeleteObject(iconinfo.hbmMask);

            // set success status
            status = true;
        }

        // cleanup cursor
        DestroyCursor(hcursor);

        return status;
    }

    HCURSOR CursorVault::_LoadBasic( BasicEntry& entry, CursorSize size )
    {
        CursorCache& cache = entry.cache_table[entry.resizable ? size : 0];

        // if not referenced
        if( cache.referenced == 0 )
        {
            ULong width;
            ULong height;

            // if resizable
            if( entry.resizable )
            {
                ASSERT(entry.width > 0 && entry.height > 0);

                // get size from table
                width = height = CursorTools::IdToSize(size);
            }
            // else use original size
            else
            {
                width = 0;
                height = 0;
            }

            // load cursor
            cache.hcursor = reinterpret_cast<HCURSOR>(LoadImage(NULL, entry.path.GetMemory(), IMAGE_CURSOR, width, height, entry.loadimage_flags));
            if( cache.hcursor == NULL )
            {
                LOG("CursorVault.LoadBasicCursor.LoadImage: %s %d %d %x", Tools::WToCString(entry.path.GetMemory()), width, height, entry.loadimage_flags);
                return NULL;
            }
        }

        // increment referenced
        cache.referenced++;

        return cache.hcursor;
    }

    void CursorVault::_UnloadBasic( BasicEntry& entry, CursorSize size )
    {
        // get cache entry by size
        CursorCache& cache = entry.cache_table[entry.resizable ? size : 0];

        // if referenced, decrement and if 0, unload
        if( cache.referenced > 0 && --cache.referenced == 0 )
            _ShutdownCache( cache );
    }

    //-------------------------------------------------------------------------
    HCURSOR CursorVault::_LoadClone( CloneEntry& entry, HCURSOR hcursor, CursorSize size )
    {
        // get cache entry by size
        CursorCache& cache = entry.cache_table[size];

        // if not referenced
        if( cache.referenced == 0 )
        {
            ICONINFO ii_base;
            BITMAP   bitmap_base;

            // get base icon info
            if( GetIconInfo(hcursor, &ii_base) == FALSE )
            {
                LOG("CursorVault.LoadIdentity.GetIconInfo");
                return NULL;
            }

            // select bitmap handle to get bitmap from
            HBITMAP hbitmap = ii_base.hbmColor ? ii_base.hbmColor : ii_base.hbmMask;

            // get base icon bitmap object
            if( hbitmap == NULL || !GetObject( hbitmap, sizeof( BITMAP ), &bitmap_base ) )
                LOG( "CursorVault.LoadIdentity.GetBitmapDimensionEx" );
            else
            {
                // require width/height over sane minimum
                if( bitmap_base.bmWidth < 2 || bitmap_base.bmHeight < 2 )
                    LOG( "CursorVault.LoadIdentity.SizeTooSmall" );
                else
                {
                    ULong width;
                    ULong height;

                    // determine target height
                    height = CursorTools::IdToSize(size);

                    // if same, use height for width (typical case, avoids float scaling errors)
                    if( bitmap_base.bmWidth == bitmap_base.bmHeight )
                        width = height;
                    // else scale
                    else
                        width = static_cast<ULong>((static_cast<Float>(bitmap_base.bmWidth) / static_cast<Float>(bitmap_base.bmHeight)) * static_cast<Float>(height));

                    // fail if width or height is 0
                    if( width == 0 || height == 0 )
                        LOG( "CursorVault.LoadIdentity.ResizeTooSmall" );
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
                            LOG( "CursorVault.LoadIdentity.CreateIconIndirect" );

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

        return cache.hcursor;
    }

    void CursorVault::_UnloadClone( CloneEntry& entry, CursorSize size )
    {
        // get cache entry by size
        CursorCache& cache = entry.cache_table[size];

        // if referenced, decrement and if 0, unload
        if( cache.referenced > 0 && --cache.referenced == 0 )
            _ShutdownCache( cache );
    }

    //-------------------------------------------------------------------------
    void CursorVault::_ShutdownCache( CursorCache& cache )
    {
        // if loaded
        ASSERT( cache.hcursor != NULL );

        // free cursor
        DestroyCursor(cache.hcursor);

        // reset
        cache.referenced = 0;
        cache.hcursor = NULL;
    }
}

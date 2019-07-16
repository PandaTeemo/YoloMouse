#include <YoloMouse/Dll/Cursor/HandleCache.hpp>

namespace Yolomouse
{
    // Entry
    //-------------------------------------------------------------------------
    HandleCache::Entry::Entry():
        handle  (NULL),
        hash    (0)
    {
    }

    // public
    //-------------------------------------------------------------------------
    HandleCache::HandleCache():
        _next_index(0)
    {
        _cache.Zero();
    }

    //-------------------------------------------------------------------------
    Hash HandleCache::GetHash( HCURSOR handle )
    {
        // ignore invalid handle
        if( handle == NULL )
            return 0;

        // if cache limit reached recalculate manually every time
        if( _next_index == CURSOR_CACHE_LIMIT )
            return _CalculateHash(handle);

        // locate existing handle
        for( CacheIterator i = _cache.begin(); i != _cache.end(); ++i )
            if( i->handle == handle )
                return i->hash;

        // calculate hash of cursor
        Hash hash = _CalculateHash(handle);
        if( hash )
        {
            // create entry
            Entry& entry = _cache[_next_index++];
            entry.handle = handle;
            entry.hash = hash;
        }

        return hash;
    }

    // private
    //-------------------------------------------------------------------------
    Hash HandleCache::_CalculateHash( HCURSOR hcursor )
    {
        ICONINFO    iconinfo = {0};
        LONG        buffer_limit;
        Byte*       buffer;
        BITMAP      bitmap;
        HBITMAP     hbitmap;
        Hash        hash = 0;

        // require valid
        if( hcursor == NULL )
        {
            LOG("HandleCache.CalculateHash.CursorIsNull");
            return 0;
        }

        // get icon info
        if( GetIconInfo(hcursor, &iconinfo) == FALSE )
        {
            LOG("HandleCache.CalculateHash.GetIconInfo: %u", GetLastError());
            return 0;
        }

        // select color else mask bitmap
        hbitmap = iconinfo.hbmColor ? iconinfo.hbmColor : iconinfo.hbmMask;

        // get icon bitmap object
        if( GetObject(hbitmap, sizeof(BITMAP), &bitmap) == 0 )
        {
            LOG("HandleCache.CalculateHash.GetObject");
            return 0;
        }

        // allocate buffer
        buffer_limit = bitmap.bmWidthBytes * bitmap.bmHeight;
        /*
            TODO: this is a temporary hack added to 0.9.1 to make earlier 8k buffer
            limit cursors semi backward compatible. remove in 2019.
        */
        //HACK:begin
        if( buffer_limit > KILOBYTES( 8 ) && buffer_limit <= KILOBYTES( 12 ) )
            buffer_limit = KILOBYTES( 8 );
        //HACK:end
        buffer = new Byte[buffer_limit];

        // if buffer created
        if( buffer != nullptr )
        {
            // read icon bitmap pixel data
            ULong buffer_count = GetBitmapBits( hbitmap, buffer_limit, buffer );

            // log if failed
            if( buffer_count == 0 )
                LOG("HandleCache.CalculateHash.GetBitmapBits");
            // else generate hash
            else
                hash = Tools::Fnv164Hash(buffer, buffer_count);

            // cleanup buffer
            delete[] buffer;
        }

        // cleanup  icon info
        if( iconinfo.hbmColor )
            DeleteObject(iconinfo.hbmColor);
        if( iconinfo.hbmMask )
            DeleteObject(iconinfo.hbmMask);

        return hash;
    }
}

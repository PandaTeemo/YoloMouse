#include <YoloMouse/Dll/HandleCache.hpp>
#include <YoloMouse/Share/SharedState.hpp>

namespace YoloMouse
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
    }

    //-------------------------------------------------------------------------
    Hash HandleCache::GetHash( HCURSOR handle )
    {
        // if cache limit reached recalculate manually every time
        if( _next_index == CURSOR_CACHE_LIMIT )
            return _CalculateHash(handle);

        // locate existing handle
        for( CacheIterator i = _cache.Begin(); i != _cache.End(); ++i )
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
        LONG        count;
        HBITMAP     hbitmap;
        BITMAP      bitmap;
        ULong       bufferLimit;
        Byte*       buffer;
        Hash        hash;

        // require valid
        if( hcursor == NULL )
        {
            elog("HandleCache.CalculateHash.CursorIsNull");
            return 0;
        }

        // get icon info
        if( GetIconInfo(hcursor, &iconinfo) == FALSE )
        {
            elog("HandleCache.CalculateHash.GetIconInfo");
            return 0;
        }
        hbitmap = iconinfo.hbmColor ? iconinfo.hbmColor : iconinfo.hbmMask;

        // get buffer size
        if( GetObject(hbitmap, sizeof(BITMAP), &bitmap) == 0 )
        {
            elog("HandleCache.CalculateHash.GetObject");
            return 0;
        }

        bufferLimit = bitmap.bmWidthBytes * bitmap.bmHeight;
        buffer = new Byte[bufferLimit];

        // get icon bitmap buffer
        count = GetBitmapBits( hbitmap, bufferLimit, buffer );

        // iconinfo cleanup 
        if( iconinfo.hbmColor )
            DeleteObject(iconinfo.hbmColor);
        if( iconinfo.hbmMask )
            DeleteObject(iconinfo.hbmMask);

        // fail if no bits read
        if( count == 0 )
        {
            elog("HandleCache.CalculateHash.GetBitmapBits");
            return 0;
        }

        // generate hash
        hash = Tools::Fnv164Hash(buffer, count);

        // buffer cleanup
        delete[] buffer;

        return hash;
    }
}

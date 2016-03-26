/*
    This optimizes the need to recalculate the hash of a cursor every time
    it changes by keeping a cache of handles-to-hashes. 
    
    Unfortunately some games (like Diablo3) are written by tards where they
    constantly create new cursor handles for no good reason. If this happens
    the cache limit will reach quickly, but instead of rotating GetHash() will
    just recalculate the hash each time from that point on. This limit should
    not be reached for properly engineered games.
*/
#pragma once
#include <Core/Container/Array.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    /**/
    class HandleCache
    {
    public:
        /**/
        HandleCache();

        /**/
        Hash GetHash( HCURSOR handle );

    private:
        // types
        struct Entry
        {
            HCURSOR handle;
            Hash    hash;

            Entry();
        };
        typedef FlatArray<Entry, CURSOR_CACHE_LIMIT> CacheMap;
        typedef CacheMap::Iterator                   CacheIterator;

        /**/
        Hash _CalculateHash( HCURSOR hcursor );
    
        // fields
        CacheMap    _cache;
        Index       _next_index;
    };
}

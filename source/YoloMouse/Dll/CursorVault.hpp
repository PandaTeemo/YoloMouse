#pragma once
#include <Core/Container/Array.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedState.hpp>

namespace YoloMouse
{
    /**/
    class CursorVault
    {
    public:
        /**/
        CursorVault();
        ~CursorVault();
    
        /**/
        Bool HasCursor( HCURSOR hcursor );

        /**/
        HCURSOR GetIdentity( Hash cursor_hash, Index size_index );
        HCURSOR GetPreset( Index preset_index, Index size_index );

        /**/
        Bool LoadIdentity( Hash cursor_hash, Index size_index, HCURSOR hcursor );
        Bool LoadPreset( Index preset_index, Index size_index );

        /**/
        void UnloadIdentity( Hash cursor_hash, Index size_index );
        void UnloadPreset( Index preset_index, Index size_index );
        void UnloadAll();

    private:
        // types
        enum ResourceState
        {
            RESOURCE_NONE,
            RESOURCE_READY,
            RESOURCE_FAILED,
        };

        struct CursorCache
        {
            // fields
            HCURSOR         hcursor;
            ULong           referenced;

            /**/
            CursorCache();
        };
        typedef FlatArray<CursorCache, CURSOR_SIZE_INDEX_COUNT> CacheTable;

        struct IdentityEntry
        {
            // fields
            Hash            cursor_hash;
            CacheTable      cache_table;

            /**/
            IdentityEntry();

            /**/
            Bool operator==( const Hash& cursor_hash_ ) const;
        };

        struct PresetEntry
        {
            // fields
            ResourceState   state;
            Bool            resizable;
            ULong           width;
            ULong           height;
            PathString      path;
            UINT            loadimage_flags;
            CacheTable      cache_table;

            /**/
            PresetEntry();
        };

        typedef FixedArray<IdentityEntry, CURSOR_RESOURCE_IDENTITY_LIMIT>   IdentityMap;
        typedef FlatArray<PresetEntry, CURSOR_RESOURCE_PRESET_COUNT>        PresetTable;

        /**/
        Bool _LoadIdentity( IdentityEntry& entry, Index size_index, HCURSOR hcursor );
        void _UnloadIdentity( IdentityEntry& entry, Index size_index );

        /**/
        Bool _InitPreset( PresetEntry& entry, Index preset_index );
        Bool _LoadPreset( PresetEntry& entry, Index size_index );
        void _UnloadPreset( PresetEntry& entry, Index size_index );

        /**/
        void _UnloadCache( CursorCache& cache );

        // fields
        IdentityMap     _identity_map;
        PresetTable     _preset_table;
        SharedState&    _state;
    };
}

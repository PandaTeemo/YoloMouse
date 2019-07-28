#pragma once
#include <Core/Container/Array.hpp>
#include <Core/Container/Array2.hpp>
#include <Core/Container/Map.hpp>
#include <Core/Types.hpp>
#include <YoloMouse/Share/Cursor/CursorTypes.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace Yolomouse
{
    /**/
    class CursorVault
    {
    public:
        /**/
        CursorVault();
        ~CursorVault();

        /**/
        void Initialize( const PathString& host_path );
        void Shutdown();
    
        /**/
        Bool IsInitialized() const;

        /**/
        Bool HasCursor( HCURSOR hcursor );

        /**/
        HCURSOR GetBasic( CursorId id, CursorVariation variation, CursorSize size );
        HCURSOR GetClone( Hash cursor_hash, CursorSize size );
        HCURSOR GetHidden();

        /**/
        HCURSOR LoadBasic( CursorId id, CursorVariation variation, CursorSize size );
        HCURSOR LoadClone( HCURSOR hcursor, Hash cursor_hash, CursorSize size );

        /**/
        void UnloadBasic( CursorId id, CursorVariation variation, CursorSize size );
        void UnloadClone( Hash cursor_hash, CursorSize size );
        void UnloadAll();

    private:
        // enums
        enum ResourceState
        {
            RESOURCE_NONE,
            RESOURCE_READY,
            RESOURCE_FAILED,
        };

        // types
        struct CursorCache
        {
            // fields
            HCURSOR         hcursor;
            ULong           referenced;

            /**/
            CursorCache();
            ~CursorCache();
        };
        typedef FlatArray<CursorCache, CURSOR_SIZE_COUNT> CacheTable;

        struct BasicEntry
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
            BasicEntry();
        };
        typedef FlatArray2<BasicEntry, CURSOR_ID_COUNT, CURSOR_VARIATION_COUNT> BasicTable;

        struct CloneEntry
        {
            // fields
            CacheTable      cache_table;
        };
        typedef Map<Hash, CloneEntry> CloneMap;

        /**/
        void _InitializeHiddenCursor();

        /**/
        void _ShutdownCaches();
        void _ShutdownHiddenCursor();

        /**/
        Bool    _PreLoadBasic( BasicEntry& entry, CursorId id, CursorVariation variation );
        Bool    _PreLoadBasicEntry( BasicEntry& entry );
        HCURSOR _LoadBasic( BasicEntry& entry, CursorSize size );
        void    _UnloadBasic( BasicEntry& entry, CursorSize size );

        /**/
        HCURSOR _LoadClone( CloneEntry& entry, HCURSOR hcursor, CursorSize size );
        void    _UnloadClone( CloneEntry& entry, CursorSize size );

        /**/
        void _ShutdownCache( CursorCache& cache );

        // fields: parameters
        PathString      _host_path;
        // fields: state
        Bool            _initialized;
        BasicTable      _basic_table;
        CloneMap        _clone_map;
        HCURSOR         _hidden_cursor;
    };
}

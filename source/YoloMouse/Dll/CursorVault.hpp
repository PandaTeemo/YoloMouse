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
        Bool Load( Index resource_index, Index size_index );

        /**/
        void Unload( Index resource_index, Index size_index );
        void UnloadAll();

        /**/
        HCURSOR GetCursor( Index resource_index, Index size_index );

        /**/
        Bool HasCursor( HCURSOR hcursor );

    private:
        // types
        enum ResourceState
        {
            RESOURCE_NONE,
            RESOURCE_READY,
            RESOURCE_FAILED,
        };

        struct CursorResource
        {
            HCURSOR         handle;
            ULong           referenced;

            CursorResource();
        };
        typedef FlatArray<CursorResource, CURSOR_SIZE_COUNT> ResourceTable;

        struct CacheEntry
        {
            ResourceState   state;
            Bool            resizable;
            ULong           width;
            ULong           height;
            PathString      path;
            UINT            loadimage_flags;
            ResourceTable   resources;

            CacheEntry();
        };
        typedef FlatArray<CacheEntry, CURSOR_RESOURCE_LIMIT> CacheTable;

        /**/
        Bool _CacheInit( CacheEntry& entry, Index resource_index );
        Bool _CacheLoad( CacheEntry& entry, Index size_index );
        void _CacheUnload( CacheEntry& entry, Index size_index );

        /**/
        void _IdealResize( ULong& out_width, ULong& out_height, Index target_height, ULong threshold );

        // fields
        CacheTable   _table;
        SharedState& _state;
    };
}

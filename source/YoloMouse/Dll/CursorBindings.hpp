#pragma once
#include <Core/Container/Array.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    /**/
    class CursorBindings
    {
    public:
        /**/
        struct Mapping
        {
            Hash    _hash;  // hash of original cursor image bits
            Index   _index; // replacement cursor file index

            Bool operator==( const Hash& hash ) const;
        };

    private:
        /**/
        typedef FlatArray<Mapping, CURSOR_MAPPING_LIMIT> MapTable;
        typedef MapTable::Iterator MapIterator;

    private:
        MapTable _map;

    public:
        /**/
        CursorBindings();
    
        /**/
        const Mapping& Get( Index mapping_index ) const;

        /**/
        Index Add( Hash cursor_hash, Index cursor_index );

        /**/
        void Remove( Index mapping_index );

        /**/
        Index Find( Hash cursor_hash );

        /**/
        Bool Load( const WCHAR* target_id );
        Bool Save( const WCHAR* target_id );
    };
}

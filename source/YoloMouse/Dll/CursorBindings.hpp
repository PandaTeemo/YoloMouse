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
        struct Binding
        {
            Hash    bitmap_hash;    // hash of original cursor image bits
            Index   resource_index; // cursor resource index
            Index   size_index;     // size index

            /**/
            Binding();
            Binding( Hash bitmap_hash_, Index resource_index_, Index size_index_ );

            /**/
            Bool Isvalid() const;

            /**/
            Bool operator==( const Hash& hash_ ) const;
        };

        /**/
        typedef FlatArray<Binding, CURSOR_BINDING_LIMIT> MapTable;
        typedef MapTable::Iterator MapIterator;

        /**/
        CursorBindings();
    
        /**/
        Binding*        GetBinding( Hash cursor_hash ) const;
        const MapTable& GetMap() const;

        /**/
        Binding& EditDefault();

        /**/
        Binding* Add( Hash cursor_hash, Index resource_index, Index size_index );

        /**/
        void Remove( CursorBindings::Binding& binding );

        /**/
        Bool Load( const WCHAR* target_id );
        Bool Save( const WCHAR* target_id );

    private:
        // fields
        MapTable _bindings;
        Binding  _default;
    };
}

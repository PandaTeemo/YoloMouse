#pragma once
#include <Core/Container/Array.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Enums.hpp>
#include <stdio.h>

namespace YoloMouse
{
    /**/
    class CursorBindings
    {
    public:
        // types
        struct Binding
        {
            Hash            cursor_hash;    // hash of original cursor image bits
            Index           size_index;     // size index
            ResourceType    resource_type;  // resource type
            union
            {
                Index       resource_index;
                Index       preset_index;   // preset file index
            };

            /**/
            Binding();

            /**/
            Bool operator==( const Hash& cursor_hash_ ) const;
        };

        // aliases
        typedef FlatArray<Binding, CURSOR_BINDING_LIMIT> BindingCollection;

        /**/
        CursorBindings();
    
        /**/
        Binding*                 GetBindingByHash( Hash cursor_hash ) const;
        const BindingCollection& GetBindings() const;

        /**/
        Binding* Add( const Binding& binding );

        /**/
        void Remove( CursorBindings::Binding& binding );

        /**/
        Bool Load( const WCHAR* target_id );
        Bool Save( const WCHAR* target_id );

    private:
        /**/
        Bool _ReadBindingLine( Binding& binding, FILE* file );
        Bool _WriteBindingLine( const Binding& binding, FILE* file );

        /**/
        static Bool _ValidateBinding( const Binding& binding );

        // fields
        BindingCollection _bindings;
    };
}

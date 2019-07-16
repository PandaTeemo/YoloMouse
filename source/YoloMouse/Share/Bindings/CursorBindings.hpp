#pragma once
#include <Core/Container/Map.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Enums.hpp>

namespace Yolomouse
{
    /**/
    class CursorBindings
    {
    public:
        // types
        struct Binding
        {
            CursorType      type;
            CursorId        id;
            CursorVariation variation;
            CursorSize      size;

            /**/
            Binding();

            /**/
            Bool IsValid() const;
        };

        // aliases
        typedef Map<Hash, Binding> BindingMap;

        /**/
        CursorBindings();
        ~CursorBindings();
    
        /**/
        Binding*          GetBinding( Hash cursor_hash );
        const BindingMap& GetBindings() const;
        Binding&          GetDefaultBinding();
        const Binding&    GetDefaultBinding() const;

        /**/
        Binding* CreateBinding( Hash hash );

        /**/
        Bool RemoveBinding( Hash hash );

    private:
        // fields
        BindingMap  _bindings;
        Binding     _default_binding;
    };
}

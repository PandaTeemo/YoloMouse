#pragma once
#include <Core/Container/Map.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Cursor/CursorInfo.hpp>

namespace Yolomouse
{
    /**/
    class CursorBindings
    {
    public:
        // aliases
        typedef Map<Hash, CursorInfo> BindingMap;

        /**/
        CursorBindings();
        ~CursorBindings();
    
        /**/
        CursorInfo*       GetBinding( Hash cursor_hash );
        const BindingMap&       GetBindings() const;
        CursorInfo&       GetDefaultBinding();
        const CursorInfo& GetDefaultBinding() const;

        /**/
        CursorInfo* CreateBinding( Hash hash );

        /**/
        Bool RemoveBinding( Hash hash );

    private:
        // fields
        BindingMap       _bindings;
        CursorInfo _default_binding;
    };
}

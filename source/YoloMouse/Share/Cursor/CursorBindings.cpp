#include <YoloMouse/Share/Cursor/CursorBindings.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    CursorBindings::CursorBindings()
    {
    }

    CursorBindings::~CursorBindings()
    {
    }

    //-------------------------------------------------------------------------
    CursorInfo* CursorBindings::GetBinding( Hash hash )
    {
        return _bindings.Get( hash );
    }

    const CursorBindings::BindingMap& CursorBindings::GetBindings() const
    {
        return _bindings;
    }

    CursorInfo& CursorBindings::GetDefaultBinding()
    {
        return _default_binding;
    }

    const CursorInfo& CursorBindings::GetDefaultBinding() const
    {
        return _default_binding;
    }

    //-------------------------------------------------------------------------
    CursorInfo* CursorBindings::CreateBinding( Hash hash )
    {
        return &_bindings.Set( hash );
    }

    //-------------------------------------------------------------------------
    Bool CursorBindings::RemoveBinding( Hash hash )
    {
        return _bindings.Remove( hash );
    }
}

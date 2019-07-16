#include <YoloMouse/Share/Bindings/CursorBindings.hpp>

namespace Yolomouse
{
    // Binding.public
    //-------------------------------------------------------------------------
    CursorBindings::Binding::Binding():
        type     (CURSOR_TYPE_INVALID),
        id       (CURSOR_ID_INVALID),
        variation(CURSOR_VARIATION_INVALID),
        size     (CURSOR_SIZE_DEFAULT)
    {
    }

    //-------------------------------------------------------------------------
    Bool CursorBindings::Binding::IsValid() const
    {
        // validate type and associated parameters
        switch( type )
        {
        case CURSOR_TYPE_BASIC:
        case CURSOR_TYPE_OVERLAY:
            return id < CURSOR_ID_COUNT && variation < CURSOR_VARIATION_COUNT && size < CURSOR_SIZE_COUNT;
        case CURSOR_TYPE_CLONE:
            return size < CURSOR_SIZE_COUNT;
        default:
            return false;
        }
    }

    // public
    //-------------------------------------------------------------------------
    CursorBindings::CursorBindings()
    {
    }

    CursorBindings::~CursorBindings()
    {
    }

    //-------------------------------------------------------------------------
    CursorBindings::Binding* CursorBindings::GetBinding( Hash hash )
    {
        return _bindings.Get( hash );
    }

    const CursorBindings::BindingMap& CursorBindings::GetBindings() const
    {
        return _bindings;
    }

    CursorBindings::Binding& CursorBindings::GetDefaultBinding()
    {
        return _default_binding;
    }

    const CursorBindings::Binding& CursorBindings::GetDefaultBinding() const
    {
        return _default_binding;
    }

    //-------------------------------------------------------------------------
    CursorBindings::Binding* CursorBindings::CreateBinding( Hash hash )
    {
        return &_bindings.Set( hash );
    }

    //-------------------------------------------------------------------------
    Bool CursorBindings::RemoveBinding( Hash hash )
    {
        return _bindings.Remove( hash );
    }
}

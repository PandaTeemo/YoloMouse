#pragma once
#include <YoloMouse/Share/Cursor/CursorBindings.hpp>
#include <stdio.h>

namespace Yolomouse
{
    /**/
    class CursorBindingsSerializer
    {
    public:
        /**/
        static Bool Load( CursorBindings& bindings, const PathString& bindings_path );
        static Bool Save( const CursorBindings& bindings, const PathString& bindings_path );

    private:
        /**/
        static Bool _ReadBindingLine( Hash& hash, CursorInfo& info, FILE* file );
        static Bool _WriteBindingLine( const Hash hash, const CursorInfo& info, FILE* file );
    };
}

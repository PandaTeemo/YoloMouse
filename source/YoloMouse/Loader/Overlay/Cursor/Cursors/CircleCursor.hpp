#pragma once
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/BaseCursor.hpp>

namespace Yolomouse
{
    /**/
    class CircleCursor:
        public BaseCursor
    {
    public:
        /**/
        CircleCursor();
        ~CircleCursor();

    private:
        /**/
        Bool _OnInitialize();

        /**/
        void _OnShutdown();

        /**/
        void _OnUpdate( UpdateDef& def );

    private:
        /**/
        Bool _InitializeMesh();

        // fields: state
        Float _rotater;
    };
}

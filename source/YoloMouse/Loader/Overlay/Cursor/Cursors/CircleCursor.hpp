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
        void _OnUpdate( UpdateDef2& def );

    private:
        /**/
        Bool _InitializeGeometry();

        // fields: state
        Float _rotater;
    };
}

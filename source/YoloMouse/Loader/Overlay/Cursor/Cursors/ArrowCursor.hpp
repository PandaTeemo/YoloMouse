#pragma once
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/BaseCursor.hpp>

namespace Yolomouse
{
    /**/
    class ArrowCursor:
        public BaseCursor
    {
    public:
        /**/
        ArrowCursor();
        ~ArrowCursor();

    private:
        /**/
        Bool _OnInitialize();

        /**/
        void _OnShutdown();

        /**/
        void _OnUpdate( UpdateDef2& def );

    private:
        // fields: state
        Float _rotater;
    };
}
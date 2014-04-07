#pragma once
#include <Core/Container/Array.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    /**/
    class Loader
    {
    private:
        struct Active
        {
            HWND    _hwnd;
            HHOOK   _hook;

            Bool operator==( HWND hwnd ) const;
        };

        typedef FixedArray<Active, LOADER_ACTIVE_LIMIT> ActiveCollection;
        typedef ActiveCollection::Iterator              ActiveIterator;

    private:
        HMODULE             _dll;
        HOOKPROC            _hook_function;
        ActiveCollection    _actives;

    public:
        /**/
        Loader();

        /**/
        static HWND GetActiveTarget();

        /**/
        Bool IsStarted() const;
        Bool IsLoaded( HWND hwnd ) const;
        Bool IsConfigured( HWND hwnd ) const;

        /**/
        Bool Start();
        void Stop();

        /**/
        Bool Load( HWND hwnd );
        Bool Unload( HWND hwnd );

        /**/
        void NotifyUpdate( HWND hwnd, Index cursor_index );
    };
}

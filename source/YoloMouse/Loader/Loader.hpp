#pragma once
#include <Core/Container/Array.hpp>
#include <Snoopy/Inject/Injector.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/NotifyMessage.hpp>

namespace YoloMouse
{
    /**/
    class Loader
    {
    private:
        /**/
        struct Active
        {
            HWND        _hwnd;
            Injector*   _injector;

            Bool operator==( HWND hwnd ) const;
        };

        typedef FixedArray<Active, LOADER_ACTIVE_LIMIT> ActiveCollection;
        typedef ActiveCollection::Iterator              ActiveIterator;

    private:
        // state
        ActiveCollection _actives;

    public:
        /**/
        Loader();
        ~Loader();

        /**/
        static HWND GetActiveTarget();

        /**/
        Bool IsLoaded( HWND hwnd ) const;
        Bool IsConfigured( HWND hwnd ) const;

        /**/
        Bool Load( HWND hwnd );
        Bool Unload( HWND hwnd );

        /**/
        Bool Notify( HWND hwnd, NotifyId id, Byte8 parameter=0 );

    private:
        /**/
        void _UnloadActive( Active& active );

        /**/
        const CHAR* _ChooseInjectDll( DWORD process_id );
    };
}

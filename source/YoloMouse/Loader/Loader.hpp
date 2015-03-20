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
            DWORD       process_id;
            Injector*   injector;

            Bool operator==( DWORD process_id ) const;
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
        static DWORD GetActiveProcessId();

        /**/
        Bool IsLoaded( DWORD process_id ) const;
        Bool IsConfigured( DWORD process_id ) const;

        /**/
        Bool Load( DWORD process_id );
        Bool Unload( DWORD process_id );

        /**/
        Bool Notify( DWORD process_id, NotifyId id, Byte8 parameter=0 );

    private:
        /**/
        void _UnloadActive( Active& active );

        /**/
        const CHAR* _ChooseInjectDll( DWORD process_id );
    };
}

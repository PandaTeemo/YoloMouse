#pragma once
#include <Core/Container/String.hpp>
#include <Snoopy/Root.hpp>

namespace Snoopy
{
    /**/
    class Injector
    {
    public:
        /**/
        Injector();
        ~Injector();

        /**/
        Bool IsLoaded() const;

        /**/
        Bool Load( HANDLE process, const CHAR* dll_path );

        /**/
        void Unload();

    private:
        /**/
        Bool _Load( const CHAR* dll_path );
        void _Unload();

        /**/
        Bool _CallRemoteLoadLibrary( const CHAR* dll_path );

        /**/
        Bool _LoadSymbolModule( DWORD64& address, const CHAR* name );

        // fields
        HANDLE  _process;
        Bool    _loaded;
    };
}

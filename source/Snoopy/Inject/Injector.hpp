#pragma once
#include <Core/Container/String.hpp>
#include <Snoopy/Root.hpp>

namespace Snoopy
{
    /**/
    class Injector
    {
    private:
        enum FunctionId 
        {
            FUNCTION_LOADLIBRARY,
            FUNCTION_FREELIBRARY,
            FUNCTION_NOTIFY,
            FUNCTION_COUNT
        };

        struct Function
        {
            const CHAR*  name;
            UHuge        address;

            Function();
        };

        // handles
        HANDLE      _process;

        // state
        void*       _argument_memory;
        DWORD64     _inject_base;
        Function    _functions[FUNCTION_COUNT];

    public:
        /**/
        Injector();
        ~Injector();

        /**/
        void SetNotifyName( const CHAR* name );

        /**/
        Bool Load( HANDLE process, const CHAR* dll_path );

        /**/
        void Unload();

        /**/
        Bool CallNotify( const void* argument, ULong size=0 );

    private:
        /**/
        Bool _GetFunctionAddress( UHuge& address, const CHAR* name );

        /**/
        Bool _CallFunction( FunctionId id, const void* argument, ULong size=0 );

        /**/
        Bool _InjectDll( const CHAR* dll_path );

        /**/
        DWORD64 _LoadSymbolModule( const CHAR* name );

        /**/
        void _Reset();
    };
}

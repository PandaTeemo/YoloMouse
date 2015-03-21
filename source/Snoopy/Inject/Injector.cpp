#include <Snoopy/Constants.hpp>
#include <Snoopy/Inject/Injector.hpp>
#include <Dbghelp.h>
#include <TlHelp32.h>

namespace Snoopy
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        const ULong LOADSYMBOLMODULE_RETRIES =      10;
        const ULong LOADSYMBOLMODULE_RETRYDELAY =   20;     //ms
        const ULong LOAD_WAIT_DELAY =               3000;   //ms
    }

    // Function
    //-------------------------------------------------------------------------
    Injector::Function::Function():
        name    (NULL),
        address (0)
    {
    }

    // public
    //-------------------------------------------------------------------------
    Injector::Injector()
    {
        // init
        _Reset();

        // register functions
        _functions[FUNCTION_LOADLIBRARY].name = "LoadLibraryA";
        _functions[FUNCTION_FREELIBRARY].name = "FreeLibrary";
    }

    Injector::~Injector()
    {
        Unload();
    }

    //-------------------------------------------------------------------------
    void Injector::SetNotifyName( const CHAR* name )
    {
        _functions[FUNCTION_NOTIFY].name = name;
    }

    //-------------------------------------------------------------------------
    Bool Injector::Load( HANDLE process, const CHAR* dll_path )
    {
        xassert(_process == NULL);

        // wait until ready
        if( WaitForInputIdle(process, LOAD_WAIT_DELAY) == 0 )
        {
            // initialize symbol loader
            if( SymInitialize(process, NULL, FALSE) )
            {
                // save process handle
                _process = process;

                // load initial modules
                if( _LoadSymbolModule("kernel32.dll") )
                {
                    // allocate argument memory
                    _argument_memory = VirtualAllocEx(process, NULL, INJECTOR_ARGUMENT_MEMORY, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

                    // if allocated
                    if( _argument_memory )
                    {
                        // inject dll
                        if( _InjectDll(dll_path) )
                            return true;
                    }
                }
            }

            // fail
            Unload();
        }

        return false;
    }

    //-------------------------------------------------------------------------
    void Injector::Unload()
    {
        // unload dll if successfully injected
        if( _inject_base )
            _CallFunction(FUNCTION_FREELIBRARY, (void*)_inject_base);

        // free argument memory
        if( _argument_memory )
            VirtualFreeEx(_process, _argument_memory, INJECTOR_ARGUMENT_MEMORY, MEM_RELEASE);

        // close symbol handler
        if( _process )
            SymCleanup(_process);

        // reset state
        _Reset();
    }

    //-------------------------------------------------------------------------
    Bool Injector::CallNotify( const void* argument, ULong size )
    {
        xassert(_process);
        return _CallFunction(FUNCTION_NOTIFY, argument, size);
    }

    // private
    //-------------------------------------------------------------------------
    Bool Injector::_GetFunctionAddress( UHuge& address, const CHAR* name )
    {
        SYMBOL_INFO symbol = {0};

        // init parameters
        symbol.SizeOfStruct = sizeof(symbol);

        // get address of function
        if( !SymFromName(_process, name, &symbol) || symbol.Address == 0 )
            return false;

        // store address
        address = symbol.Address;
        return true;
    }

    //-------------------------------------------------------------------------
    Bool Injector::_InjectDll( const CHAR* dll_path )
    {
        CHAR   full_path[STRING_PATH_SIZE];
        CHAR*  pdll_name = NULL;
        xassert(_process);

        // get full dll path
        if(!GetFullPathNameA(dll_path, COUNT(full_path), full_path, &pdll_name))
            return false;

        // tell process to load dll
        if( !_CallFunction(FUNCTION_LOADLIBRARY, full_path, static_cast<ULong>(strlen(full_path) + 1)) )
            return false;

        // load symbols for new dll
        _inject_base = _LoadSymbolModule(pdll_name);
        if(_inject_base == 0)
            return false;

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Injector::_CallFunction( FunctionId id, const void* argument, ULong size )
    {
        xassert(id < FUNCTION_COUNT);
        Function&   f = _functions[id];
        void*       remote_argument = 0;

        // if not loaded
        if( f.address == 0 )
        {
            xassert(f.name);

            // get address of function
            if( !_GetFunctionAddress(f.address, f.name) )
                return false;
        }

        // if argument is a pointer to an object
        if( size > 0 )
        {
            xassert(size <= INJECTOR_ARGUMENT_MEMORY);

            // write object to argument memory
            if(!WriteProcessMemory(_process, _argument_memory, argument, size, NULL))
                return false;

            // argument is pointer to argument memory
            remote_argument = _argument_memory;
        }
        // else argument is the provided value
        else
            remote_argument = const_cast<void*>(argument);

        // call function
        HANDLE remote_thread = CreateRemoteThread(
            _process,
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)f.address,
            remote_argument,
            NULL,
            NULL);
        if( remote_thread == NULL )
            return false;

        // wait for completion
        Bool status = WaitForSingleObject(remote_thread, INJECTOR_FUNCTION_WAIT) == WAIT_OBJECT_0;

        // close thread object
        CloseHandle(remote_thread);

        return status;
    }

    //-------------------------------------------------------------------------
    DWORD64 Injector::_LoadSymbolModule( const CHAR* name )
    {
        MODULEENTRY32 me;
        HANDLE        handle;

        // loop until retries exhausted
        for( ULong retry = 0; retry < LOADSYMBOLMODULE_RETRIES; ++retry )
        {
            // create toolhelp snapshot
            handle = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32, GetProcessId(_process) );

            // if created
            if( handle != INVALID_HANDLE_VALUE )
            {
                DWORD64 base_address;

                // init
                me.dwSize = sizeof(me);

                // walk modules
                if( Module32First(handle, &me) )
                {
                    do
                    {
                        // if matches
                        if( _stricmp(me.szModule, name) == 0 )
                        {
                            // load module into symbol loader
                            base_address = SymLoadModuleEx(
                                _process,
                                NULL,
                                me.szExePath,
                                me.szModule,
                                (DWORD64)me.modBaseAddr,
                                me.modBaseSize,
                                NULL,
                                0);

                            break;
                        }
                    }
                    while( Module32Next(handle, &me) );
                }

                // close toolhelp snapshot
                CloseHandle(handle);

                return base_address;
            }

            // wait a little
            Sleep(LOADSYMBOLMODULE_RETRYDELAY);
        }

        return 0;
    }

    //-------------------------------------------------------------------------
    void Injector::_Reset()
    {
        _process = NULL;
        _argument_memory = NULL;
        _inject_base = 0;

        for( Index i = 0; i < FUNCTION_COUNT; ++i )
            _functions[i].address = 0;
    }
}

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
        const Char* DLL_KERNEL32_NAME =     "kernel32.dll";
        const ULong LOAD_WAIT_DELAY =       2000; // ms
        const ULong LOAD_ATTEMPT_COUNT =    3;
        const ULong ARGUMENT_MEMORY_SIZE =  STRING_PATH_SIZE;
        const Char* FUNCTION_NAME =         "LoadLibraryA";
        const ULong FUNCTION_WAIT_TIME =    3000; // ms
        const ULong FUNCTION_STACK_SIZE =   KILOBYTES(32);
    }

    // public
    //-------------------------------------------------------------------------
    Injector::Injector():
        _process(NULL),
        _loaded (false)
    {
    }

    Injector::~Injector()
    {
        ASSERT( !IsLoaded() );
    }

    //-------------------------------------------------------------------------
    Bool Injector::IsLoaded() const
    {
        return _loaded;
    }

    //-------------------------------------------------------------------------
    Bool Injector::Load( HANDLE process, const CHAR* dll_path )
    {
        ASSERT(!IsLoaded());

        // set fields
        _process = process;

        // load attempt loop
        for( Index attempt = 0; attempt < LOAD_ATTEMPT_COUNT; ++attempt )
        {
            // load can sometimes fail when it shouldnt due to "reasons" so attempt multiple times
            if( _Load( dll_path ) )
            {
                // set loaded
                _loaded = true;
                return true;
            }

            // undo
            _Unload();
        }

        return false;
    }

    //-------------------------------------------------------------------------
    void Injector::Unload()
    {
        ASSERT(IsLoaded());

        // unload
        _Unload();

        // reset state
        _loaded = false;
    }

    // private
    //-------------------------------------------------------------------------
    Bool Injector::_Load( const CHAR* dll_path )
    {
        CHAR    full_dll_path[STRING_PATH_SIZE];
        DWORD64 dll_kernel32_address;

        // wait until ready. this doesnt work in all cases, therefore ignore
        // return value and rely on following code and multiple attempts to 
        // qualify a succcessful injection.
        WaitForInputIdle(_process, LOAD_WAIT_DELAY);

        // initialize symbol loader
        if( !SymInitialize( _process, NULL, FALSE ) )
            return false;

        // load initial modules
        if( !_LoadSymbolModule(dll_kernel32_address, DLL_KERNEL32_NAME) )
            return false;

        // get full dll path
        if(!GetFullPathNameA(dll_path, COUNT(full_dll_path), full_dll_path, NULL))
            return false;

        // tell process to load dll
        if( !_CallRemoteLoadLibrary(full_dll_path) )
            return false;

        return true;
    }

    void Injector::_Unload()
    {
        // close symbol handler
        SymCleanup(_process);
    }

    //-------------------------------------------------------------------------
    Bool Injector::_CallRemoteLoadLibrary( const CHAR* dll_path )
    {
        Bool        status = false;
        SYMBOL_INFO function_symbol = {0};

        // get function address
        function_symbol.SizeOfStruct = sizeof(function_symbol);
        if( !SymFromName(_process, FUNCTION_NAME, &function_symbol) || function_symbol.Address == 0 )
            return false;

        // dll path is argument size
        size_t argument_size = strlen( dll_path ) + 1;

        // allocate argument memory
        void* argument_memory = VirtualAllocEx(_process, NULL, argument_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        if( argument_memory == nullptr )
            return false;

        // write argument to argument memory
        if( WriteProcessMemory( _process, argument_memory, dll_path, argument_size, NULL ) )
        {
            // create/start remote thread pointing to function
            HANDLE remote_thread = CreateRemoteThread(
                _process,
                NULL,
                FUNCTION_STACK_SIZE,
                (LPTHREAD_START_ROUTINE)function_symbol.Address,
                argument_memory,
                0,
                NULL);
            if( remote_thread != NULL )
            {
                // wait for completion
                status = WaitForSingleObject(remote_thread, FUNCTION_WAIT_TIME) == WAIT_OBJECT_0;

                // close remote thread object
                CloseHandle(remote_thread);
            }
        }

        // free argument memory
        VirtualFreeEx(_process, argument_memory, 0, MEM_RELEASE);

        return status;
    }

    //-------------------------------------------------------------------------
    Bool Injector::_LoadSymbolModule( DWORD64& address, const CHAR* name )
    {
        MODULEENTRY32 me;
        HANDLE        handle;

        // create toolhelp snapshot
        handle = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32, GetProcessId(_process) );
        if( handle == INVALID_HANDLE_VALUE )
            return false;

        // init
        address = 0;
        me.dwSize = sizeof(me);

        // walk modules
        if( Module32First(handle, &me) )
        {
            do
            {
                // if matches
                if( _stricmp(me.szModule, name) == 0 )
                {
                    // load module into symbol loader and get 
                    address = SymLoadModuleEx(
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

        return address != 0;
    }
}

#include <Core/System/IpcPair.hpp>
#include <Core/Support/Tools.hpp>
#include <stdio.h>

namespace Core
{
    // public
    //-------------------------------------------------------------------------
    IpcPair::IpcPair():
        _size     (0),
        _listener (nullptr),
        _instance (0),
        _hmap     (NULL),
        _hwait    (NULL)
    {
        for( Index i = 0; i < 2; ++i )
        {
            _hevent[i] = NULL;
            _memory[i] = nullptr;
        }
    }

    IpcPair::~IpcPair()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool IpcPair::Initialize( const Char* name, ULong size, IListener& listener )
    {
        ASSERT( !IsInitialized() );

        // initialize
        if( !_Initialize( name, size, listener ) )
        {
            // undo
            _Shutdown();
            return false;
        }

        return true;
    }

    void IpcPair::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown
        _Shutdown();
    }

    //-------------------------------------------------------------------------
    Bool IpcPair::IsInitialized() const
    {
        return _hmap != nullptr;
    }

    //-------------------------------------------------------------------------
    Index IpcPair::GetInstance() const
    {
        return _instance;
    }

    //-------------------------------------------------------------------------
    Bool IpcPair::Send( const void* message, ULong size, ULong wait_ms )
    {
        ASSERT( size <= _size );
        Index other_instance = !_instance;

        // get other instance memory location
        MemoryEntry* other_memory = _memory[other_instance];

        // fail wait loop
        while( true )
        {
            // sync and lock
            if( InterlockedExchange(&other_memory->lock, 1) == 0 )
            {
                // copy message to memory of other instance
                Tools::MemCopy( other_memory->data, reinterpret_cast<const Byte*>(message), size );

                // signal event of other instance
                return SetEvent( _hevent[other_instance] ) == TRUE;
            }

            // fail if no remaining wait
            if( wait_ms == 0 )
                return false;

            // sleep 1 ms and decrement wait. this is ghetto but efficient if fails are rare.
            Sleep( 1 );
            wait_ms--;
        }
    }

    // private
    //-------------------------------------------------------------------------
    Bool IpcPair::_Initialize( const Char* name, ULong size, IListener& listener )
    {
        Char temp_name[IPC_NAME_LIMIT];

        // set fields
        _size = size;
        _listener = &listener;

        // determine total size for lock and duplex memory use
        ULong memory_size = size + sizeof( Byte4 );
        ULong total_size = memory_size * 2;

        // create memory mapping
        sprintf_s( temp_name, sizeof(temp_name), "m.%s", name );
        _hmap = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, total_size, temp_name );
        if( _hmap == NULL )
            return false;

        // if memory mapping is existing one, become second instance
        if( GetLastError() == ERROR_ALREADY_EXISTS )
            _instance = 1;

        // map memory for each instance
        Byte* mapped_memory = reinterpret_cast<Byte*>(MapViewOfFile( _hmap, FILE_MAP_ALL_ACCESS, 0, 0, total_size));
        if( mapped_memory == nullptr )
            return false;

        // for each instance
        for( Index i = 0; i < 2; ++i )
        {
            // assign mapped memory to memory entry and initialize
            MemoryEntry* memory = _memory[i] = reinterpret_cast<MemoryEntry*>(mapped_memory);
            memory->lock = 0;
            mapped_memory += memory_size;

            // create event
            sprintf_s( temp_name, sizeof(temp_name), "e%u.%s", i, name );
            _hevent[i] = CreateEventA(NULL, FALSE, FALSE, temp_name);
            if( _hevent[i] == nullptr )
                return false;
        }

        // register wait on event for this instance
        if( !RegisterWaitForSingleObject( &_hwait, _hevent[_instance], _EventWakeHandler, this, INFINITE, WT_EXECUTEINPERSISTENTTHREAD ) )
            return false;

        // success!
        return true;
    }

    void IpcPair::_Shutdown()
    {
        // unregister wait on event
        if( _hwait != NULL )
        {
            UnregisterWait( _hwait );
            _hwait = NULL;
        }

        // close events
        for( HANDLE& hevent: _hevent )
        {
            if( hevent != NULL )
            {
                CloseHandle( hevent );
                hevent = NULL;
            }
        }

        // unmap memory
        if( _memory[0] != nullptr )
        {
            UnmapViewOfFile(_memory[0]);
            _memory[0] = nullptr;
            _memory[1] = nullptr;
        }

        // close mapped memory handle
        if( _hmap != NULL )
        {
            CloseHandle(_hmap);
            _hmap = NULL;
        }
    }

    //-------------------------------------------------------------------------
    VOID CALLBACK IpcPair::_EventWakeHandler( _In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired )
    {
        // get this
        IpcPair* dis = reinterpret_cast<IpcPair*>(lpParameter);

        // get memory entry
        MemoryEntry* memory = dis->_memory[dis->_instance];

        // call recv handler
        dis->_listener->OnRecv(memory->data);

        // release lock
        memory->lock = 0;
    }
}

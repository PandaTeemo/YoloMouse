#pragma once
#include <Core/Root.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class SharedMemory
    {
    private:
        const Char* _name;
        HANDLE      _handle;
        TYPE*       _memory;

    public:
        /**/
        SharedMemory( const Char* name ):
            _name   (name),
            _handle (NULL),
            _memory (NULL)
        {
        }

        ~SharedMemory()
        {
        }

        /**/
        Bool Open( Bool host )
        {
            xassert(_memory == NULL);
            xassert(_handle == NULL);

            // if host create
            if( host )
                _handle = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TYPE), _name );
            // else use existing
            else
                _handle = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, _name );
            if(_handle == NULL)
                return false;

            // map to memory
            _memory = (TYPE*)MapViewOfFile( _handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TYPE));
            if(_memory == NULL)
            {
                CloseHandle(_handle);
                return false;
            }

            return true;
        }

        void Close()
        {
            xassert(_memory);
            xassert(_handle);

            // unmap from memory
            UnmapViewOfFile(_memory);

            // close shared memory handle
            CloseHandle(_handle);
        }

        /**/
        TYPE& Object()
        {
            xassert(_memory);
            return *_memory;
        }

        /**/
        TYPE* operator->() const
        {
            xassert(_memory);
            return _memory;
        }
    };
}

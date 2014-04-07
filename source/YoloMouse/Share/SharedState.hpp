#pragma once
#include <Core/Container/Array.hpp>
#include <Core/Support/Singleton.hpp>
#include <Core/Windows/SharedMemory.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    /**/
    class SharedState:
        public Singleton<SharedState>
    {
    private:
        /**/
        typedef FlatArray<HCURSOR, SHAREDSTATE_CURSOR_LIMIT> SharedTable;

        /**/
        struct Root
        {
            Char        path[STRING_PATH_SIZE];
            SharedTable cursors;
        };

    private:
        Bool                _host;
        SharedMemory<Root>  _memory;

    public:
        /**/
        SharedState();

        /**/
        Bool Open( Bool host );
        void Close();

        /**/
        const Char* GetPath() const;
        HCURSOR     GetCursor( Index cursor_index );

        /**/
        void SetPath( const Char* path );

        /**/
        Index FindCursor( HCURSOR hcursor );

    private:
        /**/
        void _LoadCursors();
        void _UnloadCursors();
    };
}

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
        typedef FlatArray<HCURSOR, SHARED_CURSOR_LIMIT>     CursorArray;
        typedef FlatArray<CursorArray, CURSOR_SIZE_COUNT>   CursorTable;

        /**/
        struct Root
        {
            Char        path[STRING_PATH_SIZE];
            CursorTable cursors;
            CursorSize  size;
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
        CursorSize  GetCursorSize() const;

        /**/
        void SetPath( const Char* path );
        void SetCursorSize( CursorSize size );

        /**/
        Index FindCursor( HCURSOR hcursor );

    private:
        /**/
        void _LoadCursors();
        void _UnloadCursors();
    };
}

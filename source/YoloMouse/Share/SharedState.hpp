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
        // array of HCURSOR handles casted to a 64bit value for 32/64 bit coexistence
        typedef FlatArray<Byte8, SHARED_CURSOR_LIMIT>       CursorArray;
        typedef FlatArray<CursorArray, CURSOR_SIZE_COUNT>   CursorTable;

        /**/
        struct Root
        {
            CursorTable cursors;
            CursorSize  size;
        };

        struct Client
        {
            CursorTable cursors;
        };

    public:
        /**/
        SharedState();

        /**/
        Bool Open( Bool host );
        void Close();

        /**/
        HCURSOR      GetCursor( Index cursor_index );
        CursorSize   GetCursorSize() const;

        /**/
        void SetCursorSize( CursorSize size );

        /**/
        Index FindCursor( HCURSOR hcursor );

    private:
        /**/
        HCURSOR _LoadCursor( Index cursor_index, const WCHAR* base_path );
        void    _LoadCursors();

        /**/
        void _CacheCursors();

        /**/
        void _FreeCursors();

        // fields
        Bool                _host;
        Client              _client;
        SharedMemory<Root>  _shared;

    };
}

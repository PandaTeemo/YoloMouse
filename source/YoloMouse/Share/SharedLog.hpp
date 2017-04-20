#pragma once
#include <Core/Container/String.hpp>
#include <YoloMouse/Share/constants.hpp>

namespace YoloMouse
{
    /**/
    class SharedLog
    {
    public:
        static const ULong SIZE = LOG_MEMORY_LIMIT;

        /**/
        SharedLog();

        /**/
        void Intialize();

        /**/
        Bool IsEmpty() const;

        /**/
        Bool Write( const Char* format, ... );

        /**/
        template<class STRING>
        Bool Read( Index& offset, STRING& message )
        {
            // require ready
            if( !_ready )
                return false;

            // if beginning required
            if( offset == INVALID_INDEX )
                offset = _begin;

            // fail if end
            if( _memory[offset] == 0 )
                return false;

            // empty string
            message.Empty();

            // read message
            for( ; _memory[offset] && _memory[offset] != '\n'; offset = (offset + 1) % SIZE )
                message.Add(_memory[offset]);

            // skip \n
            if( _memory[offset] == '\n' )
                offset = (offset + 1) % SIZE;

            return true;
        }

    private:
        // fields
        Bool    _ready;
        Index   _offset;
        Index   _begin;
        Char    _memory[SIZE];
    };
}

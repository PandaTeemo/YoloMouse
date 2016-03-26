#include <YoloMouse/Share/SharedLog.hpp>
#include <Core/Support/Tools.hpp>
#include <stdio.h>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    SharedLog::SharedLog():
        _ready(false)
    {
    }

    //-------------------------------------------------------------------------
    Bool SharedLog::IsEmpty() const
    {
        return _memory[_begin] == 0;
    }

    //-------------------------------------------------------------------------
    void SharedLog::Intialize()
    {
        _offset = 0;
        _begin = 0;
        Tools::MemZero(_memory, SIZE);
        _ready = true;
    }

    //-------------------------------------------------------------------------
    Bool SharedLog::Write( const Char* format, ... )
    {
        va_list args;
        Char    message[STRING_MAX_SIZE];
        Char*   end_message = message + sizeof(message);
        Char*   pmessage = message;
        Bool    new_begin = false;

        // require ready
        if( !_ready )
            return false;

        // init message
        *pmessage = 0;

        // get current window name
        HWND hwnd = GetForegroundWindow();
        if( hwnd )
        {
            Long count = GetWindowTextA(hwnd, message, APP_NAME_LIMIT);
            if( count > 0 )
            {
                pmessage += count;
                pmessage += sprintf_s(pmessage, end_message - pmessage, "> ");
            }
        }

        // build message
        va_start(args, format);
        int count = vsprintf_s(pmessage, end_message - pmessage, format, args);
        va_end(args);
        if( count <= 0 )
            return false;

        // prepend \n
        if( _offset != _begin )
        {
            _memory[_offset] = '\n';
            _offset = (_offset + 1) % SIZE;
        }

        // for each character in message
        for( Char* c = message;; ++c )
        {
            // add character to log memory
            _memory[_offset] = *c;

            // end on zero
            if( *c == 0 )
                break;

            // increment offset
            _offset = (_offset + 1) % SIZE;

            // check if new begin needed
            if( _offset == _begin )
                new_begin = true;
        }

        // create new begin
        if( new_begin )
        {
            // skip until next
            for( ; _memory[_begin] != '\n'; _begin = (_begin + 1) % SIZE );
            for( ; _memory[_begin] == 0 || _memory[_begin] == '\n'; _begin = (_begin + 1) % SIZE );
        }

        return true;
    }
}

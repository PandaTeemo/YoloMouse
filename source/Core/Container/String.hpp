#pragma once
#include <Core/Constants.hpp>
#include <Core/Container/Array.hpp>
#include <Core/Support/Tools.hpp>

namespace Core
{
    /**/
    template<class ARRAY>
    class BaseString:
        public ARRAY
    {
    public:
        /**/
        BaseString()
        {
        }
        BaseString( const Char* chars ):
            ARRAY(const_cast<Char*>(chars), Tools::CStrLen(chars))
        {
        }
        BaseString( const Char* chars, ULong count ):
            ARRAY(const_cast<Char*>(chars), count)
        {
        }

        /**/
        Char* GetZ()
        {
            if( _memory[_count] != 0 )
            {
                xassert(_count < _limit);
                _memory[_count] = 0;
            }

            return _memory;
        }

        /**/
        Bool ReadToken( BaseString& token, Char delimiter ) const
        {
            Iterator begin = token.IsEmpty() ? Begin() : token.End();

            // skip delimiter
            for( ; begin != End() && *begin == delimiter; ++begin );
            Iterator end = begin;

            // read token
            for( ; end != End() && *end != delimiter; ++end );

            // get count
            ULong count = (ULong)(end - begin);

            // done if empty
            if( count == 0 )
                return false;

            // create token
            token.SetMemory(begin);
            token.SetCount(count);

            return true;
        }

        /**/
        BaseString& ToUpper()
        {
            for( Iterator c = Begin(); c != End(); c++ )
                if(*c >= 'a' && *c <= 'z')
                    *c += 'A'-'a';

            return *this;
        }

        /**/
        Index ToHash() const
        {
            Index hash = 0;

            for( Iterator c = Begin(); c != End(); c++ )
                hash = *c + (hash << 6) + (hash << 16) - hash;

            return hash;
        }

        /**/
        template<class COLLECTION>
        void CopyZ( const COLLECTION& other )
        {
            ULong _count = other.GetCount();
            xassert(_count < _limit);
            Copy(other.GetMemory(), _count);
            _memory[_count] = 0;
        }

        void CopyZ( const Char* chars )
        {
            ULong _count = Tools::CStrLen(chars);
            xassert(_count < _limit);
            Copy(chars, _count);
            _memory[_count] = 0;
        }
    };

    /**/
    typedef BaseString<BaseArray<Char, PointerArrayTraits<Char>>> String;

    /**/
    template<class ARRAY>
    class FixedString:
        public BaseString<ARRAY>
    {
    public:
        /**/
        FixedString()
        {
        }
        FixedString( const String& string )
        {
            Copy(string);
            xassert(_count < _limit);
            _memory[_count] = 0;
        }
        FixedString( const Char* chars )
        {
            CopyZ(chars);
        }

        /**/
        operator const String() const
        {
            return String(_memory, _count);
        }
    };

    /**/
    typedef FixedString<FixedArray<Char, STRING_SHORT_SIZE>>    ShortString;
    typedef FixedString<FixedArray<Char, STRING_MEDIUM_SIZE>>   MediumString;
    typedef FixedString<FixedArray<Char, STRING_MAX_SIZE>>      MaxString;
}

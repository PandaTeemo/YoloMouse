#pragma once
#include <Core/Root.hpp>

namespace Core
{
    class Tools
    {
    public:
        /**/
        template<typename TYPE>
        static void Swap( TYPE& a, TYPE& b )
        {
            TYPE t = a;
            a = b;
            b = t;
        }

        /**/
        template<typename TYPE>
        static TYPE Min( TYPE a, TYPE b )
        {
            return a < b ? a : b;
        }
        template<typename TYPE>
        static TYPE Max( TYPE a, TYPE b )
        {
            return a > b ? a : b;
        }

        /**/
        template<typename TYPE>
        static TYPE Clamp( TYPE value, TYPE min, TYPE max )
        {
            return ((value > max) ? (max) : ((value < min) ? (min) : (value)));
        }

        /**/
        template<typename TYPE>
        static void MemCopy( TYPE* to, const TYPE* from, ULong count=1 )
        {
            _MemCpy(to, from, count * sizeof(TYPE));
        }

        template<typename TYPE>
        static void MemCopy( TYPE& to, const TYPE& from )
        {
            _MemCpy(&to, &from, sizeof(TYPE));
        }

        /**/
        template<typename TYPE>
        static inline Long MemCompare( const TYPE* a, const TYPE* b, ULong count )
        {
            return _MemCmp(a, b, count * sizeof(TYPE));
        }

        template<typename TYPE>
        static inline Long MemCompare( const TYPE& a, const TYPE& b )
        {
            return _MemCmp(&a, &b, sizeof(TYPE));
        }

        /**/
        template<typename TYPE>
        static inline void MemZero( TYPE& object )
        {
            _MemSet(&object, 0, sizeof(TYPE));
        }
        template<typename TYPE>
        static inline void MemZero( TYPE* memory, ULong count )
        {
            _MemSet(memory, 0, count * sizeof(TYPE));
        }

        /**/
        static ULong CStrLen( const Char* string );
        static CHAR* WToCString( const WCHAR* wstring );

        /**/
        static Bool StripFileName( WCHAR* path );
        static Bool DoesFileExist( const WCHAR* path );

        /**/
        static Hash Fnv164Hash( const void* memory, ULong count );

    private:
        /**/
        static void _MemSet( void* memory, Byte value, ULong size );
        static void _MemCpy( void* to, const void* from, ULong size );
        static Long _MemCmp( const void* a, const void* b, ULong size );
    };
}

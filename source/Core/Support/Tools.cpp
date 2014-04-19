#include <Core/Support/Tools.hpp>
#include <stdio.h>
#include <string.h>
#include <memory.h>

namespace Core
{
    // public
    //-------------------------------------------------------------------------
    Bool Tools::StripFileName( WCHAR* path )
    {
        WCHAR* end = path + wcslen(path);
        for( ; *end != '\\' && end != path; end-- );
        *end = 0;

        return true;
    }

    Bool Tools::DoesFileExist( const WCHAR* path )
    {
        FILE* file;

        if(_wfopen_s(&file, path, L"rt") == 0)
        {
            fclose(file);
            return true;
        }

        return false;
    }

    // private
    //-------------------------------------------------------------------------
    void Tools::_MemSet( void* memory, Byte value, ULong size )
    {
        ::memset(memory, value, size);
    }

    void Tools::_MemCpy( void* to, const void* from, ULong size )
    {
        ::memcpy(to, from, size);
    }

    Long Tools::_MemCmp( const void* a, const void* b, ULong size )
    {
        return ::memcmp(a, b, size);
    }

    ULong Tools::CStrLen( const Char* string )
    {
        assert(string);
        return strlen(string);
    }

    Hash Tools::Fnv164Hash( const void* memory, ULong count )
    {
        const Byte* bmemory = reinterpret_cast<const Byte*>(memory);
        UHuge       hash = 0xCBF29CE484222325;
        
        for( Index i = 0; i < count; i++ )
        {
            Byte byte = bmemory[i];

            hash = hash ^ byte;
            hash = hash * 0x100000001B3;
            hash = hash & 0xFFFFFFFFFFFFFFFF;
        }

        return hash;
    }
}

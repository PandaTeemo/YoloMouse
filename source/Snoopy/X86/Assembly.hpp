#pragma once
#include <Core/Container/Array.hpp>
#include <Snoopy/X86/Enums.hpp>
#include <Snoopy/X86/Types.hpp>

namespace Snoopy { namespace x86
{
    /**/
    class Assembly
    {
    private:
        Byte*           _source_address;
        ByteBufferArray _code;

    public:
        /**/
        Assembly();

        /**/
        Byte*            GetSourceAddress() const;
        ByteBufferArray& GetCode();
        ULong            GetCodeLength() const;
        const Byte*      GetCodeMemory() const;

        /**/
        void SetSourceAddress( void* address );
        void SetCodeBuffer( ByteArray buffer );

        /**/
        void Empty();

        /**/
        void OpNop      ();
        void OpPushReg  ( RegisterId register_id );
        void OpPopReg   ( RegisterId register_id );
        void OpMovRegReg( RegisterId register_to, RegisterId register_from );
        void OpMovRegImm( RegisterId register_id, Native imm );
        void OpCallReg  ( RegisterId register_id );
        void OpCallRel  ( void* address );
        void OpJmpReg   ( RegisterId register_id );
        void OpJmpRel   ( void* address );

        /**/
        Bool Relocate( const Assembly& source );

        /**/
        Bool Read( Operation& op );

    private:
        /**/
        void _ArgRelativeAddress32( void* address );
    };
}}

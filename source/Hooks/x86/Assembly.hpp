#pragma once
#include <Core/Container/Array.hpp>
#include <Hooks/x86/Types.hpp>

namespace Hooks { namespace x86
{
    /**/
    class Assembly
    {
    private:
        Byte*           _address;
        ByteBufferArray _code;

    public:
        /**/
        Assembly();

        /**/
        Byte*       GetAddress() const;
        ULong       GetCodeLength() const;
        const Byte* GetCodeMemory() const;

        /**/
        void SetAddress( void* address );
        void SetCodeBuffer( ByteArray buffer );

        /**/
        void Empty();

        /**/
        void OpNop();
        void OpCall( void* address );
        void OpJump( void* address );
        void OpPushAd();
        void OpPopAd();

        /**/
        Bool Relocate( const Assembly& source );

        /**/
        Bool Read( Operation& op );

    private:
        /**/
        void _ArgBranch( void* address );
    };
}}

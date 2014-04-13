#pragma once
#include <Core/Container/Array.hpp>
#include <Hooks/x86/Assembly.hpp>
#include <Hooks/x86/Types.hpp>

namespace Hooks
{
    /**/
    class Hook
    {
    public:
        enum Placement
        {
            BEFORE,
            AFTER,
            OVER,
        };

    private:
        static const ULong CODE_BUFFER_SIZE =   32;
        static const ULong TARGET_MIN =         5;

    private:
        typedef FixedArray<Byte, CODE_BUFFER_SIZE> CodeBuffer;

    private:
        x86::Assembly       _target;
        x86::Assembly       _target_backup;
        x86::Assembly       _trampoline;

        Bool                _enabled;
        CodeBuffer          _target_buffer;

        void*               _target_address;
        x86::HookFunction   _hook_function;
        Placement           _placement;

    public:
        /**/
        Hook( void* target_address, x86::HookFunction hook_function, Placement placement=BEFORE );
        ~Hook();

        /**/
        Bool Init();

        /**/
        Bool Enable();
        void Disable();

    private:
        /**/
        Bool _InitTarget( void* target_address );
        Bool _InitTargetBackup();
        Bool _InitTrampoline( void* hook_address );

        /**/
        Byte* _AllocTrampolineAddress();
    };
}

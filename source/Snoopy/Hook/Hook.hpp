#pragma once
#include <Core/Container/Array.hpp>
#include <Snoopy/X86/Assembly.hpp>
#include <Snoopy/X86/Types.hpp>

namespace Snoopy
{
    /**/
    class Hook
    {
    public:
        enum Placement
        {
            // our hook runs before the target allowing us to modify its input
            BEFORE,
            // our hook runs after the target allowing us to read the results
            AFTER,
        };

    private:
        // largest size of a code segment (target or trampoline)
        static const ULong CODE_BUFFER_SIZE =   64;

        // minimum length of target code (a jmp instruction to trampoline)
        static const ULong TARGET_MIN_SIZE =    5;

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
        Bool IsEnabled() const;

        /**/
        Bool Init();

        /**/
        Bool Enable();
        Bool Disable();

    private:
        /**/
        Bool _InitTarget( void* target_address );
        Bool _InitTargetBackup();
        Bool _InitTrampoline( void* hook_address );

        /**/
        Byte* _AllocTrampolineAddress();
    };
}

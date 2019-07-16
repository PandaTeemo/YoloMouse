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
        /**/
        Hook();
        ~Hook();

        /**/
        Bool IsInitialized() const;
        Bool IsEnabled() const;

        /**/
        Bool Initialize( void* target_address, x86::HookFunction hook_function );
        void Shutdown();

        /**/
        Bool Enable();
        Bool Disable();

    private:
        // constants
        // largest size of a code segment (target or trampoline)
        static const ULong CODE_BUFFER_SIZE =   64;
        // minimum length of target code (a jmp instruction to trampoline)
        static const ULong TARGET_MIN_SIZE =    5;

        // types
        typedef DynamicFlatArray<Byte, CODE_BUFFER_SIZE> CodeBuffer;

        /**/
        Bool _InitializeTarget( void* target_address );
        Bool _InitializeTargetBackup();
        Bool _InitializeTrampoline( void* hook_address );

        /**/
        Byte* _AllocTrampolineAddress();

        // fields: parameters
        void*               _target_address;
        x86::HookFunction   _hook_function;

        // fields: state
        Bool                _enabled;
        Bool                _initialized;
        x86::Assembly       _target;
        x86::Assembly       _target_backup;
        x86::Assembly       _trampoline;
        CodeBuffer          _target_buffer;
    };
}

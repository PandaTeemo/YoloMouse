#pragma once
#include <Snoopy/Root.hpp>
#ifdef _WIN64
    #include <Snoopy/X86/hde/hde64.hpp>
#else
    #include <Snoopy/X86/hde/hde32.hpp>
#endif

namespace Snoopy { namespace x86
{
    /**/
    #define HOOK_CALL __cdecl

    /**/
    #ifdef _WIN64
        typedef hde64s Operation;
        #define hde_disasm hde64_disasm 
    #else
        typedef hde32s Operation;
        #define hde_disasm hde32_disasm 
    #endif

    /**/
    typedef void (HOOK_CALL *HookFunction)( Native* arguments );
}}

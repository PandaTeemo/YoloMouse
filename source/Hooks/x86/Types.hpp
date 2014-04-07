#pragma once
#include <Hooks/Root.hpp>
#include <Hooks/x86/hde/hde32.hpp>

namespace Hooks { namespace x86
{
    /**/
    #define HOOK_CALL __cdecl

    /**/
    struct Registers
    {
        Byte4   edi;
        Byte4   esi;
        Byte4   ebp;
        Byte4   esp;
        Byte4   ebx;
        Byte4   edx;
        Byte4   ecx;
        Byte4   eax;
    };

    /**/
    typedef hde32s Operation;

    /**/
    typedef void (HOOK_CALL *HookFunction)( Registers );
}}

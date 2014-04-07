#pragma once
#include <Hooks/Root.hpp>

namespace Hooks { namespace x86
{
    // x86 op codes
    //-------------------------------------------------------------------------
    static const Byte OP_JMP =              0xe9;
    static const Byte OP_CALL =             0xe8;
    static const Byte OP_NOP =              0x90;
    static const Byte OP_PUSHAD =           0x60;
    static const Byte OP_POPAD =            0x61;
}}

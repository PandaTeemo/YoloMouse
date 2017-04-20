#pragma once
#include <Snoopy/Root.hpp>

namespace Snoopy { namespace x86
{
    // x86 register ids
    //-------------------------------------------------------------------------
    enum RegisterId
    {
        REGISTER_AX = 0,
        REGISTER_CX = 1,
        REGISTER_DX = 2,
        REGISTER_BX = 3,
        REGISTER_SP = 4,
        REGISTER_BP = 5,
        REGISTER_SI = 6,
        REGISTER_DI = 7,

        REGISTER_R8  = 100,
        REGISTER_R9  = 101,
        REGISTER_R10 = 102,
        REGISTER_R11 = 103,
        REGISTER_R12 = 104,
        REGISTER_R13 = 105,
        REGISTER_R14 = 106,
        REGISTER_R15 = 107,

        REGISTER_BASIC_COUNT = 8,
        REGISTER_EXT_COUNT =   8,

        REGISTER_EXT_DIV =     100,
    };

    // x86 op codes
    //-------------------------------------------------------------------------
    enum OpCodeId
    {
        OP_NOP =                0x90,

        OP_CALL_REG =           0xd0,
        OP_CALL_REL =           0xe8,
        OP_JMP_REG =            0xe0,
        OP_JMP_REL =            0xe9,
        OP_PUSH_REG =           0x50,
        OP_POP_REG =            0x58,
        OP_MOV_REG_IMM =        0xb8,
        OP_MOV_REG_REG =        0x8b,

        OP_ARG_MOV_REG_REG =    0xc0,

        OP_PREFIX_MOV_64 =      0x48,
        OP_PREFIX_CALL_REG =    0xFF,
        OP_PREFIX_JMP_REG =     0xFF,
        OP_PREFIX_PUSH_64 =     0x41,
        OP_PREFIX_POP_64 =      0x41,
    };
}}

#include <Snoopy/X86/Assembly.hpp>
#include <Snoopy/X86/Enums.hpp>

namespace Snoopy { namespace x86
{
    // public
    //-------------------------------------------------------------------------
    Assembly::Assembly():
        _source_address(0)
    {
    #if CPU_64
        xassert(sizeof(Native) == 8);
    #else
        xassert(sizeof(Native) == 4);
    #endif
    }

    //-------------------------------------------------------------------------
    Byte* Assembly::GetSourceAddress() const
    {
        return _source_address;
    }

    ByteBufferArray& Assembly::GetCode()
    {
        return _code;
    }

    ULong Assembly::GetCodeLength() const
    {
        return _code.GetCount();
    }

    const Byte* Assembly::GetCodeMemory() const
    {
        return _code.GetMemory();
    }

    //-------------------------------------------------------------------------
    void Assembly::SetSourceAddress( void* address )
    {
        _source_address = static_cast<Byte*>(address);
    }

    void Assembly::SetCodeBuffer( ByteArray buffer )
    {
        _code = ByteBufferArray(buffer);
    }

    //-------------------------------------------------------------------------
    void Assembly::Empty()
    {
        _code.Empty();
    }

    //-------------------------------------------------------------------------
    void Assembly::OpNop()
    {
        _code.Add(OP_NOP);
    }

    void Assembly::OpPushReg( RegisterId register_id )
    {
        // if extended register
        if( register_id / REGISTER_EXT_DIV )
        {
            _code.Add(OP_PREFIX_PUSH_64);
            register_id = static_cast<RegisterId>(register_id % REGISTER_EXT_DIV);
        }

        _code.Add(OP_PUSH_REG + register_id);
    }

    void Assembly::OpPopReg( RegisterId register_id )
    {
        // if extended register
        if( register_id / REGISTER_EXT_DIV )
        {
            _code.Add(OP_PREFIX_POP_64);
            register_id = static_cast<RegisterId>(register_id % REGISTER_EXT_DIV);
        }

        _code.Add(OP_POP_REG + register_id);
    }

    void Assembly::OpMovRegReg( RegisterId register_to, RegisterId register_from )
    {
    #if CPU_64
        _code.Add(OP_PREFIX_MOV_64);
    #endif
        _code.Add(OP_MOV_REG_REG);
        _code.Add(OP_ARG_MOV_REG_REG + (register_to * REGISTER_BASIC_COUNT) + register_from);
    }

    void Assembly::OpMovRegImm( RegisterId register_id, Native imm )
    {
    #if CPU_64
        _code.Add(OP_PREFIX_MOV_64);
    #endif
        _code.Add(OP_MOV_REG_IMM + register_id);
        _code.AddRaw(imm);
    }

    void Assembly::OpCallReg( RegisterId register_id )
    {
        _code.Add(OP_PREFIX_CALL_REG);
        _code.Add(OP_CALL_REG + register_id);
    }

    void Assembly::OpCallRel( void* address )
    {
        _code.Add(OP_CALL_REL);
        _ArgRelativeAddress32(address);
    }

    void Assembly::OpJmpReg( RegisterId register_id )
    {
        _code.Add(OP_PREFIX_JMP_REG);
        _code.Add(OP_JMP_REG + register_id);
    }

    void Assembly::OpJmpRel( void* address )
    {
        _code.Add(OP_JMP_REL);
        _ArgRelativeAddress32(address);
    }

    //-------------------------------------------------------------------------
    Bool Assembly::Relocate( const Assembly& source )
    {
        ULong       soffset = 0;
        Operation   op;

        // for each source operation
        while( soffset < source.GetCodeLength() )
        {
            // current source address aligned to an operation
            const Byte* sbuffer = source.GetCodeMemory() + soffset;
            const Byte* saddress = source.GetSourceAddress() + soffset;
            
            // disassemble operation at address
            hde_disasm(sbuffer, &op);

            // fail on error
            if( op.flags & F_ERROR )
                return false;

            // WARNING: this is major hackjobbery to identify relative vs
            // absolute address operands. a legit version would be much longer

            // if displacement and address (mod=00 rm=101)
            if( ((op.flags & (F_DISP8|F_DISP16|F_DISP32)) && ((op.modrm_mod == 0 && op.modrm_rm == 0x05 && (op.flags & F_RELATIVE)) || (CPU_64 && (op.flags & F_DISP32)))) ||
            // or relative address operation
                ((op.flags & (F_IMM8|F_IMM16|F_IMM32|F_IMM64)) && (op.flags & F_RELATIVE)) )
            {
                // determine operation address argument length
                ULong branch_length =
                    (op.flags & F_IMM64) ? 8 :
                    (op.flags & F_IMM32|F_DISP32) ? 4 :
                    (op.flags & F_IMM16|F_DISP16) ? 2 :
                    1;
                ULong base_length = op.len - branch_length;

                // copy instruction base
                _code.Add(sbuffer, base_length);

                // to absolute
                Huge absolute = (Huge)saddress + op.len +
                    ((op.flags & (F_DISP8|F_DISP16|F_DISP32)) ? op.disp.disp32 : op.imm.imm64);

                // to new relative
                Huge relative = absolute - (Huge)(_source_address + _code.GetCount()) - branch_length;

                // add new relative branch address
                _code.Add((Byte*)&relative, branch_length);
            }
            // else copy as is
            else
                _code.Add(sbuffer, op.len);

            // incremenet
            soffset += op.len;
        }

        // successful if still aligned
        return soffset == source.GetCodeLength();
    }

    //-------------------------------------------------------------------------
    Bool Assembly::Read( Operation& op )
    {
        // get operation address
        Byte* op_address = _source_address + _code.GetCount();

        // disassemble operation at address
        hde_disasm(op_address, &op);

        // fail on error
        if( op.flags & F_ERROR )
            return false;

        // add to code buffer
        _code.Add(op_address, op.len);

        return true;
    }

    //-------------------------------------------------------------------------
    void Assembly::_ArgRelativeAddress32( void* address )
    {
        // cant go too far
        eggs(::abs((Byte*)address - _source_address) < 0x7ffffff0);

        // calculate relative 32bit address
        Long rel32 = (Long)address - (Long)(_source_address + _code.GetCount()) - sizeof(Long);

        // write
        _code.AddRaw(rel32);
    }
}}

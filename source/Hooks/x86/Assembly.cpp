#include <Hooks/x86/Assembly.hpp>
#include <Hooks/x86/Enums.hpp>

namespace Hooks { namespace x86
{
    // public
    //-------------------------------------------------------------------------
    Assembly::Assembly():
        _address(0)
    {
    }

    //-------------------------------------------------------------------------
    Byte* Assembly::GetAddress() const
    {
        return _address;
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
    void Assembly::SetAddress( void* address )
    {
        _address = static_cast<Byte*>(address);
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

    void Assembly::OpCall( void* address )
    {
        _code.Add(OP_CALL);
        _ArgBranch(address);
    }

    void Assembly::OpJump( void* address )
    {
        _code.Add(OP_JMP);
        _ArgBranch(address);
    }

    void Assembly::OpPushAd()
    {
        _code.Add(OP_PUSHAD);
    }

    void Assembly::OpPopAd()
    {
        _code.Add(OP_POPAD);
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
            const Byte* saddress = source.GetAddress() + soffset;

            // disassemble operation at address
            hde32_disasm(sbuffer, &op);

            // fail on error
            if( op.flags & F_ERROR )
                return false;

            // if relative address operation
            if( (op.flags & (F_IMM8|F_IMM16|F_IMM32)) && (op.flags & F_RELATIVE) )
            {
                ULong branch_length = (op.flags & F_IMM32) ? 4 : ((op.flags & F_IMM16) ? 2 : 1);
                ULong base_length = op.len - branch_length;

                // copy instruction base
                _code.Add(sbuffer, base_length);

                // to absolute
                Huge absolute = (Huge)saddress + op.len + op.imm.imm32;

                // to new relative
                Huge relative = absolute - (Huge)(_address + _code.GetCount()) - branch_length;

                // add upate branch address
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
        Byte* op_address = _address + _code.GetCount();

        // disassemble operation at address
        hde32_disasm(op_address, &op);

        // fail on error
        if( op.flags & F_ERROR )
            return false;

        // add to code buffer
        _code.Add(op_address, op.len);

        return true;
    }

    //-------------------------------------------------------------------------
    void Assembly::_ArgBranch( void* address )
    {
        _code.AddRaw((Long)address - (Long)(_address + _code.GetCount()) - sizeof(Long));
    }
}}

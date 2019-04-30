#pragma once
#include <Core/Root.hpp>
#include <Core/Support/Tools.hpp>

namespace Core
{
    /**/
    template<typename TYPE, ULong LIMIT>
    class BaseFlatArrayTraits
    {
    protected:
        static const ULong _limit = LIMIT;
        TYPE _memory[LIMIT];
    };

    /**/
    template<typename TYPE, ULong LIMIT>
    class FlatArrayTraits:
        public BaseFlatArrayTraits<TYPE, LIMIT>
    {
    protected:
        static const ULong _count = LIMIT;
    };

    /**/
    template<typename TYPE, ULong LIMIT>
    class CountFlatArrayTraits:
        public BaseFlatArrayTraits<TYPE, LIMIT>
    {
    protected:
        static const ULong  _limit = LIMIT;
        ULong _count;

    public:
        /**/
        CountFlatArrayTraits( ULong count=0 ):
            _count(count)
        {
        }
    };

    /**/
    template<typename TYPE>
    class BasePointerArrayTraits
    {
    protected:
        TYPE* _memory;

        /**/
        BasePointerArrayTraits( TYPE* memory=NULL ):
            _memory(memory)
        {
        }

    public:
        /**/
        void SetMemory( TYPE* memory )
        {
            _memory = memory;
        }
    };

    /**/
    template<typename TYPE>
    class PointerArrayTraits:
        public BasePointerArrayTraits<TYPE>
    {
    protected:
        union
        {
            ULong _count;
            ULong _limit;
        };

        /**/
        PointerArrayTraits( TYPE* memory=NULL, ULong count=0 ):
            BasePointerArrayTraits(memory),
            _count(count)
        {
        }
    };

    /**/
    template<typename TYPE>
    class LimitPointerArrayTraits:
        public BasePointerArrayTraits<TYPE>
    {
    protected:
        ULong _count;
        ULong _limit;

        /**/
        LimitPointerArrayTraits( TYPE* memory=NULL, ULong count=0, ULong limit=0 ):
            BasePointerArrayTraits(memory),
            _count(count),
            _limit(limit)
        {
        }
    };

    /**/
    template<typename TYPE, typename TRAITS>
    class BaseArray:
        public TRAITS
    {
    public:
        typedef TYPE* Iterator;

    public:
        /**/
        BaseArray()
        {
        }
        BaseArray( ULong count ):
            TRAITS(count)
        {
        }
        BaseArray( TYPE* memory, ULong count ):
            TRAITS(memory, count)
        {
        }
        BaseArray( TYPE* memory, ULong count, ULong limit ):
            TRAITS(memory, count, limit)
        {
        }

        /**/
        void Zero()
        {
            Tools::MemZero(_memory, _count);
        }

        /**/
        ULong GetCount() const
        {
            return _count;
        }
        ULong GetSize() const
        {
            return _count * sizeof(TYPE);
        }

        ULong GetLimit() const
        {
            return _limit;
        }

        /**/
        const TYPE* GetMemory() const
        {
            return _memory;
        }
        TYPE* EditMemory()
        {
            return _memory;
        }

        /**/
        void SetCount( ULong count )
        {
            _count = count;
            xassert(count <= _limit);
        }

        void SetLimit( ULong limit )
        {
            _limit = limit;
        }

        void PushCount( ULong count )
        {
            _count += count;
            xassert(_count <= _limit);
        }

        /**/
        Iterator Add()
        {
            xassert(_count < _limit);
            return _memory + _count++;
        }
        void Add( const TYPE& value )
        {
            xassert(_count < _limit);
            _memory[_count++] = value;
        }
        void Add( const TYPE* values, ULong count )
        {
            xassert(sizeof(TYPE) == 1);
            xassert((_count + count) <= _limit);

            Tools::MemCopy(
                reinterpret_cast<Byte*>(_memory + _count),
                reinterpret_cast<const Byte*>(values),
                count);

            _count += count;
        }
        template<typename OBJECT>
        void AddRaw( const OBJECT& object )
        {
            xassert(sizeof(TYPE) == 1);
            xassert((_count + sizeof(OBJECT) - 1) < _limit);

            Tools::MemCopy(
                reinterpret_cast<Byte*>(_memory + _count),
                reinterpret_cast<const Byte*>(&object),
                sizeof(OBJECT));

            _count += sizeof(OBJECT);
        }

        /**/
        void PopSwap( Iterator iterator )
        {
            xassert(IndexOf(iterator) < _count);
            *iterator = _memory[_count - 1];
            _count--;
        }

        /**/
        void Pop()
        {
            xassert(_count > 0);
            _count--;
        }

        /**/
        void Empty()
        {
            _count = 0;
        }
        Bool IsEmpty() const
        {
            return _count == 0;
        }
        Bool IsFull() const
        {
            return _count == _limit;
        }

        /**/
        TYPE& operator[]( Index index )
        {
            xassert(index < _count);
            return _memory[index];
        }
        const TYPE& operator[]( Index index ) const
        {
            xassert(index < _count);
            return _memory[index];
        }

        /**/
        const Iterator begin() const
        {
            return const_cast<Iterator>(_memory);
        }
        const Iterator end() const
        {
            return const_cast<Iterator>(_memory + _count);
        }

        Iterator begin()
        {
            return _memory;
        }
        Iterator end()
        {
            return _memory + _count;
        }

        /**/
        const Iterator ReverseBegin() const
        {
            return _memory + _count - 1;
        }
        const Iterator ReverseEnd() const
        {
            return _memory - 1;
        }

        Iterator ReverseBegin()
        {
            return _memory + _count - 1;
        }
        Iterator ReverseEnd()
        {
            return _memory - 1;
        }

        /**/
        Index IndexOf( Iterator iterator )
        {
            return static_cast<Index>(iterator - Begin());
        }

        /**/
        template<typename OBJECT>
        Iterator Find( const OBJECT& o ) const
        {
            for( Iterator i = begin(); i != end(); ++i )
                if(*i == o)
                    return i;

            return NULL;
        }

        /**/
        template<class COLLECTION>
        void Copy( const COLLECTION& other )
        {
            xassert(other.GetCount() <= _limit);

            TYPE* i = _memory;
            for( COLLECTION::Iterator ci = other.Begin(); ci != other.End(); ci++, i++ )
                *i = *ci;

            _count = other.GetCount();
        }

        /**/
        void Copy( const TYPE* memory, ULong count )
        {
            xassert(count <= _limit);

            for( Index i = 0; i < count; i++ )
                _memory[i] = memory[i];

            _count = count;
        }

        /**/
        template<class COLLECTION>
        Bool Compare( const COLLECTION& other ) const
        {
            if(other.GetCount() != _count)
                return false;
            else
            {
                Iterator i = begin(), j = other.begin();
                for(; i != end() && *i == *j; i++, j++ );
                return i == end();
            }
        }

        /**/
        template<class COLLECTION>
        Bool operator == ( const COLLECTION& other ) const
        {
            return Compare(other);
        }
    };

    /**/
    template<typename TYPE>
    class Array:
        public BaseArray<TYPE, PointerArrayTraits<TYPE>>
    {
    public:
        /**/
        Array()
        {
        }

        /**/
        Array( TYPE* memory, ULong count ):
            BaseArray(memory, count)
        {
        }

        /**/
        template<typename OTHER> Array( OTHER& other ):
            BaseArray(other.EditMemory(), other.GetCount())
        {
        }
    };

    /**/
    template<typename TYPE, ULong LIMIT>
    class FlatArray:
        public BaseArray<TYPE, FlatArrayTraits<TYPE, LIMIT>>
    {
    };

    /**/
    template<typename TYPE, ULong LIMIT>
    class FixedArray:
        public BaseArray<TYPE, CountFlatArrayTraits<TYPE, LIMIT>>
    {
    public:
        /**/
        FixedArray( ULong count = 0 ):
            BaseArray(count)
        {
        }
    };

    /**/
    template<typename TYPE>
    class BufferArray:
        public BaseArray<TYPE, LimitPointerArrayTraits<TYPE>>
    {
    public:
        /**/
        BufferArray():
            BaseArray(NULL, 0, 0)
        {
        }

        BufferArray( TYPE* buffer, ULong count ):
            BaseArray(buffer, count, count)
        {
        }

        BufferArray( TYPE* buffer, ULong count, ULong limit ):
            BaseArray(buffer, count, limit)
        {
        }

        template<typename OTHER>
        BufferArray( OTHER& other ):
            BaseArray(other.EditMemory(), other.GetCount(), other.GetLimit())
        {
        }
    };

    /**/
    typedef Array<Byte>         ByteArray;
    typedef BufferArray<Byte>   ByteBufferArray;
}

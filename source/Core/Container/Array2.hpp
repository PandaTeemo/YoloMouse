#pragma once
#include <Core/Container/Array.hpp>

namespace Core
{
    // traits
    //-------------------------------------------------------------------------
    /**/
    template<typename TYPE, ULong WIDTH, ULong HEIGHT>
    class BaseFlatArrayTraits2
    {
    protected:
        // fields
        static constexpr ULong _limit = WIDTH * HEIGHT;
        TYPE _data[_limit];
    };

    /**/
    template<typename TYPE, ULong WIDTH, ULong HEIGHT>
    class FlatArrayTraits2:
        public BaseFlatArrayTraits2<TYPE, WIDTH, HEIGHT>
    {
    protected:
        // fields
        static constexpr ULong _width = WIDTH;
        static constexpr ULong _height = HEIGHT;
    };

    /**/
    template<typename TYPE>
    class BasePointerArrayTraits2
    {
    public:
        /**/
        BasePointerArrayTraits2( const TYPE* memory=nullptr ):
            _data(const_cast<TYPE*>(memory))
        {}

    protected:
        TYPE* _data;
    };

    /**/
    template<typename TYPE>
    class CountPointerArrayTraits2:
        public BasePointerArrayTraits2<TYPE>
    {
    public:
        /**/
        CountPointerArrayTraits2( const TYPE* memory=nullptr, ULong width=0, ULong height=0 ):
            BasePointerArrayTraits2(const_cast<TYPE*>(memory)),
            _limit  (width * height),
            _width  (width),
            _height (height)
        {}

    protected:
        // fields
        ULong _limit;
        ULong _width;
        ULong _height;
    };

    /**/
    template<typename TYPE, typename TRAITS>
    class BaseArray2:
        public TRAITS
    {
    public:
        // types
        typedef TYPE        Type;
        typedef TYPE*       Iterator;
        typedef const TYPE* ConstIterator;

        /**/
        BaseArray2()
        {}

        BaseArray2( const TYPE* memory, ULong width, ULong height ):
            TRAITS(memory, width, height)
        {}

        /**/
        TYPE* operator[]( Index y )
        {
            return EditRow( y );
        }

        const TYPE* operator[]( Index y ) const
        {
            return GetRow( y );
        }

        /**/
        Iterator begin() const
        {
            return const_cast<Iterator>(_data);
        }

        Iterator end() const
        {
            return const_cast<Iterator>(_data + _limit);
        }

        /**/
        ULong GetWidth() const
        {
            return _width;
        }

        ULong GetHeight() const
        {
            return _height;
        }

        ULong GetLimit() const
        {
            return _limit;
        }

        const TYPE* GetData() const
        {
            return _data;
        }

        const TYPE* GetRow( Index y ) const
        {
            ASSERT(y < _height);
            return _data + y * _width;
        }

        Array<TYPE> GetArray() const
        {
            return Array<TYPE>( _data, _limit );
        }

        /**/
        TYPE* EditData()
        {
            return _data;
        }

        TYPE* EditRow( Index y )
        {
            ASSERT(y < _height);
            return _data + y * _width;
        }

        /**/
        void Clear( const TYPE& value )
        {
            for( Index i = 0; i < _limit; i++ )
                _data[i] = value;
        }

        /**/
        template<typename ARRAY2>
        void CopyTo( const ARRAY2& from, Index x, Index y )
        {
            ASSERT((from.GetWidth() + x) <= _width);
            ASSERT((from.GetHeight() + y) <= _height);

            for( Index i = 0; i < from.GetWidth(); i++ )
                for( Index j = 0; j < from.GetHeight(); j++ )
                    _data[(i + x) + (j + y) * _width] = from[j][i];
        }
    };

    /*
        memory: pointer
        width:  variable
        height: variable
        limit:  width * height
    */
    template<typename TYPE>
    class Array2:
        public BaseArray2<TYPE, CountPointerArrayTraits2<TYPE>>
    {
    public:
        /**/
        using BaseArray2::BaseArray2;

        Array2()
        {}

        /**/
        template<class ARRAY>
        Array2( const ARRAY& other ):
            BaseArray(other.GetData(), other.GetWidth(), other.GetHeight())
        {}
    };

    /*
        memory: flat
        width:  static
        height: static
        limit:  static
    */
    template<typename TYPE, ULong WIDTH, ULong HEIGHT>
    class FlatArray2:
        public BaseArray2<TYPE, FlatArrayTraits2<TYPE, WIDTH, HEIGHT>>
    {
    public:
        /**/
        using BaseArray2::BaseArray2;

        FlatArray2()
        {}
    };

    // common typedefs
    //-------------------------------------------------------------------------
    typedef Array2<Byte> CharArray2;
    typedef Array2<Byte> ByteArray2;
}

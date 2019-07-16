#pragma once
#include <Core/Type/Tuple.hpp>
#include <Core/Container/List.hpp>

namespace Core
{
    /**/
    template<typename KEY, typename VALUE>
    class Map
    {
    public:
        // types
        struct KeyValue
        {
            /**/
            Bool operator==( const KeyValue& other ) const
            {
                return key == other.key;
            }

            // fields
            KEY     key;
            VALUE   value;
        };

        typedef List<KeyValue>                Collection;
        typedef typename Collection::Iterator Iterator;

        /**/
        Map() = default;

        Map( const Map& other )
        {
            _Copy(other);
        }

        Map( std::initializer_list<KeyValue> ilist )
        {
            // for each initializer list entry
            for( const KeyValue* i = ilist.begin(); i != ilist.end(); ++i )
                _AccessKeyValue( i->key ).value = i->value;
        }

        ~Map()
        {
            _Destruct();
        }

        /**/
        void Empty()
        {
            _list.Empty();
        }

        /**/
        const Iterator begin() const
        {
            return _list.begin();
        }

        const Iterator end() const
        {
            return _list.end();
        }

        /**/
        ULong GetCount() const
        {
            return _list.GetCount();
        }

        template<typename TKEY>
        VALUE* Get( const TKEY& key )
        {
            // find key value pair
            Iterator ikv = _FindKeyValue( key );
            if( ikv == _list.end() )
                return nullptr;

            // return value
            return &ikv->value;
        }

        template<typename TKEY>
        const VALUE* Get( const TKEY& key ) const
        {
            // find key value pair
            Iterator ikv = _FindKeyValue( key );
            if( ikv == _list.end() )
                return nullptr;

            // return value
            return &ikv->value;
        }

        template<typename TKEY, typename TVALUE>
        Bool Get( const TKEY& key, TVALUE& value ) const
        {
            // find key value pair
            Iterator ikv = _FindKeyValue( key );
            if( ikv == _list.end() )
                return false;

            // return value
            value = ikv->value;

            return true;
        }

        template<typename TKEY, typename TVALUE>
        Bool Get( const TKEY& key, TVALUE*& value ) const
        {
            // find key value pair
            Iterator ikv = _FindKeyValue( key );
            if( ikv == _list.end() )
                return false;

            // return value reference
            value = &ikv->value;

            return true;
        }

        /**/
        template<typename TKEY>
        VALUE& Set( const TKEY& key )
        {
            // access key value pair and return value
            return _AccessKeyValue( key ).value;
        }

        template<typename TKEY, typename TVALUE>
        void Set( const TKEY& key, const TVALUE& value )
        {
            // access key value pair and update value
            _AccessKeyValue( key ).value = value;
        }

        /**/
        template<typename TKEY>
        Bool Remove( const TKEY& key )
        {
            // find key value pair
            Iterator ikv = _FindKeyValue( key );
            if( ikv == _list.end() )
                return false;

            // remove entry
            _list.RemoveAt( ikv );

            return true;
        }

    private:
        /**/
        void _Destruct()
        {
            // empty list
            _list.Empty();
        }

        /**/
        void _Copy( const Map& other )
        {
            // for each bucket add to this list
            for( Iterator i = other._list.begin(); i != other._list.end(); ++i )
            {
                KeyValue& kv = _list.Add() = KeyValue(i->key, i->value);
                kv.key = i->key;
                kv.value = i->value;
            }
        }

        /**/
        template<typename TKEY>
        KeyValue& _AccessKeyValue( const TKEY& key )
        {
            // find existing key value pair
            Iterator ikv = _FindKeyValue( key );

            // return if found
            if( ikv != _list.end() )
                return *ikv;

            // add new
            KeyValue& kv = _list.Add();

            // udpate key
            kv.key = key;

            return kv;
        }


        /**/
        template<typename TKEY>
        Iterator _FindKeyValue( const TKEY& key ) const
        {
            // locate bucket by key
            for( Iterator i = _list.begin(); i != _list.end(); ++i )
                if( i->key == key )
                    return i;

            return _list.end();
        }

        // fields
        Collection _list;
    };
}

#pragma once
#include <Core/Support/Tools.hpp>
#include <Core/Container/Array.hpp>
#include <Core/Container/String.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class Enum
    {
    public:
        struct KeyValue
        {
            TYPE    key;
            String  value;
        };

    public:
        typedef Array<KeyValue>                         KeyValueCollection;
        typedef typename KeyValueCollection::Iterator   KeyValueIterator;

    private:
        KeyValueCollection _items;

    public:
        /**/
        Enum( KeyValue* items, UShort count ):
            _items(items, count)
        {
        }

        /**/
        Bool GetValue( String& value, const TYPE& key ) const
        {
            String s;

            for( KeyValueIterator i = _items.Begin(); i != _items.End(); ++i )
            {
                if( key == i->key )
                {
                    value = i->value;
                    return true;
                }
            }

            return false;
        }

        /**/
        Bool GetKey( TYPE& key, const String& value ) const
        {
            String s;

            for( KeyValueIterator i = _items.Begin(); i != _items.End(); ++i )
            {
                if( value == i->value )
                {
                    key = i->key;
                    return true;
                }
            }

            return false;
        }
    };

    /* common types */
    typedef Enum<Id> IdEnum;
}

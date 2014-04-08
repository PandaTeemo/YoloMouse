#pragma once
#include <Core/Container/Array.hpp>
#include <Core/Container/String.hpp>

namespace Core
{
    /**/
    class Settings
    {
    public:
        struct KeyValue
        {
            ShortString     key;
            MediumString    value;
        };

        typedef Array<KeyValue>                 KeyValueCollection;
        typedef KeyValueCollection::Iterator    KeyValueIterator;

    private:
        KeyValueCollection  _keyvalues;
        MediumString        _path;
        Bool                _ready;
        ULong               _accessed;

    public:
        /**/
        Settings( KeyValueCollection& items, String path );

        /**/
        Bool Load();
        Bool Save();

        /**/
        String  Get( Id id ) const;
        Bool    GetBoolean( Id id ) const;
        Long    GetNumber( Id id ) const;
        const KeyValueCollection& GetCollection() const;

        /**/
        void Set( Id id, String value );
        void SetBoolean( Id id, Bool boolean );
        void SetNumber( Id id, Long number );

        /**/
        Id Find( String key );
    };
}

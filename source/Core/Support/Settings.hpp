#pragma once
#include <Core/Container/Array.hpp>
#include <Core/Container/String.hpp>
#include <Core/Types.hpp>

namespace Core
{
    /**/
    class Settings
    {
    public:
        // types
        struct KeyValue
        {
            MediumString    key;
            MediumString    value;
        };

        typedef Array<KeyValue>                 KeyValueCollection;
        typedef KeyValueCollection::Iterator    KeyValueIterator;

        /**/
        Settings( KeyValueCollection& items );

        /**/
        void SetPath( const PathString& path );

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

    private:
        // fields
        KeyValueCollection  _keyvalues;
        PathString          _path;
        Bool                _ready;
        ULong               _accessed;
    };
}

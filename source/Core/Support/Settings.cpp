#include <Core/Support/Settings.hpp>
#include <stdio.h>

namespace Core
{
    // public
    //-------------------------------------------------------------------------
    Settings::Settings( KeyValueCollection& keyvalues ):
        _keyvalues  (keyvalues),
        _ready      (false),
        _accessed   (0)
    {
        _path[0] = 0;
    }

    //-------------------------------------------------------------------------
    void Settings::SetPath( const WCHAR* path )
    {
        wcscpy_s( _path, COUNT(_path), path );
    }

    //-------------------------------------------------------------------------
    Bool Settings::Load()
    { 
        Char    line[STRING_MEDIUM_SIZE];
        FILE*   file;

        if(_wfopen_s(&file, _path, L"rt") == 0)
        {
            while(fgets(line, sizeof(line), file))
            {
                Char* equal = strchr(line, '=');
                if(equal)
                {
                    Char* nl = strchr(line, '\n');
                    if(nl)
                        *nl = 0;
                    *equal = 0;

                    Id id = Find(line);
                    if( id != INVALID_INDEX )
                        Set(id, equal+1);
                }
            }

            fclose(file);
            return true;
        }
        else
            return Save();
    }

    //-------------------------------------------------------------------------
    Bool Settings::Save()
    {
        FILE* file;

        if(_wfopen_s(&file, _path, L"wt") == 0)
        {
            for( KeyValueIterator i = _keyvalues.Begin(); i != _keyvalues.End(); ++i )
                fprintf(file, "%s=%s\n", i->key.GetMemory(), i->value.GetMemory());

            fclose(file);
            return true;
        }

        return false;
    }

    //-------------------------------------------------------------------------
    String Settings::Get( Id id ) const
    {
        xassert( static_cast<Index>(id) < _keyvalues.GetCount() );
        return _keyvalues[id].value;
    }

    Bool Settings::GetBoolean( Id id ) const
    {
        String string = Get(id);
        return string.GetCount() && string[0] == '1';
    }

    Long Settings::GetNumber( Id id ) const
    {
        String string = Get(id);
        return atoi(string.GetMemory());
    }

    const Settings::KeyValueCollection& Settings::GetCollection() const
    {
        return _keyvalues;
    }

    //-------------------------------------------------------------------------
    void Settings::Set( Id id, String value )
    {
        return _keyvalues[id].value.CopyZ(value);
    }

    void Settings::SetBoolean( Id id, Bool boolean )
    {
        return _keyvalues[id].value.CopyZ(boolean ? "1" : "0");
    }

    void Settings::SetNumber( Id id, Long number )
    {
        Char buffer[STRING_SHORT_SIZE];
        _itoa_s(number, buffer, sizeof(STRING_SHORT_SIZE), 10);
        return _keyvalues[id].value.CopyZ(buffer);
    }

    //-------------------------------------------------------------------------
    Id Settings::Find( String key )
    {
        for( Index i = 0; i < _keyvalues.GetCount(); ++i )
            if(key == _keyvalues[i].key)
                return i;

        return INVALID_ID;
    }
}

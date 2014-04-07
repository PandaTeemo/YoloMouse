#pragma once
#include <Core/Support/Settings.hpp>
#include <stdio.h>

namespace Core
{
    // public
    //-------------------------------------------------------------------------
    Settings::Settings( KeyValueCollection& keyvalues, String path ):
        _keyvalues  (keyvalues),
        _path       (path),
        _ready      (false),
        _accessed   (0)
    {
    }

    //-------------------------------------------------------------------------
    Bool Settings::Load()
    { 
        Char    line[STRING_MEDIUM_SIZE];
        FILE*   file;

        if(fopen_s(&file, _path.GetMemory(), "rt") == 0)
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

        if(fopen_s(&file, _path.GetMemory(),"wt") == 0)
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

    //-------------------------------------------------------------------------
    Id Settings::Find( String key )
    {
        for( Index i = 0; i < _keyvalues.GetCount(); ++i )
            if(key == _keyvalues[i].key)
                return i;

        return INVALID_ID;
    }
}

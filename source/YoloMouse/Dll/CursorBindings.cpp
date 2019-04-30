#include <YoloMouse/Dll/CursorBindings.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <YoloMouse/Share/SharedState.hpp>
#include <io.h>
#include <wchar.h>

namespace YoloMouse
{
    // CursorBindings.Binding
    //-------------------------------------------------------------------------
    CursorBindings::Binding::Binding():
        cursor_hash     (0),
        size_index      (INVALID_INDEX),
        resource_type   (RESOURCE_UNKNOWN),
        resource_index  (0)
    {
    }

    Bool CursorBindings::Binding::operator==( const Hash& cursor_hash_ ) const
    {
        return cursor_hash == cursor_hash_;
    }

    // public
    //-------------------------------------------------------------------------
    CursorBindings::CursorBindings()
    {
    }

    //-------------------------------------------------------------------------
    CursorBindings::Binding* CursorBindings::GetBindingByHash( Hash cursor_hash ) const
    {
        return _bindings.Find<Hash>(cursor_hash);
    }

    const CursorBindings::BindingCollection& CursorBindings::GetBindings() const
    {
        return _bindings;
    }

    //-------------------------------------------------------------------------
    CursorBindings::Binding* CursorBindings::Add( const Binding& binding )
    {
        // validate
        if( !_ValidateBinding( binding ) )
        {
            elog("CursorBindings.Add.ValidateBinding");
            return NULL;
        }

        // find free binding
        Binding* free_binding = _bindings.Find<Hash>(0);

        // fail if full
        if( binding == NULL )
        {
            elog("CursorBindings.Add.Full");
            return NULL;
        }

        // update free binding
        *free_binding = binding;

        return free_binding;
    }

    //-------------------------------------------------------------------------
    void CursorBindings::Remove( CursorBindings::Binding& binding )
    {
        // resets hash to 0, making it free entry
        binding = Binding();
    }

    //-------------------------------------------------------------------------
    Bool CursorBindings::Load( const WCHAR* target_id )
    {
        PathString  load_path;
        FILE*       file = NULL;

        // build save path
        if(!SharedTools::BuildUserPath(load_path, COUNT(load_path), target_id, EXTENSION_INI, NULL))
        {
            elog("CursorBindings.Load.BuildUserPath: %s", Tools::WToCString(target_id));
            return false;
        }

        // open binary file for reading
        if( _wfopen_s(&file, load_path, L"rt") != 0 )
            return false;

        // reset bindings collection
        _bindings.Zero();

        // for each line in file, until end of file or sanity limit reached
        for( Index linei = 0; feof(file) == 0 && linei < (CURSOR_BINDING_LIMIT * 3); ++linei )
        {
            Binding binding;

            // read binding line
            if( _ReadBindingLine( binding, file ) )
            {
                // find free binding
                Binding* free_binding = _bindings.Find<Hash>(0);

                // if available, replace with read binding
                if( free_binding != NULL )
                    *free_binding = binding;
            }
        }

        // close file
        fclose(file);

        return true;
    }

    Bool CursorBindings::Save( const WCHAR* target_id )
    {
        ULong       written = 0;
        PathString  save_path;
        FILE*       file = NULL;

        // build save path
        if(!SharedTools::BuildUserPath(save_path, COUNT(save_path), target_id, EXTENSION_INI, NULL))
        {
            elog("CursorBindings.Save.BuildUserPath: %s", Tools::WToCString(target_id));
            return false;
        }
        
        // open binary file for writing
        if( _wfopen_s(&file, save_path, L"wt") != 0 )
        {
            elog("CursorBindings.Save.FileOpen: %s", Tools::WToCString(save_path));
            return false;
        }

        // for each binding
        for( const Binding& binding: _bindings )
        {
            // if valid entry
            if( binding.cursor_hash != 0 )
            {
                // if successfully written, increment written count
                if( _WriteBindingLine( binding, file ) )
                    written++;
            }
        }

        // close file
        fclose(file);

        // if nothing written remove file instead
        if( written == 0 )
            _wunlink(save_path);

        return true;
    }

    // private
    //-------------------------------------------------------------------------
    Bool CursorBindings::_ReadBindingLine( Binding& binding, FILE* file )
    {
        Index size = CURSOR_SIZE_INDEX_DEFAULT;
        Char  iresource_type = 0;
        Char  ini_line[INI_LINE_LIMIT];

        // set defaults
        binding.cursor_hash =       0;
        binding.resource_type =     RESOURCE_UNKNOWN;
        binding.resource_index =    INVALID_INDEX;

        // read line
        if( fgets( ini_line, sizeof( ini_line ), file ) == NULL )
            return false;

        // read binding line from file (hash,size,resource-type,resource-id)
        if( sscanf_s( ini_line, "%I64u,%u,%c,%u\n", &binding.cursor_hash, &size, &iresource_type, 1, &binding.resource_index ) != 4 )
        {
            //BEGIN: v0.9.1 backwards compatibility
            if( sscanf_s( ini_line, "%u=%I64u,%u\n", &binding.resource_index, &binding.cursor_hash, &size ) < 2 )
                return false;
            iresource_type = RESOURCE_PRESET;
            //END: v0.9.1 backwards compatibility
        }

        // translate fields
        binding.resource_type = static_cast<ResourceType>(iresource_type);
        binding.size_index = SharedTools::CursorSizeToSizeIndex(size);

        // validate
        return _ValidateBinding( binding );
    }

    Bool CursorBindings::_WriteBindingLine( const Binding& binding, FILE* file )
    {
        // write binding line to file
        fprintf_s( file, "%I64u,%u,%c,%u\n",
            binding.cursor_hash,
            CURSOR_SIZE_TABLE[binding.size_index],
            binding.resource_type,
            binding.resource_index );

        return true;
    }

    //-------------------------------------------------------------------------
    Bool CursorBindings::_ValidateBinding( const Binding& binding )
    {
        // validate hash
        if( binding.cursor_hash == 0 )
            return false;

        // validate size index
        if( binding.size_index >= CURSOR_SIZE_INDEX_COUNT )
            return false;

        // validate resource type + parameters
        switch( binding.resource_type )
        {
        case RESOURCE_PRESET:
            if( binding.resource_index >= CURSOR_RESOURCE_PRESET_COUNT )
                return false;
            break;
        case RESOURCE_IDENTITY:
            break;
        default:
            return false;
        }

        return true;
    }
}

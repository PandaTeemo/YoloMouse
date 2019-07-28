#include <YoloMouse/Share/Cursor/CursorBindingsSerializer.hpp>
#include <YoloMouse/Share/Cursor/CursorTools.hpp>
#include <io.h>
#include <wchar.h>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // conversions
        //---------------------------------------------------------------------
        CursorType CharToCursorType( Char c )
        {
            switch( c )
            {
            case 'b':
            case 'p':
                return CURSOR_TYPE_BASIC;
            case 'o':
                return CURSOR_TYPE_OVERLAY;
            case 'c':
            case 'i':
                return CURSOR_TYPE_CLONE;
            default:
                return CURSOR_TYPE_INVALID;
            }
        }

        Char CursorTypeToChar( CursorType type )
        {
            switch( type )
            {
            case CURSOR_TYPE_BASIC:
                return 'b';
            case CURSOR_TYPE_OVERLAY:
                return 'o';
            case CURSOR_TYPE_CLONE:
                return 'c';
            default:
                return '?';
            }
        }
    }

    // public
    //-------------------------------------------------------------------------
    Bool CursorBindingsSerializer::Load( CursorBindings& bindings, const PathString& bindings_path )
    {
        FILE* file = NULL;

        // open binary file for reading
        if( _wfopen_s( &file, bindings_path.GetMemory(), L"rt" ) != 0 )
        {
            LOG( "CursorBindingsSerializer.Load.FileNotFound: %s", Tools::WToCString(bindings_path.GetMemory()) );
            return false;
        }

        // for each line in file, until end of file or sanity limit reached
        for( Index linei = 0; feof(file) == 0 && linei < 1000; ++linei )
        {
            Hash                    hash;
            CursorInfo binding;

            // read binding line
            if( _ReadBindingLine( hash, binding, file ) )
            {
                // if default binding (hash is 0)
                if( hash == 0 )
                    bindings.GetDefaultBinding() = binding;
                else
                {
                    // create binding
                    CursorInfo* create_binding = bindings.CreateBinding(hash);

                    // if created, replace with read binding
                    if( create_binding != nullptr )
                        *create_binding = binding;
                }
            }
        }

        // close file
        fclose(file);

        return true;
    }

    Bool CursorBindingsSerializer::Save( const CursorBindings& bindings, const PathString& bindings_path )
    {
        ULong written = 0;
        FILE* file = NULL;

        // open binary file for writing
        if( _wfopen_s(&file, bindings_path.GetMemory(), L"wt") != 0 )
        {
            LOG("CursorBindingsSerializer.Save.FileOpen: %s", Tools::WToCString(bindings_path.GetMemory()));
            return false;
        }

        // if default binding exists
        if( bindings.GetDefaultBinding().IsValid() )
        {
            // if successfully written, increment written count
            if( _WriteBindingLine( 0, bindings.GetDefaultBinding(), file ) )
                written++;
        }

        // for each binding
        for( const CursorBindings::BindingMap::KeyValue& kv: bindings.GetBindings() )
        {
            // if successfully written, increment written count
            if( _WriteBindingLine( kv.key, kv.value, file ) )
                written++;
        }

        // close file
        fclose(file);

        // if nothing written remove file instead
        if( written == 0 )
            _wunlink(bindings_path.GetMemory());

        return true;
    }

    // private
    //-------------------------------------------------------------------------
    Bool CursorBindingsSerializer::_ReadBindingLine( Hash& hash, CursorInfo& info, FILE* file )
    {
        Char    type_char = 0;
        Index   index =     INVALID_INDEX;
        ULong   size =      CURSOR_SIZE_DEFAULT;
        Char    ini_line    [STRING_LINE_SIZE];

        // set defaults
        info = CursorInfo();

        // read line
        if( fgets( ini_line, sizeof( ini_line ), file ) == NULL )
            return false;

        // read binding line from file (hash,size,resource-type,resource-id)
        if( sscanf_s( ini_line, "%I64u,%u,%c,%u\n", &hash, &size, &type_char, 1, &index) != 4 )
        {
            //BEGIN: v0.9.1 backwards compatibility
            if( sscanf_s( ini_line, "%u=%I64u,%u\n", &index, &hash, &size ) < 2 )
                return false;
            type_char = 'b';
            //END: v0.9.1 backwards compatibility
        }

        // translate fields
        CursorTools::IndexToIdVariation( info.id, info.variation, index );
        info.type = CharToCursorType(type_char);
        info.size = CursorTools::SizeToId(size);

        // validate
        return info.IsValid();
    }

    Bool CursorBindingsSerializer::_WriteBindingLine( const Hash hash, const CursorInfo& info, FILE* file )
    {
        Index index = 0;

        // get index from id/variation (can fail)
        CursorTools::IdVariationToIndex( index, info.id, info.variation );

        // write binding line to file
        fprintf_s( file, "%I64u,%u,%c,%u\n",
            hash,
            CursorTools::IdToSize(info.size),
            CursorTypeToChar(info.type),
            index );

        return true;
    }
}

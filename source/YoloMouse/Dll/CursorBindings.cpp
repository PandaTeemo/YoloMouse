#include <YoloMouse/Dll/CursorBindings.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <io.h>
#include <wchar.h>

namespace YoloMouse
{
    // CursorBindings.Binding
    //-------------------------------------------------------------------------
    Bool CursorBindings::Binding::operator==( const Hash& hash_ ) const
    {
        return bitmap_hash == hash_;
    }

    // public
    //-------------------------------------------------------------------------
    CursorBindings::CursorBindings()
    {
        _bindings.Zero();
    }

    //-------------------------------------------------------------------------
    CursorBindings::Binding* CursorBindings::GetBinding( Hash cursor_hash ) const
    {
        // find binding
        MapIterator binding = _bindings.Find<Hash>(cursor_hash);

        // return binding
        return binding ? binding : NULL;
    }

    const CursorBindings::MapTable& CursorBindings::GetMap() const
    {
        return _bindings;
    }

    //-------------------------------------------------------------------------
    CursorBindings::Binding* CursorBindings::Add( Hash cursor_hash, Index resource_index, Index size_index )
    {
        // bounds checks
        if( cursor_hash == 0 )
            return NULL;
        if( resource_index >= CURSOR_RESOURCE_LIMIT )
            return NULL;
        if( size_index >= CURSOR_SIZE_COUNT )
            return NULL;

        // find free binding
        MapIterator binding = _bindings.Find<Hash>(0);

        // fail if full
        if( binding == NULL )
            return NULL;

        // build
        binding->resource_index = resource_index;
        binding->size_index = size_index;
        binding->bitmap_hash = cursor_hash;

        return binding;
    }

    //-------------------------------------------------------------------------
    void CursorBindings::Remove( CursorBindings::Binding& binding )
    {
        // reset
        binding.bitmap_hash = 0;
        binding.resource_index = INVALID_INDEX;
        binding.size_index = INVALID_INDEX;
    }

    //-------------------------------------------------------------------------
    Bool CursorBindings::Load( const WCHAR* target_id )
    {
        PathString  save_path;
        Hash        bitmap_hash;
        Index       resource_index;
        FILE*       file = NULL;

        // build save path
        if(!SharedTools::BuildSavePath(save_path, COUNT(save_path), target_id))
            return false;

        // open binary file for writing
        if( _wfopen_s(&file, save_path, L"rt") != 0 )
            return false;

        // reset map
        _bindings.Zero();

        // for each line
        while( true )
        {
            Index size_index = CURSOR_SIZE_DEFAULT;

            // read line
            if( fscanf_s(file, "%u=%I64u,%u\n", &resource_index, &bitmap_hash, &size_index) < 2 )
                break;

            // add entry
            Add( bitmap_hash, resource_index, size_index );
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
        if(!SharedTools::BuildSavePath(save_path, COUNT(save_path), target_id))
            return false;
        
        // open binary file for writing
        if( _wfopen_s(&file, save_path, L"wt") != 0 )
            return false;

        // for each map entry
        for( MapIterator binding = _bindings.Begin(); binding != _bindings.End(); ++binding )
        {
            // write valid entry
            if( binding->bitmap_hash != 0 )
            {
                fprintf_s(file, "%u=%I64u,%u\n", binding->resource_index, binding->bitmap_hash, binding->size_index);
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
}

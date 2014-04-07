#include <YoloMouse/Dll/CursorBindings.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

namespace YoloMouse
{
    // CursorBindings.Mapping
    //-------------------------------------------------------------------------
    Bool CursorBindings::Mapping::operator==( const Hash& hash ) const
    {
        return _hash == hash;
    }

    // public
    //-------------------------------------------------------------------------
    CursorBindings::CursorBindings()
    {
        _map.Zero();
    }

    //-------------------------------------------------------------------------
    const CursorBindings::Mapping& CursorBindings::Get( Index mapping_index ) const
    {
        return _map[mapping_index];
    }

    //-------------------------------------------------------------------------
    Index CursorBindings::Add( Hash cursor_hash, Index cursor_index )
    {
        // find free mapping
        MapIterator cm = _map.Find<Hash>(0);

        // fail if full
        if( cm == NULL )
            return INVALID_INDEX;

        // set hash last (avoid mutex need)
        cm->_index = cursor_index;
        cm->_hash = cursor_hash;

        return _map.IndexOf(cm);
    }

    //-------------------------------------------------------------------------
    void CursorBindings::Remove( Index mapping_index )
    {
        Mapping& cm = _map[mapping_index];

        // clear hash first (avoid mutex need)
        cm._hash = 0;
        cm._index = INVALID_INDEX;
    }

    //-------------------------------------------------------------------------
    Index CursorBindings::Find( Hash cursor_hash )
    {
        // find mapping
        MapIterator cm = _map.Find<Hash>(cursor_hash);

        // return index
        return cm ? _map.IndexOf(cm) : INVALID_INDEX;
    }


    //-------------------------------------------------------------------------
    Bool CursorBindings::Load( const Char* target_id )
    {
        Char    save_path[STRING_PATH_SIZE];
        Hash    cursor_hash;
        Index   cursor_index;
        FILE*   file = NULL;

        // build save path
        SharedTools::BuildTargetSavePath(save_path, sizeof(save_path), target_id);

        // open binary file for writing
        if( fopen_s(&file, save_path, "rt") != 0 )
            return false;

        // reset map
        _map.Zero();

        // add each map entry
        while( fscanf_s(file, "%u=%I64u\n", &cursor_index, &cursor_hash) == 2 )
            Add( cursor_hash, cursor_index );

        // close file
        fclose(file);

        return true;
    }

    Bool CursorBindings::Save( const Char* target_id )
    {
        ULong   written = 0;
        Char    save_path[STRING_PATH_SIZE];
        FILE*   file = NULL;

        // build save path
        SharedTools::BuildTargetSavePath(save_path, sizeof(save_path), target_id);
        
        // open binary file for writing
        if( fopen_s(&file, save_path, "wt") != 0 )
            return false;

        // for each map entry
        for( MapIterator cm = _map.Begin(); cm != _map.End(); ++cm )
        {
            // write valid entry
            if( cm->_hash != 0 )
            {
                fprintf_s(file, "%u=%I64u\n", cm->_index, cm->_hash);
                written++;
            }
        }

        // close file
        fclose(file);

        // if nothing written remove file instead
        if( written == 0 )
            _unlink(save_path);

        return true;
    }
}

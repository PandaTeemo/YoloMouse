#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Cursor/CursorTools.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        const ULong CURSOR_SIZE_TABLE[CURSOR_SIZE_COUNT] =
        {
            0,
            16,
            24,
            32,
            40,
            48,
            56,
            64,
            72,
            80,
            88,
            96,
            104,
            112,
            120,
            128,
        };

        const ULong CURSOR_SIZE_TABLE_V_0_8_3[] =
        {
            0,
            16,
            24,
            32,
            48,
            64,
            80,
            96,
            112,
            128,
        };
    }

    // public
    //-------------------------------------------------------------------------
    CursorSize CursorTools::SizeToId( ULong size )
    {
        // if 0 return minimum size id
        if( size == 0 )
            return CURSOR_SIZE_MIN;

        // if old table (size is index)
        if( size < CURSOR_SIZE_TABLE[CURSOR_SIZE_MIN] )
        {
            // get size from old table
            if( size < COUNT( CURSOR_SIZE_TABLE_V_0_8_3 ) )
                size = CURSOR_SIZE_TABLE_V_0_8_3[size];
            // else return default index
            else
                return CURSOR_SIZE_DEFAULT;
        }

        // locate size index nearest requested size
        for( Index i = CURSOR_SIZE_MIN; i < CURSOR_SIZE_COUNT; ++i )
            if( size <= CURSOR_SIZE_TABLE[i] )
                return i;

        // use default
        return CURSOR_SIZE_DEFAULT;
    }

    ULong CursorTools::IdToSize( CursorSize id )
    {
        return CURSOR_SIZE_TABLE[id];
    }

    ULong CursorTools::HandleToSize( HCURSOR hcursor )
    {
        ICONINFO icon_info;
        BITMAP   bitmap = { 0 };
 
        // get base icon info
        if( GetIconInfo(hcursor, &icon_info) == FALSE )
            return 0;

        // select bitmap handle to get bitmap from
        HBITMAP hbitmap = icon_info.hbmColor ? icon_info.hbmColor : icon_info.hbmMask;

        // if either bitmap handle exists get base icon bitmap object
        if( hbitmap != NULL )
            GetObject( hbitmap, sizeof( BITMAP ), &bitmap );

        // cleanup temporary resources
        if( icon_info.hbmColor != NULL )
            DeleteObject(icon_info.hbmColor);
        if( icon_info.hbmMask != NULL )
            DeleteObject(icon_info.hbmMask);

        // return size as bitmap height (will be 0 if failed)
        return bitmap.bmHeight;
    }

    Bool CursorTools::IdVariationToIndex( Index& index, CursorId id, CursorVariation variation )
    {
        // if both id and variation valid
        if( id < CURSOR_ID_COUNT && variation < CURSOR_VARIATION_COUNT )
        {
            index = id * 10 + variation;
            return true;
        }

        return false;
    }

    Bool CursorTools::IndexToIdVariation( CursorId& id, CursorVariation& variation, Index index )
    {
        // decode id
        id = static_cast<CursorId>(index / 10);
        if( id > CURSOR_ID_COUNT )
            return false;

        // decode variation
        variation = static_cast<CursorId>(index % 10);
        if( variation > CURSOR_VARIATION_COUNT )
            return false;

        return true;
    }

    void CursorTools::PatchProperties( CursorInfo& properties, const CursorInfo& updates, CursorUpdateFlags flags )
    {
        // update type
        if( updates.type != CURSOR_TYPE_INVALID )
        {
            // when change type, reset id (but keep variation and size)
            if( properties.type != updates.type )
            {
                properties.id = 0;
                flags &= ~CURSOR_UPDATE_INCREMENT_ID;
            }
            properties.type = updates.type;
        }

        // update id
        if( updates.id < CURSOR_ID_COUNT )
            properties.id = updates.id;
        // else increment id
        else if( flags & CURSOR_UPDATE_INCREMENT_ID )
        {
            properties.id = (properties.id + 1) % CURSOR_ID_COUNT;
            if( properties.type == CURSOR_TYPE_OVERLAY && properties.id >= CURSOR_ID_OVERLAY_COUNT )
                properties.id = 0;
        }

        // update variation
        if( updates.variation < CURSOR_VARIATION_COUNT )
            properties.variation = updates.variation;
        // else increment variation
        else if( flags & CURSOR_UPDATE_INCREMENT_VARIATION )
            properties.variation = (properties.variation + 1) % CURSOR_VARIATION_COUNT;

        // update size
        if( updates.size >= CURSOR_SIZE_MIN && updates.size < CURSOR_SIZE_COUNT )
            properties.size = updates.size;
        // else increment size
        else if( flags & CURSOR_UPDATE_INCREMENT_SIZE )
            properties.size = Tools::Min(properties.size + 1, CURSOR_SIZE_COUNT - 1);
        // else decrement size
        else if( flags & CURSOR_UPDATE_DECREMENT_SIZE )
            properties.size = Tools::Max<CursorSize>(properties.size - 1, CURSOR_SIZE_MIN);
    }
}

#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Tools/CursorTools.hpp>
//#include <Psapi.h>
//#include <Shlobj.h>
//#include <stdio.h>

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
        // if original size
        if( size == 0 )
            return CURSOR_SIZE_ORIGINAL;

        // if old table (size is index)
        if( size < CURSOR_SIZE_TABLE[1] )
        {
            // get size from old table
            if( size < COUNT( CURSOR_SIZE_TABLE_V_0_8_3 ) )
                size = CURSOR_SIZE_TABLE_V_0_8_3[size];
            // else return default index
            else
                return CURSOR_SIZE_DEFAULT;
        }

        // locate size index nearest requested size
        for( Index i = 0; i < CURSOR_SIZE_COUNT; ++i )
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
}

#include <YoloMouse/Loader/Overlay/Cursor/OverlayCursorVault.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/CursorFactory.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    OverlayCursorVault::OverlayCursorVault():
        _initialized(false)
    {
        _cursors.Zero();
    }

    OverlayCursorVault::~OverlayCursorVault()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    void OverlayCursorVault::Initialize()
    {
        ASSERT( !IsInitialized() );

        // for each cursor id
        for( Id id = 0; id < CURSOR_ID_COUNT; ++id )
        {
            // create cursor (can be null)
            _cursors[id] = CursorFactory::CreateCursor( static_cast<CursorId>(id) );
        }

        // set initialized
        _initialized = true;
    }

    void OverlayCursorVault::Shutdown()
    {
        ASSERT( IsInitialized() );

        // for each cursor id
        for( Id id = 0; id < CURSOR_ID_COUNT; ++id )
        {
            // get cursor if created
            IOverlayCursor*& cursor = _cursors[id];

            // if created
            if( cursor != nullptr )
            {
                // free cursor
                delete cursor;
                cursor = nullptr;
            }
        }

        // reset built
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    const OverlayCursorVault::CursorTable& OverlayCursorVault::GetCursors() const
    {
        return _cursors;
    }

    //-------------------------------------------------------------------------
    Bool OverlayCursorVault::IsInitialized() const
    {
        return _initialized;
    }
}

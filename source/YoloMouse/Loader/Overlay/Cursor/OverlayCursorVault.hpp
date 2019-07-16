#pragma once
#include <YoloMouse/Loader/Overlay/Cursor/IOverlayCursor.hpp>
#include <YoloMouse/Loader/Overlay/Overlay.hpp>

namespace Yolomouse
{
    /**/
    class OverlayCursorVault
    {
    public:
        // aliases
        typedef FlatArray<IOverlayCursor*, CURSOR_ID_COUNT> CursorTable;

        /**/
        OverlayCursorVault();
        ~OverlayCursorVault();

        /**/
        void Initialize();
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        const CursorTable& GetCursors() const;

    private:
        // fields
        Bool        _initialized;
        Overlay*    _overlay;
        CursorTable _cursors;

    };
}

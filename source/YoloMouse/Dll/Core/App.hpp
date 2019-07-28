#pragma once
#include <Core/Container/Map.hpp>
#include <Core/Support/Singleton.hpp>
#include <Snoopy/Hook/Hook.hpp>
#include <YoloMouse/Share/Cursor/CursorVault.hpp>
#include <YoloMouse/Dll/Cursor/HandleCache.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/Ipc/IpcMessenger.hpp>
#include <YoloMouse/Share/Cursor/CursorBindings.hpp>

namespace Yolomouse
{
    /**/
    class App:
        public Singleton<App>,
        public IpcMessenger::IListener
    {
    public:
        /**/
        App();
        ~App();

        /**/
        Bool IsInitialized() const;
        Bool IsLoaded() const;

        /**/
        Bool Initialize( HINSTANCE hdll );
        void Shutdown();

        /**/
        Bool Load( const PathString& host_path, const PathString& log_path, const PathString& bindings_path );
        void Unload();

    private:
        // implement:IpcMessenger::IListener
        void OnRecv( const IpcMessage& message );

    private:
        // types
        struct WindowCursorEntry
        {
            HWND    hwnd =                  NULL;
            HCURSOR last_cursor =           NULL;
            Bool    original_class_set =    false;
            HCURSOR original_class_cursor = NULL;
        };
        // key is window class ATOM value
        typedef Map<DWORD, WindowCursorEntry> WindowCursorMap;

        // enums
        enum ClassLongNIndex: int
        {
            CLASSLONG_NINDEX_DEFAULT =      0,
            CLASSLONG_NINDEX_REFRESH =      -1000,
            CLASSLONG_NINDEX_SETCURSOR =    -1001,
        };

        /**/
        void _LoadBindings();
        Bool _LoadHooks();

        /**/
        void _UnloadHooks();

        /**/
        void _RefreshCursor( HCURSOR hcursor, HWND hwnd );
        void _RefreshCurrentCursor();

        /**/
        CursorInfo* _UpdateCursorBinding( HCURSOR hcursor, Hash cursor_hash, const CursorInfo& updates, CursorUpdateFlags flags );
        
        /**/
        HCURSOR _LoadBoundCursor( HCURSOR hcursor, Hash cursor_hash, CursorInfo& info );
        HCURSOR _LoadBoundBasicCursor( CursorInfo& info );
        HCURSOR _LoadBoundCloneCursor( HCURSOR hcursor, Hash cursor_hash, CursorInfo& info );
        void    _UnloadBoundCursor( Hash cursor_hash, const CursorInfo& info );
        HCURSOR _GetBoundCursor( Hash cursor_hash, const CursorInfo& info );

        /**/
        Bool    _GetCurrentCursor( HCURSOR& hcursor, HWND& hwnd );
        HCURSOR _GetClassCursor( HWND hwnd );
        void    _SetClassCursor( HWND hwnd, HCURSOR hcursor, ClassLongNIndex nindex_override );
        void    _RestoreClassCursors();

        /**/
        Bool _NotifyCursorChanging( const CursorInfo& info );
        Bool _NotifyCursorShowing( Bool showing );

        /**/
        void _OnRequestLoad( const LoadIpcMessage& message );
        void _OnRequestExit();
        void _OnRequestSetCursor( const SetCursorIpcMessage& message );
        void _OnRequestSetDefaultCursor();
        void _OnRequestResetCursor();

        /**/
        void _OnInjectedCursorChanging( HCURSOR& out_cursor, HCURSOR in_cursor );
        void _OnInjectedCursorShowing( Bool showing );

        /**/
        static VOID HOOK_CALL _OnHookSetCursor( volatile Native* arguments );
        static VOID HOOK_CALL _OnHookSetClassLong( volatile Native* arguments );
        static VOID HOOK_CALL _OnHookShowCursor( volatile Native* arguments );

        // fields: parameters
        HINSTANCE       _hdll;
        PathString      _bindings_path;
        // fields: state
        Bool            _initialized;
        Bool            _loaded;
        Long            _update_size_delta;
        WindowCursorMap _window_cursor_map;
        // fields: objects
        IpcMessenger    _ipc;
        CursorBindings  _bindings;
        CursorVault     _cursor_vault;
        HandleCache     _cache;
        Snoopy::Hook    _hook_setcursor;
        Snoopy::Hook    _hook_setclasslonga;
        Snoopy::Hook    _hook_setclasslongw;
        Snoopy::Hook    _hook_showcursor;
    };
}

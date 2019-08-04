#pragma once
#include <Core/Container/Map.hpp>
#include <Core/Support/Singleton.hpp>
#include <Core/Events/SystemMonitorEvent.hpp>
#include <Core/Support/EventDispatcher.hpp>
#include <YoloMouse/Loader/Events/OverlayEvent.hpp>
#include <YoloMouse/Loader/Overlay/Overlay.hpp>
#include <YoloMouse/Loader/Target/Target.hpp>

namespace Yolomouse
{
    /**/
    class TargetController:
        public Singleton<TargetController>,
        public EventListener<SystemMonitorEvent>,
        public EventListener<OverlayEvent>,
        public Target::IEventHandler
    {
    public:
        /**/
        TargetController();
        ~TargetController();

        /**/
        Bool Initialize();
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        void SetGamesOnly( Bool enable );

        /**/
        Bool AccessTarget( Target*& target, Bool allow_restricted_mode );

    private:
        // impl
        void OnTargetShutdown( Id process_id );

    private:
        // types
        typedef Map<Id, Target> TargetMap;

        /**/
        void _ShutdownTargets();

        /**/
        Target* _SpawnTarget( Id process_id, Bool require_configured, Bool allow_restricted_mode );

        /**/
        void _OnWindowForeground( HWND hwnd );
        void _OnWindowHover( HWND hwnd );
        Bool _OnEvent( const SystemMonitorEvent& event );
        Bool _OnEvent( const OverlayEvent& event );

        // fields: parameters
        Bool      _games_only;
        // fields: state
        Bool      _initialized;
        TargetMap _targets;
        Target*   _hover_target;
    };
}

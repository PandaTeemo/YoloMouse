//TODO3: review concurrency dangers
#include <Core/Windows/WindowTools.hpp>
#include <Core/Windows/SystemMonitor.hpp>
#include <YoloMouse/Loader/Inject/InjectEnvironment.hpp>
#include <YoloMouse/Loader/Target/Support/CursorVisibilityHacker.hpp>
#include <YoloMouse/Loader/Target/TargetController.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    TargetController::TargetController():
        _games_only  (false),
        _initialized (false),
        _hover_target(nullptr)
    {
    }

    TargetController::~TargetController()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool TargetController::Initialize()
    {
        ASSERT( !IsInitialized() );

        // initialize inject environment (can fail)
        InjectEnvironment::Instance().Initialize();

        // initialize cursor visibility hacker (can fail)
        CursorVisibilityHacker::Instance().Initialize();

        // register events
        SystemMonitor::Instance().events.Add( *this );
        Overlay::Instance().events.Add( *this );

        // set initialized
        _initialized = true;

        return true;
    }

    void TargetController::Shutdown()
    {
        CursorVisibilityHacker& cursor_visibility_hacker = CursorVisibilityHacker::Instance();
        InjectEnvironment&      inject_environment = InjectEnvironment::Instance();

        // unregister events
        Overlay::Instance().events.Remove( *this );
        SystemMonitor::Instance().events.Remove( *this );

        // shutdown any created targets
        _ShutdownTargets();

        // shutdown cursor visibility hacker
        if( cursor_visibility_hacker.IsInitialized() )
            cursor_visibility_hacker.Shutdown();

        // shutdown inject environment
        if( inject_environment.IsInitialized() )
            inject_environment.Shutdown();

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool TargetController::IsInitialized() const
    {
        return _initialized;
    }

    //-------------------------------------------------------------------------
    void TargetController::SetGamesOnly( Bool enable )
    {
        _games_only = enable;
    }

    //-------------------------------------------------------------------------
    Bool TargetController::AccessTarget( Target*& target, Bool allow_restricted_mode )
    {
        ASSERT( IsInitialized() );
        DWORD process_id = 0;

        // get focus window
        HWND hwnd = WindowTools::GetFocusWindow();
        if( hwnd == NULL )
        {
            LOG("TargetController.AccessTarget.GetFocusWindow");
            return false;
        }

        // get process and thread id
        if( GetWindowThreadProcessId( hwnd, &process_id ) == 0 )
        {
            LOG("TargetController.AccessTarget.GetWindowThreadProcessId");
            return false;
        }

        // find existing target entry
        target = _targets.Get( process_id );

        // if not found
        if( target == nullptr )
        {
            // spawn new target
            target = _SpawnTarget( process_id, false, allow_restricted_mode );

            // if failed
            if( target == nullptr )
            {
                // if restricted mode not allowed, notify user to allow it
                if( !allow_restricted_mode )
                    Overlay::Instance().SetMessage( NOTIFY_REQUIRE_RESTRICTED_MODE );
            }
            else
            {
                // notify user if entered restricted mode
                if( target->IsRestricted() )
                    Overlay::Instance().SetMessage( NOTIFY_ENTERED_RESTRICTED_MODE );
            }
        }

        return target != nullptr;
    }

    // impl
    //-------------------------------------------------------------------------
    void TargetController::OnTargetShutdown( Id process_id )
    {
        // clear if current hover target
        if( _hover_target != nullptr && process_id == _hover_target->GetProcessId() )
            _hover_target = nullptr;

        // remove target
        _targets.Remove( process_id );
    }

    // private
    //-------------------------------------------------------------------------
    void TargetController::_ShutdownTargets()
    {
        // for each target
        for( TargetMap::KeyValue& kv : _targets )
        {
            Target& target = kv.value;

            // if initialized
            if( target.IsInitialized() )
            {
                // stop if started
                if( target.IsStarted() )
                    target.Stop();

                // shutdown
                target.Shutdown();
            }
        }
    }

    //-------------------------------------------------------------------------
    Target* TargetController::_SpawnTarget( Id process_id, Bool require_configured, Bool allow_restricted_mode )
    {
        // add new target
        Target& target = _targets.Set( process_id );

        // initialize target
        if( target.Initialize( process_id, require_configured, *this ) )
        {
            // start target
            if( target.Start(allow_restricted_mode) )
                return &target;

            // if fail shutdown target
            target.Shutdown();
        }

        // if fail remove target
        _targets.Remove( process_id );

        return nullptr;
    }

    //-------------------------------------------------------------------------
    void TargetController::_OnWindowForeground( HWND hwnd )
    {
        DWORD process_id = 0;

        // get window process id
        if( GetWindowThreadProcessId( hwnd, &process_id ) != 0 )
        {
            // find existing target entry
            Target* target = _targets.Get( process_id );

            // if not found, spawn target if installed
            if( target == nullptr )
                _SpawnTarget( process_id, true, true );
        }
    }

    void TargetController::_OnWindowHover( HWND hwnd )
    {
        DWORD process_id = 0;

        // get window process id
        if( GetWindowThreadProcessId( hwnd, &process_id ) != 0 )
        {
            // if hover target exists
            if( _hover_target != nullptr )
            {
                // if current hover target changing
                if( process_id != _hover_target->GetProcessId() )
                {
                    // notify previous hover target
                    if( _hover_target->IsStarted() )
                        _hover_target->OnHoverOut();

                    // update hover target
                    _hover_target = _targets.Get( process_id );
                }
            }
            // else lookup hover target
            else
                _hover_target = _targets.Get( process_id );

            // if started target exists notify hover
            if( _hover_target != nullptr && _hover_target->IsStarted() )
                _hover_target->OnHover(hwnd);
        }
    }

    Bool TargetController::_OnEvent( const SystemMonitorEvent& event )
    {
        if( event.id == SystemMonitorEvent::WINDOW_FOREGROUND )
        {
            _OnWindowForeground( event.hwnd );
            return true;
        }

        return false;
    }

    Bool TargetController::_OnEvent( const OverlayEvent& event )
    {
        if( event.id == OverlayEvent::WINDOW_HOVER )
        {
            _OnWindowHover( event.hwnd );
            return true;
        }

        return false;
    }
}

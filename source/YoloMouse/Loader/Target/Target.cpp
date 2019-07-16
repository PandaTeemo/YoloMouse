#include <YoloMouse/Loader/Core/App.hpp>
#include <YoloMouse/Loader/Overlay/Overlay.hpp>
#include <YoloMouse/Loader/Target/Support/CursorVisibilityHacker.hpp>
#include <YoloMouse/Loader/Target/Target.hpp>
#include <YoloMouse/Share/Bindings/CursorBindingsSerializer.hpp>
#include <Psapi.h>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    Target::Target():
        _process_id     (INVALID_ID),
        _initialized    (false),
        _started        (false),
        _hover_hwnd     (NULL),
        _showing        (true),
        _process        (NULL),
        _wait_handle    (NULL)
    {
        _bindings_path.Zero();
    }

    Target::~Target()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    Bool Target::IsInitialized() const
    {
        return _initialized;
    }

    Bool Target::IsStarted() const
    {
        return _started;
    }

    //-------------------------------------------------------------------------
    Id Target::GetProcessId() const
    {
        return _process_id;
    }

    //-------------------------------------------------------------------------
    Bool Target::ActionSetCursor( CursorType type, CursorId id )
    {
        ASSERT( type != CURSOR_TYPE_INVALID );
        ASSERT( id < CURSOR_ID_COUNT );

        // if inject session loaded
        if( _inject_session.IsLoaded() )
            return _inject_session.SendSetCursor( type, id, CURSOR_VARIATION_INVALID, 0 );
        // else restricted state
        else
        {
            // update as overlay binding directly
            _UpdateRestrictedBinding( CURSOR_TYPE_OVERLAY, id, CURSOR_VARIATION_INVALID, 0 );

            // update cursor
            _UpdateCursor();
        }

        return true;
    }

    Bool Target::ActionSetDefaultCursor()
    {
        // if inject session loaded
        if( _inject_session.IsLoaded() )
            return _inject_session.SendSetDefaultCursor();
        else
        {
            // in restricted state, cursors are already the default
        }

        return true;
    }

    Bool Target::ActionResetCursor()
    {
        // if inject session loaded
        if( _inject_session.IsLoaded() )
            return _inject_session.SendResetCursor();
        // else restricted state
        else
        {
            // reset cursor binding
            _binding = CursorBindings::Binding();

            // update cursor
            _UpdateCursor();
        }

        return true;
    }

    Bool Target::ActionSetCursorSize( Long size_index_delta )
    {
        // if inject session loaded
        if( _inject_session.IsLoaded() )
            return _inject_session.SendSetCursor( CURSOR_TYPE_INVALID, CURSOR_ID_INVALID, CURSOR_VARIATION_INVALID, size_index_delta );
        // else restricted state
        else
        {
            // update as overlay binding directly
            _UpdateRestrictedBinding( CURSOR_TYPE_OVERLAY, CURSOR_ID_INVALID, CURSOR_VARIATION_INVALID, size_index_delta );

            // update cursor
            _UpdateCursor();
        }

        return true;
    }

    // friends: TargetController
    //-------------------------------------------------------------------------
    Bool Target::Initialize( Id process_id, Bool require_configured, IEventHandler& event_handler )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _process_id = process_id;
        _event_handler = &event_handler;

        // build bindings path
        if( !_BuildBindingsPath() )
            return false;

        // if require already configured require bindings path exists
        if( require_configured && !Tools::DoesFileExist(_bindings_path.GetMemory()) )
            return false;

        // initialize process listener
        if( !_InitializeProcessListener() )
            return false;

        // set initialized
        _initialized = true;

        return true;
    }

    void Target::Shutdown()
    {
        ASSERT( IsInitialized() );
        ASSERT( !IsStarted() );

        // shutdown process listener
        _ShutdownProcessListener();

        // reset fields
        _process_id = INVALID_ID;

        // reset initialized
        _initialized = false;
    }

    //-------------------------------------------------------------------------
    Bool Target::Start()
    {
        ASSERT( !IsStarted() );

        // initialize inject session. if fail we will assume anticheat or 
        // some other restriction and operate using overlay cursors only.
        if( _inject_session.Initialize( _process_id ) )
        {
            // register events
            _inject_session.events.Add( *this );

            // load inject session, else undo initialize
            if( !_inject_session.Load( _bindings_path ) )
                _inject_session.Shutdown();
        }

        // if inject session failed and therefore restricted state
        if( !_inject_session.IsLoaded() )
        {
            // load cursor map from file (can fail)
            CursorBindingsSerializer::Load( _restricted_bindings, _bindings_path );

            // use/load only default binding
            _binding = _restricted_bindings.GetDefaultBinding();

            // if invalid or not overlay, build valid default overlay binding
            if( _binding.type != CURSOR_TYPE_OVERLAY || !_binding.IsValid() )
                _BuildDefaultOverlayBinding( _binding );
        }

        // set started
        _started = true;

        return true;
    }

    void Target::Stop()
    {
        ASSERT( IsStarted() );

        // force hover out event in case not caught by listening for it
        OnHoverOut();

        // if inject session initialized
        if( _inject_session.IsInitialized() )
        {
            // unregister events
            _inject_session.events.Remove( *this );

            // if inject session loaded
            if( _inject_session.IsLoaded() )
                _inject_session.Unload();

            // shutdown inject session
            _inject_session.Shutdown();
        }

        // reset started
        _started = false;
    }

    //-------------------------------------------------------------------------
    void Target::OnHover( HWND hwnd )
    {
        // if hover hwnd changing
        if( hwnd != _hover_hwnd )
        {
            // set new hovering hwnd
            _hover_hwnd = hwnd;

            // refresh injected cursor
            if( _inject_session.IsLoaded() )
                _inject_session.SendRefreshCursor();
            // else update cursor in restricted state
            else
                _UpdateCursor();
        }
    }

    void Target::OnHoverOut()
    {
        // reset hovering state
        _hover_hwnd = NULL;

        // update cursor
        _UpdateCursor();
    }

    // private
    //-------------------------------------------------------------------------
    Bool Target::_InitializeProcessListener()
    {
        // open process with synchronize privilege
        _process = OpenProcess(SYNCHRONIZE, FALSE, _process_id);
        if( _process != NULL )
        {
            // register process exit handler
            if( RegisterWaitForSingleObject( &_wait_handle, _process, _OnProcessExit, this, INFINITE, WT_EXECUTEONLYONCE ) )
                return true;

            // close process handle
            CloseHandle(_process);
            _process = NULL;
        }

        return false;
    }

    void Target::_ShutdownProcessListener()
    {
        // if process handle opened
        if( _process != NULL )
        {
            // if wait handle created, unregister it
            if( _wait_handle != NULL )
            {
                UnregisterWait(_wait_handle);
                _wait_handle = NULL;
            }

            // close process handle
            CloseHandle(_process);
            _process = NULL;
        }
    }

    //-------------------------------------------------------------------------
    void Target::_UpdateRestrictedBinding( CursorType type, CursorId id, CursorVariation variation, CursorSize size_delta )
    {
        // get last binding id
        CursorId last_id = _binding.id;

        // restricted binding type is always overlay
        _binding.type = CURSOR_TYPE_OVERLAY;

        // if updating id, update id
        if( id != CURSOR_ID_INVALID )
            _binding.id = id;

        // if updating variation, update variation
        if( variation != CURSOR_VARIATION_INVALID )
            _binding.variation = variation;
        // else if not updating size, rotate current variation
        else if( size_delta == 0 )
            _binding.variation = _binding.variation == CURSOR_VARIATION_INVALID || id != last_id ? 0 : ((_binding.variation + 1) % CURSOR_VARIATION_COUNT);

        // if specified, update size by size delta
        if( size_delta != 0 )
            _binding.size = Tools::Clamp<Long>(_binding.size + size_delta, 0, CURSOR_SIZE_COUNT - 1);

        // update default restricted binding
        _restricted_bindings.GetDefaultBinding() = _binding;

        // save cursor bindings (can fail)
        CursorBindingsSerializer::Save( _restricted_bindings, _bindings_path );
    }

    void Target::_UpdateCursor()
    {
        Overlay& overlay = Overlay::Instance();

        // if hovering, showing, and binding is overlay type, update overlay cursor
        if( _hover_hwnd != NULL && _showing && _binding.type == CURSOR_TYPE_OVERLAY )
        {
            // if cursor not supported, update to use id 0 instead.
            if( !Overlay::Instance().IsCursorInstalled( _binding.id ) )
                _binding.id = 0;

            // show overlay cursor
            overlay.ShowCursor( _binding.id, _binding.variation, _binding.size );

            // if restricted state, hack hide application cursor
            if( !_inject_session.IsLoaded() )
                CursorVisibilityHacker::Instance().Hide(_hover_hwnd, HACK_VISIBILITY_TIMEOUT);
        }
        else
        {
            // hide overlay cursor
            overlay.HideCursor();

            // if hovering and restricted state, hack show application cursor
            if( _hover_hwnd != NULL && !_inject_session.IsLoaded() )
                CursorVisibilityHacker::Instance().Show(_hover_hwnd, HACK_VISIBILITY_TIMEOUT);
        }
    }

    //-------------------------------------------------------------------------
    Bool Target::_BuildBindingsPath()
    {
        static const ULong SLASH_LIMIT = 4;
        Bool        ok = false;
        PathString  path;

        // get process
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, _process_id);
        if( process != NULL )
        {
            // get executable path and build target id string
            if( GetProcessImageFileName(process, path.EditMemory(), path.GetCount()) != 0 )
            {
                ULong   length = (ULong)wcslen(path.GetMemory());
                ULong   slashes = 0;
                WCHAR*  end = length + path.EditMemory();
                WCHAR   c;

                // state to supress path numbers. this helps to maintain a common
                // id for targets with versioned directories.
                Bool supress_numbers = false;

                // for each character from the end of the path
                do
                {
                    // get next character
                    c = *--end;

                    // if not alphanumeric
                    if( (c < 'a' || c > 'z') &&
                        (c < 'A' || c > 'Z') &&
                        (supress_numbers || c < '0' || c > '9') )
                    {
                        // replace with _
                        *end = '_';
                    }

                    // if slash
                    if( (c == '\\' || c == '/') )
                    {
                        supress_numbers = true;
                        slashes++;
                    }
                }
                while( end > path.GetMemory() && slashes < SLASH_LIMIT );

                // copy starting at end to target id
                if( swprintf_s( _bindings_path.EditMemory(), _bindings_path.GetCount(), L"%s\\%s.%s",
                    App::Instance().GetUserPath().GetMemory(),
                    end + 1,
                    EXTENSION_INI ) > 0 )
                {
                    // set ok status
                    ok = true;
                }
            }
            else
                LOG("TargetTools.BuildTargetId.GetProcessImageFileName");
        }

        // close process
        CloseHandle(process);

        return ok;
    }

    //-------------------------------------------------------------------------
    void Target::_OnInjectedCursorChanging( const CursorBindings::Binding& binding )
    {
        // save binding
        _binding = binding;

        // if hovering over target
        if( _hover_hwnd != NULL )
        {
            // reset showing state. cursor changing does not imply making it visible
            // but since injected impl isnt perfect and messages/state may be lost this
            // is a fallback to restoring visibility for now. if games are found to
            // change cursors while being hidden, review this again.
            _showing = true;

            // update cursor
            _UpdateCursor();
        }
    }

    void Target::_OnInjectedCursorShowing( Bool showing )
    {
        // save last showing state
        _showing = showing;

        // if hovering over target
        if( _hover_hwnd != NULL )
        {
            // update cursor
            _UpdateCursor();
        }
    }

    Bool Target::_OnEvent( const InjectSessionEvent& event )
    {
        // handle event
        switch( event.id )
        {
        case InjectSessionEvent::CURSOR_CHANGING:
            _OnInjectedCursorChanging( event.u.cursor_changing );
            return true;
        case InjectSessionEvent::CURSOR_SHOWING0:
            _OnInjectedCursorShowing( event.u.cursor_showing );
            return true;
        default:
            return false;
        }
    }

    //-------------------------------------------------------------------------
    VOID CALLBACK Target::_OnProcessExit( _In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired )
    {
        Target* target = reinterpret_cast<Target*>(lpParameter);
        ASSERT(target);

        // if target initialized
        if( target->IsInitialized() )
        {
            Id process_id = target->GetProcessId();

            // if target started, stop
            if( target->IsStarted() )
                target->Stop();

            // shutdown session
            target->Shutdown();

            // notify target shutdown
            target->_event_handler->OnTargetShutdown( process_id );
        }
    }

    //-------------------------------------------------------------------------
    void Target::_BuildDefaultOverlayBinding( CursorBindings::Binding& binding )
    {
        binding.type = CURSOR_TYPE_OVERLAY;
        binding.id = 0;
        binding.variation = 0;
        binding.size = CURSOR_SIZE_DEFAULT;
    }
}

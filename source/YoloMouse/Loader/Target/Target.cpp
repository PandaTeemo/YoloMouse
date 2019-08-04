#include <YoloMouse/Loader/Core/App.hpp>
#include <YoloMouse/Loader/Overlay/Overlay.hpp>
#include <YoloMouse/Loader/Target/Support/CursorVisibilityHacker.hpp>
#include <YoloMouse/Loader/Target/Target.hpp>
#include <YoloMouse/Share/Cursor/CursorBindingsSerializer.hpp>
#include <YoloMouse/Share/Cursor/CursorTools.hpp>
#include <Psapi.h>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    Target::Target():
        _process_id (INVALID_ID),
        _initialized(false),
        _started    (false),
        _hover_hwnd (NULL),
        _showing    (true),
        _process    (NULL),
        _wait_handle(NULL)
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

    Bool Target::IsRestricted() const
    {
        return !_inject_session.IsLoaded();
    }

    //-------------------------------------------------------------------------
    Id Target::GetProcessId() const
    {
        return _process_id;
    }

    //-------------------------------------------------------------------------
    Bool Target::SetCursor( const CursorInfo& updates, CursorUpdateFlags flags )
    {
        // if restricted state
        if( IsRestricted() )
        {
            // if inactive
            if( _active_cursor.type == CURSOR_TYPE_INVALID )
            {
                // ignore clone cursor attempt
                if( flags & ( CURSOR_UPDATE_INCREMENT_SIZE | CURSOR_UPDATE_DECREMENT_SIZE ) )
                    return false;

                // activate with default
                _BuildDefaultCursor(_active_cursor);
            }

            // update binding
            if( !_SetRestrictedBinding( updates, flags ) )
                return false;

            // update cursor
            _UpdateCursor();

            return true;
        }
        // else update through inject session
        else
            return _inject_session.SendSetCursor( updates, flags );
    }

    Bool Target::SetDefaultCursor()
    {
        // if restricted state
        if( IsRestricted() )
        {
            // in restricted state, cursors are already the default
        }
        // else update through inject session
        else
            return _inject_session.SendSetDefaultCursor();

        return true;
    }

    Bool Target::ResetCursor()
    {
        // if restricted state
        if( IsRestricted() )
        {
            // reset binding
            _ResetRestrictedBinding();

            // update cursor
            _UpdateCursor();
        }
        // else update through inject session
        else
            return _inject_session.SendResetCursor();

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
    Bool Target::Start( Bool allow_restricted_mode )
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

        // if restricted state (inject session failed)
        if( IsRestricted() )
        {
            // fail if not allowed
            if( !allow_restricted_mode )
                return false;

            //TODO2: also notify user via overlay that this happened
            LOG( "Target.Start: Entering Restricted Mode" );

            // load cursor map from file (can fail)
            CursorBindingsSerializer::Load( _restricted_bindings, _bindings_path );

            // use/load only default binding
            _active_cursor = _restricted_bindings.GetDefaultBinding();

            // verify supported cursor
            if( !_IsValidCursor(_active_cursor) )
                _BuildDefaultCursor(_active_cursor);
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

            // if restricted state, update cursor locally
            if( IsRestricted() )
                _UpdateCursor();
            // else update through inject session
            else
                _inject_session.SendRefreshCursor();
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
    Bool Target::_IsValidCursor( const CursorInfo& properties )
    {
        // verify supported cursor
        return properties.IsValid() && (properties.type == CURSOR_TYPE_BASIC || properties.type == CURSOR_TYPE_OVERLAY);
    }

    //-------------------------------------------------------------------------
    Bool Target::_SetRestrictedBinding( const CursorInfo& updates, CursorUpdateFlags flags )
    {
        CursorInfo cursor = _active_cursor;

        // patch cursor with updates
        CursorTools::PatchProperties(cursor, updates, flags);

        // verify supported cursor
        if( !_IsValidCursor(cursor) )
            return false;

        // update active cursor
         _active_cursor = cursor;

        // update default restricted binding
        _restricted_bindings.GetDefaultBinding() = _active_cursor;

        // save cursor bindings (can fail)
        CursorBindingsSerializer::Save( _restricted_bindings, _bindings_path );

        return true;
    }

    void Target::_ResetRestrictedBinding()
    {
        // reset active cursor
        _active_cursor = CursorInfo();

        // update default restricted binding
        _restricted_bindings.GetDefaultBinding() = _active_cursor;

        // save cursor bindings (can fail)
        CursorBindingsSerializer::Save( _restricted_bindings, _bindings_path );
    }

    void Target::_UpdateCursor()
    {
        Overlay& overlay = Overlay::Instance();

        // if hovering
        if( _hover_hwnd != NULL )
        {
            // if restricted state
            if( IsRestricted() )
            {
                CursorVisibilityHacker& cursor_visibility_hacker = CursorVisibilityHacker::Instance();

                // if showing and valid cursor
                if( _showing && _active_cursor.type != CURSOR_TYPE_INVALID )
                {
                    // set cursor via overlay and hack hide
                    if( overlay.SetCursorIterated( _active_cursor ) )
                        cursor_visibility_hacker.Hide(_hover_hwnd, HACK_VISIBILITY_TIMEOUT);
                }
                // else hide overlay cursor and show app cursor
                else
                {
                    overlay.SetCursorHidden();
                    cursor_visibility_hacker.Show(_hover_hwnd, HACK_VISIBILITY_TIMEOUT);
                }
            }
            // else injected state
            else
            {
                // if showing and overlay cursor set cursor via overlay else hide
                if( _showing && _active_cursor.type == CURSOR_TYPE_OVERLAY )
                    overlay.SetCursorIterated( _active_cursor );
                else
                    overlay.SetCursorHidden();
            }
        }
        // else ensure overlay cursor hidden (this should happen first before new target gets hover control)
        else
            overlay.SetCursorHidden();
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
    void Target::_OnInjectedCursorChanging( const CursorInfo& info )
    {
        // update cursor
        _active_cursor = info;

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
    void Target::_BuildDefaultCursor( CursorInfo& info )
    {
        info.type = CURSOR_TYPE_OVERLAY;
        info.id = 0;
        info.variation = 0;
        info.size = CURSOR_SIZE_DEFAULT;
    }
}

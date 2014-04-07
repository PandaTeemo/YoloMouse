#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Loader/Loader.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

namespace YoloMouse
{
    // constants
    //-------------------------------------------------------------------------
    static const Char* _HOOKPROC_NAME = "?OnHookProc@@YGJHIJ@Z";

    // TargetState.Mapping
    //-------------------------------------------------------------------------
    Bool Loader::Active::operator==( HWND hwnd ) const
    {
        return _hwnd == hwnd;
    }

    // public
    //-------------------------------------------------------------------------
    Loader::Loader():
        _dll            (NULL),
        _hook_function  (NULL)
    {
    }

    //-------------------------------------------------------------------------
    HWND Loader::GetActiveTarget()
    {
        return GetForegroundWindow();
    }

    //-------------------------------------------------------------------------
    Bool Loader::IsStarted() const
    {
        return _hook_function != NULL;
    }

    Bool Loader::IsLoaded( HWND hwnd ) const
    {
        xassert(IsStarted());
        return _actives.Find(hwnd) != NULL;
    }

    Bool Loader::IsConfigured( HWND hwnd ) const
    {
        Char save_path[STRING_PATH_SIZE];
        Char target_id[STRING_PATH_SIZE];

        // build target id
        if(! SharedTools::BuildTargetId( target_id, sizeof(target_id), hwnd ) )
            return false;

        // build save path
        SharedTools::BuildTargetSavePath(save_path, sizeof(save_path), target_id);

        // sucess if save file exists
        return Tools::DoesFileExist(save_path);
    }

    //-------------------------------------------------------------------------
    Bool Loader::Start()
    {
        xassert(!IsStarted());
        xassert(_dll == NULL);
        xassert(_hook_function == NULL);

        // load dll
        _dll = LoadLibrary(PATH_DLL32);
        if(_dll)
        {
            // get hook proc
            _hook_function = (HOOKPROC)GetProcAddress(_dll, _HOOKPROC_NAME);
            if(_hook_function)
                return true;
        }

        return false;
    }

    void Loader::Stop()
    {
        xassert(IsStarted());
        xassert(_dll);

        // unload all
        for( ActiveIterator active = _actives.Begin(); active != _actives.End(); ++active )
            UnhookWindowsHookEx(active->_hook);

        // free dll
        FreeLibrary(_dll);

        // reset state
        _dll = NULL;
        _hook_function = NULL;
        _actives.Empty();
    }

    //-------------------------------------------------------------------------
    Bool Loader::Load( HWND hwnd )
    {
        xassert(IsStarted());
        xassert(!IsLoaded(hwnd));
        DWORD process_id;

        // get thread id
        DWORD thread_id = GetWindowThreadProcessId(hwnd, &process_id);

        // get process bitness
        Bitness bitness = SystemTools::GetProcessBitness(process_id);

        //TODO: handle 64bit
        if( bitness != BITNESS_32 )
            return false;

        // set hook
        HHOOK hook = SetWindowsHookEx(WH_CALLWNDPROC, _hook_function, _dll, thread_id);
        if( hook == NULL )
            return false;

        // add to active list
        Active& active = _actives.Add();
        active._hook = hook;
        active._hwnd = hwnd;

        // notify init and pass hash
        SendMessage(hwnd, WMYOLOMOUSE_INIT, 0, 0);

        return true;
    }

    Bool Loader::Unload( HWND hwnd )
    {
        xassert(IsStarted());

        // find active entry
        ActiveIterator active = _actives.Find(hwnd);
        if( active == NULL )
            return false;

        // remove hook
        UnhookWindowsHookEx(active->_hook);

        // remove active
        _actives.PopSwap(active);

        return true;
    }

    //-------------------------------------------------------------------------
    void Loader::NotifyUpdate( HWND hwnd, Index cursor_index )
    {
        xassert(IsLoaded(hwnd));
        SendMessage(hwnd, WMYOLOMOUSE_ASSIGN, (WPARAM)cursor_index, 0);
    }
}

#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Loader/Loader.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

namespace YoloMouse
{
    // TargetState.Mapping
    //-------------------------------------------------------------------------
    Bool Loader::Active::operator==( HWND hwnd ) const
    {
        return _hwnd == hwnd;
    }

    // public
    //-------------------------------------------------------------------------
    Loader::Loader()
    {
    }

    Loader::~Loader()
    {
        // unload all
        for( ActiveIterator active = _actives.Begin(); active != _actives.End(); ++active )
            _UnloadActive(*active);
    }

    //-------------------------------------------------------------------------
    HWND Loader::GetActiveTarget()
    {
        return GetForegroundWindow();
    }

    //-------------------------------------------------------------------------
    Bool Loader::IsLoaded( HWND hwnd ) const
    {
        return _actives.Find(hwnd) != NULL;
    }

    Bool Loader::IsConfigured( HWND hwnd ) const
    {
        WCHAR save_path[STRING_PATH_SIZE];
        WCHAR target_id[STRING_PATH_SIZE];

        // build target id
        if(! SharedTools::BuildTargetId( target_id, COUNT(target_id), hwnd ) )
            return false;

        // build save path
        if(!SharedTools::BuildSavePath(save_path, COUNT(save_path), target_id))
            return false;

        // sucess if save file exists
        return Tools::DoesFileExist(save_path);
    }

    //-------------------------------------------------------------------------
    Bool Loader::Load( HWND hwnd )
    {
        xassert(!IsLoaded(hwnd));
        DWORD process_id;

        // get thread id
        DWORD thread_id = GetWindowThreadProcessId(hwnd, &process_id);

        // choose inject dll
        const Char* inject_dll = _ChooseInjectDll(process_id);
        eggs(inject_dll);

        // create injector
        Injector* injector = new Injector;
        eggs(injector);

        // set notify name
        injector->SetNotifyName(INJECT_NOTIFY_FUNCTION);

        // load
        if(injector->Load(process_id, inject_dll) && !_actives.IsFull())
        {
            ActiveIterator active = _actives.Add();

            // add to active list
            active->_hwnd = hwnd;
            active->_injector = injector;

            // notify init
            if( Notify(hwnd, NOTIFY_INIT) )
                return true;

            // remove
            _actives.PopSwap(active);
        }

        // cleanup
        delete injector;

        return false;
    }

    Bool Loader::Unload( HWND hwnd )
    {
        // find active entry
        ActiveIterator active = _actives.Find(hwnd);
        if( active == NULL )
            return false;

        // unload active entry
        _UnloadActive(*active);

        // remove active
        _actives.PopSwap(active);

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Loader::Notify( HWND hwnd, NotifyId id, Byte8 parameter )
    {
        NotifyMessage m;

        // locate active entry
        ActiveIterator active = _actives.Find(hwnd);
        if(active == NULL)
            return false;

        // build message
        m.id = id;
        m.hwnd = reinterpret_cast<Byte8>(hwnd);
        m.parameter = parameter;

        // call remote notify handler
        return active->_injector->CallNotify(&m, sizeof(m));
    }

    // private
    //-------------------------------------------------------------------------
    void Loader::_UnloadActive( Active& active )
    {
        // unload injected dll
        active._injector->Unload();

        // destroy injector
        delete active._injector;
    }

    //-------------------------------------------------------------------------
    const CHAR* Loader::_ChooseInjectDll( DWORD process_id )
    {
        // get process bitness
        Bitness bitness = SystemTools::GetProcessBitness(process_id);

        // choose dll
        if( bitness == BITNESS_32 )
            return PATH_DLL32;
        if( bitness == BITNESS_64 )
            return PATH_DLL64;

        return NULL;
    }
}

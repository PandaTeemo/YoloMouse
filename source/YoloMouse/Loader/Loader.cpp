#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Loader/Loader.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

namespace YoloMouse
{
    // TargetState.Mapping
    //-------------------------------------------------------------------------
    Bool Loader::Active::operator==( DWORD process_id ) const
    {
        return this->process_id == process_id;
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
    DWORD Loader::GetActiveProcessId()
    {
        DWORD process_id = 0;

        // get active window
        HWND hwnd = GetForegroundWindow();

        // get process id
        GetWindowThreadProcessId(hwnd, &process_id);

        return process_id;
    }

    //-------------------------------------------------------------------------
    Bool Loader::IsLoaded( DWORD process_id ) const
    {
        return _actives.Find(process_id) != NULL;
    }

    Bool Loader::IsConfigured( DWORD process_id ) const
    {
        WCHAR save_path[STRING_PATH_SIZE];
        WCHAR target_id[STRING_PATH_SIZE];

        // build target id
        if(! SharedTools::BuildTargetId( target_id, COUNT(target_id), process_id ) )
            return false;

        // build save path
        if(!SharedTools::BuildSavePath(save_path, COUNT(save_path), target_id))
            return false;

        // sucess if save file exists
        return Tools::DoesFileExist(save_path);
    }

    //-------------------------------------------------------------------------
    Bool Loader::Load( DWORD process_id )
    {
        // ignore if already loaded
        if( IsLoaded(process_id) )
            return true;

        // choose inject dll
        const Char* inject_dll = _ChooseInjectDll(process_id);
        if( inject_dll == NULL )
            return false;

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
            active->process_id = process_id;
            active->injector = injector;

            // notify init
            if( Notify(process_id, NOTIFY_INIT) )
                return true;

            // remove
            _actives.PopSwap(active);
        }

        // cleanup
        delete injector;

        return false;
    }

    Bool Loader::Unload( DWORD process_id )
    {
        // find active entry
        ActiveIterator active = _actives.Find(process_id);
        if( active == NULL )
            return false;

        // unload active entry
        _UnloadActive(*active);

        // remove active
        _actives.PopSwap(active);

        return true;
    }

    //-------------------------------------------------------------------------
    Bool Loader::Notify( DWORD process_id, NotifyId id, Byte8 parameter )
    {
        NotifyMessage m;

        // locate active entry
        ActiveIterator active = _actives.Find(process_id);
        if(active == NULL)
            return false;

        // build message
        m.id = id;
        m.parameter = parameter;

        // call remote notify handler
        return active->injector->CallNotify(&m, sizeof(m));
    }

    // private
    //-------------------------------------------------------------------------
    void Loader::_UnloadActive( Active& active )
    {
        // unload injected dll
        active.injector->Unload();

        // destroy injector
        delete active.injector;
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

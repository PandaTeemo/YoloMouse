#include <YoloMouse/Loader/InstanceManager.hpp>
#include <YoloMouse/Share/SharedTools.hpp>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    InstanceManager::InstanceManager():
        _targets(LOADER_TARGET_LIMIT)
    {
    }

    InstanceManager::~InstanceManager()
    {
        // unload all
        for( Index i = 0; i < LOADER_TARGET_LIMIT; ++i )
            _targets[i].Unload();
    }

    //-------------------------------------------------------------------------
    Instance* InstanceManager::Load( DWORD process_id )
    {
        // find free instance
        Instance* instance = Find(0);
        if( instance == NULL )
            return NULL;

        // load instance
        if( !instance->Load(process_id) )
            return NULL;

        return instance;
    }

    //-------------------------------------------------------------------------
    Instance* InstanceManager::Find( DWORD process_id )
    {
        for( InstanceIterator target = _targets.Begin(); target != _targets.End(); ++target )
            if( process_id == target->GetProcessId() )
                return target;

        return NULL;
    }
    
    //-------------------------------------------------------------------------
    Bool InstanceManager::IsConfigured( DWORD process_id ) const
    {
        WCHAR   save_path[STRING_PATH_SIZE];
        WCHAR   target_id[STRING_PATH_SIZE];
        Bool    status = false;

        // get process
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, process_id);
        if( process )
        {
            // build target id
            if( SharedTools::BuildTargetId(target_id, COUNT(target_id), process))
            {
                // build save path
                if(SharedTools::BuildSavePath(save_path, COUNT(save_path), target_id))
                {
                    // success if save file exists
                    status = Tools::DoesFileExist(save_path);
                }

                // close process
                CloseHandle(process);
            }
        }

        return status;
    }
}

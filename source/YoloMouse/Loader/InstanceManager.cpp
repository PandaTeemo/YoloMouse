#include <Core/Windows/SystemTools.hpp>
#include <YoloMouse/Loader/InstanceManager.hpp>
#include <YoloMouse/Share/SharedTools.hpp>
#include <YoloMouse/Share/SharedState.hpp>

namespace YoloMouse
{
    // public
    //-------------------------------------------------------------------------
    InstanceManager::InstanceManager():
        _targets    (LOADER_TARGET_LIMIT),
        _privileges (NULL)
    {
        // open privileges. required for openprocess to succeed by some games (like wildstar)
        _privileges = SystemTools::OpenDebugPrivileges();
    }

    InstanceManager::~InstanceManager()
    {
        // unload all
        for( Index i = 0; i < LOADER_TARGET_LIMIT; ++i )
            _targets[i].Unload();

        // close privileges handle
        if( _privileges )
            CloseHandle(_privileges);
    }

    //-------------------------------------------------------------------------
    Instance* InstanceManager::Load( DWORD process_id )
    {
        // find free instance
        Instance* instance = Find(0);
        if( instance == NULL )
        {
            elog("InstanceManager.Load.Find");
            return NULL;
        }

        // load instance
        if( !instance->Load(process_id) )
            return NULL;

        return instance;
    }

    //-------------------------------------------------------------------------
    Instance* InstanceManager::Find( DWORD process_id )
    {
        for( InstanceIterator target = _targets.begin(); target != _targets.end(); ++target )
            if( process_id == target->GetProcessId() )
                return target;

        return NULL;
    }
    
    //-------------------------------------------------------------------------
    Bool InstanceManager::IsConfigured( DWORD process_id ) const
    {
        PathString  save_path;
        PathString  target_id;
        Bool        status = false;

        // get process
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, process_id);
        if( process )
        {
            // build target id
            if( SharedTools::BuildTargetId(target_id, COUNT(target_id), process))
            {
                // build save path
                if( SharedTools::BuildUserPath(save_path, COUNT(save_path), target_id, EXTENSION_INI, process) )
                {
                    // success if save file exists
                    status = Tools::DoesFileExist(save_path);
                }
                else
                    elog("InstanceManager.IsConfigured.BuildUserPath: %s", Tools::WToCString(target_id));

                // close process
                CloseHandle(process);
            }
            else
                elog("InstanceManager.IsConfigured.BuildTargetId");
        }

        return status;
    }
}

#pragma once
#include <Core/Container/Array.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Loader/Instance.hpp>

namespace YoloMouse
{
    /**/
    class InstanceManager
    {
    public:
        /**/
        InstanceManager();
        ~InstanceManager();

        /**/
        Instance* Load( DWORD process_id );

        /**/
        Instance* Find( DWORD process_id );

        /**/
        Bool IsConfigured( DWORD process_id ) const;

    private:
        // types
        typedef FixedArray<Instance, LOADER_TARGET_LIMIT>   InstanceCollection;
        typedef InstanceCollection::Iterator                InstanceIterator;

        // fields
        InstanceCollection  _targets;
        HANDLE              _privileges;
    };
}

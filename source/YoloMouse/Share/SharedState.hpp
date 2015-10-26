#pragma once
#include <Core/Support/Singleton.hpp>
#include <Core/Windows/SharedMemory.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    /**/
    class SharedState:
        public Singleton<SharedState>
    {
    public:
        /**/
        SharedState();

        /**/
        Bool Open( Bool host );
        void Close();

        /**/
        PathString& EditPath();

    private:
        /**/
        struct Memory
        {
            PathString path;
        };

        // fields
        Bool                 _host;
        SharedMemory<Memory> _shared;
    };
}

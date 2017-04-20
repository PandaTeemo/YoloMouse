#pragma once
#include <Core/Support/Singleton.hpp>
#include <Core/Windows/SharedMemory.hpp>
#include <YoloMouse/Share/Constants.hpp>
#include <YoloMouse/Share/SharedLog.hpp>

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
        SharedLog& GetLog() const;

        /**/
        PathString& EditPath();

    private:
        /**/
        struct Memory
        {
            PathString  path;
            SharedLog   log;
        };

        // fields
        Bool                 _host;
        SharedMemory<Memory> _shared;
    };

    // macros
    #define elog SharedState::Instance().GetLog().Write
}

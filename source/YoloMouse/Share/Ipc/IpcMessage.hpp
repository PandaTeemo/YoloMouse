#pragma once
#include <YoloMouse/Share/Enums.hpp>
#include <YoloMouse/Share/Cursor/CursorBindings.hpp>

namespace Yolomouse
{
    /*
        message object communicated between loader and dll. must coexist between 32 and 64 bit.
    */
    #pragma pack(4)
    struct IpcMessage
    {
        // request id
        IpcRequest      request;
    };

    struct LoadIpcMessage:
        public IpcMessage
    {
        PathString      host_path;
        PathString      log_path;
        PathString      bindings_path;
    };

    struct SetCursorIpcMessage:
        public IpcMessage
    {
        CursorInfo    properties;
        CursorUpdateFlags   flags;
    };

    struct OnCursorChangingIpcMessage:
        public IpcMessage
    {
        CursorInfo  info;
    };

    struct OnCursorShowingIpcMessage:
        public IpcMessage
    {
        Bool showing;
    };
    #pragma pack()
}

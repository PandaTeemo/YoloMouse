#pragma once
#include <YoloMouse/Share/Enums.hpp>
#include <YoloMouse/Share/Bindings/CursorBindings.hpp>

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
        CursorType      type;
        CursorId        id;
        CursorVariation variation;
        CursorSize      size_delta;
    };

    struct OnCursorChangingIpcMessage:
        public IpcMessage
    {
        CursorBindings::Binding  binding;
    };

    struct OnCursorShowingIpcMessage:
        public IpcMessage
    {
        Bool showing;
    };
    #pragma pack()
}

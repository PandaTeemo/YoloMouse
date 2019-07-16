#pragma once
#include <Core/Container/Array.hpp>
#include <Core/Math/Vector2.hpp>
#include <YoloMouse/Share/Root.hpp>
#include <d3d11.h>

namespace Yolomouse
{
    /*
        estimates render time based on recently recorded time between begin/end calls
        and uses that to delay next rendering nearest next vblank. call begin/end around
        all frame processing+rendering right after a vblank.

        also collects frame timing between consecutive begin calls

        TODO: can improve by keeping/using history of render times instead
            of just last one
    */
    class RenderTimingController
    {
    public:
        /**/
        RenderTimingController();
        ~RenderTimingController();

        /**/
        void Initialize( IDXGIOutput& dxgi_output );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        Float GetFrameTime() const; // sec

        /**/
        void SetResolution( const Vector2l& resolution );

        /**/
        void Begin();
        void End();

    private:
        // constants
        static constexpr Float DEFAULT_REFRESH_RATE =   59.94f; // hz
        static constexpr Float RENDER_TIME_PAD =        2.0f;   // ms

        /**/
        void _CalculateRefreshRate();
        Float _TicksToSeconds( UHuge ticks );

        // fields: parameters
        IDXGIOutput*     _dxgi_output;
        Vector2l         _resolution;
        // fields: info
        UHuge            _tick_frequency;
        Float            _refresh_time;
        // fields: state
        Float            _render_time;
        Float            _frame_time;
        UHuge            _begin_ticks;
    };
}

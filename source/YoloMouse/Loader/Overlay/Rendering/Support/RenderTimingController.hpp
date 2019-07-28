#pragma once
#include <Core/Container/Array.hpp>
#include <Core/Math/Vector2.hpp>
#include <YoloMouse/Share/Root.hpp>
#include <d3d11.h>

namespace Yolomouse
{
    /*
        estimates render time based on recent frame skips and uses that to 
        delay next rendering. call begin/end around all frame processing+
        rendering right after a vblank.
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
        static constexpr Float DEFAULT_REFRESH_RATE =   59.94f;     // hz
        static constexpr Float RENDER_TIME_INITIAL =    0.002f;     // sec. initial render time
        static constexpr Float RENDER_TIME_INCREMENT =  0.0012f;    // sec. time to increase render time by during detected lag
        static constexpr Float RENDER_TIME_RECOVER =    0.000001f;  // sec. time to decrease render time by each frame.

        /**/
        void  _UpdateFillTime();
        void  _CalculateRefreshRate();
        Float _TicksToSeconds( UHuge ticks );

        // fields: parameters
        IDXGIOutput*    _dxgi_output;
        Vector2l        _resolution;
        // fields: info
        UHuge           _tick_frequency;
        Float           _refresh_time;
        Float           _frameskip_threshold;
        // fields: state
        ULong           _fill_time;
        Float           _render_time;
        Float           _frame_time;
        UHuge           _begin_ticks;
    };
}

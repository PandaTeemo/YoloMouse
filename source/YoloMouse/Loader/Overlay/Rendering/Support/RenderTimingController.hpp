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
        void Initialize( IDXGIOutput& dxgi_output, const Vector2l& resolution );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        Float GetFrameTime() const; // sec

        /**/
        void SetFillTime( Bool enabled );

        /**/
        void Begin();
        void End();

    private:
        /**/
        void  _UpdateFillTime();
        void  _CalculateRefreshRate( const Vector2l& resolution );

        // fields: parameters
        IDXGIOutput*    _dxgi_output;
        Bool            _option_fill_time;
        // fields: info
        Float           _refresh_time;
        Float           _frameskip_threshold;
        // fields: state
        ULong           _fill_time;
        Float           _render_time;
        Float           _frame_time;
        UHuge           _begin_ticks;
    };
}

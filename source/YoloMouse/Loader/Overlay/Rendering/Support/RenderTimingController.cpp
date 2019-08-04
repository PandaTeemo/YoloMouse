#include <Core/Math/Math.hpp>
#include <Core/System/SystemTools.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Support/RenderTimingController.hpp>

namespace Yolomouse
{
    // local
    //-------------------------------------------------------------------------
    namespace
    {
        // constants
        //---------------------------------------------------------------------
        static constexpr Float DEFAULT_REFRESH_RATE =   59.94f;     // hz
        static constexpr Float RENDER_TIME_INITIAL =    0.002f;     // sec. initial render time
        static constexpr Float RENDER_TIME_INCREMENT =  0.0012f;    // sec. time to increase render time by during detected lag
        static constexpr Float RENDER_TIME_RECOVER =    0.000001f;  // sec. time to decrease render time by each frame.
        static constexpr Float RENDER_TIME_LIMIT =      0.5f;       // as factor of refresh rate
    }

    // public
    //-------------------------------------------------------------------------
    RenderTimingController::RenderTimingController():
        _dxgi_output        (nullptr),
        _option_fill_time   (false),
        _refresh_time       (0),
        _render_time        (RENDER_TIME_INITIAL),
        _frameskip_threshold(0),
        _fill_time          (0),
        _frame_time         (0),
        _begin_ticks        (0)
    {
    }

    RenderTimingController::~RenderTimingController()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    void RenderTimingController::Initialize( IDXGIOutput& dxgi_output, const Vector2l& resolution )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _dxgi_output = &dxgi_output;

        // calculate refresh rate
        _CalculateRefreshRate(resolution);

        // update fill time
        _UpdateFillTime();

        // initialize begin ticks
        _begin_ticks = SystemTools::GetTickTime();
    }

    void RenderTimingController::Shutdown()
    {
        ASSERT( IsInitialized() );

        // set fields
        _dxgi_output = nullptr;
    }

    //-------------------------------------------------------------------------
    Bool RenderTimingController::IsInitialized() const
    {
        return _dxgi_output != nullptr;
    }

    //-------------------------------------------------------------------------
    Float RenderTimingController::GetFrameTime() const
    {
        return _frame_time;
    }

    //-------------------------------------------------------------------------
    void RenderTimingController::SetFillTime( Bool enabled )
    {
        _option_fill_time = enabled;
    }

    //-------------------------------------------------------------------------
    void RenderTimingController::Begin()
    {
        ASSERT( IsInitialized() );

        // if fill time option enabled
        if( _option_fill_time )
        {
            // sleep fill time (estimated render time - refresh time)
            Sleep( _fill_time );
        }
    }

    void RenderTimingController::End()
    {
        ASSERT( IsInitialized() );

        // get current ticks
        UHuge current_ticks = SystemTools::GetTickTime();

        // calculate frame time
        _frame_time = SystemTools::GetTicksToSeconds(current_ticks - _begin_ticks);

        // update begin ticks
        _begin_ticks = current_ticks;

        // if fill time option enabled
        if( _option_fill_time )
        {
            // if we're skipping frames, increase render time estimate. limit to half refresh time
            if( _frame_time > _frameskip_threshold )
                _render_time = Tools::Min( _render_time + RENDER_TIME_INCREMENT, _refresh_time * RENDER_TIME_LIMIT );
            // else decrease render time estimate a little. limit to initial render time
            else
                _render_time = Tools::Max( _render_time - RENDER_TIME_RECOVER, RENDER_TIME_INITIAL );

            // update fill time
            _UpdateFillTime();
        }

        // log
        LOG3( "RenderTimingController.FrameTime FRAME:%f FILL:%u", _frame_time, _fill_time );
    }

    //-------------------------------------------------------------------------
    void RenderTimingController::_UpdateFillTime()
    {
        // update fill time as refresh time minus current render time
        _fill_time = static_cast<ULong>((_refresh_time - _render_time) * 1000.0f);
    }

    void RenderTimingController::_CalculateRefreshRate( const Vector2l& resolution )
    {
        UINT num_modes = 0;

        // set default refresh time in case fail
        _refresh_time = 1.0f / DEFAULT_REFRESH_RATE;

        // get num modes
        if( _dxgi_output->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, 0, &num_modes, NULL ) == S_OK )
        {
            // create modes
            DXGI_MODE_DESC*       modes = new DXGI_MODE_DESC[num_modes];
            const DXGI_MODE_DESC* best_mode = nullptr;
            ULong                 best_score = ~0;

            // get modes
            if( _dxgi_output->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, 0, &num_modes, modes ) == S_OK )
            {
                // for each mode
                for( Index i = 0; i < num_modes; ++i )
                {
                    const DXGI_MODE_DESC& mode = modes[i];

                    // calculate resolution nearness score
                    ULong score = Math<Long>::Absolute(
                        (static_cast<Long>(mode.Width) - resolution.x) +
                        (static_cast<Long>(mode.Height) - resolution.y));

                    // if better than best, update best score and mode
                    if( score < best_score )
                    {
                        best_mode = &mode;
                        best_score = score;
                    }
                }
            }

            // if best mode found
            if( best_mode != nullptr )
                _refresh_time = static_cast<Float>(best_mode->RefreshRate.Denominator) / static_cast<Float>(best_mode->RefreshRate.Numerator);

            // cleanup
            delete[] modes;
        }

        // update frameskip threshold
        _frameskip_threshold = _refresh_time * 1.5f;
    }
}

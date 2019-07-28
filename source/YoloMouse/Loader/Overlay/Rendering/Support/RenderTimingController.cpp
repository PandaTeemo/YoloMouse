#include <Core/Math/Math.hpp>
#include <Core/System/SystemTools.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Support/RenderTimingController.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    RenderTimingController::RenderTimingController():
        _dxgi_output        (nullptr),
        _resolution         (0, 0),
        _tick_frequency     (0),
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
    void RenderTimingController::Initialize( IDXGIOutput& dxgi_output )
    {
        ASSERT( !IsInitialized() );

        // set fields
        _dxgi_output = &dxgi_output;

        // get tick frequency
        _tick_frequency = SystemTools::GetTickFrequency();

        // calculate refresh rate
        _CalculateRefreshRate();

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
    void RenderTimingController::SetResolution( const Vector2l& resolution )
    {
        _resolution = resolution;
    }

    //-------------------------------------------------------------------------
    void RenderTimingController::Begin()
    {
        ASSERT( IsInitialized() );

        // sleep fill time (estimated render time - refresh time)
        Sleep( _fill_time );
    }

    void RenderTimingController::End()
    {
        ASSERT( IsInitialized() );

        // get current ticks
        UHuge current_ticks = SystemTools::GetTickTime();

        // calculate frame time
        _frame_time = _TicksToSeconds(current_ticks - _begin_ticks);

        // update begin ticks
        _begin_ticks = current_ticks;

        // if we're skipping frames, increase render time estimate. limit to half refresh time
        if( _frame_time > _frameskip_threshold )
            _render_time = Tools::Min( _render_time + RENDER_TIME_INCREMENT, _refresh_time * 0.5f );
        // else decrease render time estimate a little. limit to initial render time
        else
            _render_time = Tools::Max( _render_time - RENDER_TIME_RECOVER, RENDER_TIME_INITIAL );

        // update fill time
        _UpdateFillTime();

        // log
        LOG3( "RenderTimingController.FrameTime FRAME:%f FILL:%u", _frame_time, _fill_time );
    }

    //-------------------------------------------------------------------------
    void RenderTimingController::_UpdateFillTime()
    {
        // update fill time as refresh time minus current render time
        _fill_time = static_cast<ULong>((_refresh_time - _render_time) * 1000.0f);
    }

    void RenderTimingController::_CalculateRefreshRate()
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
                        (static_cast<Long>(mode.Width) - _resolution.x) +
                        (static_cast<Long>(mode.Height) - _resolution.y));

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

    Float RenderTimingController::_TicksToSeconds( UHuge ticks )
    {
        // calculate frame time
        return static_cast<Float>((ticks * 1000000) / _tick_frequency) / 1000000.0f;
    }
}

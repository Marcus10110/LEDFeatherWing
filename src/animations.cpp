#include "animations.h"

namespace Animation
{
    Animation::Animation() = default;
    Animation::~Animation() = default;

    void Animation::Reset( const AnimationSettings& settings )
    {
        mSettings = settings;
    }

    FnAnimation::FnAnimation( AnimationFn fn ) : mFn( std::move( fn ) )
    {
    }

    FnAnimation::~FnAnimation() = default;

    void FnAnimation::Reset( const AnimationSettings& settings )
    {
        Animation::Reset( settings );
    }

    void FnAnimation::Render( SetLedFn set_led, GetLedFn get_led, uint32_t ms )
    {
        mFn( set_led, get_led, mSettings, ms );
    }

    FnAnimation WhiteAnimation = FnAnimation( []( SetLedFn& set_led, GetLedFn& get_led, const AnimationSettings& settings, uint32_t ms ) {
        for( int i = 0; i < settings.mCount; ++i )
        {
            set_led( i, 255, 255, 255 );
        }
    } );

    FnAnimation DaylightAnimation =
        FnAnimation( []( SetLedFn& set_led, GetLedFn& get_led, const AnimationSettings& settings, uint32_t ms ) {
            // compute sun color, etc.
            for( int i = 0; i < settings.mCount; ++i )
            {
                set_led( i, 255, 255, 255 );
            }
        } );
}
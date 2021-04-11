#include "animations.h"
#include <array>
#include <limits>
#include <stdlib.h>
namespace Animation
{
    Animation::Animation() = default;
    Animation::~Animation() = default;

    void Animation::Reset( const AnimationSettings& settings )
    {
        mSettings = settings;
    }

    namespace
    {
        int random( int max )
        {
            return rand() % max;
        }

        int random( int min, int max )
        {
            return ( rand() % ( max - min ) ) + min;
        }

        std::tuple<uint8_t, uint8_t, uint8_t> HsvToRgb( uint16_t h16, uint8_t s8, uint8_t v8 )
        {
            const float h = h16 / 65535. * 360.;
            const float s = s8 / 255.;
            const float v = v8 / 255.;

            float r = 0, g = 0, b = 0;

            float hh, p, q, t, ff;
            long i;

            hh = h;
            if( hh >= 360.0 )
                hh = 0.0;
            hh /= 60.0;
            i = ( long )hh;
            ff = hh - i;
            p = v * ( 1.0 - s );
            q = v * ( 1.0 - ( s * ff ) );
            t = v * ( 1.0 - ( s * ( 1.0 - ff ) ) );

            switch( i )
            {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;

            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            case 5:
            default:
                r = v;
                g = p;
                b = q;
                break;
            }
            return std::make_tuple( static_cast<uint8_t>( r * 255 ), static_cast<uint8_t>( g * 255 ), static_cast<uint8_t>( b * 255 ) );
        }

        std::tuple<uint16_t, uint8_t, uint8_t> RgbToHsv( uint8_t r8, uint8_t g8, uint8_t b8 )
        {
            const float r = r8 / 255.;
            const float g = g8 / 255.;
            const float b = b8 / 255.;

            float h = 0;
            float s = 0;
            float v = 0;

            double min, max, delta;

            min = r < g ? r : g;
            min = min < b ? min : b;

            max = r > g ? r : g;
            max = max > b ? max : b;

            v = max; // v
            delta = max - min;
            if( delta < 0.00001 )
            {
                return {};
            }
            if( max > 0.0 )
            {                        // NOTE: if Max is == 0, this divide would cause a crash
                s = ( delta / max ); // s
            }
            else
            {
                return {};
            }
            if( r >= max )             // > is bogus, just keeps compilor happy
                h = ( g - b ) / delta; // between yellow & magenta
            else if( g >= max )
                h = 2.0 + ( b - r ) / delta; // between cyan & yellow
            else
                h = 4.0 + ( r - g ) / delta; // between magenta & cyan

            h *= 60.0; // degrees

            if( h < 0.0 )
                h += 360.0;

            return std::make_tuple( static_cast<uint16_t>( h / 360.0 * 65535. ), static_cast<uint8_t>( s * 255 ),
                                    static_cast<uint8_t>( v * 255 ) );
        }
        class FnAnimation : public Animation
        {
          public:
            using AnimationFn = std::function<void( SetLedFn&, GetLedFn&, const AnimationSettings&, uint32_t )>;
            FnAnimation( AnimationFn fn ) : mFn( std::move( fn ) )
            {
            }
            ~FnAnimation() override = default;
            void Reset( const AnimationSettings& settings ) override
            {
                Animation::Reset( settings );
            }
            void Render( SetLedFn set_led, GetLedFn get_led, uint32_t ms ) override
            {
                mFn( set_led, get_led, mSettings, ms );
            }

          protected:
            AnimationFn mFn;
        };

        FnAnimation WhiteAnimation =
            FnAnimation( []( SetLedFn& set_led, GetLedFn& get_led, const AnimationSettings& settings, uint32_t ms ) {
                for( int i = 0; i < settings.mCount; ++i )
                {
                    set_led( i, 255, 255, 255 );
                }
            } );

        FnAnimation StripIdAnimation =
            FnAnimation( []( SetLedFn& set_led, GetLedFn& get_led, const AnimationSettings& settings, uint32_t ms ) {
                for( int i = 0; i < settings.mCount; ++i )
                {
                    if( i < settings.mIndex * 10 + 5 )
                    {
                        set_led( i, 255, 255, 255 );
                    }
                    else
                    {
                        set_led( i, 0, 0, 0 );
                    }
                }
            } );

        FnAnimation CircleRainbowAnimation =
            FnAnimation( []( SetLedFn& set_led, GetLedFn& get_led, const AnimationSettings& settings, uint32_t ms ) {
                float start = ( ( ( ms % 7000 ) / 7000. ) + ( settings.mIndex / 2.0 ) ) * std::numeric_limits<uint16_t>::max();
                const float step =
                    ( static_cast<float>( std::numeric_limits<uint16_t>::max() ) / static_cast<float>( settings.mCount ) ) / 2.0;
                for( int i = 0; i < settings.mCount; ++i )
                {
                    start += step;

                    while( start > std::numeric_limits<uint16_t>::max() )
                    {
                        start = start - std::numeric_limits<uint16_t>::max();
                    }

                    auto rgb = HsvToRgb( static_cast<uint16_t>( start ), 255, 255 );
                    set_led( i, std::get<0>( rgb ), std::get<1>( rgb ), std::get<2>( rgb ) );
                }
            } );

        FnAnimation SparkleAnimation =
            FnAnimation( []( SetLedFn& set_led, GetLedFn& get_led, const AnimationSettings& settings, uint32_t ms ) {
                // TODO: lets get way more sparkles, but make more of them dim.

                // each pixel has a random change of sparkling.
                // lets say that we want about one third of the strip to sparkle each
                // second on average.
                // every second, we want N new stars.
                // odds = LED_COUNT * fps / stars per sec
                constexpr int odds = ( 30 * 600 ) / ( 180 );

                for( int i = 0; i < settings.mCount; i++ )
                {
                    if( random( odds ) == 0 )
                    {
                        // sparkle!
                        float hue = random( 100 ) / 100.0;            // random color
                        float brightness = random( 20, 100 ) / 100.0; // all are at least 50% brightness.
                        brightness = brightness * brightness * brightness * brightness * brightness * brightness;
                        float saturation = random( 10, 100 ) / 100.0; // some color?
                        auto color = HsvToRgb( static_cast<uint16_t>( hue * std::numeric_limits<uint16_t>::max() ),
                                               static_cast<uint8_t>( saturation * 255 ), static_cast<uint8_t>( brightness * 255 ) );
                        set_led( i, std::get<0>( color ), std::get<1>( color ), std::get<2>( color ) );
                        if( brightness > 0.65 )
                        {
                            // lets glow the neighbors.
                            color = HsvToRgb( static_cast<uint16_t>( hue * std::numeric_limits<uint16_t>::max() ),
                                              static_cast<uint8_t>( saturation * 255 ), static_cast<uint8_t>( brightness * 255 / 4 ) );
                            if( i >= 1 )
                            {
                                set_led( i - 1, std::get<0>( color ), std::get<1>( color ), std::get<2>( color ) );
                            }
                            if( i < ( settings.mCount - 1 ) )
                            {
                                set_led( i + 1, std::get<0>( color ), std::get<1>( color ), std::get<2>( color ) );
                            }
                        }
                    }
                    else
                    {
                        // decay.
                        uint8_t r, g, b;
                        get_led( i, r, g, b );
                        auto hsv = RgbToHsv( r, g, b );
                        if( std::get<2>( hsv ) > 0.0 )
                        {
                            // decay over 5 seconds, from 255 to 0, at 30 hz. 30*5 =150 frames,
                            std::get<2>( hsv ) = std::max( 0, std::get<2>( hsv ) - 3 );
                        }
                        auto rgb = HsvToRgb( std::get<0>( hsv ), std::get<1>( hsv ), std::get<2>( hsv ) );
                        set_led( i, std::get<0>( rgb ), std::get<1>( rgb ), std::get<2>( rgb ) );
                    }
                }
            } );

        FnAnimation KaleidoscopeAnimation =
            FnAnimation( []( SetLedFn& set_led, GetLedFn& get_led, const AnimationSettings& settings, uint32_t ms ) {
                float start = ( ( ms % 5000 ) / 5000. ) * std::numeric_limits<uint16_t>::max();
                const float step = static_cast<float>( std::numeric_limits<uint16_t>::max() ) / static_cast<float>( settings.mCount ) * 1.5;
                for( int i = 0; i < settings.mCount; ++i )
                {
                    start += step;

                    while( start > std::numeric_limits<uint16_t>::max() )
                    {
                        start = start - std::numeric_limits<uint16_t>::max();
                    }

                    auto rgb = HsvToRgb( static_cast<uint16_t>( start ), 255, 255 );
                    auto i_adjusted = settings.mIndex % 2 == 1 ? i : settings.mCount - i - 1;
                    set_led( i_adjusted, std::get<0>( rgb ), std::get<1>( rgb ), std::get<2>( rgb ) );
                }
            } );


        struct GradientPoint
        {
            uint8_t mR;
            uint8_t mG;
            uint8_t mB;
            float mBegin;
            float mEnd;
        };

        // TODO: adjust this based on date to match sunrise / sunset.
        // Generated using this tool & excel: https://color.adobe.com/create/image-gradient
        const GradientPoint SkyPoints[] = { { 255, 255, 255, 0, 0 },
                                            { 221, 244, 254, 0.0801, 0.13123702732 },
                                            { 141, 221, 254, 0.1823738904, 0.1966619492 },
                                            { 99, 204, 245, 0.2109508292, 0.2282469436 },
                                            { 56, 163, 209, 0.245543058, 0.2643436108 },
                                            { 35, 106, 163, 0.2831441636, 0.2959286052 },
                                            { 59, 102, 136, 0.3087122256, 0.3275127784 },
                                            { 170, 171, 112, 0.3463133312, 0.3598491708 },
                                            { 209, 118, 40, 0.3733858316, 0.3876738904 },
                                            { 106, 41, 9, 0.4019619492, 0.4342991628 },
                                            { 0, 0, 0, 0.4666355552, 0.6035024956 },
                                            { 73, 73, 105, 0.7403686148, 0.7524008372 },
                                            { 97, 97, 147, 0.7644330596, 0.7787219396 },
                                            { 180, 124, 162, 0.7930099984, 0.8125619492 },
                                            { 246, 147, 143, 0.8321147212, 0.86670695 },
                                            { 237, 247, 247, 0.9013, 0.9013 },
                                            { 255, 255, 255, 1, 1 } };
        const int SkyPointCount = sizeof( SkyPoints ) / sizeof( GradientPoint );

        template <typename T>
        T Interpolate( T begin, T end, float fraction )
        {
            return static_cast<T>( ( begin - end ) * fraction + begin );
        }

        std::tuple<uint8_t, uint8_t, uint8_t> Interpolate( const std::tuple<uint8_t, uint8_t, uint8_t> begin,
                                                           const std::tuple<uint8_t, uint8_t, uint8_t> end, float fraction )
        {
            // let's linearly interpolate the HSV version.
            const auto begin_hsv = RgbToHsv( std::get<0>( begin ), std::get<1>( begin ), std::get<2>( begin ) );
            const auto end_hsv = RgbToHsv( std::get<0>( end ), std::get<1>( end ), std::get<2>( end ) );
            return HsvToRgb( Interpolate( std::get<0>( begin_hsv ), std::get<0>( end_hsv ), fraction ),
                             Interpolate( std::get<1>( begin_hsv ), std::get<1>( end_hsv ), fraction ),
                             Interpolate( std::get<2>( begin_hsv ), std::get<2>( end_hsv ), fraction ) );
        }

        std::tuple<uint8_t, uint8_t, uint8_t> RenderGradient( const GradientPoint* gradient, int gradient_size, float position )
        {
            for( int i = 0; i < gradient_size - 1; ++i )
            {
                const auto& current = gradient[ i ];
                const auto& next = gradient[ i + 1 ];
                if( position <= current.mEnd )
                    return std::make_tuple( current.mR, current.mG, current.mB );
                if( position < next.mBegin )
                {
                    const auto start = current.mEnd;
                    const auto end = next.mBegin;
                    const auto fraction = ( position - start ) / ( start - end );
                    return Interpolate( std::make_tuple( current.mR, current.mG, current.mB ), std::make_tuple( next.mR, next.mG, next.mB ),
                                        fraction );
                }
            }
            const auto& last = gradient[ gradient_size - 1 ];
            return std::make_tuple( last.mR, last.mG, last.mB );
        }


        FnAnimation SkyAnimation = FnAnimation( []( SetLedFn& set_led, GetLedFn& get_led, const AnimationSettings& settings, uint32_t ms ) {
            // the sky gradient goes from noon to noon.
            // convert to fraction of day.
            float time = static_cast<float>( ms / 1000 ) / ( 60 * 60 * 24 );
            // shift 50%.
            time += 0.5;
            if( time > 1 )
                time -= 1;
            if( time < 0 )
                time = 0;
            else if( time > 1 )
                time = 1;

            const auto color = RenderGradient( SkyPoints, SkyPointCount, time );

            for( int i = 0; i < settings.mCount; ++i )
            {
                set_led( i, std::get<0>( color ), std::get<1>( color ), std::get<2>( color ) );
            }
        } );

        std::array<Animation*, 6> AllAnimations = { &WhiteAnimation,   &StripIdAnimation,      &CircleRainbowAnimation,
                                                    &SparkleAnimation, &KaleidoscopeAnimation, &SkyAnimation };
    }

    int AnimationCount()
    {
        return AllAnimations.size();
    }
    Animation* GetAnimation( int animation_index )
    {
        return AllAnimations.at( animation_index );
    }
}
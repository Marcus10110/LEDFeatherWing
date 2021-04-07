#include "led.h"
#include <Adafruit_NeoPixel.h>
#include "settings.h"
#include <functional>

#define LED_PIN_A 14
#define LED_PIN_B 32
#define LED_PIN_C 15
#define LED_PIN_D 33
#define LED_COUNT 160 // TODO: support runtime varied strip length.

Adafruit_NeoPixel StripA = Adafruit_NeoPixel( LED_COUNT, LED_PIN_A, NEO_GBR + NEO_KHZ800 );
Adafruit_NeoPixel StripB = Adafruit_NeoPixel( LED_COUNT, LED_PIN_B, NEO_GBR + NEO_KHZ800 );
Adafruit_NeoPixel StripC = Adafruit_NeoPixel( LED_COUNT, LED_PIN_C, NEO_GBR + NEO_KHZ800 );
Adafruit_NeoPixel StripD = Adafruit_NeoPixel( LED_COUNT, LED_PIN_D, NEO_GBR + NEO_KHZ800 );
Adafruit_NeoPixel* Strips[ 4 ] = { &StripA, &StripB, &StripC, &StripD };
bool StripCleared[ 4 ] = { false, false, false, false };
namespace Led
{
    using LedFn = std::function<void()>;
    namespace
    {
        void Animate( Adafruit_NeoPixel& strip, const Settings::StripSettings& setting, uint8_t brightness, uint8_t animation_index,
                      uint32_t ms )
        {
            strip.setBrightness( brightness );
            float start = ( ms % 5000 ) / 5000. * 65535.0;
            const float step = ( 65535 / LED_COUNT );
            for( int i = 0; i < setting.mLength; ++i )
            {
                start += step;
                uint32_t hue_32 = static_cast<uint32_t>( start );
                uint16_t hue = static_cast<uint16_t>( hue_32 );
                strip.setPixelColor( i, Adafruit_NeoPixel::ColorHSV( hue ) );
            }
        }
    }
    void Setup()
    {
        StripA.begin();
        StripA.show();
        StripB.begin();
        StripB.show();
        StripC.begin();
        StripC.show();
        StripD.begin();
        StripD.show();
    }

    void Update( uint32_t ms )
    {
        const auto& settings = Settings::Settings;
        for( int i = 0; i < 4; ++i )
        {
            const auto& setting = settings.mStrips[ i ];
            auto& strip = *Strips[ i ];
            if( !setting.mEnable )
            {
                strip.clear();
                continue;
            }
            Animate( strip, setting, settings.mBrightness, settings.mAnimationIndex, ms );
        }
        for( int i = 0; i < 4; ++i )
        {
            const auto& setting = settings.mStrips[ i ];
            auto& strip = *Strips[ i ];
            if( setting.mEnable || !StripCleared[ i ] )
            {
                strip.show();
                if( !setting.mEnable )
                {
                    StripCleared[ i ] = true;
                }
            }
        }
    }

    void Test()
    {
        for( auto strip : Strips )
        {
            for( int i = 0; i < LED_COUNT; ++i )
            {
                strip->setPixelColor( i, 10, 20, 30 );
            }
            strip->show();
        }
    }
}
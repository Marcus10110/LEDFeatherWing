#include "led.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN_A 14
#define LED_PIN_B 32
#define LED_PIN_C 15
#define LED_PIN_D 33
#define LED_COUNT 150

Adafruit_NeoPixel StripA = Adafruit_NeoPixel( LED_COUNT, LED_PIN_A, NEO_GBR + NEO_KHZ400 );
Adafruit_NeoPixel StripB = Adafruit_NeoPixel( LED_COUNT, LED_PIN_B, NEO_GBR + NEO_KHZ400 );
Adafruit_NeoPixel StripC = Adafruit_NeoPixel( LED_COUNT, LED_PIN_C, NEO_GBR + NEO_KHZ400 );
Adafruit_NeoPixel StripD = Adafruit_NeoPixel( LED_COUNT, LED_PIN_D, NEO_GBR + NEO_KHZ400 );
Adafruit_NeoPixel* Strips[ 4 ] = { &StripA, &StripB, &StripC, &StripD };

namespace Led
{
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
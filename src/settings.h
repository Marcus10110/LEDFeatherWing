#pragma once
#include <Arduino.h>
#include <functional>
#include <ArduinoJson.h>

#define STRIP_COUNT 4
// TODO: come up with better namespaces

namespace Settings
{
    struct StripSettings
    {
        bool mEnable;
        bool mInvert;
        uint16_t mLength;
        uint8_t mIndex;
    };
    struct FeatherSettings
    {
        String mDeviceName;
        int16_t mTimezoneOffsetMinutes;
        bool mUseDst;
        uint8_t mBrightness;
        uint8_t mAnimationIndex;
        StripSettings mStrips[ STRIP_COUNT ];
    };

    extern FeatherSettings Settings;

    void SerializeSettings( const FeatherSettings& settings, String& output, std::function<void( JsonDocument* )> extra = nullptr );

    void SerializeSharedSettings( uint8_t brightness, uint8_t animation_index, String& output );

    bool DeserializeSettings( FeatherSettings& settings, String& input );

    bool DeserializePartialSettings( FeatherSettings& settings, char* input, int length );

    bool SaveToFlash( const FeatherSettings& settings );
    bool LoadFromFlash( FeatherSettings& settings );
    void LoadDefault( FeatherSettings& settings );

    void InitializeSettings();

    void PrintSettings();
}
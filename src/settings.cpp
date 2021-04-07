
#include "settings.h"
#include <EEPROM.h>
#include "utilities.h"

#define EEPROM_VERSION 2
// I think this could go up to near 4K.
#define EEPROM_SIZE 1024
// recommended from https://arduinojson.org/v6/assistant/, because we need to support loading from a String.
#define JSON_BUFFER_SIZE 768

namespace Settings
{
    FeatherSettings Settings;
    void SerializeSettings( const FeatherSettings& settings, String& output, std::function<void( JsonDocument* )> extra )
    {
        PROFILE_FUNC;
        StaticJsonDocument<JSON_BUFFER_SIZE> doc;

        doc[ "mDeviceName" ] = settings.mDeviceName;
        doc[ "mTimezoneOffsetMinutes" ] = settings.mTimezoneOffsetMinutes;
        doc[ "mUseDst" ] = settings.mUseDst;
        doc[ "mBrightness" ] = settings.mBrightness;
        doc[ "mAnimationIndex" ] = settings.mAnimationIndex;

        JsonArray strips = doc.createNestedArray( "strips" );

        for( int i = 0; i < STRIP_COUNT; ++i )
        {
            JsonObject strip = strips.createNestedObject();
            strip[ "mEnable" ] = settings.mStrips[ i ].mEnable;
            strip[ "mInvert" ] = settings.mStrips[ i ].mInvert;
            strip[ "mLength" ] = settings.mStrips[ i ].mLength;
            strip[ "mIndex" ] = settings.mStrips[ i ].mIndex;
        }

        if( extra )
        {
            extra( &doc );
        }

        serializeJson( doc, output );
    }

    void SerializeSharedSettings( uint8_t brightness, uint8_t animation_index, String& output )
    {
        PROFILE_FUNC;
        StaticJsonDocument<JSON_BUFFER_SIZE> doc;
        doc[ "mBrightness" ] = brightness;
        doc[ "mAnimationIndex" ] = animation_index;
        serializeJson( doc, output );
    }

    bool DeserializeSettings( FeatherSettings& settings, String& input )
    {
        PROFILE_FUNC;
        StaticJsonDocument<JSON_BUFFER_SIZE> doc;

        DeserializationError error = deserializeJson( doc, input );

        if( error )
        {
            Serial.print( F( "deserializeJson() failed: " ) );
            Serial.println( error.f_str() );
            return false;
        }

        settings.mDeviceName = doc[ "mDeviceName" ].as<String>();          // "LED Feather Wing 1"
        settings.mTimezoneOffsetMinutes = doc[ "mTimezoneOffsetMinutes" ]; // -480
        settings.mUseDst = doc[ "mUseDst" ];                               // true
        settings.mBrightness = doc[ "mBrightness" ];                       // 255
        settings.mAnimationIndex = doc[ "mAnimationIndex" ];

        int strip = 0;
        for( JsonObject elem : doc[ "strips" ].as<JsonArray>() )
        {
            settings.mStrips[ strip ].mEnable = elem[ "mEnable" ]; // true, true, true, true
            settings.mStrips[ strip ].mInvert = elem[ "mInvert" ]; // false, false, false, false
            settings.mStrips[ strip ].mLength = elem[ "mLength" ]; // 150, 150, 150, 150
            settings.mStrips[ strip ].mIndex = elem[ "mIndex" ];   // 0, 0, 0, 0
            strip++;
            if( strip >= STRIP_COUNT )
            {
                break;
            }
        }

        return true;
    }

    bool DeserializePartialSettings( FeatherSettings& settings, char* input, int length )
    {
        PROFILE_FUNC;
        StaticJsonDocument<JSON_BUFFER_SIZE> doc;

        DeserializationError error = deserializeJson( doc, input, length );

        if( error )
        {
            Serial.print( F( "deserializeJson() failed: " ) );
            Serial.println( error.f_str() );
            return false;
        }

        if( doc.containsKey( "mDeviceName" ) )
            settings.mDeviceName = doc[ "mDeviceName" ].as<String>(); // "LED Feather Wing 1"
        if( doc.containsKey( "mTimezoneOffsetMinutes" ) )
            settings.mTimezoneOffsetMinutes = doc[ "mTimezoneOffsetMinutes" ]; // -480
        if( doc.containsKey( "mUseDst" ) )
            settings.mUseDst = doc[ "mUseDst" ]; // true
        if( doc.containsKey( "mBrightness" ) )
            settings.mBrightness = doc[ "mBrightness" ]; // 255
        if( doc.containsKey( "mAnimationIndex" ) )
            settings.mAnimationIndex = doc[ "mAnimationIndex" ];

        if( doc.containsKey( "strips" ) )
        {
            int strip = 0;
            for( JsonObject elem : doc[ "strips" ].as<JsonArray>() )
            {
                settings.mStrips[ strip ].mEnable = elem[ "mEnable" ]; // true, true, true, true
                settings.mStrips[ strip ].mInvert = elem[ "mInvert" ]; // false, false, false, false
                settings.mStrips[ strip ].mLength = elem[ "mLength" ]; // 150, 150, 150, 150
                settings.mStrips[ strip ].mIndex = elem[ "mIndex" ];   // 0, 0, 0, 0
                strip++;
                if( strip >= STRIP_COUNT )
                {
                    break;
                }
            }
        }

        return true;
    }

    bool SaveToFlash( const FeatherSettings& settings )
    {
        PROFILE_FUNC;
        // EEPROM layout:
        // [0] = M (Z indicates write failure due to string length too long)
        // [1] = G
        // [2] = version. (always dump & default on version upgrade)
        // [3+] = null terminated content.

        String serialized_settings;
        SerializeSettings( settings, serialized_settings );
        if( !EEPROM.begin( EEPROM_SIZE ) )
        {
            Serial.println( "failed to open EEPROM" );
            return false;
        }
        EEPROM.writeChar( 0, 'M' );
        EEPROM.writeChar( 1, 'G' );
        EEPROM.write( 2, EEPROM_VERSION );
        bool success = true;
        if( EEPROM.writeString( 3, serialized_settings ) == 0 )
        {
            success = false;
            Serial.println( "serialized settings failed to write to EEPROM:" );
            Serial.println( serialized_settings.length() );
            EEPROM.writeChar( 0, 'Z' );
        }
        EEPROM.commit();
        EEPROM.end();
        return success;
    }
    bool LoadFromFlash( FeatherSettings& settings )
    {
        if( !EEPROM.begin( EEPROM_SIZE ) )
        {
            Serial.println( "failed to open EEPROM" );
            return false;
        }
        bool success = false;
        byte header[ 3 ];
        EEPROM.readBytes( 0, header, 3 );
        if( header[ 0 ] == 'M' && header[ 1 ] == 'G' && header[ 2 ] == EEPROM_VERSION )
        {
            String serialized_settings = EEPROM.readString( 3 );
            if( serialized_settings.length() == 0 )
            {
                return false;
            }
            success = DeserializeSettings( Settings, serialized_settings );
        }
        else if( header[ 0 ] == 'M' && header[ 1 ] == 'G' )
        {
            Serial.println( "EEPROM version out of date." );
        }

        EEPROM.end();
        return success;
    }
    void LoadDefault( FeatherSettings& settings )
    {
        uint8_t random_number = random( 10 );
        settings.mDeviceName = "LED Feather Wing ";
        settings.mDeviceName += random_number;

        settings.mTimezoneOffsetMinutes = -420;
        settings.mUseDst = true;
        settings.mBrightness = 100;
        settings.mAnimationIndex = 0;

        for( int i = 0; i < STRIP_COUNT; ++i )
        {
            auto& strip = settings.mStrips[ i ];
            strip.mEnable = i < 2;
            strip.mInvert = false;
            strip.mIndex = i;
            strip.mLength = 150;
        }
    }

    void InitializeSettings()
    {
        if( !LoadFromFlash( Settings ) )
        {
            Serial.println( "failed to load settings from EEPROM. Applying defaults..." );
            LoadDefault( Settings );
            if( !SaveToFlash( Settings ) )
            {
                Serial.println( "failed to write default settings to flash" );
                while( true )
                {
                    Serial.print( "." );
                    delay( 1000 );
                }
            }
        }
    }

    void PrintSettings()
    {
        String serialized_settings;
        SerializeSettings( Settings, serialized_settings );
        Serial.println( "Settings:" );
        Serial.println( serialized_settings );
    }
}
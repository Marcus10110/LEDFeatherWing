
#include <WiFi.h>
#include <ESPmDNS.h>
#include "src\wifi_config.h"
#include "src\led.h"
#include "src\web_server.h"
#include "src\settings.h"
#include "src\wall_time.h"
#include "src\sync.h"

#ifdef OTA_PASS
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ota_password = OTA_PASS;
#endif


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

bool SettingsBroadcastNeeded = false;
bool DiscoveryNeeded = false;

void setup()
{
    Serial.begin( 115200 );

    // Load or apply default settings.
    Settings::InitializeSettings();
    Settings::PrintSettings();

    // connect to WiFi
    Serial.printf( "Connecting to %s ", ssid );
    WiFi.begin( ssid, password );
    auto connection_timeout = millis() + 1000 * 10;
    while( WiFi.status() != WL_CONNECTED )
    {
        delay( 500 );
        Serial.print( "." );
        if( millis() >= connection_timeout )
        {
            Serial.println( "\nConnection timeout. Rebooting..." );
            ESP.restart();
        }
    }
    Serial.println( " CONNECTED to wifi" );

#ifdef OTA_PASS

    // mDNS is manually enabled, don't have ArduinoOTA try to start it for is.
    ArduinoOTA.setMdnsEnabled( false );
    ArduinoOTA.setPassword( ota_password );

    ArduinoOTA
        .onStart( []() {
            String type;
            if( ArduinoOTA.getCommand() == U_FLASH )
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println( "Start updating " + type );
        } )
        .onEnd( []() { Serial.println( "\nEnd" ); } )
        .onProgress(
            []( unsigned int progress, unsigned int total ) { Serial.printf( "Progress: %u%%\r", ( progress / ( total / 100 ) ) ); } )
        .onError( []( ota_error_t error ) {
            Serial.printf( "Error[%u]: ", error );
            if( error == OTA_AUTH_ERROR )
                Serial.println( "Auth Failed" );
            else if( error == OTA_BEGIN_ERROR )
                Serial.println( "Begin Failed" );
            else if( error == OTA_CONNECT_ERROR )
                Serial.println( "Connect Failed" );
            else if( error == OTA_RECEIVE_ERROR )
                Serial.println( "Receive Failed" );
            else if( error == OTA_END_ERROR )
                Serial.println( "End Failed" );
        } );

    ArduinoOTA.begin();
    Serial.println( "OTA service started" );
#endif

    // Time
    WallTime::Setup();
    WallTime::printLocalTime();

    AdvertiseServices();
    WebServer::Start( []() { SettingsBroadcastNeeded = true; }, []() { DiscoveryNeeded = true; } );
    Led::Setup();
    uint8_t existing_brightness, existing_animation_index;
    Sync::RefreshNeighbors();
    if( Sync::DiscoverExistingSettings( existing_brightness, existing_animation_index ) )
    {
        Serial.println( "discovered existing device, loading settings!" );
        Serial.print( existing_brightness );
        Serial.print( ", " );
        Serial.println( existing_animation_index );
        Settings::Settings.mAnimationIndex = existing_animation_index;
        Settings::Settings.mBrightness = existing_brightness;
        if( !Settings::SaveToFlash( Settings::Settings ) )
        {
            Serial.println( "failed to save to flash" );
        }
    }
    else
    {
        Serial.println( "no existing devices found, using stored settings" );
    }
}

void loop()
{
    static uint32_t last = 0;
    auto ms = WallTime::Ms();
    if( ms > ( last + 10000 ) )
    {
        last = ms;
        WallTime::printLocalTime();
    }
    Led::Update( ms );
    if( SettingsBroadcastNeeded )
    {
        SettingsBroadcastNeeded = false;
        if( Sync::BroadcastSettings( Settings::Settings.mBrightness, Settings::Settings.mAnimationIndex ) )
        {
            Serial.println( "sent settings to other device(s)!" );
        }
        else
        {
            Serial.println( "no devices to sync with" );
        }
    }

    if( DiscoveryNeeded )
    {
        DiscoveryNeeded = false;
        Sync::RefreshNeighbors();
    }

#ifdef OTA_PASS
    ArduinoOTA.handle();
#endif
}


String Url()
{
    String url = Settings::Settings.mDeviceName;
    url.replace( ' ', '-' );
    url.toLowerCase();
    return url;
}

void AdvertiseServices()
{
    const auto url = Url();
    if( MDNS.begin( url.c_str() ) )
    {
        Serial.println( F( "mDNS responder started" ) );
        Serial.print( F( "URL: http://" ) );
        Serial.print( url.c_str() );
        Serial.println( ".local" );
        MDNS.addService( "LEDFeatherWing", "tcp", 80 );
#ifdef OTA_PASS
        MDNS.enableArduino( 3232, true );
#endif
    }
    else
    {
        while( 1 )
        {
            Serial.println( F( "Error setting up MDNS responder" ) );
            delay( 1000 );
        }
    }
}

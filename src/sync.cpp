#include "sync.h"
#include <Arduino.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include "settings.h"
#include "utilities.h"
#include "web_server.h"

#define MAX_NEIGHBORS 10
namespace
{
    Sync::Neighbor Neighbors[ MAX_NEIGHBORS ];
    int NeighborCount = 0;
}

namespace Sync
{
    Neighbor* GetNeighbors( int* count )
    {
        *count = NeighborCount;
        return Neighbors;
    }

    void RefreshNeighbors()
    {
        NeighborCount = 0;
        // Note: this blocks for 3-5 seconds.
        auto count = MDNS.queryService( "LEDFeatherWing", "tcp" );
        Serial.print( "discovered services: " );
        Serial.println( count );
        for( int i = 0; i < count && i < MAX_NEIGHBORS; ++i )
        {
            Neighbors[ i ].mHostName = MDNS.hostname( i );
            Neighbors[ i ].mIp = MDNS.IP( i );
        }
        NeighborCount = count;
    }

    bool DiscoverExistingSettings( uint8_t& brightness, uint8_t& animation_index )
    {
        if( NeighborCount == 0 )
        {
            return false;
        }
        auto& neightbor = Neighbors[ 0 ];
        auto& ip = neightbor.mIp;
        HTTPClient http;
        String path = "http://";
        path.concat( ip.toString() );
        path.concat( "/settings" );
        Serial.print( "request url: " );
        Serial.println( path );
        http.begin( path.c_str() );
        int httpResponseCode = http.GET();
        char content[ 1024 ];
        int content_length = 0;
        if( httpResponseCode > 0 )
        {
            String payload = http.getString();
            payload.toCharArray( content, 1024 );
            content_length = min( payload.length(), 1024u );
        }
        // Free resources
        http.end();

        if( content_length == 0 )
        {
            Serial.print( "other device found, but request failed with" );
            Serial.println( httpResponseCode );
            return false;
        }
        Settings::FeatherSettings loaded_settings;
        if( Settings::DeserializePartialSettings( loaded_settings, content, content_length ) )
        {
            brightness = loaded_settings.mBrightness;
            animation_index = loaded_settings.mAnimationIndex;
            return true;
        }
        else
        {
            Serial.println( "failed to deserialize settings object" );
            return false;
        }
        return true;
    }

    bool BroadcastSettings( uint8_t brightness, uint8_t animation_index )
    {
        PROFILE_FUNC;
        // use service discovery to find every device.
        // create json HTTP post body, and send POST request to every device.
        // return false if no devices found.
        String request_data;
        Settings::SerializeSharedSettings( brightness, animation_index, request_data );

        for( int i = 0; i < NeighborCount; ++i )
        {
            Utilities::Profile( "http request" );
            auto& ip = Neighbors[ i ].mIp;
            HTTPClient http;
            String path = "http://";
            path.concat( ip.toString() );
            path.concat( "/setSettings" );
            Serial.print( "request url: " );
            Serial.println( path );
            http.begin( path.c_str() );
            // Specify content-type header
            http.addHeader( "Content-Type", "application/json; utf-8" );
            http.addHeader( WebServer::BroadcastHeaderName, "true" );
            // Send HTTP POST request
            int httpResponseCode = http.POST( request_data );
            http.end();
        }


        return true;
    }
}
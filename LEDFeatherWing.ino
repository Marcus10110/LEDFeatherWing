#include <WiFi.h>
#include "time.h"
#include "src\wifi_config.h"
#include "src\led.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8 * 60 * 60;
const int daylightOffset_sec = 3600;

bool IsDaylightSaving = false;

void CheckDaylightSaving()
{
    struct tm timeinfo;
    if( !getLocalTime( &timeinfo ) )
    {
        Serial.println( "Failed to obtain time" );
        return;
    }
    IsDaylightSaving = timeinfo.tm_isdst > 0;
}

void printLocalTime()
{
    struct timeval tv;
    struct tm timeinfo;
    if( !getLocalTime( &timeinfo ) )
    {
        Serial.println( "Failed to obtain time" );
        return;
    }
    auto a = gettimeofday( &tv, nullptr );
    if( a != 0 )
    {
        Serial.println( "Failed to obtain gettimeofday" );
        Serial.println( a );
    }
    Serial.println( &timeinfo, "%A, %B %d %Y %H:%M:%S" );
    // Serial.println( tv.tv_sec );
    // Serial.println( tv.tv_usec );
    auto ms = Ms();
    Serial.println( ms );
}

// returns the number of miliseconds since midnight local time.
uint32_t Ms()
{
    timeval tv{ 0, 0 };
    timezone zone{ 0, 0 };
    if( gettimeofday( &tv, &zone ) != 0 )
    {
        Serial.println( "Failed to obtain gettimeofday" );
        return -1;
    }
    uint32_t time_temp = tv.tv_sec;
    time_temp += gmtOffset_sec;
    if( IsDaylightSaving )
    {
        time_temp += daylightOffset_sec;
    }
    time_temp = time_temp % ( 60 * 60 * 24 );
    time_temp *= 1000;
    time_temp += ( tv.tv_usec / 1000 );
    return time_temp;
}

void setup()
{
    Serial.begin( 115200 );

    // connect to WiFi
    Serial.printf( "Connecting to %s ", ssid );
    WiFi.begin( ssid, password );
    while( WiFi.status() != WL_CONNECTED )
    {
        delay( 500 );
        Serial.print( "." );
    }
    Serial.println( " CONNECTED to wifi" );

    // init and get the time
    configTime( gmtOffset_sec, daylightOffset_sec, ntpServer );
    struct tm timeinfo;
    while( !getLocalTime( &timeinfo ) )
    {
        Serial.println( "Failed to obtain time, retrying..." );
        delay( 100 );
    }
    printLocalTime();
    CheckDaylightSaving();
    // disconnect WiFi as it's no longer needed
    WiFi.disconnect( true );
    WiFi.mode( WIFI_OFF );
    Led::Setup();
    Led::Test();
}

void loop()
{
    delay( 1000 );
    printLocalTime();
}
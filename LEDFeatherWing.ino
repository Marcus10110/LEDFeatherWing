#include <WiFi.h>
#include <ESPmDNS.h>
#include "time.h"
#include "src\wifi_config.h"
#include "src\led.h"
#include "src\web_server.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8 * 60 * 60;
const int daylightOffset_sec = 3600;

bool IsDaylightSaving = false;

// TCP server at port 80 will respond to HTTP requests
WiFiServer server( 80 );

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
    AdvertiseServices( "LEDFeatherWing" );

    // server.begin();
    // Serial.println( "server started" );
    MDNS.addService( "http", "tcp", 80 );
    WebServer::Start();
    printLocalTime();
    CheckDaylightSaving();
    // disconnect WiFi as it's no longer needed
    // WiFi.disconnect( true );
    // WiFi.mode( WIFI_OFF );
    Led::Setup();
    Led::Test();
}

uint32_t last = 0;
void loop()
{
    if( Ms() > ( last + 1000 ) )
    {
        last = Ms();
        printLocalTime();
    }
}


void AdvertiseServices( const char* MyName )
{
    if( MDNS.begin( MyName ) )
    {
        Serial.println( F( "mDNS responder started" ) );
        Serial.print( F( "I am: " ) );
        Serial.println( MyName );

        // Add service to MDNS-SD
        MDNS.addService( "n8i-mlp", "tcp", 23 );
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

/*
void HandleHttp()
{
    // Check if a client has connected
    WiFiClient client = server.available();
    if( !client )
    {
        return;
    }
    Serial.println( "" );
    Serial.println( "New client" );

    uint32_t timeout = Ms() + 5000;
    // Wait for data from client to become available
    while( client.connected() && !client.available() )
    {
        delay( 1 );
        if( Ms() >= timeout )
        {
            client.stop();
            return;
        }
    }

    // Read the first line of HTTP request
    String req = client.readStringUntil( '\r' );

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf( ' ' );
    int addr_end = req.indexOf( ' ', addr_start + 1 );
    if( addr_start == -1 || addr_end == -1 )
    {
        Serial.print( "Invalid request: " );
        Serial.println( req );
        return;
    }
    req = req.substring( addr_start + 1, addr_end );
    Serial.print( "Request: " );
    Serial.println( req );

    String s;
    if( req == "/" )
    {
        IPAddress ip = WiFi.localIP();
        String ipStr = String( ip[ 0 ] ) + '.' + String( ip[ 1 ] ) + '.' + String( ip[ 2 ] ) + '.' + String( ip[ 3 ] );
        s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP32 at ";
        s += ipStr;
        s += "</html>\r\n\r\n";
        Serial.println( "Sending 200" );
    }
    else
    {
        s = "HTTP/1.1 404 Not Found\r\n\r\n";
        Serial.println( "Sending 404" );
    }
    client.print( s );

    client.stop();
    Serial.println( "Done with client" );
}*/
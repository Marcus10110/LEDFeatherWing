#include "wall_time.h"
#include "settings.h"
#include <sys/time.h>
namespace
{
    const char* ntpServer = "pool.ntp.org";
}

namespace WallTime
{
    void Setup()
    {
        // init and get the time
        configTime( GMTOffsetSec(), DstOffsetSec(), ntpServer );
        struct tm timeinfo;
        while( !getLocalTime( &timeinfo ) )
        {
            Serial.println( "Failed to obtain time, retrying..." );
            delay( 100 );
        }
    }
    long GMTOffsetSec()
    {
        return static_cast<int32_t>( Settings::Settings.mTimezoneOffsetMinutes ) * 60;
    }
    long DstOffsetSec()
    {
        return Settings::Settings.mUseDst ? 3600 : 0;
    }
    bool IsDst()
    {
        struct tm timeinfo;
        if( !getLocalTime( &timeinfo ) )
        {
            Serial.println( "Failed to obtain time" );
            return false;
        }
        return timeinfo.tm_isdst > 0;
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
    }

    uint32_t Ms()
    {
        // TODO: (optional) replace with a function that caches midnight, for faster reads.
        timeval tv{ 0, 0 };
        timezone zone{ 0, 0 };
        if( gettimeofday( &tv, &zone ) != 0 )
        {
            Serial.println( "Failed to obtain gettimeofday" );
            return -1;
        }
        uint32_t time_temp = tv.tv_sec;
        time_temp += GMTOffsetSec();
        if( IsDst() )
        {
            time_temp += DstOffsetSec();
        }
        time_temp = time_temp % ( 60 * 60 * 24 );
        time_temp *= 1000;
        time_temp += ( tv.tv_usec / 1000 );
        return time_temp;
    }
}
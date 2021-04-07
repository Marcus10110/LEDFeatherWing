#pragma once
#include <Arduino.h>

namespace WallTime
{
    // must be called after WIFI connected and settings are initialized.
    // Note: changing time zone or DST setting requires restart.
    void Setup();

    // signed offset of current time zone to UTC in seconds. Comes from Settings.
    long GMTOffsetSec();
    // fixed offset, either +3600 or 0.
    long DstOffsetSec();
    // checks local time to see if daylight savings is in effect. Requires Settings DST to be true.
    bool IsDst();
    // print date & time to serial port
    void printLocalTime();
    // number of miliseconds since midnight, local time. Resets to zero at midnight.
    uint32_t Ms();
}
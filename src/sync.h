#pragma once
#include <cstdint>
#include <Arduino.h>

namespace Sync
{
    struct Neighbor
    {
        String mHostName;
        IPAddress mIp;
    };


    Neighbor* GetNeighbors( int* count );
    void RefreshNeighbors();
    bool DiscoverExistingSettings( uint8_t& brightness, uint8_t& animation_index );
    bool BroadcastSettings( uint8_t brightness, uint8_t animation_index );
}
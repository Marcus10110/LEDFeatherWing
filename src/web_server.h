#pragma once
#include <functional>
namespace WebServer
{
    extern const char* BroadcastHeaderName;
    void Start( std::function<void()> queue_broadcast, std::function<void()> queue_discovery );
    void Stop();
}
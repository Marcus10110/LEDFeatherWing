#pragma once
#include <Arduino.h>


#define PROFILE_FUNC Utilities::Profile prof( __func__ );

namespace Utilities
{
    class Profile
    {
      public:
        Profile( const char* label );
        ~Profile();

      private:
        uint32_t mStartTimeMs;
        String mLabel;
    };
}
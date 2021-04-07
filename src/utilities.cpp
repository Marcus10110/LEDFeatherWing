#include "utilities.h"

namespace Utilities
{
    Profile::Profile( const char* label ) : mStartTimeMs( millis() ), mLabel( label )
    {
    }
    Profile::~Profile()
    {
        auto ms = millis() - mStartTimeMs;
        Serial.print( mLabel );
        Serial.print( " took " );
        Serial.print( ms );
        Serial.println( " ms" );
    }
}
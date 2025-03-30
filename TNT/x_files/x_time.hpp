////////////////////////////////////////////////////////////////////////////
//
//  X_TIME.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_TIME_HPP
#define X_TIME_HPP

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////
//
//  x_GetTime
//
//      Retrieve time in seconds.  The value returned may or may not
//      represent the current time.  (Some systems may not have a real time
//      clock.  Or the time could be set wrong.)  The values returned should
//      only be used for timing intervals by taking the difference between
//      two samples.  Note that one time value retrieved after another is NOT
//      guaranteed to be greater than the first, use x_GetTimeDiff or
//      x_GetTimerTicksDiff to determine the difference between two samples.
//
//
//  x_GetElapsedTime and x_GetElapsedTimerTicks
//
//      Retrieves the number of seconds(or ticks) that have passed since
//      TimeStart.  Equivalent to x_GetTimeDiff( TimeStart, x_GetTime() ).
//
////////////////////////////////////////////////////////////////////////////

f64  x_GetTime              ( void );
s64  x_GetTimerTicks        ( void );

s64  x_GetTimerTicksPerSec  ( void );
f64  x_TicksToSecs          ( s64 NTicks );
f64  x_TicksToMs            ( s64 NTicks );

f64  x_GetTimeDiff          ( f64 TimeStart, f64 TimeEnd );
s64  x_GetTimerTicksDiff    ( s64 TickStart, s64 TickEnd );

f64  x_GetElapsedTime       ( f64 TimeStart );
s64  x_GetElapsedTimerTicks ( s64 TickStart );

u32  x_GetSystemUpTimeMs    ( void );


////////////////////////////////////////////////////////////////////////////
// DATE-TIME INFORMATION
////////////////////////////////////////////////////////////////////////////

struct X_DATETIME
{
    u8  month;      // January = 1, February = 2, ect.
    u8  day;        // Day of month, starting at 1
    u16 year;       // 4-digit year

    u16 hour;       // current hour
    u16 minute;     // current minute
    u16 second;     // current second
    u16 millisec;   // current millisecond
};

void x_GetDateTime( X_DATETIME* pDateTimeInfo );


////////////////////////////////////////////////////////////////////////////
// X-FILES ONLY
////////////////////////////////////////////////////////////////////////////

void x_InitTime( void );
void x_KillTime( void );

////////////////////////////////////////////////////////////////////////////

#endif

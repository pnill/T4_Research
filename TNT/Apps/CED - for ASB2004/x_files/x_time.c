///////////////////////////////////////////////////////////////////////////
//
//  X_TIME.C
//
//  if GetTime is not implemented then it will receive the 'ANSI' generic
//  version.
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_TIME_H
#include "x_time.h"
#endif

///////////////////////////////////////////////////////////////////////////
#undef TIME_IS_DEFINED  
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// N64
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64
#define TIME_IS_DEFINED

f64 x_GetTime( void )
{
    return( (f64)( ((f64)(s64)osGetTime()) / ((f64)(s64)osClockRate) ) );
}

#endif

///////////////////////////////////////////////////////////////////////////
// PSX
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PSX
#define TIME_IS_DEFINED

#include "../Engine/PSX/PSXTime.h"

f64 x_GetTime( void )
{
    return PSX_GetTimeInSecs();
}

#endif

///////////////////////////////////////////////////////////////////////////
// PC / D3D
///////////////////////////////////////////////////////////////////////////

#if defined(TARGET_PC) && defined(TARGET_D3D)
#define TIME_IS_DEFINED

    #pragma comment( lib, "kernel32" )

    #include <windows.h>            
    #include <mmsystem.h>

    f64 x_GetTime( void )
    {
        f64 Time;
        LARGE_INTEGER Clock;
        LARGE_INTEGER ClockFreq;
        QueryPerformanceCounter  ( &Clock ); 
        QueryPerformanceFrequency( &ClockFreq ); 
        Time = (f64)(Clock.QuadPart) / (f64)(ClockFreq.QuadPart);
        return( Time );
    }
#endif

///////////////////////////////////////////////////////////////////////////
// SEGA DREAMCAST
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_SDC
#define TIME_IS_DEFINED

    #include <windows.h>            
    #include <mmsystem.h>

    f64 x_GetTime( void )
    {
        f64 Time;
        LARGE_INTEGER Clock;
        LARGE_INTEGER ClockFreq;
        QueryPerformanceCounter  ( &Clock ); 
        QueryPerformanceFrequency( &ClockFreq ); 
        Time = (f64)(Clock.QuadPart) / (f64)(ClockFreq.QuadPart);
        return( Time );
    }
    /*
	#include <windows.h>
    f64 x_GetTime( void )
    {
		return GetTickCount()/(f64)1000;	// GetTickCounr needs to by multiplied to get the right units.
	}
    */

#endif

///////////////////////////////////////////////////////////////////////////
// ANSI GENERIC
///////////////////////////////////////////////////////////////////////////

#ifndef TIME_IS_DEFINED

    #include <time.h>

    f64 x_GetTime( void )
    {
        return( (f64)( ((f32)clock()) / ((f32)CLOCKS_PER_SEC) ) );
    }

#endif

///////////////////////////////////////////////////////////////////////////
#undef TIME_IS_DEFINED
///////////////////////////////////////////////////////////////////////////

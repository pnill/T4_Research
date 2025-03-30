////////////////////////////////////////////////////////////////////////////
//
// X_TIME.CPP
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_time.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#undef TIME_IS_DEFINED

#undef X_TIME_MAX_VALUE
#undef X_TIME_MAX_TICK_VALUE


////////////////////////////////////////////////////////////////////////////
// x_time implementation for PC-XBOX
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PC ) || defined( TARGET_XBOX )

    #define TIME_IS_DEFINED

    //    #pragma comment( lib, "kernel32" )
    #if defined( TARGET_PC )
        #include <windows.h>
        #include <mmsystem.h>
    #else  // TARGET_XBOX
        #include <xtl.h>
    #endif // TARGET_PC

    static s64 s_TimerTicksPerSecond = 0;

    #define X_TIME_TICKS_PER_SECOND     (s_TimerTicksPerSecond)
    #define X_TIME_MAX_TICK_VALUE       ((s64)((~(u64)0) >> 1))


    void x_InitTime( void )
    {
        LARGE_INTEGER ClockFreq;
        QueryPerformanceFrequency( &ClockFreq );

        s_TimerTicksPerSecond = (s64)ClockFreq.QuadPart;
    }

    void x_KillTime( void )
    {
    }

    s64 x_GetTimerTicks( void )
    {
        LARGE_INTEGER Clock;
        QueryPerformanceCounter( &Clock );
        return( (s64)Clock.QuadPart );
    }

#if defined( TARGET_XBOX )

    void x_GetDateTime( X_DATETIME* pDateTimeInfo )
    {
        if( pDateTimeInfo == NULL )
            return;

        SYSTEMTIME SystemTime;

        GetLocalTime( &SystemTime );

        pDateTimeInfo->month    = (u8 )SystemTime.wMonth;
        pDateTimeInfo->day      = (u8 )SystemTime.wDay;
        pDateTimeInfo->year     = (u16)SystemTime.wYear;

        pDateTimeInfo->hour     = (u16)SystemTime.wHour;
        pDateTimeInfo->minute   = (u16)SystemTime.wMinute;
        pDateTimeInfo->second   = (u16)SystemTime.wSecond;
        pDateTimeInfo->millisec = (u16)SystemTime.wMilliseconds;
    }

#else

    void x_GetDateTime( X_DATETIME* pDateTimeInfo )
    {
        if( pDateTimeInfo == NULL )
            return;

        pDateTimeInfo->month    = 0;
        pDateTimeInfo->day      = 0;
        pDateTimeInfo->year     = 0;

        pDateTimeInfo->hour     = 0;
        pDateTimeInfo->minute   = 0;
        pDateTimeInfo->second   = 0;
        pDateTimeInfo->millisec = 0;
    }

#endif



#endif // TARGET_PC || TARGET_XBOX


///////////////////////////////////////////////////////////////////////////
// x_time implementation for PS2
///////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PS2 ) && defined( _MSC_VER )

    #define TIME_IS_DEFINED

    #define X_TIME_TICKS_PER_SECOND     (1000)
    #define X_TIME_MAX_TICK_VALUE       ((s64)((~(u64)0) >> 1))


    void x_InitTime( void ) {}
    void x_KillTime( void ) {}

    s64 x_GetTimerTicks( void )
    {
        return (s64)(0);
    }

#endif


#if defined( TARGET_PS2 ) && !defined( _MSC_VER )

    #define TIME_IS_DEFINED

    #include <eekernel.h>
    #include <eeregs.h>
    #include <libscf.h>

    #define X_TIME_PS2_CLOCK            150000000                   // Ticks per second of Bus Clock @ 150MHz for T10000.
    #define X_TIME_PS2_CLOCK16          (X_TIME_PS2_CLOCK/16)       // 1/16th of Ticks per second
    #define X_TIME_PS2_CLOCK256         (X_TIME_PS2_CLOCK/256)      // 1/256th of Ticks per second

    #define X_TIME_TICKS_PER_SECOND     (X_TIME_PS2_CLOCK16)
    #define X_TIME_MAX_TICK_VALUE       ((s64)((~(u64)0) >> 1))


    // Timer TickCount and record of Timer Count between last 2 Interrupts
    u32 sTimerLastInterruptTickCount;
    u64 sTimerTickCount;

    //=========================================================================

    static int x_TimeInterruptHandler( int ca )
    {
        ca = 0;

        // Add Current Timer Value to Accumulator
        sTimerLastInterruptTickCount = 65536;
        sTimerTickCount += sTimerLastInterruptTickCount;
        *T0_MODE  = *T0_MODE | T_MODE_OVFF_M;                       // Clear Overflow Interrupt

        // Allow additional service to run, -1 prevents chaining interrupt
        return 1;
    }

    //=========================================================================

    void x_InitTime( void )
    {
        // Install Interrupt Handler for Timer0 Overflow
        AddIntcHandler( INTC_TIM0, x_TimeInterruptHandler, 0 );

        // Clear Tick Accumulator
        sTimerTickCount = 0;

        // Setup Timer0
        *T0_COUNT = 0;                                              // Clear T0 count
        *T0_MODE  = 0x01 | T_MODE_OVFE_M | T_MODE_CUE_M;            // Count 1/16th of Bus Clock, Enable Overflow Interrupt and Start

        // Enable Interrupt VBlankON
        EnableIntc( INTC_TIM0 );
    }

    //=========================================================================

    void x_KillTime( void )
    {
        // Remove Timer0 Interrupt Handler and Disable Interrupt
        RemoveIntcHandler( INTC_TIM0, x_TimeInterruptHandler(0) );
        DisableIntc( INTC_TIM0 );
    }

    //=========================================================================

    s64 x_GetTimerTicks( void )
    {
        u32     TimerValue;                                         // 16bit Timer Value
        u64     TickCount;                                          // TickCount Accumulated in Interrupt Handler
        u32     T0Interrupt;                                        // Timer Interrupt Flag
        s64     t;                                                  // Final Calculated Time from Reset in Ticks

        // Read Timer Value & Overflow Counter
        asm("di");
        TimerValue  = *T0_COUNT;
        TickCount   = sTimerTickCount;
        T0Interrupt = *T0_MODE;
        asm("ei");

        // Use TimerOverflow counter according to TimerValue <32768, use Overflow after read, >32768, use Overflow before read
        if( TimerValue > (sTimerLastInterruptTickCount/2) )
        {
            // All Values should be valid
            t  = (s64)(TickCount + TimerValue);
        }
        else
        {
            // TimerValue is small, interrupt could have occurred during reading of timer
            t  = (s64)(TickCount + TimerValue);

            // Adjust time if interrupt occurred and is pending
            if( T0Interrupt & T_MODE_OVFF_M )
                t += sTimerLastInterruptTickCount;
        }

        return t;
    }

    //=========================================================================

    void x_GetDateTime( X_DATETIME* pDateTimeInfo )
    {
        #define PS2_READ_CLOCK_ERROR_RETRIES 50

        static s32 s_ErrorCounter = 0;

        if( pDateTimeInfo == NULL )
            return;

        int        RetVal;
        sceCdCLOCK TimeInfo;

        // Reference on sceCdReadClock says that if it returns 0 too many times the
        // internal clock battery could be dead, or some other error.  This means
        // we'll never get the right time anyways, and that it may hang the system
        // if called too many times.
        if( s_ErrorCounter < PS2_READ_CLOCK_ERROR_RETRIES )
        {
            RetVal = sceCdReadClock( &TimeInfo );

            if( RetVal == 0 )
                s_ErrorCounter++;
        }
        else
        {
            RetVal = 0;
        }

        if( RetVal )
        {
            // Convert from JST to Local Time
            sceScfGetLocalTimefromRTC( &TimeInfo );

            // Date-time values from the PS2 are BCD(binary coded decimal).
            // Convert to normal integers for standard x_files use.

            pDateTimeInfo->month    = btoi( TimeInfo.month );
            pDateTimeInfo->day      = btoi( TimeInfo.day );
            pDateTimeInfo->year     = btoi( TimeInfo.year ) + 2000;

            pDateTimeInfo->hour     = btoi( TimeInfo.hour );
            pDateTimeInfo->minute   = btoi( TimeInfo.minute );
            pDateTimeInfo->second   = btoi( TimeInfo.second );
            pDateTimeInfo->millisec = 0;
        }
        else
        {
            pDateTimeInfo->month    = 0;
            pDateTimeInfo->day      = 0;
            pDateTimeInfo->year     = 0;

            pDateTimeInfo->hour     = 0;
            pDateTimeInfo->minute   = 0;
            pDateTimeInfo->second   = 0;
            pDateTimeInfo->millisec = 0;
        }
    }

#endif // TARGET_PS2



///////////////////////////////////////////////////////////////////////////
// x_Time implementation for DOLPHIN
///////////////////////////////////////////////////////////////////////////

#if defined( TARGET_DOLPHIN ) && !defined( _MSC_VER )

    #define TIME_IS_DEFINED

    #define X_TIME_TICKS_PER_SECOND     (OS_TIMER_CLOCK)
    #define X_TIME_MAX_TICK_VALUE       ((s64)0x00000FFFFFFFFFFFLL)
    // NOTE: the value for X_TIME_MAX_TICK_VALUE is set to this because
    //       an f32 will not be able to hold a number much larger.  Currently
    //       this should prevent time values from wrapping for about 5 days
    //       of straight execution time.  All time values that are stored in
    //       64-bit variables should behave normally.

    static s64 s_XTimeStart = 0;

    void x_InitTime( void )
    {
        s_XTimeStart = (s64)OSGetTime();
    }

    void x_KillTime( void )
    {
    }

    s64 x_GetTimerTicks( void )
    {
        return ((s64)OSGetTime() - s_XTimeStart) & X_TIME_MAX_TICK_VALUE;
    }

    void x_GetDateTime( X_DATETIME* pDateTimeInfo )
    {
        if( pDateTimeInfo == NULL )
            return;

        pDateTimeInfo->month    = 0;
        pDateTimeInfo->day      = 0;
        pDateTimeInfo->year     = 0;

        pDateTimeInfo->hour     = 0;
        pDateTimeInfo->minute   = 0;
        pDateTimeInfo->second   = 0;
        pDateTimeInfo->millisec = 0;
    }

#endif // TARGET_DOLPHIN


////////////////////////////////////////////////////////////////////////////
//  x_time ANSI GENERIC
////////////////////////////////////////////////////////////////////////////

#ifndef TIME_IS_DEFINED

    #include <time.h>

    #define X_TIME_TICKS_PER_SECOND     (CLOCKS_PER_SEC)
    #define X_TIME_MAX_TICK_VALUE       ((s64)((~(u64)0) >> 1))

    void x_InitTime( void ) {}
    void x_KillTime( void ) {}

    s64 x_GetTimerTicks( void )
    {
        return (s64)clock();
    }

    void x_GetDateTime( X_DATETIME* pDateTimeInfo )
    {
        if( pDateTimeInfo == NULL )
            return;

        pDateTimeInfo->month    = 0;
        pDateTimeInfo->day      = 0;
        pDateTimeInfo->year     = 0;

        pDateTimeInfo->hour     = 0;
        pDateTimeInfo->minute   = 0;
        pDateTimeInfo->second   = 0;
        pDateTimeInfo->millisec = 0;
    }

#endif


////////////////////////////////////////////////////////////////////////////
// X_TIME IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

f64 x_GetTime( void )
{
    return( (f64)x_GetTimerTicks() / (f64)X_TIME_TICKS_PER_SECOND );
}

//==========================================================================

s64 x_GetTimerTicksPerSec( void )
{
    return X_TIME_TICKS_PER_SECOND;
}

//==========================================================================

f64 x_TicksToSecs( s64 NTicks )
{
    return ((f64)NTicks / (f64)X_TIME_TICKS_PER_SECOND);
}

//==========================================================================

f64 x_TicksToMs( s64 NTicks )
{
    return ( ((f64)NTicks * 1000.0f) / (f64)X_TIME_TICKS_PER_SECOND );
}

//==========================================================================

f64 x_GetTimeDiff( f64 TimeStart, f64 TimeEnd )
{
    if( TimeStart < TimeEnd )
        return (TimeEnd - TimeStart);

    return ((((f64)X_TIME_MAX_TICK_VALUE / (f64)X_TIME_TICKS_PER_SECOND) - TimeStart) + TimeEnd);
}

//==========================================================================

s64 x_GetTimerTicksDiff( s64 TickStart, s64 TickEnd )
{
    if( TickStart < TickEnd )
        return (TickEnd - TickStart);

    return ((X_TIME_MAX_TICK_VALUE - TickStart) + TickEnd);
}

//==========================================================================

f64 x_GetElapsedTime( f64 TimeStart )
{
    return x_GetTimeDiff( TimeStart, x_GetTime() );
}

//==========================================================================

s64 x_GetElapsedTimerTicks( s64 TickStart )
{
    return x_GetTimerTicksDiff( TickStart, x_GetTimerTicks() );
}

//==========================================================================

u32 x_GetSystemUpTimeMs( void )
{
    return (u32)x_TicksToMs( x_GetTimerTicks() );
}

//==========================================================================

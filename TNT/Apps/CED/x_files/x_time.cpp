////////////////////////////////////////////////////////////////////////////
//
//  X_TIME.CPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_TIME_HPP
#include "x_time.hpp"
#endif

//--------------------------------------------------------------------------
#undef TIME_IS_DEFINED  
//--------------------------------------------------------------------------

f64  x_TicksToSecs( s64 NTicks )
{
    return ((f64)NTicks / (f64)x_GetTimerTicksPerSec()); 
}

f32  x_TicksToMs( s64 NTicks )
{
    return (f32)((f64)(NTicks*1000.0f) / (f64)x_GetTimerTicksPerSec()); 
}

//==========================================================================
//  PC
//==========================================================================

#if defined(TARGET_PC) || defined(TARGET_XBOX)
#define TIME_IS_DEFINED

    #pragma comment( lib, "kernel32" )

    #include <windows.h>            
    #include <mmsystem.h>

    void x_InitTime( void ){}
    void x_KillTime( void ){}    

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

    s64 x_GetTimerTicks( void )
    {
        LARGE_INTEGER Clock;
        QueryPerformanceCounter  ( &Clock ); 
        return( (s64)Clock.QuadPart );
    }

    s64 x_GetTimerTicksPerSec( void )
    {
        LARGE_INTEGER ClockFreq;
        QueryPerformanceFrequency( &ClockFreq ); 
        return( (s64)ClockFreq.QuadPart );
    }

#endif		// TARGET_PC

///////////////////////////////////////////////////////////////////////////
//  x_Time implementation for PS2
///////////////////////////////////////////////////////////////////////////

#if defined(TARGET_PS2) && !defined(_MSC_VER)

#include <eekernel.h>
#include <eeregs.h>

#define X_TIME_PS2_CLOCK            150000000                   // Ticks per second of Bus Clock @ 150MHz for T10000.
#define X_TIME_PS2_CLOCK16          (X_TIME_PS2_CLOCK/16)       // 1/16th of Ticks per second
#define X_TIME_PS2_CLOCK256         (X_TIME_PS2_CLOCK/256)      // 1/256th of Ticks per second

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
    *T0_MODE  = *T0_MODE | T_MODE_OVFF_M ;                          // Clear Overflow Interrupt

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
    *T0_COUNT = 0;                                                  // Clear T0 count
    *T0_MODE  = 0x01 | T_MODE_OVFE_M | T_MODE_CUE_M;                // Count 1/16th of Bus Clock, Enable Overflow Interrupt and Start

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

s64  x_GetTimerTicks( void )
{
    u32     TimerValue;                                             // 16bit Timer Value
    u64     TickCount;                                              // TickCount Accumulated in Interrupt Handler
    u32     T0Interrupt;                                            // Timer Interrupt Flag
    s64     t;                                                      // Final Calculated Time from Reset in Ticks
    
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

s64  x_GetTimerTicksPerSec( void )
{
    return X_TIME_PS2_CLOCK16;
}

//=========================================================================

f64 x_GetTime( void )
{
    s64 t;
    s64 ticks;
    
    t = x_GetTimerTicks();
    ticks = x_GetTimerTicksPerSec();
    
    // Return Time in Seconds
    return ((f64)t/(f64)ticks);
}

#define TIME_IS_DEFINED
#endif		// TARGET_PS2

#if defined(TARGET_PS2) && defined(_MSC_VER)
    void x_InitTime( void ) {}
    void x_KillTime( void ) {}    
    f64  x_GetTime ( void ) { return((f64)0); }
    #define TIME_IS_DEFINED
#endif

//==========================================================================
//  ANSI GENERIC
//==========================================================================

#ifndef TIME_IS_DEFINED

    #include <time.h>

    void x_InitTime( void ) {}
    void x_KillTime( void ) {}    
    
    f64 x_GetTime( void )
    {
        return( (f64)( ((f32)clock()) / ((f32)CLOCKS_PER_SEC) ) );
    }

#endif


//--------------------------------------------------------------------------
#undef TIME_IS_DEFINED
//--------------------------------------------------------------------------

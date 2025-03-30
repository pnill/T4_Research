////////////////////////////////////////////////////////////////////////////
//
//  E_AccumTimer.CPP
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "E_AccumTimer.hpp"
#include "x_time.hpp"
#include "x_debug.hpp"


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

accum_timer::accum_timer( void )
{
    Clear();
}

//==========================================================================

accum_timer::~accum_timer()
{

}

//==========================================================================

void accum_timer::Clear( void )
{
    s32 i;

    for( i = 0; i < 8; i++ )
        m_FrameTime[i] = 0;

    m_Index          = 0;
    m_TimesCollected = 0;
    m_StartTime      = 0;
}

//==========================================================================

void accum_timer::StartTiming( void )
{
    m_StartTime = x_GetTimerTicks();
}

//==========================================================================

void accum_timer::EndTiming( void )
{
    s64 CurrentTime;
    CurrentTime            = x_GetTimerTicks();
    m_FrameTime[ m_Index ] = x_GetTimerTicksDiff( m_StartTime, CurrentTime );
    m_Index                = (m_Index+1) & 0x07;

    m_TimesCollected++;

    if( m_TimesCollected > 8 )
        m_TimesCollected = 8;
}


//==========================================================================

f32 accum_timer::GetAvgTime( void )
{
    s32 i;
    s64 TicksPerSec;
    s64 TotalTime;

    TicksPerSec = x_GetTimerTicksPerSec();

    TotalTime = 0;
    for( i = 0; i < m_TimesCollected; i++ )
        TotalTime += m_FrameTime[ (m_Index+i) & 0x07 ];

#if defined (TARGET_PC) || defined (TARGET_XBOX)
    if( m_TimesCollected < 7 )
        return 0.016666f;
#else
    if( m_TimesCollected == 0 )
        return 0.000001f;
#endif

    return( (f32)TotalTime / (f32)(TicksPerSec * m_TimesCollected) );
}

//==========================================================================

f32 accum_timer::GetTime( s32 TimeIndex )
{
    s64 TicksPerSec;
    s64 TotalTime;

#if defined (TARGET_PC) || defined (TARGET_XBOX)
    if( m_TimesCollected == 0 )
        return 0.016666f;
#else
    if( m_TimesCollected == 0 )
        return 0.000001f;
#endif

    ASSERT( (TimeIndex >= 0) && (TimeIndex <= 7) );
    TicksPerSec = x_GetTimerTicksPerSec();
    TotalTime   = m_FrameTime[ (m_Index - TimeIndex + 8) & 0x07 ];

    return( (f32)TotalTime / (f32)(TicksPerSec) );
}

//==========================================================================

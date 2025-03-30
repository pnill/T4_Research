////////////////////////////////////////////////////////////////////////////
//
//  X_STATS.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_STATS_HPP
#define X_STATS_HPP

////////////////////////////////////////////////////////////////////////////
// 
//  This file provides really simple (but common!) statistic support.  
//  
//  Stat values come in two flavors: Debug and Permanent.  All standard 
//  use of debug stats will disappear unless X_DEBUG is defined.  All use
//  of stats must use the macros and functions defined in this file.
//  
//  All stat objects operate on a single f32 (float) value.  Values for min 
//  and max are maintained automatically.
//  
//  All functions and macros for using the stats begin with either DSTAT
//  or PSTAT which indicate debug or permanent respectively. 
//  
//  Debug stat macros and functions:
//   
//   DSTAT(s)                Create a stat named "s".
//   DSTAT_EXTERN(s)         Declare an external reference to stat s.
//   DSTAT_SetValue(s,v)     Set v as value of stat s; track min and max.
//   DSTAT_AddValue(s,v)     Add v to value of stat s; track min and max.
//   DSTAT_Clear(s)          Set value of s to 0, and clear min and max.
//   DSTAT_ClearMinMax(s)    Clear internal min and max values.
//   DSTAT_GetValue(s)       Return current value of s.
//   DSTAT_GetMin(s)         Return minimum value of s since Clear.
//   DSTAT_GetMax(s)         Return maximum value of s since Clear.
//  
//  For each DSTAT macro and function above, there is a corresponding PSTAT
//  macro or function.
//  
//  In the AddValue and SetValue functions, the internal min/max values will
//  be updated appropriately.
//  
//  The DSTAT(s) and PSTAT(s) macros should only be used in *.c files.
//  
//  The DSTAT_EXTERN(s) and PSTAT_EXTERN(s) macros should be placed in 
//  header files to expose stats to external modules when so desired.
//  
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////
        
#define STAT_MAX_VALUE  100000000.0f

#ifdef X_DEBUG
#define DSTAT(s)        xstat s = { 0.0f, STAT_MAX_VALUE, -STAT_MAX_VALUE }
#define DSTAT_EXTERN(s) extern xstat s
#else
#define DSTAT(s) 
#define DSTAT_EXTERN(s)
#endif

#define PSTAT(s)        xstat s = { 0.0f, STAT_MAX_VALUE, -STAT_MAX_VALUE }
#define PSTAT_EXTERN(s) extern xstat s

////////////////////////////////////////////////////////////////////////////
//  TYPES
////////////////////////////////////////////////////////////////////////////

struct xstat
{
    f32 Value;
    f32 Min;
    f32 Max;
};
    
////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
////////////////////////////////////////////////////////////////////////////

void DSTAT_SetValue   ( xstat& Stat, f32 NewValue );
void DSTAT_AddValue   ( xstat& Stat, f32 Addition );
void DSTAT_Clear      ( xstat& Stat );
void DSTAT_ClearMinMax( xstat& Stat );
f32  DSTAT_GetValue   ( xstat& Stat );
f32  DSTAT_GetMin     ( xstat& Stat );
f32  DSTAT_GetMax     ( xstat& Stat );

void PSTAT_SetValue   ( xstat& Stat, f32 NewValue );
void PSTAT_AddValue   ( xstat& Stat, f32 Addition );
void PSTAT_Clear      ( xstat& Stat );
void PSTAT_ClearMinMax( xstat& Stat );
f32  PSTAT_GetValue   ( xstat& Stat );
f32  PSTAT_GetMin     ( xstat& Stat );
f32  PSTAT_GetMax     ( xstat& Stat );

////////////////////////////////////////////////////////////////////////////
//  DSTAT MACRO IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG

#define DSTAT_SetValue(s,v)     STAT_SetValue((s),((f32)(v)))
#define DSTAT_AddValue(s,v)     STAT_AddValue((s),((f32)(v)))
#define DSTAT_Clear(s)          STAT_Clear((s))
#define DSTAT_ClearMinMax(s)    STAT_ClearMinMax((s))
#define DSTAT_GetValue(s)       STAT_GetValue((s))
#define DSTAT_GetMin(s)         STAT_GetMin((s))
#define DSTAT_GetMax(s)         STAT_GetMax((s))

#else

#define DSTAT_SetValue(s,v)
#define DSTAT_AddValue(s,v)
#define DSTAT_Clear(s)
#define DSTAT_ClearMinMax(s)
#define DSTAT_GetValue(s) 
#define DSTAT_GetMin(s)   
#define DSTAT_GetMax(s)   

#endif

////////////////////////////////////////////////////////////////////////////
//  PSTAT MACRO IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////

#define PSTAT_SetValue(s,v)     STAT_SetValue((s),((f32)(v)))
#define PSTAT_AddValue(s,v)     STAT_AddValue((s),((f32)(v)))
#define PSTAT_Clear(s)          STAT_Clear((s))
#define PSTAT_ClearMinMax(s)    STAT_ClearMinMax((s))
#define PSTAT_GetValue(s)       STAT_GetValue((s))
#define PSTAT_GetMin(s)         STAT_GetMin((s))
#define PSTAT_GetMax(s)         STAT_GetMax((s))

////////////////////////////////////////////////////////////////////////////
//  INLINE FUNCTIONS
////////////////////////////////////////////////////////////////////////////

inline
void STAT_SetValue( xstat& S, f32 V )
{                                                                 
    S.Value = V;                                                  
    if( S.Value < -STAT_MAX_VALUE ) S.Value = -STAT_MAX_VALUE;    
    if( S.Value >  STAT_MAX_VALUE ) S.Value =  STAT_MAX_VALUE;    
    if( S.Value < S.Min )           S.Min = S.Value;              
    if( S.Value > S.Max )           S.Max = S.Value;              
}                                                                            

inline
void STAT_AddValue( xstat& S, f32 V )                                        
{                                                                 
    S.Value += V;                                                 
    if( S.Value < -STAT_MAX_VALUE ) S.Value = -STAT_MAX_VALUE;    
    if( S.Value >  STAT_MAX_VALUE ) S.Value =  STAT_MAX_VALUE;    
    if( S.Value < S.Min )           S.Min = S.Value;              
    if( S.Value > S.Max )           S.Max = S.Value;              
}

inline 
void STAT_ClearMinMax( xstat& S )
{                                                                 
    S.Min = S.Value;                                              
    S.Max = S.Value;                                              
}

inline 
f32 STAT_GetValue( xstat& S )
{
    return( S.Value );
}

inline 
f32 STAT_GetMin( xstat& S )
{
    return( S.Min );
}

inline 
f32 STAT_GetMax( xstat& S )
{
    return( S.Max );
}

////////////////////////////////////////////////////////////////////////////

#endif

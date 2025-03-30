///////////////////////////////////////////////////////////////////////////
//
//  PS2PROF.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef PS2PROF_HPP
#define PS2PROF_HPP

///////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"

///////////////////////////////////////////////////////////////////////////
//
// PS2PROF_Init:
// If you are doing profiling on a small section of code then I suggest
// SamplesPerSec 200000<->500000. Thats 200 to 500 times per millisecond.
// Be sure and turn profiling on and off around the section you are 
// profiling when using this high of a sampling rate.  For general 
// 'constant' profiling I suggest 100000. 
// SamplesInBuffer is how many samples to collect at once.  Each sample
// is 4bytes so 1048576 samples in buffer is 4Meg.
//
///////////////////////////////////////////////////////////////////////////

void    PS2PROF_Init    ( s32 SamplesPerSec, s32 SamplesInBuffer );
void    PS2PROF_Kill    ( void );

// If profiling is off, there is no performance hit.
void    PS2PROF_CPUOn      ( void );
void    PS2PROF_CPUOff     ( void );
xbool   PS2PROF_IsCPUOn    ( void );
void    PS2PROF_GraphicsOn      ( void );
void    PS2PROF_GraphicsOff     ( void );
xbool   PS2PROF_IsGraphicsOn    ( void );

// Unfortunately it can't flush itself right now.  A call to FlushBuffer
// will write out the current full buffer to disk or write out what ever
// data has been collected if you use Force.
void    PS2PROF_FlushBuffer( xbool Force = FALSE );

///////////////////////////////////////////////////////////////////////////

#endif

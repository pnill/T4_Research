///////////////////////////////////////////////////////////////////////////
//
//  SPRHELP.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef SPRHELP_HPP
#define SPRHELP_HPP

#ifdef TARGET_PS2

#include "x_types.hpp"

#include <libdma.h>

////////////////////////////////////////////////////////////////////////////
// SCRATCHPAD INTRODUCTION:
//
//      The PS2 has a super-fast area of memory, which can be used as a
//      temporary work area. If you know that the temporary buffer that you
//      need is less than 16k, and is generally linear in layout (although
//      you don't have to use it in a linear fashion). The beauty of
//      scratch-pad ram is that you have effectively gotten rid of all
//      data-cache misses, and operations like memcpy are super fast.
//
//      Scratchpad maps into an upper area of memory that you can access
//      like normal.
//
//      EXAMPLE 1: Generally its good to sort mesh data by texture to avoid
//                 GS stalls. If you know that the data you are going to
//                 sort is less than 16k, throw it in SPR to speed up the
//                 sort routine.
//
//      EXAMPLE 2: According to some people (and depending on how heavy you
//                 rely on your DMAC), it can actually be faster to DMA
//                 data to SPR, then back regular memory rather than doing
//                 a memcpy!
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
//
// Scratchpad RAM functions
//
////////////////////////////////////////////////////////////////////////////
//
//  SPRHELP_Initialize      - initializes the scratchpad to and from
//                            DMA channels
//
//  SPRHELP_DmaFrom         - DMAs data out of scratch-pad RAM into an area
//                            of memory you have resereved.
//
//  SPRHELP_DmaTo           - DMAs data from a source memory address into
//                            scratch-pad RAM, and returns the address that
//                            the DMAed data then maps to.
//
////////////////////////////////////////////////////////////////////////////

void  SPRHELP_Initialize    ( void );
void  SPRHELP_DmaFrom       ( void* pDst, s32 NBytes );
void* SPRHELP_DmaTo         ( void* pSrc, s32 NBytes );

////////////////////////////////////////////////////////////////////////////
// Scratch-pad globals
////////////////////////////////////////////////////////////////////////////

extern sceDmaChan*  SPR_CHANNEL_TO;
extern sceDmaChan*  SPR_CHANNEL_FROM;
extern sceDmaTag    SPR_DmaTag;

#ifdef X_DEBUG
extern xbool        SPR_Initialized;
#endif

////////////////////////////////////////////////////////////////////////////
// Inlines
////////////////////////////////////////////////////////////////////////////

/*inline void SPR_WaitDMA(void)
{
  asm __volatile__
  ("
    sync.l
  WDMA:
    bc0f WDMA
    nop
  ");
}*/

//==========================================================================

inline void* SPRHELP_DmaTo( void* pSrc, s32 NBytes )
{
    ASSERT( SPR_Initialized );  // make sure we have proper channel, etc...
    ASSERT( NBytes <= 16384 );  // scratchpad is only 16k

    //---   set up the quad-word count (allow it to DMA a little padding)
    SPR_DmaTag.qwc = (NBytes+15)>>4;

    //---   Set tag ID
    //---   refe transfers from specified address then terminates
    SPR_DmaTag.id = 0x00;

    //---   Set transfer source address
    SPR_DmaTag.next = (sceDmaTag*)pSrc;

    //---   send the packet
    sceDmaSend( SPR_CHANNEL_TO, &SPR_DmaTag );
    sceDmaSync( SPR_CHANNEL_TO, 0, 0 );

    return (void*)0x70000000;
}

//==========================================================================

inline void  SPRHELP_DmaFrom( void* pDst, s32 NBytes )
{
    ASSERT( SPR_Initialized );  // make sure we have proper channel, etc...
    ASSERT( NBytes <= 16384);   // scratchpad is only 16k

    //---   set up the quad-word count (allow it to DMA a little padding)
    SPR_DmaTag.qwc = (NBytes+15)>>4;

    //---   Set tag ID
    //---   refe transfrom from specified address then terminates
    SPR_DmaTag.next = (sceDmaTag*)pDst;

    //---   send the packet
    sceDmaSend( SPR_CHANNEL_FROM, &SPR_DmaTag );
    sceDmaSync( SPR_CHANNEL_FROM, 0, 0 );
}

#endif

#endif

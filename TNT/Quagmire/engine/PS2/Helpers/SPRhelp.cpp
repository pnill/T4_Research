////////////////////////////////////////////////////////////////////////////
// SPRHelp.cpp
////////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"

#include "SPRHelp.hpp"
#include <libdma.h>

sceDmaChan*  SPR_CHANNEL_TO;
sceDmaChan*  SPR_CHANNEL_FROM;
sceDmaTag    SPR_DmaTag;

#ifdef X_DEBUG
xbool        SPR_Initialized = FALSE;
#endif

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////


void SPRHELP_Initialize( void )
{
    ASSERT( SPR_Initialized == FALSE );

    //---   Get DMA channels 8 and 9 (from and to scratchpad)
    SPR_CHANNEL_TO      = sceDmaGetChan( SCE_DMA_toSPR );
    SPR_CHANNEL_FROM    = sceDmaGetChan( SCE_DMA_fromSPR );
    ASSERT( SPR_CHANNEL_TO );
    ASSERT( SPR_CHANNEL_FROM );

    //---   Clear Transfer Tag Enable to stop padding being transferred
    SPR_CHANNEL_TO->chcr.TTE    = 0;
    SPR_CHANNEL_FROM->chcr.TTE  = 0;

    //---   Set transfer destination address (on scratchpad)
    SPR_CHANNEL_TO->sadr    = (void*)0x80000000;
    SPR_CHANNEL_FROM->sadr  = (void*)0x80000000;

#ifdef X_DEBUG
    SPR_Initialized = TRUE;
#endif
}


///////////////////////////////////////////////////////////////////////////
//
//  DMAHELP.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef DMAHELP_H
#define DMAHELP_H

///////////////////////////////////////////////////////////////////////////

#include <eekernel.h>
#include <eeregs.h>
#include <libdma.h>
#include "x_types.hpp"

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////

void DMAHELP_Wait           ( void );

void DMAHELP_BuildTagCont   ( sceDmaTag* pTag, s32 NBytes );
void DMAHELP_BuildTagEnd    ( sceDmaTag* pTag, s32 NBytes );
void DMAHELP_BuildTagRef    ( sceDmaTag* pTag, u32 RefAddress, s32 NBytes );
void DMAHELP_BuildTagRefe	( sceDmaTag* pTag, u32 RefAddress, s32 NBytes );
void DMAHELP_BuildTagCall   ( sceDmaTag* pTag, u32 NewAddress, s32 NBytes );
void DMAHELP_BuildTagRet    ( sceDmaTag* pTag, s32 NBytes );

//void DMAHELP_Trace          ( sceDmaTag* FirstTag, xbool TraceVif );

// temporary display list management functions
void DMAIMMEDIATE_Init( void );
void DMAIMMEDIATE_Kill( void );
s32 DMAIMMEDIATE_InitDisplayList( s32 size );
xbool DMAIMMEDIATE_SendDisplayList( s32 iDListIndex );
xbool DMAIMMEDIATE_KillDisplayList( s32 iDListIndex );

xbool DMAIMMEDIATE_AddDisplayListRef( s32 iDListIndex, s32 nBytes, u32 RefAddress, xbool bUseRefe = FALSE, xbool bUseAbsoluteSize = FALSE );
xbool DMAIMMEDIATE_AddDisplayListCont( s32 iDListIndex, s32 nBytes );
xbool DMAIMMEDIATE_AddDisplayListCall( s32 iDListIndex, s32 nBytes, u32 RefAddress );
xbool DMAIMMEDIATE_DisplayListEnd( s32 iDListIndex );

// Special function to dump error info to the TTY and ASSERT on badness for the temporary display lists
void DMAIMMEDIATE_ReportLastError();

                                
///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////

#endif

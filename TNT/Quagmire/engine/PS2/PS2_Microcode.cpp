//=========================================================================
//
//  PS2_Microcode.CPP
//
//=========================================================================

#include "x_files.hpp"
#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"

#include "Q_Engine.hpp"
#include "Q_PS2.hpp"

#include "vifhelp.hpp"
#include "dmahelp.hpp"
#include "dmaman.hpp"
#include "dvtrace.hpp"

//=========================================================================
// DEFINES
//=========================================================================

#define MAX_MICROCODES  8

//=========================================================================
// TYPES
//=========================================================================

typedef struct
{
    char  Name[32];
    void* CodeDMAAddr;
    void* DataDMAAddr;
} microcode;


typedef struct t_MicrocodeVIFData
{
    u32     VIF[ 4 ];
}t_MicrocodeVIFData;


//=========================================================================
// VARIABLES
//=========================================================================

static microcode    s_MCode[ MAX_MICROCODES ];
static s32          s_NMCodes=0;
static s32          s_ActiveMCode=-1;


static byte *s_pMicrocodeVIFDataBuffer = NULL;
static t_MicrocodeVIFData *s_pMicrocodeVIFData = NULL;

//=========================================================================
// FUNCTIONS
//=========================================================================

s32     PS2_RegisterMicroCode   ( char* Name,
                                  void* CodeDMAAddr,
                                  void* DataDMAAddr)
{
    microcode* pM;

    ASSERT( s_NMCodes < MAX_MICROCODES );
    pM = &s_MCode[ s_NMCodes ];
    s_NMCodes++;

    x_strcpy( pM->Name, Name );

    pM->CodeDMAAddr = CodeDMAAddr;
    pM->DataDMAAddr = DataDMAAddr;

    ASSERT( (((u32)CodeDMAAddr)&0x0F) == 0 );
    ASSERT( (((u32)DataDMAAddr)&0x0F) == 0 );

    if (!s_pMicrocodeVIFDataBuffer)
    {
        s_pMicrocodeVIFDataBuffer = (byte *)new byte[ sizeof(t_MicrocodeVIFData) + 16 ];
        s_pMicrocodeVIFData = (t_MicrocodeVIFData *)s_pMicrocodeVIFDataBuffer;
        while ((u32)s_pMicrocodeVIFData & 0xF)
            ((byte *)s_pMicrocodeVIFData) ++;
        s_pMicrocodeVIFData->VIF[0] = 0;
        s_pMicrocodeVIFData->VIF[1] = 0;
        s_pMicrocodeVIFData->VIF[2] = 0;
        s_pMicrocodeVIFData->VIF[3] = SCE_VIF1_SET_FLUSH(0);
    }

    return s_NMCodes-1;
}

//=========================================================================

s32     PS2_GetActiveMicroCode  ( void )
{
    return s_ActiveMCode;
}

//=========================================================================

void    PS2_ActivateMicroCode   ( s32 ID )
{
    microcode*  pM;

    ASSERT( (ID>=0) && (ID<s_NMCodes) );
    
    if( ID == s_ActiveMCode )
        return;

    pM = &s_MCode[ ID ];

    // Flush any previous requests
    sceDmaTag *pDMA = (sceDmaTag*)pDList;
    pDList += sizeof(sceDmaTag);
    DMAHELP_BuildTagRef( pDMA, 
                         (u32)s_pMicrocodeVIFData, 
                         sizeof( t_MicrocodeVIFData ) );

    // Upload code
    DMAHELP_BuildTagCall( (sceDmaTag*)pDList, (u32)pM->CodeDMAAddr, 0 );
    pDList += sizeof(sceDmaTag);

    // Add reference to upload data
    if( pM->DataDMAAddr )
    {
        DMAHELP_BuildTagCall( (sceDmaTag*)pDList, (u32)pM->DataDMAAddr, 0 );
        pDList += sizeof(sceDmaTag);
    }

    // Remember that this is the active microcode
    s_ActiveMCode = ID;

    // Display loading
    //x_printf("MICROCODE (%1s) Loaded.\n",s_MCode[ID].Name);
}

//=========================================================================

void PS2_InvalidateMicroCode( void )
{
    s_ActiveMCode = -1;
}




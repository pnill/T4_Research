///////////////////////////////////////////////////////////////////////////
//
//  DVTRACE.C
//
///////////////////////////////////////////////////////////////////////////

#include <eekernel.h>
#include <eeregs.h>
#include <libdma.h>

#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"

#include "dvtrace.hpp"

//=========================================================================

#define DVPRINT DVTRACE_Printf

static X_FILE* FP=NULL;
static char    OutputBuffer[256];

//=========================================================================

void DVTRACE_OpenOutputFile ( char* FileName )
{
    if( FP )
        DVTRACE_CloseOutputFile();
        
    FP = x_fopen(FileName,"wt");
    ASSERT( FP );
}

//=========================================================================

void DVTRACE_CloseOutputFile( void )
{
    ASSERT( FP );
    x_fclose(FP);
}

//=========================================================================

void DVTRACE_Printf         ( const char* FormatStr, ... )
{
    s32         Len;
    x_va_list   Args;
    
    // Build text from arguments
    x_va_start( Args, FormatStr );
    Len = x_vsprintf( OutputBuffer, FormatStr, Args );
    ASSERT( Len < 256 );
    
    // Decide where to send it
    if( FP )
    {   
        x_fprintf(FP,"%1s",OutputBuffer);
    }
    else
    {
        x_printf("%1s",OutputBuffer);
    }
}

/*
#define SCE_VIF1_SET_BASE(base, irq) ((u_int)(base) | ((u_int)0x03 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_OFFSET(offset, irq) ((u_int)(offset) | ((u_int)0x02 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MSCAL(vuaddr, irq) ((u_int)(vuaddr) | ((u_int)0x14 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MSCNT(irq) (((u_int)0x17 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_NOP(irq) ((u_int)(irq) << 31)
#define SCE_VIF1_SET_STROW(irq) (((u_int)0x30 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_UNPACK(vuaddr, num, cmd, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x60 | (cmd)) << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MPG(vuaddr, num, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x4a) << 24) | ((u_int)(irq) << 31))
*/
//=========================================================================

void InternalTraceVIF       ( u32* FirstAddress, 
                              s32  NBytesToTrace, 
                              s32* NVifTags, 
                              s32* VifSrcSize,
                              s32* VifDstSize )
{
    u32 VIFTagAddr;
    u32 NextVIFTagAddr;
    u32 VIFTag;
    s32 ID;
    
    xbool Finished = FALSE;
    
    // Clear counters
    *NVifTags = 0;
    *VifSrcSize = 0;
    *VifDstSize = 0;
    
    VIFTagAddr = (u32)FirstAddress;
    while( !Finished )
    {
       
        // Check if we are past bounds
        if( (u32)(VIFTagAddr - (u32)FirstAddress) >= (u32)NBytesToTrace )
            break;
    
        // Confirm address is aligned.
        if( (VIFTagAddr & 0x3) != 0 )
        {
            DVPRINT("VIF address is not 32 bit aligned!\n");
            break;
        }
        
        // Get copy of VIF data
        VIFTag = *((u32*)VIFTagAddr);
        
        // Increment tag counter
        *NVifTags = (*NVifTags)+1;
        
        // Get ID
        ID = (s32)(VIFTag>>24)&0xFF;
        
        x_printf("%08X  ",VIFTag);
        // Switch based on ID
        switch( ID )
        {
            // NOP
            case 0x00:
                        NextVIFTagAddr = VIFTagAddr + 4;
                        DVPRINT("VIF | NOP       | %8X:ADR | %8X:NXT |\n", VIFTagAddr, NextVIFTagAddr );
                        VIFTagAddr = NextVIFTagAddr;
                        break;

            // MPG
            case 0x4A:
                        NextVIFTagAddr = VIFTagAddr + 4;
                        DVPRINT("VIF | MPG       | %8X:ADR | %8X:NXT | %4d:SIZE |\n", VIFTagAddr, NextVIFTagAddr,(s32)((VIFTag)>>16)&0xFF );
                        VIFTagAddr = NextVIFTagAddr;
                        break;

/*
    Size        = Size >> 3;
    DestAddr    = DestAddr >> 3;
    VP          = (u32)(DestAddr&0x0000FFFF);
    VP         |= (((u32)(0x4a))<<24);  
    VP         |= (u32)((Size<<16)&0x00FF0000);
    M->NOP[0]   = 0;
    M->NOP[1]   = 0;
    M->NOP[2]   = 0;
    M->VifMPG   = VP;
*/
            // STROW
            case 0x30:
                        NextVIFTagAddr = VIFTagAddr + 4 + 16;
                        DVPRINT("VIF | STROW     | %8X:ADR | %8X:NXT |\n", VIFTagAddr, NextVIFTagAddr );
                        (*VifSrcSize) = (*VifSrcSize) + 16;
                        VIFTagAddr = NextVIFTagAddr;
                        break;

            // STMASK
            case 0x20:
                        NextVIFTagAddr = VIFTagAddr + 4 + 4;
                        DVPRINT("VIF | STMASK    | %8X:ADR | %8X:NXT |\n", VIFTagAddr, NextVIFTagAddr );
                        (*VifSrcSize) = (*VifSrcSize) + 4;
                        VIFTagAddr = NextVIFTagAddr;
                        break;
            
            // UNPACK
            case 0x70:  case 0x71:  case 0x72:  case 0x73:
            case 0x74:  case 0x75:  case 0x76:  case 0x77:
            case 0x78:  case 0x79:  case 0x7A:  case 0x7B:
            case 0x7C:  case 0x7D:  case 0x7E:  case 0x7F:  
            case 0x60:  case 0x61:  case 0x62:  case 0x63:
            case 0x64:  case 0x65:  case 0x66:  case 0x67:
            case 0x68:  case 0x69:  case 0x6A:  case 0x6B:
            case 0x6C:  case 0x6D:  case 0x6E:  case 0x6F:  
                        {
                            s32 Count;
                            s32 SrcSize;
                            s32 Format;
                            s32 ISize[16] = {32,16,8,0,32,16,8,0,32,16,8,0,32,16,8,5};
                            s32 VSize[16] = {1,1,1,0,2,2,2,0,3,3,3,0,4,4,4,4};
                            s32 SSize[16] = {4,2,1,0,8,4,2,0,12,6,3,0,16,8,4,2};
                            char FormatName[8];
                                                    
                            Count  = (s32)(VIFTag >> 16)&0xFF;
                            Format = ID&0xF;
                            SrcSize = SSize[Format]*Count;
                            NextVIFTagAddr = VIFTagAddr + 4 + SrcSize;
                            (*VifSrcSize) = (*VifSrcSize) + SrcSize;
                            (*VifDstSize) = (*VifDstSize) + Count*16;
                            
                            // Build format name
                            if( VSize[Format] == 1 )
                                x_sprintf( FormatName, "S_%1d", ISize[Format]);
                            else
                            if( VSize[Format] == 0 )
                            {
                                x_sprintf( FormatName, "INVALID" );
                                Finished = TRUE;
                            }
                            else
                                x_sprintf( FormatName, "V%1d_%1d", VSize[Format],ISize[Format]);
                            
                            // Display info
                            DVPRINT("VIF | UNPACK    | %8X:ADR | %8X:NXT | %3d:CNT | %5s:FMT | %4d:SIZE |\n",
                                VIFTagAddr,NextVIFTagAddr,Count,FormatName,SrcSize);
                            
                            // Decide on next VIF addr
                            VIFTagAddr = NextVIFTagAddr;
                            
                            break;
                        }
        
            // OFFSET
            case 0x02:
                        NextVIFTagAddr = VIFTagAddr + 4;
                        DVPRINT("VIF | OFFSET    | %8X:ADR | %8X:NXT | %4d:BASE |\n", VIFTagAddr, NextVIFTagAddr, (VIFTag&0xFFFF) );
                        VIFTagAddr = NextVIFTagAddr;
                        break;
                        
            // BASE
            case 0x03:
                        NextVIFTagAddr = VIFTagAddr + 4;
                        DVPRINT("VIF | BASE      | %8X:ADR | %8X:NXT | %4d:BASE |\n", VIFTagAddr, NextVIFTagAddr, (VIFTag&0xFFFF) );
                        VIFTagAddr = NextVIFTagAddr;
                        break;
                        
            // MSCAL
            case 0x14:
                        NextVIFTagAddr = VIFTagAddr + 4;
                        DVPRINT("VIF | MSCAL     | %8X:ADR | %8X:NXT |\n", VIFTagAddr, NextVIFTagAddr );
                        VIFTagAddr = NextVIFTagAddr;
                        break;
                        
            // MSCNT
            case 0x17:
                        NextVIFTagAddr = VIFTagAddr + 4;
                        DVPRINT("VIF | MSCNT     | %8X:ADR | %8X:NXT |\n", VIFTagAddr, NextVIFTagAddr );
                        VIFTagAddr = NextVIFTagAddr;
                        break;
             
            default:    
                        DVPRINT("VIF | UNKNOWN   | %8X:ADR | %8X:VIF | %1d:ID |\n",VIFTagAddr,(u32)VIFTag,ID);
                        Finished = TRUE;
                        break;
        }
    }
}

//=========================================================================

sceDmaTag* RecursiveTraceDMA( sceDmaTag* FirstDMATag )
{
    FirstDMATag = NULL;
    return NULL;
}

//=========================================================================

void DVTRACE_TraceDMA( sceDmaTag* FirstDMATag, xbool TraceVif )
{
    u32         NextDMATagAddr;
    u32         DMATagAddr;
    sceDmaTag   DMATag;
    s32         NDMATagsHit;
    s32         NDMABytes;
    s32         NVIFTagsHit;
    s32         NVIFSrcBytes;
    s32         NVIFDstBytes;
    s32         DMASize;
    xbool       IRQSet;
    xbool       Finished = FALSE;
    
    DVPRINT("******** DMA TRACE BEGINNING ********\n");

    NDMATagsHit = 0;
    NDMABytes = 0;
    NVIFTagsHit = 0;
    NVIFSrcBytes = 0;
    NVIFDstBytes = 0;
    
    DMATagAddr = (u32)FirstDMATag;
    while( !Finished )
    {
        // Confirm address is aligned.
        if( (DMATagAddr & 0xF) != 0 )
        {
            DVPRINT("DMA address is not 128 bit aligned!\n");
            break;
        }
        
        // Get copy of tag
        DMATag = *((sceDmaTag*)DMATagAddr);
        
        // Increment number of tags hit
        NDMATagsHit++;
        DMASize = DMATag.qwc*16;
        NDMABytes += DMASize;
        IRQSet = (DMATag.id & 0x80)?(TRUE):(FALSE);
        DMATag.id &= 0x7F;
        switch( DMATag.id )
        {
        
            case 0x10:  NextDMATagAddr = DMATagAddr + 16 + DMASize;
                        DVPRINT("DMA | CONTINUE  | %8X:ADR | %8X:NXT | %6d:SIZE |\n",DMATagAddr,NextDMATagAddr,DMASize);
                        
                        if( TraceVif ) 
                        {
                            s32 VTH,VSRC,VDST;
                            InternalTraceVIF( (u32*)(DMATagAddr+4), DMASize, &VTH, &VSRC, &VDST );
                            NVIFTagsHit += VTH;
                            NVIFSrcBytes += VSRC;
                            NVIFDstBytes += VDST;
                        }
                        
                        DMATagAddr = NextDMATagAddr;                        
                        break;
                        
            case 0x30:  NextDMATagAddr = DMATagAddr + 16;
                        DVPRINT("DMA | REFERENCE | %8X:ADR | %8X:NXT | %6d:SIZE | %8X:REF |\n",DMATagAddr,NextDMATagAddr,DMASize,(u32)DMATag.next);

                        if( TraceVif ) 
                        {
                            s32 VTH,VSRC,VDST;
                            InternalTraceVIF( (u32*)DMATag.next, DMASize, &VTH, &VSRC, &VDST );
                            NVIFSrcBytes += VSRC;
                            NVIFDstBytes += VDST;
                        }
                        
                        DMATagAddr = NextDMATagAddr;                        
                        break;
                        
            case 0x50:  NextDMATagAddr = DMATagAddr + 16 + DMASize;
                        DVPRINT("DMA | CALL      | %8X:ADR | %8X:NXT | %6d:SIZE | %8X:ADR |\n",DMATagAddr,NextDMATagAddr,DMASize,(u32)DMATag.next);

                        if( TraceVif ) 
                        {
                            s32 VTH,VSRC,VDST;
                            InternalTraceVIF( (u32*)(DMATagAddr+4), DMASize, &VTH, &VSRC, &VDST );
                            NVIFTagsHit += VTH;
                            NVIFSrcBytes += VSRC;
                            NVIFDstBytes += VDST;
                        }
                        
                        DMATagAddr = NextDMATagAddr;                        
                        break;
                        
            case 0x60:  NextDMATagAddr = DMATagAddr + 16 + DMASize;
                        DVPRINT("DMA | RET       | %8X:ADR | %8X:NXT | %6d:SIZE |\n",DMATagAddr,NextDMATagAddr,DMASize);

                        if( TraceVif ) 
                        {
                            s32 VTH,VSRC,VDST;
                            InternalTraceVIF( (u32*)(DMATagAddr+4), DMASize, &VTH, &VSRC, &VDST );
                            NVIFTagsHit += VTH;
                            NVIFSrcBytes += VSRC;
                            NVIFDstBytes += VDST;
                        }
                        
                        Finished = TRUE;
                        break;

            case 0x70:  DVPRINT("DMA | END       | %8X:ADR | %8X:NXT | %6d:SIZE |\n",DMATagAddr,0,DMASize);
                        
                        if( TraceVif && (DMASize>0) ) 
                        {
                            s32 VTH,VSRC,VDST;
                            InternalTraceVIF( (u32*)(DMATagAddr+4), DMASize, &VTH, &VSRC, &VDST );
                            NVIFSrcBytes += VSRC;
                            NVIFDstBytes += VDST;
                        }
                        
                        Finished = TRUE;
                        break;
                        
            default:    DVPRINT("DMA | UNKNOWN   | id:   %1d\n",(s32)DMATag.id);
                        DVPRINT("                  qwc : %1d\n",(s32)DMATag.qwc); 
                        DVPRINT("                  mark: %1d\n",(s32)DMATag.mark);
                        DVPRINT("                  next: %1X\n",(u32)DMATag.next);
                        DVPRINT("                  pad0: %1X\n",(u32)DMATag.p[0]);
                        DVPRINT("                  pad1: %1X\n",(u32)DMATag.p[1]);
                        Finished = TRUE;
                        break;
        }        
    }

    DVPRINT("************************************\n");
    DVPRINT("DMA Tags found:                %1d\n",NDMATagsHit);
    DVPRINT("Num DMA bytes transferred:     %1d\n",NDMABytes);
//    x_printf("Num DMA bytes transferred:     %1d\n",NDMABytes);
    DVPRINT("Avg DMA bytes per transfer:    %1d\n",NDMABytes/NDMATagsHit);
    if( TraceVif )
    {
        DVPRINT("VIF Tags found:                %1d\n",NVIFTagsHit);
        DVPRINT("Num VIF src bytes transferred: %1d\n",NVIFSrcBytes);
        DVPRINT("Num VIF dst bytes transferred: %1d\n",NVIFDstBytes);
    }
    DVPRINT("******** DMA TRACE FINISHED ********\n");
}

//=========================================================================

void DVTRACE_TraceVIF       ( u32* FirstAddress, s32 NBytes )
{
    s32 NVIFTagsHit;
    s32 NVIFSrcBytes;
    s32 NVIFDstBytes;
    
    DVPRINT("******** VIF TRACE BEGINNING ********\n");

    InternalTraceVIF( FirstAddress, NBytes, &NVIFTagsHit, &NVIFSrcBytes, &NVIFDstBytes );

    DVPRINT("************************************\n");
    DVPRINT("VIF Tags found:                %1d\n",NVIFTagsHit);
    DVPRINT("Num VIF src bytes transferred: %1d\n",NVIFSrcBytes);
    DVPRINT("Num VIF dst bytes transferred: %1d\n",NVIFDstBytes);
    DVPRINT("******** VIF TRACE FINISHED ********\n");
}

//=========================================================================
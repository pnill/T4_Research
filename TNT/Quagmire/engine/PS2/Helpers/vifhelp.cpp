///////////////////////////////////////////////////////////////////////////
//
//  VIFHELP.C
//
///////////////////////////////////////////////////////////////////////////

#include <eekernel.h>
#include <eeregs.h>
#include <eestruct.h>

#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"
#include "x_time.hpp"

#include "vifhelp.hpp"
#include "dmahelp.hpp"

//=========================================================================

#define MAX_VIF1_SAMPLES    1024
#define MAX_VIF1_LABELS     128

typedef struct
{
    s64 Ticks;
    s16 Type;
    s16 Label;
} vif1_sample;

static s64          s_VifCounter;
static vif1_sample  s_VifSample[MAX_VIF1_SAMPLES];
static s32          s_NVifSamples;
static xbool        s_RecordingOn;
static char         s_VifLabel[32][MAX_VIF1_LABELS];
static s32          s_NVifLabels;

volatile xbool s_InHandler = FALSE;

//=========================================================================

static int VIFHELP_InterruptHandler( int ca )
{
    s_InHandler = TRUE;
    
    // Do basic stuff
    ca = 0;
    s_VifCounter++;
    
    // Record sample if recording
    if( s_RecordingOn )
    {
        if( s_NVifSamples < MAX_VIF1_SAMPLES )
        {
            u32 Code;
            vif1_sample* pSample;

            pSample = s_VifSample + s_NVifSamples;
            s_NVifSamples++;

            Code = DGET_VIF1_CODE();
            pSample->Ticks = x_GetTimerTicks();
            pSample->Type  = (s16)((Code>>16)&0xFF);
            pSample->Label = (s16)((Code>> 0)&0xFFFF);
        }
        else
        {
            s_RecordingOn = FALSE;
        }
    }
    
    // Restart VIF1
    *VIF1_FBRST = 0x08;

    // Allow additional service to run, -1 prevents chaining interrupt
    s_InHandler = FALSE;
    return 1;
}

//=========================================================================

void WaitForHandler( void )
{
    while( s_InHandler) {};
}

//=========================================================================

void VIFHELP_InitModule    ( void )
{
    // Install Interrupt Handler for VIF1
    AddIntcHandler( INTC_VIF1, VIFHELP_InterruptHandler, 0 );

    s_VifCounter  = 0;
    s_NVifSamples = 0;
    s_NVifLabels  = 0;
    s_RecordingOn = FALSE;

    // Enable Interrupt VBlankON
    EnableIntc( INTC_VIF1 );
}


//=========================================================================

void VIFHELP_SetMarker    ( void* Dst, s32 LabelIndex, s32 Type )
{
    u32 Instr;
   
    Instr = 0x80000000;
    Instr |= ((u32)Type       << 16) & 0x000F0000;
    Instr |= ((u32)LabelIndex << 0 ) & 0x0000FFFF;
    
    ((u32*)Dst)[0] = Instr;
    ((u32*)Dst)[1] = 0;
    ((u32*)Dst)[2] = 0;
    ((u32*)Dst)[3] = 0;
   
}

//=========================================================================

void VIFHELP_KillModule    ( void )
{
    // Remove VIF1 Interrupt Handler
    RemoveIntcHandler( INTC_VIF1, VIFHELP_InterruptHandler(0) );
}

//=========================================================================

s32  VIFHELP_CreateLabel   ( char* Label )
{
    ASSERT( s_NVifLabels < MAX_VIF1_LABELS );
    x_strncpy( s_VifLabel[ s_NVifLabels ], Label, 32 );
    s_NVifLabels++;
    return s_NVifLabels-1;
}

//=========================================================================

void VIFHELP_ClearBuffer   ( void )
{
    WaitForHandler();
    s_NVifSamples = 0;
}

//=========================================================================

void VIFHELP_StartRecording( void )
{
    WaitForHandler();
    s_RecordingOn = TRUE;
}

//=========================================================================

void VIFHELP_EndRecording  ( void )
{
    WaitForHandler();
    s_RecordingOn = FALSE;
}

//=========================================================================

s32  VIFHELP_GetNEntries   ( void )
{
    WaitForHandler();
    return s_NVifSamples;
}

//=========================================================================

void VIFHELP_GetEntry      ( s32 Index, char** Label, s32* Type, s64* Ticks )
{
    WaitForHandler();
    ASSERT( Index>=0 && Index<s_NVifSamples );
    *Label = s_VifLabel[ s_VifSample[Index].Label ];
    *Type  = s_VifSample[Index].Type;
    *Ticks = s_VifSample[Index].Ticks;
}

//=========================================================================

void VIFHELP_DumpInfo      ( char* FileName )
{
    WaitForHandler();
    FileName = NULL;
}

//=========================================================================

void VIFHELP_DumpInfoToScreen( void )
{
    s32 i;
    s32 NEntries;

    WaitForHandler();
    NEntries = VIFHELP_GetNEntries();

    x_printf("******** VIFHELP REPORT ********\n");
    x_printf("NENTRIES:   %1d\n",NEntries);
    x_printf("VIFCOUNTER: %1ld\n",s_VifCounter);
    for( i=0; i<NEntries; i++ )
    {
        char* Label;
        s32 Type;
        s64 Ticks;
        
        VIFHELP_GetEntry( i, &Label, &Type, &Ticks );
        switch( Type )
        {
            case VIFHELP_TYPE_ENTRY: x_printf("%3d] %10ld ENTRY: %1s\n",i,Ticks,Label); break;
            case VIFHELP_TYPE_EXIT:  x_printf("%3d] %10ld EXIT : %1s\n",i,Ticks,Label); break;
            case VIFHELP_TYPE_TRACE: x_printf("%3d] %10ld TRACE: %1s\n",i,Ticks,Label); break;
        }
    }
    x_printf("*********************************\n");
   
}

//=========================================================================

void* VIFHELP_BuildTagUnpack( void*   TagPtr, 
                             s32    VUAddr, 
                             s32    Count, 
                             s32    SrcFormat, 
                             xbool  Signed,
                             xbool  Masked,
                             xbool  AbsoluteAddress )
{
    u32 VP;
    
    VP  = (u32)(VUAddr&0x0000FFFF);
    VP |= (u32)((Count<<16)&0x00FF0000);
    if( !AbsoluteAddress ) VP |= (1<<15);
    if( Signed == FALSE ) VP |= (1<<14);
    VP |= (((u32)(0x60|SrcFormat))<<24);  
    if( Masked == TRUE ) VP |= 0x10000000;      

    *((u32*)TagPtr) = VP;
    return (void*)((u32)TagPtr+4);
}

//==========================================================================

void* PS2HELP_BuildVIFTagMask( void* TagPtr,
                               s32 M03, s32 M02, s32 M01, s32 M00, 
                               s32 M07, s32 M06, s32 M05, s32 M04, 
                               s32 M11, s32 M10, s32 M09, s32 M08, 
                               s32 M15, s32 M14, s32 M13, s32 M12 )
{
    u32 VP0;
    u32 VP1;
    VP0 =   ((u32)0x20 << 24);
    VP1 =(u32)((M00<< 0)|(M01<< 2)|(M02<< 4)|(M03<< 6)|
               (M04<< 8)|(M05<<10)|(M06<<12)|(M07<<14)|
               (M08<<16)|(M09<<18)|(M10<<20)|(M11<<22)|
               (M12<<24)|(M13<<26)|(M14<<28)|(M15<<30));
    ((u32*)TagPtr)[0] = VP0;
    ((u32*)TagPtr)[1] = VP1;
    return (void*)((u32)TagPtr+8);            
}
                                
//=========================================================================

void* VIFHELP_BuildTagMask( void* TagPtr,
                           s32 M03, s32 M02, s32 M01, s32 M00, 
                           s32 M07, s32 M06, s32 M05, s32 M04, 
                           s32 M11, s32 M10, s32 M09, s32 M08, 
                           s32 M15, s32 M14, s32 M13, s32 M12 )
{
    u32 VP0;
    u32 VP1;
    VP0 =   ((u32)0x20 << 24);
    VP1 =(u32)((M00<< 0)|(M01<< 2)|(M02<< 4)|(M03<< 6)|
               (M04<< 8)|(M05<<10)|(M06<<12)|(M07<<14)|
               (M08<<16)|(M09<<18)|(M10<<20)|(M11<<22)|
               (M12<<24)|(M13<<26)|(M14<<28)|(M15<<30));
    ((u32*)TagPtr)[0] = VP0;
    ((u32*)TagPtr)[1] = VP1;
    return (void*)((u32)TagPtr+8);            
}

//=========================================================================
static
void* AlignWithNOPS( u32* pD, s32 NBytes )
{
    ASSERT( ((u32)pD & 0x03) == 0 );
    while( ( (((u32)pD)+NBytes) & 0x0F) != 0 )
    {
        *(pD)++ = 0;
    }

    return pD;
}

//=========================================================================

void* VIFHELP_Align16WithNOPS( void* pD )
{
    return AlignWithNOPS((u32*)pD,0);
}

//=========================================================================

void* VIFHELP_Align12WithNOPS( void* pD )
{
    return AlignWithNOPS((u32*)pD,4);
}

//=========================================================================

void* VIFHELP_Align8WithNOPS( void* pD )
{
    return AlignWithNOPS((u32*)pD,8);
}

//=========================================================================

void VIFHELP_DumpGIF( sceGifTag* Tag )
{
    // LIST BASICS
    x_printf("******** GIFTAG ********\n");
    x_printf("%8.8X %8.8X %8.8X %8.8X\n",((u32*)Tag)[3],((u32*)Tag)[2],((u32*)Tag)[1],((u32*)Tag)[0]);
    x_printf("NLOOP  | %5d |\n", (s32)Tag->NLOOP);
    x_printf("EOP    | %5d |\n", (s32)Tag->EOP);
    
    // FLAGS
    {
        x_printf("FLAG   | %5d | ", (s32)Tag->FLG);
        switch( Tag->FLG )
        {
            case 0x00: x_printf("PACKED mode\n"); break;
            case 0x01: x_printf("REGLIST mode\n"); break;
            case 0x02: x_printf("IMAGE mode\n"); break;
            case 0x03: x_printf("IMAGE2 mode\n"); break;
        }
    }
    
    // PRIM REGISTER DATA
    {
        u64 PRIM = (u64)Tag->PRIM;
        char PrimName[][16] = {"POINT","LINE","LINE STRIP","TRIANGLE",
                                "TRIANGLE STRIP","TRIANGLE FAN","SPRITE"};

        sceGsPrim PR = *((sceGsPrim*)&PRIM);
        x_printf("PRE    | %5d |\n", (s32)Tag->PRE);
        x_printf("PRIM   | %5d |\n", (s32)Tag->PRIM);
        
        if( Tag->PRE )
        {
            x_printf("         PRIMITIVE: %1s\n",PrimName[PR.PRIM]);
            x_printf("         SHADING:   %1s\n",(PR.IIP)?("Flat"):("Gourand"));
            x_printf("         TEXTURING: %1s\n",(PR.TME)?("On"):("Off"));
            x_printf("         FOGGING:   %1s\n",(PR.FGE)?("On"):("Off"));
            x_printf("         ALPHA:     %1s\n",(PR.ABE)?("On"):("Off"));
            x_printf("         AA:        %1s\n",(PR.AA1)?("On"):("Off"));
            x_printf("         TXTCOORD:  %1s\n",(PR.FST)?("UV"):("STQ"));
            x_printf("         CONTEXT:   %1s\n",(PR.CTXT)?("2"):("1"));
            x_printf("         INTERPFIX: %1s\n",(PR.FIX)?("On"):("Off"));
        }
    }

    // REGISTERS
    {
        s32 i;
        s32 R;
        char RegName[][16] = {"PRIM","RGBAQ","ST","UV","XYZF2","XYZ2","TEX0_1","TEX0_2",
                              "CLAMP_1","CLAMP_2","FOG","RESERVED","XYZF3","XYZ3","A+D","NOP"};
                              
        x_printf("NREG:  | %5d |\n", (s32)Tag->NREG);
        for( i=0; i<(s32)Tag->NREG; i++ )
        {
            R = (s32)(((((u64*)Tag)[1]) >> (i*4))&0x0F);
            x_printf("         %1s\n",RegName[R]);
        }
    }
    x_printf("************************\n");
    
}

//=========================================================================

void VIFHELP_BuildGifTag1( sceGifTag* Tag, s32   Mode, 
                                           s32   NRegs, 
                                           s32   NLoops, 
                                           xbool UsePrim, 
                                           s32   PrimType,
                                           u32   PrimFlags, 
                                           xbool EndOfPacket )
{
    ASSERT( (((u32)Tag)&0x0F) == 0 );
    ((u64*)Tag)[0] = 0;
    ((u64*)Tag)[1] = 0;
    Tag->FLG   = Mode;
    Tag->NREG  = NRegs;
    Tag->NLOOP = NLoops;
    Tag->PRE   = (UsePrim)?(1):(0);
    Tag->PRIM  = ((u32)PrimType) | (PrimFlags<<3);    
    Tag->EOP   = (EndOfPacket)?(1):(0);
}

//=========================================================================

void VIFHELP_BuildGifTag2( sceGifTag* Tag, s32 R0,  s32 R1,  s32 R2,  s32 R3 )
{
    Tag->REGS0 = (u32)R0;
    Tag->REGS1 = (u32)R1;
    Tag->REGS2 = (u32)R2;
    Tag->REGS3 = (u32)R3;
}

//=========================================================================

void VIFHELP_BuildGifTag3( sceGifTag* Tag, s32 R4,  s32 R5,  s32 R6,  s32 R7 )
{
    Tag->REGS4 = (u32)R4;
    Tag->REGS5 = (u32)R5;
    Tag->REGS6 = (u32)R6;
    Tag->REGS7 = (u32)R7;
}

//=========================================================================

void VIFHELP_BuildGifTag4( sceGifTag* Tag, s32 R8,  s32 R9,  s32 R10, s32 R11 )
{
    Tag->REGS8  = (u32)R8;
    Tag->REGS9  = (u32)R9;
    Tag->REGS10 = (u32)R10;
    Tag->REGS11 = (u32)R11;
}

//=========================================================================

void VIFHELP_BuildGifTag5( sceGifTag* Tag, s32 R12, s32 R13, s32 R14, s32 R15 )
{
    Tag->REGS12 = (u32)R12;
    Tag->REGS13 = (u32)R13;
    Tag->REGS14 = (u32)R14;
    Tag->REGS15 = (u32)R15;
}

//=========================================================================
/*
typedef struct
{
    sceDmaTag       DMA1;   // DMA to download vif instructions
    u32             NOP[3]; // NOP instruction for alignment
    u32             VifMPG; // Vif MPG instruction
    sceDmaTag       DMA2;   // DMA reference to microcode
} mpg_packet;

//#define SCE_VIF1_SET_MPG(vuaddr, num, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x4a) << 24) | ((u_int)(irq) << 31))

void* VIFHELP_BuildMPG( void* TagPtr, s32 DestAddr, void* SrcAddr, s32 Size )
{
    mpg_packet* M;
    u32 VP;
    
    // Check alignment and setup ptr
    ASSERT( (((u32)TagPtr)&0xF) == 0 );
    M = (mpg_packet*)TagPtr;
    
    // Setup initial dma
    DMAHELP_BuildTagCont( &M->DMA1, 4*4 );
    
    // Setup reference to microcode
    Size = ALIGN_16(Size);
    DMAHELP_BuildTagRef( &M->DMA2, (u32)SrcAddr, Size );
    
    // Build MPG instruction
    Size        = Size >> 3;
    DestAddr    = DestAddr >> 3;
    VP          = (u32)(DestAddr&0x0000FFFF);
    VP         |= (((u32)(0x4a))<<24);  
    VP         |= (u32)((Size<<16)&0x00FF0000);
    M->NOP[0]   = 0;
    M->NOP[1]   = 0;
    M->NOP[2]   = 0;
    M->VifMPG   = VP;

    // Return new location in dlist
    return (void*)((u32)TagPtr+sizeof(mpg_packet));
//    return (void*)((u32)TagPtr);

}
*/

void* VIFHELP_BuildMPG( void* TagPtr, s32 DestAddr, void* SrcAddr, s32 Size )
{

    sceDmaTag* pDMA;
    
    DestAddr = 0;
    
    // Check alignment and setup ptr
    ASSERT( (((u32)TagPtr)&0xF) == 0 );
    pDMA = (sceDmaTag*)TagPtr;
    
    // Setup reference to microcode
    Size = ALIGN_16(Size);
    DMAHELP_BuildTagRef( pDMA, (u32)SrcAddr, Size );
    
    // Return new location in dlist
    return (void*)((u32)TagPtr+sizeof(sceDmaTag));
}

//=========================================================================



                                

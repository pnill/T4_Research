////////////////////////////////////////////////////////////////////////////
// PS2 Helper routines
////////////////////////////////////////////////////////////////////////////
#include "x_debug.hpp"

#include "PS2Help.h"

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

void PS2HELP_BuildDMATagCont( sceDmaTag* pTag, s32 NBytes )
{
    // Make sure size is 16 byte multiple
    ASSERT( (NBytes & 0x0F) == 0 );
    pTag->qwc  = (u16)(NBytes>>4);
    pTag->id   = 0x10;
    pTag->next = NULL;
    pTag->mark = 0;
    pTag->pad[0] = 0;
    pTag->pad[1] = 0;
}

//==========================================================================

void PS2HELP_BuildDMATagRet( sceDmaTag* pTag, s32 NBytes )
{
    // Make sure size is 16 byte multiple
    ASSERT( (NBytes & 0x0F) == 0 );
    pTag->qwc  = (u16)(NBytes>>4);
    pTag->id   = 0x60;
    pTag->next = 0;
    pTag->mark = 0;
    pTag->pad[0] = 0;
    pTag->pad[1] = 0;
}

//==========================================================================

void* PS2HELP_BuildVIFTagUnpack ( void*  TagPtr, 
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

//==========================================================================

void PS2HELP_BuildGIFTag1( sceGifTag* Tag, s32   Mode, 
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

//==========================================================================

void PS2HELP_BuildGIFTag2( sceGifTag* Tag, s32 R0,  s32 R1,  s32 R2,  s32 R3 )
{
    Tag->REGS0 = (u32)R0;
    Tag->REGS1 = (u32)R1;
    Tag->REGS2 = (u32)R2;
    Tag->REGS3 = (u32)R3;
}

//==========================================================================

void PS2HELP_BuildGIFTag3( sceGifTag* Tag, s32 R4,  s32 R5,  s32 R6,  s32 R7 )
{
    Tag->REGS4 = (u32)R4;
    Tag->REGS5 = (u32)R5;
    Tag->REGS6 = (u32)R6;
    Tag->REGS7 = (u32)R7;
}

//==========================================================================

void PS2HELP_BuildGIFTag4( sceGifTag* Tag, s32 R8,  s32 R9,  s32 R10, s32 R11 )
{
    Tag->REGS8  = (u32)R8;
    Tag->REGS9  = (u32)R9;
    Tag->REGS10 = (u32)R10;
    Tag->REGS11 = (u32)R11;
}

//==========================================================================

void PS2HELP_BuildGIFTag5( sceGifTag* Tag, s32 R12, s32 R13, s32 R14, s32 R15 )
{
    Tag->REGS12 = (u32)R12;
    Tag->REGS13 = (u32)R13;
    Tag->REGS14 = (u32)R14;
    Tag->REGS15 = (u32)R15;
}


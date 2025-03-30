#ifndef __PS2HELP_H_INCLUDED__
#define __PS2HELP_H_INCLUDED__

#include "x_types.hpp"


////////////////////////////////////////////////////////////////////////////
// Some useful macros
////////////////////////////////////////////////////////////////////////////

#define VIFHELP_SET_STROW(irq)        (((u32)(irq) << 31) | ((u32)0x30 << 24))
#define VIFHELP_SET_MSCNT(irq)        (((u32)(irq) << 31) | ((u32)0x14 << 24))
#define VIFHELP_SET_MSCALF(addr,irq)  (((u32)(irq) << 31) | ((u32)0x15 << 24) | ((u32)(addr)))
#define VIFHELP_SET_MSCAL(addr,irq)   (((u32)(irq) << 31) | ((u32)0x17 << 24) | ((u32)(addr)))
#define VIFHELP_SET_NOP(irq)          (((u32)(irq) << 31))
#define VIFHELP_SET_STCYCL(wl,cl,irq) (((u32)(irq) << 31) | ((u32)0x01 << 24) | (u32)(cl) | ((u32)(wl) << 8))
#define VIFHELP_SET_ITOP(addr,irq)    (((u32)(irq) << 31) | ((u32)0x04 << 24) | ((u32)(addr)))
#define VIFHELP_SET_BASE(addr,irq)    (((u32)(irq) << 31) | ((u32)0x03 << 24) | ((u32)(addr)))
#define VIFHELP_SET_OFFSET(addr,irq)  (((u32)(irq) << 31) | ((u32)0x02 << 24) | ((u32)(addr)))
#define VIFHELP_SET_FLUSH(irq)        (((u32)(irq) << 31) | ((u32)0x11 << 24))
#define VIFHELP_SET_FLUSHA(irq)       (((u32)(irq) << 31) | ((u32)0x13 << 24))
#define VIFHELP_SET_FLUSHE(irq)       (((u32)(irq) << 31) | ((u32)0x10 << 24))

////////////////////////////////////////////////////////////////////////////
// Defines for the helper routines
////////////////////////////////////////////////////////////////////////////

#define VIFHELP_UNPACK_S_32     0x60
#define VIFHELP_UNPACK_S_16     0x61
#define VIFHELP_UNPACK_S_8      0x62

#define VIFHELP_UNPACK_V2_32    0x64
#define VIFHELP_UNPACK_V2_16    0x65
#define VIFHELP_UNPACK_V2_8     0x66

#define VIFHELP_UNPACK_V3_32    0x68
#define VIFHELP_UNPACK_V3_16    0x69
#define VIFHELP_UNPACK_V3_8     0x6A

#define VIFHELP_UNPACK_V4_32    0x6C
#define VIFHELP_UNPACK_V4_16    0x6D
#define VIFHELP_UNPACK_V4_8     0x6E
#define VIFHELP_UNPACK_V4_5     0x6F

#define VIFHELP_GIFREG_PRIM         0x00
#define VIFHELP_GIFREG_RGBAQ        0x01
#define VIFHELP_GIFREG_ST           0x02
#define VIFHELP_GIFREG_UV           0x03
#define VIFHELP_GIFREG_XYZF2        0x04
#define VIFHELP_GIFREG_XYZ2         0x05
#define VIFHELP_GIFREG_TEX0_1       0x06
#define VIFHELP_GIFREG_TEX0_2       0x07
#define VIFHELP_GIFREG_CLAMP_1      0x08
#define VIFHELP_GIFREG_CLAMP_2      0x09
#define VIFHELP_GIFREG_FOG          0x0A
#define VIFHELP_GIFREG_RESERVED     0x0B
#define VIFHELP_GIFREG_XYZF3        0x0C
#define VIFHELP_GIFREG_XYZ3         0x0D
#define VIFHELP_GIFREG_AD           0x0E
#define VIFHELP_GIFREG_NOP          0x0F
    
#define VIFHELP_GIFMODE_PACKED                  0
#define VIFHELP_GIFMODE_REGLIST                 1
#define VIFHELP_GIFMODE_IMAGE                   2
#define VIFHELP_GIFMODE_IMAGE2                  3

#define VIFHELP_GIFPRIMTYPE_POINT               0
#define VIFHELP_GIFPRIMTYPE_LINE                1
#define VIFHELP_GIFPRIMTYPE_LINESTRIP           2
#define VIFHELP_GIFPRIMTYPE_TRIANGLE            3
#define VIFHELP_GIFPRIMTYPE_TRIANGLESTRIP       4
#define VIFHELP_GIFPRIMTYPE_TRIANGLEFAN         5
#define VIFHELP_GIFPRIMTYPE_SPRITE              6

#define VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE        0x01
#define VIFHELP_GIFPRIMFLAGS_TEXTURE            0x02
#define VIFHELP_GIFPRIMFLAGS_FOG                0x04
#define VIFHELP_GIFPRIMFLAGS_ALPHA              0x08
#define VIFHELP_GIFPRIMFLAGS_ANTIALIAS          0x10
#define VIFHELP_GIFPRIMFLAGS_UV                 0x20
#define VIFHELP_GIFPRIMFLAGS_CONTEXT            0x40
#define VIFHELP_GIFPRIMFLAGS_INTERPFIX          0x80

////////////////////////////////////////////////////////////////////////////
// The helper structs (originally in eestruct.h and libdma.h)
////////////////////////////////////////////////////////////////////////////

typedef struct _sceDmaTag
{
    u16         qwc;        // Quad-word count
    u8          mark;       // mark
    u8          id;         // tag
    _sceDmaTag* next;       // next tag
    u32         pad[2];     // padding
} sceDmaTag;

typedef struct
{
    u32 NLOOP:15;
    u32 EOP:1;
    u32 pad16:16;
    u32 id:14;
    u32 PRE:1;
    u32 PRIM:11;
    u32 FLG:2;
    u32 NREG:4;
    u32 REGS0:4;
    u32 REGS1:4;
    u32 REGS2:4;
    u32 REGS3:4;
    u32 REGS4:4;
    u32 REGS5:4;
    u32 REGS6:4;
    u32 REGS7:4;
    u32 REGS8:4;
    u32 REGS9:4;
    u32 REGS10:4;
    u32 REGS11:4;
    u32 REGS12:4;
    u32 REGS13:4;
    u32 REGS14:4;
    u32 REGS15:4;
} sceGifTag;

////////////////////////////////////////////////////////////////////////////
// The helper routines
////////////////////////////////////////////////////////////////////////////

void  PS2HELP_BuildDMATagCont   ( sceDmaTag* pTag, s32 NBytes );
void  PS2HELP_BuildDMATagRet    ( sceDmaTag* pTag, s32 NBytes );
void* PS2HELP_BuildVIFTagUnpack ( void*  TagPtr, 
                                  s32    VUAddr, 
                                  s32    Count, 
                                  s32    SrcFormat, 
                                  xbool  Signed,
                                  xbool  Masked,
                                  xbool  AbsoluteAddress );
void* PS2HELP_BuildVIFTagMask   ( void* TagPtr,
                                  s32 M03, s32 M02, s32 M01, s32 M00, 
                                  s32 M07, s32 M06, s32 M05, s32 M04, 
                                  s32 M11, s32 M10, s32 M09, s32 M08, 
                                  s32 M15, s32 M14, s32 M13, s32 M12 );
void  PS2HELP_BuildGIFTag1      ( sceGifTag* Tag,
                                  s32   Mode,
                                  s32   NRegs, 
                                  s32   NLoops, 
                                  xbool UsePrim, 
                                  s32   PrimType,
                                  u32   PrimFlags, 
                                  xbool EndOfPacket );
void  PS2HELP_BuildGIFTag2      ( sceGifTag* Tag,
                                  s32 R0,
                                  s32 R1,
                                  s32 R2,
                                  s32 R3 );
void  PS2HELP_BuildGIFTag3      ( sceGifTag* Tag,
                                  s32 R4,
                                  s32 R5,
                                  s32 R6,
                                  s32 R7 );
void  PS2HELP_BuildGIFTag4      ( sceGifTag* Tag,
                                  s32 R8,
                                  s32 R9,
                                  s32 R10,
                                  s32 R11 );
void  PS2HELP_BuildGIFTag5      ( sceGifTag* Tag,
                                  s32 R12,
                                  s32 R13,
                                  s32 R14,
                                  s32 R15 );

#endif  // __PS2HELP_H_INCLUDED__
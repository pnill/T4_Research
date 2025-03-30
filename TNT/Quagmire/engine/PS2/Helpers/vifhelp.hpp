///////////////////////////////////////////////////////////////////////////
//
//  VIFHELP.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef VIFHELP_H
#define VIFHELP_H

///////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_H
#include "x_types.hpp"
#endif

#include <eekernel.h>
#include <eeregs.h>
#include <eestruct.h>

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////

#define VIFHELP_TYPE_ENTRY     1
#define VIFHELP_TYPE_EXIT      2
#define VIFHELP_TYPE_TRACE     3

#define VIFHELP_SET_STCYCL(wl,cl,irq) ((u32)(cl) | ((u32)(wl) << 8) | ((u32)0x01 << 24) | ((u32)(irq) << 31))


///////////////////////////////////////////////////////////////////////////

void VIFHELP_InitModule    ( void );
void VIFHELP_KillModule    ( void );
s32  VIFHELP_CreateLabel   ( char* Label );
void VIFHELP_SetMarker     ( void* Dst, s32 LabelIndex, s32 Type );
void VIFHELP_ClearBuffer   ( void );
void VIFHELP_StartRecording( void );
void VIFHELP_EndRecording  ( void );
s32  VIFHELP_GetNEntries   ( void );
void VIFHELP_GetEntry      ( s32 Index, char** Label, s32* Type, s64* Ticks );
void VIFHELP_DumpInfo      ( char* FileName );
void VIFHELP_DumpInfoToScreen( void );
                                
                                
///////////////////////////////////////////////////////////////////////////

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

void* VIFHELP_BuildTagUnpack( void*   TagPtr, 
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

#define VIFHELP_MASK_ASIS       0
#define VIFHELP_MASK_ROW        1
#define VIFHELP_MASK_COL        2
#define VIFHELP_MASK_BLOCKED    3

void* VIFHELP_BuildTagMask( void* TagPtr,
                           s32 M03, s32 M02, s32 M01, s32 M00, 
                           s32 M07, s32 M06, s32 M05, s32 M04, 
                           s32 M11, s32 M10, s32 M09, s32 M08, 
                           s32 M15, s32 M14, s32 M13, s32 M12 );
                           
void VIFHELP_BuildTagMSCAL( void );
void* VIFHELP_Align16WithNOPS( void* Tag );
void* VIFHELP_Align12WithNOPS( void* Tag );
void* VIFHELP_Align8WithNOPS( void* pD );

void* VIFHELP_BuildMPG( void* TagPtr, s32 DestAddr, void* SrcAddr, s32 Size );

///////////////////////////////////////////////////////////////////////////

#define     VIFHELP_GIFREG_PRIM         0x00
#define     VIFHELP_GIFREG_RGBAQ        0x01
#define     VIFHELP_GIFREG_ST           0x02
#define     VIFHELP_GIFREG_UV           0x03
#define     VIFHELP_GIFREG_XYZF2        0x04
#define     VIFHELP_GIFREG_XYZ2         0x05
#define     VIFHELP_GIFREG_TEX0_1       0x06
#define     VIFHELP_GIFREG_TEX0_2       0x07
#define     VIFHELP_GIFREG_CLAMP_1      0x08
#define     VIFHELP_GIFREG_CLAMP_2      0x09
#define     VIFHELP_GIFREG_FOG          0x0A
#define     VIFHELP_GIFREG_RESERVED     0x0B
#define     VIFHELP_GIFREG_XYZF3        0x0C
#define     VIFHELP_GIFREG_XYZ3         0x0D
#define     VIFHELP_GIFREG_AD           0x0E
#define     VIFHELP_GIFREG_NOP          0x0F
    
#define     VIFHELP_GIFMODE_PACKED      0
#define     VIFHELP_GIFMODE_REGLIST     1
#define     VIFHELP_GIFMODE_IMAGE       2
#define     VIFHELP_GIFMODE_IMAGE2      3

#define     VIFHELP_GIFPRIMTYPE_POINT               0
#define     VIFHELP_GIFPRIMTYPE_LINE                1
#define     VIFHELP_GIFPRIMTYPE_LINESTRIP           2
#define     VIFHELP_GIFPRIMTYPE_TRIANGLE            3
#define     VIFHELP_GIFPRIMTYPE_TRIANGLESTRIP       4
#define     VIFHELP_GIFPRIMTYPE_TRIANGLEFAN         5
#define     VIFHELP_GIFPRIMTYPE_SPRITE              6

#define     VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE        0x01
#define     VIFHELP_GIFPRIMFLAGS_TEXTURE            0x02
#define     VIFHELP_GIFPRIMFLAGS_FOG                0x04
#define     VIFHELP_GIFPRIMFLAGS_ALPHA              0x08
#define     VIFHELP_GIFPRIMFLAGS_ANTIALIAS          0x10
#define     VIFHELP_GIFPRIMFLAGS_UV                 0x20
#define     VIFHELP_GIFPRIMFLAGS_CONTEXT            0x40
#define     VIFHELP_GIFPRIMFLAGS_INTERPFIX          0x80

void VIFHELP_DumpGIF( sceGifTag* Tag );

void VIFHELP_BuildGifTag1( sceGifTag* Tag, s32   Mode, 
                                           s32   NRegs, 
                                           s32   NLoops, 
                                           xbool UsePrim, 
                                           s32   PrimType,
                                           u32   PrimFlags, 
                                           xbool EndOfPacket );
                                           
void VIFHELP_BuildGifTag2( sceGifTag* Tag, s32 R0,  s32 R1,  s32 R2,  s32 R3 );
void VIFHELP_BuildGifTag3( sceGifTag* Tag, s32 R4,  s32 R5,  s32 R6,  s32 R7 );
void VIFHELP_BuildGifTag4( sceGifTag* Tag, s32 R8,  s32 R9,  s32 R10, s32 R11 );
void VIFHELP_BuildGifTag5( sceGifTag* Tag, s32 R12, s32 R13, s32 R14, s32 R15 );


///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////

#endif

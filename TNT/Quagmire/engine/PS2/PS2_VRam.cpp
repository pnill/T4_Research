////////////////////////////////////////////////////////////////////////////
// PS2_VRAM.cpp - The PS2 VRAM sub-system
//
//  The supported bitmap sizes are those that fall under 256 blocks. These
//  would be:
//
//    8 BIT:
//      256 color palette = 4 blocks
//
//      512 x 128 = 256 blocks
//      512 x 64  = 128 blocks
//      512 x 32  = 64 blocks
//      512 x 16  = 32 blocks
//      512 x 8   = 32 blocks (1)
//      256 x 256 = 256 blocks
//      256 x 128 = 128 blocks
//      256 x 64  = 64 blocks
//      256 x 32  = 32 blocks
//      256 x 16  = 16 blocks
//      256 x 8   = 16 blocks (1)
//      128 x 128 = 64 blocks
//      128 x 64  = 32 blocks
//      128 x 32  = 16 blocks
//      128 x 16  = 8 blocks
//      128 x 8   = 8 blocks (1)
//      64 x 64   = 16 blocks
//      64 x 32   = 8 blocks
//      64 x 16   = 4 blocks
//      64 x 8    = 4 blocks (1)
//      32 x 32   = 4 blocks
//      32 x 16   = 2 blocks
//      32 x 8    = 2 blocks (1)
//      16 x 16   = 1 block
//      16 x 8    = 1 block (1)
//      8 x 8     = 1 block (1) & (2)
//
//    32 BIT:
//      512 x 32  = 256 blocks
//      512 x 16  = 128 blocks
//      512 x 8   = 64 blocks
//      256 x 64  = 256 blocks
//      256 x 32  = 128 blocks
//      256 x 16  = 64 blocks
//      256 x 8   = 32 blocks
//      128 x 128 = 256 blocks
//      128 x 64  = 128 blocks
//      128 x 32  = 64 blocks
//      128 x 16  = 32 blocks
//      128 x 8   = 16 blocks
//      64 x 64   = 64 blocks
//      64 x 32   = 32 blocks
//      64 x 16   = 16 blocks
//      64 x 8    = 8 blocks
//      32 x 32   = 16 blocks
//      32 x 16   = 8 blocks
//      32 x 8    = 4 blocks
//      16 x 16   = 4 blocks
//      16 x 8    = 2 blocks
//      8 x 8     = 1 block
//
//  1: because block height MUST be 16, half of each block is unused
//  2: because block width MUST be 16, half of each block is unused
//
//  A NOTE ABOUT THE TERMINOLOGY USED THROUGHOUT THIS FILE:
//
//      VRAM    -  The area of memory where the frame buffer, z-buffer, and
//                 texture data resides. On the PS2, it is 4 meg. (duh!!!)
//
//      page    -  A page in VRAM is 8k. There are 512 pages available in VRAM.
//
//      block   -  A block in VRAM is 256 bytes. There are 32 blocks in a
//                 page, and 16384 blocks available in VRAM.
//
//      frame1  -  The main rendering surface, either the back-buffer or
//                 front-buffer at any given time.
//
//      frame2  -  The main rendering surface, either the back-buffer or
//                 front-buffer at any given time.
//
//      zbuff   -  The z-buffer.
//
//      slot    -  My term for an area of memory where texture data resides.
//                 One slot contains texture data for one image or one mip
//                 level. Slots can be 256,128,64,32,16,8,4,2, or 1 block(s)
//                 in size.
//
//      bank    -  My term for a collection of similar-sized slots. There
//                 is a bank for each size of slot, and a special bank
//                 which contains clut data.
//
//      address -  In general, addresses are specified in terms of blocks.
//                 This is how the playstation2 likes to think.
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_bitmap.hpp"
#include "x_time.hpp"
#include "x_memory.hpp"

#include "Q_VRAM.hpp"
#include "Q_Engine.hpp"
#include "Q_PS2.hpp"

#include "DMAHelp.hpp"
#include "VIFHelp.hpp"

#include "libgraph.h"

////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#undef VRAM_Register
#undef VRAM_DBG_Register
#undef VRAM_DBG_Dump
#undef VRAM_DBG_FreeSlots
#undef VRAM_DBG_UsedSlots


//#define FORCE_RELOAD            1

static const s32 kMaxMIPs = 4;
static const s32 kCLUTAddress = 16380;
static const s32 kMIPAddresses[ kMaxMIPs ] =
{
    kCLUTAddress - 1660,
    kCLUTAddress - 636,
    kCLUTAddress - 292,
    kCLUTAddress - 228
};


////////////////////////////////////////////////////////////////////////////
// STRUCTS
////////////////////////////////////////////////////////////////////////////

typedef struct _SVRAMTexture
{
    xbool Registered;
//    s64   TimeStamp;
    s16   VRamSlots[MAX_MIP_LEVELS];
    s16   ClutSlot;
    s16   TexAddr[MAX_MIP_LEVELS];
    s16   ClutAddr;

#ifdef X_DEBUG
    char            Filename[16];       // Name of file texture was registered from
    s32             LineNumber;         // Line number in file texture was registered from
#endif

} SVRAMTexture;

typedef struct
{
    sceDmaTag   DMATagHeader;
    u32         VIF[4];
    sceGifTag   GIFHeader;
    u64         AD[8];
    sceGifTag   GIFImage;
    sceDmaTag   DMATagImage;
} SVRAMMipLoadData;

typedef struct
{
    sceDmaTag   DMATagHeader;
    u32         VIF[4];
    sceGifTag   GIFHeader;
    u64         AD[8];
    sceGifTag   GIFImage;
    sceDmaTag   DMATagImage;
} SVRAMClutLoadData;

typedef struct
{
    sceDmaTag   DMATagHeader;
    u32         VIF[4];
    sceGifTag   GIF;
    u64         AD[8];
} SVRAMTexEnvLoadData;


//---------

typedef struct TextureOrCLUTPacket
{
    u32         VIF[ 4 ];
    sceGifTag   GIFHeader;
    u64         AD[ 8 ];
    sceGifTag   GIFImage;
};

typedef struct TextureOrCLUTContainer
{
    s32                 Usage;
    TextureOrCLUTPacket Packet;
};

typedef struct TextureEnvironmentPacket
{
    u32         VIF[ 4 ];
    sceGifTag   GIFHeader;
    u64         AD[ 8 ];
};

typedef struct TextureEnvironmentContainer
{
    s32                         Usage;
    TextureEnvironmentPacket    Packet;
};

typedef struct MIPPacket
{
    s32         Usage;
    s32         nMIPs;
    s32         MIPs[ MAX_MIP_LEVELS ];
    s32         ENVs[ MAX_MIP_LEVELS ];
};

typedef struct MIPENVPacket
{
    sceDmaTag   DMATag;
    u32         VIF[ 4 ];
    sceGifTag   GIFTag;
    u64         AD[ 6 ];
};

typedef struct TextureDataPacket
{
    u32             iMIPPacket;
    u32             iCLUTPacket;

    u32             iContext;
    MIPENVPacket    MIPEnv[ MAX_MIP_LEVELS ];


    sceDmaTag       MIPDmaTags[ MAX_MIP_LEVELS ];
    sceDmaTag       CLUTDmaTag;
};


////////////////////////////////////////////////////////////////////////////
// STATICS
////////////////////////////////////////////////////////////////////////////
/*
static s32          s_Bank256RoundRobinEntry = 0;
static s32          s_Bank128RoundRobinEntry = 0;
static s32          s_Bank64RoundRobinEntry = 0;
static s32          s_Bank32RoundRobinEntry = 0;
static s32          s_BankClutRoundRobinEntry = 0;
*/

static s32*         s_pNTextureUploads = NULL;
static s32*         s_pNBytesUploaded = NULL;
static s32          s_VRamSlotTextureID[VRAM_NSLOTS];
static s32          s_VRamClutTextureID[VRAM_NCLUTS];
static SVRAMTexture s_VRamTextures[MAX_VRAM_TEXTURES];
static f32          s_MipK = 0.0f;
static s32          s_UWrapMode = WRAP_MODE_TILE;
static s32          s_VWrapMode = WRAP_MODE_TILE;
static f32          s_UMin = 0.0f;
static f32          s_UMax = 1.0f;
static f32          s_VMin = 0.0f;
static f32          s_VMax = 1.0f;

static SVRAMMipLoadData     s_BasicMipLoadData;
static SVRAMClutLoadData    s_BasicClutLoadData;
static SVRAMTexEnvLoadData  s_BasicTexEnvLoadData;


#define MAX_CLUT_PACKETS            4
#define MAX_MIP_PACKETS             128
#define MAX_MIP_CONTAINERS          128
#define MAX_TEXENV_CONTAINERS       128


static s32                          s_nCLUTPackets = 0;
static TextureOrCLUTContainer       s_CLUTPackets[ MAX_CLUT_PACKETS ];
static s32                          s_nMIPPackets = 0;
static MIPPacket                    s_pMIPPackets[ MAX_MIP_PACKETS ];

static s32                          s_nMIPs = 0;
static TextureOrCLUTContainer       s_MIPs[ MAX_MIP_CONTAINERS ];
static s32                          s_nENVs = 0;
static TextureEnvironmentContainer  s_ENVs[ MAX_TEXENV_CONTAINERS ];

static TextureDataPacket            *s_pTextureDataPackets[ MAX_VRAM_TEXTURES ];

static s32                          s_FlushList[ MAX_VRAM_TEXTURES ];
static s32                          s_nFlushCount = 0;


//PSTAT( s_NVRAMTexturesRegistered );

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

static
s32 GetPS2WrapMode( s32 QuagWrapMode )
{
    switch ( QuagWrapMode )
    {
    case WRAP_MODE_TILE:     return 0x00;
    case WRAP_MODE_CLAMP:    return 0x01;
    case WRAP_MODE_REGION:   return 0x02;
    case WRAP_MODE_R_REPEAT: return 0x03;
    case WRAP_MODE_MIRROR:
        ASSERTS( FALSE, "Mirror mode not supported on PS2" );
        return 0;
    }
    return 0;
}

//==========================================================================

static
s32 ConvertUV( s32 WrapMode, s32 TexDimension, f32 ValueToConvert )
{
    if ( (WrapMode == WRAP_MODE_REGION) ||
         (WrapMode == WRAP_MODE_R_REPEAT) )
    {
        //---   convert from the range [0.0f, 1.0f] to [0, TexDimension]
        return (s32)(ValueToConvert * (f32)TexDimension);
    }

    //---   for normal wrapping modes (clamp and tile), the Min and Max
    //      U and V values aren't used, so it doesn't matter what they
    //      are. We'll just use zero...
    return 0;
}

//==========================================================================

static s32 FindMIPData( TextureOrCLUTPacket &MIPPacket )
{
    s32 i;
    for (i=0; i<MAX_MIP_CONTAINERS; ++i)
    {
        TextureOrCLUTPacket *pWorkMIPPacket = &(s_MIPs[ i ].Packet);

        if (   (s_MIPs[ i ].Usage > 0)
            && (pWorkMIPPacket->VIF[ 0 ]            == MIPPacket.VIF[ 0 ])
            && (pWorkMIPPacket->VIF[ 1 ]            == MIPPacket.VIF[ 1 ])
            && (pWorkMIPPacket->VIF[ 2 ]            == MIPPacket.VIF[ 2 ])
            && (pWorkMIPPacket->VIF[ 3 ]            == MIPPacket.VIF[ 3 ])
            && (*(u32*)(&pWorkMIPPacket->GIFHeader) == *(u32*)(&MIPPacket.GIFHeader))
            && (pWorkMIPPacket->AD[ 0 ]             == MIPPacket.AD[ 0 ])
            && (pWorkMIPPacket->AD[ 1 ]             == MIPPacket.AD[ 1 ])
            && (pWorkMIPPacket->AD[ 2 ]             == MIPPacket.AD[ 2 ])
            && (pWorkMIPPacket->AD[ 3 ]             == MIPPacket.AD[ 3 ])
            && (pWorkMIPPacket->AD[ 4 ]             == MIPPacket.AD[ 4 ])
            && (pWorkMIPPacket->AD[ 5 ]             == MIPPacket.AD[ 5 ])
            && (pWorkMIPPacket->AD[ 6 ]             == MIPPacket.AD[ 6 ])
            && (pWorkMIPPacket->AD[ 7 ]             == MIPPacket.AD[ 7 ]) 
            && (*(u32*)(&pWorkMIPPacket->GIFImage)  == *(u32*)(&MIPPacket.GIFImage)))
        {
            s_MIPs[ i ].Usage ++;
            return i;
        }
    }

    // add it
    for (i=0; i<MAX_MIP_CONTAINERS; ++i)
    {
        if (s_MIPs[ i ].Usage <= 0)
        {
            x_memcpy( &(s_MIPs[ i ].Packet), &MIPPacket, sizeof( TextureOrCLUTPacket ) );
            s_MIPs[ i ].Usage = 1;
            s_nMIPs ++;
            ASSERT( s_nMIPs < MAX_MIP_CONTAINERS );

            return i;
        }
    }

    ASSERT( s_nMIPs < MAX_MIP_CONTAINERS );
    return (s_nMIPs - 1);
}

//==========================================================================

static s32 FindENVData( TextureEnvironmentPacket &ENVPacket )
{
    s32 i;
    for (i=0; i<MAX_TEXENV_CONTAINERS; ++i)
    {
        TextureEnvironmentPacket *pWorkENVPacket = &(s_ENVs[ i ].Packet);

        if (   (s_ENVs[ i ].Usage > 0)
            && (pWorkENVPacket->VIF[ 0 ]            == ENVPacket.VIF[ 0 ])
            && (pWorkENVPacket->VIF[ 1 ]            == ENVPacket.VIF[ 1 ])
            && (pWorkENVPacket->VIF[ 2 ]            == ENVPacket.VIF[ 2 ])
            && (pWorkENVPacket->VIF[ 3 ]            == ENVPacket.VIF[ 3 ])
            && (*(u32*)(&pWorkENVPacket->GIFHeader) == *(u32*)(&ENVPacket.GIFHeader))
            && (pWorkENVPacket->AD[ 0 ]             == ENVPacket.AD[ 0 ])
            && (pWorkENVPacket->AD[ 1 ]             == ENVPacket.AD[ 1 ])
            && (pWorkENVPacket->AD[ 2 ]             == ENVPacket.AD[ 2 ])
            && (pWorkENVPacket->AD[ 3 ]             == ENVPacket.AD[ 3 ])
            && (pWorkENVPacket->AD[ 4 ]             == ENVPacket.AD[ 4 ])
            && (pWorkENVPacket->AD[ 5 ]             == ENVPacket.AD[ 5 ])
            && (pWorkENVPacket->AD[ 6 ]             == ENVPacket.AD[ 6 ])
            && (pWorkENVPacket->AD[ 7 ]             == ENVPacket.AD[ 7 ]))
        {
            s_ENVs[ i ].Usage ++;
            return i;
        }
    }

    // add it
    for (i=0; i<MAX_TEXENV_CONTAINERS; ++i)
    {
        if (s_ENVs[ i ].Usage <= 0)
        {
            x_memcpy( &(s_ENVs[ i ].Packet), &ENVPacket, sizeof( TextureEnvironmentPacket ) );
            s_ENVs[ i ].Usage = 1;
            s_nENVs ++;
            ASSERT( s_nENVs < MAX_TEXENV_CONTAINERS );

            return i;
        }
    }

    ASSERT( s_nENVs < MAX_TEXENV_CONTAINERS );
    return (s_nENVs - 1);
}

//==========================================================================

static s32 FindMIPPacket( TextureOrCLUTPacket *pMIPPackets, TextureEnvironmentPacket *pENVPackets, s32 nCount )
{
    s32 i;
    for (i=0; i<MAX_MIP_PACKETS; ++i)
    {
        if ((s_pMIPPackets[ i ].Usage > 0) && s_pMIPPackets[ i ].nMIPs == nCount)
        {
            s32 j;
            xbool bIsIt = TRUE;
            for (j=0; j<nCount; ++j)
            {
                s32 iMIPIndex = s_pMIPPackets[ i ].MIPs[ j ];
                s32 iENVIndex = s_pMIPPackets[ i ].ENVs[ j ];
                
                TextureOrCLUTPacket *pWorkMIPPacket = &(s_MIPs[ iMIPIndex ].Packet);
                TextureEnvironmentPacket *pWorkENVPacket = &(s_ENVs[ iENVIndex ].Packet);

                if ((s_MIPs[ iMIPIndex ].Usage <= 0) || (s_ENVs[ iENVIndex ].Usage <= 0))
                    continue;

                if (   pWorkMIPPacket->VIF[ 0 ]             != pMIPPackets[ j ].VIF[ 0 ]
                    || pWorkMIPPacket->VIF[ 1 ]             != pMIPPackets[ j ].VIF[ 1 ]
                    || pWorkMIPPacket->VIF[ 2 ]             != pMIPPackets[ j ].VIF[ 2 ]
                    || pWorkMIPPacket->VIF[ 3 ]             != pMIPPackets[ j ].VIF[ 3 ]
                    || *(u32*)(&pWorkMIPPacket->GIFHeader)  != *(u32*)(&pMIPPackets[ j ].GIFHeader)
                    || pWorkMIPPacket->AD[ 0 ]              != pMIPPackets[ j ].AD[ 0 ]
                    || pWorkMIPPacket->AD[ 1 ]              != pMIPPackets[ j ].AD[ 1 ]
                    || pWorkMIPPacket->AD[ 2 ]              != pMIPPackets[ j ].AD[ 2 ]
                    || pWorkMIPPacket->AD[ 3 ]              != pMIPPackets[ j ].AD[ 3 ]
                    || pWorkMIPPacket->AD[ 4 ]              != pMIPPackets[ j ].AD[ 4 ]
                    || pWorkMIPPacket->AD[ 5 ]              != pMIPPackets[ j ].AD[ 5 ]
                    || pWorkMIPPacket->AD[ 6 ]              != pMIPPackets[ j ].AD[ 6 ]
                    || pWorkMIPPacket->AD[ 7 ]              != pMIPPackets[ j ].AD[ 7 ]
                    || *(u32*)(&pWorkMIPPacket->GIFImage)   != *(u32*)(&pMIPPackets[ j ].GIFImage)
                    || pWorkENVPacket->VIF[ 0 ]             != pENVPackets[ j ].VIF[ 0 ]
                    || pWorkENVPacket->VIF[ 1 ]             != pENVPackets[ j ].VIF[ 1 ]
                    || pWorkENVPacket->VIF[ 2 ]             != pENVPackets[ j ].VIF[ 2 ]
                    || pWorkENVPacket->VIF[ 3 ]             != pENVPackets[ j ].VIF[ 3 ]
                    || *(u32*)(&pWorkENVPacket->GIFHeader)  != *(u32*)(&pENVPackets[ j ].GIFHeader)
                    || pWorkENVPacket->AD[ 0 ]              != pENVPackets[ j ].AD[ 0 ]
                    || pWorkENVPacket->AD[ 1 ]              != pENVPackets[ j ].AD[ 1 ]
                    || pWorkENVPacket->AD[ 2 ]              != pENVPackets[ j ].AD[ 2 ]
                    || pWorkENVPacket->AD[ 3 ]              != pENVPackets[ j ].AD[ 3 ]
                    || pWorkENVPacket->AD[ 4 ]              != pENVPackets[ j ].AD[ 4 ]
                    || pWorkENVPacket->AD[ 5 ]              != pENVPackets[ j ].AD[ 5 ]
                    || pWorkENVPacket->AD[ 6 ]              != pENVPackets[ j ].AD[ 6 ] 
                    || pWorkENVPacket->AD[ 7 ]              != pENVPackets[ j ].AD[ 7 ])
                {
                    bIsIt = FALSE;
                }
            }
            if (bIsIt)
            {
                if (i == 0xc)
                {
                    s32 dbp = 0;
                    dbp ++;
                }
                s_pMIPPackets[ i ].Usage ++;
                s32 j;
                for (j=0; j<nCount; ++j)
                {
                    s_MIPs[ s_pMIPPackets[ i ].MIPs[ j ] ].Usage ++;
                    s_ENVs[ s_pMIPPackets[ i ].ENVs[ j ] ].Usage ++;
                }
                return i;
            }
        }
    }

    // add it
    for (i=0; i<MAX_MIP_PACKETS; ++i)
    {
        if (s_pMIPPackets[ i ].Usage <= 0)
        {
            s_pMIPPackets[ i ].nMIPs = nCount;
            s_pMIPPackets[ i ].Usage = 1;
            s_nMIPPackets ++;
            ASSERT( s_nMIPPackets < MAX_MIP_PACKETS );

            s32 j;
            for (j=0; j<nCount; ++j)
            {
                s_pMIPPackets[ i ].MIPs[ j ] = FindMIPData( pMIPPackets[ j ] );
                s_pMIPPackets[ i ].ENVs[ j ] = FindENVData( pENVPackets[ j ] );
            }
            return i;
        }
    }

    ASSERT( s_nMIPPackets < MAX_MIP_PACKETS );
    return (s_nMIPPackets - 1);
}

//==========================================================================

static s32 FindCLUTPacket( TextureOrCLUTPacket &pPacket )
{
    s32 i;
    for (i=0; i<MAX_CLUT_PACKETS; ++i)
    {
        TextureOrCLUTPacket *pWorkCLUTPacket = &(s_CLUTPackets[ i ].Packet);

        if (  (s_CLUTPackets[ i ].Usage > 0)
            && pWorkCLUTPacket->VIF[ 0 ]            == pPacket.VIF[ 0 ]
            && pWorkCLUTPacket->VIF[ 1 ]            == pPacket.VIF[ 1 ]
            && pWorkCLUTPacket->VIF[ 2 ]            == pPacket.VIF[ 2 ]
            && pWorkCLUTPacket->VIF[ 3 ]            == pPacket.VIF[ 3 ]
            && *(u32*)(&pWorkCLUTPacket->GIFHeader) == *(u32*)(&pPacket.GIFHeader)
            && pWorkCLUTPacket->AD[ 0 ]             == pPacket.AD[ 0 ]
            && pWorkCLUTPacket->AD[ 1 ]             == pPacket.AD[ 1 ]
            && pWorkCLUTPacket->AD[ 2 ]             == pPacket.AD[ 2 ]
            && pWorkCLUTPacket->AD[ 3 ]             == pPacket.AD[ 3 ]
            && pWorkCLUTPacket->AD[ 4 ]             == pPacket.AD[ 4 ]
            && pWorkCLUTPacket->AD[ 5 ]             == pPacket.AD[ 5 ]
            && pWorkCLUTPacket->AD[ 6 ]             == pPacket.AD[ 6 ]
            && pWorkCLUTPacket->AD[ 7 ]             == pPacket.AD[ 7 ]
            && *(u32*)(&pWorkCLUTPacket->GIFImage)  == *(u32*)(&pPacket.GIFImage) )
        {
            s_CLUTPackets[ i ].Usage ++;
            return i;
        }
    }

    // add it
    for (i=0; i<MAX_CLUT_PACKETS; ++i)
    {
        if (s_CLUTPackets[ i ].Usage <= 0)
        {
            x_memcpy( &(s_CLUTPackets[ i ].Packet), &pPacket, sizeof( TextureOrCLUTPacket ) );
            s_CLUTPackets[ i ].Usage = 1;
            s_nCLUTPackets ++;
            ASSERT( s_nCLUTPackets < MAX_CLUT_PACKETS );

            return i;
        }
    }

    ASSERT( s_nCLUTPackets < MAX_CLUT_PACKETS );
    return (s_nCLUTPackets - 1);
}

//==========================================================================

void PS2_SetMipK( f32 MipK )
{
    s_MipK = MipK;
}

//==========================================================================

void VRAM_Init( void )
{
    s32 i, j;

    for ( i = 0; i < VRAM_NSLOTS; i++ )
        s_VRamSlotTextureID[i] = -1;

    for ( i = 0; i < VRAM_NCLUTS; i++ )
        s_VRamClutTextureID[i] = -1;

    for ( i = 0; i < MAX_VRAM_TEXTURES; i++ )
    {
        s_VRamTextures[i].Registered = FALSE;
        s_VRamTextures[i].ClutSlot = -1;
        for ( j = 0; j < MAX_MIP_LEVELS; j++ )
            s_VRamTextures[i].VRamSlots[j] = -1;
    }

    s_nCLUTPackets = 0;
    x_memset( s_CLUTPackets, 0, sizeof(TextureOrCLUTContainer) * MAX_CLUT_PACKETS );

    s_nMIPPackets = 0;
    x_memset( s_pMIPPackets, 0, sizeof(MIPPacket) * MAX_MIP_PACKETS );

    s_nMIPs = 0;
    x_memset( s_MIPs, 0, sizeof(TextureOrCLUTContainer) * MAX_MIP_CONTAINERS );

    s_nENVs = 0;
    x_memset( s_ENVs, 0, sizeof(TextureEnvironmentContainer) * MAX_TEXENV_CONTAINERS );

    x_memset( s_pTextureDataPackets, 0, sizeof(TextureDataPacket*) * MAX_VRAM_TEXTURES );

    x_memset( s_FlushList, 0, sizeof(s32) * MAX_VRAM_TEXTURES );
    s_nFlushCount = 0;



    //---   set up an initial mip-load data which will be copied over
    //      (this is an optimization so we don't rebuild this on the fly!)
    DMAHELP_BuildTagCont( &s_BasicMipLoadData.DMATagHeader, 7*16 ); // 1 for VIF, 1 for GifTag, 4 for AD, 1 for GifTag
    s_BasicMipLoadData.VIF[0] = 0;
    s_BasicMipLoadData.VIF[1] = 0;
    s_BasicMipLoadData.VIF[2] = SCE_VIF1_SET_FLUSH( 0 );  // is this necessary?????
    s_BasicMipLoadData.VIF[3] = 0;  // this will be filled in later, and will be SCE_VIF1_SET_DIRECT( 6 + ImageSize, 0 )
    VIFHELP_BuildGifTag1( &s_BasicMipLoadData.GIFHeader, VIFHELP_GIFMODE_PACKED, 1, 4, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &s_BasicMipLoadData.GIFHeader, VIFHELP_GIFREG_AD, 0, 0, 0 );
    s_BasicMipLoadData.AD[0] = 0;   // this will be filled in later, and will be SCE_GS_SET_BITBLTBUF
    s_BasicMipLoadData.AD[1] = SCE_GS_BITBLTBUF;
    s_BasicMipLoadData.AD[2] = SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0x00 );
    s_BasicMipLoadData.AD[3] = SCE_GS_TRXPOS;
    s_BasicMipLoadData.AD[4] = 0;   // this will be filled in later, and will be SCE_GS_SET_TRXREG
    s_BasicMipLoadData.AD[5] = SCE_GS_TRXREG;
    s_BasicMipLoadData.AD[6] = SCE_GS_SET_TRXDIR( 0x00 );
    s_BasicMipLoadData.AD[7] = SCE_GS_TRXDIR;
    VIFHELP_BuildGifTag1( &s_BasicMipLoadData.GIFImage, VIFHELP_GIFMODE_IMAGE, 0, 16, FALSE, 0, 0, TRUE );
    // the gif tag for the image will need to be altered later
    DMAHELP_BuildTagRef( &s_BasicMipLoadData.DMATagImage, NULL, 16 );
    // the dma tag for the image will need to be altered later

    //---   set up an initial clut-load data which will be copied over
    DMAHELP_BuildTagCont( &s_BasicClutLoadData.DMATagHeader, 7*16 );    // 1 for VIF, 1 for GifTag, 4 for AD, 1 for GifTag
    s_BasicClutLoadData.VIF[0] = 0;
    s_BasicClutLoadData.VIF[1] = 0;
    s_BasicClutLoadData.VIF[2] = SCE_VIF1_SET_FLUSH( 0 );   // is this necessary?????
    s_BasicClutLoadData.VIF[3] = 0; // this will be filled in later, and will be SCE_VIF1_SET_DIRECT( 6 + ClutSize, 0 )
    VIFHELP_BuildGifTag1( &s_BasicClutLoadData.GIFHeader, VIFHELP_GIFMODE_PACKED, 1, 4, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &s_BasicClutLoadData.GIFHeader, VIFHELP_GIFREG_AD, 0, 0, 0 );
    s_BasicClutLoadData.AD[0] = 0;  // this will be filled in later, and will be SCE_GS_SET_BITBLTBUF
    s_BasicClutLoadData.AD[1] = SCE_GS_BITBLTBUF;
    s_BasicClutLoadData.AD[2] = SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0x00 );
    s_BasicClutLoadData.AD[3] = SCE_GS_TRXPOS;
    s_BasicClutLoadData.AD[4] = 0;  // this will be filled in later, and will be SCE_GS_SET_TRXREG
    s_BasicClutLoadData.AD[5] = SCE_GS_TRXREG;
    s_BasicClutLoadData.AD[6] = SCE_GS_SET_TRXDIR( 0x00 );
    s_BasicClutLoadData.AD[7] = SCE_GS_TRXDIR;
    VIFHELP_BuildGifTag1( &s_BasicClutLoadData.GIFImage, VIFHELP_GIFMODE_IMAGE, 0, 16, FALSE, 0, 0, TRUE );
    // the gif tag for the clut will need to be altered later
    DMAHELP_BuildTagRef( &s_BasicClutLoadData.DMATagImage, NULL, 16 );
    // the dma tag for the clut will need to be altered later

    //---   set up an intiial tex-environment setting which will be copied over
    DMAHELP_BuildTagCont( &s_BasicTexEnvLoadData.DMATagHeader, (1+1+4)*16 );
    s_BasicTexEnvLoadData.VIF[0] = 0;
    s_BasicTexEnvLoadData.VIF[1] = 0;
    s_BasicTexEnvLoadData.VIF[2] = SCE_VIF1_SET_FLUSH( 0 ); // is this necessary?
    s_BasicTexEnvLoadData.VIF[3] = SCE_VIF1_SET_DIRECT( 5, 0 );
    VIFHELP_BuildGifTag1( &s_BasicTexEnvLoadData.GIF, VIFHELP_GIFMODE_PACKED, 1, 4, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &s_BasicTexEnvLoadData.GIF, VIFHELP_GIFREG_AD, 0, 0, 0 );
    s_BasicTexEnvLoadData.AD[0] = 0;
    s_BasicTexEnvLoadData.AD[1] = SCE_GS_TEXFLUSH;
    s_BasicTexEnvLoadData.AD[2] = SCE_GS_SET_TEX1_1( 1, 0, 1, 1, 0, 0, 0 );
    s_BasicTexEnvLoadData.AD[3] = SCE_GS_TEX1_1;
    s_BasicTexEnvLoadData.AD[4] = 0;    // this will be filled in later and will be SCE_GS_SET_TEX0_1
    s_BasicTexEnvLoadData.AD[5] = SCE_GS_TEX0_1;
    s_BasicTexEnvLoadData.AD[6] = 0;    // this will be filled in later and will be SCE_GS_SET_CLAMP_1
    s_BasicTexEnvLoadData.AD[7] = SCE_GS_CLAMP_1;


    // Prebuild a bunch of data packets for sending textures across DMA
    // we are assuming a single data destination address for each texture context
    // Each context has specific addresses for each MIP as the CLUT
}

//==========================================================================

void VRAM_Kill( void )
{
    s32 i, j;

    VRAM_DeactivateAll();

    s_pNTextureUploads = NULL;
    s_pNBytesUploaded = NULL;
    
    for ( i = 0; i < VRAM_NSLOTS; i++ )
        s_VRamSlotTextureID[i] = -1;

    for ( i = 0; i < VRAM_NCLUTS; i++ )
        s_VRamClutTextureID[i] = -1;

    for ( i = 0; i < MAX_VRAM_TEXTURES; i++ )
    {
        s_VRamTextures[i].Registered = FALSE;
        s_VRamTextures[i].ClutSlot = -1;
        for ( j = 0; j < MAX_MIP_LEVELS; j++ )
            s_VRamTextures[i].VRamSlots[j] = -1;
    }
}

//==========================================================================
/*
static
s32 GetNVramBlocks( x_bitmap::format F, s32 W, s32 H )
{
    s32 NHorzPages, NVertPages;

    switch ( F )
    {
    case x_bitmap::FMT_P4_ABGR_8888:
    case x_bitmap::FMT_P4_UBGR_8888:
        //---   I don't use the ALIGN macros, because they will use a full 32 bits, and
        //      the compiler won't be able to use andi (and immediate)
        NHorzPages = ((W+127) & 0xff80) >> 7;
        NVertPages = ((H+127) & 0xff80) >> 7;
        //NHorzPages = ALIGN_128(W) / 128;
        //NVertPages = ALIGN_128(H) / 128;
        break;
    case x_bitmap::FMT_P8_ABGR_8888:
    case x_bitmap::FMT_P8_UBGR_8888:
        //---   I don't use the ALIGN macros, because they will use a full 32 bits, and
        //      the compiler won't be able to use andi (and immediate)
        NHorzPages = ((W+127) & 0xff80) >> 7;
        NVertPages = ((H+63) & 0xffc0) >> 6;
        //NHorzPages = ALIGN_128(W) / 128;
        //NVertPages = ALIGN_64(H) / 64;
        break;
    case x_bitmap::FMT_16_ABGR_1555:
    case x_bitmap::FMT_16_UBGR_1555:
        //---   I don't use the ALIGN macros, because they will use a full 32 bits, and
        //      the compiler won't be able to use andi (and immediate)
        NHorzPages = ((W+63) & 0xffc0) >> 6;
        NVertPages = ((H+63) & 0xffc0) >> 6;
        //NHorzPages = ALIGN_64(W) / 64;
        //NVertPages = ALIGN_64(H) / 64;
        break;
    case x_bitmap::FMT_32_ABGR_8888:
        //---   I don't use the ALIGN macros, because they will use a full 32 bits, and
        //      the compiler won't be able to use andi (and immediate)
        NHorzPages = ((W+63) & 0xffc0) >> 6;
        NVertPages = ((H+31) & 0xffe0) >> 5;
        //NHorzPages = ALIGN_64(W) / 64;
        //NVertPages = ALIGN_32(H) / 32;
        break;
    default:
        //---   I don't use the ALIGN macros, because they will use a full 32 bits, and
        //      the compiler won't be able to use andi (and immediate)
        NHorzPages = ((W+127) & 0xff80) >> 7;
        NVertPages = ((H+127) & 0xff80) >> 7;
        //NHorzPages = ALIGN_128(W) / 128;
        //NVertPages = ALIGN_128(H) / 128;
        ASSERTS( FALSE, "Unsupported bitmap format." );
        break;
    }
    
    return NHorzPages * NVertPages * 32;
}
*/
//==========================================================================

void VRAM_Register( x_bitmap& BMP, s32 iContext )
{
    s32 i;
    s32 format;

    ASSERT( BMP.SanityCheck() );
    format = BMP.GetFormat();
    ASSERT( (format == x_bitmap::FMT_P4_ABGR_8888) ||
            (format == x_bitmap::FMT_P4_UBGR_8888) ||
            (format == x_bitmap::FMT_P8_ABGR_8888) ||
            (format == x_bitmap::FMT_P8_UBGR_8888) ||
            (format == x_bitmap::FMT_16_ABGR_1555) ||
            (format == x_bitmap::FMT_16_UBGR_1555) ||
            (format == x_bitmap::FMT_32_ABGR_8888) );
    ASSERT( BMP.GetVRAMID() == -1 );
//    ASSERT( GetNVramBlocks( (x_bitmap::format)format, BMP.GetWidth(), BMP.GetHeight() ) <= 256 );

    for ( i = 0; i < MAX_VRAM_TEXTURES; i++ )
    {
        if ( s_VRamTextures[i].Registered == FALSE )
            break;
    }

    ASSERTS( i < MAX_VRAM_TEXTURES, "No free VRAM slots" );

    if( i >= MAX_VRAM_TEXTURES )
        return;

    s_VRamTextures[ i ].Registered = TRUE;
    BMP.SetVRAMID( i );

    s32 iID = BMP.GetVRAMID();

    for (i=0; i<s_nFlushCount; ++i)
    {
        if (s_FlushList[ i ] == iID)
        {
            // must perform the flush NOW for this data.
            s32 iMIPPacket = s_pTextureDataPackets[ s_FlushList[ i ] ]->iMIPPacket;
            s32 j;
            for (j=0; j<s_pMIPPackets[ iMIPPacket ].nMIPs; ++j)
            {
                s_MIPs[ s_pMIPPackets[ iMIPPacket ].MIPs[ j ] ].Usage --;
                if (s_MIPs[ s_pMIPPackets[ iMIPPacket ].MIPs[ j ] ].Usage <= 0)
                    s_nMIPs --;

                s_ENVs[ s_pMIPPackets[ iMIPPacket ].ENVs[ j ] ].Usage --;
                if (s_ENVs[ s_pMIPPackets[ iMIPPacket ].ENVs[ j ] ].Usage <= 0)
                    s_nENVs--;
            }

            s_pMIPPackets[ iMIPPacket ].Usage--;
            if (s_pMIPPackets[ iMIPPacket ].Usage <= 0)
                s_nMIPPackets--;


            s32 iClutPacket = s_pTextureDataPackets[ s_FlushList[ i ] ]->iCLUTPacket;

            s_CLUTPackets[ iClutPacket ].Usage--;
            if( s_CLUTPackets[ iClutPacket ].Usage <= 0 )
                s_nCLUTPackets--;


            if( s_pTextureDataPackets[ s_FlushList[ i ] ] != NULL )
                x_free( s_pTextureDataPackets[ s_FlushList[ i ] ] );

            s_pTextureDataPackets[ s_FlushList[ i ] ] = NULL;
        }

        s_FlushList[ i ] = -1;
    }


    if (!s_pTextureDataPackets[ iID ])
    {
        s_pTextureDataPackets[ iID ] = (TextureDataPacket*)x_malloc( sizeof(TextureDataPacket) );
        ASSERT( s_pTextureDataPackets[ iID ] );
//        x_memset( &s_pTextureDataPackets[ iID ]->MIPDmaTags, 0, sizeof( sceDmaTag ) * MAX_MIP_LEVELS );
    }
    {
        TextureOrCLUTPacket TempPacket[ MAX_MIP_LEVELS ];
        TextureEnvironmentPacket TempENVPacket[ MAX_MIP_LEVELS ];
        for (i=0; i<=BMP.GetNMips(); ++i)
        {
            //***MIPPackets
            xbool bIsCompressed = BMP.IsPS2Compressed();
            s32 BPP = BMP.GetBPP();
            s32 W = BMP.GetWidth( i );
            s32 H = BMP.GetHeight( i );
            s32 SwizzledW;
            s32 SwizzledH;
            s32 BlockAddr;
            s32 Format;
            s32 ImageSize = (BMP.GetDataSize( i ) >> 4);
            s32 TextureSize = ((W * H * BPP) >> 7);

            if ( BMP.IsDataSwizzled() && (W >= 16) )
            {
                ASSERTS( BMP.GetBPP() == 8, "No support for swizzled 4- and 16-bit images yet!" );
                SwizzledW = ((W * BPP) >> 4);
                SwizzledH = ((H * BPP) >> 4);
                Format = SCE_GS_PSMCT32;
            }
            else
            {
                SwizzledW = W;
                SwizzledH = H;
                Format = VRAM_PS2_GetFormat( BMP );
            }

            s32 w_h_size = W*H*BPP;
            s32 actual_size = BMP.GetDataSize( i );
            if (bIsCompressed && (actual_size > w_h_size))
            {
                W >>= 1;
                H >>= 1;
                SwizzledW >>= 1;
                SwizzledH >>= 1;
            }

            s32 sbw = MAX(1, (SwizzledW >> 6) );
            BlockAddr = kMIPAddresses[ i ];


            TempPacket[ i ].VIF[ 0 ] = 0;
            TempPacket[ i ].VIF[ 1 ] = 0;
            TempPacket[ i ].VIF[ 2 ] = 0;//SCE_VIF1_SET_FLUSH( 0 );
            TempPacket[ i ].VIF[ 3 ] = SCE_VIF1_SET_DIRECT( 6 + (ImageSize), 0 );
            VIFHELP_BuildGifTag1( &TempPacket[ i ].GIFHeader, VIFHELP_GIFMODE_PACKED, 1, 4, FALSE, 0, 0, TRUE );
            VIFHELP_BuildGifTag2( &TempPacket[ i ].GIFHeader, VIFHELP_GIFREG_AD, 0, 0, 0 );
            TempPacket[ i ].AD[ 0 ] = SCE_GS_SET_BITBLTBUF( BlockAddr, sbw, Format, BlockAddr, sbw, Format);
            TempPacket[ i ].AD[ 1 ] = SCE_GS_BITBLTBUF;
            TempPacket[ i ].AD[ 2 ] = SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0x00 );
            TempPacket[ i ].AD[ 3 ] = SCE_GS_TRXPOS;
            TempPacket[ i ].AD[ 4 ] = SCE_GS_SET_TRXREG( SwizzledW, SwizzledH );
            TempPacket[ i ].AD[ 5 ] = SCE_GS_TRXREG;
            TempPacket[ i ].AD[ 6 ] = SCE_GS_SET_TRXDIR( 0x00 );
            TempPacket[ i ].AD[ 7 ] = SCE_GS_TRXDIR;
            VIFHELP_BuildGifTag1( &TempPacket[ i ].GIFImage, VIFHELP_GIFMODE_IMAGE, 0, TextureSize, FALSE, 0, 0, TRUE );

            DMAHELP_BuildTagRef( &s_pTextureDataPackets[ iID ]->MIPDmaTags[ i ], NULL, 16 );
            s_pTextureDataPackets[ iID ]->MIPDmaTags[ i ].qwc = ImageSize;
            s_pTextureDataPackets[ iID ]->MIPDmaTags[ i ].next = (sceDmaTag*)BMP.GetDataPtr( i );

            //***EnvironmentPackets
            s32 TBW = (BMP.GetWidth() >> 6);
            s32 PSM = VRAM_PS2_GetFormat( BMP );
            s32 LogW = VRAM_PS2_GetLog2( W );
            s32 LogH = VRAM_PS2_GetLog2( H );

            TempENVPacket[ i ].VIF[ 0 ] = 0;
            TempENVPacket[ i ].VIF[ 1 ] = 0;
            TempENVPacket[ i ].VIF[ 2 ] = 0;//SCE_VIF1_SET_FLUSH( 0 );
            TempENVPacket[ i ].VIF[ 3 ] = SCE_VIF1_SET_DIRECT( 5, 0 );
            VIFHELP_BuildGifTag1( &TempENVPacket[ i ].GIFHeader, VIFHELP_GIFMODE_PACKED, 1, 4, FALSE, 0, 0, TRUE );
            VIFHELP_BuildGifTag2( &TempENVPacket[ i ].GIFHeader, VIFHELP_GIFREG_AD, 0, 0, 0 );
            TempENVPacket[ i ].AD[ 0 ] = 0;
            TempENVPacket[ i ].AD[ 1 ] = SCE_GS_TEXFLUSH;

            if (iContext)
            {
                TempENVPacket[ i ].AD[ 2 ] = SCE_GS_SET_TEX1_2( 1, 0, 1, 1, 0, 0, 0 );
                TempENVPacket[ i ].AD[ 3 ] = SCE_GS_TEX1_2;
                TempENVPacket[ i ].AD[ 4 ] = SCE_GS_SET_TEX0_2( kMIPAddresses[ i ], TBW, PSM, LogW, LogH, 1, 0, kCLUTAddress, 0, 0, 0, 1 );
                TempENVPacket[ i ].AD[ 5 ] = SCE_GS_TEX0_2;
                TempENVPacket[ i ].AD[ 6 ] = SCE_GS_SET_CLAMP_2( GetPS2WrapMode( s_UWrapMode ),
                                                            GetPS2WrapMode( s_VWrapMode ),
                                                            ConvertUV( s_UWrapMode, W, s_UMin ),
                                                            ConvertUV( s_UWrapMode, W, s_UMax ),
                                                            ConvertUV( s_VWrapMode, H, s_VMin ),
                                                            ConvertUV( s_VWrapMode, H, s_VMax ) );
                TempENVPacket[ i ].AD[ 7 ] = SCE_GS_CLAMP_2;
            }
            else
            {
//                TempENVPacket[ i ].AD[ 2 ] = SCE_GS_SET_TEX1_1( 1, 0, 0, 1, 0, 0, 0 );
                TempENVPacket[ i ].AD[ 2 ] = SCE_GS_SET_TEX1_1( 1, 0, 1, 1, 0, 0, 0 );
                TempENVPacket[ i ].AD[ 3 ] = SCE_GS_TEX1_1;
                TempENVPacket[ i ].AD[ 4 ] = SCE_GS_SET_TEX0_1( kMIPAddresses[ i ], TBW, PSM, LogW, LogH, 1, 0, kCLUTAddress, 0, 0, 0, 1 );
                TempENVPacket[ i ].AD[ 5 ] = SCE_GS_TEX0_1;
                TempENVPacket[ i ].AD[ 6 ] = SCE_GS_SET_CLAMP_1( GetPS2WrapMode( s_UWrapMode ),
                                                            GetPS2WrapMode( s_VWrapMode ),
                                                            ConvertUV( s_UWrapMode, W, s_UMin ),
                                                            ConvertUV( s_UWrapMode, W, s_UMax ),
                                                            ConvertUV( s_VWrapMode, H, s_VMin ),
                                                            ConvertUV( s_VWrapMode, H, s_VMax ) );
                TempENVPacket[ i ].AD[ 7 ] = SCE_GS_CLAMP_1;
            }
        }
        s_pTextureDataPackets[ iID ]->iMIPPacket = FindMIPPacket( (TextureOrCLUTPacket *)(&TempPacket), (TextureEnvironmentPacket *)(&TempENVPacket), BMP.GetNMips() + 1 );

        s32 tbws[ MAX_MIP_LEVELS ];
        s32 tbps[ MAX_MIP_LEVELS ];
        for (i=0; i<MAX_MIP_LEVELS; ++i)
        {
            s32 MIPID = i+1;
            if (MIPID < BMP.GetNMips())
            {
                tbws[ i ] = MAX( 1, (BMP.GetWidth( MIPID ) >> 6 ) );
                tbps[ i ] = kMIPAddresses[ MIPID ];
            }
            else
            {
                tbws[ i ] = MAX( 1, (BMP.GetWidth( 0 ) >> 6 ) );
                tbps[ i ] = kMIPAddresses[ 0 ];
            }
        }

        for (i=0; i<MAX_MIP_LEVELS; ++i)
        {
            s_pTextureDataPackets[ iID ]->MIPEnv[ i ].VIF[ 0 ] = 0;
            s_pTextureDataPackets[ iID ]->MIPEnv[ i ].VIF[ 1 ] = 0;
            s_pTextureDataPackets[ iID ]->MIPEnv[ i ].VIF[ 2 ] = 0;//SCE_VIF1_SET_FLUSH( 0 );
            s_pTextureDataPackets[ iID ]->MIPEnv[ i ].VIF[ 3 ] = SCE_VIF1_SET_DIRECT( 4, 0 );
            VIFHELP_BuildGifTag1( &s_pTextureDataPackets[ iID ]->MIPEnv[ i ].GIFTag, VIFHELP_GIFMODE_PACKED, 1, 3, FALSE, 0, 0, TRUE );
            VIFHELP_BuildGifTag2( &s_pTextureDataPackets[ iID ]->MIPEnv[ i ].GIFTag, VIFHELP_GIFREG_AD, 0, 0, 0 );
            if (iContext)
            {
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 0 ] = SCE_GS_SET_TEX1_2( 1, 0, 1, 1, 0, 0, 0 );
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 1 ] = SCE_GS_TEX1_2;
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 2 ] = SCE_GS_SET_MIPTBP1_2( tbps[ 0 ], tbws[ 0 ], tbps[ 1 ], tbws[ 1 ], tbps[ 2 ], tbws[ 2 ] );
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 3 ] = SCE_GS_MIPTBP1_2;
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 4 ] = SCE_GS_SET_MIPTBP2_2( tbps[ 3 ], tbws[ 3 ], tbps[ 4 ], tbws[ 4 ], tbps[ 5 ], tbws[ 5 ] );
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 5 ] = SCE_GS_MIPTBP2_2;
            }
            else
            {
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 0 ] = SCE_GS_SET_TEX1_1( 1, 0, 1, 1, 0, 0, 0 );
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 1 ] = SCE_GS_TEX1_1;
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 2 ] = SCE_GS_SET_MIPTBP1_1( tbps[ 0 ], tbws[ 0 ], tbps[ 1 ], tbws[ 1 ], tbps[ 2 ], tbws[ 2 ] );
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 3 ] = SCE_GS_MIPTBP1_1;
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 4 ] = SCE_GS_SET_MIPTBP2_1( tbps[ 3 ], tbws[ 3 ], tbps[ 4 ], tbws[ 4 ], tbps[ 5 ], tbws[ 5 ] );
                s_pTextureDataPackets[ iID ]->MIPEnv[ i ].AD[ 5 ] = SCE_GS_MIPTBP2_1;
            }
        }
        s_pTextureDataPackets[ iID ]->iContext = iContext;
    }

    {
        s32 CLUTAddr = kCLUTAddress;
        s32 BPP = BMP.GetBPP();
        s32 ClutSize;
        if (BPP == 8)
            ClutSize = 64;
        else
            ClutSize = 4;


        TextureOrCLUTPacket TempCLUTPacket;
        TempCLUTPacket.VIF[ 0 ] = 0;
        TempCLUTPacket.VIF[ 1 ] = 0;
        TempCLUTPacket.VIF[ 2 ] = SCE_VIF1_SET_FLUSH( 0 );
        TempCLUTPacket.VIF[ 3 ] = SCE_VIF1_SET_DIRECT( 6 + ClutSize, 0 );
        VIFHELP_BuildGifTag1( &TempCLUTPacket.GIFHeader, VIFHELP_GIFMODE_PACKED, 1, 4, FALSE, 0, 0, TRUE );
        VIFHELP_BuildGifTag2( &TempCLUTPacket.GIFHeader, VIFHELP_GIFREG_AD, 0, 0, 0 );
        if (BPP == 8)
        {
            TempCLUTPacket.AD[ 0 ] = SCE_GS_SET_BITBLTBUF( 0, 0, 0, CLUTAddr, 1, SCE_GS_PSMCT32 );
            TempCLUTPacket.AD[ 4 ] = SCE_GS_SET_TRXREG( 16, 16 );
        }
        else
        {
            TempCLUTPacket.AD[ 0 ] = SCE_GS_SET_BITBLTBUF( 0, 0, 0, CLUTAddr, 1, SCE_GS_PSMCT32 );
            TempCLUTPacket.AD[ 4 ] = SCE_GS_SET_TRXREG( 8, 2 );
        }
        TempCLUTPacket.AD[ 1 ] = SCE_GS_BITBLTBUF;
        TempCLUTPacket.AD[ 2 ] = SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0x00 );
        TempCLUTPacket.AD[ 3 ] = SCE_GS_TRXPOS;
        TempCLUTPacket.AD[ 5 ] = SCE_GS_TRXREG;
        TempCLUTPacket.AD[ 6 ] = SCE_GS_SET_TRXDIR( 0x00 );
        TempCLUTPacket.AD[ 7 ] = SCE_GS_TRXDIR;
        VIFHELP_BuildGifTag1( &TempCLUTPacket.GIFImage, VIFHELP_GIFMODE_IMAGE, 0, ClutSize, FALSE, 0, 0, TRUE );

        s_pTextureDataPackets[ iID ]->iCLUTPacket = FindCLUTPacket( TempCLUTPacket );

        DMAHELP_BuildTagRef( &s_pTextureDataPackets[ iID ]->CLUTDmaTag, NULL, 16 );
        s_pTextureDataPackets[ iID ]->CLUTDmaTag.qwc = ClutSize;
        s_pTextureDataPackets[ iID ]->CLUTDmaTag.next = (sceDmaTag*)BMP.GetClutPtr();
    }
}

//==========================================================================

static void AddToFlushList( s32 iIndex )
{
    s_FlushList[ s_nFlushCount ] = iIndex;
    s_nFlushCount ++;
    ASSERT( s_nFlushCount < MAX_VRAM_TEXTURES );
}

//==========================================================================

void VRAM_Flush( void )
{
    s32 i;
    for (i=0; i<s_nFlushCount; ++i)
    {
        if (s_FlushList[ i ] != -1)
        {
            s32 iMIPPacket = s_pTextureDataPackets[ s_FlushList[ i ] ]->iMIPPacket;
            s32 j;
            for (j=0; j<s_pMIPPackets[ iMIPPacket ].nMIPs; ++j)
            {
                s_MIPs[ s_pMIPPackets[ iMIPPacket ].MIPs[ j ] ].Usage --;
                if (s_MIPs[ s_pMIPPackets[ iMIPPacket ].MIPs[ j ] ].Usage <= 0)
                    s_nMIPs --;

                s_ENVs[ s_pMIPPackets[ iMIPPacket ].ENVs[ j ] ].Usage --;
                if (s_ENVs[ s_pMIPPackets[ iMIPPacket ].ENVs[ j ] ].Usage <= 0)
                    s_nENVs--;
            }

            s_pMIPPackets[ iMIPPacket ].Usage--;
            if (s_pMIPPackets[ iMIPPacket ].Usage <= 0)
                s_nMIPPackets--;

            s32 iClutPacket = s_pTextureDataPackets[ s_FlushList[ i ] ]->iCLUTPacket;

            s_CLUTPackets[ iClutPacket ].Usage--;
            if( s_CLUTPackets[ iClutPacket ].Usage <= 0 )
                s_nCLUTPackets--;


            if( s_pTextureDataPackets[ s_FlushList[ i ] ] != NULL )
                x_free( s_pTextureDataPackets[ s_FlushList[ i ] ] );

            s_pTextureDataPackets[ s_FlushList[ i ] ] = NULL;
        }
    }

    s_nFlushCount = 0;
}

//==========================================================================

void VRAM_UnRegister( x_bitmap& BMP )
{
    s32 iID = BMP.GetVRAMID();

    ASSERT( (iID >= 0) && (iID < MAX_VRAM_TEXTURES) );
    ASSERT( s_VRamTextures[ iID ].Registered );

    VRAM_Deactivate( BMP );

    s_VRamTextures[ iID ].Registered = FALSE;
    BMP.SetVRAMID( -1 );

    AddToFlushList( iID );
}

//==========================================================================

xbool VRAM_IsRegistered( x_bitmap &BMP )
{
    s32 ID = BMP.GetVRAMID();
    if (ID < 0)
        return FALSE;
    return (s_VRamTextures[ ID ].Registered );
}

//==========================================================================
/*
static
s32 FindRoundRobinSlot( s32 NBlocks, s32& rBlockAddr )
{
    s32     i;
    s32     Entry;
    s32     BankOffset;
    s32     SlotSize;
    s32     BankAddr;
    s32     NSlotsInBank;
    s32*    pRoundRobinEntry;

    //---   get some information based on the # of blocks
    if ( NBlocks <= 32 )
    {
        BankOffset = BANK_32_OFFSET;
        NSlotsInBank = VRAM_BANK_SIZE_32;
        BankAddr = VRAM_BANK_32_ADDR;
        SlotSize = 32;
        pRoundRobinEntry = &s_Bank32RoundRobinEntry;
    }
    else
    if ( NBlocks <= 64 )
    {
        BankOffset = BANK_64_OFFSET;
        NSlotsInBank = VRAM_BANK_SIZE_64;
        BankAddr = VRAM_BANK_64_ADDR;
        SlotSize = 64;
        pRoundRobinEntry = &s_Bank64RoundRobinEntry;
    }
    else
    if ( NBlocks <= 128 )
    {
        BankOffset = BANK_128_OFFSET;
        NSlotsInBank = VRAM_BANK_SIZE_128;
        BankAddr = VRAM_BANK_128_ADDR;
        SlotSize = 128;
        pRoundRobinEntry = &s_Bank128RoundRobinEntry;
    }
    else
    {
        BankOffset = BANK_256_OFFSET;
        NSlotsInBank = VRAM_BANK_SIZE_256;
        BankAddr = VRAM_BANK_256_ADDR;
        SlotSize = 256;
        pRoundRobinEntry = &s_Bank256RoundRobinEntry;
    }

    //---   now we know which texture to replace
    Entry = *pRoundRobinEntry + BankOffset;

    //---   since we know the round-robin slot will be replaced, make sure we
    //      mark the current texture as no longer loaded
    if ( s_VRamSlotTextureID[Entry] >= 0 )
    {
        for ( i = 0; i < MAX_MIP_LEVELS; i++ )
        {
            if ( s_VRamTextures[s_VRamSlotTextureID[Entry]].VRamSlots[i] == Entry )
                s_VRamTextures[s_VRamSlotTextureID[Entry]].VRamSlots[i] = -1;
        }
    }

    //---   figure out the user address
    rBlockAddr = BankAddr + (*pRoundRobinEntry) * SlotSize;

    //---   get prepared for the next round
    (*pRoundRobinEntry) += 1;
    if ( *pRoundRobinEntry >= NSlotsInBank )
        *pRoundRobinEntry = 0;

    return Entry;
}
*/
//==========================================================================

s32 VRAM_PS2_GetFormat( x_bitmap& BMP )
{
    switch ( BMP.GetFormat() )
    {
    case x_bitmap::FMT_P4_ABGR_8888:    return SCE_GS_PSMT4;
    case x_bitmap::FMT_P4_UBGR_8888:    return SCE_GS_PSMT4;
    case x_bitmap::FMT_P8_ABGR_8888:    return SCE_GS_PSMT8;
    case x_bitmap::FMT_P8_UBGR_8888:    return SCE_GS_PSMT8;
    case x_bitmap::FMT_16_ABGR_1555:    return SCE_GS_PSMCT16;
    case x_bitmap::FMT_16_UBGR_1555:    return SCE_GS_PSMCT16;
    case x_bitmap::FMT_32_ABGR_8888:    return SCE_GS_PSMCT32;
    default:
        ASSERTS( FALSE, "Unsupported bitmap format." );
        break;
    }

    return 0;
}

//==========================================================================
/*
static
s32 LoadMip( s32 ID, x_bitmap& BMP, s32 MipLevel )
{
    s32                 LRU;
    s32                 W, H, SwizzledW, SwizzledH;
    s32                 NBlocks;
    s32                 BlockAddr;
    s32                 Format;
    s32                 ImageSize;
    SVRAMMipLoadData*   pMipData;
    s32                 BPP = BMP.GetBPP();

    //---   figure out how many blocks this mip level requires
    W = BMP.GetWidth( MipLevel );
    H = BMP.GetHeight( MipLevel );
    if ( BMP.IsDataSwizzled() && (W >= 16) )
    {
        ASSERTS( BMP.GetBPP() == 8, "No support for swizzled 4- and 16-bit images yet!" );
        SwizzledW = ((W * BPP) >> 4);
        SwizzledH = ((H * BPP) >> 4);
        Format = SCE_GS_PSMCT32;
    }
    else
    {
        SwizzledW = W;
        SwizzledH = H;
        Format = VRAM_PS2_GetFormat( BMP );
    }
    NBlocks = GetNVramBlocks( BMP.GetFormat(), W, H );
    ASSERT( NBlocks );
    ASSERTS( NBlocks <= 256, "Bitmap dimensions too large." );

    //---   now, find an open slot of the right size
    LRU = FindRoundRobinSlot( NBlocks, BlockAddr );

    //---   fill the slot with the texture
    s_VRamTextures[ID].VRamSlots[MipLevel] = LRU;
    s_VRamSlotTextureID[LRU] = ID;

    //---   build the upload info to the display list
    ImageSize = ((W * H * BPP) >> 7);

    pMipData = (SVRAMMipLoadData*)pDList;
    pDList += sizeof(SVRAMMipLoadData);
    *pMipData = s_BasicMipLoadData;
    pMipData->VIF[3] = SCE_VIF1_SET_DIRECT( 6 + ImageSize, 0 );
    s32 sbw = MAX(1, (SwizzledW >> 6) );
    pMipData->AD[0] = SCE_GS_SET_BITBLTBUF( BlockAddr, sbw, Format, BlockAddr, sbw, Format );
    pMipData->AD[4] = SCE_GS_SET_TRXREG( SwizzledW, SwizzledH );

    pMipData->GIFImage.NLOOP = ImageSize;

    pMipData->DMATagImage.qwc = ImageSize;
    pMipData->DMATagImage.next = (sceDmaTag*)BMP.GetDataPtr( MipLevel );

    //---   update the stats
    if ( s_pNTextureUploads )
        *s_pNTextureUploads += 1;
    if ( s_pNBytesUploaded )
        *s_pNBytesUploaded += (ImageSize << 4);

    return BlockAddr;
}
*/
//==========================================================================

s32 VRAM_PS2_GetLog2( s32 Dimension )
{
    switch ( Dimension )
    {
    case 1024:  return 10;
    case 512:   return 9;
    case 256:   return 8;
    case 128:   return 7;
    case 64:    return 6;
    case 32:    return 5;
    case 16:    return 4;
    case 8:     return 3;
    }

    ASSERTS( FALSE, "Bitmap dimensions must be a power of two" );
    return 0;
}

//==========================================================================
/*
static
void ActivateEnvironment( x_bitmap& rBMP, s16 TextureBasePtr, s16 CLUTBasePtr, s32 MipLevel )
{
    s32                     nTextureBufferWidth;
    s32                     W, H;
    SVRAMTexEnvLoadData*    pTexEnvLoadData;
    s32                     TexturePixelStorageFormat;
    s32                     LogW, LogH;

    //---   Get some information up front
    TexturePixelStorageFormat = VRAM_PS2_GetFormat( rBMP );
    W = rBMP.GetWidth(MipLevel);
    H = rBMP.GetHeight(MipLevel);
    nTextureBufferWidth = W / 64;
    if ( nTextureBufferWidth == 0 ) nTextureBufferWidth = 1;
    LogW = VRAM_PS2_GetLog2( W );
    LogH = VRAM_PS2_GetLog2( H );

    //---   set up the mip environment
    pTexEnvLoadData = (SVRAMTexEnvLoadData*)pDList;
    *pTexEnvLoadData = s_BasicTexEnvLoadData;
    pDList += sizeof(SVRAMTexEnvLoadData);
    pTexEnvLoadData->AD[4] = SCE_GS_SET_TEX0_1( TextureBasePtr,             // TBP0 - address in VRAM
                                                nTextureBufferWidth,        // TBW - units of texels / 64
                                                TexturePixelStorageFormat,  // PSM - 
                                                LogW,                       // TW - texture width log2 component
                                                LogH,                       // TH - texture height log2 component
                                                0x1,                        // TCC - using RGBA
                                                0x00,                       // TFX - set for modulate
                                                CLUTBasePtr,                // CBP - address in VRAM
                                                0x00,                       // CPSM - using PSMCT32 for CLUT
                                                0,                          // CSM - CLUT storage mode CSM1
                                                0,                          // CSA - Entry offset of 0
                                                1 );                        // CLD - CLUT load control
    pTexEnvLoadData->AD[6] = SCE_GS_SET_CLAMP_1( GetPS2WrapMode( s_UWrapMode ),
                                                 GetPS2WrapMode( s_VWrapMode ),
                                                 ConvertUV( s_UWrapMode, W, s_UMin ),
                                                 ConvertUV( s_UWrapMode, W, s_UMax ),
                                                 ConvertUV( s_VWrapMode, H, s_VMin ),
                                                 ConvertUV( s_VWrapMode, H, s_VMax ) );
}
*/
//==========================================================================
/*
static
void ActivateMipEnvironment( x_bitmap& rBMP, s32 MinMip, s32 MaxMip, s16 tbp[], s16 cbp )
{
    sceDmaTag*  pDMA;
    u32*        pVIF;
    sceGifTag*  pGIF;
    u64*        pAD;
    s32         K;
    s32         i;
    s32         tbps[6];
    s32         tbws[6];
    s32         MipID;

    //---   load in the first mip and set up the basic environment
    ActivateEnvironment( rBMP, tbp[MinMip], cbp, MinMip );

    //---   now, bail out early if there's only one mip
    if ( (MaxMip - MinMip) == 0 )
        return;

    //---   set up the rest of the mipping environment

    //---   set up the DMA for the mipping environment
    pDMA = (sceDmaTag*)pDList;
    DMAHELP_BuildTagCont( pDMA, 5*16 );
    pDList += sizeof(sceDmaTag);

    //---   set up the VIF direct command
    pVIF = (u32*)pDList;
    pVIF[0] = 0;
    pVIF[1] = 0;
    pVIF[2] = SCE_VIF1_SET_FLUSH(0);
    pVIF[3] = SCE_VIF1_SET_DIRECT( 4, 0 );
    pDList += sizeof(u32) * 4;

    //---   set up the GIF command
    pGIF = (sceGifTag*)pDList;
    VIFHELP_BuildGifTag1( pGIF, VIFHELP_GIFMODE_PACKED, 1, 3, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( pGIF, VIFHELP_GIFREG_AD, 0, 0, 0 );
    pDList += sizeof(sceGifTag);

    //---   set up the mipping environment
    pAD = (u64*)pDList;
    K = (s32)(s_MipK * 16.0f);
    for ( i = 0; i < 6; i++ )
    {
        MipID = MinMip + 1 + i;
        if ( MipID <= MaxMip )
        {
            tbps[i] = tbp[MipID];
            tbws[i] = MAX( 1, (rBMP.GetWidth(MipID)>>6) );
        }
        else
        {
            tbps[i] = tbp[MinMip];
            tbws[i] = MAX( 1, (rBMP.GetWidth(MinMip)>>6) );
        }
    }
    pAD[0] = SCE_GS_SET_TEX1_1( 0, MaxMip-MinMip, 0x1, 0x4, 0, 0, K );    // bilinear?
//    pAD[0] = SCE_GS_SET_TEX1_1( 0, MaxMip-MinMip, 0x1, 0x5, 0, 0, K );    // trilinear?
    pAD[1] = SCE_GS_TEX1_1;
    pAD[2] = SCE_GS_SET_MIPTBP1_1( tbps[0], tbws[0], tbps[1], tbws[1], tbps[2], tbws[2] );
    pAD[3] = SCE_GS_MIPTBP1_1;
    pAD[4] = SCE_GS_SET_MIPTBP2_1( tbps[3], tbws[3], tbps[4], tbws[4], tbps[5], tbws[5] );
    pAD[5] = SCE_GS_MIPTBP2_1;
    pDList += sizeof(u64) * 6;
}
*/
//==========================================================================
/*
static
s32 FindRoundRobinClutSlot( s32& rBlockAddr )
{
    s32 Entry = s_BankClutRoundRobinEntry;

    //---   mark the round-robin clut as no longer valid, since we know it will be replaced
    if ( s_VRamClutTextureID[Entry] >= 0 )
    {
        if ( s_VRamTextures[s_VRamClutTextureID[Entry]].ClutSlot == Entry )
            s_VRamTextures[s_VRamClutTextureID[Entry]].ClutSlot = -1;
    }

    //---   figure out the block address that matches this entry
    rBlockAddr = VRAM_CLUT_ADDR + Entry * 4;
    
    //---   get prepared for the next round
    s_BankClutRoundRobinEntry++;
    if ( s_BankClutRoundRobinEntry >= VRAM_NCLUTS )
        s_BankClutRoundRobinEntry = 0;

    return Entry;
}
*/
//==========================================================================
/*
static
s32 LoadClut( s32 ID, x_bitmap& rBMP )
{
    s32                 LRU;
    s32                 BlockAddr;
    s32                 ClutSize;
    SVRAMClutLoadData*  pClutLoad;
    s32                 BPP = rBMP.GetBPP();

    //---   find the LRU clut slot
    LRU = FindRoundRobinClutSlot( BlockAddr );

    //---   fill the slot with the clut
    s_VRamTextures[ID].ClutSlot = LRU;
    s_VRamClutTextureID[LRU] = ID;

    //---   build the upload info to the display list
    if ( BPP == 8 )             ClutSize = 64;
    else                        ClutSize = 4;

    pClutLoad = (SVRAMClutLoadData*)pDList;
    pDList += sizeof(SVRAMClutLoadData);
    *pClutLoad = s_BasicClutLoadData;
    pClutLoad->VIF[3] = SCE_VIF1_SET_DIRECT( 6 + ClutSize, 0 );
    if ( BPP == 8 )
    {
        pClutLoad->AD[0] = SCE_GS_SET_BITBLTBUF( 0, 0, 0, BlockAddr, 1, SCE_GS_PSMCT32 );
        pClutLoad->AD[4] = SCE_GS_SET_TRXREG( 16, 16 );
    }
    else
    {
        pClutLoad->AD[0] = SCE_GS_SET_BITBLTBUF( 0, 0, 0, BlockAddr, 1, SCE_GS_PSMCT32 );
        pClutLoad->AD[4] = SCE_GS_SET_TRXREG( 8, 2 );
    }

    pClutLoad->GIFImage.NLOOP = ClutSize;

    pClutLoad->DMATagImage.qwc = ClutSize;
    pClutLoad->DMATagImage.next = (sceDmaTag*)rBMP.GetClutPtr();

    if ( s_pNBytesUploaded )
        *s_pNBytesUploaded += ClutSize * 16;

    return BlockAddr;
}
*/
//==========================================================================

void VRAM_Activate( x_bitmap *BMP, s32 nBMPCount )
{
    if (!BMP)
    {
        ASSERT( BMP );
        return;
    }
    if (nBMPCount <= 0 || nBMPCount > MAX_VRAM_TEXTURES)
    {
        ASSERT( nBMPCount > 0 );
        ASSERT( nBMPCount <= MAX_VRAM_TEXTURES );
        return;
    }

    s32 i;
    for (i=0; i<nBMPCount; ++i)
    {
        VRAM_Activate( BMP[ i ], 0.0f, BMP[ i ].GetNMips() );
    }
}

//==========================================================================

void VRAM_Activate( x_bitmap& BMP )
{
    s32 NMips = BMP.GetNMips();
    if (NMips > 2)
        NMips = 2;
    VRAM_Activate( BMP, 0.0f, NMips );
}

//==========================================================================

static xbool s_bUseTrilinearFiltering = TRUE;
static xbool s_bUseBilinearFiltering = TRUE;
void VRAM_UseBilinear( xbool bActivate )
{
    s_bUseBilinearFiltering = bActivate;
}

//==========================================================================

void VRAM_UseTrilinear( xbool bActivate )
{
    s_bUseTrilinearFiltering = bActivate;
}

//==========================================================================

#define DO_ENV_BEFORE (FALSE)

void VRAM_Activate( x_bitmap& BMP, s32 MinMip, s32 MaxMip )
{
//    return;

    // This *should* be a simple matter:
    //  Add the proper WRAP settings with a REF to the appropriate prebuilt packet
    //  Add the proper FILTER settings with a REF to the appropriate prebuilt packet
    //  Add a REF to the texture data - this should include all MIPs as well.

    // Problems:
    //  How do we associate the prebuilt data with the x_bitmap?
    //  Parallel arrays of x_bitmap pointers and prebuilt texture data?
    //  Prebuilt data generated when texture is registered?
    //      We should be able to use the x_bitmap VRAMID as a prebuilt packet index
    //  The prebuilt packets should be containers for references to a smaller set of packets
    //      width/height/color-depth info
    //      context setup info
    //      etc.
    //  Each Array Element should be as small as absolutely possible...
    //  We need:
    //      Context?
    //      TEX0_1 and TEX0_2 settings (one for each Context)
    //      Clamp Settings
    //      Basic MIP settings for each context
    //  External from the array
    //      TEX1_1 and TEX1_2 settings (depending on the Context) - this sets the MIP-k value
    //          If we don't have MIPs, we don't send this.

    if ((MaxMip - MinMip) > 2)
    {
        MaxMip -= ((MaxMip - MinMip) - 2);
    }

    ASSERT( ENG_GetRenderMode() );

    s32 nID = BMP.GetVRAMID();

    if (nID < 0 || nID >= MAX_VRAM_TEXTURES)
    {
        ASSERT( (nID >= 0) && (nID < MAX_VRAM_TEXTURES) );
        return;
    }

    if (s_VRamTextures[ nID ].Registered == FALSE)
    {
        ASSERT( s_VRamTextures[ nID ].Registered );
        return;
    }

    sceDmaTag *pHeaderMIPs[ MAX_MIP_LEVELS ];
    sceDmaTag *pHeaderENV;
    sceDmaTag *pHeaderCLUT;

    TextureDataPacket *pTexPacket = s_pTextureDataPackets[ BMP.GetVRAMID() ];

    // grab a chunk of the display list for the texture header
    pHeaderCLUT = (sceDmaTag *)pDList;
    pDList += sizeof( sceDmaTag );
    TextureOrCLUTPacket *pCLUTPacket = &(s_CLUTPackets[ pTexPacket->iCLUTPacket ].Packet);

    DMAHELP_BuildTagRef( pHeaderCLUT, 
                        (u32)pCLUTPacket, 
                        sizeof( TextureOrCLUTPacket ) );

    *(sceDmaTag *)pDList = pTexPacket->CLUTDmaTag;
    pDList += sizeof( sceDmaTag );

    s32 i;

    for (i=MinMip; i<=MaxMip; ++i)
    {
        // setup data for this MIP
        pHeaderMIPs[ i ] = (sceDmaTag *)pDList;
        pDList += sizeof( sceDmaTag );

        // stuff the mip dma tag directly into the dlist
        *(sceDmaTag *)pDList = pTexPacket->MIPDmaTags[ i ];
        pDList += sizeof( sceDmaTag );

        DMAHELP_BuildTagRef( pHeaderMIPs[ i ], 
                             (u32)&(s_MIPs[ s_pMIPPackets[ pTexPacket->iMIPPacket ].MIPs[ i ] ].Packet), 
                             sizeof( TextureOrCLUTPacket ) );
    }

    // setup data for the texture environment for this MIP (texture flush)
    pHeaderENV = (sceDmaTag *)pDList;
    pDList += sizeof( sceDmaTag );

    DMAHELP_BuildTagRef( pHeaderENV, 
                         (u32)&(s_ENVs[ s_pMIPPackets[ pTexPacket->iMIPPacket ].ENVs[ MinMip ] ].Packet), 
                         sizeof( TextureEnvironmentPacket ) );

    if (MaxMip-MinMip || MinMip != 0)
    {
        MIPENVPacket *pMIPENV = (MIPENVPacket *)pDList;
        pDList += sizeof( MIPENVPacket );

        *pMIPENV = pTexPacket->MIPEnv[ MinMip ];
        DMAHELP_BuildTagCont( &pMIPENV->DMATag, sizeof( MIPENVPacket ) - sizeof( sceDmaTag ) );

        // set the MIP-k value for the VIF
        s32 K = (s32)(s_MipK * 16.0f);

        s32 tbws[ 6 ];
        s32 tbps[ 6 ];
        for (i=0; i<6; ++i)
        {
            s32 MIPID = MinMip + i + 1;
            if (MIPID <= MaxMip)
            {
                tbws[ i ] = MAX( 1, (BMP.GetWidth( MIPID ) >> 6 ) );
                tbps[ i ] = kMIPAddresses[ MIPID ];
            }
            else
            {
                tbws[ i ] = MAX( 1, (BMP.GetWidth( MinMip ) >> 6 ) );
                tbps[ i ] = kMIPAddresses[ MinMip ];
            }
        }
        s_bUseTrilinearFiltering = FALSE;

        if (pTexPacket->iContext)
        {
            if (s_bUseTrilinearFiltering)
            {
                if (s_bUseBilinearFiltering)
                    pMIPENV->AD[ 0 ] = SCE_GS_SET_TEX1_2( 0, MaxMip-MinMip, 0x1, 0x5, 0, 0, K );
                else
                    pMIPENV->AD[ 0 ] = SCE_GS_SET_TEX1_2( 0, MaxMip-MinMip, 0x0, 0x5, 0, 0, K );
            }
            else
            {
                if (s_bUseBilinearFiltering)
                    pMIPENV->AD[ 0 ] = SCE_GS_SET_TEX1_2( 0, MaxMip-MinMip, 0x1, 0x4, 0, 0, K );
                else
                    pMIPENV->AD[ 0 ] = SCE_GS_SET_TEX1_2( 0, MaxMip-MinMip, 0x0, 0x4, 0, 0, K );
            }
            pMIPENV->AD[ 1 ] = SCE_GS_TEX1_2;
            pMIPENV->AD[ 2 ] = SCE_GS_SET_MIPTBP1_2( tbps[ 0 ], tbws[ 0 ], tbps[ 1 ], tbws[ 1 ], tbps[ 2 ], tbws[ 2 ] );
            pMIPENV->AD[ 3 ] = SCE_GS_MIPTBP1_2;
            pMIPENV->AD[ 4 ] = SCE_GS_SET_MIPTBP2_2( tbps[ 3 ], tbws[ 3 ], tbps[ 4 ], tbws[ 4 ], tbps[ 5 ], tbws[ 5 ] );
            pMIPENV->AD[ 5 ] = SCE_GS_MIPTBP2_2;
        }
        else
        {
            if (s_bUseTrilinearFiltering)
            {
                if (s_bUseBilinearFiltering)
                    pMIPENV->AD[ 0 ] = SCE_GS_SET_TEX1_1( 0, MaxMip-MinMip, 0x1, 0x5, 0, 0, K );
                else
                    pMIPENV->AD[ 0 ] = SCE_GS_SET_TEX1_1( 0, MaxMip-MinMip, 0x0, 0x5, 0, 0, K );
            }
            else
            {
                if (s_bUseBilinearFiltering)
                    pMIPENV->AD[ 0 ] = SCE_GS_SET_TEX1_1( 0, MaxMip-MinMip, 0x1, 0x4, 0, 0, K );
                else
                    pMIPENV->AD[ 0 ] = SCE_GS_SET_TEX1_1( 0, MaxMip-MinMip, 0x0, 0x4, 0, 0, K );
            }
            pMIPENV->AD[ 1 ] = SCE_GS_TEX1_1;
            pMIPENV->AD[ 2 ] = SCE_GS_SET_MIPTBP1_1( tbps[ 0 ], tbws[ 0 ], tbps[ 1 ], tbws[ 1 ], tbps[ 2 ], tbws[ 2 ] );
            pMIPENV->AD[ 3 ] = SCE_GS_MIPTBP1_1;
            pMIPENV->AD[ 4 ] = SCE_GS_SET_MIPTBP2_1( tbps[ 3 ], tbws[ 3 ], tbps[ 4 ], tbws[ 4 ], tbps[ 5 ], tbws[ 5 ] );
            pMIPENV->AD[ 5 ] = SCE_GS_MIPTBP2_1;
        }
    }
}

//==========================================================================

static
void Deactivate( s32 ID )
{
    s32 i;

    ASSERT( (ID >= 0) && (ID < MAX_VRAM_TEXTURES) );

    if( (ID < 0) || (ID >= MAX_VRAM_TEXTURES) )
        return;

    for ( i = 0; i < 8; i++ )
    {
        if ( s_VRamTextures[ID].VRamSlots[i] >= 0 )
        {
            ASSERT( s_VRamSlotTextureID[s_VRamTextures[ID].VRamSlots[i]] == ID );
            s_VRamSlotTextureID[s_VRamTextures[ID].VRamSlots[i]] = -1;
        }
        s_VRamTextures[ID].VRamSlots[i] = -1;
    }

    if ( s_VRamTextures[ID].ClutSlot >= 0 )
    {
        ASSERT( s_VRamClutTextureID[s_VRamTextures[ID].ClutSlot] == ID );
        s_VRamClutTextureID[s_VRamTextures[ID].ClutSlot] = -1;
    }
    s_VRamTextures[ID].ClutSlot = -1;
}

//==========================================================================

void VRAM_Deactivate( x_bitmap& BMP )
{
    Deactivate( BMP.GetVRAMID() );
}

//==========================================================================

void VRAM_DeactivateAll( void )
{
    //---   clean method
    for ( s32 i = 0; i < MAX_VRAM_TEXTURES; i++ )
    {
        if ( s_VRamTextures[i].Registered )
            Deactivate( i );
    }
}

//==========================================================================

xbool VRAM_IsActive( x_bitmap& BMP )
{
    // #### TODO: Finish this function
    return FALSE;
}

//==========================================================================

void VRAM_SetWrapMode( s32 UMode, s32 VMode,
                       f32 UMin, f32 UMax,
                       f32 VMin, f32 VMax )
{
    ASSERTS( (UMode == WRAP_MODE_CLAMP) ||
             (UMode == WRAP_MODE_TILE) ||
             (UMode == WRAP_MODE_REGION) ||
             (UMode == WRAP_MODE_R_REPEAT), "WRAP_MODE_MIRROR not supported!" );
    ASSERTS( (VMode == WRAP_MODE_CLAMP) ||
             (VMode == WRAP_MODE_TILE) ||
             (VMode == WRAP_MODE_REGION) ||
             (VMode == WRAP_MODE_R_REPEAT), "WRAP_MODE_MIRROR not supported!" );

    s_UWrapMode = UMode;
    s_VWrapMode = VMode;


    ASSERT( (UMin <= UMax) &&
            (UMin >= 0.0f) && (UMin <= 1.0f) &&
            (UMax >= 0.0f) && (UMax <= 1.0f) );
    ASSERT( (VMin <= VMax) &&
            (VMin >= 0.0f) && (VMin <= 1.0f) &&
            (VMax >= 0.0f) && (VMax <= 1.0f) );

    s_UMin = UMin;
    s_UMax = UMax;
    s_VMin = VMin;
    s_VMax = VMax;
}

//==========================================================================

void VRAM_SetStatTrackers( s32* pNTextureUploads, s32* pNBytesUploaded )
{
    s_pNTextureUploads = pNTextureUploads;
    s_pNBytesUploaded  = pNBytesUploaded;
}

//==========================================================================

s32 VRAM_PS2_GetTexAddr( x_bitmap& BMP, s32 MipID )
{
    s32 ID;

    ID = BMP.GetVRAMID();
    ASSERT( (ID >= 0) && (ID < MAX_VRAM_TEXTURES) );
    ASSERT( MipID <= BMP.GetNMips() );

    if ( s_VRamTextures[ID].VRamSlots[MipID] < 0 )
        return -1;
    else
        return s_VRamTextures[ID].TexAddr[MipID];
}

//==========================================================================

s32 VRAM_PS2_GetClutAddr( x_bitmap& BMP )
{
    s32 ID;

    ID = BMP.GetVRAMID();
    ASSERT( (ID >= 0) && (ID < MAX_VRAM_TEXTURES) );
    
    if ( s_VRamTextures[ID].ClutSlot < 0 )
        return -1;
    else
        return s_VRamTextures[ID].ClutAddr;
}


//==========================================================================
u32 VRAM_PS2_GetPermanentTexturePageAddress( void )
{
    return VRAM_PERMANENT_TEXTURE_PAGE_ADDR;
}


//==========================================================================
s32 VRAM_PS2_GetContext( x_bitmap &BMP )
{
    s32 iID = BMP.GetVRAMID();

    if (iID >= 0 && iID < MAX_VRAM_TEXTURES)
        return s_pTextureDataPackets[ iID ]->iContext;

    return 0;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// VRAM DEBUG FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void VRAM_DBG_Register( x_bitmap& BMP, char* File, s32 Line, s32 iContext )
{
    // Perform normal bitmap register operation
    VRAM_Register( BMP, iContext );

#if defined( X_DEBUG )

    if( BMP.GetVRAMID() < 0 )
        return;

    // Get last 16 chars of filename
    s32 Len = x_strlen( File ) + 1;
    if( Len > 16 )
        File += (Len - 16);

    // Store filename and line number where this bitmap is registered
    x_strncpy( s_VRamTextures[ BMP.GetVRAMID() ].Filename, File, 16 );
    s_VRamTextures[ BMP.GetVRAMID() ].Filename[15] = '\0';
    s_VRamTextures[ BMP.GetVRAMID() ].LineNumber   = Line;

#endif //X_DEBUG
}

//==========================================================================

void VRAM_DBG_Dump( const char* Filename )
{
#if defined( X_DEBUG )

    s32 NSlotsFree;
    s32 NSlotsUsed;
    s32 i;

    X_FILE* fp = NULL;

    SVRAMTexture* pTex;


    #define START_PRINT     x_fprintf(fp,
    #define END_PRINT       );

    fp = x_fopen( Filename, "wt" );
    if( fp == NULL )
    {
        x_printf( "VRAM ERROR: Couldn't open file for debug dump\n" );
        return;
    }

    NSlotsFree = VRAM_DBG_FreeSlots();
    NSlotsUsed = VRAM_DBG_UsedSlots();

    START_PRINT  "-----------------------------------------\n"  END_PRINT
    START_PRINT  "  ID  |  In Use  | LineNum | File\n"  END_PRINT
    START_PRINT  "-----------------------------------------\n"  END_PRINT

    for( i = 0; i < MAX_VRAM_TEXTURES; i++ )
    {
        pTex = &s_VRamTextures[i];

        if( pTex->Registered )
            START_PRINT  " %3d  |   USED   |   %4d   | %s\n", i, pTex->LineNumber, pTex->Filename  END_PRINT
        else
            START_PRINT  " %3d  |   FREE   |   %4d   | %s\n", i, pTex->LineNumber, pTex->Filename  END_PRINT
    }

    START_PRINT  "\n\n" END_PRINT
    START_PRINT  "-----------------------------------------\n"  END_PRINT
    START_PRINT  "VRAM Slots Free: %d\n", NSlotsFree  END_PRINT
    START_PRINT  "VRAM Slots Used: %d\n", NSlotsUsed  END_PRINT
    START_PRINT  "-----------------------------------------\n"  END_PRINT

    x_fclose( fp );

#endif //X_DEBUG
}

//==========================================================================

s32 VRAM_DBG_FreeSlots( void )
{
    s32 NSlotsFree = 0;
    s32 i;

    for( i = 0; i < MAX_VRAM_TEXTURES; ++i )
    {
        if( !s_VRamTextures[i].Registered )
            NSlotsFree++;
    }
    return NSlotsFree;
}

//==========================================================================

s32 VRAM_DBG_UsedSlots( void )
{
    s32 NSlotsUsed = 0;
    s32 i;

    for( i = 0; i < MAX_VRAM_TEXTURES; ++i )
    {
        if( s_VRamTextures[i].Registered )
            NSlotsUsed++;
    }

    return NSlotsUsed;
}

//==========================================================================

void VRAM_DBG_SanityCheck( void )
{
#if defined( X_DEBUG )
    s32 i, j;

    for ( i = 0; i < VRAM_NSLOTS; i++ )
    {
        ASSERT( s_VRamSlotTextureID[i] < MAX_VRAM_TEXTURES );
        if ( s_VRamSlotTextureID[i] >= 0 )
        {
            for ( j = 0; j < MAX_MIP_LEVELS; j++ )
            {
                if ( s_VRamTextures[s_VRamSlotTextureID[i]].VRamSlots[j] == i )
                    break;
            }
            ASSERT( j < MAX_MIP_LEVELS );
        }
    }

    for ( i = 0; i < VRAM_NCLUTS; i++ )
    {
        ASSERT( s_VRamClutTextureID[i] < MAX_VRAM_TEXTURES );
        if ( s_VRamClutTextureID[i] >= 0 )
        {
            ASSERT( s_VRamTextures[s_VRamClutTextureID[i]].ClutSlot == i );
        }
    }

    for ( i = 0; i < MAX_VRAM_TEXTURES; i++ )
    {
        for ( j = 0; j < MAX_MIP_LEVELS; j++ )
        {
            ASSERT( s_VRamTextures[i].VRamSlots[j] < VRAM_NSLOTS );
            if ( s_VRamTextures[i].VRamSlots[j] >= 0 )
            {
                ASSERT( s_VRamSlotTextureID[s_VRamTextures[i].VRamSlots[j]] == i );
            }
        }

        ASSERT( s_VRamTextures[i].ClutSlot < VRAM_NCLUTS );
        if ( s_VRamTextures[i].ClutSlot >= 0 )
            ASSERT( s_VRamClutTextureID[s_VRamTextures[i].ClutSlot] == i );
    }
#endif //X_DEBUG
}

//==========================================================================
//==========================================================================

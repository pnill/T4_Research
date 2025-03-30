////////////////////////////////////////////////////////////////////////////
// PS2_Geom.cpp - geom routines specific to the PS2
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"
#include "x_memory.hpp"
#include "x_time.hpp"

#include "Q_Engine.hpp"
#include "Q_View.hpp"
#include "Q_VRAM.hpp"
#include "Q_PS2.hpp"
#include "Q_Geom.hpp"
#include "Q_Visibility.hpp"

#include "libgraph.h"
#include "dmahelp.hpp"
#include "vifhelp.hpp"


//static s32 s_ClipStatisticsCount;
static s32 s_ClipStatisticsCullCount;
static s32 s_ClipStatisticsClipCount;
static s32 s_ClipStatisticsTotal;


#define VIEW_TEST_TRIVIAL_REJECT    (0)
#define VIEW_TEST_TRIVIAL_ACCEPT    (1)
#define VIEW_TEST_ON_FRUSTUM        (2)

////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////

typedef struct SBootPacket
{
    sceDmaTag       DMA;        // DMA info to send packet to VIF1
    u32             VIF[4];     // boot microcode...already loaded
} t_BootPacket;

typedef struct SMatrixPacket
{
    sceDmaTag       DMA;        // DMA info to send packet to VIF1
    u32             VIF[4];     // VIF1 tag for matrix info
//    u32             CMD[4];     // Matrix load command
    matrix4         C2S;        // Clip to screen
    matrix4         L2S;        // Local to screen
    matrix4         L2C;        // Local to clip
    u32             VIFKick;    // Kick for matrices
    u32             VIFPad[3];
} t_MatrixPacket;

typedef struct SScenePacket
{
    t_BootPacket     BP;
    t_MatrixPacket   MP;
} t_ScenePacket;

typedef struct SLightPacket
{
    sceDmaTag       DMA;        // DMA info to send packet to VIF1
    u32             VIF[4];     // VIF1 tag for matrix info
    vector4         LightDir1;  // light 1 direction
    vector4         LightDir2;  // light 2 direction
    vector4         LightDir3;  // light 3 direction
    matrix4         LightCol;   // 3 colors and an ambient
} t_LightPacket;

typedef struct SNL2VPacket
{
    sceDmaTag       DMA;        // DMA info to send packet to VIF1
    u32             VIF[4];     // VIF1 tag for matrix info
    vector4         Col0;       // NL2V col 0
    vector4         Col1;       // NL2V col 1
    vector4         Col2;       // NL2V col 2
} t_NL2VPacket;

typedef struct t_DoubleHeadPacket
{
    u32             VIF[8];     // VIF1 tag for MASK (+ data), STCYCL, and FLUSH (or MSCALF for footer)
} t_DoubleHeadPacket;

typedef struct t_HeadFootPacket
{
    u32             VIF[4];     // VIF1 tag for MASK (+ data), STCYCL, and FLUSH (or MSCALF for footer)
} t_HeadFootPacket;

typedef struct t_PreRenderData
{
    sceDmaTag       DMA;        // DMA info to send packet to VIF1
    matrix4         C2S;        // Clip to screen
    matrix4         L2S;        // Local to screen
    matrix4         L2C;        // Local to clip
} t_PreRenderData;

typedef struct t_StripFlagDataPacket
{
    sceDmaTag       DMA;        // DMA info to send packet to VIF1
    u32             VIF[4];     // UNPACK command
    f32             TweakU;     // tweak value                                      X
    f32             TweakV;     // tweak value                                      Y
    u32             Pad[1];     // 1 extra u32s since I'm doing things backwards    Z
    u32             Flags;      // flags value                                      W
} t_StripFlagDataPacket;

static f32 s_fTweakU = 1.0f / 2048.0f;
static f32 s_fTweakV = 1.0f / 2048.0f;

typedef struct t_StripFlagMatrixPacket
{
    sceDmaTag       DMA;        // DMA info to send packet to VIF1
    u32             VIF;        // UNPACK command
    vector4         NL2W[3];    // the matrix, but only 3 rows of it.
    u32             Pad[2];     // shooting for a multiple of 4 bytes
} t_StripFlagMatrixPacket;

typedef struct SEnvContextPacket
{
    sceDmaTag       DMA;            // DMA info to send packet to VIF1
    u32             VIF[4];         // VIF tag for unpacking commands
    sceGifTag       GIF;            // GIF tag for sending AD registers
    u64             ZBuff;          // Z buffer settings
    u64             ZBuffAddr;      // Z buffer register
    u64             Test;           // TEST settings
    u64             TestAddr;       // TEST register
    u64             Alpha;          // ALPHA settings
    u64             AlphaAddr;      // ALPHA register
    u64             FBA;            // FBA settings
    u64             FBAAddr;        // FBA register
    u64             Frame;          // FRAME settings
    u64             FrameAddr;      // FRAME register
    u64             Scissor;        // SCISSOR settings
    u64             ScissorAddr;    // SCISSOR register
    u64             XYOffset;       // XYOFFSET settings
    u64             XYOffsetAddr;   // XYOFFSET register
    u64             TexFlush;       // TEXFLUSH settings
    u64             TexFlushAddr;   // TEXFLUSH register
    u64             Tex1;           // TEX1 settings
    u64             Tex1Addr;       // TEX1 register
    u64             Tex0;           // TEX0 settings
    u64             Tex0Addr;       // TEX0 register
    u64             Clamp;          // CLAMP settings
    u64             ClampAddr;      // CLAMP register
} t_EnvContextPacket;

typedef struct SContextBlendPacket
{
    u32             VIF[ 4 ];
    sceGifTag       GIFTag;
    u64             AD[ 4 ];
} t_ContextBlendPacket;

////////////////////////////////////////////////////////////////////////////
// Externs
////////////////////////////////////////////////////////////////////////////

extern u32 VUM_Geom_Code __attribute__((section(".vudata")));
extern u32 VUM_Geom_Code_End __attribute__((section(".vudata")));

////////////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////////////

static s32 s_PreviousMaterialLoop = -1;

s32 QGeom::s_MicroCodeHandle = -1;

static t_HeadFootPacket s_FlagHead;
static t_HeadFootPacket s_FlagTail;

static t_DoubleHeadPacket s_PreRenderHead;
static t_HeadFootPacket s_PreRenderTail;

static t_ContextBlendPacket s_ContextBlend[ 4 ];


void CalcMIPPlanes( t_TexData &Data )
{
    matrix4 W2V;
    vector3 Eye;
    Data.pView->GetW2VMatrix( W2V );
    Data.pView->GetPosition( Eye );
    Data.MipPlaneN.X = W2V.M[0][2];
    Data.MipPlaneN.Y = W2V.M[1][2];
    Data.MipPlaneN.Z = W2V.M[2][2];

    Data.MipPlaneD = (Data.MipPlaneN.X * Eye.X) +
                     (Data.MipPlaneN.Y * Eye.Y) +
                     (Data.MipPlaneN.Z * Eye.Z);
}

/*
void UseMIPs( t_TexData &Data )
{
    matrix4 W2V;
    vector3 Eye;
    Data.pView->GetW2VMatrix( W2V );
    Data.pView->GetPosition( Eye );
    Data.MipPlaneN.X = W2V.M[0][2];
    Data.MipPlaneN.Y = W2V.M[1][2];
    Data.MipPlaneN.Z = W2V.M[2][2];

    Data.MipPlaneD   = (Data.MipPlaneN.X * Eye.X) +
                    (Data.MipPlaneN.Y * Eye.Y) +
                    (Data.MipPlaneN.Z * Eye.Z);
}
*/

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////


static
void SetMultipassRenderSettings( s32 loop, u32 &RenderFlags, u32 &BlendMode )
{
    if (loop == 0)
    {
        // first pass
        // A = 0, B = 1, C = 3, D = 1, Fix = 0x7F
        RenderFlags &= ~ENG_ALPHA_BLEND_OFF;
        RenderFlags |=  ENG_ALPHA_BLEND_ON;
        BlendMode =     ENG_BLEND_MULTI_1;
    }
    else
    {
        // second pass
        // A = 1, B = 0, C = 0, D = 0, Fix = 0
        RenderFlags &= ~ENG_ALPHA_TEST_OFF;
        RenderFlags |=  ENG_ALPHA_TEST_ON;
        RenderFlags &= ~ENG_ALPHA_BLEND_OFF;
        RenderFlags |=  ENG_ALPHA_BLEND_ON;
        BlendMode =     ENG_BLEND_MULTI_2;
    }
}


static
void SetPunchRenderSettings( u32 &RenderFlags )
{
    // setup for punchthru
    // let's see...   Alpha Test On, Alpha Blend Off...
    RenderFlags &= ~ENG_ALPHA_BLEND_ON;
    RenderFlags |=  ENG_ALPHA_BLEND_OFF;
    RenderFlags &= ~ENG_ALPHA_TEST_OFF;
    RenderFlags |=  ENG_ALPHA_TEST_ON;
}

static
void SetTransparencyRenderSettings( u32 &RenderFlags, u32 &BlendMode )
{
    // setup for transparency
    // Alpha Test On, Alpha Blend On...
    RenderFlags &= ~ENG_ZBUFFER_FILL_ON;
    RenderFlags |=  ENG_ZBUFFER_FILL_OFF;
    RenderFlags &= ~ENG_ALPHA_TEST_ON;
    RenderFlags |=  ENG_ALPHA_TEST_OFF;
    RenderFlags &= ~ENG_ALPHA_BLEND_OFF;
    RenderFlags |=  ENG_ALPHA_BLEND_ON;
    BlendMode =     ENG_BLEND_NORMAL;
}

static 
void SetEnvmapRenderSettings( u32 &RenderFlags, u32 &BlendMode, u8&FixedAlpha )
{
    RenderFlags &= ~ENG_ALPHA_TEST_OFF;
    RenderFlags |=  ENG_ALPHA_TEST_ON;
    RenderFlags &= ~ENG_ALPHA_BLEND_OFF;
    RenderFlags |=  ENG_ALPHA_BLEND_ON;
    BlendMode =    (ENG_BLEND_ADDITIVE | ENG_BLEND_FIXED_ALPHA);
    FixedAlpha =    58;
}



QGeom::QGeom( void )
{
    InitData();
}

//==========================================================================

QGeom::QGeom( char* filename )
{
    InitData();

    SetupFromResource( filename );
}

//==========================================================================

QGeom::~QGeom( void )
{
    KillData();
}

//==========================================================================

void QGeom::InitData( void )
{
    m_Name[0]           = '\0';
    m_Flags             = 0;
    m_pTextureNames     = NULL;
    m_pTextures         = NULL;
    m_pTexturePtrs      = NULL;
    m_pMeshes           = NULL;
    m_pSubMeshes        = NULL;
    m_NTextures         = 0;
    m_NTexturesOwned    = 0;
    m_NMeshes           = 0;
    m_NSubMeshes        = 0;
    m_MeshLocked        = -1;
    m_L2W.Identity();

    m_pRawData          = NULL;

    m_Packets       = NULL;
    m_PacketSize    = 0;
    m_pRawPtrData   = NULL;
    m_MeshTextureArray  = NULL;

    PS2HELP_BuildVIFTagMask( &s_FlagHead.VIF[0], 0, 0, 0, 3,
                                                 0, 0, 0, 3,
                                                 0, 0, 0, 3,
                                                 0, 0, 0, 3 );
    s_FlagHead.VIF[ 2 ] = VIFHELP_SET_STCYCL( 1, 1, 0 );
    s_FlagHead.VIF[ 3 ] = 0;//SCE_VIF1_SET_FLUSH( 0 );

    u32 tempAddr = 0x0080 >> 3;//0x01C0 >> 3;
    s_FlagTail.VIF[ 0 ] = 0;
    s_FlagTail.VIF[ 1 ] = 0;
    s_FlagTail.VIF[ 2 ] = 0;
    s_FlagTail.VIF[ 3 ] = SCE_VIF1_SET_MSCALF( tempAddr, 0 );

    s_PreRenderHead.VIF[ 0 ] = SCE_VIF1_SET_FLUSH(0);
    s_PreRenderHead.VIF[ 1 ] = SCE_VIF1_SET_BASE( 0, 0 );
    s_PreRenderHead.VIF[ 2 ] = SCE_VIF1_SET_OFFSET( 360, 0 );
    s_PreRenderHead.VIF[ 3 ] = 0;

    s_PreRenderHead.VIF[ 4 ] = VIFHELP_SET_STCYCL( 1, 1, 0 );
    s_PreRenderHead.VIF[ 5 ] = 0;
    s_PreRenderHead.VIF[ 6 ] = 0;
    u32 addr = 1001;//MATRIX_START;
    u32 size = (sizeof( matrix4 ) * 3) >> 4;
    VIFHELP_BuildTagUnpack( &s_PreRenderHead.VIF[ 7 ], addr, size, VIFHELP_UNPACK_V4_32, FALSE, FALSE, TRUE );

    s_PreRenderTail.VIF[ 0 ] = SCE_VIF1_SET_MSCALF(0, 0);
    s_PreRenderTail.VIF[ 1 ] = 0;
    s_PreRenderTail.VIF[ 2 ] = 0;
    s_PreRenderTail.VIF[ 3 ] = 0;//SCE_VIF1_SET_FLUSH(0);

    m_NMaterials = 0;
    m_pMaterials = NULL;
    m_NMatStages = 0;
    m_pMatStages = NULL;
    m_pContiguousMaterials = NULL;

    // Pass 1 diffuse
    s_ContextBlend[ 0 ].VIF[ 0 ] = 0;
    s_ContextBlend[ 0 ].VIF[ 1 ] = SCE_VIF1_SET_FLUSH( 0 );
    s_ContextBlend[ 0 ].VIF[ 2 ] = 0;
    s_ContextBlend[ 0 ].VIF[ 3 ] = SCE_VIF1_SET_DIRECT( 3, 0 );
    VIFHELP_BuildGifTag1( &s_ContextBlend[ 0 ].GIFTag, VIFHELP_GIFMODE_PACKED, 1, 2, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &s_ContextBlend[ 0 ].GIFTag, VIFHELP_GIFREG_AD, 0, 0, 0 );
    s_ContextBlend[ 0 ].AD[ 0 ] = SCE_GS_SET_TEST_1( 0, 5, 0x7F, 0, 0, 0, 1, 2 );
    s_ContextBlend[ 0 ].AD[ 1 ] = SCE_GS_TEST_1;
    s_ContextBlend[ 0 ].AD[ 2 ] = SCE_GS_SET_ALPHA_1( 0, 1, 3, 1, 0x7F );
    s_ContextBlend[ 0 ].AD[ 3 ] = SCE_GS_ALPHA_1;

    // pass 2 always diffuse
    s_ContextBlend[ 1 ].VIF[ 0 ] = 0;
    s_ContextBlend[ 1 ].VIF[ 1 ] = SCE_VIF1_SET_FLUSH( 0 );
    s_ContextBlend[ 1 ].VIF[ 2 ] = 0;
    s_ContextBlend[ 1 ].VIF[ 3 ] = SCE_VIF1_SET_DIRECT( 3, 0 );
    VIFHELP_BuildGifTag1( &s_ContextBlend[ 1 ].GIFTag, VIFHELP_GIFMODE_PACKED, 1, 2, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &s_ContextBlend[ 1 ].GIFTag, VIFHELP_GIFREG_AD, 0, 0, 0 );
    s_ContextBlend[ 1 ].AD[ 0 ] = SCE_GS_SET_TEST_1( 1, 2, 0x7F, 0, 0, 0, 1, 2 );
    s_ContextBlend[ 1 ].AD[ 1 ] = SCE_GS_TEST_1;
    s_ContextBlend[ 1 ].AD[ 2 ] = SCE_GS_SET_ALPHA_1( 1, 0, 0, 0, 0 );
    s_ContextBlend[ 1 ].AD[ 3 ] = SCE_GS_ALPHA_1;

    // Pass 1 punchthru
    s_ContextBlend[ 2 ].VIF[ 0 ] = 0;
    s_ContextBlend[ 2 ].VIF[ 1 ] = SCE_VIF1_SET_FLUSH( 0 );
    s_ContextBlend[ 2 ].VIF[ 2 ] = 0;
    s_ContextBlend[ 2 ].VIF[ 3 ] = SCE_VIF1_SET_DIRECT( 3, 0 );
    VIFHELP_BuildGifTag1( &s_ContextBlend[ 2 ].GIFTag, VIFHELP_GIFMODE_PACKED, 1, 2, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &s_ContextBlend[ 2 ].GIFTag, VIFHELP_GIFREG_AD, 0, 0, 0 );
    s_ContextBlend[ 2 ].AD[ 0 ] = SCE_GS_SET_TEST_1( 1, 2, 0x40, 0, 0, 0, 1, 3 );
    s_ContextBlend[ 2 ].AD[ 1 ] = SCE_GS_TEST_1;
    s_ContextBlend[ 2 ].AD[ 2 ] = SCE_GS_SET_ALPHA_1( 0, 1, 3, 1, 0x7F );
    s_ContextBlend[ 2 ].AD[ 3 ] = SCE_GS_ALPHA_1;

    // pass 1 alpha
    s_ContextBlend[ 3 ].VIF[ 0 ] = 0;
    s_ContextBlend[ 3 ].VIF[ 1 ] = SCE_VIF1_SET_FLUSH( 0 );
    s_ContextBlend[ 3 ].VIF[ 2 ] = 0;
    s_ContextBlend[ 3 ].VIF[ 3 ] = SCE_VIF1_SET_DIRECT( 3, 0 );
    VIFHELP_BuildGifTag1( &s_ContextBlend[ 3 ].GIFTag, VIFHELP_GIFMODE_PACKED, 1, 2, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &s_ContextBlend[ 3 ].GIFTag, VIFHELP_GIFREG_AD, 0, 0, 0 );
    s_ContextBlend[ 3 ].AD[ 0 ] = SCE_GS_SET_TEST_1( 1, 2, 0x7F, 0, 0, 0, 0, 1 );
    s_ContextBlend[ 3 ].AD[ 1 ] = SCE_GS_TEST_1;
    s_ContextBlend[ 3 ].AD[ 2 ] = SCE_GS_SET_ALPHA_1( 0, 1, 0, 1, 0 );
    s_ContextBlend[ 3 ].AD[ 3 ] = SCE_GS_ALPHA_1;
}

//==========================================================================

void QGeom::KillData( void )
{
    if( m_pRawData != NULL )
    {
        x_free( m_pRawData );
        m_pRawData = NULL;
    }
    if( m_pRawPtrData != NULL )
    {
        x_free( m_pRawPtrData );
        m_pRawPtrData = NULL;
    }
    if (m_pMaterials != NULL)
    {
        delete [] m_pMaterials;
        m_pMaterials = NULL;
    }
    m_NMaterials = 0;
    if (m_pMatStages != NULL)
    {
        delete [] m_pMatStages;
        m_pMatStages = NULL;
    }
    m_NMatStages = 0;
    if (m_pContiguousMaterials != NULL)
    {
        delete [] m_pContiguousMaterials;
        m_pContiguousMaterials = NULL;
    }

    // delete the mesh texture array
    SAFE_DELETE_ARRAY( m_MeshTextureArray )

    KillOwnedTextures();
}

//==========================================================================

void QGeom::SetupFromResource( char* pFilename )
{
    X_FILE* pFilePtr;

    //---   open the file
    pFilePtr = x_fopen( pFilename, "rb" );
    ASSERT( pFilePtr != NULL );

    SetupFromResource( pFilePtr );

    //---   close the file
    x_fclose( pFilePtr );
}

//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================

static xbool s_bMultipassOverride = FALSE;

void QGeom::PrepareForMultipass( void )
{
    s_bMultipassOverride = TRUE;
}

//==========================================================================

void QGeom::RecoverFromMultipass( void )
{
    s_bMultipassOverride = FALSE;
}

//==========================================================================

void QGeom::SetupFromResource_Multipass( X_FILE* pFilePtr )
{
    ////////////////////////////////////////////////////////////////////////
    // VERY IMPORTANT NOTE: THE SetupFromResource ROUTINE IS OVERLOADED A
    // COUPLE OF TIMES. IF THE DATA FORMAT CHANGES AT ALL, MAKE SURE THE
    // OVERLOADED ROUTINES GET CHANGED AS WELL.
    ////////////////////////////////////////////////////////////////////////

    ASSERT( pFilePtr != NULL );

    s32     i;
    u32     RawDataSize;
    u32     RawMeshSize;
    byte*   pTempRaw;

    t_GeomFileHeader_Multipass hdr;

    //---   Clear out any previous data
    KillData();
    InitData();

    //---   Read in the file header
    x_fread( &hdr, sizeof(hdr), 1, pFilePtr );

    //---   Copy header info to members
    x_strcpy( m_Name, hdr.Name );
    m_Flags         = hdr.Flags;
    m_NTextures     = hdr.NTextures;
    m_NMeshes       = hdr.NMeshes;
    m_NSubMeshes    = hdr.NSubMeshes;
    m_PacketSize    = hdr.PacketSize;

    //---   Calc the size of the raw data to allocate
    RawDataSize = sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    RawDataSize += sizeof(t_GeomMesh) * m_NMeshes;
    RawDataSize += sizeof(t_GeomSubMesh) * m_NSubMeshes;

//    RawDataSize += sizeof( t_GeomMaterial ) * hdr.NMaterials;
//    RawDataSize += sizeof( t_GeomMatStage ) * hdr.NMatStages;

    RawMeshSize = RawDataSize;
    RawDataSize += m_PacketSize + 16;   // add 16 bytes for alignment purposes

    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData != NULL );

    pTempRaw = m_pRawData;

    //---   Assign real data pointers into raw data buffer
    if( hdr.NTextures != 0 )
    {
        m_pTextureNames = (char*)pTempRaw;
        pTempRaw += sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    }

    m_pMeshes = (t_GeomMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomMesh) * m_NMeshes;

    m_pSubMeshes = (t_GeomSubMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomSubMesh) * m_NSubMeshes;

    for (i=0; i<m_NSubMeshes; ++i)
    {
        m_pSubMeshes[ i ].AvgPixelSize = x_fastlog2f( m_pSubMeshes[ i ].AvgPixelSize );
    }

//    for (i=0; i<m_nSubMeshes; ++i)
//    {
//        m_pSubMeshes[ i ].pPacketOffsets = (s32*)pTempRaw;
//        pTempRaw += sizeof( s32 ) * m_pSubMeshes[ i ].nPacketCount;
//        m_pSubMeshes[ i ].pPacketSizes = (s32*)pTempRaw;
//        pTempRaw += sizeof( s32 ) * m_pSubMeshes[ i ].nPacketCount;
//    }

    m_Packets = (byte*)pTempRaw;

    //---   Need to load the Materials for this mesh.
    // allocate space for the materials
    // allocate space for the stages
    // loop the correct number of times

    //---   Align packet data on 16 byte boundary if needed, and then
    //---   read in texture names, material data, mesh data, submesh data and packet data
    if( ((u32)m_Packets & 0x0F) != 0 )
    {
        m_Packets += 0x10 - ((u32)m_Packets & 0x0F);
        x_fread( m_pRawData, sizeof(byte), RawMeshSize, pFilePtr );
        x_fread( m_Packets, sizeof(byte), m_PacketSize, pFilePtr );
    }
    else
    {   //---   If no alignment was needed, read in data plus packets
        x_fread( m_pRawData, sizeof(byte), RawMeshSize + m_PacketSize, pFilePtr );
    }

    //---   Allocate bitmap memory if textures stored in file
    if( hdr.NTexturesInFile )
    {
        m_Flags             |= GEOM_FLAG_OWNS_TEXTURES;
        m_NTexturesOwned    = hdr.NTexturesInFile;
        m_pTextures         = new x_bitmap[ m_NTexturesOwned ];
        ASSERT( m_pTextures != NULL );
    }

    //---   Load the texture data
    for( i = 0; i < hdr.NTexturesInFile; i++ )
    {
        VERIFY( m_pTextures[i].Load( pFilePtr ) );
        VRAM_Register( m_pTextures[ i ] );
//        m_pTextures[ i ].SetVRAMID( -1 );
    }

    //---   Load the Materials
    if (hdr.NMaterials)
    {
        m_NMaterials                    = hdr.NMaterials;
        m_pMaterials                    = new t_GeomMaterial[ m_NMaterials ];
        m_pContiguousMaterials          = new s32[ m_NSubMeshes ];
        x_memset( m_pContiguousMaterials, 0, sizeof( s32 ) * m_NSubMeshes );
        for (i=0; i<m_NMaterials; ++i)
        {
            x_fread( &m_pMaterials[ i ], sizeof( t_GeomMaterial ), 1, pFilePtr );
        }

        s32 iPrevious = -1;
        s32 iCurrent = 0;
        for (i=0; i<m_NSubMeshes; ++i)
        {
            // count the contiguous texture blocks
            if (m_pSubMeshes[ i ].TextureID != iPrevious)
            {
                iCurrent = i;
            }

            m_pContiguousMaterials[ iCurrent ] ++;
            iPrevious = m_pSubMeshes[ i ].TextureID;
        }
    }

    //---   Load the Stages associated with the Materials
    if (hdr.NMatStages)
    {
        m_NMatStages        = hdr.NMatStages;
        m_pMatStages        = new t_GeomMatStage[ m_NMatStages ];
        for (i=0; i<m_NMatStages; ++i)
        {
            x_fread( &m_pMatStages[ i ], sizeof( t_GeomMatStage ), 1, pFilePtr );
        }
    }

    for (i=0; i<m_NMaterials; ++i)
    {
        s32 j;
        for (j=0; j<m_pMaterials[ i ].nStages; ++j)
        {
            ASSERT( j < kMaxNumStages);
            s32 iTexture = -1;
            if (m_pMaterials[ i ].nStages > 0)
                iTexture = m_pMatStages[ m_pMaterials[ i ].iStages[ j ] ].iTexture;
            if (iTexture >= 0)
            {
                if (m_pTextures)
                {
                    if (m_pTextures[ iTexture ].GetVRAMID() == -1)
                    {
                        VRAM_Register( m_pTextures[ iTexture ], j );
                    }
                }
                else if (m_pTexturePtrs)
                {
                    if (m_pTexturePtrs[ iTexture ]->GetVRAMID() == -1)
                    {
                        VRAM_Register( *m_pTexturePtrs[ iTexture ], 0 );
                    }
                }
            }
        }
    }

    // create the mesh texture array (for overriding a meshes default texture)
    m_MeshTextureArray = (x_bitmap**)x_malloc( sizeof(x_bitmap*) * m_NMeshes );
    ASSERT( m_MeshTextureArray != NULL );
    x_memset( m_MeshTextureArray, NULL, sizeof(x_bitmap*) * m_NMeshes );

    //---   Check if any pointer data is in the file
    RawDataSize = hdr.NPosPtrs;
    RawDataSize += hdr.NUVPtrs;
    RawDataSize += hdr.NRGBAPtrs;
    RawDataSize += hdr.NNormalPtrs;

    if( RawDataSize == 0 )
        return;

    m_pRawPtrData = (u32*)x_malloc( sizeof(u32) * RawDataSize );
    ASSERT( m_pRawPtrData != NULL );

    //---   Read in data for positions, UVs, color, and normal ptrs
    x_fread( m_pRawPtrData, sizeof(u32), RawDataSize, pFilePtr );

    u32 uCurPtr;
    u32 uPacketStart = (u32)m_Packets;

    //---   The pointers are offsets at this point, so the packet base address must be
    //      added to get the correct addresses for the pointers (loop unrolled 4 times)
    for( uCurPtr = 0; uCurPtr < RawDataSize - 4; uCurPtr += 4 )
    {
        m_pRawPtrData[uCurPtr]   += uPacketStart;
        m_pRawPtrData[uCurPtr+1] += uPacketStart;
        m_pRawPtrData[uCurPtr+2] += uPacketStart;
        m_pRawPtrData[uCurPtr+3] += uPacketStart;
    }
    //---   Catch the elements at the end of the array
    for( ; uCurPtr < RawDataSize; uCurPtr++ )
        m_pRawPtrData[uCurPtr] += uPacketStart;

    uCurPtr = 0;

    //---   Assign position pointers to meshes
    if( hdr.NPosPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_POSPTRS )
            {
                m_pMeshes[i].pPositions = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign UV pointers to meshes
    if( hdr.NUVPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_UVPTRS )
            {
                m_pMeshes[i].pUVs = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign color pointers to meshes
    if( hdr.NRGBAPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_RGBAPTRS )
            {
                m_pMeshes[i].pRGBAs = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign normal pointers to meshes
    if( hdr.NNormalPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_NORMALPTRS )
            {
                m_pMeshes[i].pNormals = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

//hack
#if defined(TARGET_PS2)
// turn on the MIPs.  Needs to be only on objects that actually use MIPs...
for( i = 0; i < m_NMeshes; ++i )
{
    m_pMeshes[ i ].Flags |= MESH_FLAG_USE_MIPS;
}
#endif

}

//==========================================================================

void QGeom::SetupFromResource( X_FILE* pFilePtr )
{
    if (s_bMultipassOverride)
    {
        SetupFromResource_Multipass( pFilePtr );
        return;
    }
    ////////////////////////////////////////////////////////////////////////
    // VERY IMPORTANT NOTE: THE SetupFromResource ROUTINE IS OVERLOADED A
    // COUPLE OF TIMES. IF THE DATA FORMAT CHANGES AT ALL, MAKE SURE THE
    // OVERLOADED ROUTINES GET CHANGED AS WELL.
    ////////////////////////////////////////////////////////////////////////

    ASSERT( pFilePtr != NULL );

    s32     i;
    u32     RawDataSize;
    u32     RawMeshSize;
    byte*   pTempRaw;

    t_GeomFileHeader hdr;

    //---   Clear out any previous data
    KillData();
    InitData();

    //---   Read in the file header
    x_fread( &hdr, sizeof(t_GeomFileHeader), 1, pFilePtr );

    //---   Copy header info to members
    x_strcpy( m_Name, hdr.Name );
    m_Flags         = hdr.Flags;
    m_NTextures     = hdr.NTextures;
    m_NMeshes       = hdr.NMeshes;
    m_NSubMeshes    = hdr.NSubMeshes;
    m_PacketSize    = hdr.PacketSize;

    //---   Calc the size of the raw data to allocate
    RawDataSize = sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    RawDataSize += sizeof(t_GeomMesh) * m_NMeshes;
    RawDataSize += sizeof(t_GeomSubMesh) * m_NSubMeshes;

//    RawDataSize += sizeof( t_GeomMaterial ) * hdr.NMaterials;
//    RawDataSize += sizeof( t_GeomMatStage ) * hdr.NMatStages;

    RawMeshSize = RawDataSize;
    RawDataSize += m_PacketSize + 16;   // add 16 bytes for alignment purposes

    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData != NULL );

    pTempRaw = m_pRawData;

    //---   Assign real data pointers into raw data buffer
    if( hdr.NTextures != 0 )
    {
        m_pTextureNames = (char*)pTempRaw;
        pTempRaw += sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    }

    m_pMeshes = (t_GeomMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomMesh) * m_NMeshes;

    m_pSubMeshes = (t_GeomSubMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomSubMesh) * m_NSubMeshes;

    m_Packets = (byte*)pTempRaw;

    //---   Need to load the Materials for this mesh.
    // allocate space for the materials
    // allocate space for the stages
    // loop the correct number of times

    //---   Align packet data on 16 byte boundary if needed, and then
    //---   read in texture names, material data, mesh data, submesh data and packet data
    if( ((u32)m_Packets & 0x0F) != 0 )
    {
        m_Packets += 0x10 - ((u32)m_Packets & 0x0F);
        x_fread( m_pRawData, sizeof(byte), RawMeshSize, pFilePtr );
        x_fread( m_Packets, sizeof(byte), m_PacketSize, pFilePtr );
    }
    else
    {   //---   If no alignment was needed, read in data plus packets
        x_fread( m_pRawData, sizeof(byte), RawMeshSize + m_PacketSize, pFilePtr );
    }

    //---   Allocate bitmap memory if textures stored in file
    if( hdr.NTexturesInFile )
    {
        m_Flags             |= GEOM_FLAG_OWNS_TEXTURES;
        m_NTexturesOwned    = hdr.NTexturesInFile;
        m_pTextures         = new x_bitmap[ m_NTexturesOwned ];
        ASSERT( m_pTextures != NULL );
    }

    //---   Load the texture data
    for( i = 0; i < hdr.NTexturesInFile; i++ )
    {
        VERIFY( m_pTextures[i].Load( pFilePtr ) );
        VRAM_Register( m_pTextures[ i ] );
//        m_pTextures[ i ].SetVRAMID( -1 );
    }

#if 0
    //---   Load the Materials
    if (hdr.NMaterials)
    {
        m_NMaterials        = hdr.NMaterials;
        m_pMaterials        = new t_GeomMaterial[ m_NMaterials ];
        for (i=0; i<m_NMaterials; ++i)
        {
            x_fread( m_pMaterials, sizeof( t_GeomMaterial ), 1, pFilePtr );
        }
    }

    //---   Load the Stages associated with the Materials
    if (hdr.NMatStages)
    {
        m_NMatStages        = hdr.NMatStages;
        m_pMatStages        = new t_GeomMatStage[ m_NMatStages ];
        for (i=0; i<m_NMatStages; ++i)
        {
            x_fread( m_pMatStages, sizeof( t_GeomMatStage ), 1, pFilePtr );
        }
    }

    for (i=0; i<m_NMaterials; ++i)
    {
        s32 j;
        for (j=0; j<m_pMaterials[ i ].nStages; ++j)
        {
            s32 iTexture = m_pMatStages[ m_pMaterials[ i ].iStages[ j ] ].iTexture;
            if (m_pTextures[ iTexture ].GetVRAMID() == -1)
            {
                VRAM_Register( m_pTextures[ iTexture ], j );
            }
        }
    }
#endif

    // create the mesh texture array (for overriding a meshes default texture)
    m_MeshTextureArray = (x_bitmap**)x_malloc( sizeof(x_bitmap*) * m_NMeshes );
    ASSERT( m_MeshTextureArray != NULL );
    x_memset( m_MeshTextureArray, NULL, sizeof(x_bitmap*) * m_NMeshes );

    //---   Check if any pointer data is in the file
    RawDataSize = hdr.NPosPtrs;
    RawDataSize += hdr.NUVPtrs;
    RawDataSize += hdr.NRGBAPtrs;
    RawDataSize += hdr.NNormalPtrs;

    if( RawDataSize == 0 )
        return;

    m_pRawPtrData = (u32*)x_malloc( sizeof(u32) * RawDataSize );
    ASSERT( m_pRawPtrData != NULL );

    //---   Read in data for positions, UVs, color, and normal ptrs
    x_fread( m_pRawPtrData, sizeof(u32), RawDataSize, pFilePtr );

    u32 uCurPtr;
    u32 uPacketStart = (u32)m_Packets;

    //---   The pointers are offsets at this point, so the packet base address must be
    //      added to get the correct addresses for the pointers (loop unrolled 4 times)
    for( uCurPtr = 0; uCurPtr < RawDataSize - 4; uCurPtr += 4 )
    {
        m_pRawPtrData[uCurPtr]   += uPacketStart;
        m_pRawPtrData[uCurPtr+1] += uPacketStart;
        m_pRawPtrData[uCurPtr+2] += uPacketStart;
        m_pRawPtrData[uCurPtr+3] += uPacketStart;
    }
    //---   Catch the elements at the end of the array
    for( ; uCurPtr < RawDataSize; uCurPtr++ )
        m_pRawPtrData[uCurPtr] += uPacketStart;

    uCurPtr = 0;

    //---   Assign position pointers to meshes
    if( hdr.NPosPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_POSPTRS )
            {
                m_pMeshes[i].pPositions = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign UV pointers to meshes
    if( hdr.NUVPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_UVPTRS )
            {
                m_pMeshes[i].pUVs = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign color pointers to meshes
    if( hdr.NRGBAPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_RGBAPTRS )
            {
                m_pMeshes[i].pRGBAs = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign normal pointers to meshes
    if( hdr.NNormalPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_NORMALPTRS )
            {
                m_pMeshes[i].pNormals = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

//hack
#if defined(TARGET_PS2)
// turn on the MIPs.  Needs to be only on objects that actually use MIPs...
for( i = 0; i < m_NMeshes; ++i )
{
    m_pMeshes[ i ].Flags |= MESH_FLAG_USE_MIPS;
}
#endif

}

//==========================================================================

void QGeom::SetupFromResource( byte* pByteStream )
{
    ////////////////////////////////////////////////////////////////////////
    // VERY IMPORTANT NOTE: THE SetupFromResource ROUTINE IS OVERLOADED A
    // COUPLE OF TIMES. IF THE DATA FORMAT CHANGES AT ALL, MAKE SURE THE
    // OVERLOADED ROUTINES GET CHANGED AS WELL.
    ////////////////////////////////////////////////////////////////////////

    ASSERT( pByteStream != NULL );

    s32     i;
    u32     RawDataSize;
    u32     RawMeshSize;
    byte*   pTempRaw;

    t_GeomFileHeader hdr;

    //---   Clear out any previous data
    KillData();
    InitData();

    //---   Read in the file header
    x_memcpy( &hdr, pByteStream, sizeof(t_GeomFileHeader) );
    pByteStream += sizeof(t_GeomFileHeader);

    //---   Copy header info to members
    x_strcpy( m_Name, hdr.Name );
    m_Flags         = hdr.Flags;
    m_NTextures     = hdr.NTextures;
    m_NMeshes       = hdr.NMeshes;
    m_NSubMeshes    = hdr.NSubMeshes;
    m_PacketSize    = hdr.PacketSize;

    //---   Calc the size of the raw data to allocate
    RawDataSize = sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    RawDataSize += sizeof(t_GeomMesh) * m_NMeshes;
    RawDataSize += sizeof(t_GeomSubMesh) * m_NSubMeshes;
    RawMeshSize = RawDataSize;
    RawDataSize += m_PacketSize + 16;   // add 16 bytes for alignment purposes

    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData != NULL );

    pTempRaw = m_pRawData;

    //---   Assign real data pointers into raw data buffer
    if( hdr.NTextures != 0 )
    {
        m_pTextureNames = (char*)pTempRaw;
        pTempRaw += sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    }

    m_pMeshes = (t_GeomMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomMesh) * m_NMeshes;

    m_pSubMeshes = (t_GeomSubMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomSubMesh) * m_NSubMeshes;

    m_Packets = (byte*)pTempRaw;

    //---   Allign packet data on 16 byte boundary if needed, and then
    //---   read in texture names, mesh data, submesh data and packet data
    if( ((u32)m_Packets & 0x0F) != 0 )
    {
        m_Packets += 0x10 - ((u32)m_Packets & 0x0F);
        x_memcpy( m_pRawData, pByteStream, RawMeshSize );
        pByteStream += RawMeshSize;
        x_memcpy( m_Packets, pByteStream, m_PacketSize );
        pByteStream += m_PacketSize;
    }
    else
    {   //---   If no alignment was needed, read in data plus packets
        x_memcpy( m_pRawData, pByteStream, RawMeshSize + m_PacketSize );
        pByteStream += RawMeshSize + m_PacketSize;
    }

    //---   Allocate bitmap memory if textures stored in file
    if( hdr.NTexturesInFile )
    {
        m_Flags             |= GEOM_FLAG_OWNS_TEXTURES;
        m_NTexturesOwned    = hdr.NTexturesInFile;
        m_pTextures         = new x_bitmap[ m_NTexturesOwned ];
        ASSERT( m_pTextures != NULL );
    }

    //---   Load the texture data
    for( i = 0; i < hdr.NTexturesInFile; i++ )
    {
        s32 NBytesRead;

        VERIFY( m_pTextures[i].Load( pByteStream, NBytesRead ) );
        pByteStream += NBytesRead;
        VRAM_Register( m_pTextures[i] );
    }

    // create the mesh texture array (for overriding a meshes default texture)
    m_MeshTextureArray = (x_bitmap**)x_malloc( sizeof(x_bitmap*) * m_NMeshes );
    ASSERT( m_MeshTextureArray != NULL );
    x_memset( m_MeshTextureArray, NULL, sizeof(x_bitmap*) * m_NMeshes );

    //---   Check if any pointer data is in the file
    RawDataSize = hdr.NPosPtrs;
    RawDataSize += hdr.NUVPtrs;
    RawDataSize += hdr.NRGBAPtrs;
    RawDataSize += hdr.NNormalPtrs;

    if( RawDataSize == 0 )
        return;

    m_pRawPtrData = (u32*)x_malloc( sizeof(u32) * RawDataSize );
    ASSERT( m_pRawPtrData != NULL );

    //---   Read in data for positions, UVs, color, and normal ptrs
    x_memcpy( m_pRawPtrData, pByteStream, RawDataSize * sizeof(u32) );
    pByteStream += RawDataSize * sizeof(u32);

    u32 uCurPtr;
    u32 uPacketStart = (u32)m_Packets;

    //---   The pointers are offsets at this point, so the packet base address must be
    //      added to get the correct addresses for the pointers (loop unrolled 4 times)
    for( uCurPtr = 0; uCurPtr < RawDataSize - 4; uCurPtr += 4 )
    {
        m_pRawPtrData[uCurPtr]   += uPacketStart;
        m_pRawPtrData[uCurPtr+1] += uPacketStart;
        m_pRawPtrData[uCurPtr+2] += uPacketStart;
        m_pRawPtrData[uCurPtr+3] += uPacketStart;
    }
    //---   Catch the elements at the end of the array
    for( ; uCurPtr < RawDataSize; uCurPtr++ )
        m_pRawPtrData[uCurPtr] += uPacketStart;

    uCurPtr = 0;

    //---   Assign position pointers to meshes
    if( hdr.NPosPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_POSPTRS )
            {
                m_pMeshes[i].pPositions = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign UV pointers to meshes
    if( hdr.NUVPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_UVPTRS )
            {
                m_pMeshes[i].pUVs = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign color pointers to meshes
    if( hdr.NRGBAPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_RGBAPTRS )
            {
                m_pMeshes[i].pRGBAs = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

    //---   Assign normal pointers to meshes
    if( hdr.NNormalPtrs != 0 )
    {
        for( i = 0; i < m_NMeshes; i++ )
        {
            if( m_pMeshes[i].Flags & MESH_FLAG_CONTAINS_NORMALPTRS )
            {
                m_pMeshes[i].pNormals = (void**)(m_pRawPtrData + uCurPtr);
                uCurPtr += m_pMeshes[i].NVerts;
            }
        }
    }

}

//==========================================================================

void QGeom::ActivateMicroCode( void )
{
    //---   register the microcode if needed
    if ( s_MicroCodeHandle < 0 )
    {
        s_MicroCodeHandle = PS2_RegisterMicroCode( "Geometry",
                                                   &VUM_Geom_Code,
                                                   NULL );
    }
    PS2_ActivateMicroCode( s_MicroCodeHandle );
}

//==========================================================================

static
void AsmM4_Mult( matrix4& rDest, matrix4& rSrc1, matrix4& rSrc2 )
{
    // VU0 Assembly version:
    asm __volatile__
    ("

        LQC2    vf08, 0x00(%1)  # load the Src1 matrix vectors into vf8..vf11
        LQC2    vf09, 0x10(%1)
        LQC2    vf10, 0x20(%1)
        LQC2    vf11, 0x30(%1)

        LQC2    vf12, 0x00(%2)  # load the src2 matrix vectors into vf12..vf15
        LQC2    vf13, 0x10(%2)
        LQC2    vf14, 0x20(%2)
        LQC2    vf15, 0x30(%2)

        VMULAx.xyzw     ACC,    vf08,   vf12x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf12y
        VMADDAz.xyzw    ACC,    vf10,   vf12z
        VMADDw.xyzw    vf02,    vf11,   vf12w

        VMULAx.xyzw     ACC,    vf08,   vf13x       # do the second column
        VMADDAy.xyzw    ACC,    vf09,   vf13y
        VMADDAz.xyzw    ACC,    vf10,   vf13z
        VMADDw.xyzw    vf03,    vf11,   vf13w

        VMULAx.xyzw     ACC,    vf08,   vf14x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf14y
        VMADDAz.xyzw    ACC,    vf10,   vf14z
        VMADDw.xyzw    vf04,    vf11,   vf14w

        VMULAx.xyzw     ACC,    vf08,   vf15x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf15y
        VMADDAz.xyzw    ACC,    vf10,   vf15z
        VMADDw.xyzw    vf05,    vf11,   vf15w

        SQC2    vf02, 0x00(%0)        # store the result
        SQC2    vf03, 0x10(%0)
        SQC2    vf04, 0x20(%0)
        SQC2    vf05, 0x30(%0)

    ": "+r" (&rDest.M[0][0]) : "r" (&rSrc1) , "r" (&rSrc2) );
}

//==========================================================================

static f32 s_XFOV = -1.0f;
static f32 s_Log2XC1 = -1.0f;

void QGeom::PreRender( matrix4* pShadowMatrix )
{
    sceDmaTag *pHeader;
    t_PreRenderData *pData;
    sceDmaTag *pFooter;

    matrix4         L2W __attribute__ ((aligned(16)));

    //---   be sure microcode is loaded
    ActivateMicroCode();

    //---   grab a chunk of the display list for the scene packet
    pHeader = (sceDmaTag *)pDList;
    pDList += sizeof( sceDmaTag );
    pData = (t_PreRenderData *)pDList;
    pDList += sizeof( t_PreRenderData );
    pFooter = (sceDmaTag *)pDList;
    pDList += sizeof( sceDmaTag );

    DMAHELP_BuildTagRef( pHeader, (u32)&s_PreRenderHead, sizeof( s_PreRenderHead ) );

    ENG_GetW2S( pData->L2S );
    ENG_GetW2C( pData->L2C );
    ENG_GetC2S( pData->C2S );
    L2W = m_L2W;
    if ( pShadowMatrix )
        L2W = *pShadowMatrix * L2W;
    AsmM4_Mult( pData->L2S, pData->L2S, L2W );
    AsmM4_Mult( pData->L2C, pData->L2C, L2W );

    DMAHELP_BuildTagCont( &pData->DMA, sizeof( *pData ) - sizeof( sceDmaTag ) );

    DMAHELP_BuildTagRef( pFooter, (u32)&s_PreRenderTail, sizeof( s_PreRenderTail ) );

    f32 XFOV;
    ENG_GetActiveView()->GetXFOV( XFOV );
    if ( XFOV != s_XFOV )
    {
        f32 FSX = 1.0f / x_tan( XFOV * 0.5f );
        s_Log2XC1 = x_fastlog2f( FSX * 640.0f * 0.5f );
        s_XFOV = XFOV;
    }
}

//==========================================================================

f32 QGeom::ComputeMipK( s32 SubMeshIndex, view* pView )
{
    radian          XFOV;
    f32             FSX, XC1;
    f32             ViewZ, K;
    t_GeomSubMesh*  pSubMesh;

    //---   get a ptr to the sub mesh
    pSubMesh = &m_pSubMeshes[SubMeshIndex];

    //---   okay, this will need a little explanation...
    //      On the PS2, the mip level is given by LOD = (log2(1/Q)<<L) + K
    //      Since we are using homogenous matrices, 1/Q = ViewZ
    //      Further, L should be zero (using an L would scale the mip values
    //      astronomically), so LOD = log2(ViewZ) + K
    //      This will give us a nice logarithmic function, which tells you
    //      which mip level to use depending on Z. By altering K, we shift
    //      the mip levels up or down. To get a nice value for K, we should
    //      figure out at what Z depth the ratio of texture pixels to screen
    //      pixels is 1:1. This will depend on the texture pixel size and
    //      the current camera settings. The world pixel size we have
    //      already pre-calculated. So...using this formula for screen
    //      pixel size (don't really understand this part, but look it up
    //      in a graphics book):
    //          ScreenPixelSize = XC1 * WorldPixelSize / ViewZ
    //      We want to find where the ScreenPixelSize = 1, so...
    //          1 = XC1 * WPS / ViewZ
    //      OR
    //          ViewZ = XC1 * WPS
    //      Now that we get the value of Z where there is a 1:1 ratio between
    //      the screen pixel size and texture pixel size, plug it into our
    //      LOD equation so that we are at LOD = 0 for this Z value
    //          K = -log2(ViewZ)

    pView->GetXFOV( XFOV );
    FSX = 1.0f / x_tan( XFOV * 0.5f );
    XC1 = FSX * 640.0f * 0.5f;
    ViewZ = XC1 * pSubMesh->AvgPixelSize;
    K = -(f32)x_fastlog2f(ViewZ);

    return K;
}

//==========================================================================

void QGeom::ComputeLOD( s32 MeshIndex,
                        f32 MipK,
                        vector3 &PlaneN,
                        f32 PlaneD,
                        f32& MinLOD,
                        f32& MaxLOD )
{
    f32             MinZ, MaxZ;
    f32             ViewZ;
    vector3         Pos;
    t_GeomMesh*     pMesh;

    //---   grab some initial data
    pMesh = &m_pMeshes[MeshIndex];
    Pos = m_L2W.GetTranslation();

    //---   some brief notes about this calculation:
    //      Coming into this function, PlaneN is the normal to the Mip Plane,
    //      PlaneD, is the distance from the origin in world space to the
    //      camera. So the distance along the z-axis from the camera to
    //      any point is:  dot(PlaneN, Point) - PlaneD

    ViewZ = (PlaneN.X * Pos.X) +
            (PlaneN.Y * Pos.Y) +
            (PlaneN.Z * Pos.Z) -
            PlaneD;
    
    //---   ViewZ now contains the distance between the center of our object
    //      to the camera along the Z axis
    //      So....MinZ and MaxZ are easy enough to calculate
    MinZ = ViewZ - pMesh->Radius;
    MaxZ = ViewZ + pMesh->Radius;

    //---   Clamp to 1
    MinZ = MAX(MinZ,1.0f);
    MaxZ = MAX(MaxZ,1.0f);

    //---   now that we have the min and max z values, and we have K...
    //      look at the comments in ComputeMipK() for an explanation
    MinLOD = x_fastlog2f(MinZ) + MipK;
    MaxLOD = x_fastlog2f(MaxZ) + MipK;
}

//==========================================================================

static
f32 CalcScaleMagnitude( matrix4& L2W )
{
    s32     LargestScaleAxis;
    f32     Scale[3];

    //---   calculate the scale on each axis...w/o the square root
    Scale[0] = L2W.M[0][0] * L2W.M[0][0] +
               L2W.M[0][1] * L2W.M[0][1] +
               L2W.M[0][2] * L2W.M[0][2];
    Scale[1] = L2W.M[1][0] * L2W.M[1][0] +
               L2W.M[1][1] * L2W.M[1][1] +
               L2W.M[1][2] * L2W.M[1][2];
    Scale[2] = L2W.M[2][0] * L2W.M[2][0] +
               L2W.M[2][1] * L2W.M[2][1] +
               L2W.M[2][2] * L2W.M[2][2];

    //---   now we can figure out which axis has the largest scale
    LargestScaleAxis = 0;
    if ( Scale[1] > Scale[LargestScaleAxis] )
        LargestScaleAxis = 1;
    if ( Scale[2] > Scale[LargestScaleAxis] )
        LargestScaleAxis = 2;

    //---   apply the square root to get the proper magnitude
    return x_sqrt( Scale[LargestScaleAxis] );
}

//==========================================================================

void QGeom::RenderMesh( s32 MeshID, xbool SkipVisCheck )
{
    u32         CurrentRenderFlags;
    u32         CurrentBlendMode;
    u8          CurrentFixedAlpha;


    t_TexData       Data;
    vector3         Location;
    matrix4         W2V;
    vector3         Eye;
    vector3         MipPlaneN;
    f32             MipPlaneD       = 0.0f;
    f32             Scale           = 1.0f ;

    Data.pMesh = NULL;
    Data.CurrentMeshID = -1;
    Data.MipPlaneD = 0.0f;
    Data.pTexture = NULL;
    Data.pLastTexture = NULL;
    Data.pEnvMapTexture = NULL;

    ASSERT( ENG_GetRenderMode() );
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    //---   get pointer to mesh
    Data.pMesh = &m_pMeshes[ MeshID ];

    Data.pView = ENG_GetActiveView();

    //---   perform trivial rejections first
    if ( !(Data.pMesh->Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
        return;

    if ( Data.pMesh->Flags & MESH_FLAG_SHADOW )
        return; // You should use QGeom::RenderMeshAsShadow in this case!

    //---   gather info for visibility checks
    Location = m_L2W.GetTranslation();

    //---   check bounding sphere
    if ( SkipVisCheck == FALSE )
    {
        Scale = CalcScaleMagnitude( m_L2W );

        if ( !Data.pView->SphereInView( view::V_WORLD, Location,
                                        Data.pMesh->Radius * Scale ) )
            return;
    }

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );
    ENG_GetBlendMode( CurrentBlendMode, CurrentFixedAlpha );

    u32 NewRenderFlags = CurrentRenderFlags;
    u32 NewBlendMode = CurrentBlendMode;
    u8  NewFixedAlpha = CurrentFixedAlpha;

    //---   activate the microcode and load the matrices
    PreRender();

    //---   Send the lighting matrix
    if ( Data.pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING )
    {
        SendLightingMatrix();
    }

    //---   send the NL2V matrix
//    if ( Data.pMesh->Flags & MESH_FLAG_ENVMAPPED )
//    {
//        SendNL2VMatrix();
//    }

    xbool bPerformLight = (Data.pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING);
    xbool bPerformEnvmap = (Data.pMesh->Flags & MESH_FLAG_ENVMAPPED);
    if (bPerformEnvmap)
        return;

    //---   Prepare for LOD computation
    if ( Data.pMesh->Flags & MESH_FLAG_USE_MIPS )
    {
        Data.pView->GetW2VMatrix( W2V );
        Data.pView->GetPosition( Eye );
        MipPlaneN.X = W2V.M[0][2];
        MipPlaneN.Y = W2V.M[1][2];
        MipPlaneN.Z = W2V.M[2][2];

        MipPlaneD   = (MipPlaneN.X * Eye.X) +
                      (MipPlaneN.Y * Eye.Y) +
                      (MipPlaneN.Z * Eye.Z);
    }


    //---   loop through the sub-meshes
    for ( Data.SubMeshID = Data.pMesh->FirstSubMesh;
          Data.SubMeshID < Data.pMesh->FirstSubMesh + Data.pMesh->NSubMeshes;
          ++Data.SubMeshID )
    {
        u32         PacketAddress;
        sceDmaTag*  pDMA;

        xbool bPerformClip = TRUE;

        if (m_pSubMeshes[ Data.SubMeshID ].MeshID != Data.CurrentMeshID)
        {
            //---   Save the current mesh id and setup the rendering for this mesh
            Data.CurrentMeshID = m_pSubMeshes[ Data.SubMeshID ].MeshID;

            //---   get pointer to mesh
            Data.pMesh = &m_pMeshes[ Data.CurrentMeshID ];

            //---   perform trivial rejections first
            if ( !(Data.pMesh->Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
                continue;

            if ( Data.pMesh->Flags & MESH_FLAG_SHADOW )
                continue; // You should use QGeom::RenderMeshAsShadow in this case!

            //---   Send the lighting matrix
            if (bPerformLight)
            {
                SendLightingMatrix();
            }

            //---   send the NL2V matrix
//            if (bPerformEnvmap)
//                SendNL2VMatrix();

            //---   Prepare for LOD computation
            if ( Data.pMesh->Flags & MESH_FLAG_USE_MIPS )
                CalcMIPPlanes( Data );
        }
        SendSubmeshFlags( bPerformClip, bPerformLight, bPerformEnvmap, FALSE );

        s32 nMaterialStages = 1;
        if (m_pMaterials)
        {
            if (m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0)
                nMaterialStages = m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].nStages;
        }

        s32 loop;
        for (loop=0; loop<nMaterialStages; ++loop)
        {
            //---   activate the texture for this submesh
            if ( m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0 )
                ActivateTexture( Data, loop );

            NewRenderFlags = CurrentRenderFlags;
            NewBlendMode = CurrentBlendMode;
            NewFixedAlpha = CurrentFixedAlpha;

            if (nMaterialStages > 1)
            {
                s_PreviousMaterialLoop = loop;
                // we need to worry about multipass
                if (loop == 0)
                {
                    // first pass
                    // ATE = 0, ATST = 1, AREF = 0x7F, AFAIL = 0, DATE = 0, DATM = 0, ZTE = 1, ZTST = 3
                    // A = 0, B = 1, C = 3, D = 1, Fix = 0x7F
                    NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                    NewRenderFlags |=  ENG_ALPHA_BLEND_ON;
                    NewBlendMode =     ENG_BLEND_MULTI_1;
                }
                else
                {
                    // second pass
                    // ATE = 1, ATST = 2, AREF = 0x7F, AFAIL = 0, DATE = 0, DATM = 0, ZTE = 1, ZTST = 2
                    // A = 1, B = 0, C = 0, D = 0, Fix = 0
                    NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                    NewRenderFlags |=  ENG_ALPHA_TEST_ON;
                    NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                    NewRenderFlags |=  ENG_ALPHA_BLEND_ON;
                    NewBlendMode =     ENG_BLEND_MULTI_2;
                }
            }
            else if (m_pSubMeshes[ Data.SubMeshID ].Flags & SUBMESH_FLAG_ALPHA_P)
            {
                // setup for punchthru
                // let's see...   Alpha Test On, Alpha Blend Off...
                NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                NewRenderFlags |=  ENG_ALPHA_TEST_ON;
            }
            else if (m_pSubMeshes[ Data.SubMeshID ].Flags & SUBMESH_FLAG_ALPHA_T)
            {
                // setup for transparency
                // Alpha Test On, Alpha Blend On...
                NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                NewRenderFlags |=  ENG_ALPHA_TEST_ON;
                NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                NewRenderFlags |=  ENG_ALPHA_BLEND_ON;
                NewBlendMode =     ENG_BLEND_NORMAL;
            }
//            if (bPerformEnvmap)
//            {
//                NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
//                NewRenderFlags |=  ENG_ALPHA_TEST_ON;
//                NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
//                NewRenderFlags |=  ENG_ALPHA_BLEND_ON;
//                NewBlendMode =    (ENG_BLEND_ADDITIVE | ENG_BLEND_FIXED_ALPHA);
//                NewFixedAlpha =    58;
//            }

            //-- just set it, a redundancy check like this exists in the ps2_engine code
            //if (NewRenderFlags != CurrentRenderFlags)
                ENG_SetRenderFlags( NewRenderFlags );
            //if (NewBlendMode != CurrentBlendMode || NewFixedAlpha != CurrentFixedAlpha)
                ENG_SetBlendMode( NewBlendMode, NewFixedAlpha );



            //---   figure out the packet address
            PacketAddress = (u32)&m_Packets[ m_pSubMeshes[ Data.SubMeshID ].PacketOffset ];
            ASSERT( (PacketAddress & 0x0f) == 0 );

            //---   Setup dma reference to submesh geometry
            pDMA = (sceDmaTag*)pDList;
            pDList += sizeof(sceDmaTag);
            DMAHELP_BuildTagCall( pDMA, (u32)PacketAddress, 0 );

            //---   Update geom stats
            if ( s_pStatNVerts )
                *s_pStatNVerts = *s_pStatNVerts + m_pSubMeshes[ Data.SubMeshID ].NVertsTrans;
            if ( s_pStatNTris )
                *s_pStatNTris = *s_pStatNTris + m_pSubMeshes[ Data.SubMeshID ].NTris;
            if ( s_pStatNBytes )
                *s_pStatNBytes = *s_pStatNBytes + m_pSubMeshes[ Data.SubMeshID ].PacketSize;
        }
        /*  I think this code is unnecessary and will only cause unnneeded start changes.
        if (   NewRenderFlags != CurrentRenderFlags
            || NewBlendMode != CurrentBlendMode
            || NewFixedAlpha != CurrentFixedAlpha
            || m_pSubMeshes[ Data.SubMeshID ].Flags) 
        {
            ENG_SetRenderFlags( CurrentRenderFlags );
            ENG_SetBlendMode( CurrentBlendMode, CurrentFixedAlpha );
        }
        */
    }
}

//==========================================================================

xbool QGeom::PokeNewSubmeshData( t_TexData &Data, xbool bPerformLight, xbool bPerformEnvmap, xbool SkipVisCheck )
{
    //---   Save the current mesh id and setup the rendering for this mesh
    Data.CurrentMeshID = m_pSubMeshes[ Data.SubMeshID ].MeshID;

    //---   get pointer to mesh
    Data.pMesh = &m_pMeshes[ Data.CurrentMeshID ];

    //---   perform trivial rejections first
    if ( !(Data.pMesh->Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
        return FALSE;

    if ( Data.pMesh->Flags & MESH_FLAG_SHADOW )
        return FALSE; // You should use QGeom::RenderMeshAsShadow in this case!

    //---   Send the lighting matrix
    if (bPerformLight)
    {
        SendLightingMatrix();
    }

    //---   send the NL2V matrix
    if (bPerformEnvmap)
        SendNL2VMatrix();

    //---   Prepare for LOD computation
    if ( Data.pMesh->Flags & MESH_FLAG_USE_MIPS )
        CalcMIPPlanes( Data );

    return TRUE;
}

//==========================================================================

static xbool bNeverRenderClipped = FALSE;
static xbool bOnlyRenderClipped = FALSE;
static xbool bNeverRenderEnvMap = FALSE;
static xbool bOnlyRenderEnvMap = FALSE;
s32 QGeom::RenderSubMeshes_MultiPass( xbool* SubMeshes, 
                                      t_TexData &Data, 
                                      s32 SubMeshFlags, 
                                      xbool SkipVisCheck, 
                                      s32 nStages, 
                                      u32 RenderFlags, 
                                      u32 BlendMode, 
                                      u8 FixedAlpha )
{
    s32 i;
    vector3 PlaneN[6];
    f32     PlaneD[6];
    view*   pView = ENG_GetActiveView();

    matrix4 W2V;
    f32     FX,FY;
    f32     NZ,FZ;

    // Get view info
    pView->GetFrustum( FX, FY );
    pView->GetW2VMatrix( W2V );
    pView->GetZLimits( NZ, FZ );
    pView->GetViewPlanes( view::V_WORLD, PlaneN[3], PlaneD[3],
                                         PlaneN[4], PlaneD[4],
                                         PlaneN[1], PlaneD[1],
                                         PlaneN[2], PlaneD[2],
                                         PlaneN[0], PlaneD[0],
                                         PlaneN[5], PlaneD[5]);
    vector3 ClipPlaneN[6];
    f32     ClipPlaneD[6];


    // Get view info
    pView->GetClipViewPlanes( view::V_WORLD, ClipPlaneN[3], ClipPlaneD[3],
                                             ClipPlaneN[4], ClipPlaneD[4],
                                             ClipPlaneN[1], ClipPlaneD[1],
                                             ClipPlaneN[2], ClipPlaneD[2],
                                             ClipPlaneN[0], ClipPlaneD[0],
                                             ClipPlaneN[5], ClipPlaneD[5]);
    s32 nSubMeshesRendered = 0;
    s32 iFirst = Data.SubMeshID;
    s32 iLast = (Data.SubMeshID + m_pContiguousMaterials[ Data.SubMeshID ]);
    if (iLast > m_NSubMeshes)
        iLast = m_NSubMeshes;
    
    u32 CurrentRenderFlags = RenderFlags;
    u32 CurrentBlendMode = BlendMode;
    u8 CurrentFixedAlpha = FixedAlpha;

    s32 iStage;
    for (iStage=0; iStage<nStages; ++iStage)
    {
        xbool bInit = TRUE;
        for (Data.SubMeshID = iFirst; Data.SubMeshID<iLast; ++Data.SubMeshID)
        {
            u32 Flags = m_pSubMeshes[ Data.SubMeshID ].Flags;
            //xbool bNoBilinear = Flags & SUBMESH_FLAG_NO_BILINEAR;
            Flags &= ~SUBMESH_FLAG_NO_BILINEAR;

            if (!SubMeshes[ Data.SubMeshID ])
                continue;
            if( SubMeshFlags == SUBMESH_FLAG_NONE )
            {
                if( Flags != SUBMESH_FLAG_NONE ) 
                    continue;
            }
            else if( !(Flags & SubMeshFlags) ) 
                continue;

            s32 tempMeshID = m_pSubMeshes[ Data.SubMeshID ].MeshID;
            s32 tempFlags = m_pMeshes[ tempMeshID ].Flags;
            if( tempMeshID != Data.CurrentMeshID && !(tempFlags & MESH_FLAG_VISIBLE) && !SkipVisCheck ) 
                continue;
            
            if( m_pMeshes[ tempMeshID ].Flags & MESH_FLAG_SPECIAL ) 
                continue;


            Data.pMesh = &m_pMeshes[ tempMeshID ];

            xbool bPerformClip = FALSE;
            xbool bPerformLight = (Data.pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING);
            xbool bPerformEnvmap = (Data.pMesh->Flags & MESH_FLAG_ENVMAPPED);
bPerformEnvmap = FALSE;

            if (!bPerformEnvmap && bOnlyRenderEnvMap)
                continue;
            if (bPerformEnvmap && bNeverRenderEnvMap)
                continue;

            s_ClipStatisticsTotal += m_pSubMeshes[ Data.SubMeshID ].NTris;

            if( !SkipVisCheck )
            {
                xbool bSubMeshInView = TRUE;
                SGeomSubMesh *pSubMesh = &m_pSubMeshes[ Data.SubMeshID ];

                vector3 Mid = W2V * pSubMesh->MidPoint;

                ////////////////////////////////////////////////////////////////
                // Convert pt into camera space
                ////////////////////////////////////////////////////////////////
                if(   ( Mid.Z      >= NZ  )
                   && ( Mid.X * FX <= Mid.Z )
                   && (-Mid.X * FX <= Mid.Z )
                   && ( Mid.Y * FY <= Mid.Z )
                   && (-Mid.Y * FY <= Mid.Z ))
                {
                    bSubMeshInView = TRUE;
                }
                else
                {
                    for( i=0; i<6; ++i )
                    {
                        vector3 BestPt = pSubMesh->Corner;

                        for (s32 j=0; j<3; ++j)
                        {
                            if( Dot(pSubMesh->Axis[j],PlaneN[i]) > 0 )
                                BestPt  += pSubMesh->Axis[j];
                        }

                        // Check if best point is outside
                        if( (Dot(PlaneN[i],BestPt) + PlaneD[i]) < 0 )
                        {
                            bSubMeshInView = FALSE;
                            break;
                        }
                    }
                }

                if (!bSubMeshInView)
                {
                    s_ClipStatisticsCullCount += m_pSubMeshes[ Data.SubMeshID ].NTris;
                    continue;
                }

                for( i=0; i<6; ++i )
                {
                    vector3 Corner = pSubMesh->Corner;

                    // if the corner is outside of this plane, we clip
                    if ((Dot( Corner, ClipPlaneN[ i ]) + ClipPlaneD[i] ) < 0)
                    {
                        bPerformClip = TRUE;
                        break;
                    }
                    
                    for (s32 j=0; j<3; ++j)
                    {
                        if( Dot(pSubMesh->Axis[j], ClipPlaneN[i]) < 0 )
                            Corner  += pSubMesh->Axis[j];
                    }
                    // if this built corner is outside of the plane, we clip
                    if ((Dot( Corner, ClipPlaneN[ i ]) + ClipPlaneD[i] ) < 0)
                    {
                        bPerformClip = TRUE;
                        break;
                    }
                }

                if (bPerformClip)
                {
                    s_ClipStatisticsClipCount += m_pSubMeshes[ Data.SubMeshID ].NTris;
                }
            }

            if (bOnlyRenderClipped && !bPerformClip)
                continue;
            if (bNeverRenderClipped && bPerformClip)
                continue;


            if (m_pSubMeshes[ Data.SubMeshID ].MeshID != Data.CurrentMeshID)
            {
                if (!PokeNewSubmeshData( Data, bPerformLight, bPerformEnvmap, SkipVisCheck ))
                    continue;
            }

            SendSubmeshFlags( bPerformClip, bPerformLight, bPerformEnvmap, TRUE );

            u32         PacketAddress;
            sceDmaTag*  pDMA;

            // increment the number we've rendered
            if (iStage == 0)
                nSubMeshesRendered++;

            if (bInit)
            {
                VRAM_UseBilinear( TRUE );
                // initialize the material
                if ( m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0 )
                    ActivateTexture( Data, iStage );

                SetMultipassRenderSettings( iStage, RenderFlags, BlendMode );
                if (RenderFlags != CurrentRenderFlags)
                    ENG_SetRenderFlags( RenderFlags );
                if (BlendMode != CurrentBlendMode || FixedAlpha != CurrentFixedAlpha)
                    ENG_SetBlendMode( BlendMode, FixedAlpha );

                if (iStage == 0)
                {
                    sceDmaTag *pBlend = (sceDmaTag *)pDList;
                    pDList += sizeof( sceDmaTag );
                    DMAHELP_BuildTagRef( pBlend, (u32)&s_ContextBlend[ iStage ], sizeof( s_ContextBlend[ iStage ] ) );
                }

                bInit = FALSE;
            }
            //---   activate the texture for this submesh



            PacketAddress = (u32)&m_Packets[ m_pSubMeshes[ Data.SubMeshID ].PacketOffset];
            ASSERT( (PacketAddress & 0x0f) == 0 );

            //---   Setup dma reference to submesh geometry
            pDMA = (sceDmaTag*)pDList;
            pDList += sizeof( sceDmaTag );
            DMAHELP_BuildTagCall( pDMA, (u32)PacketAddress, 0 );

            //---   Update geom stats
            if ( s_pStatNVerts )
                *s_pStatNVerts = *s_pStatNVerts + m_pSubMeshes[ Data.SubMeshID ].NVertsTrans;
            if ( s_pStatNTris )
                *s_pStatNTris = *s_pStatNTris + m_pSubMeshes[ Data.SubMeshID ].NTris;
            if ( s_pStatNBytes )
                *s_pStatNBytes = *s_pStatNBytes + m_pSubMeshes[ Data.SubMeshID ].PacketSize;

        }
    }
    Data.SubMeshID --;
    return nSubMeshesRendered;
}

//==========================================================================

s32 QGeom::RenderSubMeshes( xbool* SubMeshes, xbool SkipVisCheck, s32 SubMeshFlags, xbool SkipInViewCheck )
{
   // s32     i;
    vector3 PlaneN[6];
    f32     PlaneD[6];
    view*   pView = ENG_GetActiveView();

    matrix4 W2V;
    f32     FX,FY;
    f32     NZ,FZ;

    // Get view info
    pView->GetFrustum( FX, FY );
    pView->GetW2VMatrix( W2V );
    pView->GetZLimits( NZ, FZ );
    pView->GetViewPlanes( view::V_WORLD, PlaneN[3], PlaneD[3],
                                         PlaneN[4], PlaneD[4],
                                         PlaneN[1], PlaneD[1],
                                         PlaneN[2], PlaneD[2],
                                         PlaneN[0], PlaneD[0],
                                         PlaneN[5], PlaneD[5]);
    vector3 ClipPlaneN[6];
    f32     ClipPlaneD[6];


    // Get view info
    pView->GetClipViewPlanes( view::V_WORLD, ClipPlaneN[3], ClipPlaneD[3],
                                             ClipPlaneN[4], ClipPlaneD[4],
                                             ClipPlaneN[1], ClipPlaneD[1],
                                             ClipPlaneN[2], ClipPlaneD[2],
                                             ClipPlaneN[0], ClipPlaneD[0],
                                             ClipPlaneN[5], ClipPlaneD[5]);


    (void)SubMeshes;
    u32         CurrentRenderFlags;
    u32         CurrentBlendMode;
    u8          CurrentFixedAlpha;

    t_TexData   Data;
    s32         NSubMeshesRendered  = 0;
    xbool       bFirst              = TRUE;
    Data.pMesh                      = NULL;
    Data.CurrentMeshID              = -1;
    Data.MipPlaneD                  = 0.0f;
    Data.pTexture                   = NULL;
    Data.pLastTexture               = NULL;
    Data.pEnvMapTexture             = NULL;

    if ((SubMeshFlags & SUBMESH_FLAG_ALPHA_P))
    {
        s_ClipStatisticsCullCount = 0;
        s_ClipStatisticsClipCount = 0;
        s_ClipStatisticsTotal = 0;
    }

    ASSERT( ENG_GetRenderMode() );

    // Get the current blend modes and render settings from the engine.

    //---   gather info for visibility checks
    Data.pView = ENG_GetActiveView();

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );
    ENG_GetBlendMode( CurrentBlendMode, CurrentFixedAlpha );

    u32 NewRenderFlags = CurrentRenderFlags;
    u32 NewBlendMode = CurrentBlendMode;
    u8  NewFixedAlpha = CurrentFixedAlpha;

    //---   activate the microcode and load the matrices
    PreRender();


    //---   loop through all the submeshes in the display list
    for( Data.SubMeshID = 0; Data.SubMeshID < m_NSubMeshes; ++Data.SubMeshID )
    {

        s32 nMaterialStages = 1;
        if (m_pMaterials)
        {
            if (m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0)
                nMaterialStages = m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].nStages;
            if (   nMaterialStages > 1
                && m_pContiguousMaterials 
                && m_pContiguousMaterials[ Data.SubMeshID ] > 1 )
            {
                NSubMeshesRendered += RenderSubMeshes_MultiPass( SubMeshes, 
                                                                 Data, 
                                                                 SubMeshFlags,
                                                                 SkipVisCheck,
                                                                 nMaterialStages,
                                                                 CurrentRenderFlags, 
                                                                 CurrentBlendMode, 
                                                                 CurrentFixedAlpha );
                continue;
            }
        }
        if (!SubMeshes[ Data.SubMeshID ])
            continue;

        u32 Flags = m_pSubMeshes[ Data.SubMeshID ].Flags;
        xbool bNoBilinear = Flags & SUBMESH_FLAG_NO_BILINEAR;
        Flags &= ~SUBMESH_FLAG_NO_BILINEAR;

        if( SubMeshFlags == SUBMESH_FLAG_NONE )
        {
            if (Flags != SUBMESH_FLAG_NONE)
                continue;
        }
        else if (!(Flags & SubMeshFlags)) 
            continue;

        s32 tempMeshID = m_pSubMeshes[ Data.SubMeshID ].MeshID;
        s32 tempFlags = m_pMeshes[ tempMeshID ].Flags;
        if (tempMeshID != Data.CurrentMeshID && !(tempFlags & MESH_FLAG_VISIBLE) && !SkipVisCheck) 
            continue;
        
        if (m_pMeshes[ tempMeshID ].Flags & MESH_FLAG_SPECIAL) 
            continue;


        Data.pMesh = &m_pMeshes[ tempMeshID ];

        xbool bPerformClip = FALSE;
        xbool bPerformLight = (Data.pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING);
        xbool bPerformEnvmap = (Data.pMesh->Flags & MESH_FLAG_ENVMAPPED);

bPerformEnvmap = FALSE;

        if (!bPerformEnvmap && bOnlyRenderEnvMap)
            continue;
        if (bPerformEnvmap && bNeverRenderEnvMap)
            continue;

        s_ClipStatisticsTotal += m_pSubMeshes[ Data.SubMeshID ].NTris;

        if( !SkipVisCheck && !SkipInViewCheck )
        {
            SGeomSubMesh *pSubMesh = &m_pSubMeshes[ Data.SubMeshID ];

            EViewTestResults Result = OrientedBBoxInView( 
                                     pSubMesh->Corner,
                                     pSubMesh->Axis,
                                     PlaneN,
                                     PlaneD );//,
                                     //TRUE, TRUE );

            if ( Result == OUTSIDE_VIEW )
            {
                s_ClipStatisticsCullCount += m_pSubMeshes[ Data.SubMeshID ].NTris;
                continue;
            }

            if ( Result == PARTIAL_VIEW )
            {
/*
                Result = OrientedBBoxInView( 
                                        pSubMesh->Corner,
                                        pSubMesh->Axis,
                                        ClipPlaneN,
                                        ClipPlaneD );

                if ( Result == PARTIAL_VIEW )
*/
                {
                    s_ClipStatisticsClipCount += m_pSubMeshes[ Data.SubMeshID ].NTris;
                    bPerformClip = TRUE;
                }
            }
        }

        if (bOnlyRenderClipped && !bPerformClip)
            continue;
        if (bNeverRenderClipped && bPerformClip)
            continue;


        if (m_pSubMeshes[ Data.SubMeshID ].MeshID != Data.CurrentMeshID)
        {
            if (!PokeNewSubmeshData( Data, bPerformLight, bPerformEnvmap, SkipVisCheck ))
                continue;
        }

        SendSubmeshFlags( bPerformClip, bPerformLight, bPerformEnvmap, bFirst );

        u32         PacketAddress;
        sceDmaTag*  pDMA;

        // increment the number we've rendered
        NSubMeshesRendered++;

        s32 loop;
        for (loop=0; loop<nMaterialStages; ++loop)
        {
            NewRenderFlags = CurrentRenderFlags;
            NewBlendMode = CurrentBlendMode;
            NewFixedAlpha = CurrentFixedAlpha;

            if (nMaterialStages > 1)
            {
                // we need to worry about multipass
                s_PreviousMaterialLoop = loop;
                SetMultipassRenderSettings( loop, NewRenderFlags, NewBlendMode );
            }
            else if (m_pSubMeshes[ Data.SubMeshID ].Flags & SUBMESH_FLAG_ALPHA_P)
            {
                SetPunchRenderSettings( NewRenderFlags );
            }
            else if (m_pSubMeshes[ Data.SubMeshID ].Flags & SUBMESH_FLAG_ALPHA_T)
            {
                SetTransparencyRenderSettings( NewRenderFlags, NewBlendMode );
            }
            if (bPerformEnvmap)
            {
                SetEnvmapRenderSettings( NewRenderFlags, NewBlendMode, NewFixedAlpha );
            }
            if (bNoBilinear)
            {
                VRAM_UseBilinear( FALSE );
            }
            else
            {
                VRAM_UseBilinear( TRUE );
            }

            //---   activate the texture for this submesh
            if ( m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0 )
                ActivateTexture( Data, loop );

            ENG_SetRenderFlags( NewRenderFlags );
            ENG_SetBlendMode( NewBlendMode, NewFixedAlpha );

            if (nMaterialStages > 1 && loop == 0)
            {
                sceDmaTag *pBlend = (sceDmaTag *)pDList;
                pDList += sizeof( sceDmaTag );
                DMAHELP_BuildTagRef( pBlend, (u32)&s_ContextBlend[ loop ], sizeof( s_ContextBlend[ loop ] ) );
            }

            PacketAddress = (u32)&m_Packets[ m_pSubMeshes[ Data.SubMeshID ].PacketOffset];
            ASSERT( (PacketAddress & 0x0f) == 0 );

            //---   Setup dma reference to submesh geometry
            pDMA = (sceDmaTag*)pDList;
            pDList += sizeof( sceDmaTag );
            DMAHELP_BuildTagCall( pDMA, (u32)PacketAddress, 0 );

            //---   Update geom stats
            if ( s_pStatNVerts )
                *s_pStatNVerts = *s_pStatNVerts + m_pSubMeshes[ Data.SubMeshID ].NVertsTrans;
            if ( s_pStatNTris )
                *s_pStatNTris = *s_pStatNTris + m_pSubMeshes[ Data.SubMeshID ].NTris;
            if ( s_pStatNBytes )
                *s_pStatNBytes = *s_pStatNBytes + m_pSubMeshes[ Data.SubMeshID ].PacketSize;
        }
    }

    return NSubMeshesRendered;
}

//==========================================================================
void QGeom::RenderSubMeshAtPos( SGeomSubMesh *pSubMesh, s32 iSubMeshIndex, vector3 &pos )
{
    u32         CurrentRenderFlags;
    u32         CurrentBlendMode;
    u8          CurrentFixedAlpha;

    t_TexData       Data;

    Data.pMesh = NULL;
    Data.CurrentMeshID = -1;
    Data.MipPlaneD = 0.0f;
    Data.pTexture = NULL;
    Data.pLastTexture = NULL;
    Data.pEnvMapTexture = NULL;
    Data.SubMeshID = iSubMeshIndex;

    ENG_GetRenderFlags( CurrentRenderFlags );
    ENG_GetBlendMode( CurrentBlendMode, CurrentFixedAlpha );

    u32 NewRenderFlags = CurrentRenderFlags;
    u32 NewBlendMode = CurrentBlendMode;
    u8  NewFixedAlpha = CurrentFixedAlpha;


    ASSERT( ENG_GetRenderMode() );

    // Get the current blend modes and render settings from the engine.

    //---   gather info for visibility checks
    Data.pView = ENG_GetActiveView();

    //---   activate the microcode and load the matrices
    {
        sceDmaTag *pHeader;
        t_PreRenderData *pData;
        sceDmaTag *pFooter;

        matrix4         L2W __attribute__ ((aligned(16)));

        //---   be sure microcode is loaded
        ActivateMicroCode();

        //---   grab a chunk of the display list for the scene packet
        pHeader = (sceDmaTag *)pDList;
        pDList += sizeof( sceDmaTag );
        pData = (t_PreRenderData *)pDList;
        pDList += sizeof( t_PreRenderData );
        pFooter = (sceDmaTag *)pDList;
        pDList += sizeof( sceDmaTag );

        DMAHELP_BuildTagRef( pHeader, (u32)&s_PreRenderHead, sizeof( s_PreRenderHead ) );

        ENG_GetW2S( pData->L2S );
        ENG_GetW2C( pData->L2C );
        ENG_GetC2S( pData->C2S );
        L2W = m_L2W;

        ///
        L2W.Translate( pos );
        ///

        AsmM4_Mult( pData->L2S, pData->L2S, L2W );
        AsmM4_Mult( pData->L2C, pData->L2C, L2W );

        DMAHELP_BuildTagCont( &pData->DMA, sizeof( *pData ) - sizeof( sceDmaTag ) );

        DMAHELP_BuildTagRef( pFooter, (u32)&s_PreRenderTail, sizeof( s_PreRenderTail ) );
    }



    s32 tempMeshID = pSubMesh->MeshID;

    Data.pMesh = &m_pMeshes[ tempMeshID ];

    xbool bPerformClip = FALSE;
    xbool bPerformLight = (Data.pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING);
    xbool bPerformEnvmap = (Data.pMesh->Flags & MESH_FLAG_ENVMAPPED);
    xbool bNoBilinear = Data.pMesh->Flags & SUBMESH_FLAG_NO_BILINEAR;
bPerformEnvmap = FALSE;

    bPerformClip = TRUE;

    if (bOnlyRenderClipped && !bPerformClip)
        return;
    if (bNeverRenderClipped && bPerformClip)
        return;


    if (pSubMesh->MeshID != Data.CurrentMeshID)
    {
        //---   Save the current mesh id and setup the rendering for this mesh
        Data.CurrentMeshID = pSubMesh->MeshID;

        //---   get pointer to mesh
        Data.pMesh = &m_pMeshes[ Data.CurrentMeshID ];

        //---   perform trivial rejections first
        if ( !(Data.pMesh->Flags & MESH_FLAG_VISIBLE) )
            return;

        if ( Data.pMesh->Flags & MESH_FLAG_SHADOW )
            return; // You should use QGeom::RenderMeshAsShadow in this case!

        //---   Send the lighting matrix
        if (bPerformLight)
        {
            SendLightingMatrix();
        }

        //---   send the NL2V matrix
        if (bPerformEnvmap)
            SendNL2VMatrix();

        //---   Prepare for LOD computation
        if ( Data.pMesh->Flags & MESH_FLAG_USE_MIPS )
            CalcMIPPlanes( Data );
    }

    SendSubmeshFlags( bPerformClip, bPerformLight, bPerformEnvmap, FALSE );

    u32         PacketAddress;
    sceDmaTag*  pDMA;

    s32 nMaterialStages = 1;
    if (m_pMaterials)
    {
        if (m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0)
            nMaterialStages = m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].nStages;
    }

    s32 loop;
    for (loop=0; loop<nMaterialStages; ++loop)
    {
        //---   activate the blend settings for this Material Stage
        if (s_PreviousMaterialLoop != loop || nMaterialStages != 1)
        {
            s_PreviousMaterialLoop = loop;      // trying to ensure we only do the blend when we need it.

            sceDmaTag *pBlend = (sceDmaTag *)pDList;
            pDList += sizeof( sceDmaTag );

            DMAHELP_BuildTagRef( pBlend, (u32)&s_ContextBlend[ loop ], sizeof( s_ContextBlend[ loop ] ) );
        }
        //---   activate the texture for this submesh
        if ( m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0 )
            ActivateTexture( Data, loop );

        NewRenderFlags = CurrentRenderFlags;
        NewBlendMode = CurrentBlendMode;
        NewFixedAlpha = CurrentFixedAlpha;
        if (nMaterialStages > 1)
        {
            // we need to worry about multipass
            s_PreviousMaterialLoop = loop;
            SetMultipassRenderSettings( loop, NewRenderFlags, NewBlendMode );
        }
        else if (m_pSubMeshes[ Data.SubMeshID ].Flags & SUBMESH_FLAG_ALPHA_P)
            SetPunchRenderSettings( NewRenderFlags );
        else if (m_pSubMeshes[ Data.SubMeshID ].Flags & SUBMESH_FLAG_ALPHA_T)
            SetTransparencyRenderSettings( NewRenderFlags, NewBlendMode );
        if (bPerformEnvmap)
            SetEnvmapRenderSettings( NewRenderFlags, NewBlendMode, NewFixedAlpha );
        if (bNoBilinear)
            VRAM_UseBilinear( FALSE );
        else
            VRAM_UseBilinear( TRUE );

        //---   activate the texture for this submesh
        if ( m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0 )
            ActivateTexture( Data, loop );

        ENG_SetRenderFlags( NewRenderFlags );
        ENG_SetBlendMode( NewBlendMode, NewFixedAlpha );

        //---   figure out the packet address
        PacketAddress = (u32)&m_Packets[ m_pSubMeshes[ Data.SubMeshID ].PacketOffset];
        ASSERT( (PacketAddress & 0x0f) == 0 );

        //---   Setup dma reference to submesh geometry
        pDMA = (sceDmaTag*)pDList;
        pDList += sizeof( sceDmaTag );
        DMAHELP_BuildTagCall( pDMA, (u32)PacketAddress, 0 );

        //---   Update geom stats
        if ( s_pStatNVerts )
            *s_pStatNVerts = *s_pStatNVerts + m_pSubMeshes[ Data.SubMeshID ].NVertsTrans;
        if ( s_pStatNTris )
            *s_pStatNTris = *s_pStatNTris + m_pSubMeshes[ Data.SubMeshID ].NTris;
        if ( s_pStatNBytes )
            *s_pStatNBytes = *s_pStatNBytes + m_pSubMeshes[ Data.SubMeshID ].PacketSize;
    }
    /*dont worry about setting it back... this is bad
    if (   NewRenderFlags != CurrentRenderFlags
        || NewBlendMode != CurrentBlendMode
        || NewFixedAlpha != CurrentFixedAlpha
        || m_pSubMeshes[ Data.SubMeshID ].Flags) 
    {
        ENG_SetRenderFlags( CurrentRenderFlags );
        ENG_SetBlendMode( CurrentBlendMode, CurrentFixedAlpha );
    }
    */
}


//==========================================================================

f32 s_fLightingTweak = 1.5f;
f32 s_fAmbientTweak = 1.5f;

void QGeom::SendLightingMatrix( void )
{
    matrix4         NL2W        __attribute__ ((aligned(16)));
    matrix4         LightDir    __attribute__ ((aligned(16)));
//    matrix4         LightColors __attribute__ ((aligned(16)));
    lighting        EngLighting;
    t_LightPacket*  LP;

    //---   grab a chunk of the display list for the lighting packet
    LP = (t_LightPacket*)pDList;
    pDList += sizeof(t_LightPacket);

    // Start ASB2005 PS2 lighting model
    ENG_GetActiveLighting( EngLighting );

    if ( EngLighting.Directional[0].Active )
    {
        LightDir.M[0][0] = -EngLighting.Directional[0].Direction.X;
        LightDir.M[1][0] = -EngLighting.Directional[0].Direction.Y;
        LightDir.M[2][0] = -EngLighting.Directional[0].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero...
        LightDir.M[0][0] = 0.0f;
        LightDir.M[1][0] = 0.0f;
        LightDir.M[2][0] = 0.0f;
    }
    LightDir.M[3][0] = 0.0f;
    if ( EngLighting.Directional[1].Active )
    {
        LightDir.M[0][1] = -EngLighting.Directional[1].Direction.X;
        LightDir.M[1][1] = -EngLighting.Directional[1].Direction.Y;
        LightDir.M[2][1] = -EngLighting.Directional[1].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero
        LightDir.M[0][1] = 0.0f;
        LightDir.M[1][1] = 0.0f;
        LightDir.M[2][1] = 0.0f;
    }
    LightDir.M[3][1] = 0.0f;
    if ( EngLighting.Directional[2].Active )
    {
        LightDir.M[0][2] = -EngLighting.Directional[2].Direction.X;
        LightDir.M[1][2] = -EngLighting.Directional[2].Direction.Y;
        LightDir.M[2][2] = -EngLighting.Directional[2].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero
        LightDir.M[0][2] = 0.0f;
        LightDir.M[1][2] = 0.0f;
        LightDir.M[2][2] = 0.0f;
    }
    LightDir.M[3][2] = 0.0f;
    LightDir.M[0][3] = 0.0f;
    LightDir.M[1][3] = 0.0f;
    LightDir.M[2][3] = 0.0f;
    LightDir.M[3][3] = 1.0f;

    //---   Figure out the Light Colors matrix
    if ( EngLighting.Directional[0].Active )
    {
        LP->LightCol.M[0][0] = EngLighting.Directional[0].Color.R;
        LP->LightCol.M[0][1] = EngLighting.Directional[0].Color.G;
        LP->LightCol.M[0][2] = EngLighting.Directional[0].Color.B;
        LP->LightCol.M[0][3] = EngLighting.Directional[0].Color.A;

        s32 i;
        for (i=0; i<4; ++i)
        {
            s32 temp = (s32)(((f32)LP->LightCol.M[ 0 ][ i ]) * s_fLightingTweak);
            if (temp > 127)
                temp = 127;
            LP->LightCol.M[ 0 ][ i ] = (u8)temp;
        }
    }
    else
    {
        LP->LightCol.M[0][0] = 0.0f;
        LP->LightCol.M[0][1] = 0.0f;
        LP->LightCol.M[0][2] = 0.0f;
        LP->LightCol.M[0][3] = 0.0f;
    }
    if ( EngLighting.Directional[1].Active )
    {
        LP->LightCol.M[1][0] = EngLighting.Directional[1].Color.R;
        LP->LightCol.M[1][1] = EngLighting.Directional[1].Color.G;
        LP->LightCol.M[1][2] = EngLighting.Directional[1].Color.B;
        LP->LightCol.M[1][3] = EngLighting.Directional[1].Color.A;

        s32 i;
        for (i=0; i<4; ++i)
        {
            s32 temp = (s32)(((f32)LP->LightCol.M[ 1 ][ i ]) * s_fLightingTweak);
            if (temp > 127)
                temp = 127;
            LP->LightCol.M[ 1 ][ i ] = (u8)temp;
        }
    }
    else
    {
        LP->LightCol.M[1][0] = 0.0f;
        LP->LightCol.M[1][1] = 0.0f;
        LP->LightCol.M[1][2] = 0.0f;
        LP->LightCol.M[1][3] = 0.0f;
    }
    if ( EngLighting.Directional[2].Active )
    {
        LP->LightCol.M[2][0] = EngLighting.Directional[2].Color.R;
        LP->LightCol.M[2][1] = EngLighting.Directional[2].Color.G;
        LP->LightCol.M[2][2] = EngLighting.Directional[2].Color.B;
        LP->LightCol.M[2][3] = EngLighting.Directional[2].Color.A;
        s32 i;
        for (i=0; i<4; ++i)
        {
            s32 temp = (s32)(((f32)LP->LightCol.M[ 2 ][ i ]) * s_fLightingTweak);
            if (temp > 127)
                temp = 127;
            LP->LightCol.M[ 2 ][ i ] = (u8)temp;
        }
    }
    else
    {
        LP->LightCol.M[2][0] = 0.0f;
        LP->LightCol.M[2][1] = 0.0f;
        LP->LightCol.M[2][2] = 0.0f;
        LP->LightCol.M[2][3] = 0.0f;
    }
    LP->LightCol.M[3][0] = EngLighting.Ambient.R;
    LP->LightCol.M[3][1] = EngLighting.Ambient.G;
    LP->LightCol.M[3][2] = EngLighting.Ambient.B;
    LP->LightCol.M[3][3] = EngLighting.Ambient.A;

    s32 i;
    for (i=0; i<4; ++i)
    {
        s32 temp = (s32)(((f32)LP->LightCol.M[ 3 ][ i ]) * s_fAmbientTweak);
        if (temp > 127)
            temp = 127;
        LP->LightCol.M[ 3 ][ i ] = (u8)temp;
    }

    //---   setup the DMA tag
    DMAHELP_BuildTagCont( &LP->DMA, sizeof(t_LightPacket) - sizeof(sceDmaTag) );

    //---   setup the VIF Unpack info
    LP->VIF[0] = 0;
    LP->VIF[1] = VIFHELP_SET_STCYCL( 1, 1, 0 );
    LP->VIF[2] = 0;
    u32 addr = 1016;//MATRIX_LIGHT_DIR_ADDR;
    u32 size = (sizeof( LP->LightDir1 ) + sizeof( LP->LightDir2 ) + sizeof( LP->LightDir3 ) + sizeof( LP->LightCol )) >> 4;
    VIFHELP_BuildTagUnpack( &LP->VIF[3], addr, size, VIFHELP_UNPACK_V4_32, FALSE, FALSE, TRUE );

    //---   Calculate a normalized L2W
    //      This turns out to be the L2W without translation
    NL2W = m_L2W;
    NL2W.SetTranslation(vector3(0,0,0));

    //---   fill out the matrix
    // ASM Version:
    AsmM4_Mult( LightDir, LightDir, NL2W );
    vector4 dummy;
    LightDir.Transpose();
    LightDir.GetRows( LP->LightDir1, LP->LightDir2, LP->LightDir3, dummy );

}

//==========================================================================

void QGeom::SendNL2VMatrix( void )
{
    t_NL2VPacket*   pPacket;
    matrix4         L2V         __attribute__ ((aligned(16)));

    ENG_GetActiveView()->GetW2VMatrix( L2V );
    AsmM4_Mult( L2V, L2V, m_L2W );
//    L2V.Transpose();
//    L2V.Invert();

    //---   grab a chunk of the display list for the NL2V packet
    pPacket = (t_NL2VPacket*)pDList;
    pDList += sizeof(t_NL2VPacket);

    //---   setup the DMA tag
    DMAHELP_BuildTagCont( &pPacket->DMA, sizeof(t_NL2VPacket) - sizeof(sceDmaTag) );

    //---   setup the VIF Unpack info
    pPacket->VIF[0] = VIFHELP_SET_STCYCL( 1, 1, 0 );
    pPacket->VIF[1] = 0;
    pPacket->VIF[2] = SCE_VIF1_SET_FLUSH(0);
    u32 addr = 1013;//MATRIX_NL2V_ADDR;
    u32 size = (sizeof( pPacket->Col0 ) + sizeof( pPacket->Col1 ) + sizeof( pPacket->Col2 )) >> 4;
    VIFHELP_BuildTagUnpack( &pPacket->VIF[3], addr, size, VIFHELP_UNPACK_V4_32, FALSE, FALSE, TRUE );

    //---   fill out the matrix
    pPacket->Col0.X = L2V.M[0][0];
    pPacket->Col0.Y = L2V.M[0][1];
    pPacket->Col0.Z = L2V.M[0][2];
    pPacket->Col0.W = 0.0f;
    pPacket->Col1.X = L2V.M[1][0];
    pPacket->Col1.Y = L2V.M[1][1];
    pPacket->Col1.Z = L2V.M[1][2];
    pPacket->Col1.W = 0.0f;
    pPacket->Col2.X = L2V.M[2][0];
    pPacket->Col2.Y = L2V.M[2][1];
    pPacket->Col2.Z = L2V.M[2][2];
    pPacket->Col2.W = 0.0f;
}

//==========================================================================
static s32 debug = 0;
void QGeom::SendSubmeshFlags( xbool bClip, xbool bLight, xbool bEnvmap, xbool bFirst, xbool bShadow )
{
//    bClip = 1;
    sceDmaTag *pHeader;
    t_StripFlagDataPacket *pData;
    sceDmaTag *pFooter;

    pHeader = (sceDmaTag *)pDList;
    pDList += sizeof( sceDmaTag );
    pData = (t_StripFlagDataPacket *)pDList;
    pDList += sizeof( t_StripFlagDataPacket );
    pFooter = (sceDmaTag *)pDList;
    pDList += sizeof( sceDmaTag );

    DMAHELP_BuildTagRef( pHeader, 
                         (u32)&s_FlagHead,
                         sizeof( s_FlagHead ) );

    bEnvmap = !!bEnvmap;
    DMAHELP_BuildTagCont( &pData->DMA, sizeof( t_StripFlagDataPacket ) - sizeof( sceDmaTag ) );
    pData->VIF[ 0 ] = 0;
    pData->VIF[ 1 ] = 0;
    pData->VIF[ 2 ] = 0;
    VIFHELP_BuildTagUnpack( &pData->VIF[ 3 ], 1023, 1, VIFHELP_UNPACK_V4_32, FALSE, TRUE, TRUE ); // 1023 is VU addr, 1 is qword count
    //---   setup CMD data
    pData->Pad[ 0 ] = 0;
    pData->Pad[ 1 ] = 0;
    pData->TweakU = s_fTweakU;
    pData->TweakV = s_fTweakV;
    if (bEnvmap)
    {
        debug ++;
    }
    // tag on the "0x8" to use screenspace tweaking with environment mapping
//    pData->Flags = 0x8 | ((!!bLight & 0x1) | ((!!bEnvmap & 0x1) << 1) | ((!!bClip & 0x1) << 2));  // flags
    // turning off envmapping.  no time to finish/
//    pData->Flags = ((!!bLight & 0x1) | ((!!bEnvmap & 0x1) << 1) | ((!!bClip & 0x1) << 2)) | ((!!bShadow & 0x1) << 3);  // flags
    pData->Flags = ((!!bLight & 0x1) | ((!!bClip & 0x1) << 2)) | ((!!bShadow & 0x1) << 3);  // flags

    DMAHELP_BuildTagRef( pFooter, 
                        (u32)&s_FlagTail, 
                        sizeof( s_FlagTail ) );
}

//==========================================================================

void QGeom::SetEnvMapContext( x_bitmap* pBitmap )
{
    t_EnvContextPacket* pEC;
    s32                 L, T, R, B;
    u32                 fbp;
    s32                 psm;
    s32                 tbw;
    s32                 LogW, LogH;

    //---   grab up some engine information needed for setting context 2
    ENG_Get2DClipArea( L, T, R, B );
    if ( ENG_GetPageFlipCount() & 0x1)
        fbp = 0x00;
    else
        fbp = 0x8C;
    psm = VRAM_PS2_GetFormat( *pBitmap );
    LogW = pBitmap->GetWidth();
    LogH = pBitmap->GetHeight();
    tbw = LogW >> 6;// / 64;
    tbw = MAX( tbw, 1 );
    LogW = VRAM_PS2_GetLog2( LogW );
    LogH = VRAM_PS2_GetLog2( LogH );

    //---   grab some display list for what we're doing
    pEC = (t_EnvContextPacket*)pDList;
    pDList += sizeof(t_EnvContextPacket);

    //---   set up the DMA
    DMAHELP_BuildTagCont( &pEC->DMA, sizeof(t_EnvContextPacket)-sizeof(sceDmaTag) );

    //---   set up the VIF commands
    pEC->VIF[0] = 0;
    pEC->VIF[1] = 0;
    pEC->VIF[2] = SCE_VIF1_SET_FLUSH(0);
    pEC->VIF[3] = SCE_VIF1_SET_DIRECT( 12, 0 );

    //---   set up the GIF tag
    VIFHELP_BuildGifTag1( &pEC->GIF, VIFHELP_GIFMODE_PACKED, 1, 11, FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &pEC->GIF, VIFHELP_GIFREG_AD, 0, 0, 0 );

    //---   set up the registers
    s32 zFBP = PS2_GetZBufferAddress();
    s32 zPSM = PS2_GetZBufferPSM();
    pEC->ZBuff          = SCE_GS_SET_ZBUF( zFBP, zPSM, 0x1 );
    pEC->ZBuffAddr      = SCE_GS_ZBUF_2;
    pEC->Test           = SCE_GS_SET_TEST( 0x1, 0x5, 0x0, 0x0, 0x0, 0x0, 0x1, 0x2 );
    pEC->TestAddr       = SCE_GS_TEST_2;
    pEC->Alpha          = SCE_GS_SET_ALPHA( 0x0, 0x2, 0x2, 0x1, 58 );    // additive blending with alpha at 58
    //pEC->Alpha          = SCE_GS_SET_ALPHA( 0x0, 0x1, 0x2, 0x1, 0x80 );    // normal blending with alpha at 100%
    pEC->AlphaAddr      = SCE_GS_ALPHA_2;
    pEC->FBA            = SCE_GS_SET_FBA( 1 );
    pEC->FBAAddr        = SCE_GS_FBA_2;
    pEC->Frame          = SCE_GS_SET_FRAME( fbp, 640/64, SCE_GS_PSMCT32, 0x00 );
    pEC->FrameAddr      = SCE_GS_FRAME_2;
    pEC->Scissor        = SCE_GS_SET_SCISSOR( L, R, T, B );
    pEC->ScissorAddr    = SCE_GS_SCISSOR_2;
    pEC->XYOffset       = SCE_GS_SET_XYOFFSET((2048-(ENG_GetScreenWidth()>>1))<<4, (2048-(ENG_GetScreenHeight()>>1))<<4);
    pEC->XYOffsetAddr   = SCE_GS_XYOFFSET_2;
    pEC->TexFlush       = 0;
    pEC->TexFlushAddr   = SCE_GS_TEXFLUSH;
    pEC->Tex1           = SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 );
    pEC->Tex1Addr       = SCE_GS_TEX1_2;
    pEC->Tex0           = SCE_GS_SET_TEX0( VRAM_PS2_GetTexAddr(*pBitmap), tbw, psm, LogW, LogH, 0x1, 0x00, VRAM_PS2_GetClutAddr(*pBitmap), 0x00, 0, 0, 1 );
    pEC->Tex0Addr       = SCE_GS_TEX0_2;
    pEC->Clamp          = SCE_GS_SET_CLAMP( 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );   // pure tiling
    pEC->ClampAddr      = SCE_GS_CLAMP_2;
}

//==========================================================================

void QGeom::Render( xbool SkipVisCheck )
{
    u32         CurrentRenderFlags;
    u32         CurrentBlendMode;
    u8          CurrentFixedAlpha;

    t_TexData       Data;

    s32             MeshID;
    vector3         Location;
    matrix4         W2V;
    vector3         Eye;
    vector3         MipPlaneN;
    f32             MipPlaneD           = 0.0f;
    f32             Scale               = 1.0f ;
    xbool           LightingMatrixSent  = FALSE;
    xbool           NL2VMatrixSent      = FALSE;

    Data.pMesh = NULL;
    Data.CurrentMeshID = -1;
    Data.MipPlaneD = 0.0f;
    Data.pTexture = NULL;
    Data.pLastTexture = NULL;
    Data.pEnvMapTexture = NULL;


    //DAMMIT!!!  THIS FUNCTION IS UGLY!!!

    // This is used by the Bat and the Ball, mainly - a couple of other odds and ends that are mostly not seen.


    ASSERT( ENG_GetRenderMode() );

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );
    ENG_GetBlendMode( CurrentBlendMode, CurrentFixedAlpha );

    u32 NewRenderFlags = CurrentRenderFlags;
    u32 NewBlendMode = CurrentBlendMode;
    u8  NewFixedAlpha = CurrentFixedAlpha;

    //---   activate the microcode and load the matrices
    PreRender();

    //---   gather info for visibility checks
    Data.pView = ENG_GetActiveView();
    Location = m_L2W.GetTranslation();

    //---   find out the scale magnitude
    if ( SkipVisCheck == FALSE )
        Scale = CalcScaleMagnitude( m_L2W );


    //---   now we can render each of the meshes
    for ( MeshID = 0; MeshID < m_NMeshes; MeshID++ )
    {
        Data.pMesh = &m_pMeshes[ MeshID ];

        //---   perform trivial rejections first
        if ( !(Data.pMesh->Flags & MESH_FLAG_VISIBLE) )
            continue;

        if ( Data.pMesh->Flags & MESH_FLAG_SHADOW )
            continue; // You should use QGeom::RenderMeshAsShadow in this case!

        xbool bPerformLight = (Data.pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING);
        xbool bPerformEnvmap = (Data.pMesh->Flags & MESH_FLAG_ENVMAPPED);

bPerformEnvmap = FALSE;

        if ( SkipVisCheck == FALSE )
        {
            if ( !Data.pView->SphereInView( view::V_WORLD, Location,
                                            Data.pMesh->Radius * Scale ) )
                continue;
        }

        //---   Send the lighting matrix
        if ( Data.pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING )
        {
            if ( !LightingMatrixSent )
            {
                SendLightingMatrix();
                LightingMatrixSent = TRUE;
            }
        }

        //---   send the NL2V matrix
//        if ( Data.pMesh->Flags & MESH_FLAG_ENVMAPPED )
//        {
//            if ( !NL2VMatrixSent )
//            {
//                SendNL2VMatrix();
//                NL2VMatrixSent = TRUE;
//            }
//        }

        //---   Prepare for LOD computation
        if ( Data.pMesh->Flags & MESH_FLAG_USE_MIPS )
        {
            Data.pView->GetW2VMatrix( W2V );
            Data.pView->GetPosition( Eye );
            MipPlaneN.X = W2V.M[0][2];
            MipPlaneN.Y = W2V.M[1][2];
            MipPlaneN.Z = W2V.M[2][2];

            MipPlaneD   = (MipPlaneN.X * Eye.X) +
                          (MipPlaneN.Y * Eye.Y) +
                          (MipPlaneN.Z * Eye.Z);
        }

        //---   loop through the sub-meshes, rendering each one
        for ( Data.SubMeshID = Data.pMesh->FirstSubMesh;
              Data.SubMeshID < Data.pMesh->FirstSubMesh + Data.pMesh->NSubMeshes;
              ++Data.SubMeshID )
        {
            u32         PacketAddress;
            sceDmaTag*  pDMA            = NULL ;

            xbool       bPerformClip    = TRUE;

            //---   Save the current mesh id and setup the rendering for this mesh
            Data.CurrentMeshID = m_pSubMeshes[ Data.SubMeshID ].MeshID;

            s32 nMaterialStages = 1;
            if ((m_pMaterials) && (m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0))
                nMaterialStages = m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].nStages;

            s32 loop;
            for (loop=0; loop<nMaterialStages; ++loop)
            {
                //---   activate the texture for this submesh
                if ( m_pSubMeshes[ Data.SubMeshID ].TextureID >= 0 )
                    ActivateTexture( Data, loop );


                NewRenderFlags = CurrentRenderFlags;
                NewBlendMode = CurrentBlendMode;
                NewFixedAlpha = CurrentFixedAlpha;
                if (nMaterialStages > 1)
                {
                    // we need to worry about multipass
                    s_PreviousMaterialLoop = loop;
                    if (loop == 0)
                    {
                        // first pass
                        // A = 0, B = 1, C = 3, D = 1, Fix = 0x7F
                        NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                        NewRenderFlags |=  ENG_ALPHA_BLEND_ON;
                        NewBlendMode =     ENG_BLEND_MULTI_1;
                    }
                    else
                    {
                        // second pass
                        // A = 1, B = 0, C = 0, D = 0, Fix = 0
                        NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                        NewRenderFlags |=  ENG_ALPHA_TEST_ON;
                        NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                        NewRenderFlags |=  ENG_ALPHA_BLEND_ON;
                        NewBlendMode =     ENG_BLEND_MULTI_2;
                    }
                }
                else if (m_pSubMeshes[ Data.SubMeshID ].Flags & SUBMESH_FLAG_ALPHA_P)
                {
                    // setup for punchthru
                    // let's see...   Alpha Test On, Alpha Blend Off...
                    NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                    NewRenderFlags |=  ENG_ALPHA_TEST_ON;
                }
                else if (m_pSubMeshes[ Data.SubMeshID ].Flags & SUBMESH_FLAG_ALPHA_T)
                {
                    // setup for transparency
                    // Alpha Test On, Alpha Blend On...
                    NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                    NewRenderFlags |=  ENG_ALPHA_TEST_ON;
                    NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                    NewRenderFlags |=  ENG_ALPHA_BLEND_ON;
                    NewBlendMode =     ENG_BLEND_NORMAL;
                }
                if (bPerformEnvmap)
                {
                    NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                    NewRenderFlags |=  ENG_ALPHA_TEST_ON;
                    NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                    NewRenderFlags |=  ENG_ALPHA_BLEND_ON;
                    NewBlendMode =    (ENG_BLEND_ADDITIVE | ENG_BLEND_FIXED_ALPHA);
                    NewFixedAlpha =    58;
                }

                ENG_SetRenderFlags( NewRenderFlags );
                ENG_SetBlendMode( NewBlendMode, NewFixedAlpha );

                ENG_GetRenderMode();
                SendSubmeshFlags( bPerformClip, bPerformLight, bPerformEnvmap, FALSE );

                //---   figure out the packet address
                PacketAddress = (u32)&m_Packets[ m_pSubMeshes[ Data.SubMeshID ].PacketOffset ];
                ASSERT( (PacketAddress & 0x0f) == 0 );

                //---   Setup dma reference to submesh geometry
                pDMA = (sceDmaTag*)pDList;
                pDList += sizeof(sceDmaTag);
                DMAHELP_BuildTagCall( pDMA, (u32)PacketAddress, 0 );

                //---   Update geom stats
                if ( s_pStatNVerts )
                    *s_pStatNVerts = *s_pStatNVerts + m_pSubMeshes[ Data.SubMeshID ].NVertsTrans;
                if ( s_pStatNTris )
                    *s_pStatNTris = *s_pStatNTris + m_pSubMeshes[ Data.SubMeshID ].NTris;
                if ( s_pStatNBytes )
                    *s_pStatNBytes = *s_pStatNBytes + m_pSubMeshes[ Data.SubMeshID ].PacketSize;
            }
            /*dont worry about setting it back... this is bad
            if (   NewRenderFlags != CurrentRenderFlags
                || NewBlendMode != CurrentBlendMode
                || NewFixedAlpha != CurrentFixedAlpha
                || m_pSubMeshes[ Data.SubMeshID ].Flags) 
            {
                ENG_SetRenderFlags( CurrentRenderFlags );
                ENG_SetBlendMode( CurrentBlendMode, CurrentFixedAlpha );
            }
            */
        }
    }
}

//==========================================================================

void QGeom::RenderMeshAsShadow( s32 MeshID, matrix4 ShadowMatrix, xbool SkipVisCheck )
{
    t_TexData       Data;
    t_GeomMesh*     pMesh           = NULL ;
    view*           pView           = NULL ;
    vector3         Location;
//    matrix4         W2V;
//    vector3         Eye;
    f32             Scale           = 1.0f ;

    Data.pMesh = NULL;
    Data.CurrentMeshID = -1;
    Data.MipPlaneD = 0.0f;
    Data.pTexture = NULL;
    Data.pLastTexture = NULL;
    Data.pEnvMapTexture = NULL;

    ASSERT( ENG_GetRenderMode() );
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    //---   get pointer to mesh
    pMesh = &m_pMeshes[MeshID];
    Data.pMesh = &m_pMeshes[ MeshID ];

    Data.pView = ENG_GetActiveView();

    //---   perform trivial rejections first
    if ( !(pMesh->Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
        return;

    //---   gather info for visibility checks
    pView = ENG_GetActiveView();
    Location = m_L2W.GetTranslation();

    //---   check bounding sphere
    if ( SkipVisCheck == FALSE )
    {
        Scale = CalcScaleMagnitude( m_L2W );

        if ( !pView->SphereInView( view::V_WORLD, Location,
                                   pMesh->Radius * Scale ) )
            return;
    }

    //---   activate the microcode and load the matrices
    PreRender( &ShadowMatrix );

    //---   loop through the sub-meshes
    for ( Data.SubMeshID = pMesh->FirstSubMesh;
          Data.SubMeshID < pMesh->FirstSubMesh + pMesh->NSubMeshes;
          ++Data.SubMeshID )
    {
        u32         PacketAddress;
        sceDmaTag*  pDMA;

        if (m_pSubMeshes[ Data.SubMeshID ].MeshID != Data.CurrentMeshID)
        {
            //---   Save the current mesh id and setup the rendering for this mesh
            Data.CurrentMeshID = m_pSubMeshes[ Data.SubMeshID ].MeshID;

            //---   get pointer to mesh
            Data.pMesh = &m_pMeshes[ Data.CurrentMeshID ];

            //---   perform trivial rejections first
            if ( !(Data.pMesh->Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
                continue;
        }
        SendSubmeshFlags( TRUE, FALSE, FALSE, FALSE, TRUE );

        //---   figure out the packet address
        PacketAddress = (u32)&m_Packets[ m_pSubMeshes[ Data.SubMeshID ].PacketOffset ];
        ASSERT( (PacketAddress & 0x0f) == 0 );

        //---   Setup dma reference to submesh geometry
        pDMA = (sceDmaTag*)pDList;
        pDList += sizeof(sceDmaTag);
        DMAHELP_BuildTagCall( pDMA, (u32)PacketAddress, 0 );

        //---   Update geom stats
        if ( s_pStatNVerts )
            *s_pStatNVerts = *s_pStatNVerts + m_pSubMeshes[ Data.SubMeshID ].NVertsTrans;
        if ( s_pStatNTris )
            *s_pStatNTris = *s_pStatNTris + m_pSubMeshes[ Data.SubMeshID ].NTris;
        if ( s_pStatNBytes )
            *s_pStatNBytes = *s_pStatNBytes + m_pSubMeshes[ Data.SubMeshID ].PacketSize;
    }
}

//==========================================================================

xbool QGeom::SubMeshInView( SGeomSubMesh *pSubMesh )
{
    s32     i;
    view*   pView;
    matrix4 W2V;
    f32     FX,FY;
    f32     NZ,FZ;
    vector3 PlaneN[6];
    f32     PlaneD[6];

    // Get view info
    pView = ENG_GetActiveView();
    pView->GetFrustum( FX, FY );
    pView->GetW2VMatrix( W2V );
    pView->GetZLimits( NZ, FZ );
    pView->GetViewPlanes( view::V_WORLD, PlaneN[3], PlaneD[3],
                                         PlaneN[4], PlaneD[4],
                                         PlaneN[1], PlaneD[1],
                                         PlaneN[2], PlaneD[2],
                                         PlaneN[0], PlaneD[0],
                                         PlaneN[5], PlaneD[5]);

    vector3 Mid = pSubMesh->MidPoint;
    Mid = W2V * Mid;

    ////////////////////////////////////////////////////////////////////
    // Check if midpt is in view
    ////////////////////////////////////////////////////////////////////
//    vector4     V       __attribute__ ((aligned(16)));
//    vector4     MidPt   __attribute__ ((aligned(16)));

    ////////////////////////////////////////////////////////////////
    // Convert pt into camera space
    ////////////////////////////////////////////////////////////////
/*
    MidPt.Set( pSubMesh->MidPoint.X, pSubMesh->MidPoint.Y, pSubMesh->MidPoint.Z, 1.0f );

    // VU0 Asm version:
    asm __volatile__
    ("
        LQC2    vf12, 0x00(%1)          # load the mid pt.
        LQC2    vf08, 0x00(%2)          # load W2V col 0
        LQC2    vf09, 0x10(%2)          # load W2V col 1
        LQC2    vf10, 0x20(%2)          # load W2V col 2
        LQC2    vf11, 0x20(%2)          # load W2V col 3
        VMULAx.xyz   ACC, vf08, vf12x   # ACC += X * col 0
        VMADDAy.xyz  ACC, vf09, vf12y   # ACC += Y * col 1
        VMADDAz.xyz  ACC, vf10, vf12z   # ACC += Z * col 2
        VMADDw.xyz  vf13, vf11, vf12w   # vf13 = ACC + W * col 3
        SQC2    vf13, 0x00(%0)          # store result in V

    ": "+r" (&V) : "r" (&MidPt) , "r" (&W2V.M[0][0]) );
*/

//    if((  V.Z      >= NZ  ) &&
//       (  V.Z      <= FZ  ) &&
//       (  V.X * FX <= V.Z ) && 
//       ( -V.X * FX <= V.Z ) && 
//       (  V.Y * FY <= V.Z ) && 
//       ( -V.Y * FY <= V.Z ))
    if((  Mid.Z      >= NZ  ) &&
       (  Mid.Z      <= FZ  ) &&
       (  Mid.X * FX <= Mid.Z ) && 
       ( -Mid.X * FX <= Mid.Z ) && 
       (  Mid.Y * FY <= Mid.Z ) && 
       ( -Mid.Y * FY <= Mid.Z ))
    {
        // we can see the midpoint
        return VIEW_TEST_TRIVIAL_ACCEPT;
    }

    for( i=0; i<6; ++i )
    {
        vector3 BestPt;

        BestPt  = pSubMesh->Corner;

        s32 j;
        for (j=0; j<3; ++j)
        {
            if( Dot(pSubMesh->Axis[j],PlaneN[i]) > 0 )
                BestPt  += pSubMesh->Axis[j];
        }

        // Check if best point is outside
        if( (Dot(PlaneN[i],BestPt) + PlaneD[i]) < 0 )
            return VIEW_TEST_TRIVIAL_REJECT;
    }
    return VIEW_TEST_ON_FRUSTUM;
}

//==========================================================================

xbool QGeom::SubMeshOnView( SGeomSubMesh *pSubMesh )
{
    // in getting to this function, we should have already decided that this mesh
    //  is "in view," meaning we can see at least some part of it in the main
    //  view frustum.
    // This function takes things a step further and decides if this mesh touches
    //  the clipping frustum which is way out at the PS2 guardband.  This function
    //  grabs the Clip View Planes (should be the guardband) and 
    s32     i;
    vector3 PlaneN[6];
    f32     PlaneD[6];

    // Get view info
    ENG_GetActiveView()->GetClipViewPlanes( view::V_WORLD, PlaneN[3], PlaneD[3],
                                                           PlaneN[4], PlaneD[4],
                                                           PlaneN[1], PlaneD[1],
                                                           PlaneN[2], PlaneD[2],
                                                           PlaneN[0], PlaneD[0],
                                                           PlaneN[5], PlaneD[5]);

    for( i=0; i<6; ++i )
    {
        vector3 Corner = pSubMesh->Corner;

        // if the corner is outside of this plane, we clip
        if ((Dot(Corner, PlaneN[ i ]) + PlaneD[i]) < 0)
        {
            return TRUE;
        }
        
        s32 j;

        for (j=0; j<3; ++j)
        {
            if( Dot(pSubMesh->Axis[j],PlaneN[i]) < 0 )
                Corner  += pSubMesh->Axis[j];
        }
        // if this built corner is outside of the plane, we clip
        if ((Dot(Corner, PlaneN[ i ]) + PlaneD[i]) < 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//==========================================================================

void QGeom::ActivateTextureMIPs( t_TexData &Data )
{

    f32 MipK;
//    f32 MinLOD,MaxLOD;
    s32 MinMip, MaxMip;

    //---   compute the mipping k value
    MipK = -(s_Log2XC1+m_pSubMeshes[ Data.SubMeshID ].AvgPixelSize);

    //---   now that we have K, get the min and max mip levels that
    //      are possible with this sub-mesh
//    ComputeLOD( Data.CurrentMeshID,
//                MipK,
//                Data.MipPlaneN,
//                Data.MipPlaneD,
//                MinLOD,
//                MaxLOD );

    //---   figure out the min and max mip levels and activate the texture
//    MinMip = (s32)MinLOD;
//    MaxMip = (s32)(MaxLOD + 1.0f);
//    MinMip = MAX( MinMip, 0 );
//    MinMip = MIN( Data.pTexture->GetNMips(), MinMip );
//    MaxMip = MAX( MaxMip, 0 );
//    MaxMip = MIN( Data.pTexture->GetNMips(), MaxMip );
//    if (MinMip > 2)
//        MinMip = 2;
//    if (MaxMip > 2)
//        MaxMip = 2;
    PS2_SetMipK( MipK );
    MinMip = 0;
    MaxMip = MIN( Data.pTexture->GetNMips(), 3 );
    VRAM_Activate( *Data.pTexture, MinMip, MaxMip );
}

void QGeom::ActivateTextureNoMIPs( t_TexData &Data )
{
    //---   activate the texture
//    if ( Data.pMesh->Flags & MESH_FLAG_ENVMAPPED )
//    {
//        VRAM_Activate( *Data.pEnvMapTexture );
//        SetEnvMapContext( Data.pEnvMapTexture );
//    }
    if( m_MeshTextureArray[ Data.CurrentMeshID ] != NULL )
        VRAM_Activate( *m_MeshTextureArray[ Data.CurrentMeshID ] );
    else
        VRAM_Activate( *Data.pTexture );
}

void QGeom::ActivateTexture( t_TexData &Data, s32 iPass )
{
    ASSERTS( ((Data.pMesh->Flags & MESH_FLAG_ENVMAPPED) && !(m_Flags & GEOM_FLAG_OWNS_TEXTURES)) ||
             !(Data.pMesh->Flags & MESH_FLAG_ENVMAPPED), "You must supply the textures for an env. mapped object!" );

    //---   get a ptr to the texture
    // check for an override texture and set if there
    if( m_MeshTextureArray[ Data.CurrentMeshID ] != NULL )
        Data.pTexture = m_MeshTextureArray[ Data.CurrentMeshID ];
    else if ( m_pTextures )
    {
//        if ( Data.pMesh->Flags & MESH_FLAG_ENVMAPPED )
//            Data.pEnvMapTexture = &m_pTextures[ m_NTextures ];
        Data.pTexture = &m_pTextures[ m_pSubMeshes[ Data.SubMeshID ].TextureID ];

        if (m_pMaterials && m_pMatStages)
        {
            ASSERT( m_pMatStages[ m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].iStages[ iPass ] ].iTexture >= 0 );
            Data.pTexture = &m_pTextures[ m_pMatStages[ m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].iStages[ iPass ] ].iTexture ];
        }
    }
    else if ( m_pTexturePtrs )
    {
//        if ( Data.pMesh->Flags & MESH_FLAG_ENVMAPPED )
//            Data.pEnvMapTexture = m_pTexturePtrs[ m_NTextures ];
        Data.pTexture = m_pTexturePtrs[ m_pSubMeshes[ Data.SubMeshID ].TextureID ];

        if (m_pMaterials && m_pMatStages)
        {
            ASSERT( m_pMatStages[ m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].iStages[ iPass ] ].iTexture >= 0 );
            Data.pTexture = m_pTexturePtrs[ m_pMatStages[ m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].iStages[ iPass ] ].iTexture ];
//            if ( Data.pMesh->Flags & MESH_FLAG_ENVMAPPED )
//                Data.pTexture = m_pTexturePtrs[ m_pMatStages[ m_pMaterials[ m_pSubMeshes[ Data.SubMeshID ].TextureID ].iStages[ iPass ] ].iTexture-1 ];
        }
    }
    else
    {
        Data.pTexture = NULL;
        ASSERT( FALSE );
    }
    ASSERT( Data.pTexture );
    
    if (Data.pLastTexture != Data.pTexture)
    {
        Data.pLastTexture = Data.pTexture;

        if ( Data.pMesh->Flags & MESH_FLAG_USE_MIPS )
            ActivateTextureMIPs( Data );
        else
            ActivateTextureNoMIPs( Data );
    }
}

//==========================================================================

void QGeom::VerifyMaterials( void )
{
    s32 i;
    x_bitmap *pTextures;
    if (m_pTexturePtrs)
    {
        pTextures = m_pTexturePtrs[ 0 ];
    }
    else
    {
        pTextures = m_pTextures;
    }

    for (i=0; i<m_NMaterials; ++i)
    {
        s32 j;
        char temp[80];
        x_sprintf( temp, "Material[ %d ] has too many stages\n", i );
        ASSERTS( m_pMaterials[ i ].nStages <= 2, temp );
        x_sprintf( temp, "Material[ %d ] has no stages!\n", i );
        ASSERTS( m_pMaterials[ i ].nStages > 0, temp );
        for (j=0; j<m_pMaterials[ i ].nStages; ++j)
        {
            x_sprintf( temp, "Material[ %d ].iStages[ %d ].iTexture out of range\n", i, j );
            ASSERTS( m_pMatStages[ m_pMaterials[ i ].iStages[ j ] ].iTexture < m_NTextures, temp );
            x_sprintf( temp, "Material[ %d ].iStages[ %d ].iTexture less then ZERO\n", i, j );
            ASSERTS( m_pMatStages[ m_pMaterials[ i ].iStages[ j ] ].iTexture >= 0, temp );
            if (VRAM_IsRegistered( pTextures[ m_pMatStages[ m_pMaterials[ i ].iStages[ j ] ].iTexture ] ))
            {
                // unregister it
                // register it with the correct context (which is "j")
            }
        }
    }
}

//==========================================================================
t_GeomMesh *QGeom::GetMesh( s32 MeshID )
{
    if (MeshID < 0 || MeshID >= m_NMeshes)
        return NULL;

    return &m_pMeshes[ MeshID ];
}

//==========================================================================

SGeomSubMesh *QGeom::GetSubMesh( s32 SubMeshID )
{
    if (SubMeshID < 0 || SubMeshID >= m_NSubMeshes)
        return NULL;

    return &m_pSubMeshes[ SubMeshID ];
}

//==========================================================================

void    PS2_GetClipMetrics( s32 &nCulled, s32 &nClipped, s32 &nTotal )
{
    // find out how many actual triangles are going through each portion of the pipe
    nCulled = s_ClipStatisticsCullCount;
    nClipped = s_ClipStatisticsClipCount;
    nTotal = s_ClipStatisticsTotal;
}

//==========================================================================
//==========================================================================

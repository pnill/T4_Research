////////////////////////////////////////////////////////////////////////////
//
// GC_VRAM.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_memory.hpp"
#include "x_bitmap.hpp"

#include "Q_VRAM.hpp"
#include "Q_GC.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES AND CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define VRAM_MAX_TEXTURES   (750)
#define VRAM_MAX_STAGES     (GX_MAX_TEXMAP)

#undef VRAM_Register
#undef VRAM_DBG_Register
#undef VRAM_DBG_Dump
#undef VRAM_DBG_FreeSlots
#undef VRAM_DBG_UsedSlots


///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

struct SGCTexture
{
    x_bitmap*       m_pBitmap;          // pointer to texture in main memory

#ifdef X_DEBUG
    char            Filename[16];       // Name of file texture was registered from
    s32             LineNumber;         // Line number in file texture was registered from
#endif
};


struct SGCTevStage
{
    s32             m_VRAMID;           // current active texture at this stage, -1 if no texture
    s32             m_bIsDXT3;          // is a DXT3 texture loaded here
    GXTexWrapMode   m_GCWrapU;          // U wrap mode(S)
    GXTexWrapMode   m_GCWrapV;          // V wrap mode(T)
};


struct SGC_VRAM
{
    SGCTexture      m_Textures[VRAM_MAX_TEXTURES];  // List of active textures
    SGCTevStage     m_TevStage[VRAM_MAX_STAGES];    // info about tev stages
};


///////////////////////////////////////////////////////////////////////////
// STATIC-GLOBAL VARIABLES
///////////////////////////////////////////////////////////////////////////

static SGC_VRAM*    s_pVRAMGameCube = NULL;

static s32*         s_pNTextureUploads = NULL;
static s32*         s_pNBytesUploaded  = NULL;
static f32          s_LODBias          = 0.0f;


///////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////

static s32 FindEmptyTextureSlot( void )
{
    s32 i;

    ASSERT( s_pVRAMGameCube != NULL );

    //--- find first entry that isn't being used in texture slot array
    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        if( s_pVRAMGameCube->m_Textures[i].m_pBitmap == NULL )
            return i;
    }

    ASSERTS( FALSE, "No free VRAM slots" );

    return -1;
}

//==========================================================================

static s32 AddTexture( x_bitmap& BMP )
{
    ASSERT( (((u32)BMP.GetDataPtr()) & 0x1F) == 0 );    // make sure data is aligned to 32B
    ASSERT( BMP.GetWidth()  <= 1024 );                  // maximum width on GameCube is 1024
    ASSERT( BMP.GetHeight() <= 1024 );                  // maximum height on GameCube is 1024

    // make sure data is swizzled or compressed
    ASSERT( BMP.IsDataSwizzled() || BMP.IsCompressed() );

    ASSERT( s_pVRAMGameCube != NULL );

    //--- get an empty texture slot
    s32 VRAMIndex = FindEmptyTextureSlot();

    if( VRAMIndex == -1 )
        return -1;

    //--- set the initial values
    s_pVRAMGameCube->m_Textures[VRAMIndex].m_pBitmap = &BMP;

    //--- flush texture data from the CPU cache
    if( BMP.IsClutBased() )
        DCFlushRange( BMP.GetClutPtr(), BMP.GetClutSize() );

    DCFlushRange( BMP.GetRawDataPtr(), BMP.GetTotalDataSize() );

    return VRAMIndex;
}

//==========================================================================

static void DeleteTexture( x_bitmap& BMP )
{
    s32 VRAMID = BMP.GetVRAMID();

    ASSERT( VRAMID >= 0 );
    ASSERT( VRAMID <  VRAM_MAX_TEXTURES );
    ASSERT( s_pVRAMGameCube != NULL );

    if( (VRAMID >= 0) && (VRAMID < VRAM_MAX_TEXTURES) )
    {
        //--- make sure this BMP matches with the VRAM slot
        ASSERT( &BMP == s_pVRAMGameCube->m_Textures[VRAMID].m_pBitmap );

        //--- clear the texture values
        s_pVRAMGameCube->m_Textures[VRAMID].m_pBitmap = NULL;
    }
}

//==========================================================================

static void InitTexture( x_bitmap&  BMP,
                         s32        TevStage,
                         GXTexObj&  rTexObj,
                         GXTlutObj& rTlutObj,
                         xbool&     bUseTlut )
{
    GXTexFilter MipFilterMin;
    GXAnisotropy AnisoValue;

    //--- create VRAM object
    if( BMP.IsClutBased() )
    {
        ASSERT( BMP.GetBPC() == 16 );
        ASSERT( (((u32)BMP.GetClutPtr()) & 0x1F) == 0 );

        GXCITexFmt TexFormat  = GX_TF_C8;
        GXTlutFmt  TLUTFormat = GX_TL_RGB565;

        //--- get GameCube texture format
        switch( BMP.GetBPP() )
        {
            case 4:  TexFormat = GX_TF_C4;  break;
            case 8:  TexFormat = GX_TF_C8;  break;
            default: ASSERT( FALSE );       break;
        }

        //--- get GameCube TLUT format
        /*switch( BMP.GetFormat() )
        {
            case x_bitmap::FMT_P4_RGB_565:
            case x_bitmap::FMT_P8_RGB_565:    TLUTFormat = GX_TL_RGB565;  break;

            case x_bitmap::FMT_P4_ARGB_3444:
            case x_bitmap::FMT_P4_URGB_1555:
            case x_bitmap::FMT_P8_ARGB_3444:
            case x_bitmap::FMT_P8_URGB_1555:  TLUTFormat = GX_TL_RGB5A3;  break;

            default:  ASSERT( FALSE );  break;
        }*/

        switch( BMP.GetFormat() )
        {
            case x_bitmap::FMT_P4_RGB_565:
            case x_bitmap::FMT_P8_RGB_565:

                TLUTFormat = GX_TL_RGB565;
                break;

            default:
                TLUTFormat = GX_TL_RGB5A3;
                break;
        }

        bUseTlut = TRUE;

        //--- init the texture and TLUT objects
        GXInitTexObjCI( &rTexObj,
                        (void*)BMP.GetDataPtr(),
                        (u16)BMP.GetWidth(),
                        (u16)BMP.GetHeight(),
                        TexFormat,
                        s_pVRAMGameCube->m_TevStage[ TevStage ].m_GCWrapU,
                        s_pVRAMGameCube->m_TevStage[ TevStage ].m_GCWrapV,
                        ((BMP.GetNMips() > 1) ? GX_TRUE : GX_FALSE),
                        TLUT_ID[ TevStage ] );

        GXInitTlutObj( &rTlutObj,
                        BMP.GetClutPtr(),
                        TLUTFormat,
                        BMP.GetNClutColors() );

        // Color-index textures MUST use these settings
        MipFilterMin = GX_NEAR_MIP_NEAR;
        AnisoValue   = GX_ANISO_1;
    }
    else
    {
        GXTexFmt TexFormat = GX_TF_RGBA8;

        //--- get GameCube texture format
        /*switch( BMP.GetFormat() )
        {
            case x_bitmap::FMT_16_RGB_565:    TexFormat = GX_TF_RGB565;  break;

            case x_bitmap::FMT_16_ARGB_3444:
            case x_bitmap::FMT_16_URGB_1555:  TexFormat = GX_TF_RGB5A3;  break;

            case x_bitmap::FMT_32_ARGB_8888:
            case x_bitmap::FMT_32_URGB_8888:  TexFormat = GX_TF_RGBA8;   break;

            case x_bitmap::FMT_DXT1:          TexFormat = GX_TF_CMPR;    break;

            default:  ASSERT( FALSE );  break;
        }*/

        switch( BMP.GetBPP() )
        {
            case 4:
                if( BMP.GetFormat() == x_bitmap::FMT_I4 )
                    TexFormat = GX_TF_I4;
                else
                    TexFormat = GX_TF_CMPR;
                break;

            case 8:
                if( BMP.GetFormat() == x_bitmap::FMT_I8 )
                    TexFormat = GX_TF_I8;
                else
                    TexFormat = GX_TF_CMPR;
                break;

            case 16:
                if( BMP.GetFormat() == x_bitmap::FMT_16_RGB_565 )
                {
                    TexFormat = GX_TF_RGB565;
                }
                //else if( BMP.GetFormat() == x_bitmap::FMT_V8U8 )
                //{
                //    TexFormat = GX_TF_IA8;
                //}
                else
                {
                    TexFormat = GX_TF_RGB5A3;
                }
                break;

            case 32:
                TexFormat = GX_TF_RGBA8;
                break;

            default:
                ASSERT( FALSE );
                TexFormat = GX_TF_RGBA8;
                break;
        }

        bUseTlut = FALSE;

        void* pTexelData = (void*)BMP.GetDataPtr();

        //--- get alpha map from DXT3 texture if needed
        if( (BMP.GetFormat() == x_bitmap::FMT_DXT3) && (TevStage >= 4) )
        {
            if( BMP.GetNMips() > 1 )
                pTexelData = BMP.GetDataPtr( BMP.GetNMips()+1 );
            else
                pTexelData = (void*)( (byte*)pTexelData + ((BMP.GetWidth() * BMP.GetHeight()) / 2) );

            TexFormat = GX_TF_I4;
        }

        //--- init the texture object
        GXInitTexObj( &rTexObj,
                      pTexelData,
                      (u16)BMP.GetWidth(),
                      (u16)BMP.GetHeight(),
                      TexFormat,
                      s_pVRAMGameCube->m_TevStage[ TevStage ].m_GCWrapU,
                      s_pVRAMGameCube->m_TevStage[ TevStage ].m_GCWrapV,
                      ((BMP.GetNMips() > 1) ? GX_TRUE : GX_FALSE) );

        MipFilterMin = GX_LIN_MIP_LIN;
        AnisoValue   = GX_ANISO_1;  // 1, 2, or 4
    }

    //--- setup mip LOD info for texture object
    if( BMP.GetNMips() > 1 )
    {
        GXBool doEdgeLOD, doBiasClamp;

        if( AnisoValue == GX_ANISO_1 )
        {
            doBiasClamp = ((s_LODBias == 0.0f) ? GX_DISABLE : GX_ENABLE);
            doEdgeLOD   = GX_FALSE;
        }
        else
        {
            doBiasClamp = GX_ENABLE;
            doEdgeLOD   = GX_TRUE;
        }

        GXInitTexObjLOD( &rTexObj,
                         MipFilterMin,          //min_filter
                         GX_LINEAR,             //mag_filter
                         0.0f,                  //min_lod
                         (BMP.GetNMips() - 1),  //max_lod
                         s_LODBias,             //lod bias
                         doBiasClamp,           //bias clamp enable
                         doEdgeLOD,             //do edge lod calc
                         AnisoValue );          //max anisotrophic filter cycle(1,2,4)
    }
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// VRAM STUBS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void VRAM_Init( void )
{
    s32 i;

    s_pVRAMGameCube = new SGC_VRAM;

    ASSERTS( s_pVRAMGameCube != NULL, "Couldn't allocate VRAM data" );

    //--- clear out the texture slots
    x_memset( s_pVRAMGameCube, 0, sizeof(SGC_VRAM) );

    //--- make default tev stage settings
    for( i = 0; i < VRAM_MAX_STAGES; i++ )
    {
        s_pVRAMGameCube->m_TevStage[i].m_VRAMID    = -1;
        s_pVRAMGameCube->m_TevStage[i].m_GCWrapU   = GX_REPEAT;
        s_pVRAMGameCube->m_TevStage[i].m_GCWrapV   = GX_REPEAT;
        s_pVRAMGameCube->m_TevStage[i].m_bIsDXT3   = FALSE;
    }
}

//==========================================================================

void VRAM_Kill( void )
{
    ASSERT( s_pVRAMGameCube != NULL );

    s32 i;

    //--- deactivate all the textures in VRAM first
    VRAM_DeactivateAll();

    //--- unregister any textures
    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        if( s_pVRAMGameCube->m_Textures[i].m_pBitmap != NULL )
        {
            VRAM_UnRegister( *(s_pVRAMGameCube->m_Textures[i].m_pBitmap) );
        }
    }

    //--- finally free memory used by VRAM
    delete s_pVRAMGameCube;
    s_pVRAMGameCube = NULL;

    VRAM_SetStatTrackers( NULL, NULL );
}

//==========================================================================

void VRAM_Register( x_bitmap& BMP, s32 iContext )
{
    (void)iContext;
    ASSERT( s_pVRAMGameCube != NULL );

    s32 VRAMID = BMP.GetVRAMID();

    if( VRAMID != -1 )
    {
        if( (VRAMID < 0) || (VRAMID >= VRAM_MAX_TEXTURES) )
        {
            ASSERTS( FALSE, "VRAM_Register with possible bad BMP data" );
        }
        else if( &BMP == s_pVRAMGameCube->m_Textures[VRAMID].m_pBitmap )
        {
            ASSERTS( FALSE, "VRAM_Register with already registered BMP" );
        }
        else
        {
            ASSERTS( FALSE, "VRAM_Register with possible bad BMP data" );
        }
    }
    else
    {
        VRAMID = AddTexture( BMP );

        BMP.SetVRAMID( VRAMID );
    }
}

//==========================================================================

void VRAM_UnRegister( x_bitmap& BMP )
{
    if( BMP.GetVRAMID() != -1 )
    {
        DeleteTexture( BMP );

        BMP.SetVRAMID( -1 );
    }
}

//==========================================================================

void VRAM_Activate( x_bitmap& BMP )
{
    VRAM_GC_Activate( BMP, 0 );
}

//==========================================================================

void VRAM_Activate( x_bitmap& BMP, s32 /*MinMip*/, s32 /*MaxMip*/ )
{
    VRAM_GC_Activate( BMP, 0 );
}

//==========================================================================

void VRAM_Deactivate( x_bitmap& BMP )
{
    s32       i;
    s32       VRAMID;
    x_bitmap* pLoadedBMP;

    for( i = 0; i < VRAM_MAX_STAGES; i++ )
    {
        VRAMID = s_pVRAMGameCube->m_TevStage[i].m_VRAMID;

        if( VRAMID != -1 )
        {
            pLoadedBMP = s_pVRAMGameCube->m_Textures[ VRAMID ].m_pBitmap;

            if( pLoadedBMP == &BMP )
                VRAM_GC_Deactivate( i );
        }
    }
}

//==========================================================================

void VRAM_DeactivateAll( void )
{
    s32 i;

    for( i = 0; i < VRAM_MAX_STAGES; i++ )
    {
        VRAM_GC_Deactivate( i );
    }
}

//==========================================================================

xbool VRAM_IsActive( x_bitmap& BMP )
{
    ASSERT( s_pVRAMGameCube != NULL );

    s32 i;
    s32 VRAMID = BMP.GetVRAMID();

    //--- check if the bitmap has been set active to a tev stage
    for( i = 0; i < VRAM_MAX_STAGES; i++ )
    {
        if( VRAMID == s_pVRAMGameCube->m_TevStage[i].m_VRAMID )
            return TRUE;
    }

    return FALSE;
}

//==========================================================================

void VRAM_SetWrapMode( s32 UMode, s32 VMode, f32 /*UMin*/, f32 /*UMax*/, f32 /*VMin*/, f32 /*VMax*/ )
{
    VRAM_GC_SetWrapMode( UMode, VMode, 0 );
}

//==========================================================================

void VRAM_SetStatTrackers( s32* pNTextureUploads, s32* pNBytesUploaded )
{
    s_pNTextureUploads = pNTextureUploads;
    s_pNBytesUploaded  = pNBytesUploaded;
}

//==========================================================================

void VRAM_GC_Activate( x_bitmap& BMP, s32 TevStage )
{
    xbool     bUseTlut;
    GXTexObj  TexObj;
    GXTlutObj TlutObj;

    s32 VRAMID = BMP.GetVRAMID();

    ASSERT( VRAMID >= 0 );
    ASSERT( VRAMID <  VRAM_MAX_TEXTURES );
    ASSERT( s_pVRAMGameCube != NULL );
    ASSERT( (TevStage >= 0) && (TevStage < VRAM_MAX_STAGES) );

    if( (VRAMID >= 0) && (VRAMID < VRAM_MAX_TEXTURES) )
    {
        ASSERT( &BMP == s_pVRAMGameCube->m_Textures[ VRAMID ].m_pBitmap );

        s32 AlphaStageID = (VRAM_MAX_STAGES-1) - TevStage;

        //--- check if activating a dxt3 texture, but only allow if tevstage is 0-3
        if( (BMP.GetFormat() == x_bitmap::FMT_DXT3) && (AlphaStageID >= 4) )
        {
            //--- set the VRAM ID for the tev stage and alpha map stage
            s_pVRAMGameCube->m_TevStage[ TevStage ].m_VRAMID  = VRAMID;
            s_pVRAMGameCube->m_TevStage[ TevStage ].m_bIsDXT3 = TRUE;

            s_pVRAMGameCube->m_TevStage[ AlphaStageID ].m_VRAMID  = VRAMID;
            s_pVRAMGameCube->m_TevStage[ AlphaStageID ].m_bIsDXT3 = TRUE;

            //--- setup color and alpha textures, and load them
            InitTexture( BMP, TevStage, TexObj, TlutObj, bUseTlut );
            GXLoadTexObj( &TexObj, TEXMAP_ID[ TevStage ] );

            InitTexture( BMP, AlphaStageID, TexObj, TlutObj, bUseTlut );
            GXLoadTexObj( &TexObj, TEXMAP_ID[ AlphaStageID ] );
        }
        else
        {
            //--- since loading a non-dxt3, mark it's opposite stage as non-dxt3
            s_pVRAMGameCube->m_TevStage[ AlphaStageID ].m_bIsDXT3 = FALSE;

            //--- set the VRAM ID for the tev stage
            s_pVRAMGameCube->m_TevStage[ TevStage ].m_VRAMID  = VRAMID;
            s_pVRAMGameCube->m_TevStage[ TevStage ].m_bIsDXT3 = FALSE;

            //--- setup texture and TLUT objects
            InitTexture( BMP, TevStage, TexObj, TlutObj, bUseTlut );

            //--- if texture has a TLUT, set the TLUT ID and load it
            if( bUseTlut )
                GXLoadTlut( &TlutObj, TLUT_ID[ TevStage ] );

            //--- load texture to the appropriate TevStage
            GXLoadTexObj( &TexObj, TEXMAP_ID[ TevStage ] );
        }
    }
}

//==========================================================================

void VRAM_GC_Deactivate( s32 TevStage )
{
    ASSERT( s_pVRAMGameCube != NULL );
    ASSERT( (TevStage >= 0) && (TevStage < VRAM_MAX_STAGES) );

    if( s_pVRAMGameCube->m_TevStage[ TevStage ].m_VRAMID != -1 )
    {
        s32 AlphaStage = (VRAM_MAX_STAGES-1) - TevStage;

        // mark opposite dxt3 stage as false
        s_pVRAMGameCube->m_TevStage[ AlphaStage ].m_bIsDXT3 = FALSE;

        // if deactivating the stage with dxt3 color, deactivate the alpha as well
        if( AlphaStage >= 4 )
            s_pVRAMGameCube->m_TevStage[ AlphaStage ].m_VRAMID  = -1;

        s_pVRAMGameCube->m_TevStage[ TevStage ].m_VRAMID  = -1;
        s_pVRAMGameCube->m_TevStage[ TevStage ].m_bIsDXT3 = FALSE;
    }
}

//==========================================================================

void VRAM_GC_SetWrapMode( s32 UMode, s32 VMode, s32 TevStage )
{
    ASSERT( s_pVRAMGameCube != NULL );
    ASSERT( (TevStage >= 0) && (TevStage < VRAM_MAX_STAGES) );

    SGCTevStage* pStage = &s_pVRAMGameCube->m_TevStage[ TevStage ];

    switch( UMode )
    {
        case WRAP_MODE_CLAMP:   pStage->m_GCWrapU = GX_CLAMP;   break;
        case WRAP_MODE_TILE:    pStage->m_GCWrapU = GX_REPEAT;  break;
        case WRAP_MODE_MIRROR:  pStage->m_GCWrapU = GX_MIRROR;  break;
        default:
            break;
    }

    switch( VMode )
    {
        case WRAP_MODE_CLAMP:   pStage->m_GCWrapV = GX_CLAMP;   break;
        case WRAP_MODE_TILE:    pStage->m_GCWrapV = GX_REPEAT;  break;
        case WRAP_MODE_MIRROR:  pStage->m_GCWrapV = GX_MIRROR;  break;
        default:
            break;
    }
}

//==========================================================================

void GC_SetMipK( f32 MipK )
{
    s_LODBias = MipK;

    if( s_LODBias < -3.95f )
        s_LODBias = -3.95f;

    if( s_LODBias > 3.95f )
        s_LODBias = 3.95f;
}

//==========================================================================

void GC_GetMipK( f32& MipK )
{
    MipK = s_LODBias;
}

//==========================================================================

s32 GC_GetDXT3AlphaMapStage( s32 TexSlotID )
{
    ASSERT( s_pVRAMGameCube != NULL );
    ASSERT( (TexSlotID >= 0) && (TexSlotID < VRAM_MAX_STAGES) );

    if( TexSlotID >= 4 )
        return -1;

    if( FALSE == s_pVRAMGameCube->m_TevStage[ TexSlotID ].m_bIsDXT3 )
        return -1;

    return (VRAM_MAX_STAGES-1) - TexSlotID;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// VRAM DEBUG FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void VRAM_DBG_Register( x_bitmap& BMP, s32 iContext, char* File, s32 Line )
{
    // Perform normal bitmap register operation
    VRAM_Register( BMP, iContext );

#ifdef X_DEBUG

    if( BMP.GetVRAMID() < 0 )
        return;

    // Get last 16 chars of filename
    s32 Len = x_strlen( File ) + 1;
    if( Len > 16 )
        File += (Len - 16);

    // Store filename and line number where this bitmap is registered
    x_strncpy( s_pVRAMGameCube->m_Textures[ BMP.GetVRAMID() ].Filename, File, 16 );
    s_pVRAMGameCube->m_Textures[ BMP.GetVRAMID() ].Filename[15] = '\0';
    s_pVRAMGameCube->m_Textures[ BMP.GetVRAMID() ].LineNumber   = Line;

#endif //X_DEBUG
}

//==========================================================================

void VRAM_DBG_Dump( const char* Filename )
{
#ifdef X_DEBUG

    s32 NSlotsFree;
    s32 NSlotsUsed;
    s32 i;

    SGCTexture* pTex;

    #define START_PRINT     x_printf(
    #define END_PRINT       );

    NSlotsFree = VRAM_DBG_FreeSlots();
    NSlotsUsed = VRAM_DBG_UsedSlots();

    START_PRINT  "-----------------------------------------\n"  END_PRINT
    START_PRINT  "  ID  |  In Use  | LineNum | File\n"  END_PRINT
    START_PRINT  "-----------------------------------------\n"  END_PRINT

    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        pTex = &s_pVRAMGameCube->m_Textures[i];

        if( pTex->m_pBitmap != NULL )
            START_PRINT  " %3d  |   USED   |   %4d   | %s\n", i, pTex->LineNumber, pTex->Filename  END_PRINT
        else
            START_PRINT  " %3d  |   FREE   |   %4d   | %s\n", i, pTex->LineNumber, pTex->Filename  END_PRINT
    }

    START_PRINT  "\n\n" END_PRINT
    START_PRINT  "-----------------------------------------\n"  END_PRINT
    START_PRINT  "VRAM Slots Free: %d\n", NSlotsFree  END_PRINT
    START_PRINT  "VRAM Slots Used: %d\n", NSlotsUsed  END_PRINT
    START_PRINT  "-----------------------------------------\n"  END_PRINT

#endif //X_DEBUG
}

//==========================================================================

s32 VRAM_DBG_FreeSlots( void )
{
    s32 NSlotsFree = 0;
    s32 i;

    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        if( s_pVRAMGameCube->m_Textures[i].m_pBitmap == NULL )
            NSlotsFree++;
    }

    return NSlotsFree;
}

//==========================================================================

s32 VRAM_DBG_UsedSlots( void )
{
    s32 NSlotsUsed = 0;
    s32 i;

    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        if( s_pVRAMGameCube->m_Textures[i].m_pBitmap != NULL )
            NSlotsUsed++;
    }

    return NSlotsUsed;
}

//==========================================================================

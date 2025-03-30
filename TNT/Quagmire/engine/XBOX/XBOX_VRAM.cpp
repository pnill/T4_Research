////////////////////////////////////////////////////////////////////////////
//
// XBOX_VRAM.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_bitmap.hpp"
#include "x_memory.hpp"

#include "Q_XBOX.hpp"
#include "Q_VRAM.hpp"

#include "xtl.h"
#include "xgraphics.h"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define VRAM_MAX_TEXTURES   (608)
#define VRAM_MAX_STAGES     (4)

#undef VRAM_Register
#undef VRAM_DBG_Register
#undef VRAM_DBG_Dump
#undef VRAM_DBG_FreeSlots
#undef VRAM_DBG_UsedSlots

#if defined( X_DEBUG )
	#define DO_VRAM_DUMP_ON_REGISTER_FAIL
#endif

////////////////////////////////////////////////////////////////////////////
// STRUCTS
////////////////////////////////////////////////////////////////////////////

struct SXBOXTexture
{
    LPDIRECT3DTEXTURE8  m_pTexture;     // D3D texture
    LPDIRECT3DPALETTE8  m_pPalette;     // D3D palette data
    x_bitmap*           m_pXBMP;        // x_bitmap registered

#ifdef X_DEBUG
    static const u8 RecordedFilenameSize = 48;

    char Filename[RecordedFilenameSize];// Name of file texture was registered from
    s32  LineNumber;                    // Line number in file texture was registered from
#endif
};

struct SXBOX_VRAM
{
    SXBOXTexture        m_RegisteredBMPs[VRAM_MAX_TEXTURES];
    s32                 m_ActiveBMPIDs[VRAM_MAX_STAGES];
};


////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////
static SXBOX_VRAM* s_pVRAMXBox = NULL;

////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

static D3DFORMAT GetD3DTextureFormat( x_bitmap& BMP )
{
    u32       XBMPFormat;
    D3DFORMAT D3DFormat;

    XBMPFormat = BMP.GetFormat();

    switch( XBMPFormat )
    {
        case x_bitmap::FMT_P8_ARGB_8888:    D3DFormat = D3DFMT_P8;          break;
        case x_bitmap::FMT_16_ARGB_4444:    D3DFormat = D3DFMT_A4R4G4B4;    break;
        case x_bitmap::FMT_16_RGB_565:      D3DFormat = D3DFMT_R5G6B5;      break;
        case x_bitmap::FMT_32_ARGB_8888:    D3DFormat = D3DFMT_A8R8G8B8;    break;
        case x_bitmap::FMT_32_URGB_8888:    D3DFormat = D3DFMT_X8R8G8B8;    break;

        case x_bitmap::FMT_DXT1:    D3DFormat = D3DFMT_DXT1;    break;
        case x_bitmap::FMT_DXT2:    D3DFormat = D3DFMT_DXT2;    break;
        case x_bitmap::FMT_DXT3:    D3DFormat = D3DFMT_DXT3;    break;
        case x_bitmap::FMT_DXT4:    D3DFormat = D3DFMT_DXT4;    break;
        case x_bitmap::FMT_DXT5:    D3DFormat = D3DFMT_DXT5;    break;
        case x_bitmap::FMT_YUY2:    D3DFormat = D3DFMT_YUY2;    break;

        default:
            ASSERTS( FALSE, "Invalid x_bitmap format" );
            D3DFormat = D3DFMT_UNKNOWN;
            break;
    }

    return D3DFormat;
}

//==========================================================================

static s32 FindEmptyTextureSlot( void )
{
    s32 i;

    ASSERT( s_pVRAMXBox != NULL );

    // Find first entry that isn't being used in texture slot array
    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        if( s_pVRAMXBox->m_RegisteredBMPs[i].m_pXBMP == NULL )
            return i;
    }

#if defined( DO_VRAM_DUMP_ON_REGISTER_FAIL ) && defined( X_DEBUG )
	VRAM_DBG_Dump( "VRAM_Register_FAIL.txt" );
#endif

    ASSERTS( FALSE, "No free VRAM slots" );

    return -1;
}

//==========================================================================

static s32 AddTexture( x_bitmap& BMP )
{
    s32 VRAMIndex;
    s32 MipMapCount;
    s32 MipLevel;
    s32 PixWidth;
    s32 PixHeight;
    s32 CopySize;

    D3DFORMAT           D3DFormat;
    D3DLOCKED_RECT      D3DLockedRect;
    D3DCOLOR*           pD3DPalette;

    ASSERT( BMP.IsDataSwizzled() || BMP.IsCompressed() );
    ASSERT( s_pVRAMXBox != NULL );

    // Get a free space for the texture
    VRAMIndex = FindEmptyTextureSlot();

    if( VRAMIndex == -1 )
        return -1;

    SXBOXTexture* pTexture = &(s_pVRAMXBox->m_RegisteredBMPs[VRAMIndex]);

    MipMapCount = BMP.GetNMips() + 1;
    PixWidth    = BMP.GetWidth();
    PixHeight   = BMP.GetHeight();
    D3DFormat   = GetD3DTextureFormat( BMP );

    if( BMP.IsDataSystem() )
    {
        ASSERT( (u32)BMP.GetDataPtr() % 128 == 0 );

        pTexture->m_pTexture = new IDirect3DTexture8;
        ASSERT( pTexture->m_pTexture );

        // tell the xbox all about it
        XGSetTextureHeader( BMP.GetPWidth(), BMP.GetPHeight(), MipMapCount, // w, h, levels
                            0,                  // usage (unused)
                            D3DFormat,          // Format
                            0,                  // pool (unused)
                            pTexture->m_pTexture,
                            0,                  // Offset to the data held by this resource
                            0);                 // Texture pitch

        // register it
        pTexture->m_pTexture->Register(BMP.GetDataPtr());

        // Assign the VRAM texture bitmap(for error checking)
        pTexture->m_pXBMP = &BMP;
    }
    else
    {
        // Create the D3D texture
        DXWARN( XBOX_GetDevice()->CreateTexture( PixWidth,
                                                 PixHeight,
                                                 MipMapCount,
                                                 0,
                                                 D3DFormat,
                                                 0,
                                                 &pTexture->m_pTexture ) );

        ASSERT( pTexture->m_pTexture != NULL );

        // Assign the VRAM texture bitmap(for error checking)
        pTexture->m_pXBMP = &BMP;

        // Copy the texture data over to the D3D texture memory
        for( MipLevel = 0; MipLevel < MipMapCount; MipLevel++ )
        {
            DXWARN( pTexture->m_pTexture->LockRect( MipLevel, &D3DLockedRect, NULL, 0 ) );
            ASSERT( D3DLockedRect.pBits != NULL );

            if( BMP.IsCompressed() )
            {
                CopySize = BMP.GetWidth( MipLevel ) * BMP.GetHeight( MipLevel );

                // if DXT1, then BPP is 4, so half the other compressed formats
                if( BMP.GetFormat() == x_bitmap::FMT_DXT1 )
                    CopySize = CopySize / 2;
            }
            else
            {
                CopySize = (BMP.GetWidth( MipLevel ) * BMP.GetHeight( MipLevel ) * BMP.GetBPP()) / 8;
            }

            // Copy the texture
            x_memcpy( D3DLockedRect.pBits, BMP.GetDataPtr( MipLevel ), CopySize );

            DXWARN( pTexture->m_pTexture->UnlockRect( MipLevel ) );
        }

        if( BMP.IsClutBased() )
        {
            // Create a D3D palette
            DXWARN( XBOX_GetDevice()->CreatePalette( D3DPALETTE_256, &pTexture->m_pPalette ) );
            ASSERT( pTexture->m_pPalette != NULL );

            // Lock the palette data for copying
            DXWARN( pTexture->m_pPalette->Lock( &pD3DPalette, NULL ) );
            ASSERT( pD3DPalette != NULL );

            CopySize = BMP.GetNClutColors() * (BMP.GetBPC()/8);

            // Copy palette data
            x_memcpy( pD3DPalette, BMP.GetClutPtr(), CopySize );

            DXWARN( pTexture->m_pPalette->Unlock() );
        }
    }

    return VRAMIndex;
}

//==========================================================================

void DeleteTexture( x_bitmap& BMP )
{
    s32 VRAMID = BMP.GetVRAMID();

    ASSERT( VRAMID >= 0 );
    ASSERT( VRAMID <  VRAM_MAX_TEXTURES );
    ASSERT( s_pVRAMXBox != NULL );

    if( (VRAMID >= 0) && (VRAMID < VRAM_MAX_TEXTURES) )
    {
        SXBOXTexture* pTexture = &(s_pVRAMXBox->m_RegisteredBMPs[VRAMID]);

        // make sure this BMP matches with the VRAM slot
        ASSERT( &BMP == pTexture->m_pXBMP );

        VRAM_Deactivate( BMP );

        // clear the texture values
        pTexture->m_pXBMP = NULL;

        if( BMP.IsDataSystem() )
        {
            if( pTexture->m_pTexture != NULL )
            {
                delete pTexture->m_pTexture;
                pTexture->m_pTexture = NULL;
            }
        }
        else
        {
            if( pTexture->m_pTexture != NULL )
            {
                pTexture->m_pTexture->Release();
                pTexture->m_pTexture = NULL;
            }

            if( pTexture->m_pPalette != NULL )
            {
                pTexture->m_pPalette->Release();
                pTexture->m_pPalette = NULL;
            }
        }
        
    }
}

//==========================================================================

void VRAM_Init( void )
{
    s32 i;

    // Allocate space for vram data
    s_pVRAMXBox = (SXBOX_VRAM*)x_malloc( sizeof(SXBOX_VRAM) );

    ASSERTS( s_pVRAMXBox != NULL, "Couldn't allocate VRAM data" );

    x_memset( s_pVRAMXBox, 0, sizeof(SXBOX_VRAM) );

    // Set initial active IDs to -1
    for( i = 0; i < VRAM_MAX_STAGES; i++ )
    {
        s_pVRAMXBox->m_ActiveBMPIDs[i] = -1;
    }
}

//==========================================================================

void VRAM_Kill( void )
{
    // Unregister all registered entries
    ASSERT( s_pVRAMXBox != NULL );

    s32 i;

    VRAM_DeactivateAll();

    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        if( s_pVRAMXBox->m_RegisteredBMPs[i].m_pXBMP != NULL )
        {
            VRAM_UnRegister( *(s_pVRAMXBox->m_RegisteredBMPs[i].m_pXBMP) );
        }
    }

    // Release the memory
    if( s_pVRAMXBox )
        x_free( s_pVRAMXBox );

    s_pVRAMXBox = NULL;
}

//==========================================================================

void VRAM_Register( x_bitmap& BMP, s32 iContext )
{
    (void)iContext;
    ASSERT( s_pVRAMXBox != NULL );

    s32 VRAMID = BMP.GetVRAMID();

    if( VRAMID != -1 )
    {
        if( (VRAMID < 0) || (VRAMID >= VRAM_MAX_TEXTURES) )
        {
            ASSERTS( FALSE, "VRAM_Register with possible bad BMP data" );
        }
        else if( &BMP == s_pVRAMXBox->m_RegisteredBMPs[VRAMID].m_pXBMP )
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

xbool VRAM_IsRegistered( x_bitmap& BMP )
{
    return (BMP.GetVRAMID() != -1);
}

//==========================================================================

void VRAM_Activate( x_bitmap& BMP )
{
    VRAM_XBOX_Activate( BMP, 0 ); 
}

//==========================================================================

void VRAM_Activate( x_bitmap& BMP, s32 MinMip, s32 MaxMip )
{
    VRAM_XBOX_Activate( BMP, 0 );
}


//==========================================================================
void VRAM_UseBilinear( xbool bActivate )
{
    // Not in use on XBOX
    (void)bActivate;
}

//==========================================================================
void VRAM_UseTrilinear( xbool bActivate )
{
    // Not in use on XBOX
    (void)bActivate;
}

//==========================================================================
void VRAM_Deactivate( x_bitmap& BMP )
{
    s32 i;
    s32 VRAMID = BMP.GetVRAMID();

    for( i = 0; i < VRAM_MAX_STAGES; i++ )
    {
        if( VRAMID == s_pVRAMXBox->m_ActiveBMPIDs[i] )
            VRAM_XBOX_Deactivate( i );
    }
}

//==========================================================================

void VRAM_DeactivateAll( void )
{
    s32 i;

    for( i = 0; i < VRAM_MAX_STAGES; i++ )
    {
        VRAM_XBOX_Deactivate( i );
    }
}

//==========================================================================

xbool VRAM_IsActive( x_bitmap& BMP )
{
    ASSERT( s_pVRAMXBox != NULL );

    s32 i;
    s32 VRAMID = BMP.GetVRAMID();

    // check if the bitmap has been set active to a tev stage
    for( i = 0; i < VRAM_MAX_STAGES; i++ )
    {
        if( VRAMID == s_pVRAMXBox->m_ActiveBMPIDs[i] )
            return TRUE;
    }

    return FALSE;
}

//==========================================================================

void VRAM_SetWrapMode( s32 UMode, s32 VMode, f32 UMin, f32 UMax, f32 VMin, f32 VMax )
{
    VRAM_XBOX_SetWrapMode( UMode, VMode, 0 );
}

//==========================================================================

void VRAM_XBOX_SetWrapMode( s32 UMode, s32 VMode, s32 TexStage )
{
    ASSERT( s_pVRAMXBox != NULL );
    ASSERT( (TexStage >= 0) && (TexStage < VRAM_MAX_STAGES) );

    ASSERT( (UMode >= 0) && (UMode < 5) );
    ASSERT( (VMode >= 0) && (VMode < 5) );

    s32 DXWrapModes[5] = {
                            D3DTADDRESS_CLAMP,  // WRAP_MODE_CLAMP    0
                            D3DTADDRESS_WRAP,   // WRAP_MODE_TILE     1
                            D3DTADDRESS_MIRROR, // WRAP_MODE_MIRROR   2
                            D3DTADDRESS_BORDER, // WRAP_MODE_REGION   3
                            D3DTADDRESS_WRAP,   // WRAP_MODE_R_REPEAT 4
                         };

    DXWARN( XBOX_GetDevice()->SetTextureStageState( TexStage, D3DTSS_ADDRESSU, DXWrapModes[UMode] ) );
    DXWARN( XBOX_GetDevice()->SetTextureStageState( TexStage, D3DTSS_ADDRESSV, DXWrapModes[VMode] ) );
}

//==========================================================================

void VRAM_XBOX_Activate( x_bitmap& BMP, s32 TexStage )
{
    s32 VRAMID = BMP.GetVRAMID();

    ASSERT( VRAMID >= 0 );
    ASSERT( VRAMID <  VRAM_MAX_TEXTURES );
    ASSERT( s_pVRAMXBox != NULL );
    ASSERT( (TexStage >= 0) && (TexStage < VRAM_MAX_STAGES) );

    if( (VRAMID >= 0) && (VRAMID < VRAM_MAX_TEXTURES) )
    {
        SXBOXTexture* pTexture = &(s_pVRAMXBox->m_RegisteredBMPs[VRAMID]);

        ASSERT( &BMP == pTexture->m_pXBMP );
        ASSERT( pTexture->m_pTexture != NULL );

        // set the VRAM ID for the tev stage
        s_pVRAMXBox->m_ActiveBMPIDs[ TexStage ] = VRAMID;

        // Activate the texture
        DXWARN( XBOX_GetDevice()->SetTexture( TexStage, pTexture->m_pTexture ) );

        // Activate palette(if used)
        if( BMP.IsClutBased() )
        {
            ASSERT( pTexture->m_pPalette != NULL );

            DXWARN( XBOX_GetDevice()->SetPalette( TexStage, pTexture->m_pPalette ) );
        }
    }
    else
    {
        s_pVRAMXBox->m_ActiveBMPIDs[ TexStage ] = -1;

        DXWARN( XBOX_GetDevice()->SetTexture( TexStage, NULL ) );
    }
}

//==========================================================================

void VRAM_XBOX_Deactivate( s32 TexStage )
{
    ASSERT( s_pVRAMXBox != NULL );
    ASSERT( (TexStage >= 0) && (TexStage < VRAM_MAX_STAGES) );

    if( s_pVRAMXBox->m_ActiveBMPIDs[ TexStage ] != -1 )
    {
        s_pVRAMXBox->m_ActiveBMPIDs[ TexStage ] = -1;

        DXWARN( XBOX_GetDevice()->SetTexture( TexStage, NULL ) );
    }
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
    if( Len > SXBOXTexture::RecordedFilenameSize )
        File += (Len - SXBOXTexture::RecordedFilenameSize );

    // Store filename and line number where this bitmap is registered
    x_strncpy( s_pVRAMXBox->m_RegisteredBMPs[ BMP.GetVRAMID() ].Filename, File, SXBOXTexture::RecordedFilenameSize );
    s_pVRAMXBox->m_RegisteredBMPs[ BMP.GetVRAMID() ].Filename[SXBOXTexture::RecordedFilenameSize - 1] = '\0';
    s_pVRAMXBox->m_RegisteredBMPs[ BMP.GetVRAMID() ].LineNumber = Line;

#endif //X_DEBUG
}

//==========================================================================

void VRAM_DBG_Dump( const char* Filename )
{
#ifdef X_DEBUG

    s32 NSlotsFree;
    s32 NSlotsUsed;
    s32 i;
    char UsedFileName[255];

    X_FILE* fp = NULL;

    SXBOXTexture* pTex;

    x_sprintf( UsedFileName, "T:\\%s", Filename );

    #define START_PRINT     x_fprintf(fp,
    #define END_PRINT       );

    fp = x_fopen( UsedFileName, "wt" );
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

    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        pTex = &s_pVRAMXBox->m_RegisteredBMPs[i];

        if( pTex->m_pXBMP != NULL )
            START_PRINT  " %3d  |   USED   |   %4d   | %s\n", i, pTex->LineNumber, pTex->Filename  END_PRINT
        else
            START_PRINT  " %3d  |   FREE   |   %4d   | %s\n", i, pTex->LineNumber, pTex->Filename  END_PRINT
    }

    START_PRINT  "\n\n" END_PRINT
    START_PRINT  "-----------------------------------------\n"  END_PRINT
    START_PRINT  "VRAM Slots Free: %d\n", NSlotsFree  END_PRINT
    START_PRINT  "VRAM Slots Used: %d\n", NSlotsUsed  END_PRINT
    START_PRINT  "-----------------------------------------\n"  END_PRINT

	if( fp != NULL )
		x_fclose( fp );

#endif //X_DEBUG
}

//==========================================================================

s32 VRAM_DBG_FreeSlots( void )
{
    s32 NSlotsFree = 0;
    s32 i;

    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
    {
        if( s_pVRAMXBox->m_RegisteredBMPs[i].m_pXBMP == NULL )
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
        if( s_pVRAMXBox->m_RegisteredBMPs[i].m_pXBMP != NULL )
            NSlotsUsed++;
    }

    return NSlotsUsed;
}

//==========================================================================

void *VRAM_XBOX_GetTexture( x_bitmap& BMP )
{
    ASSERT( s_pVRAMXBox != NULL );

    s32 VRAMID = BMP.GetVRAMID();

    return (void*)s_pVRAMXBox->m_RegisteredBMPs[VRAMID].m_pTexture;
}



//==========================================================================


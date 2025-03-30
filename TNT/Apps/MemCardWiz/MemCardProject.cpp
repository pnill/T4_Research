////////////////////////////////////////////////////////////////////////////
//
// MemCardProject.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MemCardProject.h"

#include "AUX_Bitmap.hpp"


////////////////////////////////////////////////////////////////////////////
// PS2 EXPORT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

void MC_PS2Header::Init( void )
{
    m_MCardType      = MEMCARD_TYPE_PS2;
    m_NSaveTypes     = 0;
    m_SaveTypeOffset = 0;
    m_IconInfoOffset = 0;
    m_IconDataOffset = 0;
    m_TotalFileSize  = 0;
}

//==========================================================================

void MC_PS2FileType::Init( void )
{
	m_MaxSaves = 0;
	m_FileSize = 0;

	memset( m_Filename, 0, sizeof(m_Filename) );
	memset( m_BaseDirs, 0, sizeof(m_BaseDirs) );
}

//==========================================================================

void MC_PS2IconInfo::Init( void )
{
	m_AMB_B    = 32;
	m_AMB_G    = 32;
	m_AMB_R    = 32;
	m_BK_Alpha = 128;
	m_BK_BL_B  = 215;
	m_BK_BL_G  = 215;
	m_BK_BL_R  = 215;
	m_BK_BR_B  = 215;
	m_BK_BR_G  = 215;
	m_BK_BR_R  = 215;
	m_BK_TL_B  = 215;
	m_BK_TL_G  = 215;
	m_BK_TL_R  = 215;
	m_BK_TR_B  = 215;
	m_BK_TR_G  = 215;
	m_BK_TR_R  = 215;
	m_L1_B     = 255;
	m_L1_G     = 255;
	m_L1_R     = 255;
	m_L1_X     = 1.0f;
	m_L1_Y     = 0.0f;
	m_L1_Z     = 0.0f;
	m_L2_B     = 255;
	m_L2_G     = 255;
	m_L2_R     = 255;
	m_L2_X     = 0.0f;
	m_L2_Y     = 1.0f;
	m_L2_Z     = 0.0f;
	m_L3_B     = 255;
	m_L3_G     = 255;
	m_L3_R     = 255;
	m_L3_X     = 0.0f;
	m_L3_Y     = 0.0f;
	m_L3_Z     = 1.0f;

	m_IconDataSize = 0;
}

////////////////////////////////////////////////////////////////////////////
// GAMECUBE EXPORT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

void MC_GCHeader::Init( void )
{
    m_MCardType      = MEMCARD_TYPE_GAMECUBE;
    m_NSaveTypes     = 0;
    m_SaveTypeOffset = 0;
    m_IconInfoOffset = 0;
    m_IconDataOffset = 0;
    m_TotalFileSize  = 0;
}

//==========================================================================

void MC_GCFileType::Init( void )
{
	m_MaxSaves = 0;
	m_FileSize = 0;

	memset( m_Filename, 0, sizeof(m_Filename) );
}

//==========================================================================

void MC_GCIconInfo::Init( void )
{
	m_NFrames      = GC_ICON_NFRAMES_MIN;
	m_AnimSpeed    = GC_ICON_ANIMSPEED_NORMAL;
	m_IconDataSize = 0;
};


////////////////////////////////////////////////////////////////////////////
// XBOX EXPORT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

void MC_XBOXHeader::Init( void )
{
    m_MCardType      = MEMCARD_TYPE_XBOX;
    m_NSaveTypes     = 0;
    m_SaveTypeOffset = 0;
    m_IconDataSize   = 0;
    m_IconDataOffset = 0;
    m_TotalFileSize  = 0;
}

//==========================================================================

void MC_XBOXFileType::Init( void )
{
	m_MaxSaves = 0;
	m_FileSize = 0;

	memset( m_Filename, 0, sizeof(m_Filename) );
}

////////////////////////////////////////////////////////////////////////////
// PC EXPORT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

void MC_PCHeader::Init( void )
{
    m_MCardType      = MEMCARD_TYPE_PC;
    m_NSaveTypes     = 0;
    m_SaveTypeOffset = 0;
    m_IconDataSize   = 0;
    m_IconDataOffset = 0;
    m_TotalFileSize  = 0;
}

//==========================================================================

void MC_PCFileType::Init( void )
{
	m_MaxSaves = 0;
	m_FileSize = 0;

	memset( m_Filename, 0, sizeof(m_Filename) );
}


////////////////////////////////////////////////////////////////////////////
// MEMCARDWIZ PROJECT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////


void MCWPrj_PS2SaveType::Init( void )
{
	m_FileSize = 0;
	m_MaxFiles = 1;

	memset( m_Filename, 0, sizeof(m_Filename) );
	memset( m_BaseDirs, 0, sizeof(m_BaseDirs) );
}

//==========================================================================

void MCWPrj_PS2Data::Init( void )
{
	int i;

	memset( m_GameID, 0, sizeof(m_GameID) );
	memset( m_IconFile, 0, sizeof(m_IconFile) );

	m_IconInfo.Init();

	for( i = 0; i < MCW_MAX_SAVETYPES; i++ )
		m_SaveTypes[i].Init();
}

//==========================================================================

void MCWPrj_GCSaveType::Init( void )
{
	m_FileSize = 0;
	m_MaxFiles = 1;

	memset( m_Filename, 0, sizeof(m_Filename) );
}

//==========================================================================

void MCWPrj_GCData::Init( void )
{
	int i;

	memset( m_IconFile, 0, sizeof(m_IconFile) );

	m_IconInfo.Init();

	for( i = 0; i < MCW_MAX_SAVETYPES; i++ )
		m_SaveTypes[i].Init();
}

//==========================================================================

void MCWPrj_XBOXSaveType::Init( void )
{
	m_FileSize = 0;
	m_MaxFiles = 1;

	memset( m_Filename, 0, sizeof(m_Filename) );
}

//==========================================================================

void MCWPrj_XBOXData::Init( void )
{
	int i;

	memset( m_IconFile, 0, sizeof(m_IconFile) );

	for( i = 0; i < MCW_MAX_SAVETYPES; i++ )
		m_SaveTypes[i].Init();
}

//==========================================================================

void MCWPrj_PCSaveType::Init( void )
{
	m_FileSize = 0;
	m_MaxFiles = 1;

	memset( m_Filename, 0, sizeof(m_Filename) );
}

//==========================================================================

void MCWPrj_PCData::Init( void )
{
	int i;

	memset( m_IconFile, 0, sizeof(m_IconFile) );

	for( i = 0; i < MCW_MAX_SAVETYPES; i++ )
		m_SaveTypes[i].Init();
}

//==========================================================================

void MCW_Project::Init( void )
{
	memset( m_HdrStr, 0, sizeof(m_HdrStr) );

	strcpy( m_HdrStr, MCW_CUR_VERSION );

	memset( m_SaveTypeNames, 0, sizeof(m_SaveTypeNames) );

	strcpy( m_SaveTypeNames[0], "DefaultSave" );

	m_NSaveTypes = 1;

	m_PS2.Init();
	m_GC.Init();
	m_XBOX.Init();
	m_PC.Init();
}

//==========================================================================

void MCW_Project::Export( const char* pFilename, MEMCARD_TYPE ExpTarget )
{
	FILE* pFile;
    BOOL bIsSuccess = FALSE;

	pFile = fopen( pFilename, "wb" );

	if( pFile != NULL )
    {
	    switch( ExpTarget )
	    {
		    case MEMCARD_TYPE_PS2:
			    bIsSuccess = ExportPS2( pFile );
			    break;

		    case MEMCARD_TYPE_GAMECUBE:
			    bIsSuccess = ExportGC( pFile );
			    break;

		    case MEMCARD_TYPE_XBOX:
			    bIsSuccess = ExportXBOX( pFile );
			    break;

		    case MEMCARD_TYPE_PC:
			    bIsSuccess = ExportPC( pFile );
			    break;

		    default:
			    break;
	    }
    }
	fclose( pFile );

    if (bIsSuccess == FALSE)
    {
        CString buffer;
        buffer.Format("Error during export: %s!\n\nTip: Make sure your source icon files are in their proper place.",pFilename);
        AfxMessageBox(buffer, MB_OK);
    }

}

////////////////////////////////////////////////////////////////////////////
// PS2 MEMCARD FILE EXPORT
////////////////////////////////////////////////////////////////////////////

BOOL MCW_Project::ExportPS2( FILE* pFile )
{
	int   i;
	int   j;
	FILE* pIconFile;
	char* pRawIconData;
	long  IconDataSize;

	MC_PS2Header	Hdr;
	MC_PS2FileType	FType;

    ASSERT( pFile );
    if (pFile == NULL)
        return(FALSE);


	pIconFile = fopen( m_PS2.m_IconFile, "rb" );
	ASSERT( pIconFile != NULL );
    if (pIconFile == NULL)
        return(FALSE);

	fseek( pIconFile, 0, SEEK_END );
	IconDataSize = ftell( pIconFile );
	fseek( pIconFile, 0, SEEK_SET );

	ASSERT( IconDataSize > 0 );

	pRawIconData = new char[IconDataSize];
	ASSERT( pRawIconData != NULL );

	fread( pRawIconData, IconDataSize, 1, pIconFile );
	fclose( pIconFile );

	Hdr.Init();
	Hdr.m_NSaveTypes     = m_NSaveTypes;
	Hdr.m_SaveTypeOffset = sizeof(MC_PS2Header);
	Hdr.m_IconInfoOffset = Hdr.m_SaveTypeOffset + (sizeof(MC_PS2FileType) * m_NSaveTypes);
	Hdr.m_IconDataOffset = Hdr.m_IconInfoOffset + sizeof(MC_PS2IconInfo);
	Hdr.m_TotalFileSize  = Hdr.m_IconDataOffset + IconDataSize;


	fwrite( &Hdr, sizeof(MC_PS2Header), 1, pFile );

	for( i = 0; i < m_NSaveTypes; i++ )
	{
		FType.Init();

		FType.m_MaxSaves = m_PS2.m_SaveTypes[i].m_MaxFiles;
		FType.m_FileSize = m_PS2.m_SaveTypes[i].m_FileSize;

		strcpy( FType.m_Filename, m_PS2.m_SaveTypes[i].m_Filename );

		for( j = 0; j < FType.m_MaxSaves; j++ )
		{
			sprintf( FType.m_BaseDirs[j], "/%s-%s", m_PS2.m_GameID, m_PS2.m_SaveTypes[i].m_BaseDirs[j] );
		}

		fwrite( &FType, sizeof(MC_PS2FileType), 1, pFile );
	}

    m_PS2.m_IconInfo.m_IconDataSize = IconDataSize;
	fwrite( &m_PS2.m_IconInfo, sizeof(MC_PS2IconInfo), 1, pFile );

	fwrite( pRawIconData, IconDataSize, 1, pFile );

	delete[] pRawIconData;

    return(TRUE);
}

////////////////////////////////////////////////////////////////////////////
// GAMECUBE MEMCARD FILE EXPORT
////////////////////////////////////////////////////////////////////////////

#define GC_ICON_FRAME_WIDTH			32
#define GC_ICON_FRAME_HEIGHT		32
#define GC_ICON_CLUT_SIZE			(256 * sizeof(u16))
#define GC_ICON_FRAME_SIZE			(GC_ICON_FRAME_WIDTH * GC_ICON_FRAME_HEIGHT)
#define GC_ICON_DATA_SIZE(NFrames)	(GC_ICON_FRAME_SIZE * (NFrames) + GC_ICON_CLUT_SIZE)

static char GC_IconData[ GC_ICON_DATA_SIZE(GC_ICON_NFRAMES_MAX) ] = { 0 };

//--------------------------------------------------------------------------

static BOOL GC_CreateIcon( char* pFilename, s32& rNFrames, s32& rDataSize )
{
	s32   y;
	s32   W;
	s32   H;
	s32   CurFrame;
	byte* pSrc;
	byte* pDst;
	byte  TempData[ GC_ICON_DATA_SIZE(GC_ICON_NFRAMES_MAX) + GC_ICON_CLUT_SIZE ] = { 0 };

	x_bitmap IconBMP;
	x_bitmap TempBMP;

	// Clear final icon data
	x_memset( GC_IconData, 0, sizeof(GC_IconData) );

	// Load icon file
	if( !AUXBMP_LoadBitmap( IconBMP, pFilename ) )
	{
		rNFrames  = 1;
		rDataSize = GC_ICON_DATA_SIZE( 1 );
		return(FALSE);
	}

	// If icon has problems, make NFrames 1 and return
	if( (IconBMP.IsClutBased() == FALSE) ||
		(IconBMP.GetWidth()  < GC_ICON_FRAME_WIDTH) ||
		(IconBMP.GetHeight() < GC_ICON_FRAME_HEIGHT) )
	{
		rNFrames  = 1;
		rDataSize = GC_ICON_DATA_SIZE( 1 );
		return(FALSE);
	}

	// Make sure bitmap format is 8-bit
	AUXBMP_Convert( IconBMP, x_bitmap::FMT_P8_RGBA_8888, IconBMP );

	// Setup a temp bitmap that will hold the maximum # of icon frames
	TempBMP.SetupBitmap( IconBMP.GetFormat(),
						 GC_ICON_FRAME_WIDTH,
						 GC_ICON_FRAME_HEIGHT * GC_ICON_NFRAMES_MAX,
						 GC_ICON_FRAME_WIDTH,
						 GC_ICON_FRAME_HEIGHT * GC_ICON_NFRAMES_MAX,
						 FALSE,
						 GC_ICON_NFRAMES_MAX * GC_ICON_FRAME_SIZE,
						 TempData,
						 FALSE,
						 256 * 4,
						 TempData + (GC_ICON_NFRAMES_MAX * GC_ICON_FRAME_SIZE),
						 256,
						 0 );

	W = IconBMP.GetWidth();
	H = IconBMP.GetHeight();

	if( (H / GC_ICON_FRAME_HEIGHT) < (W / GC_ICON_FRAME_WIDTH) )
	{
		// Bitmap frames are laid out horizontally, convert to one
		// that has frames arranged vertically
		rNFrames = MIN( rNFrames, (W / GC_ICON_FRAME_WIDTH) );
		rNFrames = MIN( rNFrames, GC_ICON_NFRAMES_MAX );

		// For each frame, copy one frame's line at a time to the temp bitmap
		for( CurFrame = 0; CurFrame < rNFrames; CurFrame++ )
		{
			for( y = 0; y < GC_ICON_FRAME_HEIGHT; y++ )
			{
				pSrc = IconBMP.GetDataPtr() + (y * W) + (CurFrame * GC_ICON_FRAME_WIDTH);
				pDst = TempBMP.GetDataPtr() + (CurFrame * GC_ICON_FRAME_SIZE) + (y * GC_ICON_FRAME_WIDTH);

				x_memcpy( pDst, pSrc, GC_ICON_FRAME_WIDTH );
			}
		}
	}
	else
	{
		// Bitmap is laid out vertically
		rNFrames = MIN( rNFrames, (H / GC_ICON_FRAME_HEIGHT) );
		rNFrames = MIN( rNFrames, GC_ICON_NFRAMES_MAX );

		// copy icon frame lines
		for( y = 0; y < GC_ICON_FRAME_HEIGHT * rNFrames; y++ )
		{
			pSrc = IconBMP.GetDataPtr() + (y * W);
			pDst = TempBMP.GetDataPtr() + (y * GC_ICON_FRAME_WIDTH);

			x_memcpy( pDst, pSrc, GC_ICON_FRAME_WIDTH );
		}
	}

    // convert to 8x4 tiles for Gamecube
    // 4 tiles wide by 8 tiles high for Save Icon
    byte* pConvertedIcon    = new byte[TempBMP.GetTotalDataSize()];
    byte* pOriginalIcon     = TempBMP.GetDataPtr();

    static const u32 GC_ICON_WIDTH          = 32;
    static const u32 GC_TILE_WIDTH          = 8;
    static const u32 GC_TILE_HEIGHT         = 4;
    static const u32 GC_TILE_SIZE           = GC_TILE_WIDTH * GC_TILE_HEIGHT;
    static const u32 GC_TILES_ACROSS_ICON   = GC_ICON_WIDTH/GC_TILE_WIDTH;
    s32 originalTileOffset  = 0;
    s32 convertedTileOffset = 0;
    s32 tileIndex;
    s32 size = TempBMP.GetDataSize();
    s32 totalTiles = size/(GC_TILE_WIDTH*GC_TILE_HEIGHT);

    for (tileIndex=0; tileIndex<totalTiles; tileIndex++)
    {
        s32 tileRow,tileColumn;

        originalTileOffset = ((tileIndex/GC_TILES_ACROSS_ICON)*GC_TILE_SIZE*GC_TILES_ACROSS_ICON) + (tileIndex%GC_TILES_ACROSS_ICON)*GC_TILE_WIDTH;
        convertedTileOffset= tileIndex*GC_TILE_SIZE;

        for (tileRow=0; tileRow<GC_TILE_HEIGHT; tileRow++)
        {
            for (tileColumn=0; tileColumn<GC_TILE_WIDTH; tileColumn++)
            {
                pConvertedIcon[convertedTileOffset+tileRow*GC_TILE_WIDTH+tileColumn]
                = pOriginalIcon[originalTileOffset+tileRow*GC_ICON_WIDTH+tileColumn];
            }
        }
    }

    // convert CLUT to RGBA 5551/4443
    byte* pConvertedPalette = new byte[GC_ICON_CLUT_SIZE];
    s32 palIndex;
    for (palIndex=0; palIndex<256; palIndex++)
    {
        color C;
        u16 r,g,b,a;
        u8* pPal = IconBMP.GetClutPtr();;
        a = *(pPal+palIndex*4+0);
        b = *(pPal+palIndex*4+1);
        g = *(pPal+palIndex*4+2);
        r = *(pPal+palIndex*4+3);
		if( a < 224 )	// pixel is opaque
		{	
			// pack in 5551 format, msb is set to 1
			pConvertedPalette[palIndex*2]   = ( (0x0080)          | ((r & 0xF8) >> 1) | (g >> 6) );	// byte0 is 1 bit alpha, upper 5-bits 
																					// of red, upper 2-bits of green
			pConvertedPalette[palIndex*2+1] = ( ((g & 0x38) << 2) | (b >> 3) );						// byte1 is bits 3-5 of green, upper 5 of blue		
		}
		else			// pixel is translucent
		{
			// pack in 4443 format,  shift alpha by 1 and set msb to 0					
			pConvertedPalette[palIndex*2]   = ( ( (a >> 1) & 0x70 ) | ((r & 0xF0)   >> 4) );		// byte0 is 1 bit 0, 3 alpha, 4-bits red 
			                                                                 
			pConvertedPalette[palIndex*2+1] = ( (g & 0xF0)          | ((b   & 0xF0)   >> 4) );	// 4-bits green, 4-bits blue	
		}
    }

	// save the data size
	rDataSize = GC_ICON_DATA_SIZE( rNFrames );

	// copy the icon data into the final buffer
	x_memcpy( GC_IconData, pConvertedIcon, rNFrames * GC_ICON_FRAME_SIZE );
	x_memcpy( GC_IconData + rNFrames * GC_ICON_FRAME_SIZE, pConvertedPalette, GC_ICON_CLUT_SIZE );

    delete[] pConvertedIcon;
    delete[] pConvertedPalette;

    return(TRUE);
}

//==========================================================================

BOOL MCW_Project::ExportGC( FILE* pFile )
{
	int i;
	int IconDataSize;
	int IconNFrames;
	MC_GCHeader		Hdr;
	MC_GCFileType	FType;
	MC_GCIconInfo   IconInfo;

    ASSERT( pFile );
    if (pFile == NULL)
        return(FALSE);


	IconNFrames = m_GC.m_IconInfo.m_NFrames;
	if (!GC_CreateIcon( m_GC.m_IconFile, IconNFrames, IconDataSize ))
        return(FALSE);

	Hdr.Init();
	Hdr.m_NSaveTypes     = m_NSaveTypes;
	Hdr.m_SaveTypeOffset = sizeof(MC_GCHeader);
	Hdr.m_IconInfoOffset = Hdr.m_SaveTypeOffset + (sizeof(MC_GCFileType) * m_NSaveTypes);
	Hdr.m_IconDataOffset = Hdr.m_IconInfoOffset + sizeof(MC_GCIconInfo);
	Hdr.m_TotalFileSize  = Hdr.m_IconDataOffset + IconDataSize;

	Hdr.m_MCardType      = ENDIAN_SWAP_32( Hdr.m_MCardType );
	Hdr.m_NSaveTypes     = ENDIAN_SWAP_32( Hdr.m_NSaveTypes );
	Hdr.m_SaveTypeOffset = ENDIAN_SWAP_32( Hdr.m_SaveTypeOffset );
	Hdr.m_IconInfoOffset = ENDIAN_SWAP_32( Hdr.m_IconInfoOffset );
	Hdr.m_IconDataOffset = ENDIAN_SWAP_32( Hdr.m_IconDataOffset );
	Hdr.m_TotalFileSize  = ENDIAN_SWAP_32( Hdr.m_TotalFileSize );

	fwrite( &Hdr, sizeof(MC_GCHeader), 1, pFile );

	for( i = 0; i < m_NSaveTypes; i++ )
	{
		FType.Init();

		FType.m_MaxSaves = ENDIAN_SWAP_32( m_GC.m_SaveTypes[i].m_MaxFiles );
		FType.m_FileSize = ENDIAN_SWAP_32( m_GC.m_SaveTypes[i].m_FileSize );

		strcpy( FType.m_Filename, m_GC.m_SaveTypes[i].m_Filename );

		fwrite( &FType, sizeof(MC_GCFileType), 1, pFile );
	}

	IconInfo.m_AnimSpeed    = ENDIAN_SWAP_32( m_GC.m_IconInfo.m_AnimSpeed );
	IconInfo.m_NFrames      = ENDIAN_SWAP_32( m_GC.m_IconInfo.m_NFrames );
	IconInfo.m_IconDataSize = ENDIAN_SWAP_32( IconDataSize );

	fwrite( &IconInfo, sizeof(MC_GCIconInfo), 1, pFile );

	fwrite( GC_IconData, IconDataSize, 1, pFile );

    return(TRUE);
}

////////////////////////////////////////////////////////////////////////////
// XBOX MEMCARD FILE EXPORT
////////////////////////////////////////////////////////////////////////////

#define XBOX_ICON_WIDTH		64
#define XBOX_ICON_HEIGHT	64

//==========================================================================
BOOL MCW_Project::ExportXBOX( FILE* pFile )
{
	int i;
	MC_XBOXHeader	Hdr;
	MC_XBOXFileType	FType;

    FILE*           pIconFile;
    byte*           pIconData;
	int             IconDataSize;


    ASSERT( pFile );
    if (pFile == NULL)
        return(FALSE);

    // Read in the icon file.
    pIconFile = fopen( m_XBOX.m_IconFile, "r" );
    ASSERT( pIconFile );
    if (pIconFile == NULL)
        return(FALSE);

    fseek( pIconFile, 0, X_SEEK_END );
    IconDataSize = ftell( pIconFile );
    fseek( pIconFile, 0, X_SEEK_SET );
    IconDataSize -= ftell( pIconFile );

    pIconData = new byte[IconDataSize];
    fread( pIconData, IconDataSize, 1, pIconFile );
    fclose( pIconFile );

	Hdr.Init();
	Hdr.m_NSaveTypes     = m_NSaveTypes;
	Hdr.m_SaveTypeOffset = sizeof(MC_XBOXHeader);
	Hdr.m_IconDataSize   = IconDataSize;
	Hdr.m_IconDataOffset = Hdr.m_SaveTypeOffset + (sizeof(MC_XBOXFileType) * m_NSaveTypes);
	Hdr.m_TotalFileSize  = Hdr.m_IconDataOffset + IconDataSize;

	fwrite( &Hdr, sizeof(MC_XBOXHeader), 1, pFile );

	for( i = 0; i < m_NSaveTypes; i++ )
	{
		FType.Init();

		FType.m_MaxSaves = m_XBOX.m_SaveTypes[i].m_MaxFiles;
		FType.m_FileSize = m_XBOX.m_SaveTypes[i].m_FileSize;

		strcpy( FType.m_Filename, m_XBOX.m_SaveTypes[i].m_Filename );

		fwrite( &FType, sizeof(MC_XBOXFileType), 1, pFile );
	}

	fwrite( pIconData, IconDataSize, 1, pFile );

    delete[] pIconData;

    return(TRUE);
}

////////////////////////////////////////////////////////////////////////////
// PC MEMCARD FILE EXPORT
////////////////////////////////////////////////////////////////////////////

//==========================================================================
BOOL MCW_Project::ExportPC( FILE* pFile )
{
	int i;
	MC_PCHeader	Hdr;
	MC_PCFileType	FType;
/*c
    FILE*           pIconFile;
    byte*           pIconData;
	int             IconDataSize;


    // Read in the icon file.
    pIconFile = fopen( m_PC.m_IconFile, "r" );
    ASSERT( pFile );

    fseek( pIconFile, 0, X_SEEK_END );
    IconDataSize = ftell( pIconFile );
    fseek( pIconFile, 0, X_SEEK_SET );
    IconDataSize -= ftell( pIconFile );

    pIconData = new byte[IconDataSize];
    fread( pIconData, IconDataSize, 1, pIconFile );
    fclose( pIconFile );
*/
	Hdr.Init();
	Hdr.m_NSaveTypes     = m_NSaveTypes;
	Hdr.m_SaveTypeOffset = sizeof(MC_PCHeader);
//c	Hdr.m_IconDataSize   = IconDataSize;
	Hdr.m_IconDataOffset = Hdr.m_SaveTypeOffset + (sizeof(MC_PCFileType) * m_NSaveTypes);
	Hdr.m_TotalFileSize  = Hdr.m_IconDataOffset;//c + IconDataSize;

	fwrite( &Hdr, sizeof(MC_PCHeader), 1, pFile );

	for( i = 0; i < m_NSaveTypes; i++ )
	{
		FType.Init();

		FType.m_MaxSaves = m_PC.m_SaveTypes[i].m_MaxFiles;
		FType.m_FileSize = m_PC.m_SaveTypes[i].m_FileSize;

		strcpy( FType.m_Filename, m_PC.m_SaveTypes[i].m_Filename );

		fwrite( &FType, sizeof(MC_PCFileType), 1, pFile );
	}

//c	fwrite( pIconData, IconDataSize, 1, pFile );

//c    delete[] pIconData;
    return(TRUE);
}
//==========================================================================

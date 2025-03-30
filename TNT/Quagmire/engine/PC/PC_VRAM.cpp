///////////////////////////////////////////////////////////////////////////////
//
// VRAM.CPP
//
///////////////////////////////////////////////////////////////////////////////
#include "x_files.hpp"
#include "Q_VRAM.hpp"
#include "PC_Video.hpp"
#include "x_bitmap.hpp"
#include "Aux_Bitmap.hpp"

//=====================================================================================================================================
// Local Functions.
//=====================================================================================================================================
static D3DFORMAT VRAM_PC_DetermineD3DTextureFormat( x_bitmap& BMP );
static void ExportBMP( x_bitmap& BMP, s32 ID );

//=====================================================================================================================================
// Defines
//=====================================================================================================================================
#define VRAM_END            -1
#define MAX_PALETTE_ID      10000

#undef VRAM_Register
#undef VRAM_DBG_Register
#undef VRAM_DBG_Dump
#undef VRAM_DBG_FreeSlots
#undef VRAM_DBG_UsedSlots

//=====================================================================================================================================
// VRAM Local TYPES
//=====================================================================================================================================
typedef struct 
{
	LPDIRECT3DTEXTURE8  pTexture;    // Actual D3D Surface
    s32                 PaletteID;   // ID of the palette previously stored in D3D.
	x_bitmap		   *pBitmap;     // bitmap that created the texture
    s32                 Next;	     // Next in the list
    s32                 Prev;	     // Previous in the list
} vram_node;

typedef struct 
{
    vram_node*	List;		        // List of active textures
    s32			ListNNodes;			// Size of the List

    s32			ListFreeNodes;		// Link list of empty nodes in the list
    s32			ListActiveNodes;	// Link list of active nodes in the list
    u32         NextD3DClutID;      // Number which represents the next available clut id for a paletized texture.
} vram;


//=====================================================================================================================================
// Globals
//=====================================================================================================================================
static vram* s_VRam = NULL;

//=====================================================================================================================================
// Implementation.
//=====================================================================================================================================
static void RemoveNodeFromList( s32* List, s32 NodeHandle )
{
    s32 Next, Prev;
    ASSERT( NodeHandle != VRAM_END );

    Next = s_VRam->List[ NodeHandle ].Next;
    Prev = s_VRam->List[ NodeHandle ].Prev;

    if ( Next != VRAM_END )
		s_VRam->List[ Next ].Prev = Prev;
    if ( Prev != VRAM_END )
		s_VRam->List[ Prev ].Next = Next;

    if( List )
    {
        if ( *List == NodeHandle )
        {
            ASSERT( Prev == VRAM_END );
            *List = Next;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
static void AddNodeToList( s32* List, s32 NodeHandle )
{
    ASSERT( NodeHandle != VRAM_END );

    if ( *List == VRAM_END )
    {
        s_VRam->List[ NodeHandle ].Next = VRAM_END;
        s_VRam->List[ NodeHandle ].Prev = VRAM_END;
    }
    else
    {
        ASSERT( s_VRam->List[ *List ].Prev == VRAM_END );

        s_VRam->List[ NodeHandle ].Prev   = VRAM_END;
        s_VRam->List[ NodeHandle ].Next   = *List;
        s_VRam->List[ *List ].Prev        = NodeHandle;
    }
    
    *List = NodeHandle;
}


//-------------------------------------------------------------------------------------------------------------------------------------
static err GetFreeNode( s32* Index )
{
    ASSERT( Index );
    
    // If the list is completly full we will have to grow it
    if ( s_VRam->ListFreeNodes == VRAM_END )
    {
        s32             PreviousSize;
        s32             i;
        vram_node*      NewList;

        // Allocating the new list
        PreviousSize      = s_VRam->ListNNodes;
        s_VRam->ListNNodes += 100;

        NewList = (vram_node*)x_realloc( s_VRam->List, sizeof(vram_node) * s_VRam->ListNNodes );
        if ( NewList == NULL )
		{
            x_printf("Out of memory while growing the texture cache\n");
            return ERR_FAILURE;
		}

        s_VRam->List = NewList;

        // Initialize the new section in the list
        for ( i = PreviousSize; i < s_VRam->ListNNodes; i++ )
        {
            s_VRam->List[ i ].Next = i + 1;
            s_VRam->List[ i ].Prev = i - 1;
        }

	    // Make sure to terminate the list corectly
        s_VRam->List[ i - 1 ].Next        = VRAM_END;
        s_VRam->List[ PreviousSize ].Prev = VRAM_END;

        // Point the begining of the free list
        s_VRam->ListFreeNodes = PreviousSize;

        x_printf("WARNING: There are %d textures registered with VRAM.\n", s_VRam->ListNNodes);
    }

    // Set the new index
    *Index = s_VRam->ListFreeNodes;

    // Remove the free node form the list
    RemoveNodeFromList( &s_VRam->ListFreeNodes, *Index );

    return ERR_SUCCESS;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32	AddTexture(x_bitmap &BMP)
{
	s32 Index;
    s32 MipMapCount;
    s32 PitchWidth;
    s32 PitchHeight;
    D3DFORMAT D3DFormat;

    // Get a free space for the texture
    if( GetFreeNode( &Index ) == ERR_FAILURE )
	{
        return -1;
	}

    // Set the new Node into the deactivated list
    AddNodeToList( &s_VRam->ListActiveNodes, Index );

	vram_node	*pNode;
	pNode = &s_VRam->List[Index];
	ASSERT(pNode);

    // Create the texture surface for the new bitmap.
    MipMapCount = BMP.GetNMips() == 0 ? 1 : BMP.GetNMips();
    PitchWidth  = BMP.GetPWidth();
    PitchHeight = BMP.GetPHeight();
    D3DFormat   = VRAM_PC_DetermineD3DTextureFormat( BMP );

    DXWARN(g_pD3DDevice->CreateTexture( PitchWidth,
                                        PitchHeight,
                                        MipMapCount,
										0,
                                        D3DFormat,
										D3DPOOL_MANAGED,
                                        &pNode->pTexture ));

    // Was the texture created?
	ASSERT(pNode->pTexture);

    // If so, add it to the node.
    pNode->pBitmap = &BMP;
	
    ASSERT(pNode->pBitmap);

    // Now copy the texture data over to the texture surface.
    //-------------------------------------------------------------------------------------------------------------------------------------
    s32                 Level;
	IDirect3DSurface8*  pD3DSurface;
    D3DLOCKED_RECT      D3DLockedRect;
    s32                 Height, Width;
    s32                 x, y;
    s32		            r,g,b,a;

    // If this is a non palettized texture, fill the surface with the actual texture color data.
    if( BMP.IsClutBased( ) == FALSE )
    {
        u32* pSurfaceData;
    
        for( Level=0; Level < (s32)(pNode->pTexture->GetLevelCount()); Level++ )
	    {
    		ASSERT(Level <= BMP.GetNMips()+1);

            // Set the pixel format for the source bitmap so we can use the GetPixelColor function.
		    BMP.SetPixelFormat(BMP, 0, Level);
    
            // Get the surface for this mip level.
		    pNode->pTexture->GetSurfaceLevel(Level, &pD3DSurface);

		    // Lock the entier surface so we can actually copy the color data to it.
            DXCHECK(pD3DSurface->LockRect(&D3DLockedRect, 0, 0 ));

            // Get the surface data pointer (ColorData).
		    pSurfaceData = (u32*)D3DLockedRect.pBits;

            // Now update the colors.
            Height = BMP.GetPHeight(Level);
            Width  = BMP.GetPWidth(Level);

		    for( y = 0; y < Height; y++ )
		    {
			    for( x = 0; x < Width; x++ )
			    {
				    color	Color = BMP.GetPixelColor(x, y, Level);
				    r = Color.R;
				    g = Color.G;
				    b = Color.B;
				    a = Color.A;

                    pSurfaceData[ Width * y + x ] = D3DCOLOR_ARGB( a, r, g, b );
			    }
		    }

	        DXCHECK(pD3DSurface->UnlockRect());

		    // done processing this level
		    pD3DSurface->Release();
        }

        // Set the palette to -1 since there isn't one being used.
        pNode->PaletteID = -1;
    }
    // If this is a palettized Texture, then fill the surface with the bitmaps pixel values.
    else
    {
        u8*          pSurfaceData;
        s32          ClutIndex;
        s32          nClutColors;
        PALETTEENTRY Palette[256];
        color        ClutColor;

        for( Level=0; Level < (s32)(pNode->pTexture->GetLevelCount()); Level++ )
	    {
            // Set the pixel format for the source bitmap so we can use the GetPixelColor function.
		    BMP.SetPixelFormat(BMP, 0, Level);
    
            // Get the surface for this mip level.
		    pNode->pTexture->GetSurfaceLevel(Level, &pD3DSurface);

		    // Lock the entier surface so we can actually copy the color data to it.
            DXCHECK(pD3DSurface->LockRect(&D3DLockedRect, 0, 0 ));

            // Get the surface data pointer (ColorData).
		    pSurfaceData = (u8*)D3DLockedRect.pBits;

            // Now update the pixel color index data in the surface.
            Height = BMP.GetPHeight(Level);
            Width  = BMP.GetPWidth(Level);

            // Copy the clut index for each pixel into the D3D texture surface.
		    for( y = 0; y < Height; y++ )
		    {
			    for( x = 0; x < Width; x++ )
			    {
				    ClutIndex = BMP.GetPixelIndex( x, y, Level);
                    pSurfaceData[ Width * y + x ] = ClutIndex;
			    }
		    }

		    // Processing of this surface is complete
	        DXCHECK(pD3DSurface->UnlockRect());
		    pD3DSurface->Release();
        }

        // Build a copy of the palette and trasnfer that color data over to the next available 
        // D3D palette.
        nClutColors = BMP.GetNClutColors( );
        for( ClutIndex = 0; ClutIndex < nClutColors; ClutIndex++ )
        {
            ClutColor = BMP.GetClutColor( ClutIndex );
            Palette[ClutIndex].peRed    = ClutColor.R;
            Palette[ClutIndex].peGreen  = ClutColor.G;
            Palette[ClutIndex].peBlue   = ClutColor.B;
            Palette[ClutIndex].peFlags  = ClutColor.A;
        }

        // Fill the D3D Palette with the color data in A8_R8_G8_B8 format.
        g_pD3DDevice->SetPaletteEntries( s_VRam->NextD3DClutID, Palette );

        // Set the D3D Palette ID for this texture.
        pNode->PaletteID = s_VRam->NextD3DClutID;

//        ExportBMP( BMP, pNode->PaletteID );

        // Update the next valid D3D Clut ID.
        s_VRam->NextD3DClutID++;
        ASSERT( s_VRam->NextD3DClutID < MAX_PALETTE_ID );
    }


    // Done updating texture, so clean up used objects
	return Index;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void DelTexture( s32 Handle )
{
	ASSERT(Handle != -1);

	vram_node *pNode = &s_VRam->List[Handle];
	ASSERT(pNode);

	SAFE_RELEASE(pNode->pTexture);
    pNode->PaletteID = -1;

    RemoveNodeFromList( &s_VRam->ListActiveNodes, Handle );
    AddNodeToList( &s_VRam->ListFreeNodes, Handle );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_Init( void )
{
    s32		i;

    // Handle the module instance initialization
    s_VRam = (vram*)x_malloc( sizeof(vram) );
    if ( s_VRam == NULL )
	{
        x_printf("Out of memory while allocating the vram module instance\n");
        ASSERT( 0 );
	}

    x_memset( s_VRam, 0, sizeof(vram) );

    // Allocate the Necesary space
    s_VRam->ListNNodes = 300;
    s_VRam->List       = (vram_node*)x_malloc( s_VRam->ListNNodes * sizeof(vram_node));
    if ( s_VRam->List == NULL )
	{
        x_printf("Out of memory while allocating the vram nodes\n");
        ASSERT( 0 );
	}

    // Initialize the List and the Free nodes
    for ( i = 0; i < s_VRam->ListNNodes; i++ )
    {
        s_VRam->List[ i ].Next = i + 1;
        s_VRam->List[ i ].Prev = i - 1;
    }

    // Make sure to terminate the list corectly
    s_VRam->List[  0  ].Prev = VRAM_END;
    s_VRam->List[ --i ].Next = VRAM_END;

    // Initialize the list of Active Nodes
    s_VRam->ListFreeNodes     = 0;
    s_VRam->ListActiveNodes   = VRAM_END;

    // Initialize the first usable D3D Clut ID.
    s_VRam->NextD3DClutID     = 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_Kill( void )
{
    // Unregister all registered entries
	ASSERT( s_VRam );

	while( s_VRam->ListActiveNodes != VRAM_END )
	{
	    vram_node	*pNode = &s_VRam->List[s_VRam->ListActiveNodes];
		ASSERT(pNode);

		VRAM_UnRegister(*pNode->pBitmap);
	}

    // Release the memory
	if( s_VRam )
	{
		if (s_VRam->List)
			x_free(s_VRam->List);

	    x_free( s_VRam );
	}

	s_VRam = NULL;
}


//-------------------------------------------------------------------------------------------------------------------------------------
/*void VRAM_Register(  )
{
    s32 ID;

    ID = AddTexture( BMP);

    BMP.SetVRAMID( ID );
}
*/
void VRAM_Register( x_bitmap& BMP, s32 iContext )
{
    (void)iContext;
    ASSERT( s_VRam != NULL );

    s32 VRAMID = BMP.GetVRAMID();

    if( VRAMID != -1 )
    {
/*        if( (VRAMID < 0) || (VRAMID >= VRAM_MAX_TEXTURES) )
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
*/
    }
    else
    {
        VRAMID = AddTexture( BMP );

        BMP.SetVRAMID( VRAMID );
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_UnRegister( x_bitmap& BMP )
{
	if (BMP.GetVRAMID() != -1)
	{
	    DelTexture( BMP.GetVRAMID() );
		BMP.SetVRAMID(-1);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_Activate( x_bitmap& BMP )
{
    VRAM_PC_Activate( BMP, 0 ); 
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_Activate( x_bitmap& BMP, s32 MinMip, s32 MaxMip )
{
    VRAM_PC_Activate( BMP, 0 );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_Deactivate( x_bitmap& BMP )
{
    VRAM_PC_Deactivate( 0 );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_SetWrapMode( s32 UMode, s32 VMode )
{
    VRAM_PC_SetWrapMode( UMode, VMode, 0 );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_PC_Activate( x_bitmap& BMP, s32 TexStage )
{
    s32 Handle = BMP.GetVRAMID();
	
    ASSERT(Handle != -1);
	if(Handle != -1)
	{
		vram_node *pNode = &s_VRam->List[Handle];
		ASSERT(pNode);

		DXWARN( g_pD3DDevice->SetTexture( TexStage, pNode->pTexture ) );

        if( pNode->PaletteID >= 0 )
            DXWARN( g_pD3DDevice->SetCurrentTexturePalette( pNode->PaletteID ) );
	}
	else
		DXWARN( g_pD3DDevice->SetTexture(TexStage, NULL) );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_PC_Deactivate( s32 TexStage )
{
    DXWARN( g_pD3DDevice->SetTexture(TexStage, NULL) );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VRAM_PC_SetWrapMode( s32 UMode, s32 VMode, s32 TexStage )
{
    s32 DXWrapModes[5] = {
                            D3DTADDRESS_CLAMP,  // WRAP_MODE_CLAMP    0
                            D3DTADDRESS_WRAP,   // WRAP_MODE_TILE     1
                            D3DTADDRESS_MIRROR, // WRAP_MODE_MIRROR   2
                            D3DTADDRESS_BORDER, // WRAP_MODE_REGION   3
                            D3DTADDRESS_WRAP,   // WRAP_MODE_R_REPEAT 4
                         };
    
    DXWARN(g_pD3DDevice->SetTextureStageState( TexStage, D3DTSS_ADDRESSU, DXWrapModes[UMode] ));
    DXWARN(g_pD3DDevice->SetTextureStageState( TexStage, D3DTSS_ADDRESSV, DXWrapModes[VMode] ));
}


//-------------------------------------------------------------------------------------------------------------------------------------
D3DFORMAT VRAM_PC_DetermineD3DTextureFormat( x_bitmap& BMP )
{
    u32 XBMPFormat;
    D3DFORMAT D3DFormat;
    
    XBMPFormat = BMP.GetFormat( );

    switch( XBMPFormat )
    {
        case x_bitmap::FMT_P8_ARGB_8888:
            D3DFormat = D3DFMT_P8;
            break;

        case x_bitmap::FMT_16_ARGB_4444:
            D3DFormat = D3DFMT_A4R4G4B4;
            break;

        case x_bitmap::FMT_16_RGB_565:
            D3DFormat = D3DFMT_R5G6B5;
            break;

        case x_bitmap::FMT_32_ARGB_8888:
            D3DFormat = D3DFMT_A8R8G8B8;
            break;

        // Invalid texture type being used.
        default:
            ASSERT( 0 );
            break;
    }
    return D3DFormat;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void ExportBMP( x_bitmap& BMP, s32 ID )
{
    s32                 x,y,z, i;
    s32                 Width, Height;
    BITMAPFILEHEADER    FileHeader;
    BITMAPINFOHEADER    InfoHeader;
    u8*                 pBMPData;
    RGBQUAD             clut[256];
    color               PixelColor;
    char                filename[250];
    s32                 FileStart, FileEnd;
    X_FILE*             fp;

    Height = BMP.GetHeight();
    Width =  BMP.GetWidth( );

    x_sprintf( filename, "TestBMP%d.bmp", ID );

    // Copy the index data over to the local copy.
    pBMPData = (u8*)x_malloc( sizeof( u8 ) * Width * Height );
    for( y = Height - 1, z = 0; y >= 0; y--, z++ )
	{
		for( x = 0; x < Width; x++ )
		{
			i = BMP.GetPixelIndex( x, y, 0);
            pBMPData[ Width * z + x ] = i;
		}
	}

    // Copy the clut colors.
    for( i = 0; i < BMP.GetNClutColors(); i++ )
    {
        PixelColor = BMP.GetClutColor(i);
        clut[i].rgbRed   = PixelColor.R;
        clut[i].rgbGreen = PixelColor.G;
        clut[i].rgbBlue  = PixelColor.B;
    }

    // Create the info header.
    InfoHeader.biSize = sizeof( InfoHeader );
    InfoHeader.biWidth = Width;
    InfoHeader.biHeight = Height;
    InfoHeader.biPlanes = 1;
    InfoHeader.biBitCount = 8;
    InfoHeader.biCompression = BI_RGB;
    InfoHeader.biSizeImage = BI_RGB;
    InfoHeader.biXPelsPerMeter = 0;
    InfoHeader.biYPelsPerMeter = 0;
    InfoHeader.biClrUsed = 256;
    InfoHeader.biClrImportant = 0; // all colors.

    fp = x_fopen( filename, "wb" );
    FileStart = x_fseek( fp, 0, SEEK_SET );
    x_fwrite( &FileHeader, sizeof( FileHeader ), 1,   fp );
    x_fwrite( &InfoHeader, sizeof( InfoHeader ), 1,   fp );
    x_fwrite( clut,        sizeof( RGBQUAD ),    256, fp );

    FileHeader.bfOffBits = x_ftell( fp ) - FileStart;
    x_fwrite( pBMPData,    sizeof( u8 )*Width*Height,         1,   fp );

    FileHeader.bfType = ('M' << 8) | ('B');
    FileHeader.bfReserved1 = 0;
    FileHeader.bfReserved2 = 0;

    FileEnd   = x_ftell( fp );
    FileHeader.bfSize = FileEnd - FileStart;

    x_fseek( fp, 0, SEEK_SET );

    // Re-write the header now that they entire thing is full of actual data.
    x_fwrite( &FileHeader, sizeof( FileHeader ), 1, fp );

    x_fclose( fp );
    x_free( pBMPData );
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
//    x_strncpy( s_VRam->m_RegisteredBMPs[ BMP.GetVRAMID() ].Filename, File, 16 );
//    s_pVRAMXBox->m_RegisteredBMPs[ BMP.GetVRAMID() ].Filename[15] = '\0';
//    s_pVRAMXBox->m_RegisteredBMPs[ BMP.GetVRAMID() ].LineNumber   = Line;

#endif //X_DEBUG
}

//==========================================================================

void VRAM_DBG_Dump( const char* Filename )
{
#if FALSE //def X_DEBUG

    s32 NSlotsFree;
    s32 NSlotsUsed;
    s32 i;

    X_FILE* fp = NULL;

    SPCTexture* pTex;

    Filename = "T:\\VRAMDump.txt";

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

    x_fclose( fp );

#endif //X_DEBUG
}

//==========================================================================

s32 VRAM_DBG_FreeSlots( void )
{
    s32 NSlotsFree = 0;
//    s32 i;

//    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
//    {
//        if( s_pVRAMXBox->m_RegisteredBMPs[i].m_pXBMP == NULL )
//            NSlotsFree++;
//    }

    return NSlotsFree;
}

//==========================================================================

s32 VRAM_DBG_UsedSlots( void )
{
    s32 NSlotsUsed = 0;
//    s32 i;

//    for( i = 0; i < VRAM_MAX_TEXTURES; i++ )
//    {
//        if( s_pVRAMXBox->m_RegisteredBMPs[i].m_pXBMP != NULL )
//            NSlotsUsed++;
//    }

    return NSlotsUsed;
}


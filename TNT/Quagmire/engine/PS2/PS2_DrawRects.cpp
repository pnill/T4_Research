//=========================================================================
//
//  PS2_DRAWRECTS.CPP
//
//  Included by PS2_Draw
//
//=========================================================================

static sceGifTag    GIFTAG_Rects3D;
static sceGifTag    GIFTAG_Rects2D;

typedef struct
{
    sceDmaTag   DMA;
    s32         VIF[4];
    sceGifTag   GIF;
} rect_header;

//=========================================================================

static
void InitRects( void )
{

    VIFHELP_BuildGifTag1(  &GIFTAG_Rects3D, VIFHELP_GIFMODE_PACKED, 
                           3, 0, TRUE, 
                           VIFHELP_GIFPRIMTYPE_TRIANGLE,
                           VIFHELP_GIFPRIMFLAGS_TEXTURE, 
                           TRUE );
    
    VIFHELP_BuildGifTag2(  &GIFTAG_Rects3D, 
                           VIFHELP_GIFREG_ST, 
                           VIFHELP_GIFREG_RGBAQ, 
                           VIFHELP_GIFREG_XYZ2,
                           0);

    VIFHELP_BuildGifTag1(  &GIFTAG_Rects2D, VIFHELP_GIFMODE_PACKED, 
                           2, 4, TRUE, 
                           VIFHELP_GIFPRIMTYPE_TRIANGLESTRIP,
                           VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE, 
                           TRUE );
    
    VIFHELP_BuildGifTag2(  &GIFTAG_Rects2D, 
                           VIFHELP_GIFREG_RGBAQ, 
                           VIFHELP_GIFREG_XYZ2,
                           0,
                           0 );
}

//=========================================================================

static
void BuildRectHeader( rect_header* pHeader, sceGifTag* GT )
{
    s32 DMASize;
    s32 TotalVertSize;

    TotalVertSize = 4 * 2 * 16; // (each vert has color and XYZW)
    DMASize = sizeof(rect_header) - sizeof(sceDmaTag) + TotalVertSize;
    
    ASSERT( (DMASize&0x0F) == 0 );
    DMAHELP_BuildTagCont( &pHeader->DMA, DMASize );
    
    pHeader->VIF[0] = 0;
    pHeader->VIF[1] = 0;
    pHeader->VIF[2] = 0;
    pHeader->VIF[3] = SCE_VIF1_SET_DIRECT( 1+(TotalVertSize>>4), 0 );

    pHeader->GIF = *GT;
    pHeader->GIF.NLOOP = 4;

    // Turn on alpha
    if( (s_DrawMode & DRAW_ALPHA) == DRAW_ALPHA )
    {
        pHeader->GIF.PRIM |= ((VIFHELP_GIFPRIMFLAGS_ALPHA)<<3);
        //pHeader->GIF.PRIM |= ((VIFHELP_GIFPRIMFLAGS_ANTIALIAS)<<3);
    }
}

//=========================================================================

void DRAW_Rectangle(
    f32 X, f32 Y, f32 Z,    // Hot spot (2D Left-Top), (3D Center)
    f32 W, f32 H,           // (2D pixel W&H), (3D World W&H)
    color  TLColor,         // Top Left     color
    color  TRColor,         // Top Right    color
    color  BLColor,         // Bottom Left  color
    color  BRColor )        // Bottom Right color
{
    vector3     Pos[4];
    color       Color[4];

    // convert points to PS2 screen space
    f32 fXConvert = PS2_GetScreenXConversionMultiplier();
    f32 fYConvert = PS2_GetScreenYConversionMultiplier();
    X *= fXConvert;
    Y *= fYConvert;
    W *= fXConvert;
    H *= fYConvert;

    ASSERT( ENG_GetRenderMode() );

    // Set colors
    Color[0] = TLColor;
    Color[1] = BLColor;
    Color[2] = TRColor;
    Color[3] = BRColor;

    // Compute vert positions
    Pos[0].X = X;
    Pos[0].Y = Y;
    Pos[0].Z = Z;
    Pos[1].X = X;
    Pos[1].Y = Y + H;
    Pos[1].Z = Z;
    Pos[2].X = X + W;
    Pos[2].Y = Y;
    Pos[2].Z = Z;
    Pos[3].X = X + W;
    Pos[3].Y = Y + H;
    Pos[3].Z = Z;

    if( (s_DrawMode & DRAW_2D) == DRAW_2D )
    {
        rect_header*    pHeader;
        byte*           pDest;

        // Compute size of output and get dlist ptr
        pDest = pDList;
        pDList += sizeof(rect_header) + 4*2*16;

        // Build Header
        pHeader = (rect_header*)pDest;
        pDest += sizeof(rect_header);
        BuildRectHeader( pHeader, &GIFTAG_Rects2D );

        // Add the verts
        Transform2D_C_NC( 4, pDest, Pos, Color );

        // Keep our stats current
        if ( s_pStatNVerts )
            *s_pStatNVerts = *s_pStatNVerts + 4;
        if ( s_pStatNTris )
            *s_pStatNTris = *s_pStatNTris + 2;
        if ( s_pStatNBytes )
            *s_pStatNBytes = *s_pStatNBytes + sizeof(rect_header) + 4*2*16;
    }
    else
    {
        ASSERTS( FALSE, "3D Rects not supported yet." );
    }
}

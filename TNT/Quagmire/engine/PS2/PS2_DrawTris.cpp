//=========================================================================
//
//  PS2_DRAWTRIS.CPP
//
//  Included by PS2_Draw
//
//=========================================================================


//==============================================================================
//  STRUCTS
//==============================================================================

typedef struct
{
    sceDmaTag   DMA;
    s32         VIF[4];
    sceGifTag   GIF;
} tri_header;


//==============================================================================
//  STATICS
//==============================================================================

static sceGifTag    GIFTAG_TrisTC;
static sceGifTag    GIFTAG_TrisC;
static sceGifTag    GIFTAG_Tris2DTC;

static s32          s_Open_NVerts;
static s32          s_Open_VertSize;
static tri_header*  s_Open_pTriHeader;
static sceGifTag*   s_Open_pGIFTag;
static void (*s_Open_pBuildFunc)( void );

static s32          s_Source_NVerts;
static vector3*     s_Source_pPos;
static vector2*     s_Source_pUV;
static vector3*     s_Source_pNormal;
static color*       s_Source_pColor;

static byte*        s_pDest;


//==============================================================================
//  PROTOTYPES FOR CLIPPING TRI BUILDER FUNCTIONS
//==============================================================================

static void BuildTris_TNC_C( void );
static void BuildTris_TC_C( void );
static void BuildTris_NC_C(void );
static void BuildTris_C_C(void );


//==============================================================================
//  InitTris()
//      Inits the draw triangles part of DRAW
//==============================================================================
static void InitTris( void )
{
    //------------------------------------------------------------

    VIFHELP_BuildGifTag1(  &GIFTAG_TrisTC, VIFHELP_GIFMODE_PACKED, 
                           3, 0, TRUE, 
                           VIFHELP_GIFPRIMTYPE_TRIANGLE,
                           VIFHELP_GIFPRIMFLAGS_TEXTURE|VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE, 
                           TRUE );
    
    VIFHELP_BuildGifTag2(  &GIFTAG_TrisTC, 
                           VIFHELP_GIFREG_ST, 
                           VIFHELP_GIFREG_RGBAQ, 
                           VIFHELP_GIFREG_XYZ2, 
                           0 );

    //------------------------------------------------------------

    VIFHELP_BuildGifTag1(  &GIFTAG_TrisC, VIFHELP_GIFMODE_PACKED, 
                           2, 0, TRUE, 
                           VIFHELP_GIFPRIMTYPE_TRIANGLE,
                           VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE, 
                           TRUE );
    
    VIFHELP_BuildGifTag2(  &GIFTAG_TrisC, 
                           VIFHELP_GIFREG_RGBAQ, 
                           VIFHELP_GIFREG_XYZ2, 
                           0,
                           0);

    //------------------------------------------------------------


    VIFHELP_BuildGifTag1(  &GIFTAG_Tris2DTC, VIFHELP_GIFMODE_PACKED, 
                           3, 0, TRUE, 
                           VIFHELP_GIFPRIMTYPE_TRIANGLE,
                           VIFHELP_GIFPRIMFLAGS_TEXTURE|VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE|VIFHELP_GIFPRIMFLAGS_UV,
                           TRUE );
    
    VIFHELP_BuildGifTag2(  &GIFTAG_Tris2DTC, 
                           VIFHELP_GIFREG_UV, 
                           VIFHELP_GIFREG_RGBAQ, 
                           VIFHELP_GIFREG_XYZ2, 
                           0 );

    ////////////////////////////////////////////////////////////////////////
    //  Init Static Vars
    ////////////////////////////////////////////////////////////////////////

    s_Open_NVerts       = 0;
    s_Open_VertSize     = NULL;
    s_Open_pTriHeader   = NULL;
    s_Open_pGIFTag      = NULL;
    s_Open_pBuildFunc   = NULL;

    s_Source_NVerts     = 0;
    s_Source_pPos       = NULL;
    s_Source_pUV        = NULL;
    s_Source_pNormal    = NULL;
    s_Source_pColor     = NULL;

    s_pDest             = NULL;

}

        

//==============================================================================
//  BuildTriHeader
//      Builds the DMA header for draw triangles
//==============================================================================                  
static void BuildTriHeader( tri_header* pHeader, s32 NVerts, s32 SizeOfVert, sceGifTag* GT )
{
    s32 DMASize;
    s32 TotalVertSize;

    TotalVertSize = NVerts*SizeOfVert;
    DMASize = sizeof(tri_header) - sizeof(sceDmaTag) + TotalVertSize;
    ASSERT( (DMASize&0x0F) == 0 );
    DMAHELP_BuildTagCont( &pHeader->DMA, DMASize );
    
    pHeader->VIF[0] = 0;
    pHeader->VIF[1] = 0;
    pHeader->VIF[2] = 0;
    pHeader->VIF[3] = SCE_VIF1_SET_DIRECT( 1+(TotalVertSize>>4), 0 );

    pHeader->GIF = *GT;
    pHeader->GIF.NLOOP = NVerts;

    if( (s_DrawMode & DRAW_ALPHA) == DRAW_ALPHA )
    {
        pHeader->GIF.PRIM |= ((VIFHELP_GIFPRIMFLAGS_ALPHA)<<3);
    }
}

//=========================================================================

static
void BuildTris2D_TC( void )
{
    ASSERT( s_Source_pPos && s_Source_pUV );
	Transform2D_TC_NC( s_Source_NVerts, s_pDest, s_Source_pPos, s_Source_pUV, s_Source_pColor );
}

//=========================================================================

static
void BuildTris2D_C( void )
{
    ASSERT( s_Source_pPos );
	Transform2D_C_NC( s_Source_NVerts, s_pDest, s_Source_pPos, s_Source_pColor );
}

//=========================================================================

static
void BuildTris_TC_NC( void )
{
    ASSERT( s_Source_pPos && s_Source_pUV );
    Transform_TC_NC( s_Source_NVerts, s_pDest, s_Source_pPos, s_Source_pUV, s_Source_pColor );
}

//=========================================================================

static
void BuildTris_TNC_NC( void )
{
    ASSERT( s_Source_pPos && s_Source_pUV && s_Source_pNormal );
    Transform_TNC_NC( s_Source_NVerts, s_pDest, s_Source_pPos, s_Source_pUV, s_Source_pNormal, s_Source_pColor );
}

//=========================================================================

static
void BuildTris_NC_NC( void )
{
    ASSERT( s_Source_pPos && s_Source_pNormal ); 
    Transform_NC_NC( s_Source_NVerts, s_pDest, s_Source_pPos, s_Source_pNormal, s_Source_pColor );
}

//=========================================================================

static
void BuildTris_C_NC( void )
{
    ASSERT( s_Source_pPos );
    Transform_C_NC( s_Source_NVerts, s_pDest, s_Source_pPos, s_Source_pColor );
}



//==============================================================================
//  SetTriangleSource()
//      Sets the source pointers for building triangles for build functions.  
//      This function is used to build functions can have common parameter lists
//==============================================================================
inline void SetTriangleSource( s32 NVerts, vector3* Pos, vector2* UV, vector3* Normal, color* Color )
{
    ASSERT( Pos );

    ///////////////////////////////////////////////////////////////////////////
    //  Assign Source Pointers
    ///////////////////////////////////////////////////////////////////////////
    s_Source_NVerts     = NVerts;
    s_Source_pPos       = Pos;
    s_Source_pUV        = UV;
    s_Source_pNormal    = Normal;
    s_Source_pColor     = Color;

    ///////////////////////////////////////////////////////////////////////////
    //  Get a pointer to display list dest
    ///////////////////////////////////////////////////////////////////////////
    s_pDest = pDList + (s_Open_VertSize*s_Open_NVerts);

    ///////////////////////////////////////////////////////////////////////////
    //  Update the number of open tris
    ///////////////////////////////////////////////////////////////////////////
    s_Open_NVerts += s_Source_NVerts;
}


//==============================================================================
//  DRAW_BeginTriangles()
//      Begin adding triangles to the display list
//==============================================================================
void DRAW_BeginTriangles( void )
{
    ///////////////////////////////////////////////////////////////////////////
    //  Make sure we are in a render mode and a tri header isn't open
    ///////////////////////////////////////////////////////////////////////////
    ASSERT( ENG_GetRenderMode() );
    ASSERT( !s_Open_pTriHeader );

    xbool b2DOn       = ((s_DrawMode&DRAW_2D) == DRAW_2D );
    xbool LightingOn  = ((s_DrawMode & DRAW_LIGHT) == DRAW_LIGHT );
    xbool TexturingOn = ((s_DrawMode & DRAW_TEXTURE) == DRAW_TEXTURE );
    xbool ClippingOn  = ((s_DrawMode & DRAW_CLIP ) == DRAW_CLIP );

    ///////////////////////////////////////////////////////////////////////////
    //  2D Triangles
    ///////////////////////////////////////////////////////////////////////////
    if ( b2DOn )
    {
        DRAW_Begin2DTriangles();
        return;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  3D Triangles
    ///////////////////////////////////////////////////////////////////////////

    s_Open_NVerts          = 0;
    s_Open_pTriHeader      = (tri_header*)pDList;

    pDList += sizeof(tri_header);

    if( TexturingOn )
    {
        s_Open_VertSize = 3*16;
        s_Open_pGIFTag = &GIFTAG_TrisTC;

        if( LightingOn ) s_Open_pBuildFunc = (ClippingOn) ? BuildTris_TNC_C : BuildTris_TNC_NC;
        else             s_Open_pBuildFunc = (ClippingOn) ? BuildTris_TC_C  : BuildTris_TC_NC;
    }
    else
    {
        s_Open_VertSize = 2*16;
        s_Open_pGIFTag = &GIFTAG_TrisC;

        if( LightingOn ) s_Open_pBuildFunc = (ClippingOn) ? BuildTris_NC_C : BuildTris_NC_NC;
        else             s_Open_pBuildFunc = (ClippingOn) ? BuildTris_C_C  : BuildTris_C_NC;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Update Stat Tracking
    ///////////////////////////////////////////////////////////////////////////
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + sizeof(tri_header);
}




//==============================================================================
//  DRAW_EndTriangles()
//      Draw stops adding triangles to the display list
//==============================================================================
void DRAW_EndTriangles( void )
{
    ///////////////////////////////////////////////////////////////////////////
    //  Make sure we are in a render mode
    ///////////////////////////////////////////////////////////////////////////
    ASSERT( ENG_GetRenderMode() );
    ASSERT(s_Open_pTriHeader);

    BuildTriHeader( s_Open_pTriHeader, s_Open_NVerts, s_Open_VertSize, s_Open_pGIFTag );
    s_Open_pTriHeader = NULL;

    ////////////////////////////////////////////////////////////////////////
    //  Increment display list
    ////////////////////////////////////////////////////////////////////////
    pDList += s_Open_VertSize*s_Open_NVerts;


    ///////////////////////////////////////////////////////////////////////////
    //  Update Stat Tracking 
    ///////////////////////////////////////////////////////////////////////////
    if ( s_pStatNVerts )
        *s_pStatNVerts = *s_pStatNVerts + s_Open_NVerts;
    if ( s_pStatNTris )
        *s_pStatNTris = *s_pStatNTris + s_Open_NVerts / 3;
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + s_Open_VertSize*s_Open_NVerts;

}



//=========================================================================
//  DRAW_Triangles()
//      Adds triangles to the display list
//==============================================================================
void DRAW_Triangles( s32 NVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{

    ///////////////////////////////////////////////////////////////////////////
    //  Make sure we are in a render mode
    ///////////////////////////////////////////////////////////////////////////
    ASSERT( ENG_GetRenderMode() );

    ///////////////////////////////////////////////////////////////////////////
    //  Check to see if we have open tri header, if not open one
    ///////////////////////////////////////////////////////////////////////////
    xbool bBeginTrisCalled = (s_Open_pTriHeader != NULL );
    if (!bBeginTrisCalled) DRAW_BeginTriangles();

    ///////////////////////////////////////////////////////////////////////////
    //  Set Triangle Source
    ///////////////////////////////////////////////////////////////////////////
    SetTriangleSource( NVerts, Pos, UV, Normal, Color );

    ///////////////////////////////////////////////////////////////////////////
    //  Run the build function
    ///////////////////////////////////////////////////////////////////////////
    s_Open_pBuildFunc();

    ///////////////////////////////////////////////////////////////////////////
    //  If begin was called in function call end
    ///////////////////////////////////////////////////////////////////////////
    if (!bBeginTrisCalled) DRAW_EndTriangles();
}



//==============================================================================
///////////////////////////////////////////////////////////////////////////////
//  2D Triangles
///////////////////////////////////////////////////////////////////////////////
//==============================================================================


//==============================================================================
//  Draw_Begin2DTriangles()
//      Begin drawing 2D triangles
//==============================================================================
void DRAW_Begin2DTriangles( void )
{

    ///////////////////////////////////////////////////////////////////////////
    //  Make sure we are in a render mode and a tri header isn't open
    ///////////////////////////////////////////////////////////////////////////
    ASSERT( ENG_GetRenderMode() );
    ASSERT( !s_Open_pTriHeader );

    s_Open_NVerts          = 0;
    s_Open_pTriHeader      = (tri_header*)pDList;

    pDList += sizeof(tri_header);

    xbool TexturingOn = ((s_DrawMode & DRAW_TEXTURE) == DRAW_TEXTURE );

    if( TexturingOn )
    {
        s_Open_VertSize = 3*16; 
        s_Open_pGIFTag = &GIFTAG_Tris2DTC;
        s_Open_pBuildFunc = BuildTris2D_TC;
    }
    else
    {
        s_Open_VertSize = 2*16; 
        s_Open_pGIFTag = &GIFTAG_TrisC;
        s_Open_pBuildFunc = BuildTris2D_C;       
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Update Stat Tracking
    ///////////////////////////////////////////////////////////////////////////
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + sizeof(tri_header);
}



//==============================================================================
//  DRAW_Triangles2D()
//      Draws 2D triangles
//==============================================================================
void DRAW_Triangles2D( s32 NVerts, vector3* Pos, color* Color, vector2* UV )
{
    s32 i;
    f32 fXConvert = PS2_GetScreenXConversionMultiplier();
    f32 fYConvert = PS2_GetScreenYConversionMultiplier();
    for (i=0; i<NVerts; ++i)
    {
        Pos[ i ].X *= fXConvert;
        Pos[ i ].Y *= fYConvert;
    }
    ///////////////////////////////////////////////////////////////////////////
    //  Make sure we are in a render mode
    ///////////////////////////////////////////////////////////////////////////
    ASSERT( ENG_GetRenderMode() );

    ///////////////////////////////////////////////////////////////////////////
    //  Check to see if we have open tri header, if not open one
    ///////////////////////////////////////////////////////////////////////////
    xbool bBeginTrisCalled = (s_Open_pTriHeader != NULL );
    if (!bBeginTrisCalled) DRAW_Begin2DTriangles();

    ///////////////////////////////////////////////////////////////////////////
    //  Make sure we are currently building 2d tris
    ///////////////////////////////////////////////////////////////////////////
    ASSERT( (s_Open_pBuildFunc == BuildTris2D_TC) || (s_Open_pBuildFunc = BuildTris2D_C) );

    ///////////////////////////////////////////////////////////////////////////
    //  Set Triangle Source
    ///////////////////////////////////////////////////////////////////////////
    SetTriangleSource( NVerts, Pos, UV, NULL, Color );

    ///////////////////////////////////////////////////////////////////////////
    //  Run the build function
    ///////////////////////////////////////////////////////////////////////////
    s_Open_pBuildFunc();

    ///////////////////////////////////////////////////////////////////////////
    //  If begin was called in function call end
    ///////////////////////////////////////////////////////////////////////////
    if (!bBeginTrisCalled) DRAW_EndTriangles();
}

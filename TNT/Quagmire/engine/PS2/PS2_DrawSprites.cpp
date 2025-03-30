//=========================================================================
//
//  PS2_DRAWSPRITES.CPP
//
//  Included by PS2_Draw
//
// Old TnT
//=========================================================================

static sceGifTag    GIFTAG_Sprites3D;
static sceGifTag    GIFTAG_Sprites2D;

static matrix4      s_SpriteL2V __attribute__ ((aligned(16)));


//==========================================================================
//  InitSprites( void )
//      Initializes the sprite GIF tags
//==========================================================================
static void InitSprites( void )
{

	VIFHELP_BuildGifTag1(  &GIFTAG_Sprites3D, VIFHELP_GIFMODE_PACKED,
							3, 0, TRUE,
							VIFHELP_GIFPRIMTYPE_SPRITE,
							VIFHELP_GIFPRIMFLAGS_TEXTURE,
							TRUE );

	VIFHELP_BuildGifTag2(  &GIFTAG_Sprites3D,
							VIFHELP_GIFREG_ST,
							VIFHELP_GIFREG_RGBAQ,
							VIFHELP_GIFREG_XYZ2,
							0);

	VIFHELP_BuildGifTag1(  &GIFTAG_Sprites2D, VIFHELP_GIFMODE_PACKED, 
							3, 0, TRUE, 
							VIFHELP_GIFPRIMTYPE_SPRITE,
							VIFHELP_GIFPRIMFLAGS_TEXTURE | VIFHELP_GIFPRIMFLAGS_UV, 
							TRUE );

	VIFHELP_BuildGifTag2(  &GIFTAG_Sprites2D, 
							VIFHELP_GIFREG_UV, 
							VIFHELP_GIFREG_RGBAQ, 
							VIFHELP_GIFREG_XYZ2,
							0);
}


//==========================================================================
//  Clip2DSpriteCoords()
//      A simple 2d sprite clipper
//==========================================================================
static xbool ClipSpriteCoords( f32& X, f32& Y,
                               f32& W, f32& H,
                               f32& U0, f32& V0,
                               f32& U1, f32& V1 )
{
    s32     ClipL, ClipR, ClipT, ClipB;
    f32     L, R, T, B;
    f32     t;

    ENG_Get2DClipArea( ClipL, ClipT, ClipR, ClipB );

    L = X;
    R = X + W;
    T = Y;
    B = Y + H;

    //---   do trivial rejection if we can
    if ( L > ClipR )
        return FALSE;
    if ( T > ClipB )
        return FALSE;
    if ( R < ClipL )
        return FALSE;
    if ( B < ClipT )
        return FALSE;
    if ( W == 0.0f || H == 0.0f )
        return FALSE;

    //---   clip to left side
    if ( L < ClipL )
    {
        t = (ClipL - L) / (R - L);
        L += t * (R-L);
        if ( U0 < U1 )
            U0 += t * (U1-U0);
        else
            U1 += t * (U0-U1);
    }

    //---   clip to right side
    if ( R > ClipR )
    {
        t = (R - ClipR) / (R - L);
        R -= t * (R-L);
        if ( U0 < U1 )
            U1 -= t * (U1-U0);
        else
            U0 -= t * (U0-U1);
    }

    //---   clip to top side
    if ( T < ClipT )
    {
        t = (ClipT - T) / (B - T);
        T += t * (B-T);
        if ( V0 < V1 )
            V0 += t * (V1-V0);
        else
            V1 += t * (V0-V1);
    }

    //---   clip to bottom side
    if ( B > ClipB )
    {
        t = (B - ClipB) / (B - T);
        B -= t * (B-T);
        if ( V0 < V1 )
            V1 -= t * (V1-V0);
        else
            V0 -= t * (V0-V1);
    }

    //---   do trivial rejection if we can
    if ( L > ClipR )
        return FALSE;
    if ( T > ClipB )
        return FALSE;
    if ( R < ClipL )
        return FALSE;
    if ( B < ClipT )
        return FALSE;

    X = L;
    Y = T;
    W = R - L;
    H = B - T;
    
    if ( W == 0.0f || H == 0.0f )
        return FALSE;

    return TRUE;
}


//==========================================================================
//  SpriteClip_TC( void )
//      Clips a sprite to scissor
//==========================================================================
static void BuildSprite_TC_C( void )
{
    f32         t;
    f32         Q;
    vector4     V;
    byte*       pDest       = s_pDest;
    vector3*    Pos         = s_Source_pPos;
    vector2*    UV          = s_Source_pUV;
    color*      Color       = s_Source_pColor;

    ASSERT( s_Source_pPos && s_Source_pUV );

    fvec4* LT_TD = &((fvec4*)pDest)[0];    // ptr to texture uvs
    ivec4* LT_CD = &((ivec4*)pDest)[1];    // ptr to color 
    fvec4* LT_PD = &((fvec4*)pDest)[2];    // ptr to position 
    fvec4* BR_TD = &((fvec4*)pDest)[3];    // ptr to texture uvs
    ivec4* BR_CD = &((ivec4*)pDest)[4];    // ptr to color 
    fvec4* BR_PD = &((fvec4*)pDest)[5];    // ptr to position 

    TransformV3( V, &s_VertexL2C, &Pos[0] );
    LT_PD->X = V.X;
    LT_PD->Y = V.Y;
    LT_PD->Z = V.Z;
    LT_PD->W = V.W;

    TransformV3( V, &s_VertexL2C, &Pos[1] );
    BR_PD->X = V.X;
    BR_PD->Y = V.Y;
    BR_PD->Z = V.Z;
    BR_PD->W = V.W;

    if (( LT_PD->X >  LT_PD->W ) ||
        ( LT_PD->Y < -LT_PD->W ) ||
        ( BR_PD->X < -BR_PD->W ) ||
        ( BR_PD->Y >  BR_PD->W ) ||
        ( LT_PD->Z < -LT_PD->W ) ||
        ( LT_PD->Z >  LT_PD->W ) ||
        ( LT_PD->X == BR_PD->X ) || 
        ( LT_PD->Y == BR_PD->Y ) ) 
    {
        s_Open_NVerts -= 2;
        return;
    }

    ////////////////////////////////////////////////////////////////
    // Write out STQ
    ////////////////////////////////////////////////////////////////
    LT_TD->X = UV[0].X;
    LT_TD->Y = UV[0].Y;
    LT_TD->Z = 1.0f;
    LT_TD->W = 1.0f;

    BR_TD->X = UV[1].X;
    BR_TD->Y = UV[1].Y;
    BR_TD->Z = 1.0f;
    BR_TD->W = 1.0f;

    ////////////////////////////////////////////////////////////////
    // Store colors
    ////////////////////////////////////////////////////////////////
    if ( Color )
    {
        LT_CD->X = MAKE_PS2_R(Color[0].R);
        LT_CD->Y = MAKE_PS2_G(Color[0].G);
        LT_CD->Z = MAKE_PS2_B(Color[0].B);
        LT_CD->W = MAKE_PS2_A(Color[0].A);
        BR_CD->X = MAKE_PS2_R(Color[1].R);
        BR_CD->Y = MAKE_PS2_G(Color[1].G);
        BR_CD->Z = MAKE_PS2_B(Color[1].B);
        BR_CD->W = MAKE_PS2_A(Color[1].A);
    }
    else
    {
        LT_CD->X = (127);
        LT_CD->Y = (127);
        LT_CD->Z = (127);
        LT_CD->W = (127);  
        BR_CD->X = (127);
        BR_CD->Y = (127);
        BR_CD->Z = (127);
        BR_CD->W = (127); 
    }

    ////////////////////////////////////////////////////////////////////////
    //---   clip to left/right side
    ////////////////////////////////////////////////////////////////////////
    if ( LT_PD->X < -LT_PD->W  )
    {
        t = (-LT_PD->W - LT_PD->X) / (BR_PD->X - LT_PD->X);
        LT_PD->X = -LT_PD->W;

        if ( LT_TD->X < BR_TD->X)   LT_TD->X  += t * (BR_TD->X-LT_TD->X);
        else                        BR_TD->X  += t * (LT_TD->X-BR_TD->X);
    }
    if ( BR_PD->X > BR_PD->W )
    {

        t = (BR_PD->X - BR_PD->W) / (BR_PD->X - LT_PD->X);
        BR_PD->X = BR_PD->W;

        if ( LT_TD->X < BR_TD->X )  BR_TD->X  -= t * (BR_TD->X-LT_TD->X);
        else                        LT_TD->X  -= t * (LT_TD->X-BR_TD->X);
    }

    ////////////////////////////////////////////////////////////////////////
    //---   clip to top/bottom side
    ////////////////////////////////////////////////////////////////////////
    if ( LT_PD->Y > LT_PD->W  )
    {
        t = ( LT_PD->W - LT_PD->Y) / (BR_PD->Y - LT_PD->Y);
        LT_PD->Y =  LT_PD->W;

        if ( LT_TD->Y < BR_TD->Y)   LT_TD->Y  += t * (BR_TD->Y-LT_TD->Y);
        else                        BR_TD->Y  += t * (LT_TD->Y-BR_TD->Y);
    }
    if ( BR_PD->Y < -BR_PD->W )
    {

        t = (BR_PD->Y - -BR_PD->W) / (BR_PD->Y - LT_PD->Y);
        BR_PD->Y = -BR_PD->W;

        if ( LT_TD->Y < BR_TD->Y )  BR_TD->Y  -= t * (BR_TD->Y-LT_TD->Y);
        else                        LT_TD->Y  -= t * (LT_TD->Y-BR_TD->Y);
    }


    ////////////////////////////////////////////////////////////////////////
    //  Finish LT Transform
    ////////////////////////////////////////////////////////////////////////
    TransformV4( V, &s_VertexC2S, (vector4*)LT_PD );

    /* Transform vertex from clipping space into screen space*/                       
    Q = 1.0f/V.W;   
                                 
    /* Transform uvs */
    LT_TD->X *= Q;
    LT_TD->Y *= Q;
    LT_TD->Z  = Q;
    LT_TD->W  = Q;

    /* Store fixed, projected screen verts */
    V *= Q * 16.0f; 
    ((ivec4*)LT_PD)->X = (s32)(V.X+0.5f);
    ((ivec4*)LT_PD)->Y = (s32)(V.Y+0.5f);
    ((ivec4*)LT_PD)->Z = (s32)(V.Z+0.5f);
    ((ivec4*)LT_PD)->W = (s32)(V.W+0.5f);

    //(*(reinterpret_cast< u32* > ( &LT_PD->W ))) |= (0x00000001 << 15);
    //(*((u32*)(&))) |= (0x00000001 << 15);

    ////////////////////////////////////////////////////////////////////////
    //  Finish LT Transform
    ////////////////////////////////////////////////////////////////////////
    TransformV4( V, &s_VertexC2S, (vector4*)BR_PD );

    /* Transform vertex from clipping space into screen space*/                       
    Q = 1.0f/V.W;   
                                 
    /* Transform uvs */
    BR_TD->X *= Q;
    BR_TD->Y *= Q;
    BR_TD->Z  = Q;
    BR_TD->W  = Q;

    /* Store fixed, projected screen verts */
    V *= Q * 16.0f; 
    ((ivec4*)BR_PD)->X = (s32)(V.X+0.5f);
    ((ivec4*)BR_PD)->Y = (s32)(V.Y+0.5f);
    ((ivec4*)BR_PD)->Z = (s32)(V.Z+0.5f);
    ((ivec4*)BR_PD)->W = (s32)(V.W+0.5f);

}


//==========================================================================
//  DRAW_BeginSprite()
//      Begin adding sprites to display list
//==========================================================================
void DRAW_BeginSprite( void )
{
    ///////////////////////////////////////////////////////////////////////////
    //  Make sure we are in a render mode and a tri header isn't open
    ///////////////////////////////////////////////////////////////////////////
    ASSERT( ENG_GetRenderMode() );
    ASSERT( !s_Open_pTriHeader );

    xbool b2DOn       = ((s_DrawMode&DRAW_2D) == DRAW_2D );

    s_Open_NVerts        = 0;
    s_Open_pTriHeader   = (tri_header*)pDList;
    s_Open_VertSize     = 3*16;

    if ( b2DOn )    s_Open_pGIFTag = &GIFTAG_Sprites2D;
    else            s_Open_pGIFTag = &GIFTAG_Sprites3D;

    pDList += sizeof(tri_header);

    ////////////////////////////////////////////////////////////////////////
    // Get the Local 2 View matrix
    ////////////////////////////////////////////////////////////////////////
    {
        
        matrix4 W2V ;//__attribute__ ((aligned(16)));
        matrix4 L2W(s_VertexL2W); //__attribute__ ((aligned(16)));

	    view* currView = ENG_GetActiveView();
        currView->GetW2VMatrix( W2V );

        #ifdef USE_VU0_AS_COPROC
        // VU0 Asm Version:
        asm __volatile__
        ("

            LQC2    vf04, 0x00(%0)  # load W2V col 0
            LQC2    vf05, 0x10(%0)  # load W2V col 1
            LQC2    vf06, 0x20(%0)  # load W2V col 2
            LQC2    vf07, 0x30(%0)  # load W2V col 3

            LQC2    vf08, 0x00(%1)  # load L2W col 0
            LQC2    vf09, 0x10(%1)  # load L2W col 1
            LQC2    vf10, 0x20(%1)  # load L2W col 2
            LQC2    vf11, 0x30(%1)  # load L2W col 3

            VMULAx.xyzw     ACC,    vf04,   vf08x   # do the first column
            VMADDAy.xyzw    ACC,    vf05,   vf08y
            VMADDAz.xyzw    ACC,    vf06,   vf08z
            VMADDw.xyzw    vf12,    vf07,   vf08w

            VMULAx.xyzw     ACC,    vf04,   vf09x   # do the second column
            VMADDAy.xyzw    ACC,    vf05,   vf09y
            VMADDAz.xyzw    ACC,    vf06,   vf09z
            VMADDw.xyzw    vf13,    vf07,   vf09w

            VMULAx.xyzw     ACC,    vf04,   vf10x   # do the third column
            VMADDAy.xyzw    ACC,    vf05,   vf10y
            VMADDAz.xyzw    ACC,    vf06,   vf10z
            VMADDw.xyzw    vf14,    vf07,   vf10w

            VMULAx.xyzw     ACC,    vf04,   vf11x   # do the third column
            VMADDAy.xyzw    ACC,    vf05,   vf11y
            VMADDAz.xyzw    ACC,    vf06,   vf11z
            VMADDw.xyzw    vf15,    vf07,   vf11w

            SQC2    vf12, 0x00(%2)          # store the resulting L2S matrix
            SQC2    vf13, 0x10(%2)
            SQC2    vf14, 0x20(%2)
            SQC2    vf15, 0x30(%2)

         " : : "r" (&W2V.M[0][0]), "r" (&L2W.M[0][0]) , "r" (&s_SpriteL2V.M[0][0]) : "memory" );
        #else // USE_VU0_AS_COPROC
            // cpu version:
	        s_SpriteL2V = W2V * L2W;
        #endif // USE_VU0_AS_COPROC

    }

    ///////////////////////////////////////////////////////////////////////////
    //  Update Stat Tracking
    ///////////////////////////////////////////////////////////////////////////
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + sizeof(tri_header);
}


//==========================================================================
//  DRAW_EndSprite()
//      End adding sprites to display list
//==========================================================================
void DRAW_EndSprite( void )
{
    ///////////////////////////////////////////////////////////////////////////
    //  Make sure we are in a render mode
    ///////////////////////////////////////////////////////////////////////////
    ASSERT( ENG_GetRenderMode() );
    ASSERT(s_Open_pTriHeader);

    ////////////////////////////////////////////////////////////////////////
    //  Build DMA header
    ////////////////////////////////////////////////////////////////////////
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
    //if ( s_pStatNTris )
    //    *s_pStatNTris = *s_pStatNTris + s_Open_NVerts / 3;
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + s_Open_VertSize*s_Open_NVerts;
}




//==========================================================================
//  DRAW_3DSprite()
//      Creates a 3D sprite
//==========================================================================
void DRAW_3DSprite( f32 X, f32 Y, f32 Z,
				    f32 W, f32 H,
					vector2* UV, color C )
{

    vector3 Pos[2];
    color   Color[2];

    ASSERT( ENG_GetRenderMode() );
    ASSERT( s_Open_pTriHeader != NULL );

    ////////////////////////////////////////////////////////////////////////
	// Find half width and height
    ////////////////////////////////////////////////////////////////////////
	f32 halfWidth  = W * 0.5f;
	f32 halfHeight = H * 0.5f;

    ////////////////////////////////////////////////////////////////////////
	// Extract the camera's x-axis as represented in world space
    ////////////////////////////////////////////////////////////////////////
	vector3 camLeft;
	camLeft.X = s_SpriteL2V.M[0][0] * halfWidth;
	camLeft.Y = s_SpriteL2V.M[1][0] * halfWidth;
	camLeft.Z = s_SpriteL2V.M[2][0] * halfWidth;

    ////////////////////////////////////////////////////////////////////////
	// Extract the camera's y-axis as represented in world space
    ////////////////////////////////////////////////////////////////////////
	vector3 camUp;
	camUp.X = s_SpriteL2V.M[0][1] * halfHeight;
	camUp.Y = s_SpriteL2V.M[1][1] * halfHeight;
	camUp.Z = s_SpriteL2V.M[2][1] * halfHeight;

    ////////////////////////////////////////////////////////////////////////
    //  Build Sprite Pos and Color
    ////////////////////////////////////////////////////////////////////////
	Pos[0].Set( X + camLeft.X + camUp.X, Y + camLeft.Y + camUp.Y, Z + camLeft.Z + camUp.Z ); //TL
	Pos[1].Set( X - camLeft.X - camUp.X, Y - camLeft.Y - camUp.Y, Z - camLeft.Z - camUp.Z ); //BR

    Color[0] = Color[1] = C;

    ////////////////////////////////////////////////////////////////////////
    //  Set the triangle source pointers.  Even thought this isn't really 
    //  a triangle primitive we are still gonna use the pointers.
    ////////////////////////////////////////////////////////////////////////
    SetTriangleSource( 2, Pos, UV, NULL, Color );

    ////////////////////////////////////////////////////////////////////////
    //  Clip sprite
    ////////////////////////////////////////////////////////////////////////
    if ( (s_DrawMode & DRAW_CLIP) == DRAW_CLIP )
    {
        BuildSprite_TC_C();
    }
    ////////////////////////////////////////////////////////////////////////
    //  Transform the points to the screen
    ////////////////////////////////////////////////////////////////////////
    else
    {
        BuildTris_TC_NC();
    }


}


//=========================================================================
//  DRAW_SpriteUV()
//      Draws a sprite with a specified UV coordinate
//==========================================================================
void DRAW_SpriteUV( 
    f32 X,  f32 Y, f32 Z,   // Hot spot. (2D Left-Top), (3D Center)
    f32 W,  f32 H,          // Width and Height of the sprite
    f32 U0, f32 V0,         // Upper-Left UV    [0 - 1]
    f32 U1, f32 V1,         // Bottom-Right UV  [0 - 1]
    color   C )          
{
    f32 fXConvert = PS2_GetScreenXConversionMultiplier();
    f32 fYConvert = PS2_GetScreenYConversionMultiplier();
    X *= fXConvert;
    Y *= fYConvert;
    W *= fXConvert;
    H *= fYConvert;

    ASSERT( ENG_GetRenderMode() );

    ///////////////////////////////////////////////////////////////////////////
    //  Check to see if we have open tri header, if not open one
    ///////////////////////////////////////////////////////////////////////////
    xbool bBeginTrisCalled = (s_Open_pTriHeader != NULL );
    if (!bBeginTrisCalled) DRAW_BeginSprite();

    ////////////////////////////////////////////////////////////////////////
    //  3D Sprite
    ////////////////////////////////////////////////////////////////////////
    if( (s_DrawMode & DRAW_3D) == DRAW_3D )
	{

        ////////////////////////////////////////////////////////////////////
        // Setup UVs
        ////////////////////////////////////////////////////////////////////
        vector2 UV[2];
        UV[0].X = U0;
        UV[0].Y = V0;
        UV[1].X = U1;
        UV[1].Y = V1;

		DRAW_3DSprite( X, Y, Z, W, H, UV, C );

	}
    ////////////////////////////////////////////////////////////////////////
    //  2D Sprite
    ////////////////////////////////////////////////////////////////////////
	else
	{
        vector3 Pos[2];
        vector2 UV[2];
        color   Color[2];

        ////////////////////////////////////////////////////////////////////
        //  Simple clip on sprite with rejection test
        ////////////////////////////////////////////////////////////////////
        if ( (s_DrawMode & DRAW_CLIP) == DRAW_CLIP )
        {
            if ( !ClipSpriteCoords( X, Y, W, H, U0, V0, U1, V1 ) )
            {
                ////////////////////////////////////////////////////////////
                //  Check to see if we had to open a tri header, if so close
                ////////////////////////////////////////////////////////////
                if (!bBeginTrisCalled) DRAW_EndSprite();

                return;
            }
        }

        ////////////////////////////////////////////////////////////////////
		// Compute vert positions
        ////////////////////////////////////////////////////////////////////
		Pos[0].X = X;
		Pos[0].Y = Y;
		Pos[0].Z = Z;
		Pos[1].X = X + W;
		Pos[1].Y = Y + H;
		Pos[1].Z = Z;

        ////////////////////////////////////////////////////////////////////
		// Calculate UV's
        ////////////////////////////////////////////////////////////////////
		UV[0].X = U0;
		UV[0].Y = V0;
		UV[1].X = U1;
		UV[1].Y = V1;

        ////////////////////////////////////////////////////////////////////
        // Build colors
        ////////////////////////////////////////////////////////////////////
        Color[0] = Color[1] = C;

        ////////////////////////////////////////////////////////////////////
        //  Set the triangle source pointers.  Even thought this isn't really 
        //  a triangle primitive we are still gonna use the pointers.
        ////////////////////////////////////////////////////////////////////
        SetTriangleSource( 2, Pos, UV, NULL, Color );

        BuildTris2D_TC();
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Check to see if we had to open a tri header, if so close it
    ///////////////////////////////////////////////////////////////////////////
    if (!bBeginTrisCalled) DRAW_EndSprite();
}

//=========================================================================

void DRAW_Sprite( 
    f32 X, f32 Y, f32 Z,    // Hot spot. (2D Left-Top), (3D Center)
    f32 W, f32 H,           // Width and Height of the sprite
    color  Color )          // 
{
    DRAW_SpriteUV( X, Y, Z, W, H, 0.0f, 0.0f, 1.0f, 1.0f, Color );
}

//=========================================================================


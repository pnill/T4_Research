//=========================================================================
//
//  PS2_DRAWLINES.CPP
//
//  Included by PS2_Draw
//
//=========================================================================

static sceGifTag    GIFTAG_LinesC; // Just colors

//=========================================================================

inline static s32 ScissorLeft( void )
{
    return ((2048 - ENG_GetScreenWidth()/2) << 4);
}
inline static s32 ScissorTop( void )
{
    return ((2048 - ENG_GetScreenHeight()/2) << 4);
}

static
void LinesTransform_C_NC( s32 NVerts, void* Dest, vector3* Pos, color* Color )
{
    matrix4     L2S;
    s32         i;
    ivec4*      pV;

    // Get local copy of matrix
    L2S = s_VertexL2S;   

    // Transform into dest
    pV = (ivec4*)Dest;
    for( i=0; i<NVerts; i++ )             
    {
        f32   X,Y,Z,W,Q;

        // Store colors
        if( Color )
        {
            ((ivec4*)pV)[0].X = Color->R;
            ((ivec4*)pV)[0].Y = Color->G;
            ((ivec4*)pV)[0].Z = Color->B;
            ((ivec4*)pV)[0].W = Color->A;
            Color++;
        }
        else
        {
            ((ivec4*)pV)[0].X = 127;
            ((ivec4*)pV)[0].Y = 127;
            ((ivec4*)pV)[0].Z = 127;
            ((ivec4*)pV)[0].W = 127;
        }

        // Transform vertex and compute Q
        X = L2S.M[0][0]*Pos->X +  L2S.M[1][0]*Pos->Y + L2S.M[2][0]*Pos->Z + L2S.M[3][0];
        Y = L2S.M[0][1]*Pos->X +  L2S.M[1][1]*Pos->Y + L2S.M[2][1]*Pos->Z + L2S.M[3][1];
        Z = L2S.M[0][2]*Pos->X +  L2S.M[1][2]*Pos->Y + L2S.M[2][2]*Pos->Z + L2S.M[3][2];
        W = L2S.M[0][3]*Pos->X +  L2S.M[1][3]*Pos->Y + L2S.M[2][3]*Pos->Z + L2S.M[3][3];
        Q = 16.0f/W;
        
        // Write out fixed, projected version
        pV[1].X = (s32)(X*Q);
        pV[1].Y = (s32)(Y*Q);
        pV[1].Z = (s32)(Z*Q);
        pV[1].W = (s32)(W*Q);

        Pos++;
        pV+=2;
    }
}

//==========================================================================

static
void LinesTransform2D_C_NC( s32 NVerts, void* Dest, vector3* Pos, color* Color )
{
    s32         i;
    ivec4*      pV;

    // Transform into dest
    pV = (ivec4*)Dest;
    for( i=0; i<NVerts; i++ )             
    {
        // Store colors
        if( Color )
        {
            ((ivec4*)pV)[0].X = Color->R;
            ((ivec4*)pV)[0].Y = Color->G;
            ((ivec4*)pV)[0].Z = Color->B;
            ((ivec4*)pV)[0].W = Color->A;
            Color++;
        }
        else
        {
            ((ivec4*)pV)[0].X = 127;
            ((ivec4*)pV)[0].Y = 127;
            ((ivec4*)pV)[0].Z = 127;
            ((ivec4*)pV)[0].W = 127;
        }

        // Store out the coordinates
        pV[1].X = (s32)(Pos->X * 16.0f) + ScissorLeft();
        pV[1].Y = (s32)(Pos->Y * 16.0f) + ScissorTop();
        pV[1].Z = (s32)(Pos->Z * 65536.0f);
        pV[1].W = 1 << 4;

        Pos++;
        pV+=2;
    }
}

//=========================================================================

static
void InitLines( void )
{

    VIFHELP_BuildGifTag1(  &GIFTAG_LinesC, VIFHELP_GIFMODE_PACKED, 
                           2, 0, TRUE, 
                           VIFHELP_GIFPRIMTYPE_LINE,
                           VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE, 
                           TRUE );
    
    VIFHELP_BuildGifTag2(  &GIFTAG_LinesC, 
                           VIFHELP_GIFREG_RGBAQ, 
                           VIFHELP_GIFREG_XYZ2, 
                           0,
                           0);
}

//=========================================================================

typedef struct
{
    sceDmaTag   DMA;
    s32         VIF[4];
    sceGifTag   GIF;
} line_header;
                          
static          
void BuildLineHeader( line_header* pHeader, s32 NVerts, s32 SizeOfVert, sceGifTag* GT )
{
    s32 DMASize;
    s32 TotalVertSize;

    TotalVertSize = NVerts*SizeOfVert;
    DMASize = sizeof(line_header) - sizeof(sceDmaTag) + TotalVertSize;
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
void BuildLines_C_NC( s32 NVerts, vector3* Pos, color* Color )
{
    line_header* pHeader;
    byte*       pDest;
//    matrix4     L2S;

    ASSERT( Pos );         

    // Compute size of output and get dlist ptr
    pDest = pDList;
    pDList += sizeof(line_header) + 2*16*NVerts;

    // Build Header
    pHeader = (line_header*)pDest;
    pDest += sizeof(line_header);
    BuildLineHeader( pHeader, NVerts, 2*16, &GIFTAG_LinesC );
    
    LinesTransform_C_NC( NVerts, pDest, Pos, Color );

    // Keep our stats current
    if ( s_pStatNVerts )
        *s_pStatNVerts = *s_pStatNVerts + NVerts;
    if ( s_pStatNTris )
        *s_pStatNTris = *s_pStatNTris + 0;
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + sizeof(line_header) + 2*16*NVerts;
}

//==========================================================================

static
void BuildLines2D_C_NC( s32 NVerts, vector3* Pos, color* Color )
{
    line_header* pHeader;
    byte*       pDest;
//    matrix4     L2S;

    ASSERT( Pos );         

    // Compute size of output and get dlist ptr
    pDest = pDList;
    pDList += sizeof(line_header) + 2*16*NVerts;

    // Build Header
    pHeader = (line_header*)pDest;
    pDest += sizeof(line_header);
    BuildLineHeader( pHeader, NVerts, 2*16, &GIFTAG_LinesC );
    
    LinesTransform2D_C_NC( NVerts, pDest, Pos, Color );

    // Keep our stats current
    if ( s_pStatNVerts )
        *s_pStatNVerts = *s_pStatNVerts + NVerts;
    if ( s_pStatNTris )
        *s_pStatNTris = *s_pStatNTris + 0;
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + sizeof(line_header) + 2*16*NVerts;
}

//==========================================================================

#define LINESEG_CLIP_ALL_XMAX   (CLIPFLAG_XMAX | (CLIPFLAG_XMAX<<6))
#define LINESEG_CLIP_ALL_XMIN   (CLIPFLAG_XMIN | (CLIPFLAG_XMIN<<6))
#define LINESEG_CLIP_ALL_YMAX   (CLIPFLAG_YMAX | (CLIPFLAG_YMAX<<6))
#define LINESEG_CLIP_ALL_YMIN   (CLIPFLAG_YMIN | (CLIPFLAG_YMIN<<6))
#define LINESEG_CLIP_ALL_ZMAX   (CLIPFLAG_ZMAX | (CLIPFLAG_ZMAX<<6))
#define LINESEG_CLIP_ALL_ZMIN   (CLIPFLAG_ZMIN | (CLIPFLAG_ZMIN<<6))

//==========================================================================

#define LINESEG_CLIP_MAX( param, V, C )                                 \
{                                                                       \
    xbool   AIn;                                                        \
    xbool   BIn;                                                        \
    f32     T;                                                          \
    s32     In, Out;                                                    \
                                                                        \
    AIn = V[0].param < V[0].W;                                          \
    BIn = V[1].param < V[1].W;                                          \
                                                                        \
    if ( (AIn == FALSE) && (BIn == FALSE) )     return FALSE;           \
    if ( (AIn == TRUE) && (BIn == TRUE) )       return TRUE;            \
    if ( AIn == TRUE )                          { In = 0; Out = 1; }    \
    else                                        { In = 1; Out = 0; }    \
                                                                        \
    T = (V[In].W - V[In].param) /                                       \
        ((V[Out].param - V[In].param) - (V[Out].W - V[In].W));          \
                                                                        \
    V[Out].X = V[In].X + T * (V[Out].X - V[In].X);                      \
    V[Out].Y = V[In].Y + T * (V[Out].Y - V[In].Y);                      \
    V[Out].Z = V[In].Z + T * (V[Out].Z - V[In].Z);                      \
    V[Out].W = V[In].W + T * (V[Out].W - V[In].W);                      \
    C[Out].R = (s32)((f32)C[In].R + T * (f32)(C[Out].R - C[In].R));     \
    C[Out].G = (s32)((f32)C[In].G + T * (f32)(C[Out].G - C[In].G));     \
    C[Out].B = (s32)((f32)C[In].B + T * (f32)(C[Out].B - C[In].B));     \
    C[Out].A = (s32)((f32)C[In].A + T * (f32)(C[Out].A - C[In].A));     \
                                                                        \
    return TRUE;                                                        \
}

//==========================================================================

#define LINESEG_CLIP_MIN( param, V, C )                                 \
{                                                                       \
    xbool   AIn;                                                        \
    xbool   BIn;                                                        \
    f32     T;                                                          \
    s32     In, Out;                                                    \
                                                                        \
    AIn = V[0].param > -V[0].W;                                         \
    BIn = V[1].param > -V[1].W;                                         \
                                                                        \
    if ( (AIn == FALSE) && (BIn == FALSE) )     return FALSE;           \
    if ( (AIn == TRUE) && (BIn == TRUE) )       return TRUE;            \
    if ( AIn == TRUE )                          { In = 0; Out = 1; }    \
    else                                        { In = 1; Out = 0; }    \
                                                                        \
    T = (-V[In].W - V[In].param) /                                      \
        ((V[Out].param - V[In].param) - (-V[Out].W + V[In].W));         \
                                                                        \
    V[Out].X = V[In].X + T * (V[Out].X - V[In].X);                      \
    V[Out].Y = V[In].Y + T * (V[Out].Y - V[In].Y);                      \
    V[Out].Z = V[In].Z + T * (V[Out].Z - V[In].Z);                      \
    V[Out].W = V[In].W + T * (V[Out].W - V[In].W);                      \
    C[Out].R = (s32)((f32)C[In].R + T * (f32)(C[Out].R - C[In].R));     \
    C[Out].G = (s32)((f32)C[In].G + T * (f32)(C[Out].G - C[In].G));     \
    C[Out].B = (s32)((f32)C[In].B + T * (f32)(C[Out].B - C[In].B));     \
    C[Out].A = (s32)((f32)C[In].A + T * (f32)(C[Out].A - C[In].A));     \
                                                                        \
    return TRUE;                                                        \
}

//==========================================================================

xbool ClipZToMax( vector4* Verts, color* Colors ) LINESEG_CLIP_MAX(Z,Verts,Colors)
xbool ClipZToMin( vector4* Verts, color* Colors ) LINESEG_CLIP_MIN(Z,Verts,Colors)
xbool ClipXToMax( vector4* Verts, color* Colors ) LINESEG_CLIP_MAX(X,Verts,Colors)
xbool ClipXToMin( vector4* Verts, color* Colors ) LINESEG_CLIP_MIN(X,Verts,Colors)
xbool ClipYToMax( vector4* Verts, color* Colors ) LINESEG_CLIP_MAX(Y,Verts,Colors)
xbool ClipYToMin( vector4* Verts, color* Colors ) LINESEG_CLIP_MIN(Y,Verts,Colors)


//==========================================================================

xbool ClipLineSegment( vector4* Verts, color* Colors )
{
    vector4     TransVert;
    u32         ClipFlags = 0;
    matrix4*    L2C = &s_VertexL2C;

    //---   Transform verts into clipping space
    TransVert.X = L2C->M[0][0]*Verts[0].X + L2C->M[1][0]*Verts[0].Y + L2C->M[2][0]*Verts[0].Z + L2C->M[3][0];
    TransVert.Y = L2C->M[0][1]*Verts[0].X + L2C->M[1][1]*Verts[0].Y + L2C->M[2][1]*Verts[0].Z + L2C->M[3][1];
    TransVert.Z = L2C->M[0][2]*Verts[0].X + L2C->M[1][2]*Verts[0].Y + L2C->M[2][2]*Verts[0].Z + L2C->M[3][2];
    TransVert.W = L2C->M[0][3]*Verts[0].X + L2C->M[1][3]*Verts[0].Y + L2C->M[2][3]*Verts[0].Z + L2C->M[3][3];
    Verts[0] = TransVert;

    TransVert.X = L2C->M[0][0]*Verts[1].X + L2C->M[1][0]*Verts[1].Y + L2C->M[2][0]*Verts[1].Z + L2C->M[3][0];
    TransVert.Y = L2C->M[0][1]*Verts[1].X + L2C->M[1][1]*Verts[1].Y + L2C->M[2][1]*Verts[1].Z + L2C->M[3][1];
    TransVert.Z = L2C->M[0][2]*Verts[1].X + L2C->M[1][2]*Verts[1].Y + L2C->M[2][2]*Verts[1].Z + L2C->M[3][2];
    TransVert.W = L2C->M[0][3]*Verts[1].X + L2C->M[1][3]*Verts[1].Y + L2C->M[2][3]*Verts[1].Z + L2C->M[3][3];
    Verts[1] = TransVert;

    //---   Collect the clipping flags
    ClipFlags <<= 6;
    if ( Verts[0].X > +Verts[0].W ) ClipFlags |= CLIPFLAG_XMAX;
    if ( Verts[0].X < -Verts[0].W ) ClipFlags |= CLIPFLAG_XMIN;
    if ( Verts[0].Y > +Verts[0].W ) ClipFlags |= CLIPFLAG_YMAX;
    if ( Verts[0].Y < -Verts[0].W ) ClipFlags |= CLIPFLAG_YMIN;
    if ( Verts[0].Z > +Verts[0].W ) ClipFlags |= CLIPFLAG_ZMAX;
    if ( Verts[0].Z < -Verts[0].W ) ClipFlags |= CLIPFLAG_ZMIN;

    ClipFlags <<= 6;
    if ( Verts[1].X > +Verts[1].W ) ClipFlags |= CLIPFLAG_XMAX;
    if ( Verts[1].X < -Verts[1].W ) ClipFlags |= CLIPFLAG_XMIN;
    if ( Verts[1].Y > +Verts[1].W ) ClipFlags |= CLIPFLAG_YMAX;
    if ( Verts[1].Y < -Verts[1].W ) ClipFlags |= CLIPFLAG_YMIN;
    if ( Verts[1].Z > +Verts[1].W ) ClipFlags |= CLIPFLAG_ZMAX;
    if ( Verts[1].Z < -Verts[1].W ) ClipFlags |= CLIPFLAG_ZMIN;

    if( ClipFlags & 0x00000FFF )
    {
        xbool ret;

        //---   lines need to be clipped

        //---   trivial rejection
        if ( (ClipFlags & LINESEG_CLIP_ALL_XMAX) == LINESEG_CLIP_ALL_XMAX ) return FALSE;
        if ( (ClipFlags & LINESEG_CLIP_ALL_XMIN) == LINESEG_CLIP_ALL_XMIN ) return FALSE;
        if ( (ClipFlags & LINESEG_CLIP_ALL_YMAX) == LINESEG_CLIP_ALL_YMAX ) return FALSE;
        if ( (ClipFlags & LINESEG_CLIP_ALL_YMIN) == LINESEG_CLIP_ALL_YMIN ) return FALSE;
        if ( (ClipFlags & LINESEG_CLIP_ALL_ZMAX) == LINESEG_CLIP_ALL_ZMAX ) return FALSE;
        if ( (ClipFlags & LINESEG_CLIP_ALL_ZMIN) == LINESEG_CLIP_ALL_ZMIN ) return FALSE;

        //---   clip to individual planes
        if ( ClipFlags & LINESEG_CLIP_ALL_ZMIN )
        {
            ret = ClipZToMin( Verts, Colors );
            if ( ret == FALSE ) return FALSE;
        }
        if ( ClipFlags & LINESEG_CLIP_ALL_ZMAX )
        {
            ret = ClipZToMax( Verts, Colors );
            if ( ret == FALSE ) return FALSE;
        }
        if ( ClipFlags & LINESEG_CLIP_ALL_XMAX )
        {
            ret = ClipXToMax( Verts, Colors );
            if ( ret == FALSE ) return FALSE;
        }
        if ( ClipFlags & LINESEG_CLIP_ALL_XMIN )
        {
            ret = ClipXToMin( Verts, Colors );
            if ( ret == FALSE ) return FALSE;
        }
        if ( ClipFlags & LINESEG_CLIP_ALL_YMAX )
        {
            ret = ClipYToMax( Verts, Colors );
            if ( ret == FALSE ) return FALSE;
        }
        if ( ClipFlags & LINESEG_CLIP_ALL_YMIN )
        {
            ret = ClipYToMin( Verts, Colors );
            if ( ret == FALSE ) return FALSE;
        }
    }

    return TRUE;
}

//==========================================================================

static
void BuildLines_C_C( s32 NVerts, vector3* Pos, color* Color )
{
    s32 i;
    s32 LineSeg;
    s32 NClippedVerts = 0;
    f32 Q;
    s32* pVert;

    vector4*        ClippedVerts;
    color*          ClippedColors;
    matrix4*        C2S = &s_VertexC2S;
    line_header*    pLineHdr;
    vector4         TransVec;

    SMEM_StackPushMarker();
    ClippedVerts = (vector4*)SMEM_StackAlloc( NVerts * sizeof(vector4) );
    ASSERT( ClippedVerts );
    ClippedColors = (color*)SMEM_StackAlloc( NVerts * sizeof(color) );
    ASSERT( ClippedColors );
    
    for ( LineSeg = 0; LineSeg < NVerts / 2; LineSeg++ )
    {
        ClippedVerts[NClippedVerts] = Pos[LineSeg * 2];
        ClippedVerts[NClippedVerts].W = 1.0f;
        ClippedVerts[NClippedVerts+1] = Pos[LineSeg * 2 + 1];
        ClippedVerts[NClippedVerts+1].W = 1.0f;
        ClippedColors[NClippedVerts] = Color[LineSeg * 2];
        ClippedColors[NClippedVerts+1] = Color[LineSeg * 2 + 1];

        if ( ClipLineSegment( &ClippedVerts[NClippedVerts], &ClippedColors[NClippedVerts] ) )
        {
            NClippedVerts += 2;
        }
    }

    //---   set up the header
    pLineHdr = (line_header*)pDList;
    pDList += sizeof(line_header);
    BuildLineHeader( pLineHdr, NClippedVerts, 2 * 16, &GIFTAG_LinesC );

    //---   build the lines in screen space
    for ( i = 0; i < NClippedVerts; i++ )
    {
        TransVec.X = C2S->M[0][0] * ClippedVerts[i].X +
                     C2S->M[1][0] * ClippedVerts[i].Y +
                     C2S->M[2][0] * ClippedVerts[i].Z +
                     C2S->M[3][0] * ClippedVerts[i].W;
        TransVec.Y = C2S->M[0][1] * ClippedVerts[i].X +
                     C2S->M[1][1] * ClippedVerts[i].Y +
                     C2S->M[2][1] * ClippedVerts[i].Z +
                     C2S->M[3][1] * ClippedVerts[i].W;
        TransVec.Z = C2S->M[0][2] * ClippedVerts[i].X +
                     C2S->M[1][2] * ClippedVerts[i].Y +
                     C2S->M[2][2] * ClippedVerts[i].Z +
                     C2S->M[3][2] * ClippedVerts[i].W;
        TransVec.W = C2S->M[0][3] * ClippedVerts[i].X +
                     C2S->M[1][3] * ClippedVerts[i].Y +
                     C2S->M[2][3] * ClippedVerts[i].Z +
                     C2S->M[3][3] * ClippedVerts[i].W;
        Q = 16.0f / TransVec.W;

        TransVec.X *= Q;
        TransVec.Y *= Q;
        TransVec.Z *= Q;
        TransVec.W *= Q;

        //---   store the color
        pVert = (s32*)pDList;
        pVert[0] = ClippedColors[i].R;
        pVert[1] = ClippedColors[i].G;
        pVert[2] = ClippedColors[i].B;
        pVert[3] = ClippedColors[i].A;
        pDList += sizeof(s32) * 4;

        //---   store fixed, projected screen verts
        pVert = (s32*)pDList;
        pVert[0] = (s32)TransVec.X;
        pVert[1] = (s32)TransVec.Y;
        pVert[2] = (s32)TransVec.Z;
        pVert[3] = (s32)TransVec.W;
        pDList += sizeof(s32) * 4;
    }

    //---   keep our stats current
    if ( s_pStatNVerts )
        *s_pStatNVerts = *s_pStatNVerts + NClippedVerts;
    if ( s_pStatNTris )
        *s_pStatNTris = *s_pStatNTris + 0;
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + sizeof(line_header) + 2*16*NClippedVerts;


    SMEM_StackPopToMarker();
}

//=========================================================================

void DRAW_Lines( s32        NVerts, 
                  vector3*  Pos, 
                  color*    Color)
{
    ASSERT( Pos );
    ASSERT( ENG_GetRenderMode() );

	ASSERT( NVerts >= 0 );

	color* vertColors = NULL;
	if (Color)
	{
        SMEM_StackPushMarker();
        vertColors = (color*)SMEM_StackAlloc( sizeof(color)*NVerts );
        ASSERT( vertColors );

		s32 vertCount;
		for (vertCount=0; vertCount < NVerts; vertCount++)
		{
			vertColors[vertCount].R = (Color[vertCount].R << 1) / 3;
			vertColors[vertCount].G = (Color[vertCount].G << 1) / 3;
			vertColors[vertCount].B = (Color[vertCount].B << 1) / 3;
			vertColors[vertCount].A = Color[vertCount].A >> 1;
		}
	}

    if ( (s_DrawMode & DRAW_3D) == DRAW_3D )
    {
        if ( (s_DrawMode & DRAW_CLIP) == DRAW_CLIP )
            BuildLines_C_C( NVerts, Pos, vertColors );
        else
            BuildLines_C_NC( NVerts, Pos, vertColors );
        
    }
    else
    {
        if ( (s_DrawMode & DRAW_CLIP) == DRAW_CLIP )
        {
            ASSERTS( FALSE, "2D Line clipping not supported." );
        }
        BuildLines2D_C_NC( NVerts, Pos, vertColors );
    }

	if ( Color )
		SMEM_StackPopToMarker();
}

//=========================================================================


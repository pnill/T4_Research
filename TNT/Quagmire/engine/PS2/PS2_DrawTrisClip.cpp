//=========================================================================
//
//  PS2_DRAWTRISCLIP
//
//  Included by PS2_Draw
//
//=========================================================================

#define CLIPFLAG_ALL_XMIN   (CLIPFLAG_XMIN|(CLIPFLAG_XMIN<<6)|(CLIPFLAG_XMIN<<12))
#define CLIPFLAG_ALL_XMAX   (CLIPFLAG_XMAX|(CLIPFLAG_XMAX<<6)|(CLIPFLAG_XMAX<<12))
#define CLIPFLAG_ALL_YMIN   (CLIPFLAG_YMIN|(CLIPFLAG_YMIN<<6)|(CLIPFLAG_YMIN<<12))
#define CLIPFLAG_ALL_YMAX   (CLIPFLAG_YMAX|(CLIPFLAG_YMAX<<6)|(CLIPFLAG_YMAX<<12))
#define CLIPFLAG_ALL_ZMIN   (CLIPFLAG_ZMIN|(CLIPFLAG_ZMIN<<6)|(CLIPFLAG_ZMIN<<12))
#define CLIPFLAG_ALL_ZMAX   (CLIPFLAG_ZMAX|(CLIPFLAG_ZMAX<<6)|(CLIPFLAG_ZMAX<<12))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//==============================================================================
//  Texture-Color Triangle Clipping
//==============================================================================
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    f32 U,V,P0,P1;
    s32 R,G,B,A;
    f32 X,Y,Z,W;
} clipvert_tc;

static clipvert_tc  s_TCClipBuffer[32];
static clipvert_tc* s_TCClipSrc;
static clipvert_tc* s_TCClipDst;
static s32          s_TCNVerts;


//==============================================================================
//  MACRO: CLIP_TC_LOOP_MX( param )
//
//==============================================================================
#define CLIP_TC_LOOP_MX(param)                                  \
{                                                               \
    s32          NVs;                                           \
    xbool        CIN,NIN;                                       \
    clipvert_tc* pC;                                            \
    clipvert_tc* pN;                                            \
    clipvert_tc* pV;                                            \
    f32          T;                                             \
    NVs = 0;                                                    \
    pC  = s_TCClipSrc;                                          \
    CIN = (pC->param < pC->W);                                  \
    for( i=0; i<s_TCNVerts; i++ )                               \
    {                                                           \
        pN = pC+1;                                              \
        NIN = (pN->param < pN->W);                              \
        if( CIN ) { s_TCClipDst[NVs] = *pC; NVs++; }            \
        if( CIN != NIN )                                        \
        {                                                       \
            pV = &s_TCClipDst[NVs];                             \
            NVs++;                                              \
            T = (pC->W - pC->param) /                           \
                ((pN->param-pC->param)-(pN->W-pC->W));          \
            TriClip_TC_Interpolate(pV, pC, pN, T );             \
        }                                                       \
        pC  = pN;                                               \
        CIN = NIN;                                              \
    }                                                           \
    s_TCClipDst[ NVs ] = s_TCClipDst[0];                        \
    {                                                           \
        clipvert_tc* TP;                                        \
        TP          = s_TCClipSrc;                              \
        s_TCClipSrc = s_TCClipDst;                              \
        s_TCClipDst = TP;                                       \
        s_TCNVerts  = NVs;                                      \
    }                                                           \
}


//==============================================================================
//  MACRO: CLIP_TC_LOOP_MN( param )
//
//==============================================================================
#define CLIP_TC_LOOP_MN(param)                                  \
{                                                               \
    s32          NVs;                                           \
    xbool        CIN,NIN;                                       \
    clipvert_tc* pC;                                            \
    clipvert_tc* pN;                                            \
    clipvert_tc* pV;                                            \
    f32          T;                                             \
    NVs = 0;                                                    \
    pC  = s_TCClipSrc;                                          \
    CIN = (pC->param > -pC->W);                                 \
    for( i=0; i<s_TCNVerts; i++ )                               \
    {                                                           \
        pN = pC+1;                                              \
        NIN = (pN->param > -pN->W);                             \
        if( CIN ) { s_TCClipDst[NVs] = *pC; NVs++; }            \
        if( CIN != NIN )                                        \
        {                                                       \
            pV = &s_TCClipDst[NVs];                             \
            NVs++;                                              \
            T = (-pC->W - pC->param) /                          \
                ((pN->param-pC->param)-(-pN->W+pC->W));         \
            TriClip_TC_Interpolate(pV, pC, pN, T );             \
        }                                                       \
        pC  = pN;                                               \
        CIN = NIN;                                              \
    }                                                           \
    s_TCClipDst[ NVs ] = s_TCClipDst[0];                        \
    {                                                           \
        clipvert_tc* TP;                                        \
        TP          = s_TCClipSrc;                              \
        s_TCClipSrc = s_TCClipDst;                              \
        s_TCClipDst = TP;                                       \
        s_TCNVerts  = NVs;                                      \
    }                                                           \
}


//==============================================================================
//  TriClip_TC_Interpolate()
//      Interpolates between two clip vertices
//==============================================================================
inline void TriClip_TC_Interpolate(clipvert_tc* pV, clipvert_tc* pC, clipvert_tc* pN, f32 T )
{
    pV->X = pC->X + T*(pN->X - pC->X);
    pV->Y = pC->Y + T*(pN->Y - pC->Y);
    pV->Z = pC->Z + T*(pN->Z - pC->Z);
    pV->W = pC->W + T*(pN->W - pC->W);
    pV->U = pC->U + T*(pN->U - pC->U);
    pV->V = pC->V + T*(pN->V - pC->V);
    pV->R = (s32)(pC->R + T*(pN->R - pC->R));
    pV->G = (s32)(pC->G + T*(pN->G - pC->G));
    pV->B = (s32)(pC->B + T*(pN->B - pC->B));
    pV->A = (s32)(pC->A + T*(pN->A - pC->A));
}


//==============================================================================
//  TriClipTC()
//      Triangle clip TC triangles
//==============================================================================
inline void TriClipTC( byte* pDest, u32 ClipFlags, s32& FinalNVerts )
{
    s32 i;

    FinalNVerts = 0;

    // Check if tri is completely clipped out
    if( (ClipFlags & CLIPFLAG_ALL_XMIN) == CLIPFLAG_ALL_XMIN ) return;
    if( (ClipFlags & CLIPFLAG_ALL_XMAX) == CLIPFLAG_ALL_XMAX ) return;
    if( (ClipFlags & CLIPFLAG_ALL_YMIN) == CLIPFLAG_ALL_YMIN ) return;
    if( (ClipFlags & CLIPFLAG_ALL_YMAX) == CLIPFLAG_ALL_YMAX ) return;
    if( (ClipFlags & CLIPFLAG_ALL_ZMIN) == CLIPFLAG_ALL_ZMIN ) return;
    if( (ClipFlags & CLIPFLAG_ALL_ZMAX) == CLIPFLAG_ALL_ZMAX ) return;

    // Load vert data into clip buffer
    s_TCClipSrc = &s_TCClipBuffer[0];
    s_TCClipDst = &s_TCClipBuffer[16];
    s_TCNVerts  = 3;
    s_TCClipSrc[0] = ((clipvert_tc*)pDest)[0];
    s_TCClipSrc[1] = ((clipvert_tc*)pDest)[1];
    s_TCClipSrc[2] = ((clipvert_tc*)pDest)[2];
    s_TCClipSrc[3] = ((clipvert_tc*)pDest)[0];

    if( ClipFlags & CLIPFLAG_ALL_ZMAX ) CLIP_TC_LOOP_MX(Z);
    if( ClipFlags & CLIPFLAG_ALL_ZMIN ) CLIP_TC_LOOP_MN(Z);
    if( ClipFlags & CLIPFLAG_ALL_XMAX ) CLIP_TC_LOOP_MX(X);
    if( ClipFlags & CLIPFLAG_ALL_XMIN ) CLIP_TC_LOOP_MN(X);
    if( ClipFlags & CLIPFLAG_ALL_YMAX ) CLIP_TC_LOOP_MX(Y);
    if( ClipFlags & CLIPFLAG_ALL_YMIN ) CLIP_TC_LOOP_MN(Y);

    // Copy verts from clipper
    FinalNVerts = 0;
    for( i=1; i<s_TCNVerts-1; i++ )
    {
        ((clipvert_tc*)pDest)[0] = s_TCClipSrc[0];
        ((clipvert_tc*)pDest)[1] = s_TCClipSrc[i+0];
        ((clipvert_tc*)pDest)[2] = s_TCClipSrc[i+1];
        pDest += sizeof(clipvert_tc)*3;
        FinalNVerts+=3;
    }
}


//==============================================================================
//  Build_TC_BackEnd()
//      Clips triangles when need and adds them to the display list
//==============================================================================
inline s32 Build_TC_BackEnd( byte*& pDest, u32 ClipFlags )
{
    s32 NClippedVerts = 3; 

    ///////////////////////////////////////////////////////////////////////////
    // Branch to clipper if needed
    ///////////////////////////////////////////////////////////////////////////
    if( ClipFlags & 0x0003FFFF )
    {
        TriClipTC(pDest,ClipFlags,NClippedVerts);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Finish transform of either 3 tri verts or clipped verts
    ///////////////////////////////////////////////////////////////////////////
    fvec4* TD = &((fvec4*)pDest)[0];    /* ptr to texture uvs */
    fvec4* PD = &((fvec4*)pDest)[2];    /* ptr to positions   */
    for( s32 j=0; j<NClippedVerts; j++ )
    {
        vector4 V;
        f32 Q;

        TransformV4( V, &s_VertexC2S, (vector4*)PD );

        /* Transform vertex from clipping space into screen space*/
        Q = 1.0f/V.W;


        /* Transform uvs */
        TD->X *= Q;
        TD->Y *= Q;
        TD->Z  = Q;
        TD->W  = Q;

        /* Store fixed, projected screen verts */
        Q *= 16.0f;

        V *= Q; 
        ((ivec4*)PD)->X = (s32)(V.X+0.5f);
        ((ivec4*)PD)->Y = (s32)(V.Y+0.5f);
        ((ivec4*)PD)->Z = (s32)(V.Z+0.5f);
        ((ivec4*)PD)->W = (s32)(V.W+0.5f);

        pDest += 3*16;
        PD    += 3;
        TD    += 3;
    }

    return NClippedVerts;
}


//==========================================================================
//  BuildTris_TC_C()
//      Builds a clipped triangles with a UV and color component
//==========================================================================
static void BuildTris_TC_C( void )
{
    s32         i,j;
    vector4     V;
    byte*       pDest       = s_pDest;
    s32         TotalNVerts = 0;
    s32         NTris       = s_Source_NVerts/3;
    u32         ClipFlags   = 0;

    vector3*    Pos         = s_Source_pPos;
    vector2*    UV          = s_Source_pUV;
    color*      Color       = s_Source_pColor;

    ASSERT( s_Source_pPos && s_Source_pUV );

    for( i=0; i<NTris; i++ )
    {
        // Build basic vert data and determine clipping
        byte* pD = pDest;
        for( j=0; j<3; j++ )
        {

            TransformV3( V, &s_VertexL2C, Pos );

            ///////////////////////////////////////////////////////////////////////////
            // Store position values
            ///////////////////////////////////////////////////////////////////////////
            ((fvec4*)pD)[2].X = V.X;
            ((fvec4*)pD)[2].Y = V.Y;
            ((fvec4*)pD)[2].Z = V.Z;
            ((fvec4*)pD)[2].W = V.W;

            ///////////////////////////////////////////////////////////////////////////
            // Setup clip flags
            ///////////////////////////////////////////////////////////////////////////
            ClipFlags <<= 6;
            if( V.X>+V.W ) ClipFlags |= CLIPFLAG_XMAX;
            if( V.X<-V.W ) ClipFlags |= CLIPFLAG_XMIN;
            if( V.Y>+V.W ) ClipFlags |= CLIPFLAG_YMAX;
            if( V.Y<-V.W ) ClipFlags |= CLIPFLAG_YMIN;
            if( V.Z>+V.W ) ClipFlags |= CLIPFLAG_ZMAX;
            if( V.Z<-V.W ) ClipFlags |= CLIPFLAG_ZMIN;

            ////////////////////////////////////////////////////////////////
            // Write out STQ
            ////////////////////////////////////////////////////////////////
            ((fvec4*)pD)[0].X = UV->X;
            ((fvec4*)pD)[0].Y = UV->Y;
            ((fvec4*)pD)[0].Z = 1.0f;
            ((fvec4*)pD)[0].W = 1.0f;

            ////////////////////////////////////////////////////////////////
            // Store colors
            ////////////////////////////////////////////////////////////////
            if ( Color )
            {
                ((ivec4*)pD)[1].X = MAKE_PS2_R(Color->R);
                ((ivec4*)pD)[1].Y = MAKE_PS2_G(Color->G);
                ((ivec4*)pD)[1].Z = MAKE_PS2_B(Color->B);
                ((ivec4*)pD)[1].W = MAKE_PS2_A(Color->A);
                Color++;
            }
            else
            {
                ((ivec4*)pD)[1].X = MAKE_PS2_R(255);
                ((ivec4*)pD)[1].Y = MAKE_PS2_G(255);
                ((ivec4*)pD)[1].Z = MAKE_PS2_B(255);
                ((ivec4*)pD)[1].W = MAKE_PS2_A(255);
            }

            Pos++;
            UV++;
            pD+=3*16;
        }

        ////////////////////////////////////////////////////////////////////
        //  Keep track of how many new verts added
        ////////////////////////////////////////////////////////////////////
        TotalNVerts += Build_TC_BackEnd( pDest, ClipFlags ) - 3;
    }

    ////////////////////////////////////////////////////////////////////////
    //  Update number of tris
    ////////////////////////////////////////////////////////////////////////
    s_Open_NVerts += TotalNVerts;


}

//=========================================================================

static
void BuildTris_TNC_C( void )
{
    s32         i,j;
    byte*       pDest       = s_pDest;
    s32         TotalNVerts = 0;
    s32         NTris       = s_Source_NVerts/3;
    u32         ClipFlags   = 0;
    vector4     V,I;
    vector4     RGB;
    s32         R,G,B;

    matrix4     LDR, LCR;

    LDR = s_LightDir;
    LCR = s_LightColor;

    vector3*    Pos         = s_Source_pPos;
    vector2*    UV          = s_Source_pUV;
    vector3*    Normal      = s_Source_pNormal;
    color*      Color       = s_Source_pColor;

    ASSERT( s_Source_pPos && s_Source_pUV && s_Source_pNormal );

    for( i=0; i<NTris; i++ )
    {
        // Build basic vert data and determine clipping
        byte* pD = pDest;
        for( j=0; j<3; j++ )
        {
            TransformV3( V, &s_VertexL2C, Pos );

            ///////////////////////////////////////////////////////////////////////////
            // Store position values
            ///////////////////////////////////////////////////////////////////////////
            ((fvec4*)pD)[2].X = V.X;
            ((fvec4*)pD)[2].Y = V.Y;
            ((fvec4*)pD)[2].Z = V.Z;
            ((fvec4*)pD)[2].W = V.W;

            ///////////////////////////////////////////////////////////////////////////
            // Setup clip flags
            ///////////////////////////////////////////////////////////////////////////
            ClipFlags <<= 6;
            if( V.X>+V.W ) ClipFlags |= CLIPFLAG_XMAX;
            if( V.X<-V.W ) ClipFlags |= CLIPFLAG_XMIN;
            if( V.Y>+V.W ) ClipFlags |= CLIPFLAG_YMAX;
            if( V.Y<-V.W ) ClipFlags |= CLIPFLAG_YMIN;
            if( V.Z>+V.W ) ClipFlags |= CLIPFLAG_ZMAX;
            if( V.Z<-V.W ) ClipFlags |= CLIPFLAG_ZMIN;

            ////////////////////////////////////////////////////////////////
            // Write out STQ
            ////////////////////////////////////////////////////////////////
            ((fvec4*)pD)[0].X = UV->X;
            ((fvec4*)pD)[0].Y = UV->Y;
            ((fvec4*)pD)[0].Z = 1.0f;
            ((fvec4*)pD)[0].W = 1.0f;

            ////////////////////////////////////////////////////////////////
            // Build colors
            ////////////////////////////////////////////////////////////////
            TransformV3( I, &LDR, Normal );
            if(I.X < 0.0f ) I.X = 0.0f;
            if(I.Y < 0.0f ) I.Y = 0.0f;
            if(I.Z < 0.0f ) I.Z = 0.0f;
            I.W = 1.0f;

            TransformV4( RGB, &LCR, &I );
            R = (s32)RGB.X;
            G = (s32)RGB.Y;
            B = (s32)RGB.Z;
            if(R>127) R=127;
            if(G>127) G=127;
            if(B>127) B=127;

            if ( Color )
            {
                R = (R*MAKE_PS2_R(Color->R))>>7;
                G = (G*MAKE_PS2_G(Color->G))>>7;
                B = (B*MAKE_PS2_B(Color->B))>>7;
                Color++;
            }

            ////////////////////////////////////////////////////////////////
            // Store colors
            ////////////////////////////////////////////////////////////////
            ((ivec4*)pD)[1].X = R;
            ((ivec4*)pD)[1].Y = G;
            ((ivec4*)pD)[1].Z = B;
            ((ivec4*)pD)[1].W = 127;

            Pos++;
            UV++;
            pD+=3*16;
        }

        TotalNVerts += Build_TC_BackEnd( pDest, ClipFlags ) - 3;
    }

    ////////////////////////////////////////////////////////////////////////
    //  Update number of tris
    ////////////////////////////////////////////////////////////////////////
    s_Open_NVerts += TotalNVerts;


}















///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//==============================================================================
//  Color Triangle Clipping
//==============================================================================
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    s32 R,G,B,A;
    f32 X,Y,Z,W;
} clipvert_c;

static clipvert_c  s_CClipBuffer[32];
static clipvert_c* s_CClipSrc;
static clipvert_c* s_CClipDst;
static s32         s_CNVerts;


//==============================================================================
//  MACRO: CLIP_TC_LOOP_MX( param )
//
//==============================================================================
#define CLIP_C_LOOP_MX(param)                                   \
{                                                               \
    s32          NVs;                                           \
    xbool        CIN,NIN;                                       \
    clipvert_c*  pC;                                            \
    clipvert_c*  pN;                                            \
    clipvert_c*  pV;                                            \
    f32          T;                                             \
    NVs = 0;                                                    \
    pC  = s_CClipSrc;                                           \
    CIN = (pC->param < pC->W);                                  \
    for( i=0; i<s_CNVerts; i++ )                                \
    {                                                           \
        pN = pC+1;                                              \
        NIN = (pN->param < pN->W);                              \
        if( CIN ) { s_CClipDst[NVs] = *pC; NVs++; }             \
        if( CIN != NIN )                                        \
        {                                                       \
            pV = &s_CClipDst[NVs];                              \
            NVs++;                                              \
            T = (pC->W - pC->param) /                           \
                ((pN->param-pC->param)-(pN->W-pC->W));          \
            TriClip_C_Interpolate(pV, pC, pN, T );              \
        }                                                       \
        pC  = pN;                                               \
        CIN = NIN;                                              \
    }                                                           \
    s_CClipDst[ NVs ] = s_CClipDst[0];                          \
    {                                                           \
        clipvert_c* TP;                                         \
        TP          = s_CClipSrc;                               \
        s_CClipSrc  = s_CClipDst;                               \
        s_CClipDst  = TP;                                       \
        s_CNVerts   = NVs;                                      \
    }                                                           \
}


//==============================================================================
//  MACRO: CLIP_TC_LOOP_MN( param )
//
//==============================================================================
#define CLIP_C_LOOP_MN(param)                                   \
{                                                               \
    s32          NVs;                                           \
    xbool        CIN,NIN;                                       \
    clipvert_c*  pC;                                            \
    clipvert_c*  pN;                                            \
    clipvert_c*  pV;                                            \
    f32          T;                                             \
    NVs = 0;                                                    \
    pC  = s_CClipSrc;                                           \
    CIN = (pC->param > -pC->W);                                 \
    for( i=0; i<s_CNVerts; i++ )                                \
    {                                                           \
        pN = pC+1;                                              \
        NIN = (pN->param > -pN->W);                             \
        if( CIN ) { s_CClipDst[NVs] = *pC; NVs++; }             \
        if( CIN != NIN )                                        \
        {                                                       \
            pV = &s_CClipDst[NVs];                              \
            NVs++;                                              \
            T = (-pC->W - pC->param) /                          \
                ((pN->param-pC->param)-(-pN->W+pC->W));         \
            TriClip_C_Interpolate(pV, pC, pN, T );              \
        }                                                       \
        pC  = pN;                                               \
        CIN = NIN;                                              \
    }                                                           \
    s_CClipDst[ NVs ] = s_CClipDst[0];                          \
    {                                                           \
        clipvert_c* TP;                                         \
        TP          = s_CClipSrc;                               \
        s_CClipSrc = s_CClipDst;                                \
        s_CClipDst = TP;                                        \
        s_CNVerts  = NVs;                                       \
    }                                                           \
}


//==============================================================================
//  TriClip_C_Interpolate()
//      Interpolates between two clip vertices
//==============================================================================
inline void TriClip_C_Interpolate(clipvert_c* pV, clipvert_c* pC, clipvert_c* pN, f32 T )
{
    pV->X = pC->X + T*(pN->X - pC->X);
    pV->Y = pC->Y + T*(pN->Y - pC->Y);
    pV->Z = pC->Z + T*(pN->Z - pC->Z);
    pV->W = pC->W + T*(pN->W - pC->W);
    pV->R = (s32)(pC->R + T*(pN->R - pC->R));
    pV->G = (s32)(pC->G + T*(pN->G - pC->G));
    pV->B = (s32)(pC->B + T*(pN->B - pC->B));
    pV->A = (s32)(pC->A + T*(pN->A - pC->A));
}


//==============================================================================
//  TriClipC()
//      Triangle clip TC triangles
//==============================================================================
inline void TriClipC( byte* pDest, u32 ClipFlags, s32& FinalNVerts )
{
    s32 i;

    FinalNVerts = 0;

    // Check if tri is completely clipped out
    if( (ClipFlags & CLIPFLAG_ALL_XMIN) == CLIPFLAG_ALL_XMIN ) return;
    if( (ClipFlags & CLIPFLAG_ALL_XMAX) == CLIPFLAG_ALL_XMAX ) return;
    if( (ClipFlags & CLIPFLAG_ALL_YMIN) == CLIPFLAG_ALL_YMIN ) return;
    if( (ClipFlags & CLIPFLAG_ALL_YMAX) == CLIPFLAG_ALL_YMAX ) return;
    if( (ClipFlags & CLIPFLAG_ALL_ZMIN) == CLIPFLAG_ALL_ZMIN ) return;
    if( (ClipFlags & CLIPFLAG_ALL_ZMAX) == CLIPFLAG_ALL_ZMAX ) return;

    // Load vert data into clip buffer
    s_CClipSrc = &s_CClipBuffer[0];
    s_CClipDst = &s_CClipBuffer[16];
    s_CNVerts  = 3;
    s_CClipSrc[0] = ((clipvert_c*)pDest)[0];
    s_CClipSrc[1] = ((clipvert_c*)pDest)[1];
    s_CClipSrc[2] = ((clipvert_c*)pDest)[2];
    s_CClipSrc[3] = ((clipvert_c*)pDest)[0];

    if( ClipFlags & CLIPFLAG_ALL_ZMAX ) CLIP_C_LOOP_MX(Z);
    if( ClipFlags & CLIPFLAG_ALL_ZMIN ) CLIP_C_LOOP_MN(Z);
    if( ClipFlags & CLIPFLAG_ALL_XMAX ) CLIP_C_LOOP_MX(X);
    if( ClipFlags & CLIPFLAG_ALL_XMIN ) CLIP_C_LOOP_MN(X);
    if( ClipFlags & CLIPFLAG_ALL_YMAX ) CLIP_C_LOOP_MX(Y);
    if( ClipFlags & CLIPFLAG_ALL_YMIN ) CLIP_C_LOOP_MN(Y);

    // Copy verts from clipper
    FinalNVerts = 0;
    for( i=1; i<s_CNVerts-1; i++ )
    {
        ((clipvert_c*)pDest)[0] = s_CClipSrc[0];
        ((clipvert_c*)pDest)[1] = s_CClipSrc[i+0];
        ((clipvert_c*)pDest)[2] = s_CClipSrc[i+1];
        pDest += sizeof(clipvert_c)*3;
        FinalNVerts+=3;
    }
}


//==============================================================================
//  Build_C_BackEnd()
//      Clips triangles when need and adds them to the display list
//==============================================================================
inline s32 Build_C_BackEnd( byte*& pDest, u32 ClipFlags )
{
    s32 NClippedVerts = 3; 

    ///////////////////////////////////////////////////////////////////////////
    // Branch to clipper if needed
    ///////////////////////////////////////////////////////////////////////////
    if( ClipFlags & 0x0003FFFF )
    {
        TriClipC(pDest,ClipFlags,NClippedVerts);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Finish transform of either 3 tri verts or clipped verts
    ///////////////////////////////////////////////////////////////////////////
    fvec4* PD = &((fvec4*)(pDest))[1];    /* ptr to positions   */
    for( s32 j=0; j<NClippedVerts; j++ )
    {
        vector4 V;
        f32 Q;

        TransformV4( V, &s_VertexC2S, (vector4*)PD );

        /* Transform vertex from clipping space into screen space*/
        Q = 1.0f/V.W;

        /* Store fixed, projected screen verts */
        Q *= 16.0f;

        V *= Q; 
        ((ivec4*)PD)->X = (s32)(V.X + 0.5f);
        ((ivec4*)PD)->Y = (s32)(V.Y + 0.5f);
        ((ivec4*)PD)->Z = (s32)(V.Z + 0.5f);
        ((ivec4*)PD)->W = (s32)(V.W + 0.5f);

        pDest += 2*16;
        PD    += 2;
    }

    return NClippedVerts;
}


//==========================================================================
//  BuildTris_C_C()
//      Builds a clipped triangles with a UV and color component
//==========================================================================
static void BuildTris_C_C( void )
{
    s32         i,j;
    vector4     V;
    byte*       pDest       = s_pDest;
    s32         TotalNVerts = 0;
    s32         NTris       = s_Source_NVerts/3;
    u32         ClipFlags   = 0;

    vector3*    Pos         = s_Source_pPos;
    color*      Color       = s_Source_pColor;

    ASSERT( s_Source_pPos );

    for( i=0; i<NTris; i++ )
    {
        // Build basic vert data and determine clipping
        byte* pD = pDest;
        for( j=0; j<3; j++ )
        {

            TransformV3( V, &s_VertexL2C, Pos );

            ///////////////////////////////////////////////////////////////////////////
            // Store position values
            ///////////////////////////////////////////////////////////////////////////
            ((fvec4*)pD)[1].X = V.X;
            ((fvec4*)pD)[1].Y = V.Y;
            ((fvec4*)pD)[1].Z = V.Z;
            ((fvec4*)pD)[1].W = V.W;

            ///////////////////////////////////////////////////////////////////////////
            // Setup clip flags
            ///////////////////////////////////////////////////////////////////////////
            ClipFlags <<= 6;
            if( V.X>+V.W ) ClipFlags |= CLIPFLAG_XMAX;
            if( V.X<-V.W ) ClipFlags |= CLIPFLAG_XMIN;
            if( V.Y>+V.W ) ClipFlags |= CLIPFLAG_YMAX;
            if( V.Y<-V.W ) ClipFlags |= CLIPFLAG_YMIN;
            if( V.Z>+V.W ) ClipFlags |= CLIPFLAG_ZMAX;
            if( V.Z<-V.W ) ClipFlags |= CLIPFLAG_ZMIN;

            ////////////////////////////////////////////////////////////////
            // Store colors
            ////////////////////////////////////////////////////////////////
            if ( Color )
            {
                ((ivec4*)pD)[0].X = MAKE_PS2_R(Color->R);
                ((ivec4*)pD)[0].Y = MAKE_PS2_G(Color->G);
                ((ivec4*)pD)[0].Z = MAKE_PS2_B(Color->B);
                ((ivec4*)pD)[0].W = MAKE_PS2_A(Color->A);
                Color++;
            }
            else
            {
                ((ivec4*)pD)[0].X = MAKE_PS2_R(255);
                ((ivec4*)pD)[0].Y = MAKE_PS2_G(255);
                ((ivec4*)pD)[0].Z = MAKE_PS2_B(255);
                ((ivec4*)pD)[0].W = MAKE_PS2_A(255);
            }

            Pos++;
            pD+=2*16;
        }

        ////////////////////////////////////////////////////////////////////
        //  Keep track of how many new verts added
        ////////////////////////////////////////////////////////////////////
        TotalNVerts += Build_C_BackEnd( pDest, ClipFlags ) - 3;
    }

    ////////////////////////////////////////////////////////////////////////
    //  Update number of tris
    ////////////////////////////////////////////////////////////////////////
    s_Open_NVerts += TotalNVerts;

}

//=========================================================================

static
void BuildTris_NC_C( void )
{
    s32         i,j;
    byte*       pDest       = s_pDest;
    s32         TotalNVerts = 0;
    s32         NTris       = s_Source_NVerts/3;
    u32         ClipFlags   = 0;
    vector4     V, I;
    vector4     RGB;
    s32         R,G,B;

    matrix4     LDR, LCR;

    LDR = s_LightDir;
    LCR = s_LightColor;

    vector3*    Pos         = s_Source_pPos;
    vector3*    Normal      = s_Source_pNormal;
    color*      Color       = s_Source_pColor;

    ASSERT( s_Source_pPos && s_Source_pNormal );

    for( i=0; i<NTris; i++ )
    {
        // Build basic vert data and determine clipping
        byte* pD = pDest;
        for( j=0; j<3; j++ )
        {
            TransformV3( V, &s_VertexL2C, Pos );

            ///////////////////////////////////////////////////////////////////////////
            // Store position values
            ///////////////////////////////////////////////////////////////////////////
            ((fvec4*)pD)[1].X = V.X;
            ((fvec4*)pD)[1].Y = V.Y;
            ((fvec4*)pD)[1].Z = V.Z;
            ((fvec4*)pD)[1].W = V.W;

            ///////////////////////////////////////////////////////////////////////////
            // Setup clip flags
            ///////////////////////////////////////////////////////////////////////////
            ClipFlags <<= 6;
            if( V.X>+V.W ) ClipFlags |= CLIPFLAG_XMAX;
            if( V.X<-V.W ) ClipFlags |= CLIPFLAG_XMIN;
            if( V.Y>+V.W ) ClipFlags |= CLIPFLAG_YMAX;
            if( V.Y<-V.W ) ClipFlags |= CLIPFLAG_YMIN;
            if( V.Z>+V.W ) ClipFlags |= CLIPFLAG_ZMAX;
            if( V.Z<-V.W ) ClipFlags |= CLIPFLAG_ZMIN;

            ////////////////////////////////////////////////////////////////
            // Build colors
            ////////////////////////////////////////////////////////////////
            TransformV3( I, &LDR, Normal );
            if(I.X < 0.0f ) I.X = 0.0f;
            if(I.Y < 0.0f ) I.Y = 0.0f;
            if(I.Z < 0.0f ) I.Z = 0.0f;
            I.W = 1.0f;

            TransformV4( RGB, &LCR, &I );
            R = (s32)RGB.X;
            G = (s32)RGB.Y;
            B = (s32)RGB.Z;
            if(R>127) R=127;
            if(G>127) G=127;
            if(B>127) B=127;

            if ( Color )
            {
                R = (R*MAKE_PS2_R(Color->R))>>7;
                G = (G*MAKE_PS2_G(Color->G))>>7;
                B = (B*MAKE_PS2_B(Color->B))>>7;
                Color++;
            }

            ////////////////////////////////////////////////////////////////
            // Store colors
            ////////////////////////////////////////////////////////////////
            ((ivec4*)pD)[0].X = R;
            ((ivec4*)pD)[0].Y = G;
            ((ivec4*)pD)[0].Z = B;
            ((ivec4*)pD)[0].W = 127;

            Pos++;
            pD+=2*16;
        }

        TotalNVerts += Build_C_BackEnd( pDest, ClipFlags ) - 3;
    }

    ////////////////////////////////////////////////////////////////////////
    //  Update number of tris
    ////////////////////////////////////////////////////////////////////////
    s_Open_NVerts += TotalNVerts;


}

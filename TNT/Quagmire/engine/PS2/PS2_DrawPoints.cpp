//=========================================================================
//
//  PS2_DRAWPOINTS.CPP
//
//  Included by PS2_Draw
//
//=========================================================================

static sceGifTag    GIFTAG_PointsC; // Just colors

//=========================================================================

static
void PointsTransform_C_NC( s32 NVerts, void* Dest, vector3* Pos, color* Color )
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
            ((ivec4*)pV)[0].X = (Color->R<<1)/3;
            ((ivec4*)pV)[0].Y = (Color->G<<1)/3;
            ((ivec4*)pV)[0].Z = (Color->B<<1)/3;
            ((ivec4*)pV)[0].W = Color->A>>1;
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

//=========================================================================

static
void InitPoints( void )
{

    VIFHELP_BuildGifTag1(  &GIFTAG_PointsC, VIFHELP_GIFMODE_PACKED, 
                           2, 0, TRUE, 
                           VIFHELP_GIFPRIMTYPE_POINT,
                           0, 
                           TRUE );
    
    VIFHELP_BuildGifTag2(  &GIFTAG_PointsC, 
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
} point_header;
                          
static          
void BuildPointHeader( point_header* pHeader, s32 NVerts, s32 SizeOfVert, sceGifTag* GT )
{
    s32 DMASize;
    s32 TotalVertSize;

    TotalVertSize = NVerts*SizeOfVert;
    DMASize = sizeof(point_header) - sizeof(sceDmaTag) + TotalVertSize;
    ASSERT( (DMASize&0x0F) == 0 );
    DMAHELP_BuildTagCont( &pHeader->DMA, DMASize );
    
    pHeader->VIF[0] = 0;
    pHeader->VIF[1] = 0;
    pHeader->VIF[2] = 0;
    pHeader->VIF[3] = SCE_VIF1_SET_DIRECT( 1+(TotalVertSize>>4), 0 );

    pHeader->GIF = *GT;
    pHeader->GIF.NLOOP = NVerts;
}

//=========================================================================

static
void BuildPoints_C_NC( s32 NVerts, vector3* Pos, color* Color )
{
    point_header* pHeader;
    byte*       pDest;
//    matrix4     L2S;

    ASSERT( Pos );

    // Compute size of output and get dlist ptr
    pDest = pDList;
    pDList += sizeof(point_header) + 2*16*NVerts;

    // Build Header
    pHeader = (point_header*)pDest;
    pDest += sizeof(point_header);
    BuildPointHeader( pHeader, NVerts, 2*16, &GIFTAG_PointsC );
    
    PointsTransform_C_NC( NVerts, pDest, Pos, Color );

    // Keep our stats current
    if ( s_pStatNVerts )
        *s_pStatNVerts = *s_pStatNVerts + NVerts;
    if ( s_pStatNTris )
        *s_pStatNTris = *s_pStatNTris + 0;
    if ( s_pStatNBytes )
        *s_pStatNBytes = *s_pStatNBytes + sizeof(point_header) + 2*16*NVerts;
}

//=========================================================================

void DRAW_Points( s32       NVerts, 
                  vector3*  Pos, 
                  color*    Color)
{
    ASSERT( Pos );
    ASSERT( ENG_GetRenderMode() );
    ASSERT( s_DrawMode & DRAW_3D );
    BuildPoints_C_NC( NVerts, Pos, Color );
}

//=========================================================================


//==========================================================================
//
//  PS2_DRAWTRANS.CPP
//
//  Included by PS2_Draw
//
//==========================================================================


//==============================================================================
//  DEFINES
//==============================================================================

// Toggle this define on to use VU0 as a co-processor, which will speed up
// the transform times.
#define USE_VU0_AS_COPROC

// Color macros to correct color values for the PS2
#define MAKE_PS2_R(R)   ((R << 1) / 3)
#define MAKE_PS2_G(G)   ((G << 1) / 3)
#define MAKE_PS2_B(B)   ((B << 1) / 3)
#define MAKE_PS2_A(A)   (A >> 1)


// Position functions to correct vertex positions for PS2 and PS2 engine settings
inline s32 MakePS2X( f32 X )
{
    return ((s32)(X * 16.0f + 0.5f) + ((2048 - ENG_GetScreenWidth()/2)<<4));
}

inline s32 MakePS2Y( f32 Y )
{
    return ((s32)(Y * 16.0f + 0.5f) + ((2048 - ENG_GetScreenHeight()/2)<<4));
}

inline s32 MakePS2Z( f32 Z )
{
    return ((s32)(Z * 65536.0f + 0.5f));
}

inline s32 MakePS2W( f32 W )
{
    return (((s32)W) << 4 );
}


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

//==============================================================================
//  TransformV4()
//      Transforms a vector4 using a matrix
//==============================================================================
inline void TransformV4( vector4& dest, matrix4* M, vector4* V )
{
    vector4 V4( V->X, V->Y, V->Z, V->W );//__attribute__ ((aligned(16)));
    matrix4 M4(*M) ;//__attribute__ ((aligned(16)));

#ifdef USE_VU0_AS_COPROC
    asm __volatile__
    ("

    LQC2                    vf8, 0x00(%0)       # vf8 = vector
    LQC2                    vf7,    0x30(%1)    # load matrix col 3
    LQC2                    vf6,    0x20(%1)    # load matrix col 2
    LQC2                    vf5,    0x10(%1)    # load matrix col 1
    LQC2                    vf4,    0x00(%1)    # load matrix col 0
    VMULAw.xyzw     ACC,    vf7,    vf8w        # transform vert
    VMADDAz.xyzw    ACC,    vf6,    vf8z        # transform vert
    VMADDAy.xyzw    ACC,    vf5,    vf8y        # transform vert
    VMADDx.xyzw     vf9,    vf4,    vf8x        # transform vert
    SQC2    vf9, 0x00(%0)                       # store vector

    ": : "r" (&V4) , "r" (&M4.M[0][0]) : "memory" );

#else
    X = M->M[0][0]*V4.X +  M->M[1][0]*V4.Y + M->M[2][0]*V4.Z + M->M[3][0]*V4.W;
    Y = M->M[0][1]*V4.X +  M->M[1][1]*V4.Y + M->M[2][1]*V4.Z + M->M[3][1]*V4.W;
    Z = M->M[0][2]*V4.X +  M->M[1][2]*V4.Y + M->M[2][2]*V4.Z + M->M[3][2]*V4.W;
    W = M->M[0][3]*V4.X +  M->M[1][3]*V4.Y + M->M[2][3]*V4.Z + M->M[3][3]*V4.W;
#endif

    dest = V4;
}


//==============================================================================
//  TransformV3()
//      Transforms a vector3 using a matrix
//==============================================================================
inline void TransformV3( vector4& dest, matrix4* M, vector3* V )
{
    vector4 V4( V->X, V->Y, V->Z, 1.0f ); //__attribute__ ((aligned(16)));
    matrix4 M4(*M); //__attribute__ ((aligned(16)));

#ifdef USE_VU0_AS_COPROC
    asm __volatile__
    ("
    
    LQC2                    vf8,    0x00(%0)    # vf8 = vector
    LQC2                    vf7,    0x30(%1)    # load matrix col 3
    LQC2                    vf6,    0x20(%1)    # load matrix col 2
    LQC2                    vf5,    0x10(%1)    # load matrix col 1
    LQC2                    vf4,    0x00(%1)    # load matrix col 0
    VMULAw.xyzw     ACC,    vf7,    vf0w        # transform vert
    VMADDAz.xyzw    ACC,    vf6,    vf8z        # transform vert
    VMADDAy.xyzw    ACC,    vf5,    vf8y        # transform vert
    VMADDx.xyzw     vf9,    vf4,    vf8x        # transform vert
    SQC2    vf9, 0x00(%0)                       # store vector

    ": : "r" (&V4) , "r" (&M4.M[0][0]) : "memory" );
#else
    X = M->M[0][0]*V4.X +  M->M[1][0]*V4.Y + M->M[2][0]*V4.Z + M->M[3][0]*V4.W;
    Y = M->M[0][1]*V4.X +  M->M[1][1]*V4.Y + M->M[2][1]*V4.Z + M->M[3][1]*V4.W;
    Z = M->M[0][2]*V4.X +  M->M[1][2]*V4.Y + M->M[2][2]*V4.Z + M->M[3][2]*V4.W;
    W = M->M[0][3]*V4.X +  M->M[1][3]*V4.Y + M->M[2][3]*V4.Z + M->M[3][3]*V4.W;
#endif

    dest = V4;
}

//==========================================================================
//
//
//==========================================================================
inline void Build_TC_POS_UV( vector4& V, ivec4* Pos, fvec4* UV )
{

#ifdef USE_VU0_AS_COPROC

    asm __volatile__
    ("
       VDIV Q, vf00w, vf9w                  # Q = 1.0f / W
       LQC2    vf13, 0x00(%1)               # load the UV
       VNOP                                 # NOP
       VNOP                                 # NOP
       VNOP                                 # NOP
       VNOP                                 # NOP
       VNOP                                 # NOP
       VMULq.xyzw vf13, vf13, Q             # UV *= Q
       VMULq.xyzw vf9, vf9, Q               # POS *= Q
       VFTOI4.xyzw vf9, vf9                 # convert to fixed point

       SQC2    vf9, 0x00(%0)                # store result in POS
       SQC2    vf13, 0x00(%1)               # store result in UV

    ": "+r" (Pos), "+r" (UV) : );

#else
        f32 Q = 1.0f/V.W;

        // Write out STQ
        UV->X *= Q;
        UV->Y *= Q;
        UV->Z =  Q;
        UV->W =  Q;

        // Write out fixed, projected version
        Q *= 16.0f;
        Pos->X = (s32)(V.X*Q);
        Pos->Y = (s32)(V.Y*Q);
        Pos->Z = (s32)(V.Z*Q);
        Pos->W = (s32)(V.W*Q);
#endif

}

//=============================================================================
//  Transform_C_NC()
//      Transforms non clipped triangles with color component
//==============================================================================
static
void Transform_C_NC( s32 NVerts, void* Dest, vector3* Pos, color* Color )
{
//    matrix4     L2S;
    vector4     V;
    s32         i;
    ivec4*      pV;
    f32         Q;  

    // Transform into dest
    pV = (ivec4*)Dest;
    for( i=0; i<NVerts; i++ )
    {
        // Store colors
        if ( Color )
        {
            ((ivec4*)pV)[0].X = MAKE_PS2_R(Color->R);
            ((ivec4*)pV)[0].Y = MAKE_PS2_G(Color->G);
            ((ivec4*)pV)[0].Z = MAKE_PS2_B(Color->B);
            ((ivec4*)pV)[0].W = MAKE_PS2_A(Color->A);
            Color++;
        }
        else
        {
            ((ivec4*)pV)[0].X = MAKE_PS2_R(255);
            ((ivec4*)pV)[0].Y = MAKE_PS2_G(255);
            ((ivec4*)pV)[0].Z = MAKE_PS2_B(255);
            ((ivec4*)pV)[0].W = MAKE_PS2_A(255);
        }

        // Transform vertex and compute Q
        TransformV3( V, &s_VertexL2S, Pos );
        Q = 16.0f/V.W;

        V *= Q;

        // Write out fixed, projected version
        pV[1].X = (s32)(V.X);
        pV[1].Y = (s32)(V.Y);
        pV[1].Z = (s32)(V.Z);
        pV[1].W = (s32)(V.W);
        Pos++;

        pV+=2;
    }
}


//=============================================================================
//  Transform_TC_NC()
//      Transforms non clipped triangles with UV and color component
//==============================================================================
static
void Transform_TC_NC( s32 NVerts, void* Dest, vector3* Pos, vector2* UV, color* Color )
{
//    matrix4     L2S;
    vector4     V;
    s32         i;
    ivec4*      pV;

    // Transform into dest
    pV = (ivec4*)Dest;
    for( i=0; i<NVerts; i++ )
    {
        // Store colors
        if ( Color )
        {
            ((ivec4*)pV)[1].X = MAKE_PS2_R(Color->R);
            ((ivec4*)pV)[1].Y = MAKE_PS2_G(Color->G);
            ((ivec4*)pV)[1].Z = MAKE_PS2_B(Color->B);
            ((ivec4*)pV)[1].W = MAKE_PS2_A(Color->A); 
            Color++;
        }
        else
        {
            ((ivec4*)pV)[1].X = MAKE_PS2_R(255);
            ((ivec4*)pV)[1].Y = MAKE_PS2_G(255);
            ((ivec4*)pV)[1].Z = MAKE_PS2_B(255);
            ((ivec4*)pV)[1].W = MAKE_PS2_A(255);
        }

        // Transform vertex and compute Q
        TransformV3( V, &s_VertexL2S, Pos );

        // Write out STQ
        ((fvec4*)pV)[0].X = UV->X;
        ((fvec4*)pV)[0].Y = UV->Y;
        ((fvec4*)pV)[0].Z = 1;
        ((fvec4*)pV)[0].W = 1;


        Build_TC_POS_UV( V, &(pV[2]), (fvec4*)&(pV[0]) );

        UV++;
        Pos++;
        pV+=3;
    }
}


//=============================================================================
//  Transform_TNC_NC()
//      Transforms non clipped triangles with UV, normal, and color component
//==============================================================================
static
void Transform_TNC_NC( s32 NVerts, void* Dest, vector3* Pos, vector2* UV, vector3* Normal, color* Color )
{
//    matrix4     L2S;
//    matrix4     LDR;
//    matrix4     LCR;
    vector4     V;
    vector4     I;
    vector4     RGB;
    f32         Q;
    s32         i;
    ivec4*      pV;
    s32         R,G,B;

    // Transform into dest
    pV = (ivec4*)Dest;
    for( i=0; i<NVerts; i++ )
    {
        // Build colors
        TransformV3( I, &s_LightDir, Normal );
        if(I.X < 0.0f ) I.X = 0.0f;
        if(I.Y < 0.0f ) I.Y = 0.0f;
        if(I.Z < 0.0f ) I.Z = 0.0f;
        I.W = 1.0f;

        TransformV4( RGB, &s_LightColor, &I );
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

        // Store colors
        ((ivec4*)pV)[1].X = R;
        ((ivec4*)pV)[1].Y = G;
        ((ivec4*)pV)[1].Z = B;
        ((ivec4*)pV)[1].W = 127;

        // Transform vertex and compute Q
        TransformV3( V, &s_VertexL2S, Pos );

        Q = 1.0f/V.W;

        // Write out STQ
        ((fvec4*)pV)[0].X = UV->X * Q;
        ((fvec4*)pV)[0].Y = UV->Y * Q;
        ((fvec4*)pV)[0].Z = Q;
        ((fvec4*)pV)[0].W = Q;
        UV++;

        // Write out fixed, projected version
        Q *= 16.0f;
        pV[2].X = (s32)(V.X*Q);
        pV[2].Y = (s32)(V.Y*Q);
        pV[2].Z = (s32)(V.Z*Q);
        pV[2].W = (s32)(V.W*Q);

        Pos++;
        Normal++;

        pV+=3;
    }
}



//=============================================================================
//  Transform_NC_NC()
//      Transforms non clipped triangles with normal and color component
//==============================================================================
static
void Transform_NC_NC( s32 NVerts, void* Dest, vector3* Pos, vector3* Normal, color* Color )
{
//    matrix4     L2S;
//    matrix4     LDR;
//    matrix4     LCR;
    vector4     V;
    vector4     I;
    vector4     RGB;
    f32         Q;
    s32         i;
    ivec4*      pV;
    s32         R,G,B;

    // Transform into dest
    pV = (ivec4*)Dest;
    for( i=0; i<NVerts; i++ )
    {
        // Build colors
        TransformV3( I, &s_LightDir, Normal );
        if(I.X < 0.0f ) I.X = 0.0f;
        if(I.Y < 0.0f ) I.Y = 0.0f;
        if(I.Z < 0.0f ) I.Z = 0.0f;
        I.W = 1.0f;

        TransformV4( RGB, &s_LightColor, &I );
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

        // Store colors
        ((ivec4*)pV)[1].X = R;
        ((ivec4*)pV)[1].Y = G;
        ((ivec4*)pV)[1].Z = B;
        ((ivec4*)pV)[1].W = 127;

        // Transform vertex and compute Q
        TransformV3( V, &s_VertexL2S, Pos );

        Q = 16.0f/V.W;

        // Write out fixed, projected version
        pV[2].X = (s32)(V.X*Q);
        pV[2].Y = (s32)(V.Y*Q);
        pV[2].Z = (s32)(V.Z*Q);
        pV[2].W = (s32)(V.W*Q);

        Pos++;
        Normal++;

        pV+=2;
    }
}



//=============================================================================
//  Transform2D_TC_NC()
//      Transforms non clipped 2D triangles with UV and color component
//==============================================================================
static
void Transform2D_TC_NC( s32 NVerts, void* Dest, vector3* Pos, vector2* UV, color* Color )
{
    s32         i;
    ivec4*      pV;

    // Transform into dest
    pV = (ivec4*)Dest;
    for( i=0; i<NVerts; i++ )
    {
        // Store UVs
        pV[0].X = (s32)(UV->X * s_pActiveTexture->GetWidth() * 16.0f);
        pV[0].Y = (s32)(UV->Y * s_pActiveTexture->GetHeight() * 16.0f);
        pV[0].Z = 1;
        pV[0].W = 1;
        UV++;

        // Store colors
        if ( Color )
        {
            pV[1].X = MAKE_PS2_R(Color->R);
            pV[1].Y = MAKE_PS2_G(Color->G);
            pV[1].Z = MAKE_PS2_B(Color->B);
            pV[1].W = MAKE_PS2_A(Color->A);
            Color++;
        }
        else
        {
            pV[1].X = MAKE_PS2_R(255);
            pV[1].Y = MAKE_PS2_G(255);
            pV[1].Z = MAKE_PS2_B(255);
            pV[1].W = MAKE_PS2_A(255);
        }

        // Store out the coordinates
        pV[2].X = MakePS2X(Pos->X);
        pV[2].Y = MakePS2Y(Pos->Y);
        pV[2].Z = MakePS2Z(Pos->Z);
        pV[2].W = MakePS2W(1);
        Pos++;
        
        pV += 3;
    }
}



//=============================================================================
//  Transform2D_C_NC()
//      Transforms non clipped 2D triangles with color component
//==============================================================================
static
void Transform2D_C_NC( s32 NVerts, void* Dest, vector3* Pos, color* Color )
{
    s32         i;
    ivec4*      pV;

    // Transform into dest
    pV = (ivec4*)Dest;
    for( i=0; i<NVerts; i++ )
    {
        // Store colors
        if ( Color )
        {
            pV[0].X = MAKE_PS2_R(Color->R);
            pV[0].Y = MAKE_PS2_G(Color->G);
            pV[0].Z = MAKE_PS2_B(Color->B);
            pV[0].W = MAKE_PS2_A(Color->A);
            Color++;
        }
        else
        {
            pV[0].X = MAKE_PS2_R(255);
            pV[0].Y = MAKE_PS2_G(255);
            pV[0].Z = MAKE_PS2_B(255);
            pV[0].W = MAKE_PS2_A(255);
        }

        // Store out the coordinates
        pV[1].X = MakePS2X(Pos->X);
        pV[1].Y = MakePS2Y(Pos->Y);
        pV[1].Z = MakePS2Z(Pos->Z);
        pV[1].W = MakePS2W(1);

        Pos++;
        pV += 2;
    }
}

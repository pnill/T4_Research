
#include "x_math.h"
#include "x_debug.h"

//===================================================
// M3_Determinant
//---------------------------------------------------
//===================================================
f32 M3_Determinant( matrix4* M )
{
    ASSERT( M );

    return  M->M[0][0] * (M->M[1][1] * M->M[2][2] - M->M[2][1] * M->M[1][2])
          + M->M[1][0] * (M->M[2][1] * M->M[0][2] - M->M[0][1] * M->M[2][2])
          + M->M[2][0] * (M->M[0][1] * M->M[1][2] - M->M[1][1] * M->M[0][2]);
}

//===================================================
// M3_Invert
//---------------------------------------------------
// M^-1
//===================================================
err M3_Invert( matrix4* Dest, matrix4* Src )
{
    matrix4 T;
    f32     OneOverDet;

    ASSERT( Dest );
    ASSERT( Src  );

    if (Src == Dest) 
    {
        T   = *Src;
        Src = &T;
    }

    OneOverDet = M3_Determinant( Src );

    if ( x_fabs( OneOverDet ) < 1.0e-12) return ERR_FAILURE;

    OneOverDet = 1 / OneOverDet;

    Dest->M[0][0] = (Src->M[1][1] * Src->M[2][2] - Src->M[2][1] * Src->M[1][2]) * OneOverDet;
    Dest->M[1][0] = (Src->M[2][0] * Src->M[1][2] - Src->M[1][0] * Src->M[2][2]) * OneOverDet;
    Dest->M[2][0] = (Src->M[1][0] * Src->M[2][1] - Src->M[2][0] * Src->M[1][1]) * OneOverDet;
    Dest->M[0][1] = (Src->M[2][1] * Src->M[0][2] - Src->M[0][1] * Src->M[2][2]) * OneOverDet;
    Dest->M[1][1] = (Src->M[0][0] * Src->M[2][2] - Src->M[2][0] * Src->M[0][2]) * OneOverDet;
    Dest->M[2][1] = (Src->M[2][0] * Src->M[0][1] - Src->M[0][0] * Src->M[2][1]) * OneOverDet;
    Dest->M[0][2] = (Src->M[0][1] * Src->M[1][2] - Src->M[1][1] * Src->M[0][2]) * OneOverDet;
    Dest->M[1][2] = (Src->M[1][0] * Src->M[0][2] - Src->M[0][0] * Src->M[1][2]) * OneOverDet;
    Dest->M[2][2] = (Src->M[0][0] * Src->M[1][1] - Src->M[1][0] * Src->M[0][1]) * OneOverDet;

    return ERR_SUCCESS;
}

//===================================================
// M3_SetSkew
//---------------------------------------------------
// set matrix to the skew symmetric matrix corresponding to 'v X'
//===================================================
void M3_SetSkew( matrix4* M, vector3d* V )
{
    ASSERT(M);
    ASSERT(V);

    M->M[0][0] = M->M[1][1] = M->M[2][2] = 0.0;
    M->M[1][2] =  V->X;
    M->M[2][1] = -V->X;
    M->M[2][0] =  V->Y;
    M->M[0][2] = -V->Y;
    M->M[0][1] =  V->Z;
    M->M[1][0] = -V->Z;
}

//===================================================
// M3_Transpose
//---------------------------------------------------
// M^T
//===================================================
void M3_Transpose( matrix4* Dest, matrix4* Src )
{
    ASSERT(Dest);
    ASSERT(Src);

    if (Dest == Src)
    {
        f32 t;

        t = Dest->M[1][0];
        Dest->M[1][0] = Dest->M[0][1];
        Dest->M[0][1] = t;

        t = Dest->M[2][0];
        Dest->M[2][0] = Dest->M[0][2];
        Dest->M[0][2] = t;

        t = Dest->M[2][1];
        Dest->M[2][1] = Dest->M[1][2];
        Dest->M[1][2] = t;
    }
    else
    {
        Dest->M[0][0] = Src->M[0][0];
        Dest->M[1][0] = Src->M[0][1];
        Dest->M[2][0] = Src->M[0][2];

        Dest->M[0][1] = Src->M[1][0];
        Dest->M[1][1] = Src->M[1][1];
        Dest->M[2][1] = Src->M[1][2];

        Dest->M[0][2] = Src->M[2][0];
        Dest->M[1][2] = Src->M[2][1];
        Dest->M[2][2] = Src->M[2][2];
    }
}

//===================================================
// M3_Symmetrize
//---------------------------------------------------
// M + M^T
//===================================================
void M3_Symmetrize( matrix4* Dest, matrix4* Src )
{
    ASSERT( Dest );
    ASSERT( Src );

    Dest->M[0][0] = 2 * Src->M[0][0];
    Dest->M[1][1] = 2 * Src->M[1][1];
    Dest->M[2][2] = 2 * Src->M[2][2];
    Dest->M[1][0] = Dest->M[0][1] = Src->M[1][0] + Src->M[0][1];
    Dest->M[2][1] = Dest->M[1][2] = Src->M[2][1] + Src->M[1][2];
    Dest->M[0][2] = Dest->M[2][0] = Src->M[0][2] + Src->M[2][0];
}

//===================================================
// M3_Negate
//---------------------------------------------------
// -M
//===================================================
void M3_Negate( matrix4* Dest, matrix4* Src )
{
    ASSERT( Dest );
    ASSERT( Src  );

    Dest->M[0][0] = - Src->M[0][0];
    Dest->M[1][0] = - Src->M[1][0];
    Dest->M[2][0] = - Src->M[2][0];

    Dest->M[0][1] = - Src->M[0][1];
    Dest->M[1][1] = - Src->M[1][1];
    Dest->M[2][1] = - Src->M[2][1];

    Dest->M[0][2] = - Src->M[0][2];
    Dest->M[1][2] = - Src->M[1][2];
    Dest->M[2][2] = - Src->M[2][2];
}  

//===================================================
// M3_Add
//---------------------------------------------------
// M + N
//===================================================
void M3_Add( matrix4* Dest, matrix4* Src1, matrix4* Src2 )
{
    ASSERT( Dest );
    ASSERT( Src1 );
    ASSERT( Src2 );

    Dest->M[0][0] = Src1->M[0][0] + Src2->M[0][0];
    Dest->M[1][0] = Src1->M[1][0] + Src2->M[1][0];
    Dest->M[2][0] = Src1->M[2][0] + Src2->M[2][0];

    Dest->M[0][1] = Src1->M[0][1] + Src2->M[0][1];
    Dest->M[1][1] = Src1->M[1][1] + Src2->M[1][1];
    Dest->M[2][1] = Src1->M[2][1] + Src2->M[2][1];

    Dest->M[0][2] = Src1->M[0][2] + Src2->M[0][2];
    Dest->M[1][2] = Src1->M[1][2] + Src2->M[1][2];
    Dest->M[2][2] = Src1->M[2][2] + Src2->M[2][2];
}

//===================================================
// M3_Sub
//---------------------------------------------------
// M - N
//===================================================
void M3_Sub( matrix4* Dest, matrix4* Src1, matrix4* Src2 )
{
    ASSERT( Dest );
    ASSERT( Src1 );
    ASSERT( Src2 );

    Dest->M[0][0] = Src1->M[0][0] - Src2->M[0][0];
    Dest->M[1][0] = Src1->M[1][0] - Src2->M[1][0];
    Dest->M[2][0] = Src1->M[2][0] - Src2->M[2][0];

    Dest->M[0][1] = Src1->M[0][1] - Src2->M[0][1];
    Dest->M[1][1] = Src1->M[1][1] - Src2->M[1][1];
    Dest->M[2][1] = Src1->M[2][1] - Src2->M[2][1];

    Dest->M[0][2] = Src1->M[0][2] - Src2->M[0][2];
    Dest->M[1][2] = Src1->M[1][2] - Src2->M[1][2];
    Dest->M[2][2] = Src1->M[2][2] - Src2->M[2][2];
}

//===================================================
// M3_MatrixScale
//---------------------------------------------------
// s * M
//===================================================
void M3_MatrixScale( matrix4* Dest, f32 Scale, matrix4* Src )
{
    ASSERT( Dest );
    ASSERT( Src  );

    Dest->M[0][0] = Scale * Src->M[0][0]; 
    Dest->M[1][0] = Scale * Src->M[1][0]; 
    Dest->M[2][0] = Scale * Src->M[2][0];
    Dest->M[0][1] = Scale * Src->M[0][1]; 
    Dest->M[1][1] = Scale * Src->M[1][1]; 
    Dest->M[2][1] = Scale * Src->M[2][1];
    Dest->M[0][2] = Scale * Src->M[0][2]; 
    Dest->M[1][2] = Scale * Src->M[1][2]; 
    Dest->M[2][2] = Scale * Src->M[2][2];
}

//===================================================
// M3_TransformVect
//---------------------------------------------------
// (M)(v) => xv
//===================================================
void M3_TransformVect( matrix4* M, vector3d* Dest, vector3d* Src )
{
    f32 ox, oy;

    ASSERT( M    );
    ASSERT( Dest );
    ASSERT( Src  );

    ox = Src->X; 
    oy = Src->Y;
    Dest->X = M->M[0][0] * ox + M->M[1][0] * oy + M->M[2][0] * Src->Z;
    Dest->Y = M->M[0][1] * ox + M->M[1][1] * oy + M->M[2][1] * Src->Z;
    Dest->Z = M->M[0][2] * ox + M->M[1][2] * oy + M->M[2][2] * Src->Z;
}

//===================================================
// M3_InvTransformVect
//---------------------------------------------------
// These are exactly like (M)(v), except the inverse
// transform this^-1 (= this^T) is used.  This can be thought of as
// a row vector transformation, e.g.: (v^T)(this) => xv^T
//===================================================
void M3_InvTransformVect( matrix4* M, vector3d* Dest, vector3d* Src )
{
    f32 ox, oy;

    ASSERT( M    );
    ASSERT( Dest );
    ASSERT( Src  );

    ox = Src->X; 
    oy = Src->Y;
    Dest->X = M->M[0][0] * ox + M->M[0][1] * oy + M->M[0][2] * Src->Z;
    Dest->Y = M->M[1][0] * ox + M->M[1][1] * oy + M->M[1][2] * Src->Z;
    Dest->Z = M->M[2][0] * ox + M->M[2][1] * oy + M->M[2][2] * Src->Z;
}

//===================================================
// M3_Mult
//---------------------------------------------------
// M * N || Src1 * Src2
//===================================================
void M3_Mult( matrix4* Dest, matrix4* Src1, matrix4* Src2 )
{
    matrix4 T;

    ASSERT( Dest );
    ASSERT( Src1 );
    ASSERT( Src2 );

    if (Dest == Src1)
    {
        T = *Src1;
        if (Src1 == Src2) Src2 = &T;
        Src1 = &T;
    }
    else
    if (Dest == Src2)
    {
        T = *Src2;
        Src2 = &T;
    }
    

    Dest->M[0][0] = Src1->M[0][0] * Src2->M[0][0] + Src1->M[1][0] * Src2->M[0][1] + Src1->M[2][0] * Src2->M[0][2];
    Dest->M[1][0] = Src1->M[0][0] * Src2->M[1][0] + Src1->M[1][0] * Src2->M[1][1] + Src1->M[2][0] * Src2->M[1][2];
    Dest->M[2][0] = Src1->M[0][0] * Src2->M[2][0] + Src1->M[1][0] * Src2->M[2][1] + Src1->M[2][0] * Src2->M[2][2];
    Dest->M[0][1] = Src1->M[0][1] * Src2->M[0][0] + Src1->M[1][1] * Src2->M[0][1] + Src1->M[2][1] * Src2->M[0][2];
    Dest->M[1][1] = Src1->M[0][1] * Src2->M[1][0] + Src1->M[1][1] * Src2->M[1][1] + Src1->M[2][1] * Src2->M[1][2];
    Dest->M[2][1] = Src1->M[0][1] * Src2->M[2][0] + Src1->M[1][1] * Src2->M[2][1] + Src1->M[2][1] * Src2->M[2][2];
    Dest->M[0][2] = Src1->M[0][2] * Src2->M[0][0] + Src1->M[1][2] * Src2->M[0][1] + Src1->M[2][2] * Src2->M[0][2];
    Dest->M[1][2] = Src1->M[0][2] * Src2->M[1][0] + Src1->M[1][2] * Src2->M[1][1] + Src1->M[2][2] * Src2->M[1][2];
    Dest->M[2][2] = Src1->M[0][2] * Src2->M[2][0] + Src1->M[1][2] * Src2->M[2][1] + Src1->M[2][2] * Src2->M[2][2];
}

//===================================================
// M3_PreMultOn
//---------------------------------------------------
// M * this
//===================================================
void M3_PreMultOn( matrix4* Dest, matrix4* Src )
{
    M3_Mult( Dest, Src, Dest );
}

//===================================================
// M3_PostMultOn
//---------------------------------------------------
// this * M
//===================================================
void M3_PostMultOn( matrix4* Dest, matrix4* Src )
{
    M3_Mult( Dest, Dest, Src );
}

//===================================================
// M3_SetupRotation
//===================================================
void M3_SetupRotation( matrix4* Dest, vector3d* Normal, radian R )
{
    f32     s;
    f32     c;
    f32     t;
    f32     XX,XY,XZ,YY,YZ,ZZ;
    f32     SX,SY,SZ;
    f32     tX,tY;

    ASSERT(Dest);
    ASSERT(Normal);

    x_sincos(R,&s,&c);
    t = 1 - c ;

    tX = t  * Normal->X;
    tY = t  * Normal->Y;
    XX = tX * Normal->X;
    XY = tX * Normal->Y;
    XZ = tX * Normal->Z;
    YY = tY * Normal->Y;
    YZ = tY * Normal->Z;
    ZZ = t  * Normal->Z * Normal->Z;
    SX = s  * Normal->X;
    SY = s  * Normal->Y;
    SZ = s  * Normal->Z;

    Dest->M[0][0] = XX +  c;
    Dest->M[0][1] = XY + SZ;
    Dest->M[0][2] = XZ - SY;
    Dest->M[1][0] = XY - SZ;
    Dest->M[1][1] = YY +  c;
    Dest->M[1][2] = YZ + SX;
    Dest->M[2][0] = XZ + SY;
    Dest->M[2][1] = YZ - SX;
    Dest->M[2][2] = ZZ +  c;
}

//===================================================
// M3_Orthogonalize
//===================================================
void M3_Orthogonalize( matrix4* M )
{
    vector3d VX, VY, VZ;

    ASSERT( M );

    VX.X = M->M[0][0]; 
    VX.Y = M->M[0][1];
    VX.Z = M->M[0][2];
    
    VY.X = M->M[1][0]; 
    VY.Y = M->M[1][1];
    VY.Z = M->M[1][2];

    V3_Normalize( &VX );
    V3_Normalize( &VY );
    V3_Cross    ( &VZ, &VX, &VY );
    V3_Cross    ( &VY, &VZ, &VX );

    M4_SetColumns ( M, &VX, &VY, &VZ );
}



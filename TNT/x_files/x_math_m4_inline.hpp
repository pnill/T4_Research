////////////////////////////////////////////////////////////////////////////
//
//  X_MATH_M4_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_M4_INLINE_HPP
#define X_MATH_M4_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

#include "x_plus.hpp"

////////////////////////////////////////////////////////////////////////////
//  Functions which do nothing, but are declared for readability.
////////////////////////////////////////////////////////////////////////////

inline matrix4:: matrix4( void )        {   }
//inline matrix4::~matrix4( void )        {   }

////////////////////////////////////////////////////////////////////////////
//  Functions which actually have some work to do.
////////////////////////////////////////////////////////////////////////////

inline matrix4::matrix4( const matrix4& aM )
{   
    M[0][0] = aM.M[0][0];
    M[0][1] = aM.M[0][1];
    M[0][2] = aM.M[0][2];
    M[0][3] = aM.M[0][3];

    M[1][0] = aM.M[1][0];
    M[1][1] = aM.M[1][1];
    M[1][2] = aM.M[1][2];
    M[1][3] = aM.M[1][3];

    M[2][0] = aM.M[2][0];
    M[2][1] = aM.M[2][1];
    M[2][2] = aM.M[2][2];
    M[2][3] = aM.M[2][3];

    M[3][0] = aM.M[3][0];
    M[3][1] = aM.M[3][1];
    M[3][2] = aM.M[3][2];
    M[3][3] = aM.M[3][3];
}

//==========================================================================

inline matrix4::matrix4( const radian3& Rotation )
{
    f32 sx,cx;
    f32 sy,cy;
    f32 sz,cz;
    f32 sxsz;
    f32 sxcz;    

    x_sincos( Rotation.Pitch, sx, cx );
    x_sincos( Rotation.Yaw,   sy, cy );
    x_sincos( Rotation.Roll,  sz, cz );
    sxsz = sx*sz;
    sxcz = sx*cz;

    // Fill out 3x3 rotations with ZXY no zero.
    M[0][0] = cy*cz + sy*sxsz;
    M[0][1] = cx*sz;
    M[0][2] = cy*sxsz - sy*cz;
    M[1][0] = sy*sxcz - sz*cy;
    M[1][1] = cx*cz;
    M[1][2] = sy*sz + sxcz*cy;
    M[2][0] = cx*sy;
    M[2][1] = -sx;
    M[2][2] = cx*cy;

    // Fill out edges of matrix.
    M[3][0] = M[3][1] = M[3][2] = 0.0f;
    M[0][3] = M[1][3] = M[2][3] = 0.0f;
    M[3][3] = 1.0f;    
}

//==========================================================================

inline matrix4::matrix4( f32 a00, f32 a10, f32 a20, f32 a30,
                         f32 a01, f32 a11, f32 a21, f32 a31,
                         f32 a02, f32 a12, f32 a22, f32 a32,
                         f32 a03, f32 a13, f32 a23, f32 a33 )
{   
    //
    // Order corresponds to C++ linear memory, but
    // assignment is in accords with Acclaim's (x, y) 
    // standard.
    //
    
    M[0][0] = a00;
    M[0][1] = a01;
    M[0][2] = a02;
    M[0][3] = a03;
    
    M[1][0] = a10;
    M[1][1] = a11;
    M[1][2] = a12;
    M[1][3] = a13;
    
    M[2][0] = a20;
    M[2][1] = a21;
    M[2][2] = a22;
    M[2][3] = a23;
    
    M[3][0] = a30;
    M[3][1] = a31;
    M[3][2] = a32;
    M[3][3] = a33;
}

//==========================================================================

inline void matrix4::Zero( void ) 
{
    M[0][0] =
    M[0][1] =
    M[0][2] =
    M[0][3] =

    M[1][0] =
    M[1][1] =
    M[1][2] =
    M[1][3] =

    M[2][0] =
    M[2][1] =
    M[2][2] =
    M[2][3] =

    M[3][0] =
    M[3][1] =
    M[3][2] =
    M[3][3] = 0.0f;
}

//==========================================================================

inline void matrix4::Identity( void )
{    
    Zero();

    M[0][0] =
    M[1][1] =
    M[2][2] =
    M[3][3] = 1.0f;
}

//==========================================================================

inline void matrix4::Transpose( void )
{
    f32     t;    
   
    t       = M[1][0];
    M[1][0] = M[0][1];
    M[0][1] = t;
    
    t       = M[2][0];
    M[2][0] = M[0][2];
    M[0][2] = t;
    
    t       = M[3][0];
    M[3][0] = M[0][3];
    M[0][3] = t;

    t       = M[2][1];
    M[2][1] = M[1][2];
    M[1][2] = t;
    
    t       = M[3][1];
    M[3][1] = M[1][3];
    M[1][3] = t;

    t       = M[3][2];
    M[3][2] = M[2][3];
    M[2][3] = t;
}

//==========================================================================

inline void matrix4::Orthogonalize( void )
{
    vector3 VX, VY, VZ;    

    VX.X = M[0][0]; 
    VX.Y = M[0][1];
    VX.Z = M[0][2];
    
    VY.X = M[1][0]; 
    VY.Y = M[1][1];
    VY.Z = M[1][2];

    VX.Normalize();
    VY.Normalize();
    VZ = Cross( VX, VY );
    VY = Cross( VZ, VX );

    SetColumns( VX, VY, VZ );
}

//==========================================================================

inline xbool matrix4::Invert( void )
{
    f32 work_space[4][8];
    f32 a;
    s32 j, jr, k, pivot;
    s32 i, row[4];    

    // Initialize.
    for( j = 0; j < 4; j++ )
    {
        for( k = 0; k < 4; k++ )
        {
            work_space[j][k]   = M[j][k];
            work_space[j][4+k] = 0.0f;
        }

        work_space[j][4+j] = 1.0f;
        row[j] = j;
    }

    // Eliminate columns.
    for( i=0; i < 4; i++ )
    {
        // Find pivot.
        k = i;
        
        a = (f32)x_abs( work_space[row[k]][k] );
        
        for( j = i + 1; j < 4; j++ )
        {
            jr = row[j];

            if( a < (f32)x_abs( work_space[jr][i] ) )
            {
                k = j;
                a = (f32)x_abs( work_space[jr][i] );
            }
        }

        // Swap PIVOT row with row I.
        pivot  = row[k];
        row[k] = row[i];
        row[i] = pivot;

        // Normalize pivot row.
        a = work_space[pivot][i];

        if( a == 0.0f ) 
            return( FALSE );

        work_space[pivot][i] = 1.0f;

        for( k = i + 1; k < 8; k++ ) 
            work_space[pivot][k] /= a;

        // Eliminate pivot from all remaining rows.
        for( j = i + 1; j < 4; j++ )
        {
            jr = row[j];
            a  = -work_space[jr][i];
            
            if( a == 0.0f ) 
                continue;

            work_space[jr][i] = 0.0f;

            for( k = i + 1; k < 8; k++ )
                work_space[jr][k] += (a * work_space[pivot][k]);
        }
    }

    // Back solve.
    for( i = 3; i > 0; --i )
    {
        pivot = row[i];
        for( j = i-1; j >= 0; --j )
        {
            jr = row[j];
            a  = work_space[jr][i];

            for( k = i; k < 8; k++ )
                work_space[jr][k] -= (a * work_space[pivot][k]);
        }
    }

    // Copy inverse back into Dest.
    for( j=0; j<4; j++ )
    {
        jr = row[j];
        for( k=0; k<4; k++ )
        {
            M[j][k] = work_space[jr][k+4];
        }
    }

    return( TRUE );
}

//==========================================================================

inline xbool matrix4::InvertSRT( void )
{
    matrix4 Src( *this );
    f32     Determinant;

    //
    // Calculate the determinant.
    //
    Determinant = ( Src.M[0][0] * ( Src.M[1][1] * Src.M[2][2] - Src.M[1][2] * Src.M[2][1] ) -
                    Src.M[0][1] * ( Src.M[1][0] * Src.M[2][2] - Src.M[1][2] * Src.M[2][0] ) +
                    Src.M[0][2] * ( Src.M[1][0] * Src.M[2][1] - Src.M[1][1] * Src.M[2][0] ) );

    if( x_abs( Determinant ) < 0.0001f ) 
        return( FALSE );

    Determinant = 1.0f / Determinant;

    //
    // Find the inverse of the matrix.
    //
    M[0][0] =  Determinant * ( Src.M[1][1] * Src.M[2][2] - Src.M[1][2] * Src.M[2][1] );
    M[0][1] = -Determinant * ( Src.M[0][1] * Src.M[2][2] - Src.M[0][2] * Src.M[2][1] );
    M[0][2] =  Determinant * ( Src.M[0][1] * Src.M[1][2] - Src.M[0][2] * Src.M[1][1] );
    M[0][3] = 0.0f;

    M[1][0] = -Determinant * ( Src.M[1][0] * Src.M[2][2] - Src.M[1][2] * Src.M[2][0] );
    M[1][1] =  Determinant * ( Src.M[0][0] * Src.M[2][2] - Src.M[0][2] * Src.M[2][0] );
    M[1][2] = -Determinant * ( Src.M[0][0] * Src.M[1][2] - Src.M[0][2] * Src.M[1][0] );
    M[1][3] = 0.0f;

    M[2][0] =  Determinant * ( Src.M[1][0] * Src.M[2][1] - Src.M[1][1] * Src.M[2][0] );
    M[2][1] = -Determinant * ( Src.M[0][0] * Src.M[2][1] - Src.M[0][1] * Src.M[2][0] );
    M[2][2] =  Determinant * ( Src.M[0][0] * Src.M[1][1] - Src.M[0][1] * Src.M[1][0] );
    M[2][3] = 0.0f;

    M[3][0] = -( Src.M[3][0] * M[0][0] + Src.M[3][1] * M[1][0] + Src.M[3][2] * M[2][0] );
    M[3][1] = -( Src.M[3][0] * M[0][1] + Src.M[3][1] * M[1][1] + Src.M[3][2] * M[2][1] );
    M[3][2] = -( Src.M[3][0] * M[0][2] + Src.M[3][1] * M[1][2] + Src.M[3][2] * M[2][2] );
    M[3][3] = 1.0f;

    return( TRUE );
}

//==========================================================================

inline vector3 matrix4::GetScale( void ) const
{    
    vector3 Scale;
 
    Scale.X = M[0][0];
    Scale.Y = M[1][1];
    Scale.Z = M[2][2];

    return( Scale );
}

//==========================================================================

inline void matrix4::SetScale( const vector3& Scale )
{
    M[0][0] = Scale.X;
    M[1][1] = Scale.Y;
    M[2][2] = Scale.Z;
}

//==========================================================================

inline void matrix4::Scale( const vector3& Scale )
{
    // Scale rotation vectors.
    
    M[0][0] *= Scale.X;
    M[0][1] *= Scale.Y;
    M[0][2] *= Scale.Z;
    
    M[1][0] *= Scale.X;    
    M[1][1] *= Scale.Y;
    M[1][2] *= Scale.Z;
    
    M[2][0] *= Scale.X;
    M[2][1] *= Scale.Y;
    M[2][2] *= Scale.Z;
    
    // Scale translation.
    
    M[3][0] *= Scale.X;
    M[3][1] *= Scale.Y;
    M[3][2] *= Scale.Z;
}

//==========================================================================

inline void matrix4::PreScale(  const vector3& Scale )
{
    M[0][0] *= Scale.X;
    M[0][1] *= Scale.X;
    M[0][2] *= Scale.X;
    M[0][3] *= Scale.X;

    M[1][0] *= Scale.Y;
    M[1][1] *= Scale.Y;
    M[1][2] *= Scale.Y;
    M[1][3] *= Scale.Y;
    
    M[2][0] *= Scale.Z;
    M[2][1] *= Scale.Z;
    M[2][2] *= Scale.Z;
    M[2][3] *= Scale.Z;
}

//==========================================================================

inline void matrix4::SetUniScale( f32 Scale )
{
    M[0][0] = Scale;
    M[1][1] = Scale;
    M[2][2] = Scale;
}

//==========================================================================

inline void matrix4::UniScale( f32 Scale )
{
    // Scale rotation vectors.
    
    M[0][0] *= Scale;
    M[0][1] *= Scale;
    M[0][2] *= Scale;

    M[1][0] *= Scale;
    M[1][1] *= Scale;
    M[1][2] *= Scale;
    
    M[2][0] *= Scale;  
    M[2][1] *= Scale;   
    M[2][2] *= Scale;

    // Scale translation vectors.
    
    M[3][0] *= Scale;
    M[3][1] *= Scale;
    M[3][2] *= Scale;    
}

//==========================================================================

inline void matrix4::PreUniScale( f32 Scale )
{
    M[0][0] *= Scale;
    M[0][1] *= Scale;
    M[0][2] *= Scale;
    M[0][3] *= Scale;
    
    M[1][0] *= Scale;
    M[1][1] *= Scale;
    M[1][2] *= Scale;
    M[1][3] *= Scale;
    
    M[2][0] *= Scale;  
    M[2][1] *= Scale;   
    M[2][2] *= Scale;
    M[2][3] *= Scale;    
}

//==========================================================================

inline void matrix4::ClearRotation( void )
{
    M[0][0] = 1.0f;
    M[0][1] = 0.0f;
    M[0][2] = 0.0f;

    M[1][0] = 0.0f;
    M[1][1] = 1.0f;
    M[1][2] = 0.0f;

    M[2][0] = 0.0f;
    M[2][1] = 0.0f;
    M[2][2] = 1.0f;
}

//==========================================================================

inline radian3 matrix4::GetRotation( void ) const
{
    radian3 Rotation;
    radian  X, Y, Z;
    f32     s, cx;

    // Get rotation about X.
    
    s = M[2][1];    
    if( s >  1.0f )  s =  1.0f;
    if( s < -1.0f )  s = -1.0f;
    X = (f32)x_asin( -s );

    // Get rotation about Y and Z.
    
    cx = (f32)x_cos(X);
    
    if( (cx > 0.0001f) || (cx < -0.0001f) )
    {
        Y = (f32)x_atan2( M[2][0], M[2][2] );
        Z = (f32)x_atan2( M[0][1], M[1][1] );
    }
    else
    {
        Z = (f32)x_atan2( M[0][2], M[0][0] );
        Y = 0.0f;
    }

    Rotation.Pitch = X;
    Rotation.Yaw   = Y;
    Rotation.Roll  = Z;
    
    return( Rotation );
}

//==========================================================================

inline void matrix4::SetRotation( const radian3& Rotation )
{
    f32 sx, cx;
    f32 sy, cy;
    f32 sz, cz;
    f32 sxsz;
    f32 sxcz;

    x_sincos( Rotation.Pitch, sx, cx );
    x_sincos( Rotation.Yaw,   sy, cy );
    x_sincos( Rotation.Roll,  sz, cz );
    sxsz = sx*sz;
    sxcz = sx*cz;

    // Fill out 3x3 rotations with ZXY no zero.
    
    M[0][0] = cy*cz + sy*sxsz;
    M[0][1] = cx*sz;
    M[0][2] = cy*sxsz - sy*cz;

    M[1][0] = sy*sxcz - sz*cy;
    M[1][1] = cx*cz;
    M[1][2] = sy*sz + sxcz*cy;
    
    M[2][0] = cx*sy;
    M[2][1] = -sx;
    M[2][2] = cx*cy;

    // Fill out edges of matrix.
    
    M[3][0] = M[3][1] = M[3][2] = 0.0f;
    M[0][3] = M[1][3] = M[2][3] = 0.0f;
    M[3][3] = 1.0f;    
}

//==========================================================================

inline void matrix4::Rotate( const radian3& Rotation )
{
    matrix4 aM;
    aM.SetRotation( Rotation );
    *this = aM * *this;    
}

//==========================================================================

inline void matrix4::PreRotate( const radian3& Rotation )
{
    matrix4 aM;    
    aM.SetRotation( Rotation );
    *this *= aM;
}

//==========================================================================

inline void matrix4::RotateX( radian Rx )
{
    matrix4 T;
    f32     S,C;    

    if ( Rx == 0.0f ) return;

    x_sincos( Rx, S, C );

    T.Identity();

    T.M[1][1] =  C;
    T.M[2][1] = -S;
    T.M[1][2] =  S;
    T.M[2][2] =  C;

    *this = T * *this;    
}

//==========================================================================

inline void matrix4::RotateY( radian Ry )
{
    matrix4 T;
    f32    S,C;    

    if ( Ry == 0.0f ) return;

    x_sincos( Ry, S, C );

    T.Identity();

    T.M[0][0] =  C;
    T.M[2][0] =  S;
    T.M[0][2] = -S;
    T.M[2][2] =  C;

    *this = T * *this;    
}

//==========================================================================

inline void matrix4::RotateZ( radian Rz )
{
    matrix4 T;
    f32    S,C;    

    if ( Rz == 0.0f ) return;

    x_sincos( Rz, S, C );

    T.Identity();

    T.M[0][0] =  C;
    T.M[1][0] = -S;
    T.M[0][1] =  S;
    T.M[1][1] =  C;
          
    *this = T * *this;    
}

//==========================================================================

inline void matrix4::PreRotateX( radian Rx )
{
    matrix4 T;
    f32     S,C;    

    if ( Rx == 0.0f ) return;

    x_sincos( Rx, S, C );

    T.Identity();

    T.M[1][1] =  C;
    T.M[2][1] = -S;
    T.M[1][2] =  S;
    T.M[2][2] =  C;

    *this *= T;
}

//==========================================================================

inline void matrix4::PreRotateY( radian Ry )
{
    matrix4 T;
    f32    S,C;    

    if ( Ry == 0.0f ) return;

    x_sincos( Ry, S, C );

    T.Identity();

    T.M[0][0] =  C;
    T.M[2][0] =  S;
    T.M[0][2] = -S;
    T.M[2][2] =  C;

    *this *= T;
}

//==========================================================================

inline void matrix4::PreRotateZ( radian Rz )
{
    matrix4 T;
    f32    S,C;    

    if ( Rz == 0.0f ) return;

    x_sincos( Rz, S, C );

    T.Identity();

    T.M[0][0] =  C;
    T.M[1][0] = -S;
    T.M[0][1] =  S;
    T.M[1][1] =  C;
          
    *this *= T;
}

//==========================================================================

inline void matrix4::ClearTranslation( void )
{    
    M[3][0] = 0.0f;
    M[3][1] = 0.0f;
    M[3][2] = 0.0f;
}

//==========================================================================

inline vector3 matrix4::GetTranslation( void ) const
{
    vector3 Translation;    

    Translation.X = M[3][0];
    Translation.Y = M[3][1];
    Translation.Z = M[3][2];

    return Translation;
}

//==========================================================================

inline void matrix4::SetTranslation( const vector3& Translation )
{    
    M[3][0] = Translation.X;
    M[3][1] = Translation.Y;
    M[3][2] = Translation.Z;
}

//==========================================================================

inline void matrix4::Translate( const vector3& Translation )
{        
    //
    // If bottom row of M is [0 0 0 1] do optimized 
    // translate, else do normal.
    //
    if( ( M[0][3] == 0.0f ) && ( M[1][3] == 0.0f ) &&
        ( M[2][3] == 0.0f ) && ( M[3][3] == 1.0f ) )
    {
        M[3][0] += Translation.X;
        M[3][1] += Translation.Y;
        M[3][2] += Translation.Z;
    }
    else 
    {
        M[0][0] += M[0][3] * Translation.X;
        M[1][0] += M[1][3] * Translation.X;
        M[2][0] += M[2][3] * Translation.X;
        M[3][0] += M[3][3] * Translation.X;

        M[0][1] += M[0][3] * Translation.Y;
        M[1][1] += M[1][3] * Translation.Y;
        M[2][1] += M[2][3] * Translation.Y;
        M[3][1] += M[3][3] * Translation.Y;

        M[0][2] += M[0][3] * Translation.Z;
        M[1][2] += M[1][3] * Translation.Z;
        M[2][2] += M[2][3] * Translation.Z;
        M[3][2] += M[3][3] * Translation.Z;
    }
}

//==========================================================================

inline void matrix4::PreTranslate( const vector3& Translation )
{                       
    // Checking for M[3] = [0 0 0 1] only eliminates 
    // 3 mults and 3 adds here.  Optimize?
    M[3][0] += (M[0][0] * Translation.X) + (M[1][0] * Translation.Y) + (M[2][0] * Translation.Z);
    M[3][1] += (M[0][1] * Translation.X) + (M[1][1] * Translation.Y) + (M[2][1] * Translation.Z);
    M[3][2] += (M[0][2] * Translation.X) + (M[1][2] * Translation.Y) + (M[2][2] * Translation.Z);
    M[3][3] += (M[0][3] * Translation.X) + (M[1][3] * Translation.Y) + (M[2][3] * Translation.Z);
}

//==========================================================================

inline void matrix4::Transform( const matrix4& M1 )
{
    *this = M1 * *this;  
}

//==========================================================================

inline void matrix4::PreTransform( const matrix4& M1 )
{
    *this = *this * M1;
}

//==========================================================================

inline void matrix4::GetRows( vector3& V1, vector3& V2, vector3& V3 ) const
{    
    V1.X = M[0][0];
    V2.X = M[0][1];
    V3.X = M[0][2];
    V1.Y = M[1][0];
    V2.Y = M[1][1];
    V3.Y = M[1][2];
    V1.Z = M[2][0];
    V2.Z = M[2][1];
    V3.Z = M[2][2];
}

inline void matrix4::GetRows( vector4& V1, vector4& V2, vector4& V3, vector4& V4 ) const
{    
    V1.X = M[0][0];
    V2.X = M[0][1];
    V3.X = M[0][2];
    V4.X = M[0][3];

    V1.Y = M[1][0];
    V2.Y = M[1][1];
    V3.Y = M[1][2];
    V4.Y = M[1][3];

    V1.Z = M[2][0];
    V2.Z = M[2][1];
    V3.Z = M[2][2];
    V4.Z = M[2][3];

    V1.W = M[3][0];
    V2.W = M[3][1];
    V3.W = M[3][2];
    V4.W = M[3][3];
}


//==========================================================================

inline void matrix4::SetRows( const vector3& V1, const vector3& V2, 
                              const vector3& V3 )
{    
    M[0][0] = V1.X;
    M[0][1] = V2.X;
    M[0][2] = V3.X;
    M[1][0] = V1.Y;
    M[1][1] = V2.Y;
    M[1][2] = V3.Y;
    M[2][0] = V1.Z;
    M[2][1] = V2.Z;
    M[2][2] = V3.Z;
}

inline void matrix4::SetRows( const vector4& V1, const vector4& V2, const vector4& V3, const vector4& V4 )
{    
    M[0][0] = V1.X;
    M[0][1] = V2.X;
    M[0][2] = V3.X;
    M[0][3] = V4.X;

    M[1][0] = V1.Y;
    M[1][1] = V2.Y;
    M[1][2] = V3.Y;
    M[1][3] = V4.Y;

    M[2][0] = V1.Z;
    M[2][1] = V2.Z;
    M[2][2] = V3.Z;
    M[2][3] = V4.Z;

    M[3][0] = V1.W;
    M[3][1] = V2.W;
    M[3][2] = V3.W;
    M[3][3] = V4.W;
}


//==========================================================================

inline void matrix4::GetColumns( vector3& V1, vector3& V2, vector3& V3 ) const
{    
    V1.X = M[0][0];
    V1.Y = M[0][1];
    V1.Z = M[0][2];
    V2.X = M[1][0];
    V2.Y = M[1][1];
    V2.Z = M[1][2];
    V3.X = M[2][0];
    V3.Y = M[2][1];
    V3.Z = M[2][2];
}

inline void matrix4::GetColumns( vector4& V1, vector4& V2, vector4& V3, vector4& V4 ) const
{    
    V1.X = M[0][0];
    V1.Y = M[0][1];
    V1.Z = M[0][2];
    V1.W = M[0][3];

    V2.X = M[1][0];
    V2.Y = M[1][1];
    V2.Z = M[1][2];
    V2.W = M[1][3];

    V3.X = M[2][0];
    V3.Y = M[2][1];
    V3.Z = M[2][2];
    V3.W = M[2][3];

    V4.X = M[2][0];
    V4.Y = M[2][1];
    V4.Z = M[2][2];
    V4.W = M[2][3];
}

//==========================================================================

inline void matrix4::SetColumns( const vector3& V1, const vector3& V2,
                                  const vector3& V3 )
{    
    M[0][0] = V1.X;
    M[0][1] = V1.Y;
    M[0][2] = V1.Z;
    M[1][0] = V2.X;
    M[1][1] = V2.Y;
    M[1][2] = V2.Z;
    M[2][0] = V3.X;
    M[2][1] = V3.Y;
    M[2][2] = V3.Z;
}

inline void matrix4::SetColumns( const vector4& V1, const vector4& V2,
                                  const vector4& V3, const vector4& V4 )
{    
    M[0][0] = V1.X;
    M[0][1] = V1.Y;
    M[0][2] = V1.Z;
    M[0][3] = V1.W;

    M[1][0] = V2.X;
    M[1][1] = V2.Y;
    M[1][2] = V2.Z;
    M[1][3] = V2.W;

    M[2][0] = V3.X;
    M[2][1] = V3.Y;
    M[2][2] = V3.Z;
    M[2][3] = V3.W;

    M[3][0] = V4.X;
    M[3][1] = V4.Y;
    M[3][2] = V4.Z;
    M[3][3] = V4.W;
}

//==========================================================================

inline void matrix4::SetupAxisRotate( radian Angle, const vector3& Point1, 
                                      const vector3& Point2 )
{   
    vector3 Translation( Point1 );
    vector3 Unit( Point2 - Point1 );

    Translation.Negate();
    Translate( Translation );

    SetupAxisRotate( Angle, Unit );

    Translation.Negate();
    Translate( Translation );
}

inline void matrix4::SetupAxisRotate( radian Angle, const vector3& Axis )
{
	f32 cosq = x_cos(Angle);
	f32 sinq = x_sin(Angle);

	f32 cosq1 = 1.0f - cosq;
	f32 xsinq = Axis.X * sinq;
	f32 ysinq = Axis.Y * sinq;
	f32 zsinq = Axis.Z * sinq;

	f32 xx = Axis.X * Axis.X;
	f32 yy = Axis.Y * Axis.Y;
	f32 zz = Axis.Z * Axis.Z;

	f32 xy = Axis.X * Axis.Y;
	f32 yz = Axis.Y * Axis.Z;
	f32 xz = Axis.X * Axis.Z;

	M[0][0] = xx + ((1.0f - xx) * cosq);
	M[0][1] = xy * cosq1 + zsinq;
	M[0][2] = xz * cosq1 - ysinq;
	M[0][3] = 0.0f;

	M[1][0] = xy * cosq1 - zsinq;
	M[1][1] = yy + ((1.0f - yy) * cosq);
	M[1][2] = yz * cosq1 + xsinq;
	M[1][3] = 0.0f;

	M[2][0] = xz * cosq1 + ysinq;
	M[2][1] = yz * cosq1 - xsinq;
	M[2][2] = zz + ((1.0f - zz) * cosq);
	M[2][3] = 0.0f;

	M[3][0] = 0.0f;
	M[3][1] = 0.0f;
	M[3][2] = 0.0f;
	M[3][3] = 1.0f;
}

//==========================================================================

inline void matrix4::SetupSRT( const vector3& Scale,
                               const radian3& Rotate,
                               const vector3& Translate )
{    
    SetRotation( Rotate );

    // Pre mult scaling
    M[0][0] *= Scale.X;
    M[0][1] *= Scale.X;
    M[0][2] *= Scale.X;
    M[1][0] *= Scale.Y;
    M[1][1] *= Scale.Y;    
    M[1][2] *= Scale.Y;    
    M[2][0] *= Scale.Z;
    M[2][1] *= Scale.Z;
    M[2][2] *= Scale.Z;
    
    // Pre mult translations
    M[3][0] += Translate.X;
    M[3][1] += Translate.Y;
    M[3][2] += Translate.Z;
}

//==========================================================================

inline xbool matrix4::operator == ( const matrix4& M ) const
{
	return  ( 0 == x_memcmp( this, &M, sizeof( matrix4 ) ) );
}

//==========================================================================

inline xbool matrix4::operator != ( const matrix4& M ) const
{
	return  ( 0 != x_memcmp( this, &M, sizeof( matrix4 ) ) );
}

//==========================================================================

inline vector3 matrix4::operator * ( const vector3& V ) const
{
    vector3 Rvec;

    Rvec.X = M[0][0]*V.X + M[1][0]*V.Y + M[2][0]*V.Z + M[3][0];
    Rvec.Y = M[0][1]*V.X + M[1][1]*V.Y + M[2][1]*V.Z + M[3][1];
    Rvec.Z = M[0][2]*V.X + M[1][2]*V.Y + M[2][2]*V.Z + M[3][2];

    return Rvec;
}


//==========================================================================

inline vector3 matrix4::Transform( const vector3& V ) const
{
    return( *this * V );    
}

//==========================================================================

inline void matrix4::Transform(       vector3* Dest, 
                                const vector3* Source,
                                      s32      NVerts ) const
{
    s32 i;
    f32 X, Y, Z;

    ASSERT( Dest );
    ASSERT( Source );    
    ASSERT( NVerts > 0 );

    for ( i = 0; i < NVerts; i++ )
    {
        X = Source[i].X;
        Y = Source[i].Y;
        Z = Source[i].Z;
        Dest[i].X  = M[0][0] * X + M[1][0] * Y + M[2][0] * Z + M[3][0];
        Dest[i].Y  = M[0][1] * X + M[1][1] * Y + M[2][1] * Z + M[3][1];
        Dest[i].Z  = M[0][2] * X + M[1][2] * Y + M[2][2] * Z + M[3][2];
    }
}

//==========================================================================
inline void matrix4::UnTransform(	      vector3* Dest, 
									const vector3* Source,
									      s32      NVerts ) const
{
    s32 i;
    f32 X, Y, Z;

    ASSERT( Dest );
    ASSERT( Source );    
    ASSERT( NVerts > 0 );

    for ( i = 0; i < NVerts; i++ )
    {
        X = Source[i].X;
        Y = Source[i].Y;
        Z = Source[i].Z;
        Dest[i].X  = M[0][0] * X + M[0][1] * Y + M[0][2] * Z + M[0][3];
        Dest[i].Y  = M[1][0] * X + M[1][1] * Y + M[1][2] * Z + M[1][3];
        Dest[i].Z  = M[2][0] * X + M[2][1] * Y + M[2][2] * Z + M[2][3];
    }
}

//==========================================================================

inline vector3 matrix4::TransformNoTranslate( const vector3& V ) const
{
    vector3 Rvec;

    Rvec.X = M[0][0]*V.X + M[1][0]*V.Y + M[2][0]*V.Z;
    Rvec.Y = M[0][1]*V.X + M[1][1]*V.Y + M[2][1]*V.Z;
    Rvec.Z = M[0][2]*V.X + M[1][2]*V.Y + M[2][2]*V.Z;

    return Rvec; 
}

//==========================================================================

inline void matrix4::TransformNoTranslate(       vector3* Dest, 
                                           const vector3* Source,
                                                 s32      NVerts ) const
{
    s32 i;
    f32 X, Y, Z;

    ASSERT( Dest );
    ASSERT( Source );    
    ASSERT( NVerts > 0 );

    for ( i = 0; i < NVerts; i++ )
    {
        X = Source[i].X;
        Y = Source[i].Y;
        Z = Source[i].Z;
        Dest[i].X  = M[0][0] * X + M[1][0] * Y + M[2][0] * Z;
        Dest[i].Y  = M[0][1] * X + M[1][1] * Y + M[2][1] * Z;
        Dest[i].Z  = M[0][2] * X + M[1][2] * Y + M[2][2] * Z;
    }
}

//==========================================================================

inline vector4 matrix4::operator * ( const vector4& V ) const
{
    vector4 Rvec;

    Rvec.X = M[0][0]*V.X + M[1][0]*V.Y + M[2][0]*V.Z + M[3][0]*V.W;
    Rvec.Y = M[0][1]*V.X + M[1][1]*V.Y + M[2][1]*V.Z + M[3][1]*V.W;
    Rvec.Z = M[0][2]*V.X + M[1][2]*V.Y + M[2][2]*V.Z + M[3][2]*V.W;
    Rvec.W = M[0][3]*V.X + M[1][3]*V.Y + M[2][3]*V.Z + M[3][3]*V.W;
    return Rvec;
}

//==========================================================================

inline vector4 matrix4::Transform( const vector4& V ) const
{
    return( *this * V );    
}

//==========================================================================

inline void matrix4::Transform(       vector4* Dest, 
                                const vector4* Source,
                                      s32      NVerts ) const
{
    s32 i;

    ASSERT( Dest );
    ASSERT( Source );    
    ASSERT( NVerts > 0 );

    for ( i = 0; i < NVerts; i++ )
    {
		Dest[i] = Transform(Source[i]);
    }
}

//==========================================================================

inline const matrix4& matrix4::operator = ( const matrix4& M1 )
{
    if( this != &M1 )
    {
        M[0][0] = M1.M[0][0];
        M[0][1] = M1.M[0][1];
        M[0][2] = M1.M[0][2];
        M[0][3] = M1.M[0][3];

        M[1][0] = M1.M[1][0];
        M[1][1] = M1.M[1][1];
        M[1][2] = M1.M[1][2];
        M[1][3] = M1.M[1][3];

        M[2][0] = M1.M[2][0];
        M[2][1] = M1.M[2][1];
        M[2][2] = M1.M[2][2];
        M[2][3] = M1.M[2][3];

        M[3][0] = M1.M[3][0];
        M[3][1] = M1.M[3][1];
        M[3][2] = M1.M[3][2];
        M[3][3] = M1.M[3][3];
    }

    return *this;
}

//==========================================================================
                                                      
inline matrix4& matrix4::operator += ( const matrix4& M1 )
{
    M[0][0] += M1.M[0][0];
    M[0][1] += M1.M[0][1];
    M[0][2] += M1.M[0][2];
    M[0][3] += M1.M[0][3];

    M[1][0] += M1.M[1][0];
    M[1][1] += M1.M[1][1];
    M[1][2] += M1.M[1][2];
    M[1][3] += M1.M[1][3];

    M[2][0] += M1.M[2][0];
    M[2][1] += M1.M[2][1];
    M[2][2] += M1.M[2][2];
    M[2][3] += M1.M[2][3];

    M[3][0] += M1.M[3][0];
    M[3][1] += M1.M[3][1];
    M[3][2] += M1.M[3][2];
    M[3][3] += M1.M[3][3];

    return( *this );
}

//==========================================================================

inline matrix4& matrix4::operator -= ( const matrix4& M1 )
{
    M[0][0] -= M1.M[0][0];
    M[0][1] -= M1.M[0][1];
    M[0][2] -= M1.M[0][2];
    M[0][3] -= M1.M[0][3];

    M[1][0] -= M1.M[1][0];
    M[1][1] -= M1.M[1][1];
    M[1][2] -= M1.M[1][2];
    M[1][3] -= M1.M[1][3];

    M[2][0] -= M1.M[2][0];
    M[2][1] -= M1.M[2][1];
    M[2][2] -= M1.M[2][2];
    M[2][3] -= M1.M[2][3];

    M[3][0] -= M1.M[3][0];
    M[3][1] -= M1.M[3][1];
    M[3][2] -= M1.M[3][2];
    M[3][3] -= M1.M[3][3];

    return ( *this );
}

//==========================================================================

inline matrix4& matrix4::operator *= ( const matrix4& M1 )
{
    *this = *this * M1;
    return( *this );
}

//==========================================================================

inline matrix4 operator + ( const matrix4& M1, const matrix4& M2 )
{
    matrix4 T;

    T.M[0][0] = M1.M[0][0] + M2.M[0][0];
    T.M[0][1] = M1.M[0][1] + M2.M[0][1];
    T.M[0][2] = M1.M[0][2] + M2.M[0][2];
    T.M[0][3] = M1.M[0][3] + M2.M[0][3];
    
    T.M[1][0] = M1.M[1][0] + M2.M[1][0];
    T.M[1][1] = M1.M[1][1] + M2.M[1][1];
    T.M[1][2] = M1.M[1][2] + M2.M[1][2];
    T.M[1][3] = M1.M[1][3] + M2.M[1][3];
    
    T.M[2][0] = M1.M[2][0] + M2.M[2][0];
    T.M[2][1] = M1.M[2][1] + M2.M[2][1];
    T.M[2][2] = M1.M[2][2] + M2.M[2][2];
    T.M[2][3] = M1.M[2][3] + M2.M[2][3];
    
    T.M[3][0] = M1.M[3][0] + M2.M[3][0];
    T.M[3][1] = M1.M[3][1] + M2.M[3][1];
    T.M[3][2] = M1.M[3][2] + M2.M[3][2];
    T.M[3][3] = M1.M[3][3] + M2.M[3][3];

    return( T );
}

//==========================================================================

inline matrix4 operator - ( const matrix4& M1, const matrix4& M2 )
{
    matrix4 T;

    T.M[0][0] = M1.M[0][0] - M2.M[0][0];
    T.M[0][1] = M1.M[0][1] - M2.M[0][1];
    T.M[0][2] = M1.M[0][2] - M2.M[0][2];
    T.M[0][3] = M1.M[0][3] - M2.M[0][3];
    
    T.M[1][0] = M1.M[1][0] - M2.M[1][0];
    T.M[1][1] = M1.M[1][1] - M2.M[1][1];
    T.M[1][2] = M1.M[1][2] - M2.M[1][2];
    T.M[1][3] = M1.M[1][3] - M2.M[1][3];
    
    T.M[2][0] = M1.M[2][0] - M2.M[2][0];
    T.M[2][1] = M1.M[2][1] - M2.M[2][1];
    T.M[2][2] = M1.M[2][2] - M2.M[2][2];
    T.M[2][3] = M1.M[2][3] - M2.M[2][3];
    
    T.M[3][0] = M1.M[3][0] - M2.M[3][0];
    T.M[3][1] = M1.M[3][1] - M2.M[3][1];
    T.M[3][2] = M1.M[3][2] - M2.M[3][2];
    T.M[3][3] = M1.M[3][3] - M2.M[3][3];

    return( T );
}

//==========================================================================

inline matrix4 operator * ( const matrix4& M1, const matrix4& M2 )
{
    matrix4 Rval;

    //
    // If bottom row of M1 and M2 = [0 0 0 1] then perform
    // optimized matrix multiply, otherwise do normal multiply.
    // Provides about a 15% increase in speed on PIII 450.
    //
    if( ( M1.M[0][3] == 0 ) && ( M2.M[0][3] == 0 ) &&
        ( M1.M[1][3] == 0 ) && ( M2.M[1][3] == 0 ) && 
        ( M1.M[2][3] == 0 ) && ( M2.M[2][3] == 0 ) && 
        ( M1.M[3][3] == 1 ) && ( M2.M[3][3] == 1 ) ) 
    {
        Rval.M[0][0] = (M1.M[0][0]*M2.M[0][0]) + (M1.M[1][0]*M2.M[0][1]) + (M1.M[2][0]*M2.M[0][2]);
        Rval.M[1][0] = (M1.M[0][0]*M2.M[1][0]) + (M1.M[1][0]*M2.M[1][1]) + (M1.M[2][0]*M2.M[1][2]);
        Rval.M[2][0] = (M1.M[0][0]*M2.M[2][0]) + (M1.M[1][0]*M2.M[2][1]) + (M1.M[2][0]*M2.M[2][2]);
        Rval.M[3][0] = (M1.M[0][0]*M2.M[3][0]) + (M1.M[1][0]*M2.M[3][1]) + (M1.M[2][0]*M2.M[3][2]) + M1.M[3][0];

        Rval.M[0][1] = (M1.M[0][1]*M2.M[0][0]) + (M1.M[1][1]*M2.M[0][1]) + (M1.M[2][1]*M2.M[0][2]);
        Rval.M[1][1] = (M1.M[0][1]*M2.M[1][0]) + (M1.M[1][1]*M2.M[1][1]) + (M1.M[2][1]*M2.M[1][2]);
        Rval.M[2][1] = (M1.M[0][1]*M2.M[2][0]) + (M1.M[1][1]*M2.M[2][1]) + (M1.M[2][1]*M2.M[2][2]);
        Rval.M[3][1] = (M1.M[0][1]*M2.M[3][0]) + (M1.M[1][1]*M2.M[3][1]) + (M1.M[2][1]*M2.M[3][2]) + M1.M[3][1];

        Rval.M[0][2] = (M1.M[0][2]*M2.M[0][0]) + (M1.M[1][2]*M2.M[0][1]) + (M1.M[2][2]*M2.M[0][2]);
        Rval.M[1][2] = (M1.M[0][2]*M2.M[1][0]) + (M1.M[1][2]*M2.M[1][1]) + (M1.M[2][2]*M2.M[1][2]);
        Rval.M[2][2] = (M1.M[0][2]*M2.M[2][0]) + (M1.M[1][2]*M2.M[2][1]) + (M1.M[2][2]*M2.M[2][2]);
        Rval.M[3][2] = (M1.M[0][2]*M2.M[3][0]) + (M1.M[1][2]*M2.M[3][1]) + (M1.M[2][2]*M2.M[3][2]) + M1.M[3][2];

        Rval.M[0][3] = 0.0f;
        Rval.M[1][3] = 0.0f;
        Rval.M[2][3] = 0.0f;
        Rval.M[3][3] = 1.0f;
    }
    else
    {
        s32 i;
        f32* D;
        f32* L;
        f32* R;

        D = (f32*)Rval.M;
        L = (f32*)M1.M;
        R = (f32*)M2.M;

        for (i=0; i<4; i++)
        {
            D[0] =   (L[0]*R[0]) + (L[4]*R[1]) + (L[8]*R[2])  + (L[12]*R[3]);
            D[1] =   (L[1]*R[0]) + (L[5]*R[1]) + (L[9]*R[2])  + (L[13]*R[3]);
            D[2] =   (L[2]*R[0]) + (L[6]*R[1]) + (L[10]*R[2]) + (L[14]*R[3]);
            D[3] =   (L[3]*R[0]) + (L[7]*R[1]) + (L[11]*R[2]) + (L[15]*R[3]);
            D+=4;
            R+=4;
        }
    }

    return ( Rval );
}

//==========================================================================

inline f32 matrix4::operator () ( s32 Row, s32 Column ) const
{
    ASSERT( (0 <= Row) && (Row < 4) );
    ASSERT( (0 <= Column) && (Column < 4) );
    return M[Row][Column];
}

//==========================================================================
        
inline f32& matrix4::operator () ( s32 Row, s32 Column )
{
    ASSERT( (0 <= Row) && (Row < 4) );
    ASSERT( (0 <= Column) && (Column < 4) );
    return M[Row][Column];
}

//==========================================================================

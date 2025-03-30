///////////////////////////////////////////////////////////////////////////
//
//  X_QUATRN.C - Quaternion math code
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_H
#include "x_math.h"
#endif

#ifndef X_DEBUG_H
#include "x_debug.h"
#endif

#ifndef X_PLUS_H
#include "x_plus.h"
#endif

///////////////////////////////////////////////////////////////////////////
//  PRIVATE PROTOTYPES
///////////////////////////////////////////////////////////////////////////

void M4_Mult ( matrix4* Dest, matrix4* Left, matrix4* Right );
void Q_Mult  ( quaternion* Dest, quaternion* Qa,  quaternion* Qb );

///////////////////////////////////////////////////////////////////////////
//  FUNCTION DEFINITIONS
/////////////////////////////////////////////////////////////////////////// 

//===================================================
// Q_Copy
//===================================================
void Q_Copy( quaternion* Dest, quaternion* Src )
{
    ASSERT(Dest);
    ASSERT(Src);

    *Dest = *Src;
}

//===================================================
// Q_Zero
//===================================================
void Q_Zero( quaternion* Q )
{
    ASSERT(Q);

    Q->Axis.X = 0;
    Q->Axis.Y = 0;
    Q->Axis.Z = 0;
    Q->S = 0;
}

//===================================================
// Q_Set
//===================================================
void Q_Set( quaternion* Q, f32 X, f32 Y, f32 Z, f32 S )
{
    ASSERT(Q);

    Q->Axis.X = X;
    Q->Axis.Y = Y;
    Q->Axis.Z = Z;
    Q->S = S;
}

//===================================================
// Q_Get
//===================================================
void Q_Get( quaternion* Q, f32* X, f32* Y, f32* Z, f32* S )
{
    ASSERT (X);
    ASSERT (Y);
    ASSERT (Z);
    ASSERT (S);
    ASSERT (Q);

    *X = Q->Axis.X;
    *Y = Q->Axis.Y;
    *Z = Q->Axis.Z;
    *S = Q->S;
}

//===================================================
// Q_Identity
//===================================================
void Q_Identity( quaternion* Q )
{
    ASSERT(Q);
    Q->Axis.X = 0;
    Q->Axis.Y = 0;
    Q->Axis.Z = 0;
    Q->S      = 1;
}

//===================================================
// Q_Add
//===================================================
void Q_Add( quaternion* Dest, quaternion* Qa, quaternion* Qb )
{
    ASSERT (Dest);
    ASSERT (Qa);
    ASSERT (Qb);

    Dest->Axis.X = Qa->Axis.X + Qb->Axis.X;
    Dest->Axis.Y = Qa->Axis.Y + Qb->Axis.Y;
    Dest->Axis.Z = Qa->Axis.Z + Qb->Axis.Z;
    Dest->S = Qa->S + Qb->S;
}

//===================================================
// Q_Sub
//===================================================
void Q_Sub( quaternion* Dest, quaternion* Qa, quaternion* Qb )
{
    ASSERT (Dest);
    ASSERT (Qa);
    ASSERT (Qb);

    Dest->Axis.X = Qa->Axis.X - Qb->Axis.X;
    Dest->Axis.Y = Qa->Axis.Y - Qb->Axis.Y;
    Dest->Axis.Z = Qa->Axis.Z - Qb->Axis.Z;
    Dest->S = Qa->S - Qb->S;
}

//===================================================
// Q_Scale
//===================================================
void Q_Scale( quaternion* Dest, quaternion* Qa, f32 C )
{
    ASSERT(Dest);
    ASSERT(Qa);

    Dest->Axis.X = Qa->Axis.X * C;
    Dest->Axis.Y = Qa->Axis.Y * C;
    Dest->Axis.Z = Qa->Axis.Z * C;
    Dest->S = Qa->S * C;
}

//===================================================
// Q_Negate
//===================================================
void Q_Negate( quaternion* Dest, quaternion* Src )
{
    ASSERT(Dest);
    ASSERT(Src);

    Dest->Axis.X = -Src->Axis.X;
    Dest->Axis.Y = -Src->Axis.Y;
    Dest->Axis.Z = -Src->Axis.Z;
    Dest->S      = -Src->S;
}

//===================================================
// Q_Mult
//---------------------------------------------------
// This is a private function and reserves the right to make
// Dest != Left and Dest != Right
//===================================================
void Q_Mult( quaternion* Dest, quaternion* Qa,  quaternion* Qb )
{
    quaternion T;

    ASSERT(Dest);
    ASSERT(Qa);
    ASSERT(Qb);

    if (Dest == Qa)
    {
        T = *Qa;
        if (Qa == Qb) Qb = &T;
        Qa = &T;
    }
    else
    if (Dest == Qb)
    {
        T = *Qb;
        Qb = &T;
    }

    Dest->S =       (  Qa->S       *  Qb->S       ) -
                    (  Qa->Axis.X  *  Qb->Axis.X  ) -
                    (  Qa->Axis.Y  *  Qb->Axis.Y  ) -
                    (  Qa->Axis.Z  *  Qb->Axis.Z  );

    Dest->Axis.X =  (  Qa->S       *  Qb->Axis.X  ) +
                    (  Qa->Axis.X  *  Qb->S       ) +
                    (  Qa->Axis.Y  *  Qb->Axis.Z  ) -
                    (  Qa->Axis.Z  *  Qb->Axis.Y  );

    Dest->Axis.Y =  (  Qa->S       *  Qb->Axis.Y  ) +
                    (  Qa->Axis.Y  *  Qb->S       ) +
                    (  Qa->Axis.Z  *  Qb->Axis.X  ) -
                    (  Qa->Axis.X  *  Qb->Axis.Z  );

    Dest->Axis.Z =  (  Qa->S       *  Qb->Axis.Z  ) +
                    (  Qa->Axis.Z  *  Qb->S       ) +
                    (  Qa->Axis.X  *  Qb->Axis.Y  ) -
                    (  Qa->Axis.Y  *  Qb->Axis.X  );
}

//===================================================
// Q_PreMultOn
//===================================================
void Q_PreMultOn( quaternion* Dest, quaternion* Q )
{
    ASSERT(Dest);
    ASSERT(Q);
    Q_Mult( Dest, Q, Dest );
}

//===================================================
// Q_PostMultOn
//===================================================
void Q_PostMultOn( quaternion* Dest, quaternion* Q )
{
    ASSERT(Dest);
    ASSERT(Q);
    Q_Mult( Dest, Dest, Q );
}

//===================================================
// Q_TransformVect
//---------------------------------------------------
// The Quat transformation routine use 19 multiplies and 12 adds
// (counting the multiplications by 2.0).  See Eqn (20) of "A
// Comparison of Transforms and Quaternions in Robotics," Funda and
// Paul, Proceedings of International Conference on Robotics and
// Automation, 1988, p. 886-991.
//===================================================
void Q_TransformVect( quaternion* Q, vector3d* Dest, vector3d* Src )
{
    vector3d uv, uuv;

    ASSERT( Q );
    ASSERT( Dest );
    ASSERT( Src  );

    V3_Cross( &uv,  &Q->Axis, Src );         
    V3_Cross( &uuv, &Q->Axis, &uv );         
    V3_Scale( &uv,  &uv,  Q->S * 2.0f );     
    V3_Scale( &uuv, &uuv, 2.0f );            
    V3_Add  ( Dest, Src, &uv );              
    V3_Add  ( Dest, Dest, &uuv );            
}

//===================================================
// Q_TransformVect
//===================================================
void Q_InvTransformVect( quaternion* Q, vector3d* Dest, vector3d* Src )
{
    vector3d uv, uuv;

    ASSERT( Q );
    ASSERT( Dest );
    ASSERT( Src  );

    V3_Cross( &uv,  &Q->Axis, Src );         
    V3_Cross( &uuv, &Q->Axis, &uv );         
    V3_Scale( &uv,  &uv, -Q->S * 2.0f );     
    V3_Scale( &uuv, &uuv, 2.0f );              
    V3_Add  ( Dest, Src, &uv );              
    V3_Add  ( Dest, Dest, &uuv );            
}

//===================================================
// Q_Length
//===================================================
f32 Q_Length( quaternion* Q )
{
    ASSERT(Q);
    return x_sqrt( Q->Axis.X * Q->Axis.X + Q->Axis.Y * Q->Axis.Y + Q->Axis.Z * Q->Axis.Z + Q->S * Q->S );
}

//===================================================
// Q_Normalize
//===================================================
void Q_Normalize( quaternion* Q )
{
    f32 Mag;
    ASSERT(Q);

    Mag = Q_Length( Q );
    ASSERT(Mag && "Length of quaternion is ZERO");

    if( Mag ) 
    {
        Mag = 1.0f/Mag;
        Q->Axis.X *= Mag;
        Q->Axis.Y *= Mag;
        Q->Axis.Z *= Mag;
        Q->S      *= Mag;
    }
}

//===================================================
// Q_Invert
//===================================================
void Q_Invert( quaternion* Dest, quaternion* Src )
{
    ASSERT(Dest);
    ASSERT(Src);
    /* OLD METHOD
    V3_Negate( &Q->Axis );
    */
    if ( Dest == Src)
    {
        Dest->S = -Dest->S;
    }
    else
    {
        *Dest   = *Src;
        Dest->S = -Dest->S;
    }
}

//===================================================
// Q_Dot
//===================================================
f32 Q_Dot( quaternion* Qa, quaternion* Qb )
{
    f32 Dot;

    ASSERT(Qa);
    ASSERT(Qb);

    Dot =   Qa->Axis.X * Qb->Axis.X +
            Qa->Axis.Y * Qb->Axis.Y +
            Qa->Axis.Z * Qb->Axis.Z +
            Qa->S      * Qb->S;

    return Dot;
}

//===================================================
// Q_SetupFromMatrix
//===================================================
void Q_SetupFromMatrix( quaternion* Q, matrix4* Matrix )
{
    f32 qs2, qx2, qy2, qz2;  // squared magniudes of quaternion components
    s32 i;

    // first compute squared magnitudes of quaternion components - at least one
    // will be greater than 0 since quaternion is unit magnitude

    qs2 = 0.25f * (Matrix->M[0][0] + Matrix->M[1][1] + Matrix->M[2][2] + 1.0f );
    qx2 = qs2 - 0.5f * (Matrix->M[1][1] + Matrix->M[2][2]);
    qy2 = qs2 - 0.5f * (Matrix->M[2][2] + Matrix->M[0][0]);
    qz2 = qs2 - 0.5f * (Matrix->M[0][0] + Matrix->M[1][1]);


    // find maximum magnitude component
    i = (qs2 > qx2 ) ?
    ((qs2 > qy2) ? ((qs2 > qz2) ? 0 : 3) : ((qy2 > qz2) ? 2 : 3)) :
    ((qx2 > qy2) ? ((qx2 > qz2) ? 1 : 3) : ((qy2 > qz2) ? 2 : 3));

    // compute signed quaternion components using numerically stable method
    switch(i) 
    {
        case 0:
            {
                f32 tmp;

                Q->S      = x_sqrt(qs2);
                tmp = 0.25f / Q->S;
                Q->Axis.X = (Matrix->M[1][2] - Matrix->M[2][1]) * tmp;
                Q->Axis.Y = (Matrix->M[2][0] - Matrix->M[0][2]) * tmp;
                Q->Axis.Z = (Matrix->M[0][1] - Matrix->M[1][0]) * tmp;
            }
        break;
        case 1:
            {
                f32 tmp;

                Q->Axis.X = x_sqrt(qx2);
                tmp = 0.25f / Q->Axis.X;
                Q->S      = (Matrix->M[1][2] - Matrix->M[2][1]) * tmp;
                Q->Axis.Y = (Matrix->M[1][0] + Matrix->M[0][1]) * tmp;
                Q->Axis.Z = (Matrix->M[2][0] + Matrix->M[0][2]) * tmp;
            }
        break;
        case 2:
            {
                f32 tmp;

                Q->Axis.Y = x_sqrt(qy2);
                tmp = 0.25f / Q->Axis.Y;
                Q->S      = (Matrix->M[2][0] - Matrix->M[0][2]) * tmp;
                Q->Axis.Z = (Matrix->M[2][1] + Matrix->M[1][2]) * tmp;
                Q->Axis.X = (Matrix->M[0][1] + Matrix->M[1][0]) * tmp;
            }
        break;
        case 3:
            {
                f32 tmp;

                Q->Axis.Z = x_sqrt(qz2);
                tmp = 0.25f / Q->Axis.Z;
                Q->S      = (Matrix->M[0][1] - Matrix->M[1][0]) * tmp;
                Q->Axis.X = (Matrix->M[0][2] + Matrix->M[2][0]) * tmp;
                Q->Axis.Y = (Matrix->M[1][2] + Matrix->M[2][1]) * tmp;
            }
            break;
    }

    // for consistency, force positive scalar component [ (s; v) = (-s; -v) ]
    if (Q->S < 0) Q_Negate( Q, Q );

    // normalize, just to be safe
    Q_Normalize( Q );
}


//===================================================
// Q_SetupFromAxisAngle
//===================================================
void Q_SetupFromAxisAngle ( quaternion* Q, vector3d* Axis, radian R )
{
    f32 S;
    f32 C;

    ASSERT (Q);
    ASSERT (Axis);

    x_sincos(R/2,&S,&C);

    Q->Axis.X = Axis->X * S;
    Q->Axis.Y = Axis->Y * S;
    Q->Axis.Z = Axis->Z * S;
    Q->S = C;
}

//===================================================
// Q_BuldFromAngles
//===================================================
void Q_SetupFromAngles( quaternion* Q, radian Rx, radian Ry, radian Rz )
{
    quaternion QR;
    f32 S,C;

    ASSERT (Q);

    Q_Set (Q,0,0,0,1);

    // Rotate in X
    x_sincos(Rx/2,&S,&C);
    Q_Set  ( &QR, S, 0, 0, C );
    Q_PreMultOn( Q, &QR );

    // Rotate in Y
    x_sincos(Ry/2,&S,&C);
    Q_Set  ( &QR, 0, S, 0, C );
    Q_PreMultOn( Q, &QR );

    // Rotate in Z
    x_sincos(Rz/2,&S,&C);
    Q_Set  ( &QR, 0, 0, S, C );
    Q_PreMultOn( Q, &QR );
}

//===================================================
// Q_BlendLinear
//===================================================
void Q_BlendLinear( quaternion* Dest, quaternion* Qa, quaternion* Qb, f32 T )
{
    quaternion QT0;
    quaternion QT1;
    f32 Cs;
    ASSERT(Dest);
    ASSERT(Qa);
    ASSERT(Qb);

    QT0 = *Qa;
    QT1 = *Qb;

    // Cosine of angle between Q0,Q1... dot product
    Cs = (QT0.Axis.X*QT1.Axis.X) + 
         (QT0.Axis.Y*QT1.Axis.Y) + 
         (QT0.Axis.Z*QT1.Axis.Z) + 
         (QT0.S*QT1.S);

    // Check if we need to interpolate the other way around
    if (Cs <= 0.0f)
    {
        QT1.Axis.X = -QT1.Axis.X;
        QT1.Axis.Y = -QT1.Axis.Y;
        QT1.Axis.Z = -QT1.Axis.Z;
        QT1.S      = -QT1.S;
    }

    Dest->Axis.X  = QT1.Axis.X  + T * ( QT1.Axis.X - QT0.Axis.X );
    Dest->Axis.Y  = QT1.Axis.Y  + T * ( QT1.Axis.Y - QT0.Axis.Y );
    Dest->Axis.Z  = QT1.Axis.Z  + T * ( QT1.Axis.Z - QT0.Axis.Z );
    Dest->S       = QT1.S       + T * ( QT1.S      - QT0.S      );

    Q_Normalize( Dest );
}

//===================================================
// Q_BlendSpherical
//===================================================
void Q_BlendSpherical( quaternion* Q, quaternion* Q0, quaternion* Q1, f32 T )
{
    quaternion QT0;
    quaternion QT1;
    f32 Cs,Sn;
    f32 Angle,InvSn,TAngle;
    f32 C0,C1;

    // Copy quaternions into temp
    QT0 = *Q0;
    QT1 = *Q1;

    // Cosine of angle between Q0,Q1... dot product
    Cs = (QT0.Axis.X*QT1.Axis.X) + 
         (QT0.Axis.Y*QT1.Axis.Y) + 
         (QT0.Axis.Z*QT1.Axis.Z) + 
         (QT0.S*QT1.S);

    // Check if we need to interpolate the other way around
    if (Cs <= 0.0f)
    {
        QT1.Axis.X = -QT1.Axis.X;
        QT1.Axis.Y = -QT1.Axis.Y;
        QT1.Axis.Z = -QT1.Axis.Z;
        QT1.S      = -QT1.S;
        Cs         = (QT0.Axis.X*QT1.Axis.X) + 
                     (QT0.Axis.Y*QT1.Axis.Y) + 
                     (QT0.Axis.Z*QT1.Axis.Z) + 
                     (QT0.S     *QT1.S);
    }

    // Compute sine of angle between Q0,Q1
    Sn = 1.0f - Cs*Cs;
    if (Sn < 0.0f) Sn = -Sn;
    Sn = x_sqrt( Sn );

    // Check if quaternions are very close together
    if ( (Sn < 1e-3) && (Sn > -1e-3) )
    {
        *Q = QT0;
        return;
    }

    Angle       = x_atan2( Sn, Cs );
    InvSn       = 1.0f/Sn;    
    TAngle      = T*Angle;

    C0      = x_sin( Angle - TAngle) * InvSn;
    C1      = x_sin( TAngle ) * InvSn;    

    Q->Axis.X   = C0*QT0.Axis.X + C1*QT1.Axis.X;
    Q->Axis.Y   = C0*QT0.Axis.Y + C1*QT1.Axis.Y;
    Q->Axis.Z   = C0*QT0.Axis.Z + C1*QT1.Axis.Z;
    Q->S        = C0*QT0.S      + C1*QT1.S;
}

//===================================================
// Q_BlendSpherical
//===================================================
void Q_BlendSpherical2( quaternion* Dest, quaternion* Qa, quaternion* Qb, f32 T )
{
    quaternion Qtemp1;
    quaternion Qtemp2;
    f32        C1, C2, S;
    f32        Theta;
    f32        CosTheta;

    ASSERT(Dest);
    ASSERT(Qa);
    ASSERT(Qb);

    Q_Copy ( &Qtemp1, Qa );
    Q_Copy ( &Qtemp2, Qb );

    CosTheta = Q_Dot(Qa,Qb);
    // if B is on opposite hemisphere from A, use -B instead 
    if (CosTheta < 0.0f)
    {
        Q_Negate(&Qtemp2,&Qtemp2);
    }

    Theta   = Q_AngleBetween( &Qtemp1, &Qtemp2 );
    S       = x_sin( Theta );
    C1      = x_sin( ( 1 - T ) * Theta ) / S;
    C2      = x_sin( T * Theta )         / S;

    Q_Scale( &Qtemp1, &Qtemp1, C1 );
    Q_Scale( &Qtemp2, &Qtemp2, C2 );

    Q_Add  ( Dest, &Qtemp1, &Qtemp2 );
}

//===================================================
// Q_GetAxis
//===================================================
void Q_GetAxis( quaternion* Q, vector3d* V )
{
    radian angle;
    radian s;

    ASSERT(V);
    ASSERT(Q);

    angle = Q->S;

    if ( angle >  1.0f ) angle =  1.0f;
    if ( angle < -1.0f ) angle = -1.0f;

    s = x_sin( x_acos( angle ) );

    if ( s != 0 ) V3_Scale( V, &Q->Axis, 1.0f / s );
    else V3_Zero( V );
}

//===================================================
// Q_GetAngle
//===================================================
radian Q_GetAngle( quaternion* Q )
{
    radian angle;

    ASSERT(Q);

    angle = Q->S;

    if (angle >  1.0f) angle =  1.0f;
    if (angle < -1.0f) angle = -1.0f;

    return x_acos( angle )*2;
}

//===================================================
// Q_AngleBetween
//===================================================
radian Q_AngleBetween( quaternion* Qa, quaternion* Qb )
{
    f32 Dot;
    ASSERT(Qa);
    ASSERT(Qb);

    Dot = Q_Dot(Qa, Qb);

    if ( Dot >  1.0f ) Dot =  1.0f;
    if ( Dot < -1.0f ) Dot = -1.0f;

    return x_acos( Dot );
}

//===================================================
// Q_SetupMatrix
//===================================================
void Q_SetupMatrix( quaternion* Q, matrix4* M )
{
    f32 tx  = 2.0f*Q->Axis.X;    
    f32 ty  = 2.0f*Q->Axis.Y;    
    f32 tz  = 2.0f*Q->Axis.Z;
    f32 twx = tx*Q->S;    
    f32 twy = ty*Q->S;    
    f32 twz = tz*Q->S;
    f32 txx = tx*Q->Axis.X;    
    f32 txy = ty*Q->Axis.X;    
    f32 txz = tz*Q->Axis.X;
    f32 tyy = ty*Q->Axis.Y;   
    f32 tyz = tz*Q->Axis.Y;   
    f32 tzz = tz*Q->Axis.Z;

    ASSERT(M);
    ASSERT(Q);

    M->M[0][0] = 1.0f-(tyy+tzz);   
    M->M[1][0] = txy-twz;          
    M->M[2][0] = txz+twy;
    M->M[0][1] = txy+twz;          
    M->M[1][1] = 1.0f-(txx+tzz);   
    M->M[2][1] = tyz-twx;
    M->M[0][2] = txz-twy;          
    M->M[1][2] = tyz+twx;          
    M->M[2][2] = 1.0f-(txx+tyy);

    M->M[3][0] = M->M[3][1] = M->M[3][2] = 
    M->M[0][3] = M->M[1][3] = M->M[2][3] = 0.0f;
    M->M[3][3] = 1.0f;
}

//===================================================
// Q_Derivative
//===================================================
void Q_Derivative( quaternion* Dest, quaternion* Src, vector3d* W )
{
    quaternion q;

    ASSERT( Dest );
    ASSERT( Src  );
    ASSERT( W    );

    q = *Src;

    Dest->S      = 0.5f * (-q.Axis.X * W->X - q.Axis.Y * W->Y - q.Axis.Z * W->Z);
    Dest->Axis.X = 0.5f * ( q.S      * W->X - q.Axis.Z * W->Y + q.Axis.Y * W->Z);
    Dest->Axis.Y = 0.5f * ( q.Axis.Z * W->X + q.S      * W->Y - q.Axis.X * W->Z);
    Dest->Axis.Z = 0.5f * (-q.Axis.Y * W->X + q.Axis.X * W->Y + q.S      * W->Z);
}

///////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
//  X_MATH_Q_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_Q_INLINE_HPP
#define X_MATH_Q_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  Functions which do nothing, but are declared for readability.
////////////////////////////////////////////////////////////////////////////

inline quaternion:: quaternion( void )        {   }
//inline quaternion::~quaternion( void )        {   }

////////////////////////////////////////////////////////////////////////////
//  Functions which actually have some work to do.
////////////////////////////////////////////////////////////////////////////

inline quaternion::quaternion( f32 aX, f32 aY, f32 aZ, f32 aW )
{
    X = aX;
    Y = aY;
    Z = aZ;
    W = aW;
}

//==========================================================================

inline quaternion::quaternion( const quaternion&  Q )
{
    *this = Q;
}

//==========================================================================

inline quaternion::quaternion( const matrix4& M )
{
    SetupMatrix( M );
}

//==========================================================================

inline quaternion::quaternion( const radian3& Rotation )
{
	X = Y = Z = 0.0f; W = 1.0f;
    SetupRotate( Rotation );
}

//==========================================================================

inline void quaternion::Zero( void )
{
    X = Y = Z = W = 0.0f;
}

//==========================================================================

inline void quaternion::Identity( void )
{
    X = Y = Z = 0.0f; W = 1.0f;
}

//==========================================================================

inline void quaternion::Invert( void )
{
    // This can be done also be negating (X,Y,Z) stead of W
    W = -W; 
}

//==========================================================================

inline void quaternion::Scale( f32 Scale )
{
    X *= Scale; Y *= Scale; Z *= Scale; W *= Scale;
}

//==========================================================================

inline void quaternion::RotateX( radian Rx )
{
    f32 S, C;
    quaternion Q;
    x_sincos( Rx/2.0f, S, C );
    Q.X = S; Q.Y = 0.0f; Q.Z = 0.0f; Q.W = C;
    *this = Q * *this;
}

//==========================================================================

inline void quaternion::RotateY( radian Ry )
{
    f32 S, C;
    quaternion Q;
    x_sincos( Ry/2.0f, S, C );
    Q.X = 0.0f; Q.Y = S; Q.Z = 0.0f; Q.W = C;
    *this = Q * *this;
}

//==========================================================================

inline void quaternion::RotateZ( radian Rz )
{
    f32 S, C;
    quaternion Q;
    x_sincos( Rz/2.0f, S, C );
    Q.X = 0.0f; Q.Y = 0.0f; Q.Z = S; Q.W = C;
    *this = Q * *this;
}

//==========================================================================

inline void quaternion::SetupRotate( const radian3& Rotation )
{
    RotateZ( Rotation.Roll );
    RotateX( Rotation.Pitch );
    RotateY( Rotation.Yaw );
}

//==========================================================================

inline void quaternion::Rotate( const radian3& Rotation )
{
    quaternion Q;
    Q = *this;
    Q.RotateZ( Rotation.Roll );
    Q.RotateX( Rotation.Pitch );
    Q.RotateY( Rotation.Yaw );
    *this = Q * *this;
}


//==========================================================================

inline void quaternion::PreRotateX( radian Rx )
{
    f32 S, C;
    quaternion Q;
    x_sincos( Rx/2.0f, S, C );
    Q.X = S; Q.Y = 0.0f; Q.Z = 0.0f; Q.W = C;
    *this *= Q;
}

//==========================================================================

inline void quaternion::PreRotateY( radian Ry )
{
    f32 S, C;
    quaternion Q;
    x_sincos( Ry/2.0f, S, C );
    Q.X = 0.0f; Q.Y = S; Q.Z = 0.0f; Q.W = C;
    *this *= Q;
}

//==========================================================================

inline void quaternion::PreRotateZ( radian Rz )
{
    f32 S, C;
    quaternion Q;
    x_sincos( Rz/2.0f, S, C );
    Q.X = 0.0f; Q.Y = 0.0f; Q.Z = S; Q.W = C;
    *this *= Q;
}

//==========================================================================

inline void quaternion::PreRotate( const radian3& Rotation )
{
    quaternion Q;
    Q = *this;
    PreRotateY( Rotation.Yaw );
    PreRotateX( Rotation.Pitch );
    PreRotateZ( Rotation.Roll );
    *this *= Q;
}

//==========================================================================

inline void quaternion::SetupPreRotate( const radian3& Rotation )
{
    PreRotateY( Rotation.Yaw );
    PreRotateX( Rotation.Pitch );
    PreRotateZ( Rotation.Roll );
}


//==========================================================================

inline void quaternion::Transform( const quaternion& Q )
{
    *this = Q * *this;  
}

//==========================================================================

inline void quaternion::PreTransform( const quaternion& Q )
{
    *this = *this * Q;
}

//==========================================================================

inline quaternion& quaternion::operator += ( const quaternion& Q )
{
    *this = *this + Q;
    return *this;
}

//==========================================================================

inline quaternion& quaternion::operator -= ( const quaternion& Q )
{
    *this = *this - Q;
    return *this;
}

//==========================================================================

inline quaternion& quaternion::operator *= ( const quaternion& Q )
{
    *this = *this * Q;
    return *this;
}

//==========================================================================

inline void quaternion::SetupMatrix( const matrix4& Matrix )
{
    f32 tmp;
    f32 qs2, qx2, qy2, qz2;  // squared magniudes of quaternion components
    s32 i;

    // first compute squared magnitudes of quaternion components - at least one
    // will be greater than 0 since quaternion is unit magnitude
    qs2 = 0.25f * (Matrix.M[0][0] + Matrix.M[1][1] + Matrix.M[2][2] + 1.0f );
    qx2 = qs2 - 0.5f * (Matrix.M[1][1] + Matrix.M[2][2]);
    qy2 = qs2 - 0.5f * (Matrix.M[2][2] + Matrix.M[0][0]);
    qz2 = qs2 - 0.5f * (Matrix.M[0][0] + Matrix.M[1][1]);


    // find maximum magnitude component
    i = (qs2 > qx2 ) ?
    ((qs2 > qy2) ? ((qs2 > qz2) ? 0 : 3) : ((qy2 > qz2) ? 2 : 3)) :
    ((qx2 > qy2) ? ((qx2 > qz2) ? 1 : 3) : ((qy2 > qz2) ? 2 : 3));

    // compute signed quaternion components using numerically stable method
    switch(i) 
    {
        case 0:
                W = x_sqrt(qs2);
                tmp = 0.25f / W;
                X = (Matrix.M[1][2] - Matrix.M[2][1]) * tmp;
                Y = (Matrix.M[2][0] - Matrix.M[0][2]) * tmp;
                Z = (Matrix.M[0][1] - Matrix.M[1][0]) * tmp;
                break;
        case 1:
                X = x_sqrt(qx2);
                tmp = 0.25f / X;
                W = (Matrix.M[1][2] - Matrix.M[2][1]) * tmp;
                Y = (Matrix.M[1][0] + Matrix.M[0][1]) * tmp;
                Z = (Matrix.M[2][0] + Matrix.M[0][2]) * tmp;
                break;
        case 2:
                Y = x_sqrt(qy2);
                tmp = 0.25f / Y;
                W = (Matrix.M[2][0] - Matrix.M[0][2]) * tmp;
                Z = (Matrix.M[2][1] + Matrix.M[1][2]) * tmp;
                X = (Matrix.M[0][1] + Matrix.M[1][0]) * tmp;
                break;
        case 3:
                Z = x_sqrt(qz2);
                tmp = 0.25f / Z;
                W = (Matrix.M[0][1] - Matrix.M[1][0]) * tmp;
                X = (Matrix.M[0][2] + Matrix.M[2][0]) * tmp;
                Y = (Matrix.M[1][2] + Matrix.M[2][1]) * tmp;
                break;
    }

    // for consistency, force positive scalar component [ (s; v) = (-s; -v) ]
    if( W < 0) Negate();

    // normalize, just to be safe
    Normalize();
}

//==========================================================================

inline void quaternion::BuildMatrix( matrix4& M ) const
{
    f32 tx  = 2.0f*X;    
    f32 ty  = 2.0f*Y;    
    f32 tz  = 2.0f*Z;
    f32 twx = tx*W;    
    f32 twy = ty*W;    
    f32 twz = tz*W;
    f32 txx = tx*X;    
    f32 txy = ty*X;    
    f32 txz = tz*X;
    f32 tyy = ty*Y;   
    f32 tyz = tz*Y;   
    f32 tzz = tz*Z;

    M.M[0][0] = 1.0f-(tyy+tzz);   
    M.M[1][0] = txy-twz;          
    M.M[2][0] = txz+twy;
    M.M[0][1] = txy+twz;          
    M.M[1][1] = 1.0f-(txx+tzz);   
    M.M[2][1] = tyz-twx;
    M.M[0][2] = txz-twy;          
    M.M[1][2] = tyz+twx;          
    M.M[2][2] = 1.0f-(txx+tyy);

    M.M[3][0] = M.M[3][1] = M.M[3][2] = 
    M.M[0][3] = M.M[1][3] = M.M[2][3] = 0.0f;
    M.M[3][3] = 1.0f;
}

//==========================================================================

inline radian AngleBetween( const quaternion& Qa, const quaternion& Qb )
{
/*
    f32 d;

    d = Dot( Qa, Qb );

    if ( d >  1.0f ) d =  1.0f;
    if ( d < -1.0f ) d = -1.0f;

    return x_acos( d );
*/

	// Correct quaternion angle(corrects for any scaling)
    f32 d;

    d = Dot( Qa, Qb ) / ( Dot( Qa, Qa) * Dot( Qb, Qb) );

    return x_acos( d );
}

//==========================================================================

inline radian quaternion::GetAngle( void ) const 
{
    radian angle;
    angle = W;
    if (angle >  1.0f) angle =  1.0f;
    if (angle < -1.0f) angle = -1.0f;
    return x_acos( angle )*2;
}

//==========================================================================

inline vector3 quaternion::GetAxis( void ) const
{
    vector3 V;
    radian  Angle;
    radian  s;

    Angle = W;

    if ( Angle >  1.0f ) Angle =  1.0f;
    if ( Angle < -1.0f ) Angle = -1.0f;

    s = x_acos( Angle );
    s = x_sin ( s );

    if ( s > -0.000001f && s < 0.000001f ) 
    {
        V.Zero();
    }
    else 
    {
        s = 1.0f/s;
        V.X = X*s;
        V.Y = Y*s;
        V.Z = Z*s;        
    }

    return V;
}

//==========================================================================

inline void quaternion::SetupAxisAngle( const vector3& Axis, radian R )
{
    f32 S, C;

    x_sincos(R/2.0f,S,C);

    W  = C;
    X  = Axis.X*S;
    Y  = Axis.Y*S;
    Z  = Axis.Z*S;
}

//==========================================================================

inline f32 Dot( const quaternion& Qa, const quaternion& Qb )
{
    f32 d;

    d = Qa.X * Qb.X +
        Qa.Y * Qb.Y +
        Qa.Z * Qb.Z +
        Qa.W * Qb.W;

    return d;
}

//==========================================================================

inline void quaternion::Normalize( void )
{
    f32 Mag;

    Mag = Length();
    Mag = 1.0f/Mag;

    X *= Mag;
    Y *= Mag;
    Z *= Mag;
    W *= Mag;
}

//==========================================================================

inline f32 quaternion::Length( void )
{
    return x_sqrt( X*X + Y*Y + Z*Z + W*W );
}

//==========================================================================

inline void quaternion::Negate( void )
{
    X = -X; Y = -Y; Z = -Z; W = -W;
}

//==========================================================================
inline quaternion quaternion::operator - ( void ) const
{
    quaternion lhs(*this);
    lhs.Negate();
    return ( lhs );
}

//==========================================================================

inline quaternion operator * ( const quaternion& Qa,  const quaternion& Qb )
{
    quaternion Q;

    Q.W =   (  Qa.W  *  Qb.W  ) -
            (  Qa.X  *  Qb.X  ) -
            (  Qa.Y  *  Qb.Y  ) -
            (  Qa.Z  *  Qb.Z  );

    Q.X =   (  Qa.W  *  Qb.X  ) +
            (  Qa.X  *  Qb.W  ) +
            (  Qa.Y  *  Qb.Z  ) -
            (  Qa.Z  *  Qb.Y  );

    Q.Y =   (  Qa.W  *  Qb.Y  ) +
            (  Qa.Y  *  Qb.W  ) +
            (  Qa.Z  *  Qb.X  ) -
            (  Qa.X  *  Qb.Z  );

    Q.Z =   (  Qa.W  *  Qb.Z  ) +
            (  Qa.Z  *  Qb.W  ) +
            (  Qa.X  *  Qb.Y  ) -
            (  Qa.Y  *  Qb.X  );

    return Q;
}

//==========================================================================
// The Quat transformation routine use 19 multiplies and 12 adds
// (counting the multiplications by 2.0).  See Eqn (20) of "A
// Comparison of Transforms and Quaternions in Robotics," Funda and
// Paul, Proceedings of International Conference on Robotics and
// Automation, 1988, p. 886-991.
//==========================================================================

inline vector3 operator * ( const quaternion& Q, const vector3& V )
{   
    vector3 Axis;
    vector3 uv, uuv;

    Axis.Set(Q.X, Q.Y, Q.Z);
    uv    = Cross( Axis, V );         
    uuv   = Cross( Axis, uv );         
    uv   *= Q.W * 2.0f;     
    uuv  *= 2.0f;            
    Axis  = V + uv;
    Axis += uuv;

    return Axis;
}

//==========================================================================

inline quaternion operator * ( const quaternion& aQ, f32 S )
{
    quaternion Q;
    Q.X = aQ.X * S;
    Q.Y = aQ.Y * S;
    Q.Z = aQ.Z * S;
    Q.W = aQ.W * S;
    return Q;
}

//==========================================================================

inline quaternion operator * ( f32 S, const quaternion& aQ )
{
    quaternion Q;
    Q.X = aQ.X * S;
    Q.Y = aQ.Y * S;
    Q.Z = aQ.Z * S;
    Q.W = aQ.W * S;
    return Q;
}

//==========================================================================

inline quaternion operator + ( const quaternion& Qa, const quaternion& Qb )
{
    quaternion Q;
    Q.X = Qa.X + Qb.X;
    Q.Y = Qa.Y + Qb.Y;
    Q.Z = Qa.Z + Qb.Z;
    Q.W = Qa.W + Qb.W;
    return Q;
}

//==========================================================================

inline quaternion operator - ( const quaternion& Qa, const quaternion& Qb )
{
    quaternion Q;
    Q.X = Qa.X - Qb.X;
    Q.Y = Qa.Y - Qb.Y;
    Q.Z = Qa.Z - Qb.Z;
    Q.W = Qa.W - Qb.W;
    return Q;
}

//==========================================================================

inline radian Pitch( const quaternion& Q )
{
    return Pitch( Q * vector3(0.0f,0.0f,1.0f) );
}

//==========================================================================

inline radian Yaw( const quaternion& Q )
{
    return Yaw( Q * vector3(0.0f,0.0f,1.0f) );
}

//==========================================================================

inline void PitchYaw( const quaternion& Q, radian& Pitch, radian& Yaw )
{
    PitchYaw( Q * vector3(0.0f,0.0f,1.0f), Pitch, Yaw );
}

//==========================================================================

inline quaternion BlendSpherical( const quaternion& Qa, const quaternion& Qb, f32 T )
{
    quaternion Q;
    quaternion QT0;
    quaternion QT1;
    f32 Cs,Sn;
    f32 Angle,InvSn,TAngle;
    f32 C0,C1;

    // Copy quaternions into temp
    QT0 = Qa;
    QT1 = Qb;

    // Cosine of angle between Q0,Q1... dot product
    Cs = (QT0.X*QT1.X) + 
         (QT0.Y*QT1.Y) + 
         (QT0.Z*QT1.Z) + 
         (QT0.W*QT1.W);

    // Check if we need to interpolate the other way around
    if (Cs <= 0.0f)
    {
        QT1.X = -QT1.X;
        QT1.Y = -QT1.Y;
        QT1.Z = -QT1.Z;
        QT1.W = -QT1.W;
        Cs    = (QT0.X*QT1.X) + 
                (QT0.Y*QT1.Y) + 
                (QT0.Z*QT1.Z) + 
                (QT0.W*QT1.W);
    }

    // Compute sine of angle between Q0,Q1
    Sn = 1.0f - Cs*Cs;
    if (Sn < 0.0f) Sn = -Sn;
    Sn = x_sqrt( Sn );

    // Check if quaternions are very close together
    if ( (Sn < 1e-3f) && (Sn > -1e-3f) )
    {
        return QT0;
    }

    Angle       = x_atan2( Sn, Cs );
    InvSn       = 1.0f/Sn;    
    TAngle      = T*Angle;

    C0      = x_sin( Angle - TAngle) * InvSn;
    C1      = x_sin( TAngle ) * InvSn;    

    Q.X   = C0*QT0.X + C1*QT1.X;
    Q.Y   = C0*QT0.Y + C1*QT1.Y;
    Q.Z   = C0*QT0.Z + C1*QT1.Z;
    Q.W   = C0*QT0.W + C1*QT1.W;

    return Q;
}

//==========================================================================

inline quaternion BlendSpherical2( const quaternion& Qa, const quaternion& Qb, f32 T )
{
    quaternion Qtemp1;
    quaternion Qtemp2;
    f32        C1, C2, S;
    f32        Theta;
    f32        CosTheta;


    Qtemp1 = Qa;
    Qtemp2 = Qb;

    CosTheta = Dot( Qa, Qb );
    // if B is on opposite hemisphere from A, use -B instead 
    if( CosTheta < 0.0f )
    {
        Qtemp2.Negate();
    }

    Theta   = AngleBetween( Qtemp1, Qtemp2 );
    S       = 1.0f/x_sin( Theta );
    C1      = x_sin( ( 1.0f - T ) * Theta ) * S;
    C2      = x_sin( T * Theta )         * S;

    Qtemp1.Scale( C1 );
    Qtemp2.Scale( C2 );

    return Qtemp1 + Qtemp2;
}

//==========================================================================
inline quaternion BlendLinear( const quaternion& Qa, const quaternion& Qb, f32 T, xbool bNormalize )
{
    quaternion Q;
    quaternion QT0;
    quaternion QT1;
    f32 Cs;

    QT0 = Qa;
    QT1 = Qb;

    // Cosine of angle between Q0,Q1... dot product
    Cs = (QT0.X*QT1.X) + 
         (QT0.Y*QT1.Y) + 
         (QT0.Z*QT1.Z) + 
         (QT0.W*QT1.W);

    // Check if we need to interpolate the other way around
    if (Cs <= 0.0f)
    {
        QT1.X = -QT1.X;
        QT1.Y = -QT1.Y;
        QT1.Z = -QT1.Z;
        QT1.W = -QT1.W;
    }

    // Do interpolation
    Q.X  = QT0.X  + T * ( QT1.X - QT0.X );
    Q.Y  = QT0.Y  + T * ( QT1.Y - QT0.Y );
    Q.Z  = QT0.Z  + T * ( QT1.Z - QT0.Z );
    Q.W  = QT0.W  + T * ( QT1.W - QT0.W );

    // make sure that is normalize
	if( bNormalize )
		Q.Normalize();

    return Q;
}

//==========================================================================

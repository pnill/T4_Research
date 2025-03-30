///////////////////////////////////////////////////////////////////////////
//                                                                       //
//  X_MATH.H                                                             //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_H
#define X_MATH_H

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//  The matrix structure in this math library uses the "New Matrix       //
//  Convention".  What does that mean?  Glad you asked.  Here are        //
//  examples:                                                            //
//                                                                       //
//  NEW STANDARD:                                                        //
//                                                                       //
//      X = 3x + 5y + 2z                                                 //
//      Y = 7x + 1y + 4z                                                 //
//      Z = 8x + 6y + 9z                                                 //
//                                                                       //
//      |X|   |3 5 2|   |x|                                              //
//      |Y| = |7 1 4| * |y|                                              //
//      |Z|   |8 6 9|   |z|                                              //
//                                                                       //
//      A "point" is a "column vector".  To transform a point using a    //
//      matrix, the point is "post-multiplied" with the matrix.          //
//                                                                       //
//      Note that operations performed by a series of matrices appear    //
//      right to left.  For example,                                     //
//                                                                       //
//      |X|   |  M  |   |  M  |   |  M  |   |x|                          //
//      |Y| = | op3 | * | op2 | * | op1 | * |y|                          //
//      |Z|   |     |   |     |   |     |   |z|                          //
//                                                                       //
//  OLD STANDARD:                                                        //
//                                                                       //
//      X = 3x + 5y + 2z                                                 //
//      Y = 7x + 1y + 4z                                                 //
//      Z = 8x + 6y + 9z                                                 //
//                                                                       //
//                          |3 7 8|                                      //
//      |X Y Z| = |x y z| * |5 1 6|                                      //
//                          |2 4 9|                                      //
//                                                                       //
//      A "point" is a "row vector".  To transform a point using a       //
//      matrix, the point is "pre-multiplied" with the matrix.           //
//                                                                       //
//      Note that operations performed by a series of matrices appear    //
//      left to right.  For example,                                     //
//                                                                       //
//                          |  M  |   |  M  |   |  M  |                  //
//      |X Y Z| = |x y z| * | op1 | * | op2 | * | op3 |                  //
//                          |     |   |     |   |     |                  //
//                                                                       //
//  Why did the standard change?  We're not sure.  It was probably       //
//  driven by a similar change in the mathematics field.  One            //
//  observation is that the layout of the coefficients in the "new"      //
//  matrix is the same as the layout of the coefficients in the example  //
//  system of equations.                                                 //
//                                                                       //
//                                                                       //
//  Matrix4 Layout:                                                      //
//                                                                       //
//        0    1    2    3                                               //
//      +----+----+----+----+                                            //
//    0 | Rx | Ry | Rz | Tx |   Tx = M[3][0]                             //
//      |----+----+----+----|                                            //
//    1 | Rx | Ry | Rz | Ty |   The x-component of a transformed         //
//      |----+----+----+----|   vector = Vx*M[0][0] +                    //
//    2 | Rx | Ry | Rz | Tz |            Vy*M[1][0] +                    //
//      |----+----+----+----|            Vz*M[2][0] +                    //
//    3 | 0  | 0  | 0  | 1  |               M[3][0];                     //
//      +----+----+----+----+                                            //
//                                                                       //
//  Linear memory layout:                                                //
//                                                                       //
//        0    1    2    3                                               //
//      +----+----+----+----+                                            //
//    0 | 00 | 04 | 08 | 12 |   Tx = M[3][0] = M[12]                     //
//      |----+----+----+----|                                            //
//    1 | 01 | 05 | 09 | 13 |                                            //
//      |----+----+----+----|                                            //
//    2 | 02 | 06 | 10 | 14 |                                            //
//      |----+----+----+----|                                            //
//    3 | 03 | 07 | 11 | 15 |                                            //
//      +----+----+----+----+                                            //
//                                                                       //
//  Interpretation of a Rotation/Translation matrix:                     //
//                                                                       //
//        0    1    2    3                                               //
//      +----+----+----+----+                                            //
//    0 | R0 | R3 | R6 | TX |   * R012 Src space X-axis in Dst space     //
//      |----+----+----+----|   * R345 Src space Y-axis in Dst space     //
//    1 | R1 | R4 | R7 | TY |   * R678 Src space Z-axis in Dst space     //
//      |----+----+----+----|   * R036 Dst space X-axis in Src space     //
//    2 | R2 | R5 | R8 | TZ |   * R147 Dst space Y-axis in Src space     //
//      |----+----+----+----|   * R258 Dst space Z-axis in Src space     //
//    3 |  0 |  0 |  0 |  1 |   * TXYZ Src space origin in Dst space     //
//      +----+----+----+----+                                            //
//                                                                       //
//  Example: Camera Space (C) to World Space (W) transformation          //
//           C2W = T(EyePosInWorld) * RY(Yaw) * RX(Pitch) * RZ(Roll)     //
//                                                                       //
//           Src space is Camera, Dst space is World.  R012 will be      //
//           the camera's x-axis in world space and TXYZ will be the     //
//           origin of Camera space (0,0,0) in World space... or the     //
//           position of the Camera's eye.                               //
//                                                                       //
//  Example: Given a view, what is the "line of sight" expressed as a    //
//           unit vector in world coordinates?                           //
//                                                                       //
//           The "line of sight" unit vector in View space is {0,0,1}.   //
//           We need the matrix which converts from View to World...     //
//                                                                       //
//              matrix4  V2W;                                            //
//              VIEW_GetV2WMatrix( View, &V2W );                         //   
//                                                                       //
//           We need {0,0,1} (in View space) to be expressed in World    //
//           space.  Since the vector components are 0's and a 1, the    //
//           conversion (which is a linear combination) is easy!         //
//                                                                       //
//              vector3d LOS;  // Line Of Sight                          //   
//              LOS.X = V2W.M[2][0];                                     //
//              LOS.Y = V2W.M[2][1];                                     //
//              LOS.Z = V2W.M[2][2];                                     //
//                                                                       //
//           If we had wanted the direction vector {1,4,9} converted     //
//           from View to World...                                       //
//                                                                       //
//              vector3d V;  // some vector                              //
//              V.X = 1*V2W.M[0][0] + 4*V2W.M[1][0] + 9*V2W.M[2][0];     //
//              V.Y = 1*V2W.M[0][1] + 4*V2W.M[1][1] + 9*V2W.M[2][1];     //
//              V.Z = 1*V2W.M[0][2] + 4*V2W.M[1][2] + 9*V2W.M[2][2];     //
//                                     |           |                     //  
//                                     +-----------+                     //
//                                           |                           //
//           By the way, this triple --------+ is the View's Y-axis      //
//           (or the View's "up" vector) expressed in World space.       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//  INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_H
#include "x_types.h"
#endif

#ifdef TARGET_PC
#include <float.h>
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
//  DEFINES / MACROS
///////////////////////////////////////////////////////////////////////////

#ifdef PI
    #undef PI
#endif
#define PI      ((f32)3.141592653589793238462643)

#define RAD_TO_DEG(A) ((f32)(((A) * (180.0f / PI))))
#define DEG_TO_RAD(A) ((f32)(((A) * (PI / 180.0f))))

#define R_0     DEG_TO_RAD(  0)
#define R_1     DEG_TO_RAD(  1)
#define R_2     DEG_TO_RAD(  2)
#define R_3     DEG_TO_RAD(  3)
#define R_4     DEG_TO_RAD(  4)
#define R_5     DEG_TO_RAD(  5)
#define R_6     DEG_TO_RAD(  6)
#define R_7     DEG_TO_RAD(  7)
#define R_8     DEG_TO_RAD(  8)
#define R_9     DEG_TO_RAD(  9)
#define R_10    DEG_TO_RAD( 10)
#define R_11    DEG_TO_RAD( 11)
#define R_12    DEG_TO_RAD( 12)
#define R_13    DEG_TO_RAD( 13)
#define R_14    DEG_TO_RAD( 14)
#define R_15    DEG_TO_RAD( 15)
#define R_30    DEG_TO_RAD( 30)
#define R_45    DEG_TO_RAD( 45)
#define R_60    DEG_TO_RAD( 60)
#define R_89    DEG_TO_RAD( 89)
#define R_90    DEG_TO_RAD( 90)
#define R_120   DEG_TO_RAD(120)
#define R_135   DEG_TO_RAD(135)
#define R_150   DEG_TO_RAD(150)
#define R_180   DEG_TO_RAD(180)
#define R_210   DEG_TO_RAD(210)
#define R_225   DEG_TO_RAD(225)
#define R_240   DEG_TO_RAD(240)
#define R_270   DEG_TO_RAD(270)
#define R_300   DEG_TO_RAD(300)
#define R_315   DEG_TO_RAD(315)
#define R_330   DEG_TO_RAD(330)
#define R_360   DEG_TO_RAD(360)

///////////////////////////////////////////////////////////////////////////
//  TYPES
///////////////////////////////////////////////////////////////////////////

typedef struct
{
    f32 X, Y, Z;
} vector3d;


typedef struct
{
    f32 M[4][4];
} matrix4;

typedef struct
{
    vector3d Axis;
    f32      S;
} quaternion;


typedef f32 radian;


typedef struct
{
    radian X, Y, Z;
} radian3d;

///////////////////////////////////////////////////////////////////////////
//  INITIALIZATION
///////////////////////////////////////////////////////////////////////////

void    x_InitMATH( void );
void    x_KillMATH( void );

///////////////////////////////////////////////////////////////////////////
//  BASIC MATH FUNCTIONS
///////////////////////////////////////////////////////////////////////////

f32     x_sqr                   ( f32 x             );
f32     x_sqrt                  ( f32 x             );
f32     x_1oversqrt             ( f32 x             );
f32     x_flpr                  ( f32 x, f32  Range );
f64     x_floor                 ( f64 x             );
f64     x_ceil                  ( f64 x             );
f64     x_fmod                  ( f64 x, f64  y     );
f64     x_modf                  ( f64 x, f64* whole );
f32     x_fabs                  ( f32 x             );
s32     x_abs                   ( s32 x             );
f64     x_log                   ( f64 x             );
f64     x_log10                 ( f64 x             );
f64     x_exp                   ( f64 x             );
f64     x_pow                   ( f64 x, f64 y      );

f64     x_frexp                 ( f64 value, s32* eptr );
f64     x_ldexp                 ( f64 x,     s32  exp  );

///////////////////////////////////////////////////////////////////////////
//  TRIGONOMETRIC MATH
///////////////////////////////////////////////////////////////////////////

f32     x_sin                   ( radian x     );
f32     x_cos                   ( radian x     );
void    x_sincos                ( radian x, f32* S, f32* C );
f32     x_tan                   ( radian x     );
f64     x_tanh                  ( f64 x        );
radian  x_atan                  ( f32 x        );
radian  x_atan2                 ( f32 y, f32 x );
radian  x_asin                  ( f32 x        );
radian  x_acos                  ( f32 x        );
f32     x_fastsin               ( radian x     );
f32     x_fastcos               ( radian x     );

// Modulate angles: ModAngle[0,2*PI) ModAngle2(-PI,PI]
radian  x_ModAngle              ( radian x     );
radian  x_ModAngle2             ( radian x     );
radian  x_MinAngleDiff          ( radian a, radian b );

///////////////////////////////////////////////////////////////////////////
//  ROTATION CONVERSIONS
///////////////////////////////////////////////////////////////////////////

void    x_XYZToZXYRot           ( radian3d* Rotations );
void    x_ZXYToXYZRot           ( radian3d* Rotations );
void    x_XYZToXZYRot           ( radian3d* Rotations );
void    x_XZYToXYZRot           ( radian3d* Rotations );

///////////////////////////////////////////////////////////////////////////
//  VECTOR 3D
///////////////////////////////////////////////////////////////////////////

void    V3_Zero                 ( vector3d* V );
void    V3_Set                  ( vector3d* V, f32  X, f32  Y, f32  Z );
void    V3_Get                  ( vector3d* V, f32* X, f32* Y, f32* Z );
void    V3_Copy                 ( vector3d* Dest, vector3d* Src );
void    V3_Add                  ( vector3d* Dest, vector3d* Va, vector3d* Vb );
void    V3_Sub                  ( vector3d* Dest, vector3d* Va, vector3d* Vb );
void    V3_Scale                ( vector3d* Dest, vector3d* V, f32 S );
void    V3_Negate               ( vector3d* Dest, vector3d* Src );
f32     V3_Distance             ( vector3d* Va, vector3d* Vb );
f32     V3_Dot                  ( vector3d* Va, vector3d* Vb );
void    V3_Cross                ( vector3d* Dest, vector3d* Va, vector3d* Vb );
f32     V3_Length               ( vector3d* V );
void    V3_Normalize            ( vector3d* V );
radian  V3_AngleBetween         ( vector3d* Va, vector3d* Vb );
void    V3_RotateX              ( vector3d* Dest, vector3d* V, radian Rx );
void    V3_RotateY              ( vector3d* Dest, vector3d* V, radian Ry );
void    V3_RotateZ              ( vector3d* Dest, vector3d* V, radian Rz );
void    V3_ZToVecRot            ( vector3d* V, radian* Rx, radian* Ry );
void    V3_VecToZRot            ( vector3d* V, radian* Rx, radian* Ry );

///////////////////////////////////////////////////////////////////////////
//  Projection:  PXC0, PXC1, PYC0, PYC1 supplied by view
//  SX = PXC0 + PXC1*VX/VZ;
//  SY = PYC0 + PYC1*VY/VZ;
//  SZ = VY;
///////////////////////////////////////////////////////////////////////////

void    V3_Project              ( vector3d* Dest, vector3d* Src, s32 NVerts,
                                  f32 PXC0, f32 PXC1, f32 PYC0, f32 PYC1 );

///////////////////////////////////////////////////////////////////////////
//  VECTOR 3D / PLANE FUNCTIONS
//  Front of plane = (X*NX + Y*NY + Z*NZ + D >= 0)
///////////////////////////////////////////////////////////////////////////

void    V3_ClipLineSegment      ( vector3d* CV, vector3d* V0, vector3d* V1,
                                  f32 NX, f32 NY, f32 NZ, f32 D );

xbool   V3_ClipPoly             ( vector3d* Dest, s32* NDest, 
                                  vector3d* Src,  s32  NSrc, 
                                  f32 NX, f32 NY, f32 NZ, f32 D );

///////////////////////////////////////////////////////////////////////////
//  MATRIX4
///////////////////////////////////////////////////////////////////////////

void    M4_Copy                 ( matrix4* Dest, matrix4* Src );
void    M4_Set                  ( matrix4* M, f32 M11, f32 M21, f32 M31, f32 M41,
                                              f32 M12, f32 M22, f32 M32, f32 M42,
                                              f32 M13, f32 M23, f32 M33, f32 M43,
                                              f32 M14, f32 M24, f32 M34, f32 M44 );
void    M4_Zero                 ( matrix4* M );
void    M4_Identity             ( matrix4* M );
void    M4_Transpose            ( matrix4* M );
void    M4_Orthogonalize        ( matrix4* M );

///////////////////////////////////////////////////////////////////////////
//  MATRIX4 - INVERSION
//  M4_Invert    - Heavy duty gaussian elmination and back substitution
//  M4_SRTInvert - Quick invert for Scale Rotate Translate matrices
///////////////////////////////////////////////////////////////////////////

err     M4_Invert               ( matrix4* Dest, matrix4* Src );
err     M4_SRTInvert            ( matrix4* Dest, matrix4* Src );

void    M4_SetScale             ( matrix4* M, vector3d* Scale );
void    M4_ScaleOn              ( matrix4* M, vector3d* Scale );
void    M4_SetUniScale          ( matrix4* M, f32 Scale );
void    M4_UniScaleOn           ( matrix4* M, f32 Scale );

void    M4_ClearRotations       ( matrix4* M );
void    M4_SetRotationsXYZ      ( matrix4* M, radian3d* Rotation );
void    M4_SetRotationsZXY      ( matrix4* M, radian3d* Rotation );
void	M4_SetRotationsXZY		( matrix4* M, radian3d* Rotation );
void    M4_GetRotationsXYZ      ( matrix4* M, radian3d* Rotation );
void    M4_GetRotationsZXY      ( matrix4* M, radian3d* Rotation );
void	M4_GetRotationsXZY		( matrix4* M, radian3d* Rotation );
void    M4_RotateXYZOn          ( matrix4* M, radian3d* Rotation );
void    M4_RotateXOn            ( matrix4* M, radian Rx );
void    M4_RotateYOn            ( matrix4* M, radian Ry );
void    M4_RotateZOn            ( matrix4* M, radian Rz );

void    M4_ClearTranslations    ( matrix4* M );
void    M4_SetTranslation       ( matrix4* M, vector3d* Translation );
void    M4_GetTranslation       ( matrix4* M, vector3d* Translation );
void    M4_TranslateOn          ( matrix4* M, vector3d* Translation );

void    M4_SetRows              ( matrix4* M, vector3d* Va, vector3d* Vb, vector3d* Vc );
void    M4_GetRows              ( matrix4* M, vector3d* Va, vector3d* Vb, vector3d* Vc );
void    M4_SetColumns           ( matrix4* M, vector3d* Va, vector3d* Vb, vector3d* Vc );
void    M4_GetColumns           ( matrix4* M, vector3d* Va, vector3d* Vb, vector3d* Vc );

void    M4_Mult                 ( matrix4* Dest, matrix4* Left, matrix4* Right );
void    M4_PreMultOn            ( matrix4* Dest, matrix4* M );
void    M4_PostMultOn           ( matrix4* Dest, matrix4* M );

void    M4_TransformVerts       ( matrix4* M, vector3d* Dest, vector3d* Src, s32 NVerts );

void    M4_SetupRotation        ( matrix4* M, vector3d* Normal, radian R );
void    M4_SetupSRT             ( matrix4* Dest, vector3d* Scale,
                                                 radian3d* Rotation,
                                                 vector3d* Translation ); 

///////////////////////////////////////////////////////////////////////////
// MATRIX3 
// Matrix3 uses matrix4 for compatibility.
///////////////////////////////////////////////////////////////////////////

err     M3_Invert               ( matrix4* Dest, matrix4* Src );
void    M3_Mult                 ( matrix4* Dest, matrix4* Src1, matrix4* Src2 );
void    M3_PreMultOn            ( matrix4* Dest, matrix4* Src );
void    M3_PostMultOn           ( matrix4* Dest, matrix4* Src );
void    M3_Negate               ( matrix4* Dest, matrix4* Src );
void    M3_Add                  ( matrix4* Dest, matrix4* Src1, matrix4* Src2 );
void    M3_Sub                  ( matrix4* Dest, matrix4* Src1, matrix4* Src2 );
void    M3_Transpose            ( matrix4* Dest, matrix4* Src );
void    M3_Symmetrize           ( matrix4* Dest, matrix4* Src );
void    M3_MatrixScale          ( matrix4* Dest, f32 Scale, matrix4* Src );
void    M3_TransformVect        ( matrix4* M, vector3d* Dest, vector3d* Src );
void    M3_InvTransformVect     ( matrix4* M, vector3d* Dest, vector3d* Src );
void    M3_SetupRotation        ( matrix4* M, vector3d* Normal, radian R );
void    M3_SetSkew              ( matrix4* M, vector3d* V );
f32     M3_Determinant          ( matrix4* M );
void    M3_Orthogonalize        ( matrix4* M );


///////////////////////////////////////////////////////////////////////////
//  QUATERNION
///////////////////////////////////////////////////////////////////////////

void    Q_Copy                  ( quaternion* Dest, quaternion* Src );
void    Q_Zero                  ( quaternion* Q );

void    Q_Set                   ( quaternion* Q, f32  X, f32  Y, f32  Z, f32  S );
void    Q_Get                   ( quaternion* Q, f32* X, f32* Y, f32* Z, f32* S );

void    Q_Identity              ( quaternion* Q );
void    Q_Normalize             ( quaternion* Q );
void    Q_Negate                ( quaternion* Dest, quaternion* Src );
void    Q_Invert                ( quaternion* Dest, quaternion* Src );

void    Q_Add                   ( quaternion* Dest, quaternion* Qa, quaternion* Qb ); 
void    Q_Sub                   ( quaternion* Dest, quaternion* Qa, quaternion* Qb ); 
void    Q_Scale                 ( quaternion* Dest, quaternion* Qa, f32 S );          

void    Q_Mult                  ( quaternion* Dest, quaternion* Qa,  quaternion* Qb );
void    Q_PreMultOn             ( quaternion* Dest, quaternion* Q );
void    Q_PostMultOn            ( quaternion* Dest, quaternion* Q );

f32     Q_Dot                   ( quaternion* Qa, quaternion* Qb );                   
f32     Q_Length                ( quaternion* Q );                                    
radian  Q_AngleBetween          ( quaternion* Qa, quaternion* Qb );                   
void    Q_TransformVect         ( quaternion* Q, vector3d* Dest, vector3d* Src );
void    Q_InvTransformVect      ( quaternion* Q, vector3d* Dest, vector3d* Src );

void    Q_SetupFromMatrix       ( quaternion* Q, matrix4* Matrix );
void    Q_SetupFromAxisAngle    ( quaternion* Q, vector3d* UnitAxis, radian R );
void    Q_SetupFromAngles       ( quaternion* Q, radian Rx, radian Ry, radian Rz );

void    Q_SetupMatrix           ( quaternion* Q, matrix4* M );

// Blend from Qa to Qb
void    Q_BlendLinear           ( quaternion* Dest, quaternion* Qa, quaternion* Qb, f32 T ); 
void    Q_BlendSpherical        ( quaternion* Dest, quaternion* Qa, quaternion* Qb, f32 T ); 

void    Q_GetAxis               ( quaternion* Q, vector3d* V );                              
radian  Q_GetAngle              ( quaternion* Q );                                           

///////////////////////////////////////////////////////////////////////////
// Return the quaternion derivatives in 'Dest', given a current
// orientation Src and body angular velocity w.  Note that what is
// returned in 'Dest' is not a unit quaternion, but the derivative
// of one!
///////////////////////////////////////////////////////////////////////////
void    Q_Derivative            ( quaternion* Dest, quaternion* Src, vector3d* W );


///////////////////////////////////////////////////////////////////////////
// TRIANGLE MATH
///////////////////////////////////////////////////////////////////////////

xbool TRI_RayTriIntersect        ( f32*      t,
                                   vector3d* RayOrigin,
                                   vector3d* RayDir,
                                   vector3d* V1,
                                   vector3d* V2,
                                   vector3d* V3 );

xbool TRI_TriTriIntersect        ( vector3d* T1V1,
                                   vector3d* T1V2,
                                   vector3d* T1V3,
                                   vector3d* T2V1,
                                   vector3d* T2V2,
                                   vector3d* T2V3 );

xbool TRI_IsBackFace             ( vector3d* ScreenXY_1,
                                   vector3d* ScreenXY_2,
                                   vector3d* ScreenXY_3 );

void TRI_Normal                  ( vector3d* N,     // Unit normal of triangle
                                   vector3d* V1,
                                   vector3d* V2,
                                   vector3d* V3 );

void TRI_Plane                   ( vector3d* N,     // Normal of plane
                                   f32*      D,     // Dist to plane along normal
                                   vector3d* V1,    // FRONT = (NX*X + NY*Y + NZ*Z - D > 0)
                                   vector3d* V2,
                                   vector3d* V3 );

////////////////////////////////////////////////////////////////////////////////
//  TRIG MACROS
////////////////////////////////////////////////////////////////////////////////

#ifndef TARGET_SDC

extern s16  x_SinTable[4096];
#define x_fastsin(x) ((f32)x_SinTable[  ((s32)((x)*(2048/PI)))         & 0x00000FFF] * (1/32000.0f))
#define x_fastcos(x) ((f32)x_SinTable[ (((s32)((x)*(2048/PI))) + 1024) & 0x00000FFF] * (1/32000.0f))

#else

#define x_fastsin(x) x_sin(x)
#define x_fastcos(x) x_cos(x)

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////

#endif


////////////////////////////////////////////////////////////////////////////
//
// x_math.hpp
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_HPP
#define X_MATH_HPP

////////////////////////////////////////////////////////////////////////////
//                                                                        //
//                                                                        //
//  The matrix structure in this math library uses the "New Matrix        //
//  Convention".  What does that mean?  Glad you asked.  Here are         //
//  examples:                                                             //
//                                                                        //
//  NEW STANDARD:                                                         //
//                                                                        //
//      X = 3x + 5y + 2z                                                  //
//      Y = 7x + 1y + 4z                                                  //
//      Z = 8x + 6y + 9z                                                  //
//                                                                        //
//      |X|   |3 5 2|   |x|                                               //
//      |Y| = |7 1 4| * |y|                                               //
//      |Z|   |8 6 9|   |z|                                               //
//                                                                        //
//      A "point" is a "column vector".  To transform a point using a     //
//      matrix, the point is "post-multiplied" with the matrix.           //
//                                                                        //
//      Note that operations performed by a series of matrices appear     //
//      right to left.  For example,                                      //
//                                                                        //
//      |X|   |  M  |   |  M  |   |  M  |   |x|                           //
//      |Y| = | op3 | * | op2 | * | op1 | * |y|                           //
//      |Z|   |     |   |     |   |     |   |z|                           //
//                                                                        //
//  OLD STANDARD:                                                         //
//                                                                        //
//      X = 3x + 5y + 2z                                                  //
//      Y = 7x + 1y + 4z                                                  //
//      Z = 8x + 6y + 9z                                                  //
//                                                                        //
//                          |3 7 8|                                       //
//      |X Y Z| = |x y z| * |5 1 6|                                       //
//                          |2 4 9|                                       //
//                                                                        //
//      A "point" is a "row vector".  To transform a point using a        //
//      matrix, the point is "pre-multiplied" with the matrix.            //
//                                                                        //
//      Note that operations performed by a series of matrices appear     //
//      left to right.  For example,                                      //
//                                                                        //
//                          |  M  |   |  M  |   |  M  |                   //
//      |X Y Z| = |x y z| * | op1 | * | op2 | * | op3 |                   //
//                          |     |   |     |   |     |                   //
//                                                                        //
//  Why did the standard change?  We're not sure.  It was probably        //
//  driven by a similar change in the mathematics field.  One             //
//  observation is that the layout of the coefficients in the "new"       //
//  matrix is the same as the layout of the coefficients in the example   //
//  system of equations.                                                  //
//                                                                        //
//                                                                        //
//  Matrix4 Layout:                                                       //
//                                                                        //
//        0    1    2    3                                                //
//      +----+----+----+----+                                             //
//    0 | Rx | Ry | Rz | Tx |   Tx = M[3][0]                              //
//      |----+----+----+----|                                             //
//    1 | Rx | Ry | Rz | Ty |   The x-component of a transformed          //
//      |----+----+----+----|   vector = Vx*M[0][0] +                     //
//    2 | Rx | Ry | Rz | Tz |            Vy*M[1][0] +                     //
//      |----+----+----+----|            Vz*M[2][0] +                     //
//    3 | 0  | 0  | 0  | 1  |               M[3][0];                      //
//      +----+----+----+----+                                             //
//                                                                        //
//  Linear memory layout:                                                 //
//                                                                        //
//        0    1    2    3                                                //
//      +----+----+----+----+                                             //
//    0 | 00 | 04 | 08 | 12 |   Tx = M[3][0] = M[12]                      //
//      |----+----+----+----|                                             //
//    1 | 01 | 05 | 09 | 13 |                                             //
//      |----+----+----+----|                                             //
//    2 | 02 | 06 | 10 | 14 |                                             //
//      |----+----+----+----|                                             //
//    3 | 03 | 07 | 11 | 15 |                                             //
//      +----+----+----+----+                                             //
//                                                                        //
//  Interpretation of a Rotation/Translation matrix:                      //
//                                                                        //
//        0    1    2    3                                                //
//      +----+----+----+----+                                             //
//    0 | R0 | R3 | R6 | TX |   * R012 Src space X-axis in Dst space      //
//      |----+----+----+----|   * R345 Src space Y-axis in Dst space      //
//    1 | R1 | R4 | R7 | TY |   * R678 Src space Z-axis in Dst space      //
//      |----+----+----+----|   * R036 Dst space X-axis in Src space      //
//    2 | R2 | R5 | R8 | TZ |   * R147 Dst space Y-axis in Src space      //
//      |----+----+----+----|   * R258 Dst space Z-axis in Src space      //
//    3 |  0 |  0 |  0 |  1 |   * TXYZ Src space origin in Dst space      //
//      +----+----+----+----+                                             //
//                                                                        //
//  Example: Camera Space (C) to World Space (W) transformation           //
//           C2W = T(EyePosInWorld) * RY(Yaw) * RX(Pitch) * RZ(Roll)      //
//                                                                        //
//           Src space is Camera, Dst space is World.  R012 will be       //
//           the camera's x-axis in world space and TXYZ will be the      //
//           origin of Camera space (0,0,0) in World space... or the      //
//           position of the Camera's eye.                                //
//                                                                        //
//  Example: Given a view, what is the "line of sight" expressed as a     //
//           unit vector in world coordinates?                            //
//                                                                        //
//           The "line of sight" unit vector in View space is {0,0,1}.    //
//           We need the matrix which converts from View to World...      //
//                                                                        //
//              matrix4  V2W;                                             //
//              VIEW_GetV2WMatrix( View, &V2W );                          //
//                                                                        //
//           We need {0,0,1} (in View space) to be expressed in World     //
//           space.  Since the vector components are 0's and a 1, the     //
//           conversion (which is a linear combination) is easy!          //
//                                                                        //
//              vector3d LOS;  // Line Of Sight                           //
//              LOS.X = V2W.M[2][0];                                      //
//              LOS.Y = V2W.M[2][1];                                      //
//              LOS.Z = V2W.M[2][2];                                      //
//                                                                        //
//           If we had wanted the direction vector {1,4,9} converted      //
//           from View to World...                                        //
//                                                                        //
//              vector3d V;  // some vector                               //
//              V.X = 1*V2W.M[0][0] + 4*V2W.M[1][0] + 9*V2W.M[2][0];      //
//              V.Y = 1*V2W.M[0][1] + 4*V2W.M[1][1] + 9*V2W.M[2][1];      //
//              V.Z = 1*V2W.M[0][2] + 4*V2W.M[1][2] + 9*V2W.M[2][2];      //
//                                     |           |                      //
//                                     +-----------+                      //
//                                           |                            //
//           By the way, this triple --------+ is the View's Y-axis       //
//           (or the View's "up" vector) expressed in World space.        //
//                                                                        //
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"


#if defined( TARGET_PC ) || defined( TARGET_XBOX )
    //#include <float.h>
    #include <math.h>
#endif


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#ifdef PI
#undef PI
#endif
#define PI  (3.141592653589793238462643f)

#define RADIAN(A)     ((f32)(((A) * (PI / 180.0f))))
#define RAD_TO_DEG(A) ((f32)(((A) * (180.0f / PI))))
#define DEG_TO_RAD(A) ((f32)(((A) * (PI / 180.0f))))

#define R_0     RADIAN(  0.0f)
#define R_1     RADIAN(  1.0f)
#define R_2     RADIAN(  2.0f)
#define R_3     RADIAN(  3.0f)
#define R_4     RADIAN(  4.0f)
#define R_5     RADIAN(  5.0f)
#define R_6     RADIAN(  6.0f)
#define R_7     RADIAN(  7.0f)
#define R_8     RADIAN(  8.0f)
#define R_9     RADIAN(  9.0f)
#define R_10    RADIAN( 10.0f)
#define R_11    RADIAN( 11.0f)
#define R_12    RADIAN( 12.0f)
#define R_13    RADIAN( 13.0f)
#define R_14    RADIAN( 14.0f)
#define R_15    RADIAN( 15.0f)
#define R_22_5	RADIAN(22.5f)
#define R_30    RADIAN( 30.0f)
#define R_45    RADIAN( 45.0f)
#define R_50	RADIAN( 50.0f)
#define R_60    RADIAN( 60.0f)
#define R_70	RADIAN( 70.0f)
#define R_89    RADIAN( 89.0f)
#define R_90    RADIAN( 90.0f)
#define R_100	RADIAN(100.0f)
#define R_110	RADIAN(110.0f)
#define R_120   RADIAN(120.0f)
#define R_135   RADIAN(135.0f)
#define R_150   RADIAN(150.0f)
#define R_155	RADIAN(155.0f)
#define R_160	RADIAN(160.0f)
#define R_170	RADIAN(170.0f)
#define R_180   RADIAN(180.0f)
#define R_205	RADIAN(205.0f)
#define R_210   RADIAN(210.0f)
#define R_225   RADIAN(225.0f)
#define R_240   RADIAN(240.0f)
#define R_270   RADIAN(270.0f)
#define R_280	RADIAN(280.0f)
#define R_290	RADIAN(290.0f)
#define R_300   RADIAN(300.0f)
#define R_315   RADIAN(315.0f)
#define R_330   RADIAN(330.0f)
#define R_340	RADIAN(340.0f)
#define R_350	RADIAN(350.0f)
#define R_360   RADIAN(360.0f)


////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////

typedef f32 radian;

struct vector2;
struct vector3;
struct vector4;
struct radian3;
struct matrix4;
struct quaternion;

//struct plane;
//struct triangle;

////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------
//  Basic math functions.
//--------------------------------------------------------------------------
//
//  These functions are generally patterned after any comperable ANSI
//  standard C library functions.  Pay careful attention to the use of f32
//  ("float") versus f64 ("double") versus s32 ("int").
//
//  x_sqr       - Square.
//  x_sqrt      - Square root.
//  x_floor     - Floor.
//  x_ceil      - Ceiling.
//  x_log       - Log base e.  Natural log.
//  x_log10     - Log base 10.
//  x_exp       - Raise e to a power.
//  x_pow       - Raise a number to a power.
//  x_fmod      - True modulus between two numbers.
//  x_modf      - Break number into whole (integer) and fractional parts.
//  x_frexp     - Break number into exponent and mantissa.
//  x_ldexp     - Assemble number from exponent and mantissa.
//
//  Additional functions.
//
//  x_fastlog2  - Fast approximation of Log base 2 (i.e. power of two)
//  x_fastlog2f - single-precision version of x_fastlog2
//  x_1oversqrt - One over square root.  Optimized per platform.
//  x_lpr       - Least Positive Residue.  Non-negative modulus value.
//  x_abs       - Absolute value of any signed numeric type.
//
//  The ANSI functions abs() and fabs() are both satisfied by x_abs() which 
//  is overloaded for each signed numeric type.
//
//--------------------------------------------------------------------------

f32     x_sqr           ( f32 a              );
f32     x_sqrt          ( f32 a              );
f64     x_floor         ( f64 a              );
f64     x_ceil          ( f64 a              );
f64     x_log           ( f64 a              );
f64     x_log10         ( f64 a              );
f64     x_log2          ( f64 a              );
f64     x_exp           ( f64 a              );
f64     x_pow           ( f64 a, f64  b      );
f64     x_fmod          ( f64 a, f64  b      );
f64     x_modf          ( f64 a, f64* pWhole );
f64     x_frexp         ( f64 a, s32* pExp   );
f64     x_ldexp         ( f64 a, s32  Exp    );

f64     x_fastlog2      ( f64 a              );
f32     x_fastlog2f     ( f32 a              );
f32     x_1oversqrt     ( f32 a              );
f32     x_lpr           ( f32 a, f32  Range  );

s8      x_abs           ( s8  a );
s16     x_abs           ( s16 a );
s32     x_abs           ( s32 a );
s64     x_abs           ( s64 a );
f32     x_abs           ( f32 a );
f64     x_abs           ( f64 a );

#if defined( TARGET_PC ) || defined( TARGET_XBOX )
    // can't get the x functions to work on the PC
    #define x_floor( a )    floor( a )
    #define x_ceil( a )     ceil( a )
#endif

//--------------------------------------------------------------------------
//  Trigonometric math functions.
//--------------------------------------------------------------------------
//
//  These functions are generally patterned after any comperable ANSI
//  standard C library functions.  Pay careful attention to the use of f32
//  versus radian.
//
//  x_sin          - Sine.
//  x_cos          - Cosine.
//  x_tan          - Tangent.
//  x_tanh         - Hyperbolic tangent.
//  x_asin         - Arc sine.
//  x_acos         - Arc cosine.
//  x_atan         - Arc tangent.
//  x_atan2        - Standard "atan2" arc tangent.  (y can equal 0.)
//
//  Additional functions.
//
//  x_sincos       - Sine and cosine in one function call.
//  x_fastsincos   - Fast version of x_sincos
//  x_fastsin      - Fast approximation of sine.
//  x_fastcos      - Fast approximation of cosine.
//  x_ModAngle     - Provide equivalent angle in range [   0, 2*PI ).
//  x_ModAngle2    - Provide equivalent angle in range ( -PI,   PI ].
//  x_MinAngleDiff - Provide smallest angle between two given angles.
//
//--------------------------------------------------------------------------

f32     x_sin           ( radian Angle   );
f32     x_cos           ( radian Angle   );
f32     x_tan           ( radian Angle   );
f64     x_tanh          ( f64    Angle   );
radian  x_asin          ( f32    Sine    );
radian  x_acos          ( f32    Cosine  );
radian  x_atan          ( f32    Tangent );
radian  x_atan2         ( f32 y, f32 x   );

void    x_sincos        ( radian Angle, f32& Sine, f32& Cosine );
void    x_fastsincos    ( radian Angle, f32& Sine, f32& Cosine );
f32     x_fastsin       ( radian Angle );
f32     x_fastcos       ( radian Angle );
radian  x_ModAngle      ( radian Angle );
radian  x_ModAngle2     ( radian Angle );
radian  x_MinAngleDiff  ( radian Angle1, radian Angle2 );
f32     x_round         ( f32 x, f32 y );


#include "x_math_inline.hpp"

////////////////////////////////////////////////////////////////////////////
//  Operations on "radian3":
////////////////////////////////////////////////////////////////////////////
//
//  A radian3 contains three angles: Pitch, Yaw, and Roll.  The application
//  of these angles is ALWAYS applied in the following order:
//      (1) Roll  [rotate on z axis]
//      (2) Pitch [rotate on x axis]
//      (3) Yaw   [rotate on y axis].
//
//  Given:
//      radian3  R;
//      radian   P, Y, R;
//
//  radian3()       - Standard void CTOR.
//  radian3(R)      - Standard copy CTOR.
//  radian3(P,Y,R)  - Standard promotion CTOR
//  ~radian3        - DTOR.
//
//  Zero            - Set all three rotations to 0.
//  Set(P,Y,R)      - Set rotations to given values.
//
////////////////////////////////////////////////////////////////////////////

struct radian3
{
//--------------------------------------------------------------------------
// Fields.
//

radian  Pitch,  Yaw,  Roll;

//--------------------------------------------------------------------------
// Operations.
//

                    radian3     ( void );
                    radian3     ( const radian3& V );
                    radian3     ( radian Pitch, radian Yaw, radian Roll );
//                    ~radian3    ( void );

        void        Zero        ( void );
        void        Set         ( radian Pitch, radian Yaw, radian Roll );

        radian3     operator -  ( void );   // unary negate
        radian3&    operator += ( const radian3& R );
        radian3&    operator -= ( const radian3& R );
        radian3&    operator *= ( f32 Scalar );
        radian3&    operator /= ( f32 Scalar );
        xbool       operator == ( const radian3& R ) const;
        xbool       operator != ( const radian3& R ) const;
const   radian3&    operator =  ( const radian3& R );
friend  radian3     operator +  ( const radian3& R1, const radian3& R2 );
friend  radian3     operator -  ( const radian3& R1, const radian3& R2 );
friend  radian3     operator /  ( const radian3& R,  f32 Scalar );
friend  radian3     operator *  ( const radian3& R,  f32 Scalar );
friend  radian3     operator *  ( f32 Scalar, const radian3& R );
};

#include "x_math_r3_inline.hpp"

////////////////////////////////////////////////////////////////////////////
//  Operations on "vector2":
////////////////////////////////////////////////////////////////////////////

struct vector2
{
//--------------------------------------------------------------------------
// Fields.
//

f32     X, Y;

//--------------------------------------------------------------------------
// Operations.
//

                    vector2     ( void );
                    vector2     ( const vector2& V );
                    vector2     ( const radian R );
                    vector2     ( f32 X, f32 Y );
//                    ~vector2    ( void );

        void        Zero        ( void );
        void        Set         ( f32 X, f32 Y );
        void        Negate      ( void );
        void        Normalize   ( void );
        void        Scale       ( f32     Scalar );
        void        Rotate      ( radian  R );
        f32         Length      ( void ) const;
        f32         SquaredLength( void ) const;
        radian      Angle       ( void ) const;
        vector2     operator -  ( void );   // unary negate
        vector2&    operator += ( const vector2& V );
        vector2&    operator -= ( const vector2& V );
        vector2&    operator *= ( f32 Scalar );
        vector2&    operator /= ( f32 Scalar );
        xbool       operator == ( const vector2& V ) const;
        xbool       operator != ( const vector2& V ) const;
const   vector2&    operator =  ( const vector2& V );
friend  vector2     operator +  ( const vector2& V1, const vector2& V2 );
friend  vector2     operator -  ( const vector2& V1, const vector2& V2 );
friend  vector2     operator /  ( const vector2& V,  f32 Scalar );
friend  vector2     operator *  ( const vector2& V,  f32 Scalar );
friend  vector2     operator *  ( f32 Scalar, const vector2& V );
friend  f32         Dot         ( const vector2& V1, const vector2& V2 );
friend  radian      AngleBetween( const vector2& V1, const vector2& V2 );
friend  f32         SquaredLength( const vector2& V );
friend  f32         Length      ( const vector2& V );
friend  radian      Angle       ( const vector2& V );
};

#include "x_math_v2_inline.hpp"

////////////////////////////////////////////////////////////////////////////
//  Operations on "vector3":
////////////////////////////////////////////////////////////////////////////
//
//  Given:
//      vector3  V, V1, V2;
//      f32      S, X, Y, Z;
//      radian3  R;
//
//  vector3()       - Standard void CTOR.
//  vector3(V)      - Standard copy CTOR.
//  vector3(R)      - Standard promotion CTOR.  Unit Z vector, rotated.
//  vector3(X,Y,Z)  - Standard promotion CTOR.
//  ~vector3        - DTOR.
//
//  Zero            - Set all elements to 0.
//  Set(X,Y,Z)      - Set all elements to given values.
//  Negate          - Negate all elements.
//  Normalize       - Normalize the vector.
//  Scale           - Scale the vector by given value.
//  RotateX         - Rotate vector about X axis.
//  RotateY         - Rotate vector about Y axis.
//  RotateZ         - Rotate vector about Z axis.
//  Rotate          - Rotate vector using data from a radian3.
//
//  Length          - Get vector's length.
//  Pitch           - Get vector's pitch.
//  Yaw             - Get vector's yaw.
//  PitchYaw        - Get vector's pitch and yaw.
//
//  V1  = V2        - Assignment.
//  V1 += V2        - Vector addition.
//  V1 -= V2        - Vector subtraction.
//  V1 +  V2        - Vector addition.
//  V1 -  V2        - Vector subtraction.
//  V  *= S         - Vector scalar.
//  V  *  S         - Vector scalar.
//  S  *  V         - Vector scalar.
//  V  /= S         - Vector scalar.
//  V  /  S         - Vector scalar.
//       -V         - Vector negation.  (Unary negate operator.)
//
//  V1 == V2        - Equality test.
//  V1 != V2        - Inequality test.
//
//  Dot(V1,V2)      - Dot product between two vectors.
//  Cross(V1,V2)    - Cross product between two vectors.
//  AngleBetween(V1,V2) 
//                  - Angle between two vectors.
//
//  The functions Length, Pitch, Yaw, and PitchYaw come in two flavors:
//  member function and friend function.  For example:
//      Distance = V.Length();
//      Distance = Length( V );
//  Use which ever version is preferred.
//
////////////////////////////////////////////////////////////////////////////
//
//  V3_Project
//
//      Perform a "view to screen" projection on an array of vertices 
//      (vector3's) placing the results in another array.  The projection 
//      parameters can be retreived from a view object.
//
//      Screen.X = PXC0 + (PXC1 * (V.X / V.Z))
//      Screen.Y = PYC0 + (PYC1 * (V.Y / V.Z))
//      Screen.Z = V.Z
//
//  V3_SolveLineAndPlane
//
//      Given: (a) two vectors representing points on a line, and (b) a 
//      plane description.  Find: point which lies within both the line and
//      and the plane.
//
//      The plane is represented by its positive normal (NX,NY,NZ) and its 
//      distance (D) from the origin.  Points on the positive side of the 
//      plane satisfy the following relationship.
//
//          X*NX + Y*Y + Z*NZ + D >= 0
//
//  V3_ClipPolyToPlane
//
//      Given a series of points in space representing a polygon and a 
//      plane, produce a polygon which has been clipped to the plane.  The
//      polygon representation, before and after clipping, consists of n 
//      points having edges V0 to V1, V1 to V2, ..., V(n-2) to V(n-1), and
//      V(n-1) to V0.  The plane is represented as in the previous 
//      function. 
//
////////////////////////////////////////////////////////////////////////////

struct vector3
{

//--------------------------------------------------------------------------
// Fields.
//

f32     X, Y, Z;

//--------------------------------------------------------------------------
// Operations.
//

                    vector3     ( void );
                    vector3     ( const vector3& V );
                    vector3     ( const vector2& V );
                    vector3     ( const radian3& R );
                    vector3     ( f32 X, f32 Y, f32 Z );
//                    ~vector3    ( void );

        void        Zero        ( void );
        void        Set         ( f32 X, f32 Y, f32 Z );
        void        Negate      ( void );
        void        Normalize   ( void );
        void        Scale       ( f32     Scalar );
        void        RotateX     ( radian  Rx );
        void        RotateY     ( radian  Ry );
        void        RotateZ     ( radian  Rz );
        void        Rotate      ( const radian3& Rotation );
        f32         Length      ( void ) const;
        f32         SquaredLength( void ) const;
        float       Dot         ( const vector3& V ) const;
        vector3     Cross       ( const vector3& V ) const;
        radian      Pitch       ( void ) const;
        radian      Yaw         ( void ) const;
        void        PitchYaw    ( radian& Pitch, radian& Yaw ) const; 
        vector3     operator -  ( void ) const;
        vector3&    operator += ( const vector3& V );
        vector3&    operator -= ( const vector3& V );
        vector3&    operator *= ( f32 Scalar );
        vector3&    operator /= ( f32 Scalar );
        xbool       operator == ( const vector3& V ) const;
        xbool       operator != ( const vector3& V ) const;
const   vector3&    operator =  ( const vector3& V );
friend  vector3     operator +  ( const vector3& V1, const vector3& V2 );
friend  vector3     operator -  ( const vector3& V1, const vector3& V2 );
friend  vector3     operator /  ( const vector3& V,  f32 Scalar );
friend  vector3     operator *  ( const vector3& V,  f32 Scalar );
friend  vector3     operator *  ( f32 Scalar, const vector3& V );
friend  f32         Dot         ( const vector3& V1, const vector3& V2 );
friend  vector3     Cross       ( const vector3& V1, const vector3& V2 );
friend  f32         Distance    ( const vector3& V1, const vector3& V2 );
friend  f32         DistanceSquared( const vector3& V1, const vector3& V2 );
friend  radian      AngleBetween( const vector3& V1, const vector3& V2 );
friend  f32         SquaredLength( const vector3& V );
friend  radian      Pitch       ( const vector3& V );
friend  radian      Yaw         ( const vector3& V );
friend  void        PitchYaw    ( const vector3& V, radian& Pitch, 
                                                    radian& Yaw );
};

#include "x_math_v3_inline.hpp"

////////////////////////////////////////////////////////////////////////////
//  Operations on "vector4":
////////////////////////////////////////////////////////////////////////////
//
//  Given:
//      vector4  V, V1, V2;
//      f32      S, X, Y, Z;
//
//  vector4()         - Standard void CTOR.
//  vector4(V)        - Standard copy CTOR.
//  vector4(X,Y,Z,W)  - Standard promotion CTOR.
//  ~vector4          - DTOR.
//
//  Zero              - Set all elements to 0.
//  Set(X,Y,Z,W)      - Set all elements to given values.
//  Negate            - Negate all elements.
//  Normalize         - Normalize the vector.
//  Scale             - Scale the vector by given value.
//
//  Length            - Get vector's length.
//
//  V1  = V2          - Assignment.
//  V1 += V2          - Vector addition.
//  V1 -= V2          - Vector subtraction.
//  V1 +  V2          - Vector addition.
//  V1 -  V2          - Vector subtraction.
//  V  *= S           - Vector scalar.
//  V  *  S           - Vector scalar.
//  S  *  V           - Vector scalar.
//  V  /= S           - Vector scalar.
//  V  /  S           - Vector scalar.
//       -V           - Vector negation.  (Unary negate operator.)
//
//  V1 == V2          - Equality test.
//  V1 != V2          - Inequality test.
//
//
////////////////////////////////////////////////////////////////////////////

struct vector4
{

//--------------------------------------------------------------------------
// Fields.
//

f32     X, Y, Z, W;

//--------------------------------------------------------------------------
// Operations.
//

                    vector4     ( void );
                    vector4     ( const vector4& V );
                    vector4     ( const vector3& V );
                    vector4     ( const radian3& R );
                    vector4     ( f32 X, f32 Y, f32 Z, f32 W );
//                    ~vector4    ( void );

        void        Zero        ( void );
        void        Set         ( f32 X, f32 Y, f32 Z, f32 W );
        void        Negate      ( void );
        void        Normalize   ( void );
        void        Scale       ( f32 Scalar );
        f32         Length      ( void ) const;
        f32         SquaredLength( void ) const;
        vector4     operator -  ( void );   // unary negate
        vector4&    operator += ( const vector4& V );
        vector4&    operator -= ( const vector4& V );
        vector4&    operator *= ( f32 Scalar );
        vector4&    operator /= ( f32 Scalar );
        xbool       operator == ( const vector4& V ) const;
        xbool       operator != ( const vector4& V ) const;
const   vector4&    operator =  ( const vector4& V );
friend  vector4     operator +  ( const vector4& V1, const vector4& V2 );
friend  vector4     operator -  ( const vector4& V1, const vector4& V2 );
friend  vector4     operator /  ( const vector4& V,  f32 Scalar );
friend  vector4     operator *  ( const vector4& V,  f32 Scalar );
friend  vector4     operator *  ( f32 Scalar, const vector4& V );

};

#include "x_math_v4_inline.hpp"

////////////////////////////////////////////////////////////////////////////
//  Operations on "matrix4":
////////////////////////////////////////////////////////////////////////////
//
//  Given:
//      matrix4  M, M1, M2;
//      vector3  V, V1, V2;
//      radian3  R;
//
//  Naming conventions:
//      "Pre" - "Prepend" the given operation into the matrix.
//      "Uni" - Uniform.
//      "SRT" - Scale Rotate Translate.
//      "PYR" - Pitch Yaw Roll.
//
//  matrix4()       - Standard void CTOR.
//  matrix4(M)      - Standard copy CTOR.
//  matrix4(R)      - Standard promotion CTOR.  Rotation matrix.
//  matrix4(a00...) - Standard promotion CTOR
//  ~matrix4        - DTOR.
//
//  Zero            - Set all elements to zero.
//  Identity        - Set matrix to identity state.
//  Transpose       - Matrix transposition.
//  Orthogonalize   - Matrix orthogonalization.
//  Invert          - Industrial strength matrix inversion.
//  InvertSRT       - Quick inversion of SRT only matrix.
//
//  GetScale        - Get scale values from matrix diagonal.
//  SetScale        - Overwrite scale values in matrix diagonal.
//  Scale           - Scale the matrix.
//  PreScale        - Pre-scale the matrix.
//  SetUniScale     - Overwrite scale values in matrix diagonal.
//  UniScale        - Scale the matrix.
//  PreUniScale     - Pre-scale the matrix.
//
//  ClearRotation   - Set upper left 9 cells of matrix to identity values.
//  GetRotation     - Get rotations from the matrix as radian3.
//  SetRotation     - Overwrite upper left 9 cells with rotation data.
//
//  Rotate          - Rotate the matrix.
//  PreRotate       - Pre-rotate the matrix.
//
//  RotateX         - Rotate the matrix on X axis.
//  RotateY         - Rotate the matrix on Y axis.
//  RotateZ         - Rotate the matrix on Z axis.
//  PreRotateX      - Pre-rotate the matrix on X axis.
//  PreRotateY      - Pre-rotate the matrix on Y axis.
//  PreRotateZ      - Pre-rotate the matrix on Z axis.
//
//  ClearTranslation
//                  - Clear the translation cells of the matrix.
//  GetTranslation  - Get translation values from matrix.
//  SetTranslation  - Overwrite translation values in matrix.
//  Translate       - Translate the matrix.
//  PreTranslate    - Pre-translate the matrix.
//
//  Transform(M)    - Transform the matrix using another matrix.
//  PreTransform    - Pre-transform the matrix using another matrix.
//
//  GetRows         - Get 3 row vectors from matrix.
//  SetRows         - Set matrix values from 3 row vectors.
//  GetColumns      - Get 3 column vectors from matrix.
//  SetColumns      - Set matrix values from 3 column vectors.
//
//  SetupAxisRotate - Set all values in matrix to rotation about an axis.
//  SetupSRT        - Set all values in matrix to scale, rotate, translate.
//
//  M * V           - Transform a vector and return result.
//  Transform(V)    - Transform a vector and return result.
//  Transform(V1*,V2*,N)
//                  - Transform N vectors from one list into another list.
//
//  M1  = M2        - Matrix assignment.
//  M1 += M2        - Cell-wise addition.
//  M1 -= M2        - Cell-wise subtraction.
//  M1 +  M2        - Cell-wise addition.
//  M1 -  M2        - Cell-wise subtraction.
//  M1 *  M2        - Matrix multiplication.
//  M1 *= M2        - Matrix multiplication.
//
////////////////////////////////////////////////////////////////////////////

struct matrix4
{

//--------------------------------------------------------------------------
// Fields.
//

f32     M[4][4];

//--------------------------------------------------------------------------
// Operations.
//

                    matrix4         ( void );
                    matrix4         ( const matrix4& M );
                    matrix4         ( const radian3& Rotation );
                    matrix4         ( f32 a00, f32 a01, f32 a02, f32 a03,
                                      f32 a10, f32 a11, f32 a12, f32 a13,
                                      f32 a20, f32 a21, f32 a22, f32 a23,
                                      f32 a30, f32 a31, f32 a32, f32 a33 );
//                    ~matrix4        ( void );

        void        Zero            ( void );
        void        Identity        ( void );
        void        Transpose       ( void );
        void        Orthogonalize   ( void );
        xbool       Invert          ( void );
        xbool       InvertSRT       ( void );
        vector3     GetScale        ( void ) const;
        void        SetScale        ( const vector3& Scale );
        void        Scale           ( const vector3& Scale );
        void        PreScale        ( const vector3& Scale );
        void        SetUniScale     (       f32      Scale );
        void        UniScale        (       f32      Scale );
        void        PreUniScale     (       f32      Scale );
        void        ClearRotation   ( void );
        radian3     GetRotation     ( void ) const;
        void        SetRotation     ( const radian3& Rotation );
        void        Rotate          ( const radian3& Rotation );
        void        PreRotate       ( const radian3& Rotation );
        void        RotateX         ( radian Rx );
        void        RotateY         ( radian Ry );
        void        RotateZ         ( radian Rz );
        void        PreRotateX      ( radian Rx );
        void        PreRotateY      ( radian Ry );
        void        PreRotateZ      ( radian Rz );
        void        ClearTranslation( void );
        vector3     GetTranslation  ( void ) const;
        void        SetTranslation  ( const vector3& Translation );
        void        Translate       ( const vector3& Translation );
        void        PreTranslate    ( const vector3& Translation );
        void        Transform       ( const matrix4& M );
        void        PreTransform    ( const matrix4& M );
        void        GetRows         (       vector3& V1,       vector3& V2,       vector3& V3 ) const;
        void        GetRows         (       vector4& V1,       vector4& V2,       vector4& V3,       vector4& V4  ) const;
        void        SetRows         ( const vector3& V1, const vector3& V2, const vector3& V3 );
        void        SetRows         ( const vector4& V1, const vector4& V2, const vector4& V3, const vector4& V4 );
        void        GetColumns      (       vector3& V1,       vector3& V2,       vector3& V3 ) const;
        void        GetColumns      (       vector4& V1,       vector4& V2,       vector4& V3,       vector4& V4) const;
        void        SetColumns      ( const vector3& V1, const vector3& V2, const vector3& V3 );
        void        SetColumns      ( const vector4& V1, const vector4& V2, const vector4& V3, const vector4& V4 );
        void        SetupAxisRotate ( radian Angle, const vector3& Point1, const vector3& Point2 );
        void        SetupAxisRotate ( radian Angle, const vector3& Axis ); // axis thru origin
        void        SetupSRT        ( const vector3& Scale,
                                      const radian3& Rotate,
                                      const vector3& Translate );
        vector3     operator *      ( const vector3& V ) const;
        vector3     Transform       ( const vector3& V ) const;
        void        Transform       (       vector3* Dest, 
                                      const vector3* Source, 
                                            s32      NVerts = 1 ) const;
        void        UnTransform     (       vector3* Dest, 
                                      const vector3* Source,
                                            s32      NVerts = 1 ) const;

        vector3     TransformNoTranslate( const vector3& V ) const;
        void        TransformNoTranslate(       vector3* Dest, 
                                          const vector3* Source, 
                                                s32      NVerts = 1 ) const;

        vector4     operator *      ( const vector4& V ) const;
        vector4     Transform       ( const vector4& V ) const;



        void        Transform       (       vector4* Dest, 
                                      const vector4* Source, 
                                            s32      NVerts = 1 ) const;

        xbool       operator ==		( const matrix4& M ) const;
        xbool       operator !=		( const matrix4& M ) const;
const   matrix4&    operator =      ( const matrix4& M ); 
        matrix4&    operator +=     ( const matrix4& M );
        matrix4&    operator -=     ( const matrix4& M );
        matrix4&    operator *=     ( const matrix4& M );
friend  matrix4     operator +      ( const matrix4& M1, const matrix4& M2 );
friend  matrix4     operator -      ( const matrix4& M1, const matrix4& M2 );
friend  matrix4     operator *      ( const matrix4& M1, const matrix4& M2 );
        f32         operator ()     ( s32 Row, s32 Column ) const;
        f32&        operator ()     ( s32 Row, s32 Column );
};

#include "x_math_m4_inline.hpp"

////////////////////////////////////////////////////////////////////////////
//  Operations on "quaternion":
////////////////////////////////////////////////////////////////////////////
//
//  Given:
//      f32       S, T
//      radian    r, r1, r2
//      quaterion Q, Qa, Qb
//      matrix4   M
//      vector3   V
//      radian3   R     - Rotaion order as noted in structure radian3.
//
////////////////////////////////////////////////////////////////////////////
//
//  quaternion()        - Standard CTOR.
//  quaternion(Q)       - Standard copy CTOR.
//  quaternion(M)       - Standard promotion CTOR.  Rotation matrix.
//  quaternion(R)       - Standard promotion CTOR.  Radian3.
//  quaternion(x,y,z,w) - Standard promotion CTOR.  Numeric.
//  ~quaternion()       - DTOR.
//
//  Identity()          - Set quaternion to identity state.
//  Zero()              - Set all elements to zero.
//  SetupMatrix(M)      - Sets up the quaternion using a rotaion matrix.
//  SetupRotate(R)      - Sets up the quaternion using a vector3.
//  SetupPreRotate(R)   - Sets up the quaternion using a vector3. 
//  SetupAxisAngle(V,r) - Sets up the quaternion using an axis and an angle.
//  BuildMatrix(M)      - Build a matrix from the quatenion class instance.
//  GetAxis()           - Build an axis from the quatenion class instance.
//  GetAngle()          - Build an angle from the quatenion class instance.
//
//  Invert()            - Standard inversion of the quaternion. "-W"
//  Negate()            - Negate all elements.
//  Scale(S)            - Scales all the quatenions elements by a scalar.
//  Normalize()         - Normalize the quatenion to a 4d univector. 
//  Length()            - Get the 4d vector length.
//  Rotate(R)           - Post-Rotates the quaternion base on the radian3.
//  PreRotate(R)        - Pre -Rotates the quaternion base on the radian3.
//  RotateX             - Post-Rotates the quaternion around the -X- axis.
//  RotateY             - Post-Rotates the quaternion around the -Y- axis.
//  RotateZ             - Post-Rotates the quaternion around the -Z- axis.
//  PreRotateX          - Pre -Rotates the quaternion around the -X- axis.
//  PreRotateY          - Pre -Rotates the quaternion around the -Y- axis.
//  PreRotateZ          - Pre -Rotates the quaternion around the -Z- axis.
//
//  Transform(Q)        - Post-Concatenates the quaternion based on the incoming one.
//  PreTransform(Q)     - Pre -Concatenates the quaternion based on the incoming one.
//
//  AngleBetween(Qa,Qb) - It finds the angle between the two incoming quaternions.
//  Dot(Qa,Qb)          - It does the 4D dot product between the incoming quaternions.
//
//  Pitch()             - It find the pitch angle of vector(0,0,1) in quaterion space.
//  Yaw()               - It find the yaw angle of vector(0,0,1) in quaterion space.
//  PitchYaw(Q,r1,r2)   - It equal to call Pitch() and Yaw() separately, but faster.
//
//  BlendSpherical(Qa,Qb,T)  - Interpolates from Qa to Qb spherically between the range of T[0,1].
//  BlendSpherical2(Qa,Qb,T) - Interpolates from Qa to Qb spherically between the range of T[0,1].
//  BlendLinear(Qa,Qb,T,bNorm) - Interpolates from Qa to Qb linearly between the range of T[0,1], the normalizes the final Quaternion.
//
////////////////////////////////////////////////////////////////////////////
//
//  -Q                  - Negates the quaternion.
//  Qa += Qb            - Adds all the components of quaternion b into a.
//  Qa -= Qb            - Subtracts all the components of quaternion b from a.
//  Qa *= Qb            - Pre-Concadenates the quatenion a based on b. 
//
//  Q  * V              - Pre-Transforms the vector by the quaternion.
//  V  * Q              - Pos-Transforms the vector by the quaternion.
//  Q  * S              - Scales all the quaternions elements by a scalar 'S'.
//  S  * Q              - Scales all the quaternions elements by a scalar 'S'.
//  Qa * Qb             - Concatenates the quaternions in the given order.
//  Qb * Qa             - Concatenates the quaternions in the given order.
//  Qa + Qb             - Adds all the components of both quaternions.
//  Qa - Qb             - Subtracts all the conponents quaternion b from quaternion a.
//
////////////////////////////////////////////////////////////////////////////

struct quaternion
{
//--------------------------------------------------------------------------
// Fields.
//

f32 X, Y, Z, W;

//--------------------------------------------------------------------------
// Operations.
//
                    quaternion      ( void );
                    quaternion      ( f32 X, f32 Y, f32 Z, f32 W );
                    quaternion      ( const quaternion& Q );
                    quaternion      ( const matrix4& M );
                    quaternion      ( const radian3& Rotation );
//                   ~quaternion      ( void );

        void        Identity        ( void );
        void        Zero            ( void );
        void        SetupMatrix     ( const matrix4& Matrix   );
        void        SetupRotate     ( const radian3& Rotation );
        void        SetupPreRotate  ( const radian3& Rotation );
        void        SetupAxisAngle  ( const vector3& Axis, radian R );
        void        BuildMatrix     ( matrix4& Matrix ) const;
        vector3     GetAxis         ( void ) const;
        radian      GetAngle        ( void ) const;
        void        Invert          ( void );
        void        Negate          ( void );
        void        Scale           ( f32 Scale );
        void        Normalize       ( void );
        f32         Length          ( void );
        void        Rotate          ( const radian3& Rotation );
        void        PreRotate       ( const radian3& Rotation );
        void        RotateX         ( radian Rx );
        void        RotateY         ( radian Ry );
        void        RotateZ         ( radian Rz );
        void        PreRotateX      ( radian Rx );
        void        PreRotateY      ( radian Ry );
        void        PreRotateZ      ( radian Rz );
        void        Transform       ( const quaternion& Q );
        void        PreTransform    ( const quaternion& Q );
        quaternion  operator -      ( void ) const;
        quaternion& operator +=     ( const quaternion& Q );
        quaternion& operator -=     ( const quaternion& Q );
        quaternion& operator *=     ( const quaternion& Q );
friend  radian      AngleBetween    ( const quaternion& Qa, const quaternion& Qb );
friend  f32         Dot             ( const quaternion& Qa, const quaternion& Qb );
friend  vector3     operator *      ( const quaternion& Qa, const vector3&    V  );
friend  quaternion  operator *      ( f32 S, const quaternion& Q );
friend  quaternion  operator *      ( const quaternion& Q, f32 S );
friend  quaternion  operator *      ( const quaternion& Qa, const quaternion& Qb );
friend  quaternion  operator +      ( const quaternion& Qa, const quaternion& Qb );
friend  quaternion  operator -      ( const quaternion& Qa, const quaternion& Qb );
friend  radian      Pitch           ( const quaternion& Q );
friend  radian      Yaw             ( const quaternion& Q );
friend  void        PitchYaw        ( const quaternion& Q, radian& Pitch, radian& Yaw );
friend  quaternion  BlendSpherical  ( const quaternion& Qa, const quaternion& Qb, f32 T );
friend  quaternion  BlendSpherical2 ( const quaternion& Qa, const quaternion& Qb, f32 T );
friend  quaternion  BlendLinear     ( const quaternion& Qa, const quaternion& Qb, f32 T, xbool bNormalize = TRUE );
};

#include "x_math_q_inline.hpp"

////////////////////////////////////////////////////////////////////////////
//
//  ClipLineToPlane     - It returns whether interception happened, 
//                        and the intersection point.
//
//  ClipRayToPlane      - It returns whether interception happened and T 
//                        which can range from -Inf to +Inf. Tip: When it 
//                        returns false T is set to 0 such after the call
//                        if( T <= 0 ) no colision at the direction of the ray.
//                                              
//  ClipPolyToPlane     - It returns whether interception happened, 
//                        and the clipped poly.
//
////////////////////////////////////////////////////////////////////////////

xbool ClipLineToPlane( vector3& Dest, vector3& V0, vector3& V1,
                       f32 NX, f32 NY, f32 NZ, f32 D );

xbool ClipRayToPlane ( f32& DestT, vector3& Origin, vector3& Direction,
                       f32 NX, f32 NY, f32 NZ, f32 D );

xbool ClipPolyToPlane( vector3* Dest, s32& NDest, 
                       vector3* Src,  s32  NSrc, 
                       f32 NX, f32 NY, f32 NZ, f32 D );

////////////////////////////////////////////////////////////////////////////
/*
void    Project...

xbool   SolveLineAndPlane(       vector3& SV,   // new point
                                 f32&      t,   // t from V0 to V1
                           const vector3& V0,   // point on line
                           const vector3& V1,   // point on line
                           const f32      NX,   // plane normal X
                           const f32      NY,   // plane normal Y
                           const f32      NZ,   // plane normal Z
                           const f32       D ); // plane distance

xbool   ClipPolyToPlane  ( vector3d* Dest, s32* NDest, 
                           vector3d* Src,  s32  NSrc, 
                           f32 NX, f32 NY, f32 NZ, f32 D );
*/
////////////////////////////////////////////////////////////////////////////

#endif


#include "x_math.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"

#include "Q_View.hpp"
#include "Q_Engine.hpp"

///////////////////////////////////////////////////////////////////////////
//  DEFINES
///////////////////////////////////////////////////////////////////////////

#define VIEW_CLEAN_COUNT               500   // Ops between cleanings

// Each bit set in DirtyFlags means that the following is dirty...
#define DIRTY_PROJECT      ((u32)1<<0)  // ... projection constants
#define DIRTY_FRUSTUM      ((u32)1<<1)  // ... frustum constants
#define DIRTY_W2V          ((u32)1<<2)  // ... World To View matrix
#define DIRTY_V2W          ((u32)1<<3)  // ... View To World matrix
#define DIRTY_PLANES       ((u32)1<<4)  // ... view planes in world space
#define DIRTY_EDGES        ((u32)1<<5)  // ... view plane edges in world space
#define DIRTY_YFOV         ((u32)1<<6)  // ... view aspect ratio
#define DIRTY_CLIP_FRUSTUM ((u32)1<<7)  // ... clip frustum constants
#define DIRTY_W2C          ((u32)1<<8)  // ... World To Clip matrix
#define DIRTY_CLIP_PLANES  ((u32)1<<9)  // ... clip view plane edges in world space
#define DIRTY_CLIPYFOV     ((u32)1<<10) // ... clip view aspect ratio

#define DIRTY_ALL          ((u32)0xFFFF)


///////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void view::Clean( void )
{
    f32 Dot0,Dot1,Dot2;
    f32 Len0,Len1,Len2;
    s32 i;

    // Check orthogonalization
    m_CleanCount--;
    if( m_CleanCount > 0 )
        return;

    Dot0 = Dot1 = Dot2 = 0;
    Len0 = Len1 = Len2 = 0;
    for( i = 0; i < 3; i++ )
    {
        Len0 += m_WorldOrient.M[0][i] * m_WorldOrient.M[0][i];
        Len1 += m_WorldOrient.M[1][i] * m_WorldOrient.M[1][i];
        Len2 += m_WorldOrient.M[2][i] * m_WorldOrient.M[2][i];
    }

    // what do we do if Len0,1,or2 are still 0?  
    if (Len0 > 0.0f)
        Len0 = x_1oversqrt( Len0 );
    if (Len1 > 0.0f)
        Len1 = x_1oversqrt( Len1 );
    if (Len2 > 0.0f)
        Len2 = x_1oversqrt( Len2 );

    for( i = 0; i < 3; i++ )
    {
        m_WorldOrient.M[0][i] *= Len0;
        m_WorldOrient.M[1][i] *= Len1;
        m_WorldOrient.M[2][i] *= Len2;
    }

    m_WorldOrient.Orthogonalize();

    m_CleanCount = VIEW_CLEAN_COUNT;

    // nuke translational portion
    m_WorldOrient.M[3][0] = 0;
    m_WorldOrient.M[3][1] = 0;
    m_WorldOrient.M[3][2] = 0;
    m_WorldOrient.M[3][3] = 1;
    m_WorldOrient.M[0][3] = 0;
    m_WorldOrient.M[1][3] = 0;
    m_WorldOrient.M[2][3] = 0;

    m_DirtyFlags |= (DIRTY_W2V | DIRTY_V2W | DIRTY_W2C | DIRTY_PLANES | DIRTY_CLIP_PLANES | DIRTY_EDGES);
}

//=========================================================================

view::view( void )
{
    s32 W, H;

    // Set the defaults for whatever target.
    ENG_GetResolution(W,H);

    // Clear the hold strcture to zero
    x_memset( this, 0, sizeof(this) );

    m_WorldPos.Zero();

    m_WorldOrient.Identity();

    m_CleanCount    = VIEW_CLEAN_COUNT;
    m_ViewL         = 0;
    m_ViewR         = W-1;
    m_ViewT         = 0;
    m_ViewB         = H-1;

    // Set the aspect ratio
    m_AspectRatio   = (f32)W / (f32)H;

    m_XFOV          = R_60;
#if defined(TARGET_PS2)
    m_ClipXFOV      = DEG_TO_RAD(74.856629616580690428523013535817f);
#endif

    m_ZNear         = 10.0f;
    m_ZFar          = 10000.0f;
    m_DirtyFlags    = DIRTY_ALL;
    m_Locked        = FALSE;

#if defined( TARGET_PS2 )
    m_ClipAspectRatio   = (f32)H/(f32)W;//0.75f;
#endif

}

//=========================================================================

view::~view( void )
{
    x_memset( this, 0, sizeof(this) );
}

//=========================================================================

void view::SetViewport( s32 Left, s32 Top, s32 Right, s32 Bottom )
{
    ASSERT( m_Locked == FALSE );

    m_ViewL = Left;
    m_ViewT = Top;
    m_ViewR = Right;
    m_ViewB = Bottom;

    ASSERT( m_ViewR > m_ViewL );
    ASSERT( m_ViewB > m_ViewT );

    m_DirtyFlags |= (DIRTY_PROJECT | DIRTY_FRUSTUM | DIRTY_CLIP_FRUSTUM | DIRTY_YFOV);
}

//=========================================================================

void view::SetAspectRatio( s32 MonitorResX, s32 MonitorResY )
{
    ASSERT( m_Locked == FALSE );
    ASSERT( MonitorResX > 0 );
    ASSERT( MonitorResY > 0 );

    // Set the aspect ratio
    m_AspectRatio = (f32)MonitorResX / (f32)MonitorResY;

#if defined( TARGET_PS2 )
    m_ClipAspectRatio = 0.75f;
#endif

    m_DirtyFlags |= (DIRTY_PROJECT | DIRTY_FRUSTUM | DIRTY_CLIP_FRUSTUM | DIRTY_YFOV);
}

//=========================================================================

void view::GetAspectRatio( f32& A ) const
{
    A = m_AspectRatio;
}

//=========================================================================
#if defined(TARGET_PS2)
void view::GetClipAspectRatio( f32& A ) const
{
    A = m_ClipAspectRatio;
}
#endif
//==========================================================================

f32 view::GetAspectRatio( void ) const
{
    return m_AspectRatio;
}

//=========================================================================
void view::SetXFOV( radian XFOV )
{
    ASSERT( m_Locked == FALSE );

#if defined(TARGET_PS2)
    if (XFOV != m_XFOV)
    {
//        f32 temp = ((m_ViewR - m_ViewL + 1.0f) * 0.5f) / x_tan( XFOV * 0.5f );
        f32 temp = x_tan( XFOV * 0.5f ) * m_ZNear;
        temp *= (2048.0f / (f32)((m_ViewR - m_ViewL + 1)>>1));

        m_ClipXFOV = 2.0f * x_atan2( temp, m_ZNear );

        if( m_ClipXFOV < R_1   )  m_ClipXFOV = R_1;
        if( m_ClipXFOV > R_180 )  m_ClipXFOV = R_180;
    }
#endif

    m_XFOV = XFOV;

    if( m_XFOV < R_1   )  m_XFOV = R_1;
    if( m_XFOV > R_120 )  m_XFOV = R_120;


    m_DirtyFlags |= (DIRTY_PROJECT | DIRTY_FRUSTUM | DIRTY_PLANES | DIRTY_CLIP_PLANES | DIRTY_EDGES | DIRTY_CLIPYFOV | DIRTY_YFOV);
}

//==========================================================================

void view::SetXFOVUnclamped( radian XFOV )
{
    ASSERT( m_Locked == FALSE );

#if defined(TARGET_PS2)
    if (m_XFOV != XFOV)
    {
        f32 temp = ((m_ViewR - m_ViewL + 1.0f) * 0.5f) / x_tan( XFOV * 0.5f );
        if (temp != 0)
            m_ClipXFOV = x_atan2( 2048.0f, temp );
        else
            m_ClipXFOV = 0.0f;
    }
#endif

    m_XFOV = XFOV;

    m_DirtyFlags |= (DIRTY_PROJECT | DIRTY_FRUSTUM | DIRTY_CLIP_FRUSTUM | DIRTY_PLANES | DIRTY_CLIP_PLANES | DIRTY_EDGES | DIRTY_YFOV | DIRTY_CLIPYFOV);
}

//=========================================================================

void view::SetZLimits( f32 NearZ, f32 FarZ )
{
    ASSERT( NearZ > 0.0f );
    ASSERT( FarZ > NearZ );
    ASSERT( m_Locked == FALSE );

    m_ZNear = MAX( 1.0f, NearZ );
    m_ZFar  = FarZ;
}

//=========================================================================

void view::ConvertPoint( system DSTSystem, vector3& Dst,
                         system SRCSystem, const vector3& Src )
{
//    vector3  World;
    matrix4  M;

    //
    // quick case
    //
    if( SRCSystem == DSTSystem )
    {
        Dst = Src;
        return;
    }

    //
    // Check for World->View
    //
    if( (SRCSystem == V_WORLD) && (DSTSystem == V_CAMERA) )
    {
        register f32 WX,WY,WZ;

        GetW2VMatrix( M );

        WX      = Src.X;
        WY      = Src.Y;
        WZ      = Src.Z;

        Dst.X  = (M.M[0][0]*WX) + (M.M[1][0]*WY) + (M.M[2][0]*WZ) + M.M[3][0];
        Dst.Y  = (M.M[0][1]*WX) + (M.M[1][1]*WY) + (M.M[2][1]*WZ) + M.M[3][1];
        Dst.Z  = (M.M[0][2]*WX) + (M.M[1][2]*WY) + (M.M[2][2]*WZ) + M.M[3][2];

        return;
    }

    //
    // Check for View->World
    //
    if( (SRCSystem == V_CAMERA) && (DSTSystem == V_WORLD) )
    {
        register f32 WX,WY,WZ;

        GetV2WMatrix( M );

        WX      = Src.X;
        WY      = Src.Y;
        WZ      = Src.Z;

        Dst.X  = (M.M[0][0]*WX) + (M.M[1][0]*WY) + (M.M[2][0]*WZ) + M.M[3][0];
        Dst.Y  = (M.M[0][1]*WX) + (M.M[1][1]*WY) + (M.M[2][1]*WZ) + M.M[3][1];
        Dst.Z  = (M.M[0][2]*WX) + (M.M[1][2]*WY) + (M.M[2][2]*WZ) + M.M[3][2];

        return;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  'CAMERA' MANIPULATIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//=========================================================================

void view::SetPosition( system System, const vector3& Position )
{
    ASSERT( m_Locked == FALSE );

    ConvertPoint( V_WORLD, m_WorldPos, System, Position );

    m_DirtyFlags |= (DIRTY_W2V | DIRTY_V2W | DIRTY_W2C | DIRTY_PLANES | DIRTY_CLIP_PLANES | DIRTY_EDGES);
}

//=========================================================================

void view::SetRotations( system System, radian Pitch, radian Yaw, radian Roll )
{
    ASSERT( m_Locked == FALSE );

    m_WorldOrient.Identity();

    Pivot( System, V_AXIS_Z, Roll );
    Pivot( System, V_AXIS_X, Pitch );
    Pivot( System, V_AXIS_Y, Yaw );
}

//=========================================================================

void view::SetRotations( system System, const matrix4& mat )
{
    ASSERT( (m_Locked == FALSE) && (System == V_WORLD) ); //Currently only support world rotation

    m_WorldOrient = mat;

    //Clean up the matrix and set dirty flags
    Clean();
}

//=========================================================================

void view::Pivot( system System, axis Axis, radian Angle )
{
    ASSERT( m_Locked == FALSE);

    // PIVOT IN PLACE ABOUT AXIS PARALLEL TO WORLD 
    if( System == V_WORLD )
    {
        switch( Axis )
        {
            case V_AXIS_X:  m_WorldOrient.RotateX( Angle );  break;
            case V_AXIS_Y:  m_WorldOrient.RotateY( Angle );  break;
            case V_AXIS_Z:  m_WorldOrient.RotateZ( Angle );  break;
            default:        ASSERT( FALSE );                 break;
        }
    }
    else if( System == V_CAMERA ) // PIVOT IN PLACE ABOUT AXIS PARALLEL TO CAMERA AXIS
    {
        matrix4 M;

        M.Identity();

        switch( Axis )
        {
            case V_AXIS_X:  M.RotateX( Angle );  break;
            case V_AXIS_Y:  M.RotateY( Angle );  break;
            case V_AXIS_Z:  M.RotateZ( Angle );  break;
            default:        ASSERT( FALSE );     break;
        }

        m_WorldOrient *= M;
    }
    else
    {
        ASSERT( FALSE );
    }

    //
    // Clean matrix and update dirty flags
    //
    Clean();
    m_DirtyFlags |= (DIRTY_W2V | DIRTY_V2W | DIRTY_W2C | DIRTY_PLANES | DIRTY_CLIP_PLANES | DIRTY_EDGES);
}

//=========================================================================

void view::Rotate( system System, axis Axis, radian Angle )
{
    matrix4 M;
    vector3 V;

    ASSERT( m_Locked == FALSE );

    M.Identity();

    switch( Axis )
    {
        case V_AXIS_X:  M.RotateX( Angle );  break;
        case V_AXIS_Y:  M.RotateY( Angle );  break;
        case V_AXIS_Z:  M.RotateZ( Angle );  break;
        default:        ASSERT( FALSE );     break;
    }

    //
    // ROTATE SYSTEM
    //
    if( System == V_WORLD ) // ROTATE IN WORLD SYSTEM
    {
        V = m_WorldPos;

        M.Transform( &m_WorldPos, &V, 1 );

        m_WorldOrient = M * m_WorldOrient;
    }
    else if( System == V_CAMERA ) // ROTATE IN CAMERA SYSTEM
    {
        m_WorldOrient *= M;
    }
    else
    {
        ASSERT( FALSE );
    }

    //
    // Clean matrix and update dirty flags
    //
    Clean();
    m_DirtyFlags |= (DIRTY_W2V | DIRTY_V2W | DIRTY_W2C | DIRTY_PLANES | DIRTY_CLIP_PLANES | DIRTY_EDGES);
}

//=========================================================================

void view::Translate( system System, const vector3& Translation )
{
    vector3 V;

    ASSERT( m_Locked == FALSE );

    //
    // Determine translation in world space
    //
    switch( System )
    {
        case V_WORLD:       V = Translation;
                            break;

        case V_CAMERA:      m_WorldOrient.Transform( &V, &Translation, 1 );
                            break;

        default:            ASSERT( FALSE ); 
                            break;
    }

    // Update world position
    m_WorldPos += V;

    // Clean matrix and update dirty flags
    Clean();
    m_DirtyFlags |= (DIRTY_W2V | DIRTY_V2W | DIRTY_W2C | DIRTY_PLANES | DIRTY_CLIP_PLANES | DIRTY_EDGES);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  REQUESTS FOR VIEW INFO
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//=========================================================================

void view::GetW2VMatrix( matrix4& W2V )
{
    if( m_DirtyFlags & DIRTY_W2V )
    {
        matrix4  M;
        vector3  T( m_WorldPos );

        T = -T;

        m_WorldOrient.M[3][0] = 0;
        m_WorldOrient.M[3][1] = 0;
        m_WorldOrient.M[3][2] = 0;
        m_WorldOrient.M[3][3] = 1;
        m_WorldOrient.M[0][3] = 0;
        m_WorldOrient.M[1][3] = 0;
        m_WorldOrient.M[2][3] = 0;

        m_W2V = m_WorldOrient;

        m_W2V.Transpose();

        M.Identity();

        M.SetTranslation( T );

        m_W2V *= M;

        m_DirtyFlags &= ~(DIRTY_W2V);
    }

    W2V = m_W2V;
}

//=========================================================================

void view::GetV2WMatrix( matrix4& V2W )
{
    if( m_DirtyFlags & DIRTY_V2W )
    {
        m_WorldOrient.M[3][0] = 0;
        m_WorldOrient.M[3][1] = 0;
        m_WorldOrient.M[3][2] = 0;
        m_WorldOrient.M[3][3] = 1;
        m_WorldOrient.M[0][3] = 0;
        m_WorldOrient.M[1][3] = 0;
        m_WorldOrient.M[2][3] = 0;

        m_V2W = m_WorldOrient;

        m_V2W.Translate( m_WorldPos );

        m_DirtyFlags &= ~(DIRTY_V2W);
    }

    V2W = m_V2W;
}

//=========================================================================

void view::GetFrustum( f32& FrustumSX, f32& FrustumSY )
{
    if( m_DirtyFlags & DIRTY_FRUSTUM )
    {
        radian YFOV;

        GetYFOV( YFOV );
        m_FrustumSX  = (f32)(1.0f / x_tan( m_XFOV * 0.5f ));
        m_FrustumSY  = (f32)(1.0f / x_tan(   YFOV * 0.5f ));
        m_DirtyFlags &= ~(DIRTY_FRUSTUM);
    }

    FrustumSX = m_FrustumSX;
    FrustumSY = m_FrustumSY;
}

//=========================================================================

#if defined(TARGET_PS2)
void view::GetClipFrustum( f32& FrustumSX, f32& FrustumSY )
{
    if( m_DirtyFlags & DIRTY_CLIP_FRUSTUM )
    {
        radian XFOV;
        radian YFOV;
        XFOV = m_ClipXFOV;
        XFOV *= 0.5f;
        YFOV = m_ClipXFOV;

        m_ClipFrustumSX  = (f32)(1.0f / x_tan( XFOV ));
        m_ClipFrustumSY  = m_ClipFrustumSX;//(f32)(1.0f / x_tan( YFOV ));

        m_DirtyFlags &= ~(DIRTY_CLIP_FRUSTUM);
    }

    FrustumSX = m_ClipFrustumSX;
    FrustumSY = m_ClipFrustumSY;
}
#endif

//=========================================================================

void view::GetProjection( f32& ProjectXC0, f32& ProjectXC1,
                          f32& ProjectYC0, f32& ProjectYC1 )
{
    if( m_DirtyFlags & DIRTY_PROJECT )
    {
        /////////////////////////////////////////////////
        // SX = ProjectXC0 + ProjectXC1 * (VX / VZ)
        // SY = ProjectYC0 + ProjectYC1 * (VY / VZ)
        /////////////////////////////////////////////////

        f32 FSX,FSY;

        GetFrustum( FSX, FSY );

        ASSERT( m_AspectRatio > 0.0f );
        ASSERT( m_AspectRatio < 5.0f );

        m_ProjectXC0  = (m_ViewR + m_ViewL) / 2.0f;
        m_ProjectYC0  = (m_ViewT + m_ViewB) / 2.0f;
        m_ProjectXC1  = -FSX * ((m_ViewR - m_ViewL) / 2.0f);
        m_ProjectYC1  = -FSY * ((m_ViewB - m_ViewT) / 2.0f);
        m_ProjectYC1 /= m_AspectRatio;
        m_DirtyFlags &= ~(DIRTY_PROJECT);
    }

    ProjectXC0 = m_ProjectXC0;
    ProjectYC0 = m_ProjectYC0;
    ProjectXC1 = m_ProjectXC1;
    ProjectYC1 = m_ProjectYC1;
}

//=========================================================================

void view::GetViewPlanes( system System,
                          vector3& TopN, f32& TopD,
                          vector3& BotN, f32& BotD,
                          vector3& LftN, f32& LftD,
                          vector3& RgtN, f32& RgtD,
                          vector3& NearN, f32& NearD,
                          vector3& FarN,  f32& FarD )
{
    if( (m_DirtyFlags & DIRTY_PLANES) || (m_VPSystem!=System) )
    {
        f32         FX,FY;          // Frustum slopes
        f32         XLen,YLen;      // 1/(length of unnormalized vectors)
        vector3     Eye(0,0,0);     // Position of eye in view space
        vector3     FarPt(0,0,0);
        vector3     NearPt(0,0,0);

        // get frustum slopes
        GetFrustum( FX, FY );

        // build normal vectors for planes in view space
        XLen    = x_1oversqrt( FX*FX + 1.0f );
        YLen    = x_1oversqrt( FY*FY + 1.0f );
        m_TopN.X  = (  0);        m_TopN.Y  = (-FY)*YLen;   m_TopN.Z  = YLen;
        m_BotN.X  = (  0);        m_BotN.Y  = ( FY)*YLen;   m_BotN.Z  = YLen;
        m_LftN.X  = (-FX)*XLen;   m_LftN.Y  = (  0);        m_LftN.Z  = XLen;
        m_RgtN.X  = ( FX)*XLen;   m_RgtN.Y  = (  0);        m_RgtN.Z  = XLen;
        m_NearN.X = (  0);        m_NearN.Y = (  0);        m_NearN.Z = 1.0f;
        m_FarN.X  = (  0);        m_FarN.Y  = (  0);        m_FarN.Z  = -1.0f;
        NearPt.Z  = m_ZNear;
        FarPt.Z   = m_ZFar;

        // move vectors from CAMERA to requested system
        ConvertPoint( System, m_TopN,  V_CAMERA, m_TopN );
        ConvertPoint( System, m_BotN,  V_CAMERA, m_BotN );
        ConvertPoint( System, m_LftN,  V_CAMERA, m_LftN );
        ConvertPoint( System, m_RgtN,  V_CAMERA, m_RgtN );
        ConvertPoint( System, m_NearN, V_CAMERA, m_NearN );
        ConvertPoint( System, m_FarN,  V_CAMERA, m_FarN );
        ConvertPoint( System, Eye,     V_CAMERA, Eye );
        ConvertPoint( System, NearPt,  V_CAMERA, NearPt );
        ConvertPoint( System, FarPt,   V_CAMERA, FarPt );
        
        m_TopN  -= Eye;
        m_BotN  -= Eye;
        m_LftN  -= Eye;
        m_RgtN  -= Eye;
        m_NearN -= Eye;
        m_FarN  -= Eye;

        // compute distance 'D' values of plane equation
        m_TopD  = -(m_TopN.X*Eye.X + m_TopN.Y*Eye.Y + m_TopN.Z*Eye.Z);
        m_BotD  = -(m_BotN.X*Eye.X + m_BotN.Y*Eye.Y + m_BotN.Z*Eye.Z);
        m_LftD  = -(m_LftN.X*Eye.X + m_LftN.Y*Eye.Y + m_LftN.Z*Eye.Z);
        m_RgtD  = -(m_RgtN.X*Eye.X + m_RgtN.Y*Eye.Y + m_RgtN.Z*Eye.Z);
        m_NearD = -(m_NearN.X*NearPt.X + m_NearN.Y*NearPt.Y + m_NearN.Z*NearPt.Z);
        m_FarD  = -(m_FarN.X*FarPt.X + m_FarN.Y*FarPt.Y + m_FarN.Z*FarPt.Z);

        m_DirtyFlags &= ~(DIRTY_PLANES);
        m_DirtyFlags |= DIRTY_CLIP_PLANES;
        m_VPSystem    = System;
    }

    TopN  = m_TopN;
    BotN  = m_BotN;
    LftN  = m_LftN;
    RgtN  = m_RgtN;
    NearN = m_NearN;
    FarN  = m_FarN;
    TopD  = m_TopD;
    BotD  = m_BotD;
    LftD  = m_LftD;
    RgtD  = m_RgtD;
    NearD = m_NearD;
    FarD  = m_FarD;
}

//=========================================================================

#if defined(TARGET_PS2)
void view::GetClipViewPlanes( system System,
                              vector3& TopN, f32& TopD,
                              vector3& BotN, f32& BotD,
                              vector3& LftN, f32& LftD,
                              vector3& RgtN, f32& RgtD,
                              vector3& NearN, f32& NearD,
                              vector3& FarN,  f32& FarD )
{
    if( (m_DirtyFlags & DIRTY_CLIP_PLANES) || (m_VPSystem!=System) )
    {
        f32         FX,FY;          // Frustum slopes
        f32         XLen,YLen;      // 1/(length of unnormalized vectors)
        vector3     Eye(0,0,0);     // Position of eye in view space
        vector3     FarPt(0,0,0);
        vector3     NearPt(0,0,0);

        // get frustum slopes
        GetClipFrustum( FX, FY );

        // build normal vectors for planes in view space
        XLen    = x_1oversqrt( FX*FX + 1.0f );
        YLen    = x_1oversqrt( FY*FY + 1.0f );
        m_ClipTopN.X  = (  0);        m_ClipTopN.Y  = (-FY)*YLen;   m_ClipTopN.Z  = YLen;
        m_ClipBotN.X  = (  0);        m_ClipBotN.Y  = ( FY)*YLen;   m_ClipBotN.Z  = YLen;
        m_ClipLftN.X  = (-FX)*XLen;   m_ClipLftN.Y  = (  0);        m_ClipLftN.Z  = XLen;
        m_ClipRgtN.X  = ( FX)*XLen;   m_ClipRgtN.Y  = (  0);        m_ClipRgtN.Z  = XLen;

        m_NearN.X = 0;  m_NearN.Y = 0;  m_NearN.Z = 1.0f;
        m_FarN.X  = 0;  m_FarN.Y  = 0;  m_FarN.Z  = -1.0f;

        NearPt.Z  = m_ZNear;
        FarPt.Z   = m_ZFar;

        // move vectors from CAMERA to requested system
        ConvertPoint( System, m_ClipTopN,  V_CAMERA, m_ClipTopN );
        ConvertPoint( System, m_ClipBotN,  V_CAMERA, m_ClipBotN );
        ConvertPoint( System, m_ClipLftN,  V_CAMERA, m_ClipLftN );
        ConvertPoint( System, m_ClipRgtN,  V_CAMERA, m_ClipRgtN );
        ConvertPoint( System, m_NearN, V_CAMERA, m_NearN );
        ConvertPoint( System, m_FarN,  V_CAMERA, m_FarN );
        ConvertPoint( System, Eye,     V_CAMERA, Eye );
        ConvertPoint( System, NearPt,  V_CAMERA, NearPt );
        ConvertPoint( System, FarPt,   V_CAMERA, FarPt );
        
        m_ClipTopN  -= Eye;
        m_ClipBotN  -= Eye;
        m_ClipLftN  -= Eye;
        m_ClipRgtN  -= Eye;
        m_NearN -= Eye;
        m_FarN  -= Eye;

        // compute distance 'D' values of plane equation
        m_ClipTopD  = -(m_ClipTopN.X*Eye.X + m_ClipTopN.Y*Eye.Y + m_ClipTopN.Z*Eye.Z);
        m_ClipBotD  = -(m_ClipBotN.X*Eye.X + m_ClipBotN.Y*Eye.Y + m_ClipBotN.Z*Eye.Z);
        m_ClipLftD  = -(m_ClipLftN.X*Eye.X + m_ClipLftN.Y*Eye.Y + m_ClipLftN.Z*Eye.Z);
        m_ClipRgtD  = -(m_ClipRgtN.X*Eye.X + m_ClipRgtN.Y*Eye.Y + m_ClipRgtN.Z*Eye.Z);
        m_NearD     = -(m_NearN.X*NearPt.X + m_NearN.Y*NearPt.Y + m_NearN.Z*NearPt.Z);
        m_FarD      = -(m_FarN.X*FarPt.X   + m_FarN.Y*FarPt.Y   + m_FarN.Z*FarPt.Z  );

        m_DirtyFlags &= ~(DIRTY_CLIP_PLANES);
        m_VPSystem    = System;
    }

    TopN  = m_ClipTopN;
    BotN  = m_ClipBotN;
    LftN  = m_ClipLftN;
    RgtN  = m_ClipRgtN;
    NearN = m_NearN;
    FarN  = m_FarN;
    TopD  = m_ClipTopD;
    BotD  = m_ClipBotD;
    LftD  = m_ClipLftD;
    RgtD  = m_ClipRgtD;
    NearD = m_NearD;
    FarD  = m_FarD;
}

//=========================================================================
#endif

void view::GetScreenRay( system   System, 
                         f32      SX, 
                         f32      SY, 
                         vector3& Dir )
{
    vector3 Eye(0,0,0);
    f32 XC0,XC1;
    f32 YC0,YC1;

    // Get projection constants
    GetProjection( XC0, XC1, YC0, YC1 );

    // Compute Direction in viewspace
    Dir.Z = 1000.0f;
    Dir.X = (SX-XC0) * (Dir.Z / XC1);
    Dir.Y = (SY-YC0) * (Dir.Z / YC1);

    // Convert into destination space
    ConvertPoint( System, Dir, V_CAMERA, Dir );
    ConvertPoint( System, Eye, V_CAMERA, Eye );

    // Make direction again
    Dir -= Eye;

    // Normalize
    Dir.Normalize();
}

//=========================================================================

void view::GetViewEdges( system   System,
                         vector3& TLEdge,
                         vector3& TREdge,
                         vector3& BLEdge,
                         vector3& BREdge )
{
    if( (m_DirtyFlags & DIRTY_EDGES) || (m_VESystem!=System) )
    {
        f32 FSX,FSY, DX,DY,DZ, L;
        vector3 EY;

        GetFrustum( FSX, FSY );

        // COMPUTE DELTAS
        DZ  = 1.0f;
        DY  = DZ / FSY;
        DX  = DZ / FSX;
        L   = x_1oversqrt(DX*DX + DY*DY + 1.0f);
        DZ *= L;
        DX *= L;
        DY *= L;

        // BUILD UNIT VECTORS
        m_VETL.X = DX;   m_VETL.Y = DY;   m_VETL.Z = DZ;
        m_VETR.X =-DX;   m_VETR.Y = DY;   m_VETR.Z = DZ;
        m_VEBL.X = DX;   m_VEBL.Y =-DY;   m_VEBL.Z = DZ;
        m_VEBR.X =-DX;   m_VEBR.Y =-DY;   m_VEBR.Z = DZ;

        // TRANSFORM VECTORS INTO USERS SPACE
        EY.X = EY.Y = EY.Z = 0;;
        ConvertPoint( System, m_VETL, V_CAMERA, m_VETL);
        ConvertPoint( System, m_VETR, V_CAMERA, m_VETR);
        ConvertPoint( System, m_VEBL, V_CAMERA, m_VEBL);
        ConvertPoint( System, m_VEBR, V_CAMERA, m_VEBR);
        ConvertPoint( System, EY,     V_CAMERA, EY);

        // CONVERT BACK INTO A DIRECTION
        m_VETL -= EY;
        m_VETR -= EY;
        m_VEBL -= EY;
        m_VEBR -= EY;

        // REMEMBER SYSTEM
        m_DirtyFlags &= ~(DIRTY_EDGES);
        m_VESystem = System;
    }

    TLEdge = m_VETL;
    TREdge = m_VETR;
    BLEdge = m_VEBL;
    BREdge = m_VEBR;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  VIEW INQUIRIES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


//=========================================================================

xbool view::PointInView( system System, const vector3& P )
{
    f32     FX,FY;
    vector3 V;

    // Convert pt into camera space
    if( System == V_WORLD )
    {
        matrix4 M;
        GetW2VMatrix( M );
        V.X  = (M.M[0][0]*P.X) + (M.M[1][0]*P.Y) + (M.M[2][0]*P.Z) + M.M[3][0];
        V.Y  = (M.M[0][1]*P.X) + (M.M[1][1]*P.Y) + (M.M[2][1]*P.Z) + M.M[3][1];
        V.Z  = (M.M[0][2]*P.X) + (M.M[1][2]*P.Y) + (M.M[2][2]*P.Z) + M.M[3][2];
    }
    else
    {
        ConvertPoint( V_CAMERA, V, System, P );
    }

    GetFrustum( FX, FY );

    if(  V.Z      < m_ZNear )  return FALSE;       // check near plane
    if(  V.Z      > m_ZFar  )  return FALSE;       // check far plane
    if(  V.X * FX > V.Z     )  return FALSE;       // check right plane
    if( -V.X * FX > V.Z     )  return FALSE;       // check left plane
    if(  V.Y * FY > V.Z     )  return FALSE;       // check top plane
    if( -V.Y * FY > V.Z     )  return FALSE;       // check bottom plane

    return TRUE;
}

//=========================================================================

xbool view::SphereInView( system System, const vector3& P, f32 Radius )
{
    f32      FX, FY;
    f32      Offset;
    radian   YFOV;
    vector3  V;

    ASSERT( Radius >= 0 );

    ConvertPoint( V_CAMERA, V, System, P );
    GetFrustum ( FX, FY );

    if( V.Z + Radius < m_ZNear )  return FALSE;       // check near plane
    if( V.Z - Radius > m_ZFar  )  return FALSE;       // check far plane

    Offset = Radius / x_cos( R_90 - (m_XFOV / 2.0f) );
    if(  V.X * FX > V.Z + Offset )  return FALSE;       // check right plane
    if( -V.X * FX > V.Z + Offset )  return FALSE;       // check left plane

    GetYFOV( YFOV );
    Offset = Radius / x_cos( R_90 - (YFOV / 2.0f) );
    if(  V.Y * FY > V.Z + Offset )  return FALSE;       // check top plane
    if( -V.Y * FY > V.Z + Offset )  return FALSE;       // check bottom plane

    return TRUE;
}

//=========================================================================

void view::PointToScreen( system System, const vector3& P, vector3& ScreenP )
{
    f32 PXC0,PXC1,PYC0,PYC1;
    vector3 V;

    ConvertPoint ( V_CAMERA, V, System, P );
    GetProjection( PXC0, PXC1, PYC0, PYC1 );

    //
    // project the point into the screen
    // V3_Project(ScreenP,&V,1,PXC0,PXC1,PYC0,PYC1);
    //

    if( V.Z >= 0.01f )
    {
        ScreenP.X = PXC0 + PXC1 * ( V.X / V.Z );
        ScreenP.Y = PYC0 + PYC1 * ( V.Y / V.Z );
        ScreenP.Z = V.Z;
    }
    else
    {
        // Z is less than 0.01f so check if it's too
        // close to zero

        f32 SZ = V.Z;

        // Snap SZ to 0.01f if it's too close to zero
        if( SZ > -0.01f )
            SZ = 0.01f;

        // Flip Z if it is negative
        if( SZ < 0.0f ) 
            SZ = -SZ;

        ScreenP.X = PXC0 + PXC1 * ( V.X / SZ );
        ScreenP.Y = PYC0 + PYC1 * ( V.Y / SZ );
        ScreenP.Z = SZ;
    }
}

//=========================================================================

xbool view::PolyInView( vector3* pVert,  s32  NVerts,
                        vector3& TopN,   f32& TopD,
                        vector3& BotN,   f32& BotD,
                        vector3& LftN,   f32& LftD,
                        vector3& RgtN,   f32& RgtD,
                        vector3& NearN,  f32& NearD,
                        vector3& FarN,   f32& FarD )
{
    static vector3 DstV0[4*3*2];
    vector3*   DstV1;

    ASSERT( pVert && (NVerts <= 4) );
    DstV1 = DstV0 + NVerts * 3;

    ClipPolyToPlane( DstV0, NVerts, pVert, NVerts, NearN.X, NearN.Y, NearN.Z, NearD );
    if( NVerts == 0 ) return FALSE;

    ClipPolyToPlane( DstV1, NVerts, DstV0, NVerts, LftN.X, LftN.Y, LftN.Z, LftD );
    if( NVerts == 0 ) return FALSE;

    ClipPolyToPlane( DstV0, NVerts, DstV1, NVerts, BotN.X, BotN.Y, BotN.Z, BotD );
    if( NVerts == 0 ) return FALSE;

    ClipPolyToPlane( DstV1, NVerts, DstV0, NVerts, RgtN.X, RgtN.Y, RgtN.Z, RgtD );
    if( NVerts == 0 ) return FALSE;

    ClipPolyToPlane( DstV0, NVerts, DstV1, NVerts, TopN.X, TopN.Y, TopN.Z, TopD );
    if( NVerts == 0 ) return FALSE;

    ClipPolyToPlane( DstV1, NVerts, DstV0, NVerts, FarN.X, FarN.Y, FarN.Z, FarD );
    if( NVerts == 0 ) return FALSE;

    return TRUE;
}

//=========================================================================

xbool view::OrientedBBoxInView( vector3& Corner, vector3* pAxis )
{
    static const s32  BoxSide[4*6] = {3,1,5,7,  7,5,4,6,  6,4,0,2,
                                      2,0,1,3,  2,3,7,6,  5,1,0,4};
    s32        i,j;
    vector3    VL[4];
    vector3    PlaneN[6];
    f32        PlaneD[6];
    vector3    BD[8];

    // Check if midpoint is in view for TRIVIAL ACCEPTANCE
    if( 1 )
    {
        //vector3 MN,MX;
        vector3 MD;
        f32     FX,FY;
        vector3 V;

        GetFrustum( FX, FY );

        // Get center point
        //MN = Corner;
        //MX = Corner + pAxis[0] + pAxis[1] + pAxis[2];
        //MD.X = (MN.X + MX.X) * 0.5f;
        //MD.Y = (MN.Y + MX.Y) * 0.5f;
        //MD.Z = (MN.Z + MX.Z) * 0.5f;

        MD = Corner;

        // Convert pt into camera space
        V.X  = (m_W2V.M[0][0]*MD.X) + (m_W2V.M[1][0]*MD.Y) + (m_W2V.M[2][0]*MD.Z) + m_W2V.M[3][0];
        V.Y  = (m_W2V.M[0][1]*MD.X) + (m_W2V.M[1][1]*MD.Y) + (m_W2V.M[2][1]*MD.Z) + m_W2V.M[3][1];
        V.Z  = (m_W2V.M[0][2]*MD.X) + (m_W2V.M[1][2]*MD.Y) + (m_W2V.M[2][2]*MD.Z) + m_W2V.M[3][2];

        if((  V.Z      >= m_ZNear ) &&
           (  V.Z      <= m_ZFar  ) &&
           (  V.X * FX <= V.Z     ) && 
           ( -V.X * FX <= V.Z     ) && 
           (  V.Y * FY <= V.Z     ) && 
           ( -V.Y * FY <= V.Z     ))
        {
            return TRUE;
        }
    }

    // Get the view planes in world space
    GetViewPlanes( V_WORLD, PlaneN[3], PlaneD[3],
                            PlaneN[4], PlaneD[4],
                            PlaneN[1], PlaneD[1],
                            PlaneN[2], PlaneD[2],
                            PlaneN[0], PlaneD[0],
                            PlaneN[5], PlaneD[5]);

    // Pick best and worst candidate for being in planes
    if( 1 )
    {
        s32 NWorstIn = 0;

        for( i=0; i < 6; i++ )
        {
            vector3 BestPt;
            vector3 WorstPt;

            BestPt  = Corner;
            WorstPt = Corner;

            for( j = 0; j < 3; j++ )
            {
                if( Dot(pAxis[j],PlaneN[i]) > 0 )
                    BestPt  += pAxis[j];
                else
                    WorstPt += pAxis[j];
            }

            // Check if best point is outside
            if( (Dot(PlaneN[i],BestPt) + PlaneD[i]) < 0 )
            {
                return FALSE;
            }

            // Check if worst point is inside
            if( (Dot(PlaneN[i],WorstPt) + PlaneD[i]) >= 0 )
                NWorstIn++;
        }

        if( NWorstIn == 6 )
        {
            return TRUE;
        }
    }

    // Build the 8 pts on cube
    BD[0] = Corner;
    BD[1] = Corner + pAxis[2];
    BD[2] = Corner + pAxis[1];
    BD[3] = Corner + pAxis[1] + pAxis[2];
    BD[4] = Corner + pAxis[0];
    BD[5] = Corner + pAxis[0] + pAxis[2];
    BD[6] = Corner + pAxis[0] + pAxis[1];
    BD[7] = Corner + pAxis[0] + pAxis[1] + pAxis[2];

    // Use the 8 verts and do TRIVIAL REJECTION & ACCEPTANCE
    if( 1 )
    {
        s32 VertInCount[8]   = {0,0,0,0,0,0,0,0};
        s32 PlaneOutCount[6] = {0,0,0,0,0,0};

        //
        // Check for trivial acceptance or rejection
        //
        for( i = 0; i < 6; i++ )
        {
            for( j = 0; j < 8; j++ )
            {
                if( (PlaneN[i].X*BD[j].X + PlaneN[i].Y*BD[j].Y + PlaneN[i].Z*BD[j].Z + PlaneD[i]) >= 0 )
                {
                    VertInCount[j]++;
                }
                else
                {
                    PlaneOutCount[i]++;
                }
            }

            if( PlaneOutCount[i] == 8 ) 
            {
                return FALSE;
            }
        }

        for( i = 0; i < 8; i++ )
        if( VertInCount[i] == 6 ) 
        {
            return TRUE;
        }
    }

    // build bounding box sides and clip individually
    for( i = 0; i < 6; i++ )
    {
        s32 Result;

        VL[0] = BD[ BoxSide[(i<<2)+0] ];
        VL[1] = BD[ BoxSide[(i<<2)+1] ];
        VL[2] = BD[ BoxSide[(i<<2)+2] ];
        VL[3] = BD[ BoxSide[(i<<2)+3] ];

        Result = PolyInView( VL, 4, PlaneN[3], PlaneD[3],
                                    PlaneN[4], PlaneD[4],
                                    PlaneN[1], PlaneD[1],
                                    PlaneN[2], PlaneD[2],
                                    PlaneN[0], PlaneD[0],
                                    PlaneN[5], PlaneD[5] );

        if( Result == TRUE ) return TRUE;
    }

    return FALSE;
}

//=========================================================================

xbool view::BBoxInView( const vector3& MN, const vector3& MX )
{
    vector3 Corner;
    vector3 Axis[3];

    //
    // Check if eye is in bbox, SUPER TRIVIAL ACCEPTANCE
    //
    if( 1 )
    {
        vector3 Eye;

        GetPosition( Eye );

        if ( ((Eye.X >= MN.X) && (Eye.X <= MX.X)) &&
             ((Eye.Y >= MN.Y) && (Eye.Y <= MX.Y)) &&
             ((Eye.Z >= MN.Z) && (Eye.Z <= MX.Z)) ) return TRUE;
    }

    //
    // Build the eight verts
    //
    Corner = MN;
    Axis[0].X = MX.X - MN.X;
    Axis[0].Y = 0;
    Axis[0].Z = 0;
    Axis[1].X = 0;
    Axis[1].Y = MX.Y - MN.Y;
    Axis[1].Z = 0;
    Axis[2].X = 0;
    Axis[2].Y = 0;
    Axis[2].Z = MX.Z - MN.Z;
    return OrientedBBoxInView( Corner, Axis );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  HIGH LEVEL OPERATIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//=========================================================================

void view::HorizLookAt( system System, const vector3& To)
{
    vector3 V;
    vector3 WorldTo;
    radian  RX,RY;

    // Convert user point into world space
    ConvertPoint( V_WORLD, WorldTo, System, To );

    // Get vector from camera to point
    V = WorldTo - m_WorldPos;

    // Find rotations to look in that direction
    V.PitchYaw( RX, RY );

    // Build new camera orientation
    m_WorldOrient.Identity();
    m_WorldOrient.RotateX( RX );
    m_WorldOrient.RotateY( RY );

    // Reset clean count and dirty flags
    m_CleanCount = VIEW_CLEAN_COUNT;
    m_DirtyFlags |= (DIRTY_W2V | DIRTY_V2W | DIRTY_PLANES | DIRTY_CLIP_PLANES | DIRTY_EDGES);
}

//=========================================================================

void view::OrbitPoint( const vector3& Point, f32 Dist, radian Pitch, radian Yaw )
{
    vector3 Pos( 0, 0, Dist );

    SetPosition( V_WORLD, Pos );
    Rotate     ( V_WORLD, V_AXIS_X, Pitch );
    Rotate     ( V_WORLD, V_AXIS_Y, Yaw   );
    Translate  ( V_WORLD, Point );
    HorizLookAt( V_WORLD, Point );
}

//=========================================================================

void view::GetPitchYaw( radian& Pitch, radian& Yaw )
{
    matrix4 V2W;
    GetV2WMatrix( V2W );

    vector3 V;
    V.Set( V2W.M[2][0], V2W.M[2][1], V2W.M[2][2] );
    V.PitchYaw( Pitch, Yaw );

/* Old Code here just in case
    vector3 E( 0, 0, 0 );
    vector3 V( 0, 0, 1 );

    // convert Z-axis from camera space to world space
    ConvertPoint( V_WORLD, V, V_CAMERA, V );
    ConvertPoint( V_WORLD, E, V_CAMERA, E );
    //V3_Sub( &V, &V, &E );
    V -= E;

    // compute pitch and yaw
    //V3_ZToVecRot( &V, Pitch, Yaw );
    V.PitchYaw( Pitch, Yaw );
    
//    *Pitch = x_asin ( V.Y );
//    *Yaw   = x_atan2( V.X, V.Z );
*/
}


//=========================================================================

void view::GetYaw( radian& Yaw )
{
    matrix4 V2W;
    GetV2WMatrix( V2W );

    vector3 V( V2W.M[2][0], V2W.M[2][1], V2W.M[2][2] );
    // compute yaw
    Yaw = V.Yaw();
}

//=========================================================================

f32 view::FindScreenPercentage( const vector3&  WorldPos,
                                f32             WorldHeight )
{
    vector3 ViewPos;

    f32 PXC0,PXC1,PYC0,PYC1;
    f32 ScreenHeight;
    f32 ScreenSize;

    ASSERT( WorldHeight > 0 );

    ConvertPoint ( V_CAMERA, ViewPos, V_WORLD, WorldPos );
    GetProjection( PXC0, PXC1, PYC0, PYC1 );


    if( ViewPos.Z < 10.0f ) ViewPos.Z = 10.0f;
    ScreenHeight = (f32)( m_ViewB - m_ViewT + 1 );

    ScreenSize  = -PYC1 * WorldHeight;
    ScreenSize /= ViewPos.Z;
    ScreenSize /= ScreenHeight;
    ScreenSize *= 100.0f;

    return ScreenSize;
}

//=========================================================================

f32 view::ScaleToMatchScreenSize( const vector3&  WorldPos,
                                  f32             WorldHeight,
                                  f32             ScreenPercentage )
{
    f32 ScreenSize;

    ASSERT( WorldHeight > 0 );
    ASSERT( ScreenPercentage > 0 );

    ScreenSize = FindScreenPercentage( WorldPos, WorldHeight );
    if( ScreenSize == 0 ) return 0;

    return( ScreenPercentage / ScreenSize );
}

//=========================================================================

void view::GetViewport( s32& Left, s32& Top, s32& Right, s32& Bottom ) const
{
    Left   = m_ViewL;
    Top    = m_ViewT;
    Bottom = m_ViewB;
    Right  = m_ViewR;
}

//=========================================================================

void view::GetPosition( vector3& Position ) const
{
    Position = m_WorldPos;
}

//=========================================================================

void view::GetOrientation( matrix4& Orientation ) const
{
    Orientation = m_WorldOrient;
}

//=========================================================================

void view::GetXFOV( radian& XFOV ) const
{
    XFOV = m_XFOV;
}

//=========================================================================
#if defined(TARGET_PS2)
void view::GetClipXFOV( radian& XFOV ) const
{
    XFOV = m_ClipXFOV;
}
#endif
//=========================================================================

radian view::GetXFOV( void ) const
{
    return m_XFOV;
}

//=========================================================================

void view::GetYFOV( radian& YFOV )
{
    ///if (m_DirtyFlags & DIRTY_YFOV)
    {
        f32 W,H,AspectInvert;

        H  = (f32)(m_ViewB - m_ViewT);
        W  = (f32)(m_ViewR - m_ViewL);
        AspectInvert  = H / W;

        m_YFOV = 2.0f * x_atan2( ((x_tan( m_XFOV * 0.5f ) * m_ZNear) * AspectInvert), m_ZNear );
        m_DirtyFlags &= ~(DIRTY_YFOV);
    }

    YFOV = m_YFOV;
}

//=========================================================================
#if defined(TARGET_PS2)
void view::GetClipYFOV( radian& YFOV )
{
    if (m_DirtyFlags & DIRTY_CLIPYFOV)
    {
        m_ClipYFOV = m_ClipXFOV;

        m_DirtyFlags &= ~(DIRTY_CLIPYFOV);
    }

    YFOV = m_ClipYFOV;
}
#endif
//=========================================================================

radian view::GetYFOV( void )
{
    //--Quag3 Way
    //if (m_DirtyFlags & DIRTY_YFOV)
    {
        f32 W,H,A;

        W  = (f32)(m_ViewB - m_ViewT);
        H  = (f32)(m_ViewR - m_ViewL);
        A  = m_AspectRatio;

        m_YFOV = 1.5f * x_atan( x_tan( m_XFOV * 0.5f ) * (W / (H / A)) );

        m_DirtyFlags &= ~(DIRTY_YFOV);
    }
    return m_YFOV;
}

//=========================================================================

void view::GetZLimits( f32& NearZ, f32& FarZ ) const
{
    NearZ = m_ZNear;
    FarZ  = m_ZFar;
}

//=========================================================================

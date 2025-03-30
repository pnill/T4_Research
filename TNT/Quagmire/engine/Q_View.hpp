////////////////////////////////////////////////////////////////////////////
//
// Q_VIEW.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef Q_VIEW_HPP
#define Q_VIEW_HPP

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_math.hpp"


////////////////////////////////////////////////////////////////////////////
// view class
////////////////////////////////////////////////////////////////////////////

class view
{
public:
    ////////////////////////////////////////////////////////////////////////
    // TYPES
    ////////////////////////////////////////////////////////////////////////

    enum system
    {
        V_WORLD,
        V_CAMERA,
    };
    
    enum axis
    {
        V_AXIS_X,
        V_AXIS_Y,
        V_AXIS_Z
    };

    ////////////////////////////////////////////////////////////////////////
    // FUNCTIONS
    ////////////////////////////////////////////////////////////////////////
     view( void );
    ~view( void );

    ////////////////////////////////////////////////////////////////////////
    // BASIC VIEWPORT PARAMETERS
    ////////////////////////////////////////////////////////////////////////
    //
    // SetViewport accepts the rectangular region of the screen that is to be
    // used as the 'window' in which the scene will be rendered.  The viewport
    // may be slid off screen and will be clipped correctly.  The aspect ratio
    // is computed based off of the monitor resolution that is provided and 
    // the assumption that the video device has a 4:3 ratio.  The XFOV is set 
    // to a default of 60 degrees but can be used for zoom effects and large  
    // FOVs.  The XFOV has a max value of 120 degrees. The ZLimits are used for 
    // clipping as well as zbuffer accuracy so pull them in as close as you 
    // can.
    //
    ////////////////////////////////////////////////////////////////////////

    void    SetViewport         ( s32 Left, s32 Top, s32 Right, s32 Bottom );
    void    SetAspectRatio      ( s32 MonitorResX, s32 MonitorResY );
    void    SetXFOV             ( radian XFOV );
    void    SetXFOVUnclamped    ( radian XFOV );
    void    SetZLimits          ( f32 NearZ, f32 FarZ );

    void    GetXFOV             ( radian& XFOV ) const;
    void    GetYFOV             ( radian& YFOV );
    void    GetAspectRatio      ( f32& AspectRatio ) const;

    radian  GetXFOV             ( void ) const;
    radian  GetYFOV             ( void );
    f32     GetAspectRatio      ( void ) const;

#if defined(TARGET_PS2)
    void    GetClipXFOV             ( radian& XFOV ) const;
    void    GetClipYFOV         ( radian& YFOV );
    void    GetClipAspectRatio      ( f32& AspectRatio ) const;

//    radian  GetXFOV             ( void ) const;
//    radian  GetYFOV             ( void );
//    f32     GetAspectRatio      ( void ) const;
#endif

    void    GetZLimits          ( f32& NearZ, f32& FarZ ) const;
    void    GetViewport         ( s32& Left, s32& Top, s32& Right, s32& Bottom ) const;


    ////////////////////////////////////////////////////////////////////////
    // CAMERA MANIPULATIONS / GYRATIONS
    ////////////////////////////////////////////////////////////////////////
    //
    // SetPosition and Translate can move the camera in either World space or
    // Camera space depending on the system passed in.  These calls update the
    // camera's location, not the direction it's looking.
    //
    // SetRotations and Pivot change the direction the camera is looking while 
    // leaving the camera at it's current location.  SetRotations takes in
    // Pitch, Yaw, and Roll while Pivot takes in a single axis and an angle.
    //
    // Rotate will change the position and orientation of the camera by 
    // rotating the position and direction around a system axis.
    //
    // Pivot will not change the position, but it will change the orientation.
    // During a pivot, the camera is rotated about an axis which is parallel
    // to the specified axis, but which passes thru the camera.  Think of it
    // as "rotating in place".
    //
    ////////////////////////////////////////////////////////////////////////

    void SetPosition        ( system System, const vector3& Position );
    void Translate          ( system System, const vector3& Translation );
    void GetPosition        ( vector3& Position ) const;
    void GetOrientation     ( matrix4& Orientation ) const;

    void SetRotations       ( system System, radian Pitch, radian Yaw, radian Roll );
    void SetRotations       ( system System, const matrix4& Orient );
    void Rotate             ( system System, axis Axis, radian Angle );
    void Pivot              ( system System, axis Axis, radian Angle );


    ////////////////////////////////////////////////////////////////////////
    // WORLD TO VIEW AND VIEW TO WORLD MATRICES
    ////////////////////////////////////////////////////////////////////////
    //
    // The W2V matrix takes points in world space and places them in camera
    // space which is described at the top of this file.  Camera space is not
    // scaled or skewed and it does not depend on the XFOV or viewport.  
    // Points transformed into camera space by this matrix are ready to be
    // projected using the projection values available from the view.
    //
    // The V2W matrix takes a point from camera (view) space and transforms it 
    // out into the world.  If you transformed (0,0,0) you would get the 
    // location of the camera in the world, normally just ask for the position.
    // If you transformed (0,0,1) you'd get a point 1 unit directly in front of
    // the camera.
    //
    // Take a look at the matrix notes in x_math.h and you'll see how you can
    // use these matrices to do things like 3D sprites.
    //
    ////////////////////////////////////////////////////////////////////////

    void GetW2VMatrix       ( matrix4& W2V );
    void GetV2WMatrix       ( matrix4& V2W );

    ////////////////////////////////////////////////////////////////////////
    // WORLD TO CLIP MATRIX
    ////////////////////////////////////////////////////////////////////////
    // 
    // Special case PS2 matrix - takes us from World to homogenous clip
    //  space.
    //
    ////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PS2 )
    void GetW2CMatrix       ( matrix4& W2C );
#endif


    ////////////////////////////////////////////////////////////////////////
    // VIEW EDGES, PLANES, AND FRUSTUM
    ////////////////////////////////////////////////////////////////////////
    //
    // GetViewEdges will return unit vectors in the direction of each edge of
    // the frustum in the system you requested.  These can be used for 
    // rendering frustum sides or projecting the frustum onto a ground plane.
    //
    // GetViewPlanes returns the normal and negative distance along the normal
    // as the plane equation.  Inside of plane = (X*NX + Y*NY + Z*NZ + D >= 0).
    // Notice you can request the planes in either system.
    //
    // GetFrustum returns the slope of the frustum in camera space.  This is
    // useful for clipping and bounds checking of points once they are in
    // camera space.
    //
    //  Right Frustum Edge is when (VX * FrustumSX == VZ)
    //  Top   Frustum Edge is when (VY * FrustumSY == VZ)
    //
    // GetScreenRay takes pixel coordinates on the screen and returns a 
    // direction vector that points from the camera's eye to the point on the
    // screen.
    //
    ////////////////////////////////////////////////////////////////////////

    void GetViewEdges   ( system   System,
                          vector3& TLEdge,
                          vector3& TREdge,
                          vector3& BLEdge,
                          vector3& BREdge );

    void GetViewPlanes  ( system   System,
                          vector3& TopN,  f32& TopD, 
                          vector3& BotN,  f32& BotD, 
                          vector3& LftN,  f32& LftD, 
                          vector3& RgtN,  f32& RgtD,
                          vector3& NearN, f32& NearD,
                          vector3& FarN,  f32& FarD );

#if defined(TARGET_PS2)
    void GetClipViewPlanes( system System,
                            vector3& TopN, f32& TopD,
                            vector3& BotN, f32& BotD,
                            vector3& LftN, f32& LftD,
                            vector3& RgtN, f32& RgtD,
                            vector3& NearN, f32& NearD,
                            vector3& FarN,  f32& FarD );
#endif

    void GetFrustum     ( f32& FrustumSX, 
                          f32& FrustumSY );

#if defined(TARGET_PS2)
    void GetClipFrustum ( f32& FrustumSX, 
                          f32& FrustumSY );
#endif

    void GetScreenRay   ( system    System, 
                          f32       SX, 
                          f32       SY, 
                          vector3&  Dir );


    ////////////////////////////////////////////////////////////////////////
    // PROJECTION CONSTANTS
    ////////////////////////////////////////////////////////////////////////
    //
    // The projection constants are used in projecting points once they are 
    // in camera space.  Here are the equations; they should be familiar to
    // those of you who have done projections before.
    //
    // SX = ProjectXC0 + ProjectXC1*(VX/VZ);
    // SY = ProjectYC0 + ProjectYC1*(VY/VZ);
    //
    // ProjectXC0 is usually the X center of the viewport in screen coordinates
    // ProjectYC0 is usually the Y center of the viewport in screen coordinates
    // ProjectXC1 is a scaling based on viewport X size and aspect ratio
    // ProjectYC1 is a scaling based on viewport Y size and aspect ratio
    //
    // You'll get standard screen coordinates in SX,SY.
    //
    ////////////////////////////////////////////////////////////////////////

    void GetProjection  ( f32& ProjectXC0, 
                          f32& ProjectXC1,
                          f32& ProjectYC0, 
                          f32& ProjectYC1 );


    ////////////////////////////////////////////////////////////////////////
    // POINT CONVERSIONS
    ////////////////////////////////////////////////////////////////////////
    //
    // PointToScreen takes a point in either system and projects it up onto
    // the screen.  You'll get standard screen coordinates based on the view's
    // viewport, aspect ratio, xfov... the whole nine yards.  
    //
    // ConvertPoint converts back and forth between the two systems, CAMERA
    // space and WORLD space.  If you need a point 5 units to the left of the
    // camera, convert (5,0,0) from camera space to world space.
    //
    ////////////////////////////////////////////////////////////////////////

    void PointToScreen  ( system System, const vector3& P, vector3& ScreenP );
    void ConvertPoint   ( system DSTSystem, vector3& Dst, 
                          system SRCSystem, const vector3& Src );


    ////////////////////////////////////////////////////////////////////////
    // IN VIEW REQUESTS
    ////////////////////////////////////////////////////////////////////////
    //
    // PointInView will return if a point is visible given this view.  Notice
    // that either system is available.
    //
    // SphereInView is much like PointInView except now you can specify a 
    // radius about the point to consider as well.
    //
    // BBoxInView will return if a world space axis aligned bounding box is in
    // the view.  This works well for balls, people, stadium pieces...  The
    // usual shortcuts are taken into account but it is accurate and robust.
    // MN contains the minimum WORLD x,y,z of the box and MX contains the 
    // maximum values.
    //
    ////////////////////////////////////////////////////////////////////////

    xbool PointInView           ( system System, const vector3& P );
    xbool SphereInView          ( system System, const vector3& P, f32 Radius );
    xbool BBoxInView            ( const vector3& MN, const vector3& MX );
    xbool OrientedBBoxInView    ( vector3& Corner, vector3* pAxis );


    ////////////////////////////////////////////////////////////////////////
    // NO ROLL SITUATIONS - USEFUL SHORTCUTS
    ////////////////////////////////////////////////////////////////////////
    //
    // HorizLookAt takes the camera at it's current position and aims it at the
    // point you specify.  The camera does not roll so Y is always up.
    //
    // GetPitchYaw will return the pitch and yaw of the current camera 
    // orientation.
    //
    // OrbitPoint will set the location of the camera as well as the 
    // orientation to make it look at the point specified.  The dist is the 
    // dist from the point and the pitch and yaw specify how the camera is 
    // rotated around the point.
    //
    ////////////////////////////////////////////////////////////////////////

    void HorizLookAt    ( system System, const vector3& To );
    void GetPitchYaw    ( radian& Pitch, radian& Yaw );
    void GetYaw         ( radian& Yaw );
    void OrbitPoint     ( const vector3& Point, f32 Dist, radian Pitch, radian Yaw );


    ////////////////////////////////////////////////////////////////////////
    // SCREEN SIZE FUNCTIONS - GREAT FOR LOD
    ////////////////////////////////////////////////////////////////////////
    //
    // FindScreenPercentage will return to you the precentage of the screen
    // vertically that an object at that world position and of that world 
    // height would fill the screen.  The location of the camera, field of 
    // view (zoom), and the size of the object are all taken into account.  If
    // the function returns 100.0 then the object should take up the same 
    // number of pixels vertically as the current viewport is using.  One 
    // example is for character LOD.  By passing in the position of the player,
    // his general height = 72 inches, you will get back the percentage that
    // he is taking up on the screen.  By using that to determine LOD it won't
    // matter if the camera is close or just zoomed in.
    //
    // ScaleToMatchScreenSize is a nifty function if you have an object out
    // in the world and you want it to end up a certain size on the screen. 
    // For example, if you have a baseball that is 3.0 inches 'big' and you want
    // to make sure that it always covers at least 1.6% of the screen, or about
    // 8 pixels on a 480 tall resolution = (8/480)*100 = (1.6).  What you do
    // is call ScaleToMatchScreenSize with the world position of the ball, the
    // world 'height' of the ball = 3.0, and the desired screen percentage of
    // (1.6).  The value returned is how much to scale the ball by in order to
    // make it match the percentage you requested of the screen.  If the scale
    // returned is greater than 1.0 then the ball must be scaled UP to cover
    // 8 pixels.  If the returned value is less than 1.0 then the ball would
    // already cover 8 or more pixels and must be scaled down so you might not 
    // want to apply the scale.
    //
    ////////////////////////////////////////////////////////////////////////

    f32 FindScreenPercentage   ( const vector3&  WorldPos, 
                                 f32             WorldHeight );

    f32 ScaleToMatchScreenSize ( const vector3&  WorldPos, 
                                 f32             WorldHeight, 
                                 f32             ScreenPercentage );


    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    // PROTECTED !!! PROTECTED !!! PROTECTED !!! PROTECTED !!! PROTECTED !!! 
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////

protected:

    ////////////////////////////////////////////////////////////////////////
    // FUNCTIONS
    ////////////////////////////////////////////////////////////////////////

    void  Clean     ( void );

    xbool PolyInView( vector3* pVert,  s32  NVerts,
                      vector3& TopN,   f32& TopD,
                      vector3& BotN,   f32& BotD,
                      vector3& LftN,   f32& LftD,
                      vector3& RgtN,   f32& RgtD,
                      vector3& NearN,  f32& NearD,
                      vector3& FarN,   f32& FarD );


    ////////////////////////////////////////////////////////////////////////
    // VARIABLES
    ////////////////////////////////////////////////////////////////////////

    xbool       m_Locked;                     // TRUE -> cannot change internal values   
    vector3     m_WorldPos;                   // Camera translation in world coordinates
    s32         m_CleanCount;                 // Operations before matrix cleaning is necessary
    matrix4     m_WorldOrient;                // Orients camera from Camera -> World space
    matrix4     m_W2V;                        // Transforms from World->View
    matrix4     m_V2W;                        // Transforms from View->World

    s32         m_ViewL;                      // Edges of viewport
    s32         m_ViewT;
    s32         m_ViewR;
    s32         m_ViewB;
    f32         m_AspectRatio;                // Y*AspectRatio = X
    radian      m_XFOV;
    radian      m_YFOV; 

#if defined( TARGET_PS2 )
    f32         m_ClipAspectRatio;            // Y*AspectRatio = X
    radian      m_ClipXFOV;
    radian      m_ClipYFOV; 
    matrix4     m_W2C;                        // Transforms from World->Clip

    vector3     m_ClipTopN;                   // Inside normal for top clip vplane in world space
    vector3     m_ClipBotN;                   // Inside normal for bot clip vplane in world space
    vector3     m_ClipLftN;                   // Inside normal for lft clip vplane in world space
    vector3     m_ClipRgtN;                   // Inside normal for rgt clip vplane in world space
    f32         m_ClipTopD;                   // IsPointIn = (X*NX + Y*NY + Z*NZ + D > 0);
    f32         m_ClipBotD; 
    f32         m_ClipLftD; 
    f32         m_ClipRgtD;
#endif
    f32         m_ZNear, m_ZFar;              // Z limits in view space
    f32         m_ProjectXC0;                 // Center of viewport
    f32         m_ProjectXC1;                 // Accounts for aspect ratio, viewport size, and FOV
    f32         m_ProjectYC0;                 // SY = ProjectYC0 + ProjectYC1*VY/VZ;
    f32         m_ProjectYC1;

    f32         m_FrustumSX;                 // X Frustum Slope
    f32         m_FrustumSY;                 // Top Frustum Edge = (FrustumSY*VY == VZ)

#if defined(TARGET_PS2)
    f32         m_ClipFrustumSX;             // X Clip Frustum Slope
    f32         m_ClipFrustumSY;             // Top Clip Frustum Edge = (FrustumSY*VY == VZ)
#endif

    vector3     m_VETL, m_VEBL, 
                m_VETR, m_VEBR;               // Normalized direction vectors of view volume edges
    system      m_VESystem;

    vector3     m_TopN;                       // Inside normal for top vplane in world space
    vector3     m_BotN;                       // Inside normal for bot vplane in world space
    vector3     m_LftN;                       // Inside normal for lft vplane in world space
    vector3     m_RgtN;                       // Inside normal for rgt vplane in world space
    vector3     m_NearN;                      // Inside normal for near z plane
    vector3     m_FarN;                       // Inside normal for far z plane
    f32         m_TopD;                       // IsPointIn = (X*NX + Y*NY + Z*NZ + D > 0);
    f32         m_BotD; 
    f32         m_LftD; 
    f32         m_RgtD;
    f32         m_NearD;
    f32         m_FarD;
    system      m_VPSystem;                   // System view planes are in

    u32         m_DirtyFlags;                 // See DIRTY_??? defines
};

////////////////////////////////////////////////////////////////////////////

#endif // Q_VIEW_HPP
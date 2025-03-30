//==========================================================================
//
//
//
//
//==========================================================================


#ifndef __VISIBILITY_HPP_INCLUDED__
#define __VISIBILITY_HPP_INCLUDED__

////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_math.hpp"


////////////////////////////////////////////////////////////////////////////
//  Defines
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PS2 )
#define USE_PS2_ASM 0x1
#endif




////////////////////////////////////////////////////////////////////////////
//  Helper Functions
////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//  View Frustrum Culling Methods
////////////////////////////////////////////////////////////////////////////

enum EViewTestResults
{
    OUTSIDE_VIEW = 0,
    INSIDE_VIEW,
    PARTIAL_VIEW
};


//==========================================================================
// FUNCTION:    OrientedBBoxInView
// 
// DESCRIPTION: Test whether or not an oriented bounding box is in view.
//
// INPUTS:      1) The corner of a bounding box
//              2) 3 Axes describing the dimensions of the bounding box
//              3) 6 plane normals describing the view frustum (N,L,R,T,B,F)
//              4) 6 plane distances describing the view frustum
//              5) Test for trivial acceptance only, results will be
//                 INSIDE_VIEW or PARTIAL_VIEW, and a full test will not
//                 be performed.
//              6) whether or not z-plane intersections are allowed (if they are
//                 not, it will return OUTSIDE_VIEW if a near plane intersection
//                 occurs). Characters may want to do this, but other
//                 geometry will not.
//
//==========================================================================

EViewTestResults OrientedBBoxInView( const vector3& Corner,
                                     const vector3* Axes,
                                     const vector3* PlaneN,
                                     const f32* PlaneD,
                                     xbool TrivialAcceptOnly = FALSE,
                                     xbool AllowNearZIntersection = TRUE );

EViewTestResults OrientedBBoxInView( const vector4& Corner,
                                     const vector4* Axes,
                                     const vector3* PlaneN,
                                     const f32* PlaneD,
                                     xbool TrivialAcceptOnly = FALSE,
                                     xbool AllowNearZIntersection = TRUE );


//==========================================================================
// FUNCTION:    SphereInView
// 
// DESCRIPTION: Test whether or not a sphere is in view.
//
// INPUTS:      1) The center point of the sphere
//              2) The radius of the sphere
//              3) 6 plane normals describing the view frustum (N,L,R,T,B,F)
//              4) 6 plane distances describing the view frustum
//
//==========================================================================

EViewTestResults SphereInView( const vector3& Center,
                               f32 Radius,
                               const vector3* PlaneN,
                               const f32* PlaneD );


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  INLINES INLINES INLINES INLINES INLINES
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

inline EViewTestResults SphereInView( const vector3& Center,
                               f32 Radius,
                               const vector3* PlaneN,
                               const f32* PlaneD )
{
    s32 i;
    f32 t;

    for ( i = 0; i < 6; i++ )
    {
        //---   find the distance from the sphere's center is to the plane
        t = PlaneN[i].Dot( Center ) + PlaneD[i];

        //---   is the sphere completely behind the plane?
        if ( (t + Radius) < 0.0f )
            return OUTSIDE_VIEW;

        //---   does the sphere intersect with the plane?
        if ( (t - Radius) < 0.0f )
            return PARTIAL_VIEW;
    }

    return INSIDE_VIEW;
}

#endif // __VISIBILITY_HPP_INCLUDED__
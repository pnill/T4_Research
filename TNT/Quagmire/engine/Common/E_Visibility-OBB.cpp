//==========================================================================
//
//
//
//
//==========================================================================

////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"
#include "x_time.hpp"
#include "Q_Visibility.hpp"



//==========================================================================
// ASSEMBLY VERSION OF MATRIX MULTIPLY
//==========================================================================
/*
static void AsmM4_Mult( matrix4& rDest, const matrix4& rSrc1, const matrix4& rSrc2 )
{
#ifdef TARGET_PS2

    ASSERTS( ((u32)&rDest & 0xf) == 0, "Matrices must be aligned to 16 bytes" );
    ASSERTS( ((u32)&rSrc1 & 0xf) == 0, "Matrices must be aligned to 16 bytes" );
    ASSERTS( ((u32)&rSrc2 & 0xf) == 0, "Matrices must be aligned to 16 bytes" );

    // VU0 Assembly version:
    asm __volatile__
    ("

        LQC2    vf08, 0x00(%1)  # load the Src1 matrix vectors into vf8..vf11
        LQC2    vf09, 0x10(%1)
        LQC2    vf10, 0x20(%1)
        LQC2    vf11, 0x30(%1)

        LQC2    vf12, 0x00(%2)  # load the src2 matrix vectors into vf12..vf15
        LQC2    vf13, 0x10(%2)
        LQC2    vf14, 0x20(%2)
        LQC2    vf15, 0x30(%2)

        VMULAx.xyzw     ACC,    vf08,   vf12x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf12y
        VMADDAz.xyzw    ACC,    vf10,   vf12z
        VMADDw.xyzw    vf02,    vf11,   vf12w

        VMULAx.xyzw     ACC,    vf08,   vf13x       # do the second column
        VMADDAy.xyzw    ACC,    vf09,   vf13y
        VMADDAz.xyzw    ACC,    vf10,   vf13z
        VMADDw.xyzw    vf03,    vf11,   vf13w

        VMULAx.xyzw     ACC,    vf08,   vf14x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf14y
        VMADDAz.xyzw    ACC,    vf10,   vf14z
        VMADDw.xyzw    vf04,    vf11,   vf14w

        VMULAx.xyzw     ACC,    vf08,   vf15x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf15y
        VMADDAz.xyzw    ACC,    vf10,   vf15z
        VMADDw.xyzw    vf05,    vf11,   vf15w

        SQC2    vf02, 0x00(%0)        # store the result
        SQC2    vf03, 0x10(%0)
        SQC2    vf04, 0x20(%0)
        SQC2    vf05, 0x30(%0)

    ": "+r" (&rDest) : "r" (&rSrc1) , "r" (&rSrc2) );
#else
    rDest = rSrc1 * rSrc2;
#endif
}
*/

////////////////////////////////////////////////////////////////////////////
// THE PS2 HAS SOME SPECIAL ASSEMBLY CODE TO DO SOME OF THE ROUTINES
// (Written by yours truly, Darrin Stewart!)
////////////////////////////////////////////////////////////////////////////

#if defined(TARGET_PS2) && defined(USE_PS2_ASM)

#define     OBB_IN_VIEW_BASIC(x_,y_,z_,u_,v_,w_)        _asm_WorldOBBoxInViewBasic(x_,y_,z_,u_,v_,w_)
#define     OBB_IN_VIEW_ADVANCED(x_,y_,z_,w_)           _asm_WorldOBBoxInViewAdvanced(x_,y_,z_,w_)
extern "C" s32 _asm_WorldOBBoxInViewBasic( const vector3* Corner, const vector3* Axes, const vector3* PlaneN, const f32* PlaneD, xbool TrivialAcceptOnly, xbool AllowNearZIntersection );
extern "C" s32 _asm_WorldOBBoxInViewAdvanced( const vector3* Corner, const vector3* Axes, const vector3* PlaneN, const f32* PlaneD );

#else

#define     OBB_IN_VIEW_BASIC(x_,y_,z_,u_,v_,w_)        OBBoxInViewBasic(x_,y_,z_,u_,v_,w_)
#define     OBB_IN_VIEW_ADVANCED(x_,y_,z_,w_)           OBBoxInViewAdvanced(x_,y_,z_,w_)

#endif // defined(TARGET_PS2) && defined(USE_PS2_ASM)


#if !defined(TARGET_PS2) || !defined(USE_PS2_ASM)

//==========================================================================
// Function:    OBBIntLineToPlane
// 
// Description:	Figures out the intersection between a line and plane
//
// Inputs:      pV0, pV1 are the endpoints of the line
//              pPlaneN is the plane normal
//              D is the plane's distance from the origin
//
// Outputs:     pDest will hold the intersection point
//==========================================================================

void OBBIntLineToPlane( vector3* pDest, const vector3* pV0, const vector3* pV1,
                         const vector3* pPlaneN, f32 D )
{
    f32 s, t;

    //---   figure out the projection of the line endpoints on the plane's
    //      normal
    s = ( pV0->X*pPlaneN->X + pV0->Y*pPlaneN->Y + pV0->Z*pPlaneN->Z + D );
    t = ( pV1->X*pPlaneN->X + pV1->Y*pPlaneN->Y + pV1->Z*pPlaneN->Z + D );
    
    //---   if the line segment doesn't cross the plane, we can't clip it
    if ( (s<0) == (t<0) )
        return;

    //---   let t be the length of the projected line segment
    t = s - t;

    //---   if the line segment is close to parallel with the plane, clip
    //      it at the midway point, otherwise figure out a parametric
    //      value to clip at.
    if( t < 0.001f && t > -0.001f ) t = 0.5f;
    else                            t = s / t;
    
    //---   perform the interpolation
    pDest->X = pV0->X + t*( pV1->X - pV0->X );
    pDest->Y = pV0->Y + t*( pV1->Y - pV0->Y );
    pDest->Z = pV0->Z + t*( pV1->Z - pV0->Z );
}

//==========================================================================
// Function:    OBBClipPolyToPlane
// 
// Description:	Given a poly, clip it to a plane, and return the number of
//              clipped verts.
//
// Inputs:      pDest is the place to store clipped verts
//              pSrc is the 
//
// Outputs:     pDest will hold the intersection point
//==========================================================================

xbool OBBClipPolyToPlane( vector3*  pDest,
                       s32&      NDest,
                       const vector3*  pSrc,
                       s32       NSrc,
                       const vector3&  PlaneN,
                       f32       D )
{

    const vector3*    p0;
    const vector3*    p1;
    vector3*    cp;
    s32         NewCount=0;
    xbool       p0in,p1in;
    s32         i;
    xbool       AnythingClipped = FALSE;

    ASSERT(pDest);
    ASSERT(pSrc);

    //---   Check for simple return;
    NDest = 0;
    if( NSrc == 0 ) return FALSE;

    //---   Get side of plane point is on
    p0   = &pSrc[ NSrc - 1 ];
    p0in = ( (p0->X*PlaneN.X + p0->Y*PlaneN.Y + p0->Z*PlaneN.Z + D) >= 0);

    //---   Loop through points
    for( i = 0; i < NSrc; i++ )
    {
        //---   Get side of plane point is on
        p1   = &pSrc[i];
        p1in = ( (p1->X*PlaneN.X + p1->Y*PlaneN.Y + p1->Z*PlaneN.Z + D) >= 0);

        //---   Check if we include this point
        if( p0in )
        {
            pDest[ NewCount ] = *p0;
            NewCount++;
        }

        //---   Check if we need to clip these points
        if( p0in != p1in )
        {
            AnythingClipped = TRUE;

            //---   get new point in destination for clipped point
            cp = &pDest[NewCount];
            NewCount++;

            OBBIntLineToPlane( cp, p0, p1, &PlaneN, D );
        }

        //---   Move to next line segment
        p0   = p1;
        p0in = p1in;
    }

    //---   Return new count to user
    NDest = NewCount;
    
    return AnythingClipped;
}

//==========================================================================

static
xbool PolyInView( vector3* pVert,  s32 NVerts,
                  const vector3& TopN,   const f32& TopD,
                  const vector3& BotN,   const f32& BotD,
                  const vector3& LftN,   const f32& LftD,
                  const vector3& RgtN,   const f32& RgtD,
                  const vector3& NearN,  const f32& NearD,
                  const vector3& FarN,   const f32& FarD)
{
    static vector3 DstV0[4*3*2];
    vector3*   DstV1;

    ASSERT( pVert && (NVerts<=4));
    DstV1 = DstV0 + NVerts*3;

    OBBClipPolyToPlane( DstV0, NVerts, pVert, NVerts, NearN, NearD );
    if( NVerts == 0 ) return FALSE;

    OBBClipPolyToPlane( DstV1, NVerts, DstV0, NVerts, LftN, LftD );
    if( NVerts == 0 ) return FALSE;

    OBBClipPolyToPlane( DstV0, NVerts, DstV1, NVerts, BotN, BotD );
    if( NVerts == 0 ) return FALSE;

    OBBClipPolyToPlane( DstV1, NVerts, DstV0, NVerts, RgtN, RgtD );
    if( NVerts == 0 ) return FALSE;

    OBBClipPolyToPlane( DstV0, NVerts, DstV1, NVerts, TopN, TopD );
    if( NVerts == 0 ) return FALSE;

    OBBClipPolyToPlane( DstV1, NVerts, DstV0, NVerts, FarN, FarD );
    if( NVerts == 0 ) return FALSE;

    return TRUE;
}

//==========================================================================

s32 OBBoxInViewBasic( vector3* Corner, const vector3* Axes, const vector3* PlaneN, const f32* PlaneD, xbool TrivialAcceptOnly, xbool AllowNearZIntersection )
{
    s32 Res;
    s32 NIntersections;
    s32 i, j;

    Res = 3;
    {
        vector3 BestPt;
        vector3 WorstPt;

        NIntersections = 0;
        for ( i = 0; i < 6; i++ )
        {
            BestPt = *Corner;
            WorstPt = *Corner;

            //---   find the two corners of the OBB that describes the diagonal
            //      that is most closely aligned with the plane's normal. These
            //      two corners should be opposites in the OBB. If you think
            //      of this diagonal running in the general direction of the plane
            //      then the direction goes from WorstPt to BestPt. BestPt will
            //      have the best chance of being in view, and WorstPt will
            //      have the worst chance of being in view. This is pretty damn
            //      confusing, but if you diagram it out on paper, you can see
            //      what is going on.
            for ( j = 0; j < 3; j++ )
            {
                if ( Axes[j].Dot( PlaneN[i] ) > 0.0f )
                    BestPt  += Axes[j];
                else
                    WorstPt += Axes[j];
            }

            //---   check if best point is outside. this is done by
            //      plugging BestPt into the plane equation. If the result
            //      is less than zero, it is behind the plane, otherwise
            //      it is either in the plane or in front of the plane.
            //---   Plane equation is Ax+By+Cz+D = 0, or A*Pt+D = 0
            if ( (PlaneN[i].Dot( BestPt ) + PlaneD[i]) < 0.0f )
            {
                Res = 0;
                break;
            }

            //---   check if the worst pt is in, this means the entire bounding
            //      box is in front of this plane
            if ( (PlaneN[i].Dot( WorstPt ) + PlaneD[i]) >= 0.0f )
            {
                ;
            }
            else
            {
                //---   the bounding box intersects with this plane
                NIntersections++;

                //---   If this plane happens to be the NearZ plane, consider it culled,
                //      'cause clipping players to NearZ is a MAJOR pain in the ass.
                if ( !AllowNearZIntersection && (i == 0) )
                {
                    Res = 0;
                    break;
                }
            }
        }

        //---   if the bounding box is entirely in front of all six planes, that means
        //      we are completely inside the view frustum
        if ( (Res == 3) && (NIntersections == 0) )
        {
            Res = 1;
        }

        //---   if we only care about trivial acceptance, bail out early
        if ( (Res == 3) && TrivialAcceptOnly )
        {
            Res = 2;
        }

        //---   if the bounding box only crosses one plane, we are guaranteed that
        //      it is not entirely out of view...if it had intersected two or more
        //      planes, we could potentially have a problem, cause it could still
        //      be out of view:
        //          /\											.
        //        /   \											.
        //      /     /          OUT							.
        //    /     /											.
        //    \   / +-------------------						.
        //     \/   |											.
        //          |   IN										.
        //          |											.
        //  OUT     |											.
        //          |											.
        //
        //      if the bounding box crosses all 6 planes, that means the bounding box
        //      encompasses the entire view frustum, so it is guaranteed to be visible
        if ( (Res == 3) && ((NIntersections == 1) || (NIntersections == 6)) )
        {
            Res = 2;
        }
    }

    return Res;
}

//==========================================================================

s32 OBBoxInViewAdvanced( vector3* Corner, const vector3* Axes, const vector3* PlaneN, const f32* PlaneD )
{
    s32     i;
    xbool   Result;
    vector3 BD[8];
    vector3 VL[4];
    static const s32  BoxSide[4*6] = {3,1,5,7,  7,5,4,6,  6,4,0,2,
                                      2,0,1,3,  2,3,7,6,  5,1,0,4};
    BD[0] = *Corner;
    BD[1] = *Corner + Axes[2];
    BD[2] = *Corner + Axes[1];
    BD[3] = *Corner + Axes[1] + Axes[2];
    BD[4] = *Corner + Axes[0];
    BD[5] = *Corner + Axes[0] + Axes[2];
    BD[6] = *Corner + Axes[0] + Axes[1];
    BD[7] = *Corner + Axes[0] + Axes[1] + Axes[2];

    for ( i = 0; i < 6; i++ )
    {
        VL[0] = BD[BoxSide[(i<<2)+0]];
        VL[1] = BD[BoxSide[(i<<2)+1]];
        VL[2] = BD[BoxSide[(i<<2)+2]];
        VL[3] = BD[BoxSide[(i<<2)+3]];

        Result = PolyInView( VL, 4, PlaneN[3], PlaneD[3],
                                    PlaneN[4], PlaneD[4],
                                    PlaneN[1], PlaneD[1],
                                    PlaneN[2], PlaneD[2],
                                    PlaneN[0], PlaneD[0],
                                    PlaneN[5], PlaneD[5] );
        if ( Result == TRUE )
        {
            return 2;
        }
    }

    return 0;
}

#endif // !defined(TARGET_PS2) || !defined(USE_PS2_ASM)

//==========================================================================

EViewTestResults OrientedBBoxInView( const vector3& Corner,
                                     const vector3* Axes,
                                     const vector3* PlaneN,
                                     const f32* PlaneD,
                                     xbool TrivialAcceptOnly,
                                     xbool AllowNearZIntersection )
{

    ////////////////////////////////////////////////////////////////////////
    //---   try to do a basic oriented b-box check
    ////////////////////////////////////////////////////////////////////////
    s32 Res = OBB_IN_VIEW_BASIC( const_cast<vector3*>((const vector3*)&Corner), Axes, PlaneN, PlaneD, TrivialAcceptOnly, AllowNearZIntersection );

    ////////////////////////////////////////////////////////////////////////
    //---   the result was indeterminate, meaning that it the bounding box
    //      intersected with some view planes, but we don't know where it
    //      intersected the view planes. Do an advanced check which actually
    //      clips the bounding box polys against the 6 planes.
    ////////////////////////////////////////////////////////////////////////
    if ( Res == 3 )
    {
        Res = OBB_IN_VIEW_ADVANCED( const_cast<vector3*>((const vector3*)&Corner), Axes, PlaneN, PlaneD );
    }

    return (EViewTestResults)Res;
}



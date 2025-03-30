///////////////////////////////////////////////////////////////////////////
//
// AM_PLAY.C
//
///////////////////////////////////////////////////////////////////////////

#include "AM_Play.h"
#include "x_debug.hpp"
#include "x_math.hpp"
#include "x_plus.hpp"


///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

//#define	FRAMES_HALVED
#define	FRAMES_FRACTIONAL

#ifndef MAX_ANIMGROUPS
#define	MAX_ANIMGROUPS	3
#endif


///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PLAYBACK ROUTINES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/*
inline f32 LPR( f32 A, f32 B )
{
    A = (((s32)(A*65536.0f)) % ((s32)(B*65536.0f)))/65536.0f;
    if( A < 0 ) A += B;
    return A;
}
*/

///////////////////////////////////////////////////////////////////////////
xbool  ANIM_WhenIsEvent        (s32				AnimID,
                                s16				EventID,
                                f32*			Frame,
								anim_group*		AnimGroup )
{
	anim_info* pAnim = &AnimGroup->AnimInfo[AnimID];
    anim_event* EV;
    anim_event* EVEnd;
    
    ASSERT(AnimGroup);
    ASSERT((AnimID >= 0) && (AnimID < AnimGroup->NAnimations));

    EV = AnimGroup->AnimEvent + pAnim->EventIndex;
	EVEnd = &EV[pAnim->NEvents];
    while (EV != EVEnd)
	{
		if (EV->EventID == EventID)
		{
			*Frame = (f32)(EV->Frame);
			return TRUE;
		}
		EV++;
	}

    return FALSE;
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PROJECT FUNCTIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static inline void CompRelativePt(	f32            Scale,
									vector3*       StartWorldPos,
									radian         StartWorldRootDir,
									vector3*       AnimTargetPos,
									radian         AnimTargetRootDir,
									vector3*       AnimBasePos,
									radian         AnimBaseRootDir,
									vector3*       FinalWorldPos,
									radian*        FinalWorldRootDir )
{
    vector3  P;
    radian   R;
    f32      S,C;
    f32      X,Z;

    // Make target pos relative to base pos
    P.X = AnimTargetPos->X - AnimBasePos->X;
    P.Y = AnimTargetPos->Y;
    P.Z = AnimTargetPos->Z - AnimBasePos->Z;

    // Scale relative pos
    P.X *= Scale;
    P.Y *= Scale;
    P.Z *= Scale;

    // Find angle to rotate relative pos by
    R = StartWorldRootDir - AnimBaseRootDir;
    x_sincos( R, S, C );
    X   = P.X;
    Z   = P.Z;
    P.X = (C*X) + (S*Z);
    P.Z = (C*Z) - (S*X);

    // Translate base to world pos
    FinalWorldPos->X = P.X + StartWorldPos->X;
    FinalWorldPos->Y = P.Y;
    FinalWorldPos->Z = P.Z + StartWorldPos->Z;

    // Compute root dir
    if (FinalWorldRootDir)
    {
        *FinalWorldRootDir = StartWorldRootDir + (AnimTargetRootDir - AnimBaseRootDir);
    }
}

///////////////////////////////////////////////////////////////////////////

void ANIM_ProjectEndOfAnim(	s32					AnimID,
                            f32					WorldScale,
                            xbool				Mirror,
                            radian				StartWorldRootDir,
                            vector3*			StartWorldPos,
                            radian*				FinalWorldRootDir,
                            vector3*			FinalWorldPos,
                            s32*				AnimFrames,
							anim_group*			AnimGroup)
{
    anim_info*  Anim;
    vector3     S,E;
    f32         RD0,RD1;
	f32			AngleToRad;

    ASSERT((AnimID>=0) && (AnimID < AnimGroup->NAnimations));
    ASSERT(FinalWorldRootDir && FinalWorldPos && AnimFrames);

    // Get Ptr to animinfo
    Anim = &AnimGroup->AnimInfo[AnimID];
    *AnimFrames = Anim->NFrames;
	AngleToRad = EXPORTBITS_GETANGLETORADIANS(Anim->ExportBits);

    // Build end of animation position
    S.X = 0.0f;//Anim->TransXAtFrame0 * VALUE_TO_INCHES;
    S.Y = Anim->TransYAtFrame0 * VALUE_TO_INCHES;
    S.Z = 0.0f;//Anim->TransZAtFrame0 * VALUE_TO_INCHES;
    E.X = Anim->TransXAtFrameN * VALUE_TO_INCHES;
    E.Y = Anim->TransYAtFrameN * VALUE_TO_INCHES;
    E.Z = Anim->TransZAtFrameN * VALUE_TO_INCHES;
    RD0 = R_0;//Anim->RootDirAtFrame0*AngleToRad;
    RD1 = Anim->RootDirAtFrameN*AngleToRad;

    // Mirror translation
    if (Mirror)
    {
        S.X = -S.X;
        E.X = -E.X;
        RD0 = -RD0;
        RD1 = -RD1;
    }

    CompRelativePt (WorldScale,
                    StartWorldPos,
                    StartWorldRootDir,
                    &E,
                    RD1,
                    &S,
                    RD0,
                    FinalWorldPos,
                    FinalWorldRootDir);
}

//////////////////////////////////////////////////////////////////////////

void ANIM_ProjectEvent(	s32					AnimID,
                        s32					EventID,
                        f32					WorldScale,
                        xbool				Mirror,
                        radian				StartWorldRootDir,
                        vector3*			StartWorldPos,
                        vector3*			EventWorldPos,
                        vector3*			RootWorldPos,
                        s32*				EventFrame,
						anim_group*			AnimGroup)
{
    anim_event* EV;
    anim_info*  Anim;
    s32         i, EC, EI;
    vector3     E,R,A;
    radian      RD0;
	f32			AngleToRad;

    ASSERT((AnimID>=0) && (AnimID < AnimGroup->NAnimations));
    ASSERT(EventWorldPos && EventFrame);

    // Get Ptr to animinfo
    Anim = &AnimGroup->AnimInfo[AnimID];
    ASSERT((Anim->NEvents>0) && "Anim has no events");

    // Search for requested event
    for (i=0, EC=Anim->NEvents, EI=Anim->EventIndex; i<EC; i++)
	{
		if (AnimGroup->AnimEvent[ EI + i ].EventID == EventID)
		   break;
	}
    ASSERT((i!=Anim->NEvents) && "Could not find event");
    EV = &AnimGroup->AnimEvent[ EI + i ];

	AngleToRad = EXPORTBITS_GETANGLETORADIANS(Anim->ExportBits);

    // Build initial event pos
    E.X = EV->PX * VALUE_TO_INCHES;
    E.Y = EV->PY * VALUE_TO_INCHES;
    E.Z = EV->PZ * VALUE_TO_INCHES;
    R.X = EV->RPX * VALUE_TO_INCHES;
    R.Y = EV->RPY * VALUE_TO_INCHES;
    R.Z = EV->RPZ * VALUE_TO_INCHES;
    A.X = 0.0f;//Anim->TransXAtFrame0 * VALUE_TO_INCHES;
    A.Y = Anim->TransYAtFrame0 * VALUE_TO_INCHES;
    A.Z = 0.0f;//Anim->TransZAtFrame0 * VALUE_TO_INCHES;
    RD0 = R_0;//Anim->RootDirAtFrame0*AngleToRad;

    if (Mirror)
    {
        E.X = -E.X;
        R.X = -R.X;
        A.X = -A.X;
        RD0 = -RD0;
    }

    // Compute Event pos
    CompRelativePt (WorldScale,
                         StartWorldPos,
                         StartWorldRootDir,
                         &E,
                         0,
                         &A,
                         RD0,
                         EventWorldPos,
                         NULL);

    // Compute Root pos
    CompRelativePt (WorldScale,
                         StartWorldPos,
                         StartWorldRootDir,
                         &R,
                         0,
                         &A,
                         RD0,
                         RootWorldPos,
                         NULL);

    *EventFrame    = EV->Frame;
}




///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////


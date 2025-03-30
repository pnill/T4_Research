///////////////////////////////////////////////////////////////////////////
//
// AM_PLAY.C
//
///////////////////////////////////////////////////////////////////////////

#include "AM_Play.hpp"
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

#define		BLEND_AMOUNT	TweakBlending

///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////

static xbool        AM_SurpressEvents = FALSE;

extern xbool		AM_Inited;

f32 TweakBlending = R_1;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PLAYBACK ROUTINES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

inline f32 LPR( f32 A, f32 B )
{
    A = (((s32)(A*65536.0f)) % ((s32)(B*65536.0f)))/65536.0f;
    if( A < 0 ) A += B;
    return A;
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetAnimation   (s32                AnimID,
                                     xbool              MirrorIsOn)
{
    f32      TotalTransLen;
    radian   R;
    s32      i,NStreams;

    ASSERT(AM_Inited==1);
    ASSERT(m_AnimGroup);

    // Set mirroring for this animation
	if( MirrorIsOn )
		m_Flags |= ANIM_FLAGS_MIRROR;
	else
		m_Flags &= ~ANIM_FLAGS_MIRROR;

	//---	Blend for the new animation
	m_Flags |= ANIM_FLAGS_BLENDING;

    // Free old animation
    if (m_AnimInfo != NULL)
    {
        ASSERT(m_BlockData != NULL);
        AMCACHE_UnlockBlock(this);
    }
    m_BlockData				= NULL;
    m_AnimInfo				= NULL;
    m_BlockID				= -1;
    m_BlockFrame0			= 1;
    m_BlockFrameN			= -1;
    m_CurrentCycle			= 0;
    m_CurrentFrame			= 0;
	m_CurrentExportFrame	= 0;
    if (AnimID == -1) return;

    // Setup animation
    ASSERT((AnimID>=0) && (AnimID<m_AnimGroup->NAnimations));
    m_AnimInfo				= &m_AnimGroup->AnimInfo[AnimID];
	m_FrameToExportFrame	= (f32)(m_AnimInfo->ExportNFrames-1) / (f32)(m_AnimInfo->NFrames-1);
	m_ExportFrameToFrame	= 1.0f/m_FrameToExportFrame;
	m_AngleToRadians		= R_360/(f32)(1L<<m_AnimInfo->ExportBits);

	// load in first block
    m_BlockID				= 0;
    AMCACHE_LockBlock (this);

    // Build computed values
    m_RootDirAtFrame0    = m_AnimInfo->RootDirAtFrame0 * m_AngleToRadians;
    m_RootDirAtFrameN    = m_AnimInfo->RootDirAtFrameN * m_AngleToRadians;
    m_MoveDirAtFrame0    = m_AnimInfo->MoveDirAtFrame0 * m_AngleToRadians;
    m_MoveDirAtFrameN    = m_AnimInfo->MoveDirAtFrameN * m_AngleToRadians;
    m_TransAtFrame0.X    = m_AnimInfo->TransXAtFrame0  * VALUE_TO_INCHES;
    m_TransAtFrame0.Y    = m_AnimInfo->TransYAtFrame0  * VALUE_TO_INCHES;
    m_TransAtFrame0.Z    = m_AnimInfo->TransZAtFrame0  * VALUE_TO_INCHES;
    m_TransAtFrameN.X    = m_AnimInfo->TransXAtFrameN  * VALUE_TO_INCHES;
    m_TransAtFrameN.Y    = m_AnimInfo->TransYAtFrameN  * VALUE_TO_INCHES;
    m_TransAtFrameN.Z    = m_AnimInfo->TransZAtFrameN  * VALUE_TO_INCHES;

    // Snap RootDirAtFrameN to RootDirAtFrame0 if within 5 degrees
    R = m_RootDirAtFrameN - m_RootDirAtFrame0;
    while (R < -R_180) R += R_360;
    while (R >  R_180) R -= R_360;
    if (ABS(R) < (R_1*5)) m_RootDirAtFrameN = m_RootDirAtFrame0;

    // Snap MoveDirAtFrameN to MoveDirAtFrame0 if within 5 degrees
    R = m_MoveDirAtFrameN - m_MoveDirAtFrame0;
    while (R < -R_180) R += R_360;
    while (R >  R_180) R -= R_360;
    if (ABS(R) < (R_1*5)) m_MoveDirAtFrameN = m_MoveDirAtFrame0;
  
    // Compute TotalTrans
    m_TotalTrans.X  = m_TransAtFrameN.X - m_TransAtFrame0.X;
    m_TotalTrans.Z  = m_TransAtFrameN.Z - m_TransAtFrame0.Z;
    m_TotalTrans.Y  = 0;
    TotalTransLen   = m_TotalTrans.X*m_TotalTrans.X + 
                      m_TotalTrans.Z*m_TotalTrans.Z;

    // Compute total movedir
    if (TotalTransLen < (0.1*0.1))
	{
		m_TotalMoveDir = m_RootDirAtFrame0;
        m_TotalTrans.X = 0;
        m_TotalTrans.Z = 0;
	}
    else
		m_TotalMoveDir = x_atan2(m_TotalTrans.X,m_TotalTrans.Z);


    // Set raw motion values
    m_RawPos            = m_TransAtFrame0;
    m_RawMoveDir        = m_MoveDirAtFrame0;
    m_RawRootDir        = m_RootDirAtFrame0;

    // Compute initial Yaw value

    R = m_RootDirAtFrame0;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        R = -R;

	f32 Yaw = m_WorldRootDir - R;
//	while( Yaw >= R_360 )	Yaw -= R_360;
//	while( Yaw < 0.0f )		Yaw += R_360;
	SetYaw(Yaw);

    // Compute number of streams
    NStreams = 0;
    NStreams += m_AnimGroup->NBones * 3; /* All bone rotations */
    NStreams += 3/* Root: X,Y,Z */ + 2/* FaceDir, MoveDir */;

    // Accumulate prop streams
    for (i=0; i<8; i++)
    {
        m_PropStream[i] = 0;
        if ((u32)m_AnimInfo->PropsPresent & (1<<i))
        {
            m_PropStream[i] = (u8)NStreams;
            NStreams += 6;
        }
    }

    // Accumulate point streams
    for (i=0; i<8; i++)
    {
        m_PointStream[i] = 0;
        if ((u32)m_AnimInfo->PointsPresent & (1<<i))
        {
            m_PointStream[i] = (u8)NStreams;
            NStreams += 1;
        }
    }

    // Accumulate user streams
    for (i=0; i<8; i++)
    {
        m_UserStream[i] = 0;
        if ((u32)m_AnimInfo->UserStreamsPresent & (1<<i))
        {
            m_UserStream[i] = (u8)NStreams;
            NStreams += 1;
        }
    }
}


///////////////////////////////////////////////////////////////////////////
//#define ACCURATE_FACE_AND_MOVE_DIR

radian mocap_player::GetRawMotion (	vector3*        Trans,
									radian*         MoveDir,
									radian*         RootDir)
{
    vector3     Trans0;
    vector3     Trans1;
    s16*        DataPtr0;
    s16*        DataPtr1;
    f32         Frac;

    ASSERT((m_CurrentExportFrame>=m_BlockFrame0) && 
           (m_CurrentExportFrame<=m_BlockFrameN));

    // Get ptrs to the frames
    GetFrameDataPtrs(&DataPtr0,&DataPtr1,&Frac);

	DataPtr0 += (m_AnimGroup->NBones*3);
    DataPtr1 += (m_AnimGroup->NBones*3);

    // Pull translation and angles from streams and interpolate.
    Trans1.X        = (f32)(DataPtr1[0]) * VALUE_TO_INCHES;
    Trans1.Y        = (f32)(DataPtr1[1]) * VALUE_TO_INCHES;
    Trans1.Z        = (f32)(DataPtr1[2]) * VALUE_TO_INCHES;
    Trans0.X        = (f32)(DataPtr0[0]) * VALUE_TO_INCHES;
    Trans0.Y        = (f32)(DataPtr0[1]) * VALUE_TO_INCHES;
    Trans0.Z        = (f32)(DataPtr0[2]) * VALUE_TO_INCHES;
    //######### FIX ME!!! ###########
    // These following 2 lines are wrong, and must be interpolated properly
    // between the frame data 0 and frame data 1.  The temp fix just uses
    // the closest frame data to the current animation frame.
    //
    //*MoveDir        = (f32)(DataPtr0[3]) * m_AngleToRadians;
    //*RootDir        = (f32)(DataPtr0[4]) * m_AngleToRadians;
    if( Frac < 0.5f )
    {
        *MoveDir = (f32)(DataPtr0[3]) * m_AngleToRadians;
        *RootDir = (f32)(DataPtr0[4]) * m_AngleToRadians;
    }
    else
    {
        *MoveDir = (f32)(DataPtr1[3]) * m_AngleToRadians;
        *RootDir = (f32)(DataPtr1[4]) * m_AngleToRadians;
    }

    // Interpolate motion data
    Trans->X = Trans0.X + Frac*(Trans1.X - Trans0.X);
    Trans->Y = Trans0.Y + Frac*(Trans1.Y - Trans0.Y);
    Trans->Z = Trans0.Z + Frac*(Trans1.Z - Trans0.Z);

	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::QueueEvents (f32            StartFrame,
                     f32            EndFrame)
{
    s32 StartCycle;
    s32 EndCycle;
    s32 NFrames;
    s32 i;
    anim_event* EV;
    s32 EVFrame;

    ASSERT(m_AnimInfo);

    // Fudge start frame to capture 0 frame event
    if ((m_CurrentCycle==0) && (StartFrame==0.0f))
    {
        StartFrame = -0.01f;
    }

    // Clear old events and check if animation has any events
    m_NEventsQueued = 0;
    if( m_AnimInfo->NEvents == 0 ) return;
    if( AM_SurpressEvents ) return;

    // Compute start and end cycles
    NFrames    = m_AnimInfo->NFrames;
    StartCycle = (s32)(StartFrame - LPR(StartFrame,(f32)(NFrames-1))) / NFrames;
    EndCycle   = (s32)(EndFrame   - LPR(EndFrame  ,(f32)(NFrames-1))) / NFrames;
    EV         = &m_AnimGroup->AnimEvent[ m_AnimInfo->EventIndex ];

    if (EndFrame > StartFrame)
    {
        while (StartCycle <= EndCycle)
        {
            for (i=0; i<m_AnimInfo->NEvents; i++)
            {
                // Check if event is in range
                EVFrame = EV[i].Frame + (NFrames*StartCycle);
                if ((EVFrame > StartFrame) && (EVFrame <= EndFrame))
                {
                    ASSERT(m_NEventsQueued < ANIM_MAX_EVENTS);
					if( m_NEventsQueued >= ANIM_MAX_EVENTS )
						break;

                    m_EventQueue[m_NEventsQueued] = (u8)i;
                    m_NEventsQueued++;
                }
            }

            StartCycle++;
        }
    }
    else
    {
        while (StartCycle >= EndCycle)
        {
            for (i=m_AnimInfo->NEvents-1; i>=0; i--)
            {
                // Check if event is in range
                EVFrame = EV[i].Frame + (NFrames*StartCycle);
                if ((EVFrame >= EndFrame) && (EVFrame < StartFrame))
                {
                    ASSERT(m_NEventsQueued < ANIM_MAX_EVENTS);
					if( m_NEventsQueued >= ANIM_MAX_EVENTS )
						break;

                    m_EventQueue[m_NEventsQueued] = (u8)i;
                    m_NEventsQueued++;
                }
            }
            StartCycle--;
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void mocap_player::AdvNFrames( f32              NFrames,
                               vector3&         NewWorldPos,
                               radian&          NewWorldRootDir,
                               radian&          NewWorldMoveDir,
                               radian&          NewWorldTotalMoveDir)
{
    f32         NumFramesInAnim;
    s32         CyclesCrossed;
    f32         ModulatedFrame;
    f32         dX, dZ;
    f32         C, S;
    radian      RD;
    radian      NewRawMoveDir;
    radian      NewRawRootDir;
    vector3     NewRawTrans;
    radian      NewRootDir;
    vector3     NewTrans;
	radian		NewFrameYaw;

    ASSERT((NFrames<1000.0f) && (NFrames>-1000.0f));

    // Clear event queue
    m_NEventsQueued = 0;

    // If frame increment is zero, nothing to do
    if (NFrames == 0.0f) 
    {
        NewWorldPos          = m_WorldPos;
        NewWorldPos.Y        = m_RawPos.Y * m_WorldScale;
        NewWorldRootDir      = m_WorldRootDir;
        NewWorldMoveDir      = m_WorldMoveDir;
        NewWorldTotalMoveDir = m_WorldTotalMoveDir;
        return;
    }

    // Queue events into event stack.
    QueueEvents(m_CurrentFrame,m_CurrentFrame + NFrames);

    // Compute new CurrentFrame and CurrentCycle
    NumFramesInAnim = (f32)(m_AnimInfo->NFrames-1);
    ModulatedFrame  = m_CurrentFrame + NFrames;
    CyclesCrossed   = 0;
    if (NFrames > 0)
    {
        while (ModulatedFrame > NumFramesInAnim) 
        {
            ModulatedFrame -= NumFramesInAnim;
            CyclesCrossed++;
        }
    }
    else
    {
        while (ModulatedFrame < 0) 
        {
            ModulatedFrame += NumFramesInAnim;
            CyclesCrossed--;
        }
    }
    m_CurrentCycle	+= CyclesCrossed;
    m_CurrentFrame	= ModulatedFrame;

    ASSERT(m_CurrentFrame >=0 && m_CurrentFrame<=NumFramesInAnim);

	//---	set the export frame based on the current frame
	m_CurrentExportFrame = m_CurrentFrame*m_FrameToExportFrame;
	if( m_CurrentExportFrame >= (f32)(m_AnimInfo->ExportNFrames-1) )
		m_CurrentExportFrame = (f32)(m_AnimInfo->ExportNFrames-1); // keep it off the very last frame
    
    // Be sure correct anim block is loaded
    if ((m_CurrentExportFrame < m_BlockFrame0) || 
        (m_CurrentExportFrame > m_BlockFrameN))
    {
        AMCACHE_UnlockBlock(this);
        AMCACHE_LockBlock(this);
    }

    // Get motion data for this frame in cycle0
    NewFrameYaw = GetRawMotion( &NewRawTrans,
								&NewRawMoveDir,
								&NewRawRootDir);
    NewTrans    = NewRawTrans;
    NewRootDir  = NewRawRootDir;

    // Check if we can do cycle shortcut
    if ((m_RootDirAtFrameN==m_RootDirAtFrame0) &&
        (CyclesCrossed != 0))
    {
        NewTrans.X += m_TotalTrans.X * (f32)CyclesCrossed;
        NewTrans.Z += m_TotalTrans.Z * (f32)CyclesCrossed;
    }
    else
    if (CyclesCrossed > 0)
    {
        RD = m_RootDirAtFrameN - m_RootDirAtFrame0;
        x_fastsincos( RD, S, C );
        
        if (m_Flags & ANIM_FLAGS_MIRROR) 
            RD = -RD;

        while (CyclesCrossed > 0)
        {
            f32 X = NewTrans.X;
            f32 Z = NewTrans.Z;
            NewTrans.X  = (C*X + S*Z) + m_TotalTrans.X;
            NewTrans.Z  = (C*Z - S*X) + m_TotalTrans.Z;
            NewRootDir += RD;
            CyclesCrossed--;
        }
    }
    else
    if (CyclesCrossed < 0)
    {
        RD = -(m_RootDirAtFrameN - m_RootDirAtFrame0);
		x_fastsincos( RD, S, C );

        if (m_Flags & ANIM_FLAGS_MIRROR) 
            RD = -RD;

        while (CyclesCrossed < 0)
        {
            f32 X = NewTrans.X;
            f32 Z = NewTrans.Z;
            NewTrans.X  = (C*X + S*Z) - m_TotalTrans.X;
            NewTrans.Z  = (C*Z - S*X) - m_TotalTrans.Z;
            NewRootDir += RD;
            CyclesCrossed++;
        }
    }

    // Calculate the horizontal translation in animation space and mirror
    // if needed.
    dX = NewTrans.X - m_RawPos.X;
    dZ = NewTrans.Z - m_RawPos.Z;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        dX = -dX;

    // Rotate the Raw ZX deltas by the current Yaw
	x_fastsincos( m_WorldYaw, S, C );
    NewWorldPos.Z = dZ*C - dX*S;
    NewWorldPos.X = dX*C + dZ*S;
    NewWorldPos.Y = NewRawTrans.Y;

    // Scale deltas 
    NewWorldPos.X *= m_WorldScale;
    NewWorldPos.Y *= m_WorldScale;
    NewWorldPos.Z *= m_WorldScale;

    // Add current position on
    NewWorldPos.X += m_WorldPos.X;
    NewWorldPos.Z += m_WorldPos.Z;

    // Compute directions
    RD	= (NewRootDir - m_RawRootDir);
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        RD = -RD;
    NewWorldRootDir      = m_WorldRootDir + RD;
    NewWorldMoveDir      = m_WorldMoveDir + RD;
    NewWorldTotalMoveDir = m_WorldTotalMoveDir + RD;

    // Set new raw data for next advance
    m_RawPos     = NewRawTrans;
    m_RawMoveDir = NewRawMoveDir;
    m_RawRootDir = NewRawRootDir;

	//---	set the amount of blending #### NOTE: this should be cumulative and then cleared after blending is applied.
	//		however, this will have to wait until we get all bone rotations handled in one place.
	m_MaxBlend = BLEND_AMOUNT*NFrames;
}

///////////////////////////////////////////////////////////////////////////

void   mocap_player::JumpToFrame ( f32                DestFrame )
{
    f32         NumFramesInAnim;
    s32         CyclesCrossed;
    f32         ModulatedFrame;
    radian      NewRawMoveDir;
    radian      NewRawRootDir;
    vector3     NewRawTrans;
    f32         NFrames;
    f32         OldWorldRootDir;
	radian		NewFrameYaw;

    // Record current world root dir for later use
    OldWorldRootDir = m_WorldRootDir;

    // Clear event queue
    m_NEventsQueued = 0;
    
    // Compute number of frames to advance
    NFrames = DestFrame - m_CurrentFrame;

    // If frame increment is zero, nothing to do
    if (NFrames == 0.0f) 
        return;

    // Compute new CurrentFrame and CurrentCycle
    NumFramesInAnim = (f32)(m_AnimInfo->NFrames-1);
    ModulatedFrame  = m_CurrentFrame + NFrames;
    CyclesCrossed   = 0;
    if (NFrames > 0)
    {
        while (ModulatedFrame > NumFramesInAnim) 
        {
            ModulatedFrame -= NumFramesInAnim;
            CyclesCrossed++;
        }
    }
    else
    {
        while (ModulatedFrame < 0) 
        {
            ModulatedFrame += NumFramesInAnim;
            CyclesCrossed--;
        }
    }
    m_CurrentCycle += CyclesCrossed;
    m_CurrentFrame  = ModulatedFrame;
    ASSERT(m_CurrentFrame >=0 && m_CurrentFrame<=NumFramesInAnim);

	//---	set the export frame based on the current frame
	m_CurrentExportFrame = m_CurrentFrame*m_FrameToExportFrame;
	if( m_CurrentExportFrame >= (f32)(m_AnimInfo->ExportNFrames-1) )
		m_CurrentExportFrame = (f32)(m_AnimInfo->ExportNFrames-1); // keep it off the very last frame
    
    // Be sure correct anim block is loaded
    if ((m_CurrentExportFrame < m_BlockFrame0) || 
        (m_CurrentExportFrame > m_BlockFrameN))
    {
        AMCACHE_UnlockBlock(this);
        AMCACHE_LockBlock(this);
    }

    // Get motion data for this frame in cycle0
    NewFrameYaw = GetRawMotion( &NewRawTrans,
								&NewRawMoveDir,
								&NewRawRootDir);
/*
    // Check if we can do cycle shortcut
    if ((m_RootDirAtFrameN==m_RootDirAtFrame0) &&
        (CyclesCrossed != 0))
    {
        NewRawTrans.X += m_TotalTrans.X * (f32)CyclesCrossed;
        NewRawTrans.Z += m_TotalTrans.Z * (f32)CyclesCrossed;
    }
    else
    if (CyclesCrossed > 0)
    {
        RD = m_RootDirAtFrameN - m_RootDirAtFrame0;

        S  = x_fastsin(RD);
        C  = x_fastcos(RD);
        
        while (CyclesCrossed > 0)
        {
            f32 X = NewRawTrans.X;
            f32 Z = NewRawTrans.Z;
            NewRawTrans.X  = (C*X + S*Z) + m_TotalTrans.X;
            NewRawTrans.Z  = (C*Z - S*X) + m_TotalTrans.Z;
            CyclesCrossed--;
        }
    }
    else
    if (CyclesCrossed < 0)
    {
        RD = -(m_RootDirAtFrameN - m_RootDirAtFrame0);
        S  = x_fastsin(RD);
        C  = x_fastcos(RD);

        while (CyclesCrossed < 0)
        {
            f32 X = NewRawTrans.X;
            f32 Z = NewRawTrans.Z;
            NewRawTrans.X  = (C*X + S*Z) - m_TotalTrans.X;
            NewRawTrans.Z  = (C*Z - S*X) - m_TotalTrans.Z;
            CyclesCrossed++;
        }
    }
*/
    // Set new raw data for next advance
    m_RawPos   = NewRawTrans;
    m_RawMoveDir = NewRawMoveDir;
    m_RawRootDir = NewRawRootDir;

    // Compute yaw and other dirs to keep rootdir aligned
    SetWorldRootDir( OldWorldRootDir );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PROJECT FUNCTIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void    CompRelativePt         (f32            Scale,
                                     vector3*       StartWorldPos,
                                     radian         StartWorldRootDir,
                                     vector3*       AnimTargetPos,
                                     radian         AnimTargetRootDir,
                                     vector3*       AnimBasePos,
                                     radian         AnimBaseRootDir,
                                     vector3*       FinalWorldPos,
                                     radian*        FinalWorldRootDir)
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
    S   = x_fastsin(R);
    C   = x_fastcos(R);
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
	AngleToRad = R_360/(1L<<Anim->ExportBits);

    // Build end of animation position
    S.X = Anim->TransXAtFrame0 * VALUE_TO_INCHES;
    S.Y = Anim->TransYAtFrame0 * VALUE_TO_INCHES;
    S.Z = Anim->TransZAtFrame0 * VALUE_TO_INCHES;
    E.X = Anim->TransXAtFrameN * VALUE_TO_INCHES;
    E.Y = Anim->TransYAtFrameN * VALUE_TO_INCHES;
    E.Z = Anim->TransZAtFrameN * VALUE_TO_INCHES;
    RD0 = Anim->RootDirAtFrame0*AngleToRad;
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

	AngleToRad = R_360/(1L<<Anim->ExportBits);

    // Build initial event pos
    E.X = EV->PX * VALUE_TO_INCHES;
    E.Y = EV->PY * VALUE_TO_INCHES;
    E.Z = EV->PZ * VALUE_TO_INCHES;
    R.X = EV->RPX * VALUE_TO_INCHES;
    R.Y = EV->RPY * VALUE_TO_INCHES;
    R.Z = EV->RPZ * VALUE_TO_INCHES;
    A.X = Anim->TransXAtFrame0 * VALUE_TO_INCHES;
    A.Y = Anim->TransYAtFrame0 * VALUE_TO_INCHES;
    A.Z = Anim->TransZAtFrame0 * VALUE_TO_INCHES;
    RD0 = Anim->RootDirAtFrame0*AngleToRad;

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

xbool   mocap_player::TimeTillEvent  (s16                EventID,
                                     f32&               Frame)
{
    anim_event* Event;
    s32         i;
    f32         NumFrames;
    s32         PreFrameEvent;  // Earliest event before RelFrame
    s32         PostFrameEvent; // Earliest event after RelFrame

    ASSERT(m_AnimInfo);
    ASSERT(m_AnimGroup);

    NumFrames   = (f32)(m_AnimInfo->NFrames-1);
    Event       = &m_AnimGroup->AnimEvent[ m_AnimInfo->EventIndex ];

    // Assign pre and post events
    PreFrameEvent  = -1;
    PostFrameEvent = -1;

    // Assume playing animation forward
    for (i=0; i<m_AnimInfo->NEvents; i++)
    {
        // Be sure event matches requested event id
        if (Event[i].EventID == EventID)
        {
            // Check for earliest event < RelFrame
            if ((PreFrameEvent==-1) && ((f32)(Event[i].Frame) < m_CurrentFrame))
                PreFrameEvent = i;

            // Check for earliest event >= RelFrame
            if ((PostFrameEvent==-1) && ((f32)(Event[i].Frame) >= m_CurrentFrame))
            {
                PostFrameEvent = i;
                break;
            }
        }
    }

    // Check if PostFrame event is available
    if (PostFrameEvent != -1)
    {
        (Frame) = (f32)(Event[PostFrameEvent].Frame) + (m_CurrentCycle*NumFrames);
        (Frame) -= (m_CurrentFrame + (m_CurrentCycle*NumFrames));
        return TRUE;
    }

    // Check if PreFrame event is available
    if (PreFrameEvent != -1)
    {
        (Frame) = (f32)(Event[PreFrameEvent].Frame) + ((m_CurrentCycle+1)*NumFrames);
        (Frame) -= (m_CurrentFrame + (m_CurrentCycle*NumFrames));
        return TRUE;
    }

    // No event found
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////

void mocap_player::GetFrameDataPtrs( s16**        DataPtr0,
                                     s16**        DataPtr1,
                                     f32*         Frac)
{
    f32 F;
    s32 FInt;

    ASSERT((m_CurrentExportFrame>=m_BlockFrame0) && 
           (m_CurrentExportFrame<=m_BlockFrameN));

    // Compute where in data the frames begin
//#ifdef FRAMES_HALVED
//    F       = (m_CurrentFrame - (f32)(m_BlockID*ANIM_FRAMES_PER_BLOCK)) * 0.5f;
//#elif defined( FRAMES_FRACTIONAL )
//    F       = (m_CurrentFrame - (f32)(m_BlockID*ANIM_FRAMES_PER_BLOCK)) * m_FrameToExportFrame;
//#else
//    F       = (m_CurrentFrame - (f32)(m_BlockID*ANIM_FRAMES_PER_BLOCK));
//#endif
//	F = ((m_CurrentFrame-m_BlockFrame0) * m_FrameToExportFrame);
	F = m_CurrentExportFrame - m_BlockFrame0;
	if( F < 0.0f )
		F = 0.0f;
	else if( F > (f32)m_BlockFrameN )
		F = (f32)m_BlockFrameN;

    FInt    = (s32)F;

    // Check if we are exactly on last frame
    if (((f32)FInt == F) && (FInt!=0))
    {
        FInt--;
        *Frac   = 1.0f;
    }
    else
    {   
        *Frac = F - (f32)FInt;
    }

    // Compute ptrs and fraction
//#ifdef FRAMES_HALVED
//    ASSERT((FInt>=0) && (FInt<=(ANIM_FRAMES_PER_BLOCK*0.5f)));
//#elif defined( FRAMES_FRACTIONAL )
//    ASSERT((FInt>=0) && (FInt<=(ANIM_FRAMES_PER_BLOCK*m_FrameToExportFrame)));
//#else
//    ASSERT((FInt>=0) && (FInt<=ANIM_FRAMES_PER_BLOCK));
//#endif

    *DataPtr0  = m_BlockData + (FInt * ((s32)m_AnimInfo->NStreams));
    *DataPtr1  = (*DataPtr0) + (s32)(m_AnimInfo->NStreams);
}

///////////////////////////////////////////////////////////////////////////

radian mocap_player::GetRotation(	s32			Bone,
									radian3*	Dest)
{
    f32		Frac;
    radian3	Rot0;
    radian3	Rot1;
    s16*	DataPtr0;
    s16*	DataPtr1;
    radian	DR;

    ASSERT(AM_Inited);

    // Check if animation has correct data decompressed
    ASSERT((m_CurrentExportFrame>=m_BlockFrame0) && 
           (m_CurrentExportFrame<=m_BlockFrameN));

    // Get ptrs to the frames
    GetFrameDataPtrs(&DataPtr0,&DataPtr1,&Frac);

    // Decide whether to do inner anim blending
    if (m_Flags & ANIM_FLAGS_INTERPOLATE)
    {
		DataPtr0 += 3*Bone;
		DataPtr1 += 3*Bone;

        Rot0.Pitch = (f32)(DataPtr0[0]) * m_AngleToRadians;
        Rot0.Yaw   = (f32)(DataPtr0[1]) * m_AngleToRadians;
        Rot0.Roll  = (f32)(DataPtr0[2]) * m_AngleToRadians;
        Rot1.Pitch = (f32)(DataPtr1[0]) * m_AngleToRadians;
        Rot1.Yaw   = (f32)(DataPtr1[1]) * m_AngleToRadians;
        Rot1.Roll  = (f32)(DataPtr1[2]) * m_AngleToRadians;

        // Blend rotations
        DR = Rot1.Pitch - Rot0.Pitch;
        while (DR < -R_180) DR += R_360;
        while (DR >  R_180) DR -= R_360;
        Dest->Pitch = Rot0.Pitch + Frac*DR;

        DR = Rot1.Yaw - Rot0.Yaw;
        while (DR < -R_180) DR += R_360;
        while (DR >  R_180) DR -= R_360;
        Dest->Yaw = Rot0.Yaw + Frac*DR;

        DR = Rot1.Roll - Rot0.Roll;
        while (DR < -R_180) DR += R_360;
        while (DR >  R_180) DR -= R_360;
        Dest->Roll = Rot0.Roll + Frac*DR;
    }
    else
    {
		DataPtr0 += 3*Bone;

        Dest->Pitch = (f32)(DataPtr0[0]) * m_AngleToRadians;
        Dest->Yaw   = (f32)(DataPtr0[1]) * m_AngleToRadians;
        Dest->Roll  = (f32)(DataPtr0[2]) * m_AngleToRadians;
    }

	return 0;
}

///////////////////////////////////////////////////////////////////////////

radian    mocap_player::GetRotations           (radian3*           Dest)
{
    f32      Frac;
    s32      i;
    radian3  Rot0;
    radian3  Rot1;
    s16*     DataPtr0;
    s16*     DataPtr1;
    radian   DR;

    ASSERT(AM_Inited);

    // Check if animation has correct data decompressed
    ASSERT((m_CurrentExportFrame>=m_BlockFrame0) && 
           (m_CurrentExportFrame<=m_BlockFrameN));

    // Get ptrs to the frames
    GetFrameDataPtrs(&DataPtr0,&DataPtr1,&Frac);

    // Decide whether to do inner anim blending
    if (m_Flags & ANIM_FLAGS_INTERPOLATE)
    {
        for (i=0; i<m_AnimGroup->NBones; i++)
        {
            // Grab rotation data on both sides of frame
            Rot0.Pitch = (f32)(DataPtr0[0]) * m_AngleToRadians;
            Rot0.Yaw   = (f32)(DataPtr0[1]) * m_AngleToRadians;
            Rot0.Roll  = (f32)(DataPtr0[2]) * m_AngleToRadians;
            Rot1.Pitch = (f32)(DataPtr1[0]) * m_AngleToRadians;
            Rot1.Yaw   = (f32)(DataPtr1[1]) * m_AngleToRadians;
            Rot1.Roll  = (f32)(DataPtr1[2]) * m_AngleToRadians;
            DataPtr0 += 3;
            DataPtr1 += 3;

            // Blend rotations
            DR = Rot1.Pitch - Rot0.Pitch;
            while (DR < -R_180) DR += R_360;
            while (DR >  R_180) DR -= R_360;
            Dest[i].Pitch = Rot0.Pitch + Frac*DR;

            DR = Rot1.Yaw - Rot0.Yaw;
            while (DR < -R_180) DR += R_360;
            while (DR >  R_180) DR -= R_360;
            Dest[i].Yaw = Rot0.Yaw + Frac*DR;

            DR = Rot1.Roll - Rot0.Roll;
            while (DR < -R_180) DR += R_360;
            while (DR >  R_180) DR -= R_360;
            Dest[i].Roll = Rot0.Roll + Frac*DR;
        }
    }
    else
    {
        for (i=0; i<m_AnimGroup->NBones; i++)
        {
            Dest[i].Pitch = (f32)(DataPtr0[0]) * m_AngleToRadians;
            Dest[i].Yaw   = (f32)(DataPtr0[1]) * m_AngleToRadians;
            Dest[i].Roll  = (f32)(DataPtr0[2]) * m_AngleToRadians;
            DataPtr0  += 3;
        }
    }

	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::GetBoneMatrix (s32                BoneID,
                                     matrix4&           LocalToWorldM)
{
    radian3    TempRotArray[30];

    ASSERT(m_AnimGroup);
    ASSERT((BoneID>=0) && (BoneID<m_AnimGroup->NBones));
    ASSERT(m_AnimGroup->NBones <= 30);

    // Load rotations into temp buffer
    GetRotations (TempRotArray);

    // Get matrix from skeleton
    AMSKL_GetBoneMatrix (m_AnimGroup,
                         &LocalToWorldM,
                         TempRotArray,
                         (m_Flags & ANIM_FLAGS_MIRROR),
                         BoneID,
                         m_WorldScale,
                         m_WorldYaw,
                         &m_WorldPos);
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::GetAllBoneMatrices     (matrix4*           LocalToWorldM)
{
    radian3    TempRotArray[30];

    ASSERT(m_AnimGroup);
    ASSERT(m_AnimGroup->NBones <= 30);
    ASSERT(LocalToWorldM);

    // Load rotations into temp buffer
    GetRotations (TempRotArray);

    // Get matrix from skeleton
    AMSKL_GetBoneMatrices ( m_AnimGroup,
                            LocalToWorldM,
                            TempRotArray,
                            (m_Flags & ANIM_FLAGS_MIRROR),
                            m_WorldScale,
                            m_WorldYaw,
                            &m_WorldPos );
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::GetBonePosition(s32                BoneID,
                                     vector3&           WorldPos)
{
    matrix4 M;
    GetBoneMatrix(BoneID,M);
    WorldPos = M.GetTranslation();
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::GetBoneOrient	(s32                BoneID,
                                     radian3&           WorldRot)
{
//    radian3    TempRotArray[30];
	radian3		TempRot;

    ASSERT(m_AnimGroup);
    ASSERT((BoneID>=0) && (BoneID<m_AnimGroup->NBones));
//    ASSERT(m_AnimGroup->NBones <= 30);

    // Load rotations into temp buffer
//    GetRotations (TempRotArray);

    // Pull out rotations for this bone
//    WorldRot = TempRotArray[BoneID];
    GetRotation( BoneID, &WorldRot );
    WorldRot.Yaw += m_WorldYaw;

    // Convert from XZY to XYZ
    //x_XZYToXYZRot( WorldRot );
}

///////////////////////////////////////////////////////////////////////////
static void PROP_SetZYXRot( matrix4& M, radian3& Rot );
static void PROP_GetYZXRot( matrix4& M, radian3& Rot );

//////////////////////////////////////////////////////////////////////////
xbool   mocap_player::GetPropInfo            (s32                PropIndex,
                                     vector3&           Pos,
                                     radian3&           Rot)
{
    radian3     PropRot;
    vector3     PropPos;
    vector3     RootPos;
    s16*        DataPtr0;
    s16*        DataPtr1;
    f32         Frac;
    s32         PSO,RO;
    radian3     Rot1,Rot0;
    radian      DR;
    matrix4     M;

    ASSERT(m_AnimInfo);
    ASSERT((PropIndex>=0)&&(PropIndex<8));
    ASSERT((m_CurrentExportFrame>=m_BlockFrame0) && 
           (m_CurrentExportFrame<=m_BlockFrameN));

    // Check if prop is present
    if (!(m_AnimInfo->PropsPresent & (1<<PropIndex))) 
        return FALSE;

    // Get ptrs to the frames
    GetFrameDataPtrs(&DataPtr0,&DataPtr1,&Frac);
    PSO = m_PropStream[PropIndex];

    // Check if data is present
    if ((DataPtr0[PSO+0]==0) &&
        (DataPtr0[PSO+1]==0) &&
        (DataPtr0[PSO+2]==0) &&
        (DataPtr0[PSO+3]==0) &&
        (DataPtr0[PSO+4]==0) &&
        (DataPtr0[PSO+5]==0)) return FALSE;

    // Decide whether to do inner anim blending
    if (m_Flags & ANIM_FLAGS_INTERPOLATE)
    {
        RO = (m_AnimGroup->NBones*3);
        RootPos.X = ((f32)DataPtr0[RO+0]  + Frac * (DataPtr1[RO+0] - DataPtr0[RO+0])) * VALUE_TO_INCHES;
        RootPos.Y = ((f32)DataPtr0[RO+1]  + Frac * (DataPtr1[RO+1] - DataPtr0[RO+1])) * VALUE_TO_INCHES;
        RootPos.Z = ((f32)DataPtr0[RO+2]  + Frac * (DataPtr1[RO+2] - DataPtr0[RO+2])) * VALUE_TO_INCHES;
        PropPos.X = ((f32)DataPtr0[PSO+3] + Frac * (DataPtr1[PSO+3] - DataPtr0[PSO+3])) * VALUE_TO_INCHES;
        PropPos.Y = ((f32)DataPtr0[PSO+4] + Frac * (DataPtr1[PSO+4] - DataPtr0[PSO+4])) * VALUE_TO_INCHES;
        PropPos.Z = ((f32)DataPtr0[PSO+5] + Frac * (DataPtr1[PSO+5] - DataPtr0[PSO+5])) * VALUE_TO_INCHES;

        Rot0.Pitch = (f32)(DataPtr0[PSO+0])	* m_AngleToRadians;
        Rot0.Yaw = (f32)(DataPtr0[PSO+1])	* m_AngleToRadians;
        Rot0.Roll = (f32)(DataPtr0[PSO+2])	* m_AngleToRadians;
        Rot1.Pitch = (f32)(DataPtr1[PSO+0])	* m_AngleToRadians;
        Rot1.Yaw = (f32)(DataPtr1[PSO+1])	* m_AngleToRadians;
        Rot1.Roll = (f32)(DataPtr1[PSO+2])	* m_AngleToRadians;

        // Blend rotations
        DR = Rot1.Pitch - Rot0.Pitch;
        while (DR < -R_180) DR += R_360;
        while (DR >  R_180) DR -= R_360;
        PropRot.Pitch = Rot0.Pitch + Frac*DR;

        DR = Rot1.Yaw - Rot0.Yaw;
        while (DR < -R_180) DR += R_360;
        while (DR >  R_180) DR -= R_360;
        PropRot.Yaw = Rot0.Yaw + Frac*DR;

        DR = Rot1.Roll - Rot0.Roll;
        while (DR < -R_180) DR += R_360;
        while (DR >  R_180) DR -= R_360;
        PropRot.Roll = Rot0.Roll + Frac*DR;
    }
    else
    {
        RootPos.X = (f32)(DataPtr0[(m_AnimGroup->NBones*3)+0]) * VALUE_TO_INCHES;
        RootPos.Y = (f32)(DataPtr0[(m_AnimGroup->NBones*3)+1]) * VALUE_TO_INCHES;
        RootPos.Z = (f32)(DataPtr0[(m_AnimGroup->NBones*3)+2]) * VALUE_TO_INCHES;
        PropRot.Pitch = (f32)(DataPtr0[PSO+0])	* m_AngleToRadians;
        PropRot.Yaw = (f32)(DataPtr0[PSO+1])	* m_AngleToRadians;
        PropRot.Roll = (f32)(DataPtr0[PSO+2])	* m_AngleToRadians;
        PropPos.X = (f32)(DataPtr0[PSO+3])	* VALUE_TO_INCHES;
        PropPos.Y = (f32)(DataPtr0[PSO+4])	* VALUE_TO_INCHES;
        PropPos.Z = (f32)(DataPtr0[PSO+5])	* VALUE_TO_INCHES;
    }

    // Make relative to root node
    PropPos = PropPos - RootPos;

    // Scale by player scale
    PropPos = PropPos * m_WorldScale;

    if (m_Flags & ANIM_FLAGS_MIRROR)
    {
        PropPos.X = -PropPos.X;
        PropRot.Yaw = -PropRot.Yaw;
        PropRot.Roll = -PropRot.Roll;
    }

    // Rotate into world by base rotation
    PropPos.RotateY(m_WorldYaw);

    // Translate world by player root
    PropPos += m_WorldPos;

    ////////////////////////////////////////////////////
    //The prop rotation is given so that you multiply the
    //  rotations (from Left to Right) like so:
    //   RZ * RY * RX
    //But we also need to apply the World Yaw of the mocap
    //  to the prop, so to make this easier we get the rotations
    //  in RY * RZ * RX order (Left to Right) and simply add the
    //  mocap World Yaw to the result rotation values.
    //This way the props are done similar to the method used
    //  in the mocap's skeleton bones.
    ////////////////////////////////////////////////////
    PROP_SetZYXRot( M, PropRot );
    PROP_GetYZXRot( M, PropRot );
    PropRot.Yaw += m_WorldYaw;
    ////////////////////////////////////////////////////

    // Return values
    Pos = PropPos;
    Rot = PropRot;
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
static void PROP_SetZYXRot( matrix4& M, radian3& Rot )
{
    //This function only sets the 3x3 rotation
    //Order of mult. (from Left to Right) is:
    // RZ * RY * RX
    
    f32 sx, sy, sz;
    f32 cx, cy, cz;

    x_fastsincos( Rot.Pitch, sx, cx );
    x_fastsincos( Rot.Yaw, sy, cy );
    x_fastsincos( Rot.Roll, sz, cz );

    f32 cxcz = (cx * cz),  sxcz = (sx * cz);
    f32 cxsz = (cx * sz),  sxsz = (sx * sz);

    M.M[0][0] = (cy * cz);
    M.M[0][1] = (cy * sz);
    M.M[0][2] = -sy;
    M.M[1][0] = (sxcz * sy) - cxsz;
    M.M[1][1] = (sxsz * sy) + cxcz;
    M.M[1][2] = (sx * cy);
    M.M[2][0] = (cxcz * sy) + sxsz;
    M.M[2][1] = (cxsz * sy) - sxcz;
    M.M[2][2] = (cx * cy);
}

//========================================================================
static void PROP_GetYZXRot( matrix4& M, radian3& Rot )
{
    //This function gets the rotation values from the matrix,
    //as if the matrix was created by multiplying in this order:
    // (Left to Right)->  RY * RZ * RX
    //If there is any scaling in the matrix, this function probably won't
    //work correctly due to some optimizations

    radian rAlpha, rBeta, rEnigma;
    f32    xzLength, xyLength;
    f32    cosAlpha, sinAlpha;

    const vector3& XA = *(vector3*)M.M[0]; //xAxis;
    const vector3& YA = *(vector3*)M.M[1]; //yAxis;

    //Project XAxis onto XZ plane to find inverse of Y rot(-Yaw)
    // angle Alpha is -Yaw
    rAlpha   = x_atan2( XA.Z, XA.X );
    cosAlpha = x_fastcos( rAlpha );

    //xzLength is the length of the XAxis proj. on the XZ plane
    if( XA.X == 0.0f )
    {   xzLength = XA.Z;
        sinAlpha = 1.0f;
    }
    else
    {   xzLength = XA.X / cosAlpha;
        sinAlpha = XA.Z / xzLength;
    }

    // angle Beta is the Z rotation (Roll)
    rBeta = x_atan2( XA.Y, xzLength );

    //Need to undo the Y rotation to find the proper X rotation(pitch)
    //YAX and YAZ are the modified YAxis X and Z components after
    // rotating it by Alpha, or -Yaw (the Y component doesn't change)
    f32 YAX =  cosAlpha * YA.X + sinAlpha * YA.Z;
    f32 YAZ = -sinAlpha * YA.X + cosAlpha * YA.Z;

    //xyLength is the length of the modified YAxis projected
    // onto the XY plane
    if( YA.Y == 0.0f )
        xyLength = YAX;
    else
        xyLength = YA.Y / xzLength; //x_fastcos( rBeta );

    // angle Enigma is the X rotation (Pitch)
    rEnigma = x_atan2( YAZ, xyLength );

    //that's it, just negate Alpha to get the correct Yaw
    Rot.Yaw   = -rAlpha;
    Rot.Roll  = rBeta;
    Rot.Pitch = rEnigma;
}


///////////////////////////////////////////////////////////////////////////
/*
void    mocap_player::GetPointTrackInfo      (s32                PointIndex,
                                     vector3&           WorldPos)
{
    vector3     P0;
    radian3     RR;
    s32         PSO;   //PointStreamIndex
    s16*        DataPtr0;
    s16*        DataPtr1;
    f32         Frac;
    u16         RawPt0;
    s32         X0,Y0,Z0;
    xbool       UpBit;

    ASSERT(Anim);
    ASSERT(m_AnimInfo);
    ASSERT((PointIndex>=0)&&(PointIndex<8));
    ASSERT(m_AnimInfo->PointsPresent & (1<<PointIndex));
    ASSERT(((s32)m_CurrentExportFrame>=m_BlockFrame0) && 
           ((s32)m_CurrentExportFrame<=m_BlockFrameN));

    GetFrameDataPtrs(Anim,&DataPtr0,&DataPtr1,&Frac);
    PSO = m_PointStream[PointIndex];

    // Read s16 from data stream
    RawPt0 = (u16)DataPtr0[PSO];

    // Break RawPt into X,Y,Z components
    UpBit = (RawPt0>>15)&0x01;
    X0 = ((RawPt0>>10) & 0x1F);
    Y0 = ((RawPt0>> 5) & 0x1F);
    Z0 = ((RawPt0>> 0) & 0x1F);

    // Map from range (0-31) into (-40<->40)
    P0.X = (((f32)X0 * (80.0f/31.0f)) + -40.0f);
    P0.Y = (((f32)Y0 * (80.0f/31.0f)) + -40.0f);
    P0.Z = (((f32)Z0 * (80.0f/31.0f)) + -40.0f);

    // Use UpBit
    if (UpBit) P0.Z += 20.0f;
    else       P0.Z -= 20.0f;

    // Get Rotations of the root node at this frame
    RR.Pitch = (f32)(DataPtr0[0]) * m_AngleToRadians;
    RR.Yaw   = (f32)(DataPtr0[1]) * m_AngleToRadians;
    RR.Roll  = (f32)(DataPtr0[2]) * m_AngleToRadians;

    // Mirror position
    if (m_Flags & ANIM_FLAGS_MIRROR)
    {
        P0.X = -P0.X;
    }

    // Scale based on player size
    P0 *= m_WorldScale;
    //V3_Scale(&P0,&P0,m_WorldScale);

    // Rotate into world
    P0.RotateY( m_WorldYaw );
    //V3_RotateY(&P0,&P0,m_WorldYaw);

    // Translate to world position
    P0 += m_WorldPos;
    //V3_Add(&P0,&P0,&m_WorldPos);

    // Return position
    *WorldPos = P0;
}
*/
///////////////////////////////////////////////////////////////////////////

s32     mocap_player::GetNEventsQueued       (void)
{
    return m_NEventsQueued;
}

///////////////////////////////////////////////////////////////////////////

s32     mocap_player::GetEventID             (s32                Index)
{
    anim_event* EV;
    ASSERT((Index>=0) && (Index<m_NEventsQueued));
    EV = &m_AnimGroup->AnimEvent[ m_EventQueue[Index] + m_AnimInfo->EventIndex];
    return EV->EventID;
}

///////////////////////////////////////////////////////////////////////////

f32     mocap_player::GetEventFrame          (s32                Index)
{
    anim_event* EV;
    ASSERT((Index>=0) && (Index<m_NEventsQueued));
    EV = &m_AnimGroup->AnimEvent[ m_EventQueue[Index] + m_AnimInfo->EventIndex];
    return EV->Frame;
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::GetEventPos            (s32                Index,
                                     vector3&           EventPos)
{
    anim_event* EV;
    vector3     P;
    vector3     R;

    ASSERT((Index>=0) && (Index<m_NEventsQueued));

    EV = &m_AnimGroup->AnimEvent[ m_EventQueue[Index] + m_AnimInfo->EventIndex ];
    P.X = EV->PX * VALUE_TO_INCHES;
    P.Y = EV->PY * VALUE_TO_INCHES;
    P.Z = EV->PZ * VALUE_TO_INCHES;
    R.X = EV->RPX * VALUE_TO_INCHES;
    R.Y = EV->RPY * VALUE_TO_INCHES;
    R.Z = EV->RPZ * VALUE_TO_INCHES;

    // Make pos relative to root node
    P -= R;
    //V3_Sub(&P,&P,&R);

    // Mirror event pos
    if (m_Flags & ANIM_FLAGS_MIRROR)
    {
        P.X = -P.X;
    }

    // Scale to player scale
    P *= m_WorldScale;
    //V3_Scale(&P,&P,m_WorldScale);

    // Rotate Y by base rotation
    P.RotateY( m_WorldYaw );
    //V3_RotateY(&P,&P,m_WorldYaw);

    // Translate out to player's position
    P += m_WorldPos;
    //V3_Add(&P,&P,&m_WorldPos);

    EventPos = P;
}

///////////////////////////////////////////////////////////////////////////
/*
void mocap_player::SetEventSuppress( xbool On )
{
    AM_SurpressEvents = On;
}

xbool mocap_player::GetEventSuppress( void )
{
    return( AM_SurpressEvents );
}
*/
///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////

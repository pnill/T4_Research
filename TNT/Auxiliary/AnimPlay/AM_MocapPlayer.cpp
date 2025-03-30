///////////////////////////////////////////////////////////////////////////
//
// AM_MocapPlayer.cpp
//
///////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_plus.hpp"
#include "AM_Play.h"
#include "AM_Cache.h"
#include "AM_MocapPlayer.h"
#include "AM_Skel.h"
#include "AM_Stats.h"


extern xbool AM_Inited;
inline quaternion BlendQuaternion( const quaternion& Qa, const quaternion& Qb, f32 T )
{
	return BlendSpherical( Qa, Qb, T );
//	return BlendLinear( Qa, Qb, T, NORMALIZE_QUAT_ON_BLEND );
}

///////////////////////////////////////////////////////////////////////////
mocap_player::mocap_player( void )
{
	Init( NULL );
}

///////////////////////////////////////////////////////////////////////////
mocap_player::~mocap_player( void )
{
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::Init( anim_group* pAnimGroup )
{
    m_AnimGroup             = pAnimGroup;
    m_bMirror               = FALSE;
    m_bFramesConst          = FALSE;
    m_CurrentFrame          = 0;
    m_CurrentCycle          = 0;
    m_LastFrameInCycle      = 0;
    m_FramesPerCycle        = 0;
    m_CycleTotalTrans.Zero();
    m_CycleTotalMoveDir     = 0.0f;
    m_CycleDeltaRootDir     = 0.0f;
    m_CycleMoveDirAtFrame0  = 0.0f;
    m_CycleMoveDirAtFrameN  = 0.0f;
    m_CycleRawPos.Zero();
    m_CycleRawRootDir       = 0;
    m_CycleRawMoveDir       = 0;
    m_AnimFlags             = 0;
    m_WorldPos.Zero();
    m_WorldMoveDir          = 0;
    m_WorldTotalMoveDir     = 0;
    m_WorldRootDir          = 0;
    m_WorldYaw              = 0;
    m_WorldScale            = 1.0f;
    m_bTransform            = FALSE;
    m_Transform.Identity();
    m_NEventsQueued         = 0;
    x_memset( m_EventQueue, 0, sizeof( u8 ) * ANIM_MAX_EVENTS );
    m_nMMM                  = 0;
    s32 i;
    for (i=0; i<MAX_MOTION_BLEND_COUNT; ++i)
    {
        m_MMM[ i ].pAnimInfo            = NULL;
        m_MMM[ i ].Block.pData          = NULL;
        m_MMM[ i ].Block.Frame0         = 1;
        m_MMM[ i ].Block.FrameN         = -1;
        m_MMM[ i ].CurrentExportFrame   = 0;
        m_MMM[ i ].BlendOrderRatio      = 1.0f;
    }
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::Kill( void )
{
	m_nMMM = 0;
    SetAnimation(-1,FALSE);
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::SetYaw( radian Yaw )
{
    // Move yaw into good range
    Yaw = x_ModAngle(Yaw);
    //while (Yaw >= R_360) Yaw -= R_360;
    //while (Yaw <  0)     Yaw += R_360;
    m_WorldYaw = Yaw;

    // Update Angles
    if( m_bMirror ) 
    {
        m_WorldMoveDir      = Yaw - m_CycleRawMoveDir;		//m_MMM[0].RawMoveDir;
        m_WorldTotalMoveDir = Yaw - m_CycleTotalMoveDir;	//m_MMM[0].TotalMoveDir;
        m_WorldRootDir      = Yaw - m_CycleRawRootDir;		//m_MMM[0].RawRootDir;
    }
    else
    {
        m_WorldMoveDir      = Yaw + m_CycleRawMoveDir;		//m_MMM[0].RawMoveDir;
        m_WorldTotalMoveDir = Yaw + m_CycleTotalMoveDir;	//m_MMM[0].TotalMoveDir;
        m_WorldRootDir      = Yaw + m_CycleRawRootDir;		//m_MMM[0].RawRootDir;
    }
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::SetWorldMoveDir( radian WorldMoveDir )
{
    radian RawMoveDir;
    
    
    WorldMoveDir = x_ModAngle( WorldMoveDir );
    //while (WorldMoveDir >= R_360) WorldMoveDir -= R_360;
    //while (WorldMoveDir <  0)     WorldMoveDir += R_360;

    RawMoveDir = m_CycleRawMoveDir; //m_MMM[0].RawMoveDir;
    if( m_bMirror ) 
        RawMoveDir = -RawMoveDir;

    SetYaw( WorldMoveDir - RawMoveDir);
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::SetWorldFinalMoveDir( radian WorldFinalMoveDir )
{
    radian RawFinalMoveDir;
    
    ASSERT(m_MMM[0].pAnimInfo);
	ASSERTS(m_nMMM==1, "no multi-motion version of this function yet" );
    
    WorldFinalMoveDir = x_ModAngle( WorldFinalMoveDir );
    //while (WorldFinalMoveDir >= R_360) WorldFinalMoveDir -= R_360;
    //while (WorldFinalMoveDir <  0)     WorldFinalMoveDir += R_360;

    RawFinalMoveDir = m_CycleMoveDirAtFrameN; //m_MMM[0].pAnimInfo->MoveDirAtFrameN * m_MMM[0].AngleToRadians;
    if( m_bMirror ) 
        RawFinalMoveDir = -RawFinalMoveDir;

    SetYaw( WorldFinalMoveDir - RawFinalMoveDir);
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::SetWorldTotalMoveDir( radian WorldTotalMoveDir )
{
    radian RawTotalMoveDir;
    
    ASSERT(m_MMM[0].pAnimInfo);
	ASSERTS(m_nMMM==1, "no multi-motion version of this function yet" );
    
    WorldTotalMoveDir = x_ModAngle( WorldTotalMoveDir );
    //while (WorldTotalMoveDir >= R_360) WorldTotalMoveDir -= R_360;
    //while (WorldTotalMoveDir <  0)     WorldTotalMoveDir += R_360;

    RawTotalMoveDir = m_CycleTotalMoveDir; //m_MMM[0].TotalMoveDir;
    if( m_bMirror ) 
        RawTotalMoveDir = -RawTotalMoveDir;

    SetYaw( WorldTotalMoveDir - RawTotalMoveDir );
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::SetWorldRootDir( radian WorldRootDir )
{
    radian RawRootDir;
    
    
    WorldRootDir = x_ModAngle( WorldRootDir );
    //while (WorldRootDir >= R_360) WorldRootDir -= R_360;
    //while (WorldRootDir <  0)     WorldRootDir += R_360;

    RawRootDir = m_CycleRawRootDir; //m_MMM[0].RawRootDir;
    if( m_bMirror ) 
        RawRootDir = -RawRootDir;

    SetYaw( WorldRootDir - RawRootDir );
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::SetWorldFinalRootDir( radian WorldFinalRootDir )
{
    radian RawFinalRootDir;
    
	ASSERT(m_MMM[0].pAnimInfo);
	ASSERTS(m_nMMM==1, "no multi-motion version of this function yet" );
    
    WorldFinalRootDir = x_ModAngle( WorldFinalRootDir );
    //while (WorldFinalRootDir >= R_360) WorldFinalRootDir -= R_360;
    //while (WorldFinalRootDir <  0)     WorldFinalRootDir += R_360;

    RawFinalRootDir = m_CycleDeltaRootDir; //m_MMM[0].pAnimInfo->RootDirAtFrameN * m_MMM[0].AngleToRadians;
    if( m_bMirror ) 
        RawFinalRootDir = -RawFinalRootDir;

    SetYaw( WorldFinalRootDir - RawFinalRootDir);
}

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
inline void mocap_player::SetCycleFrame( s32 Cycle, f32 Frame )
{
    m_CurrentCycle	= Cycle;
    m_CurrentFrame  = Frame;
    ASSERT((m_CurrentFrame>=0) && (m_CurrentFrame<=m_FramesPerCycle));

	//---	if a valid motion has been set, set the export frame data
	if( m_MMM[0].pAnimInfo )
	{
		CalcSetExportFrame();

		// Get motion data for this frame in cycle0
		CalcSetRawMotion();
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	After calling SetMotionPlaybackData, SetCycleFrame() should be called
//	to set the export frames and raw data to the correct location.
void mocap_player::SetMotionPlaybackData( SPlaybackData&	rMotionData,
										  s32				AnimID )
{
	radian	R;
	f32		TotalTransLen;
	s32		StreamsPerRotSet;
	s32		StreamsPerProp;
	s32		NStreams;

    // Free old animation
    if (rMotionData.pAnimInfo != NULL)
    {
        ASSERT(rMotionData.Block.pData != NULL);
        AMCACHE_UnlockBlock( m_AnimGroup, rMotionData.pAnimInfo, rMotionData.Block );
		AMCACHE_ResetBlock( rMotionData.Block );
    }

	//---	if no motion was set, clear the current set motion and exit now
    if (AnimID == -1)
	{
		rMotionData.pAnimInfo	= NULL;
		AMCACHE_ResetBlock( rMotionData.Block );
		return;
	}

    // Setup animation
    ASSERT((AnimID>=0) && (AnimID<m_AnimGroup->NAnimations));
    rMotionData.pAnimInfo			= &m_AnimGroup->AnimInfo[AnimID];
	rMotionData.FrameToExportFrame	= (f32)(rMotionData.pAnimInfo->ExportNFrames-1) / (f32)(rMotionData.pAnimInfo->NFrames-1);
	rMotionData.ExportFrameToFrame	= 1.0f/rMotionData.FrameToExportFrame;
	rMotionData.AngleToRadians		= EXPORTBITS_GETANGLETORADIANS(rMotionData.pAnimInfo->ExportBits);
	rMotionData.ValueToQuat			= EXPORTBITS_GETVALUETOQUATERNION(rMotionData.pAnimInfo->ExportBits);
	rMotionData.ValueToQuatW		= EXPORTBITS_GETVALUETOQUATERNION_W(rMotionData.pAnimInfo->ExportBits);

    // Build computed values
    rMotionData.RootDirAtFrameN	= rMotionData.pAnimInfo->RootDirAtFrameN * rMotionData.AngleToRadians;
    rMotionData.MoveDirAtFrame0	= rMotionData.pAnimInfo->MoveDirAtFrame0 * rMotionData.AngleToRadians;
    rMotionData.MoveDirAtFrameN	= rMotionData.pAnimInfo->MoveDirAtFrameN * rMotionData.AngleToRadians;
    rMotionData.TransYAtFrame0	= rMotionData.pAnimInfo->TransYAtFrame0  * VALUE_TO_INCHES;
	rMotionData.TransYAtFrameN	= rMotionData.pAnimInfo->TransYAtFrameN  * VALUE_TO_INCHES;
    rMotionData.TotalTrans.X	= rMotionData.pAnimInfo->TransXAtFrameN  * VALUE_TO_INCHES;
    rMotionData.TotalTrans.Z	= rMotionData.pAnimInfo->TransZAtFrameN  * VALUE_TO_INCHES;
    rMotionData.TotalTrans.Y	= 0.0f;

    // Snap RootDirAtFrameN to RootDirAtFrame0 if within 5 degrees
    R = rMotionData.RootDirAtFrameN;
    while (R < -R_180) R += R_360;
    while (R >  R_180) R -= R_360;
    if (ABS(R) < (R_1*5.0f)) rMotionData.RootDirAtFrameN = R_0;

    // Snap MoveDirAtFrameN to MoveDirAtFrame0 if within 5 degrees
    R = rMotionData.MoveDirAtFrameN - rMotionData.MoveDirAtFrame0;
    while (R < -R_180) R += R_360;
    while (R >  R_180) R -= R_360;
    if (ABS(R) < (R_1*5.0f)) rMotionData.MoveDirAtFrameN = rMotionData.MoveDirAtFrame0;
  
    // Compute TotalTrans
    TotalTransLen   = rMotionData.TotalTrans.X*rMotionData.TotalTrans.X + 
                      rMotionData.TotalTrans.Z*rMotionData.TotalTrans.Z;

    // Compute total movedir
    if (TotalTransLen < (0.1f*0.1f))
	{
		rMotionData.TotalMoveDir = R_0;
        rMotionData.TotalTrans.X = 0.0f;
        rMotionData.TotalTrans.Z = 0.0f;
	}
    else
		rMotionData.TotalMoveDir = x_atan2( rMotionData.TotalTrans.X, rMotionData.TotalTrans.Z );

    // Compute number of streams
	StreamsPerRotSet	= ( rMotionData.pAnimInfo->Flags&ANIMINFO_FLAG_QUAT_W ) ? 4 : 3;
	StreamsPerProp		= StreamsPerRotSet + 3;

	//---	get the total number of streams in this animation
	NStreams = m_AnimGroup->NBones*StreamsPerRotSet/* All bone rotations */ + 3/* Root: X,Y,Z */ + 2/* FaceDir, MoveDir */;

	// Accumulate prop streams
	u8* pCur	= &rMotionData.PropStream[0];
	u8*	pEnd	= &rMotionData.PropStream[ANIM_MAX_PROPS];
	s32	Flag	= 1;
	while( pCur != pEnd )
	{
		if ((u32)rMotionData.pAnimInfo->PropsStreamsPresent & Flag)
		{
			*pCur++ = (u8)NStreams;
			NStreams += StreamsPerProp;
		}
		else
			*pCur++ = 0;
		Flag <<= 1;
	}

    // Accumulate point streams
	pCur	= &rMotionData.PointStream[0];
	pEnd	= &rMotionData.PointStream[ANIM_MAX_POINTS];
	Flag	= 1;
	while( pCur != pEnd )
	{
		if ((u32)rMotionData.pAnimInfo->PointsPresent & Flag)
			*pCur++ = (u8)NStreams++;
		else
			*pCur++ = 0;
		Flag <<= 1;
	}

    // Accumulate user streams
	pCur	= &rMotionData.UserStream[0];
	pEnd	= &rMotionData.UserStream[ANIM_MAX_USER_STREAMS];
	Flag	= 1;
	while( pCur != pEnd )
	{
		if ((u32)rMotionData.pAnimInfo->UserStreamsPresent & Flag)
			*pCur++ = (u8)NStreams++;
		else
			*pCur++ = 0;
		Flag <<= 1;
	}
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::SetAnimation( s32				AnimID,
                                 xbool				bMirror )
{
    ASSERT(AM_Inited);

    if( m_AnimGroup == NULL )
        return;

	//---	cancel any multiple animation blending.
	m_nMMM	= 1;
	SetMotionPlaybackData( m_MMM[0], AnimID );

	m_AnimFlags					= m_MMM[0].pAnimInfo ? m_MMM[0].pAnimInfo->Flags : m_AnimFlags;	// if there is no motion set, the flags remain the same that they were
	m_bMirror					= bMirror;
	m_FramesPerCycle			= m_MMM[0].pAnimInfo ? m_MMM[0].pAnimInfo->NFrames : 0;
	m_LastFrameInCycle			= m_FramesPerCycle - 1;
	m_bFramesConst				= TRUE;

	m_MMM[0].Blend				= 1.0f;
	m_MMM[0].FrameRatio			= 1.0f;
	ASSERT(m_MMM[0].BlendOrderRatio == 1.0f);

	SetCycleFrame( 0, 0.0f );

	m_CycleTotalTrans			= m_MMM[0].TotalTrans;
	m_CycleTotalMoveDir			= m_MMM[0].TotalMoveDir;
	m_CycleDeltaRootDir			= m_MMM[0].RootDirAtFrameN;
	m_CycleMoveDirAtFrame0		= m_MMM[0].MoveDirAtFrame0;
	m_CycleMoveDirAtFrameN		= m_MMM[0].MoveDirAtFrameN;
}


//==========================================================================
inline
void mocap_player::CalcSetAnim2FrameRatio( void )
{
	//---	calculate a new number of frames per cycle based on the ratio of the animations
	//		if the number of frames for both animation is the same, this is a trivial task
	if( !m_bFramesConst )
	{
		f32 fNFrames0Cyc	= (f32)(m_MMM[0].pAnimInfo->NFrames-1);
		f32 fNFrames1Cyc	= (f32)(m_MMM[1].pAnimInfo->NFrames-1);
		f32	LastFrameInCycle= (fNFrames0Cyc*m_MMM[0].Blend + fNFrames1Cyc*m_MMM[1].Blend);
		f32	OldFrameRatio0	= m_MMM[0].FrameRatio;
		f32	CyclesPerFrame;

		m_LastFrameInCycle	= (s32)LastFrameInCycle;
		m_FramesPerCycle	= (s32)LastFrameInCycle+1;
		CyclesPerFrame		= 1.0f / (f32)LastFrameInCycle;
		m_MMM[0].FrameRatio	= fNFrames0Cyc*CyclesPerFrame;
		m_MMM[1].FrameRatio	= fNFrames1Cyc*CyclesPerFrame;

		//---	calculate the new current frame in the cycle based on animation Zero's ratio
		m_CurrentFrame		= (m_CurrentFrame*OldFrameRatio0) / m_MMM[0].FrameRatio;
	}
	else
	{
		m_CurrentFrame		= (m_CurrentFrame*m_MMM[0].FrameRatio);
		m_MMM[0].FrameRatio	= 1.0f;
		m_MMM[1].FrameRatio	= 1.0f;
	}
}

//==========================================================================
void mocap_player::SetAnim2Blend( f32 Blend )
{
	ASSERT(m_nMMM==2);
	ASSERT((Blend>=0.0f)&&(Blend <= 1.0f));

	//---	set the blend levels
	m_MMM[0].Blend				= 1.0f - Blend;
	m_MMM[1].Blend				= Blend;
	m_MMM[0].BlendOrderRatio	= 1.0f;
	m_MMM[1].BlendOrderRatio	= Blend;

	//---	calculate the frame ratios
	CalcSetAnim2FrameRatio();

	//---	calculate the total translation
	m_CycleTotalTrans			= m_MMM[0].TotalTrans + (m_MMM[1].TotalTrans-m_MMM[0].TotalTrans)*Blend;

	//---	calculate the blended direction values
	BlendRadian( m_CycleTotalMoveDir, m_MMM[0].TotalMoveDir, m_MMM[1].TotalMoveDir, Blend );
	BlendRadian( m_CycleDeltaRootDir, m_MMM[0].RootDirAtFrameN, m_MMM[1].RootDirAtFrameN, Blend );
	BlendRadian( m_CycleMoveDirAtFrame0, m_MMM[0].MoveDirAtFrame0, m_MMM[1].MoveDirAtFrame0, Blend );
	BlendRadian( m_CycleMoveDirAtFrameN, m_MMM[0].MoveDirAtFrameN, m_MMM[1].MoveDirAtFrameN, Blend );
}

//==========================================================================
void mocap_player::SetAnimation2( s32 AnimID, f32 Blend )
{
    ASSERT(AM_Inited);
    ASSERT(m_AnimGroup);
	ASSERT((Blend>=0.0f)&&(Blend <= 1.0f));
	ASSERTS(m_MMM[0].pAnimInfo, "Must be a primary animation set with SetAnimation() before SetAnimation2() is called.");

	//---	let the system know whether th enumber of frames is constant
	if( m_MMM[0].pAnimInfo->NFrames == ANIM_GetNFrames( AnimID, m_AnimGroup ) )
		m_bFramesConst	= TRUE;
	else
		m_bFramesConst	= FALSE;

	//---	set the second motion data block
	SetMotionPlaybackData( m_MMM[1], AnimID );

	if( m_MMM[1].pAnimInfo==NULL )
	{
		m_nMMM = 1;
		return;
	}

	m_nMMM=2;
	SetAnim2Blend( Blend );
	SetCycleFrame( GetCurrentCycle(), GetCurrentFrame() );
}

//==========================================================================
//
//	The frame ratio is the value used to modify the m_CurrentFrame value to get
//	it into a value which is relative to this animation's frame in 60ths of a sec
//	per frame.  Then, the resulting value can be modified by the frame to export frame
//	conversion value to get the frame in exported data.
//
void mocap_player::CalcSetMultiAnimFrameRatio( void )
{
	SPlaybackData*		pMotionData;
	SPlaybackData*		pEnd;
	f32					LastFrameInCycle;
	f32					LastFrameInCycle1;
	f32					Weight0;
	f32					Weight1;

	//---	calculate a new number of frames per cycle based on the ratio of the animations
	//		if the number of frames for both animation is the same, this is a trivial task
	if( !m_bFramesConst )
	{
		//---	calculate the new m_FramesPerCycle by weighting the lengths of the animations by their
		//		and combining them.
		LastFrameInCycle= (f32)(m_MMM[0].pAnimInfo->NFrames-1);
		pMotionData		= &m_MMM[0];
		pEnd			= &m_MMM[m_nMMM];
		while( ++pMotionData != pEnd )
		{
			Weight1				= pMotionData->BlendOrderRatio;
			Weight0				= 1.0f - Weight1;
			LastFrameInCycle1	= (f32)(pMotionData->pAnimInfo->NFrames-1);
			LastFrameInCycle	= Weight0*LastFrameInCycle + Weight1*LastFrameInCycle1;
		}

		//---	set the new frames per cycle.
		m_LastFrameInCycle	= (s32)LastFrameInCycle;
		m_FramesPerCycle	= m_LastFrameInCycle+1;

		//---	save the old frame ratio to recalculate the new current frame
		f32	OldFrameRatio0	= m_MMM[0].FrameRatio;

		//---	set the new frame ratios for all the motions
		f32	CyclesPerFrame	= 1.0f / LastFrameInCycle;
		pMotionData			= &m_MMM[0];
		pEnd				= &m_MMM[m_nMMM];
		while( pMotionData != pEnd )
		{
			LastFrameInCycle	= (f32)(pMotionData->pAnimInfo->NFrames-1);
			m_MMM[0].FrameRatio	= LastFrameInCycle*CyclesPerFrame;
		}

		//---	calculate the new current frame in the cycle based on animation Zero's ratio
		m_CurrentFrame		= (m_CurrentFrame*OldFrameRatio0) / m_MMM[0].FrameRatio;
	}
	else
	{
		m_CurrentFrame		= (m_CurrentFrame*m_MMM[0].FrameRatio);

		pMotionData	= &m_MMM[0];
		pEnd		= &m_MMM[m_nMMM];
		while( pMotionData != pEnd )
		{
			pMotionData->FrameRatio	= 1.0f;
			pMotionData++;
		}
	}
}

//==========================================================================
void mocap_player::SetMultAnimBlend( f32* pBlendList )
{
	f32					fTotalWeight;
	SPlaybackData*		pMotionData;
	SPlaybackData*		pEnd;
	f32					fBlend;
	f32					fBlendOrderRatio;
	vector3				TotalTrans;
	radian				TotalMoveDir;
	radian				DeltaRootDir;
	radian				MoveDirAtFrame0;
	radian				MoveDirAtFrameN;

	//---	count the total weight given in the animation list, also determine
	//		if all motions in the list have the same number of frames which
	//		will speed up the processing for this function and elsewear.
	TotalTrans.Set( 0.0f, 0.0f, 0.0f );
	TotalMoveDir		= 0.0f;
	DeltaRootDir		= 0.0f;
	MoveDirAtFrame0		= 0.0f;
	MoveDirAtFrameN		= 0.0f;
	fTotalWeight		= 0.0f;
	pMotionData			= &m_MMM[0];
	pEnd				= &m_MMM[m_nMMM];
	while( pMotionData != pEnd )
	{
		fBlend							= *pBlendList++;
		fTotalWeight					+= fBlend;
		pMotionData->Blend				= fBlend;
		fBlendOrderRatio				= fBlend/fTotalWeight;
		pMotionData->BlendOrderRatio	= fBlendOrderRatio;

		//---	accumulate the total transition
		TotalTrans = TotalTrans + (pMotionData->TotalTrans - TotalTrans )*fBlendOrderRatio;

		//---	calculate the blended direction values
		BlendRadian( TotalMoveDir, TotalMoveDir, pMotionData->TotalMoveDir, fBlendOrderRatio );
		BlendRadian( DeltaRootDir, DeltaRootDir, pMotionData->RootDirAtFrameN, fBlendOrderRatio );
		BlendRadian( MoveDirAtFrame0, MoveDirAtFrame0, pMotionData->MoveDirAtFrame0, fBlendOrderRatio );
		BlendRadian( MoveDirAtFrameN, MoveDirAtFrameN, pMotionData->MoveDirAtFrameN, fBlendOrderRatio );

		++pMotionData;
	}
	ASSERT((fTotalWeight>=(1.0f-0.001f))&&(fTotalWeight<=(1.0f+0.001f)));

	//---	save the total displacement values
	m_CycleTotalTrans	= TotalTrans;
	m_CycleDeltaRootDir	= DeltaRootDir;

	//---	calculate the frame ratio
	CalcSetMultiAnimFrameRatio();
}

//==========================================================================
void mocap_player::SetMultAnimation( s32* pAnimIDList, f32* pBlendList, s32 ListLength, xbool bMirror )
{
	xbool				bSameNumFrames;
	s32*				pAnim;
	s32*				pEnd;
	SPlaybackData*		pMotionData;
	s32					NFrames;
	s32					AnimID;
	ASSERT(pAnimIDList);

	//---	make sure the length of the list is not greater than the maximum
	ASSERT(ListLength <= MAX_MOTION_BLEND_COUNT);
	if( ListLength > MAX_MOTION_BLEND_COUNT )
		ListLength = MAX_MOTION_BLEND_COUNT;
	m_nMMM		= ListLength;
	m_bMirror	= bMirror;

	//---	count the total weight given in the animation list, also determine
	//		if all motions in the list have the same number of frames which
	//		will speed up the processing for this function and elsewear.
	bSameNumFrames		= TRUE;
	NFrames				= ANIM_GetNFrames( *pAnimIDList, m_AnimGroup );
	pAnim				= pAnimIDList;
	pEnd				= &pAnimIDList[m_nMMM];
	pMotionData			= &m_MMM[0];
	while( pAnim != pEnd )
	{
		AnimID	= *pAnim++;
		ASSERT((AnimID >= 0)&&(AnimID < 100000));

		//---	check the number of frames in this animation against the
		//		first animation to determine if there is a difference.
		if( ANIM_GetNFrames( AnimID, m_AnimGroup ) != NFrames )
			bSameNumFrames = FALSE;

		//---	set the motion playback data
		SetMotionPlaybackData( *pMotionData, AnimID );
		++pMotionData;
	}

	//---	flag that the number of frames is identical
	m_bFramesConst	= bSameNumFrames;

	//---	calculate the frame ratio
	SetMultAnimBlend( pBlendList );

	//---	set the motion to frame zero
	SetCycleFrame( 0, 0.0f );
}

//==========================================================================
void mocap_player::QuaternionBlend( xbool bSetting )
{
}

///////////////////////////////////////////////////////////////////////////
//	Get the raw translation and facing of this motion at the export frame
inline
void mocap_player::CalcSetAnimRawMotion( SPlaybackData& rMotionData )
{
	vector3     Trans0;
	vector3     Trans1;
	s16*        DataPtr0;
	s16*        DataPtr1;
	f32			MoveDir0;
	f32			MoveDir1;
	f32			RootDir0;
	f32			RootDir1;
	f32         Frac;

	ASSERT((rMotionData.CurrentExportFrame>=rMotionData.Block.Frame0) && 
		   (rMotionData.CurrentExportFrame<=rMotionData.Block.FrameN));

	// Get ptrs to the frames
	GetFrameDataPtrs( rMotionData, &DataPtr0, &DataPtr1, &Frac );

	if( rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_QUAT_W )
	{
		DataPtr0 += (m_AnimGroup->NBones*4);
		DataPtr1 += (m_AnimGroup->NBones*4);
	}
	else
	{
		DataPtr0 += (m_AnimGroup->NBones*3);
		DataPtr1 += (m_AnimGroup->NBones*3);
	}

	// Pull translation and angles from streams and interpolate.
	Trans1.X        = (f32)(DataPtr1[0]) * VALUE_TO_INCHES;
	Trans1.Y        = (f32)(DataPtr1[1]) * VALUE_TO_INCHES;
	Trans1.Z        = (f32)(DataPtr1[2]) * VALUE_TO_INCHES;
	Trans0.X        = (f32)(DataPtr0[0]) * VALUE_TO_INCHES;
	Trans0.Y        = (f32)(DataPtr0[1]) * VALUE_TO_INCHES;
	Trans0.Z        = (f32)(DataPtr0[2]) * VALUE_TO_INCHES;
	MoveDir0		= (f32)(DataPtr0[3]) * rMotionData.AngleToRadians;
	MoveDir1		= (f32)(DataPtr1[3]) * rMotionData.AngleToRadians;
	RootDir0		= (f32)(DataPtr0[4]) * rMotionData.AngleToRadians;
	RootDir1		= (f32)(DataPtr1[4]) * rMotionData.AngleToRadians;

	// Interpolate the direction data
	BlendRadian( rMotionData.RawRootDir, RootDir0, RootDir1, Frac );
	BlendRadian( rMotionData.RawMoveDir, MoveDir0, MoveDir1, Frac );

	// Interpolate motion data
	rMotionData.RawPos.X = Trans0.X + Frac*(Trans1.X - Trans0.X);
	rMotionData.RawPos.Y = Trans0.Y + Frac*(Trans1.Y - Trans0.Y);
	rMotionData.RawPos.Z = Trans0.Z + Frac*(Trans1.Z - Trans0.Z);
}


///////////////////////////////////////////////////////////////////////////
inline
void mocap_player::CalcSetRawMotion( void )
{
	f32		Weight0;
	f32		Weight1;

	//---	get the complete motion from the first motion
	CalcSetAnimRawMotion( m_MMM[0] );
	m_CycleRawRootDir	= m_MMM[0].RawRootDir;
	m_CycleRawMoveDir	= m_MMM[0].RawMoveDir;
	m_CycleRawPos		= m_MMM[0].RawPos;

	//---	if there are more than one motion, blend their values together.
	if( m_nMMM >= 2 )
	{
		SPlaybackData*	pMotionData	= &m_MMM[0];
		SPlaybackData*	pEnd		= &m_MMM[m_nMMM];
		while( ++pMotionData != pEnd )
		{
			CalcSetAnimRawMotion( *pMotionData );

			Weight1			= pMotionData->BlendOrderRatio;
			Weight0			= 1.0f - Weight1;

			BlendRadian( m_CycleRawRootDir, m_CycleRawRootDir, pMotionData->RawRootDir, Weight1 );
			BlendRadian( m_CycleRawMoveDir, m_CycleRawMoveDir, pMotionData->RawMoveDir, Weight1 );

			m_CycleRawPos		= Weight0*m_CycleRawPos + Weight1*pMotionData->RawPos;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	Currently, this function only queues up events from the primary motion.
//
void mocap_player::QueueEvents( f32 StartFrame, f32 EndFrame )
{
	//======================
#ifdef	ANIM_STATS_ACTIVE
	s32	EventsQueued = m_NEventsQueued;
#endif
	AMSTATS_AM_EVENTS_START//==== PROFILING
	//======================

    s32 StartCycle;
    s32 EndCycle;
    s32 NFrames;
    s32 i;
    anim_event* EV;
    s32 EVFrame;

	//
	//	return early if there is no real animation set
	//
	if( !m_MMM[0].pAnimInfo )
		return;

    ASSERT(m_MMM[0].pAnimInfo);

	//---	get start and end frames in terms of the primary motion
	StartFrame	*= m_MMM[0].FrameRatio;
	EndFrame	*= m_MMM[0].FrameRatio;

    //---	Fudge start frame to capture 0 frame event
    if ((m_CurrentCycle==0) && (StartFrame==0.0f))
        StartFrame = -0.01f;

    //---	Clear old events and check if animation has any events
    m_NEventsQueued = 0;
    if( m_MMM[0].pAnimInfo->NEvents == 0 )
		return;

    //---	Compute start and end cycles
    NFrames    = m_MMM[0].pAnimInfo->NFrames;
    StartCycle = (s32)(StartFrame - LPR(StartFrame,(f32)(NFrames-1))) / NFrames;
    EndCycle   = (s32)(EndFrame   - LPR(EndFrame  ,(f32)(NFrames-1))) / NFrames;
    EV         = &m_AnimGroup->AnimEvent[ m_MMM[0].pAnimInfo->EventIndex ];

    if (EndFrame > StartFrame)
    {
        while (StartCycle <= EndCycle)
        {
            for (i=0; i<m_MMM[0].pAnimInfo->NEvents; i++)
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
            for (i=m_MMM[0].pAnimInfo->NEvents-1; i>=0; i--)
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

	//====================================================
	AMSTATS_AM_EVENTS_END(m_NEventsQueued-EventsQueued);//==== PROFILING
	//====================================================
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::AdvNFrames( f32              NFrames,
                               vector3&         NewWorldPos,
                               radian&          NewWorldRootDir,
                               radian&          NewWorldMoveDir,
                               radian&          NewWorldTotalMoveDir)
{
	s32         LastFrameInCycle;
	s32         CyclesCrossed;
	f32         ModulatedFrame;
	f32         C, S;
	f32			X, Z;
	radian		RD;
	radian		OldRawRootDir;
	vector3		OldRawPos;
	radian		NewRawRootDir;
	vector3		NewRawPos;
	radian		DeltaRootDir;
	vector3		DeltaPos;

	//
	//	if there is no motion set, return now
	//
	if( !m_MMM[0].pAnimInfo )
		return;

	ASSERT(m_MMM[0].pAnimInfo);
	ASSERT((NFrames<1000.0f) && (NFrames>-1000.0f));

	// Clear event queue
	m_NEventsQueued = 0;

	// If frame increment is zero, nothing to do
	if (NFrames == 0.0f) 
	{
		NewWorldPos          = m_WorldPos;
		NewWorldRootDir      = m_WorldRootDir;
		NewWorldMoveDir      = m_WorldMoveDir;
		NewWorldTotalMoveDir = m_WorldTotalMoveDir;
		return;
	}

	// Queue events into event stack.
	QueueEvents( m_CurrentFrame, m_CurrentFrame + NFrames );

	// Compute new CurrentFrame and CurrentCycle
	LastFrameInCycle = m_LastFrameInCycle;
	ModulatedFrame  = m_CurrentFrame + NFrames;
	CyclesCrossed   = 0;
	if (NFrames > 0)
	{
		while (ModulatedFrame > LastFrameInCycle) 
		{
			ModulatedFrame -= LastFrameInCycle;
			CyclesCrossed++;
		}
	}
	else
	{
		while (ModulatedFrame < 0) 
		{
			ModulatedFrame += LastFrameInCycle;
			CyclesCrossed--;
		}
	}
	m_CurrentCycle	+= CyclesCrossed;
	m_CurrentFrame	= ModulatedFrame;

	ASSERT(m_CurrentFrame >=0 && m_CurrentFrame<=LastFrameInCycle);

	//---	set the export frame based on the current frame
	CalcSetExportFrame();

	//---	save the current raw values within the current motion
	OldRawPos		= m_CycleRawPos;
	OldRawRootDir	= m_CycleRawRootDir;

	//---	get the complete delta motion since the last export frame.
	CalcSetRawMotion();

	//---	get the new raw values within the current motion
	NewRawPos		= m_CycleRawPos;
	NewRawRootDir	= m_CycleRawRootDir;

	//---	Add any cycles to the new raw motion values
	//		Calculate the new values assuming that the motion isn't
	//		mirrored at first.
	if( CyclesCrossed )
	{
		if( m_CycleDeltaRootDir==R_0 )
		{
			NewRawPos.X += m_CycleTotalTrans.X * (f32)CyclesCrossed;
			NewRawPos.Z += m_CycleTotalTrans.Z * (f32)CyclesCrossed;
		}
		else if (CyclesCrossed > 0)
		{
			RD = m_CycleDeltaRootDir;
			x_sincos( RD, S, C );

			while (CyclesCrossed > 0)
			{
				X = NewRawPos.X;
				Z = NewRawPos.Z;
				NewRawPos.X  = (C*X + S*Z) + m_CycleTotalTrans.X;
				NewRawPos.Z  = (C*Z - S*X) + m_CycleTotalTrans.Z;
				NewRawRootDir += RD;
				CyclesCrossed--;
			}
		}
		else // if (CyclesCrossed < 0)
		{
			RD = -m_CycleDeltaRootDir;
			x_sincos( RD, S, C );

			while (CyclesCrossed < 0)
			{
				X = NewRawPos.X;
				Z = NewRawPos.Z;
				NewRawPos.X  = (C*X + S*Z) - m_CycleTotalTrans.X;
				NewRawPos.Z  = (C*Z - S*X) - m_CycleTotalTrans.Z;
				NewRawRootDir += RD;
				CyclesCrossed++;
			}
		}
	}

	//---	calculate the deltas between the old and new values
	DeltaRootDir	= NewRawRootDir - OldRawRootDir;
	DeltaPos.X		= NewRawPos.X - OldRawPos.X;
	DeltaPos.Y		= NewRawPos.Y;
	DeltaPos.Z		= NewRawPos.Z - OldRawPos.Z;

	//---	apply a mirror to the delta values if the motion is mirrored
	if( m_bMirror ) 
	{
		DeltaPos.X		= -DeltaPos.X;
		DeltaRootDir	= -DeltaRootDir;
	}

	//---	Rotate the Raw ZX deltas by the current Yaw
	x_sincos( m_WorldYaw, S, C );
	NewWorldPos.Z = DeltaPos.Z*C - DeltaPos.X*S;
	NewWorldPos.X = DeltaPos.X*C + DeltaPos.Z*S;
	NewWorldPos.Y = DeltaPos.Y;

	//---	Scale deltas 
	NewWorldPos.X *= m_WorldScale;
	NewWorldPos.Y *= m_WorldScale;
	NewWorldPos.Z *= m_WorldScale;

	//---	Add current position on
	NewWorldPos.X += m_WorldPos.X;
	NewWorldPos.Z += m_WorldPos.Z;

	NewWorldRootDir      = m_WorldRootDir + DeltaRootDir;
	NewWorldMoveDir      = m_WorldMoveDir + DeltaRootDir;
	NewWorldTotalMoveDir = m_WorldTotalMoveDir + DeltaRootDir;
}

///////////////////////////////////////////////////////////////////////////
void   mocap_player::JumpToFrame ( f32 DestFrame )
{
    s32         LastFrameInCycle;
    s32         Cycle;
    f32         OldWorldRootDir;

	//
	//	if there is no motion set, return now
	//
	if( !m_MMM[0].pAnimInfo )
		return;

	ASSERT(m_MMM[0].pAnimInfo);

    // Record current world root dir for later use
    OldWorldRootDir = m_WorldRootDir;

    // Clear event queue
    m_NEventsQueued = 0;
    
    // if the destination frame is the same as the current frame, exit quickly
	if( DestFrame == m_CurrentFrame )
	{
		m_CurrentCycle = 0;
		return;
	}

	//---	calculate the current cycle and frame
	Cycle			= 0;
	LastFrameInCycle	= m_LastFrameInCycle;
	if( (DestFrame < 0.0f) || (DestFrame > LastFrameInCycle) )
	{
		while( DestFrame < 0.0f )
		{
			DestFrame += LastFrameInCycle;
			--Cycle;
		}
		while( DestFrame > LastFrameInCycle )
		{
			DestFrame -= LastFrameInCycle;
			++Cycle;
		}
	}

	//---	store the current cycle and frame
	SetCycleFrame( Cycle, DestFrame );

    // Compute yaw and other dirs to keep rootdir aligned
    SetWorldRootDir( OldWorldRootDir );
}

///////////////////////////////////////////////////////////////////////////
inline
void mocap_player::SetExportFrame( SPlaybackData& rMotionData, f32 fExportFrame )
{
	//---	if there is no motion set up, exit early
	if( rMotionData.pAnimInfo == NULL )
	{
		rMotionData.CurrentExportFrame = 0.0f;
		return;
	}

	f32	fExportNFrames	 = (f32)(rMotionData.pAnimInfo->ExportNFrames-1);

	//---	set the export frame based on the current frame
	if( fExportFrame > fExportNFrames )
		fExportFrame = fExportNFrames; // keep it off the very last frame

	rMotionData.CurrentExportFrame = fExportFrame;
    
    // Be sure correct anim block is loaded
    if ((fExportFrame < rMotionData.Block.Frame0) || 
        (fExportFrame > rMotionData.Block.FrameN))
    {
		if( rMotionData.Block.pData )
			AMCACHE_UnlockBlock( m_AnimGroup, rMotionData.pAnimInfo, rMotionData.Block );
        AMCACHE_LockBlock(m_AnimGroup, rMotionData.pAnimInfo, rMotionData.Block, fExportFrame );
    }
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::CalcSetExportFrame( void )
{
	SPlaybackData*	pMotionData;
	SPlaybackData*	pEnd;
	f32				fExportFrame;

	//---	Calculate and set the export frame for the primary motion
	fExportFrame	= m_CurrentFrame*m_MMM[0].FrameRatio*m_MMM[0].FrameToExportFrame;
	SetExportFrame( m_MMM[0], fExportFrame );

	//---	if there are multiple motions, set the export frame for the rest.
	if( m_nMMM > 1 )
	{
		pMotionData	= &m_MMM[1];
		pEnd		= &m_MMM[m_nMMM-1];
		if( m_bFramesConst )
		{
			while( pMotionData!=pEnd )
				SetExportFrame( *pMotionData, fExportFrame );
		}
		else
		{
			while( pMotionData!=pEnd )
			{
				fExportFrame	= m_CurrentFrame*pMotionData->FrameRatio*pMotionData->FrameToExportFrame ;
				SetExportFrame( *pMotionData, fExportFrame );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
xbool   mocap_player::TimeTillEvent( s16                EventID,
                                     f32&               Frame )
{
    anim_event* Event;
    s32         i;
    f32         NumFrames;
    s32         PreFrameEvent;  // Earliest event before RelFrame
    s32         PostFrameEvent; // Earliest event after RelFrame
	f32			CurrentFrame;	// scaled to animation zero

    ASSERT(m_MMM[0].pAnimInfo);
    ASSERT(m_AnimGroup);

    NumFrames   = (f32)(m_MMM[0].pAnimInfo->NFrames-1);
    Event       = &m_AnimGroup->AnimEvent[ m_MMM[0].pAnimInfo->EventIndex ];

	CurrentFrame	= m_CurrentFrame*m_MMM[0].FrameRatio;

    // Assign pre and post events
    PreFrameEvent  = -1;
    PostFrameEvent = -1;

    // Assume playing animation forward
    for (i=0; i<m_MMM[0].pAnimInfo->NEvents; i++)
    {
        // Be sure event matches requested event id
        if (Event[i].EventID == EventID)
        {
            // Check for earliest event < RelFrame
            if ((PreFrameEvent==-1) && ((f32)(Event[i].Frame) < CurrentFrame))
                PreFrameEvent = i;

            // Check for earliest event >= RelFrame
            if ((PostFrameEvent==-1) && ((f32)(Event[i].Frame) >= CurrentFrame))
            {
                PostFrameEvent = i;
                break;
            }
        }
    }

    // Check if PostFrame event is available
    if (PostFrameEvent != -1)
    {
        Frame = (f32)(Event[PostFrameEvent].Frame) - CurrentFrame;
        Frame /= m_MMM[0].FrameRatio;
        return TRUE;
    }

    // Check if PreFrame event is available
    if (PreFrameEvent != -1)
    {
        Frame = (f32)(Event[PreFrameEvent].Frame) + NumFrames - CurrentFrame;
        Frame /= m_MMM[0].FrameRatio;
        return TRUE;
    }

    // No event found
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////
void mocap_player::GetFrameDataPtrs( SPlaybackData&	rMotionData,
									 s16**			DataPtr0,
									 s16**			DataPtr1,
									 f32*			Frac )
{
    f32 F;
    s32 FInt;

    ASSERT((rMotionData.CurrentExportFrame>=rMotionData.Block.Frame0) && 
           (rMotionData.CurrentExportFrame<=rMotionData.Block.FrameN));

    // Compute where in data the frames begin
	F = rMotionData.CurrentExportFrame - rMotionData.Block.Frame0;
	if( F < 0.0f )
		F = 0.0f;
	else if( F > (f32)rMotionData.Block.FrameN )
		F = (f32)rMotionData.Block.FrameN;

    FInt    = (s32)F;

    // Check if we are exactly on last frame
    if (((f32)FInt == F) && (FInt!=0))
    {
        FInt--;
        *Frac   = 1.0f;
    }
    else
    {   
        *Frac	= F - (f32)FInt;
    }

    // Compute ptrs and fraction
    *DataPtr0  = rMotionData.Block.pData + (FInt * ((s32)rMotionData.pAnimInfo->NStreams));
    *DataPtr1  = (*DataPtr0) + (s32)(rMotionData.pAnimInfo->NStreams);
}

///////////////////////////////////////////////////////////////////////////
inline
void mocap_player::GetAnimBoneData( SPlaybackData& rMotionData, URotData& Dest, s32	Bone, f32 Blend )
{
    f32			Frac;
    radian3		Rot0;
    radian3		Rot1;
	radian3		R, R2;
    s16*		DataPtr0;
    s16*		DataPtr1;
	quaternion	Quat0;
	quaternion	Quat1;
	quaternion	Q;
	f32			qW;
	f32			ValueToQuat;
	f32			ValueToQuatW;
	f32			AngleToRad;

    ASSERT(AM_Inited);

	// if there is no motion set, return zero rotation
	if( !rMotionData.pAnimInfo )
	{
		Dest._q.X = Dest._q.Y = Dest._q.Z = 0.0f;
		Dest._q.W = 1.0f;
		return;
	}

    // Check if animation has correct data decompressed
    ASSERT((rMotionData.CurrentExportFrame>=rMotionData.Block.Frame0) && 
           (rMotionData.CurrentExportFrame<=rMotionData.Block.FrameN));

    // Get ptrs to the frames
    GetFrameDataPtrs( rMotionData, &DataPtr0, &DataPtr1, &Frac );

	if( rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_QUATERNION )
	{
		if( rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_QUAT_W )
		{
			//---	get pointers to the quaternion location
			DataPtr0 += 4*Bone;
			DataPtr1 += 4*Bone;

			//---	load the two quaternion values
			ValueToQuat		= rMotionData.ValueToQuat;
			ValueToQuatW	= rMotionData.ValueToQuatW;
			Quat0.X			= (f32)((u16*)DataPtr0)[0]*ValueToQuat - 1.0f;
			Quat0.Y			= (f32)((u16*)DataPtr0)[1]*ValueToQuat - 1.0f;
			Quat0.Z			= (f32)((u16*)DataPtr0)[2]*ValueToQuat - 1.0f;
			Quat0.W			= (f32)((u16*)DataPtr0)[3]*ValueToQuatW;
			Quat1.X			= (f32)((u16*)DataPtr1)[0]*ValueToQuat - 1.0f;
			Quat1.Y			= (f32)((u16*)DataPtr1)[1]*ValueToQuat - 1.0f;
			Quat1.Z			= (f32)((u16*)DataPtr1)[2]*ValueToQuat - 1.0f;
			Quat1.W			= (f32)((u16*)DataPtr1)[3]*ValueToQuatW;
		}
		else
		{
			//---	get pointers to the quaternion location
			DataPtr0 += 3*Bone;
			DataPtr1 += 3*Bone;

			//---	load the two quaternion values
			ValueToQuat		= rMotionData.ValueToQuat;
			Quat0.X			= (f32)((u16*)DataPtr0)[0]*ValueToQuat - 1.0f;
			Quat0.Y			= (f32)((u16*)DataPtr0)[1]*ValueToQuat - 1.0f;
			Quat0.Z			= (f32)((u16*)DataPtr0)[2]*ValueToQuat - 1.0f;
			Quat1.X			= (f32)((u16*)DataPtr1)[0]*ValueToQuat - 1.0f;
			Quat1.Y			= (f32)((u16*)DataPtr1)[1]*ValueToQuat - 1.0f;
			Quat1.Z			= (f32)((u16*)DataPtr1)[2]*ValueToQuat - 1.0f;

			qW				= 1.0f - (Quat0.X*Quat0.X + Quat0.Y*Quat0.Y + Quat0.Z*Quat0.Z);
			if( qW > 0.0f )	Quat0.W	= (f32)x_sqrt( qW );
			else			Quat0.W	= 0.0f;

			qW				= 1.0f - (Quat1.X*Quat1.X + Quat1.Y*Quat1.Y + Quat1.Z*Quat1.Z);
			if( qW > 0.0f )	Quat1.W = (f32)x_sqrt( qW );
			else			Quat1.W = 0.0f;
		}

		//---	get the interpolated motion (if it must be blended with
		//		the current value, do that as well)
		if( Blend == 1.0f )
			Dest._q	= BlendQuaternion( Quat0, Quat1, Frac );
		else
		{
			Q			= BlendQuaternion( Quat0, Quat1, Frac );
			Dest._q		= BlendQuaternion( Dest._q, Q, Blend );
		}
	}
	else
	{
		// Decide whether to do inner anim blending
		if( rMotionData.pAnimInfo->Flags&ANIMINFO_FLAG_QUATBLEND )
		{
			DataPtr0 += 3*Bone;
			DataPtr1 += 3*Bone;

			// Grab rotation data on both sides of frame
			AngleToRad	= rMotionData.AngleToRadians;
			Rot0.Pitch = (f32)(DataPtr0[0])*AngleToRad;
			Rot0.Yaw   = (f32)(DataPtr0[1])*AngleToRad;
			Rot0.Roll  = (f32)(DataPtr0[2])*AngleToRad;
			Rot1.Pitch = (f32)(DataPtr1[0])*AngleToRad;
			Rot1.Yaw   = (f32)(DataPtr1[1])*AngleToRad;
			Rot1.Roll  = (f32)(DataPtr1[2])*AngleToRad;
//	ASSERT_RADIAN3(Rot0);
//	ASSERT_RADIAN3(Rot1);

			if( Blend == 1.0f )
				Dest._r	= BlendUsingQuaternions( Rot0, Rot1, Frac );
			else
			{
				R		= BlendUsingQuaternions( Rot0, Rot1, Frac );
				Dest._r	= BlendUsingQuaternions( Dest._r, R, Blend );
			}
		}
		else
		{
			DataPtr0 += 3*Bone;
			DataPtr1 += 3*Bone;

			AngleToRad	= rMotionData.AngleToRadians;
			Rot0.Pitch = (f32)(DataPtr0[0])*AngleToRad;
			Rot0.Yaw   = (f32)(DataPtr0[1])*AngleToRad;
			Rot0.Roll  = (f32)(DataPtr0[2])*AngleToRad;
			Rot1.Pitch = (f32)(DataPtr1[0])*AngleToRad;
			Rot1.Yaw   = (f32)(DataPtr1[1])*AngleToRad;
			Rot1.Roll  = (f32)(DataPtr1[2])*AngleToRad;

			//---	get the interpolated motion (if it must be blended with
			//		the current value, do that as well)
			if( Blend == 1.0f )
				BlendRadian3( Dest._r, Rot0, Rot1, Frac );
			else
			{
				R2	= Dest._r;
				BlendRadian3( R, Rot0, Rot1, Frac );
				BlendRadian3( Dest._r, R2, R, Blend );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
inline
void mocap_player::GetAnimBoneData( SPlaybackData& rMotionData, URotData* Dest, f32 Blend )
{
    f32			Frac;
    radian3		Rot0;
    radian3		Rot1;
	radian3		R, R2;
    s16*		DataPtr0;
    s16*		DataPtr1;
	quaternion	Quat0;
	quaternion	Quat1;
	quaternion	Q;
	f32			qW;
	u32			End;
	f32			ValueToQuat;
	f32			ValueToQuatW;
	f32			AngleToRad;
	URotData*	pRotData	= Dest;

    ASSERT(AM_Inited);

	//
	//	if there is no motion set, return a rotation of zero
	//
	if( !rMotionData.pAnimInfo )
	{
		URotData* pData	= &Dest[0];
		URotData* pEnd	= &Dest[m_AnimGroup->NBones];
		while( pData != pEnd )
		{
			pData->_q.X = pData->_q.Y = pData->_q.Z = 0.0f;
			pData->_q.W = 1.0f;
			++pData;
		}

		return;
	}

    // Check if animation has correct data decompressed
    ASSERT((rMotionData.CurrentExportFrame>=rMotionData.Block.Frame0) && 
           (rMotionData.CurrentExportFrame<=rMotionData.Block.FrameN));

    // Get ptrs to the frames
    GetFrameDataPtrs( rMotionData, &DataPtr0, &DataPtr1, &Frac );

	if( rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_QUATERNION )
	{
		if( rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_QUAT_W )
		{
			End			= (u32)(&DataPtr0[4*m_AnimGroup->NBones]);

			ValueToQuat		= rMotionData.ValueToQuat;
			ValueToQuatW	= rMotionData.ValueToQuatW;
			while( (u32)DataPtr0 != End )
			{
				//---	load the two quaternion values
				Quat0.X			= (f32)((u16*)DataPtr0)[0]*ValueToQuat - 1.0f;
				Quat0.Y			= (f32)((u16*)DataPtr0)[1]*ValueToQuat - 1.0f;
				Quat0.Z			= (f32)((u16*)DataPtr0)[2]*ValueToQuat - 1.0f;
				Quat0.W			= (f32)((u16*)DataPtr0)[3]*ValueToQuatW;
				Quat1.X			= (f32)((u16*)DataPtr1)[0]*ValueToQuat - 1.0f;
				Quat1.Y			= (f32)((u16*)DataPtr1)[1]*ValueToQuat - 1.0f;
				Quat1.Z			= (f32)((u16*)DataPtr1)[2]*ValueToQuat - 1.0f;
				Quat1.W			= (f32)((u16*)DataPtr1)[3]*ValueToQuatW;

				//---	get the interpolated motion (if it must be blended with
				//		the current value, do that as well)
				if( Blend == 1.0f )
					pRotData->_q	= BlendQuaternion( Quat0, Quat1, Frac );
				else
				{
					Q				= BlendQuaternion( Quat0, Quat1, Frac );
					pRotData->_q	= BlendQuaternion( pRotData->_q, Q, Blend );
				}

				++pRotData;

				//---	get pointers to the quaternion location
				DataPtr0 += 4;
				DataPtr1 += 4;
			}
		}
		else
		{
			End			= (u32)(&DataPtr0[3*m_AnimGroup->NBones]);

			ValueToQuat	= rMotionData.ValueToQuat;
			while( (u32)DataPtr0 != End )
			{
				//---	load the two quaternion values
				Quat0.X			= (f32)((u16*)DataPtr0)[0]*ValueToQuat - 1.0f;
				Quat0.Y			= (f32)((u16*)DataPtr0)[1]*ValueToQuat - 1.0f;
				Quat0.Z			= (f32)((u16*)DataPtr0)[2]*ValueToQuat - 1.0f;
				Quat1.X			= (f32)((u16*)DataPtr1)[0]*ValueToQuat - 1.0f;
				Quat1.Y			= (f32)((u16*)DataPtr1)[1]*ValueToQuat - 1.0f;
				Quat1.Z			= (f32)((u16*)DataPtr1)[2]*ValueToQuat - 1.0f;

				qW				= 1.0f - (Quat0.X*Quat0.X + Quat0.Y*Quat0.Y + Quat0.Z*Quat0.Z);
				if( qW > 0.0f )	Quat0.W	= (f32)x_sqrt( qW );
				else			Quat0.W	= 0.0f;

				qW				= 1.0f - (Quat1.X*Quat1.X + Quat1.Y*Quat1.Y + Quat1.Z*Quat1.Z);
				if( qW > 0.0f )	Quat1.W = (f32)x_sqrt( qW );
				else			Quat1.W = 0.0f;

				//---	get the interpolated motion (if it must be blended with
				//		the current value, do that as well)
				if( Blend == 1.0f )
					pRotData->_q	= BlendQuaternion( Quat0, Quat1, Frac );
				else
				{
					Q				= BlendQuaternion( Quat0, Quat1, Frac );
					pRotData->_q	= BlendQuaternion( pRotData->_q, Q, Blend );
				}

				++pRotData;

				//---	get pointers to the quaternion location
				DataPtr0 += 3;
				DataPtr1 += 3;
			}
		}
	}
	else
	{
		End			= (u32)(&DataPtr0[3*m_AnimGroup->NBones]);

		// Decide whether to do inner anim blending
		if( rMotionData.pAnimInfo->Flags&ANIMINFO_FLAG_QUATBLEND )
		{
			AngleToRad	= rMotionData.AngleToRadians;
			while( (u32)DataPtr0 != End )
			{
				// Grab rotation data on both sides of frame
				Rot0.Pitch = (f32)(DataPtr0[0])*AngleToRad;
				Rot0.Yaw   = (f32)(DataPtr0[1])*AngleToRad;
				Rot0.Roll  = (f32)(DataPtr0[2])*AngleToRad;
				Rot1.Pitch = (f32)(DataPtr1[0])*AngleToRad;
				Rot1.Yaw   = (f32)(DataPtr1[1])*AngleToRad;
				Rot1.Roll  = (f32)(DataPtr1[2])*AngleToRad;
//	ASSERT_RADIAN3(Rot0);
//	ASSERT_RADIAN3(Rot1);

				//---	get the interpolated motion (if it must be blended with
				//		the current value, do that as well)
				if( Blend == 1.0f )
					pRotData->_r	= BlendUsingQuaternions( Rot0, Rot1, Frac );
				else
				{
					R				= BlendUsingQuaternions( Rot0, Rot1, Frac );
					pRotData->_r	= BlendUsingQuaternions( pRotData->_r, R, Blend );
				}

				++pRotData;
				DataPtr0 += 3;
				DataPtr1 += 3;
			}
		}
		else
		{
			AngleToRad	= rMotionData.AngleToRadians;
			while( (u32)DataPtr0 != End )
			{
				Rot0.Pitch = (f32)(DataPtr0[0])*AngleToRad;
				Rot0.Yaw   = (f32)(DataPtr0[1])*AngleToRad;
				Rot0.Roll  = (f32)(DataPtr0[2])*AngleToRad;
				Rot1.Pitch = (f32)(DataPtr1[0])*AngleToRad;
				Rot1.Yaw   = (f32)(DataPtr1[1])*AngleToRad;
				Rot1.Roll  = (f32)(DataPtr1[2])*AngleToRad;

				//---	get the interpolated motion (if it must be blended with
				//		the current value, do that as well)
				if( Blend == 1.0f )
					BlendRadian3( pRotData->_r, Rot0, Rot1, Frac );
				else
				{
					R2	= pRotData->_r;
					BlendRadian3( R, Rot0, Rot1, Frac );
					BlendRadian3( pRotData->_r, R2, R, Blend );
				}

				++pRotData;
				DataPtr0 += 3;
				DataPtr1 += 3;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
inline void mocap_player::GetBoneMatrix( s32 BoneID, matrix4& LocalToWorldM )
{
    URotData    TempRotArray[50];

    ASSERT(m_AnimGroup);
    ASSERT((BoneID>=0) && (BoneID<m_AnimGroup->NBones));
    ASSERT(m_AnimGroup->NBones <= 50);

    // Load rotations into temp buffer
    GetBoneData( TempRotArray );

    // Get matrix from skeleton
    AMSKL_GetBoneMatrix(m_AnimGroup,
						LocalToWorldM,
						BoneID,
						TempRotArray,
						GetBoneDataFormat(),
						m_bMirror,
						FALSE,
						m_WorldScale,
						GetRenderFacing(),
						m_WorldPos,
						m_bTransform ? &m_Transform : NULL );
}

///////////////////////////////////////////////////////////////////////////
inline void mocap_player::GetBoneMatrices( matrix4* LocalToWorldM )
{
    URotData	TempRotArray[50];

    ASSERT(m_AnimGroup);
    ASSERT(m_AnimGroup->NBones <= 50);
    ASSERT(LocalToWorldM);

    // Load rotations into temp buffer
    GetBoneData( TempRotArray );

    // Get matrix from skeleton
    AMSKL_GetBoneMatrices ( m_AnimGroup,
                            LocalToWorldM,
                            TempRotArray,
							GetBoneDataFormat(),
                            m_bMirror,
							FALSE,
                            m_WorldScale,
                            GetRenderFacing(),
                            m_WorldPos,
							m_bTransform ? &m_Transform : NULL );
}

///////////////////////////////////////////////////////////////////////////
URotData& mocap_player::GetBoneData( s32 Bone, URotData& Dest )
{
	SPlaybackData*	pMotionData;
	SPlaybackData*	pEnd;

	//---	get the bone data for the first bone
	GetAnimBoneData( m_MMM[0], Dest, Bone );

	//---	if there is only one motion, get the data and go
	if( m_nMMM >= 2 )
	{
		//---	there are 2 or more blended motions
		pMotionData	= &m_MMM[0];
		pEnd		= &m_MMM[m_nMMM];
		ASSERT((m_nMMM>=1)&&(m_nMMM<MAX_MOTION_BLEND_COUNT));

		while( ++pMotionData != pEnd )
			GetAnimBoneData( *pMotionData, Dest, Bone, pMotionData->BlendOrderRatio );
	}

	return Dest;
}

///////////////////////////////////////////////////////////////////////////
URotData* mocap_player::GetBoneData( URotData* Dest )
{
	//====================
	AMSTATS_INTERP_START//==== PROFILING
	//====================

	SPlaybackData*	pMotionData;
	SPlaybackData*	pEnd;

	GetAnimBoneData( m_MMM[0], Dest );

	//---	if there are more than one motion blended, combine the influence of the others.
	if( m_nMMM >= 2 )
	{
		//---	there are 2 or more blended motions
		pMotionData	= &m_MMM[0];
		pEnd		= &m_MMM[m_nMMM];
		ASSERT((m_nMMM>=1)&&(m_nMMM<MAX_MOTION_BLEND_COUNT));

		while( ++pMotionData != pEnd )
			GetAnimBoneData( *pMotionData, Dest, pMotionData->BlendOrderRatio );
	}

	//==================
	AMSTATS_INTERP_END//==== PROFILING
	//==================

	return Dest;
}

//////////////////////////////////////////////////////////////////////////
inline
xbool mocap_player::GetAnimPropInfo( SPlaybackData&		rMotionData,
									 s32				PropIndex,
									 prop_def*			pPropDef,
									 vector3&			Pos,
									 URotData&			Rot,
									 f32				Blend )
{
    radian3     PropRot;
    vector3     PropPos;
	radian3		TempRot;
    s16*        DataPtr0;
    s16*        DataPtr1;
    f32         Frac;
    s32         PSO;
    radian3     Rot1,Rot0;
	quaternion	Quat0;
	quaternion	Quat1;
	quaternion	PropQuat;
	f32			qW;
	f32			Weight0;

	//
	//	if there is no set motion, return the prop base position
	//
	if( !rMotionData.pAnimInfo )
	{
		PropRot = pPropDef->Rot;
		PropPos	= pPropDef->Pos;

		//---	blend the prop values into the parameter as perscribed by the caller.
		if( Blend == 1.0f )
		{
			Rot._r	= PropRot;
			Pos		= PropPos;
		}
		else
		{
			BlendRadian3( TempRot, Rot._r, PropRot, Blend );
			Rot._r		= TempRot;
			Pos		= Pos*(1.0f - Blend) + PropPos*Blend;
		}

		return TRUE;
	}

    ASSERT(rMotionData.pAnimInfo);
    ASSERT((PropIndex>=0)&&(PropIndex<ANIM_MAX_PROPS));
    ASSERT((rMotionData.CurrentExportFrame>=rMotionData.Block.Frame0) && 
           (rMotionData.CurrentExportFrame<=rMotionData.Block.FrameN));
	ASSERT(pPropDef);

	if( rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_QUATERNION )
	{
		//---	if the prop has streamed data, interpolate the prop position/rotation from the stream, otherwise get it from the
		//		prop definition.
		if( rMotionData.pAnimInfo->PropsStreamsPresent & (1<<PropIndex) )
		{
			// Get ptrs to the frames
			GetFrameDataPtrs( rMotionData, &DataPtr0, &DataPtr1, &Frac );
			PSO = rMotionData.PropStream[PropIndex];

			if( rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_QUAT_W )
			{
				PropPos.X = ((f32)DataPtr0[PSO+4] + Frac*(DataPtr1[PSO+4] - DataPtr0[PSO+4])) * VALUE_TO_INCHES;
				PropPos.Y = ((f32)DataPtr0[PSO+5] + Frac*(DataPtr1[PSO+5] - DataPtr0[PSO+5])) * VALUE_TO_INCHES;
				PropPos.Z = ((f32)DataPtr0[PSO+6] + Frac*(DataPtr1[PSO+6] - DataPtr0[PSO+6])) * VALUE_TO_INCHES;

				//---	load the two quaternion values
				Quat0.X		= (f32)((u16*)DataPtr0)[PSO+0]*rMotionData.ValueToQuat - 1.0f;
				Quat0.Y		= (f32)((u16*)DataPtr0)[PSO+1]*rMotionData.ValueToQuat - 1.0f;
				Quat0.Z		= (f32)((u16*)DataPtr0)[PSO+2]*rMotionData.ValueToQuat - 1.0f;
				Quat0.W		= (f32)((u16*)DataPtr0)[PSO+3]*rMotionData.ValueToQuatW;
				Quat1.X		= (f32)((u16*)DataPtr1)[PSO+0]*rMotionData.ValueToQuat - 1.0f;
				Quat1.Y		= (f32)((u16*)DataPtr1)[PSO+1]*rMotionData.ValueToQuat - 1.0f;
				Quat1.Z		= (f32)((u16*)DataPtr1)[PSO+2]*rMotionData.ValueToQuat - 1.0f;
				Quat1.W		= (f32)((u16*)DataPtr1)[PSO+3]*rMotionData.ValueToQuatW;
			}
			else
			{
				PropPos.X = ((f32)DataPtr0[PSO+3] + Frac*(DataPtr1[PSO+3] - DataPtr0[PSO+3])) * VALUE_TO_INCHES;
				PropPos.Y = ((f32)DataPtr0[PSO+4] + Frac*(DataPtr1[PSO+4] - DataPtr0[PSO+4])) * VALUE_TO_INCHES;
				PropPos.Z = ((f32)DataPtr0[PSO+5] + Frac*(DataPtr1[PSO+5] - DataPtr0[PSO+5])) * VALUE_TO_INCHES;

				//---	load the two quaternion values
				Quat0.X		= (f32)((u16*)DataPtr0)[PSO+0]*rMotionData.ValueToQuat - 1.0f;
				Quat0.Y		= (f32)((u16*)DataPtr0)[PSO+1]*rMotionData.ValueToQuat - 1.0f;
				Quat0.Z		= (f32)((u16*)DataPtr0)[PSO+2]*rMotionData.ValueToQuat - 1.0f;
				Quat1.X		= (f32)((u16*)DataPtr1)[PSO+0]*rMotionData.ValueToQuat - 1.0f;
				Quat1.Y		= (f32)((u16*)DataPtr1)[PSO+1]*rMotionData.ValueToQuat - 1.0f;
				Quat1.Z		= (f32)((u16*)DataPtr1)[PSO+2]*rMotionData.ValueToQuat - 1.0f;

				qW				= 1.0f - (Quat0.X*Quat0.X + Quat0.Y*Quat0.Y + Quat0.Z*Quat0.Z);
				if( qW > 0.0f )	Quat0.W	= (f32)x_sqrt( qW );
				else			Quat0.W	= 0.0f;

				qW				= 1.0f - (Quat1.X*Quat1.X + Quat1.Y*Quat1.Y + Quat1.Z*Quat1.Z);
				if( qW > 0.0f )	Quat1.W = (f32)x_sqrt( qW );
				else			Quat1.W = 0.0f;
			}

			PropQuat	= BlendQuaternion( Quat0, Quat1, Frac );
		}
		else
		{
			//---	the prop has no streamed, data so just grab it streight from the prop definition.
			PropQuat	= pPropDef->Quat;
			PropPos		= pPropDef->Pos;
		}

		//---	blend the prop values into the parameter as perscribed by the caller.
		if( Blend == 1.0f )
		{
			Rot._q		= PropQuat;
			Pos			= PropPos;
		}
		else
		{
			Weight0		= 1.0f - Blend;
			Rot._q		= BlendQuaternion( Rot._q, PropQuat, Blend );
			Pos			= Pos*Weight0 + PropPos*Blend;
		}
	}
	else
	{
		//---	if the prop has streamed data, interpolate the prop position/rotation from the stream, otherwise get it from the
		//		prop definition.
		if( rMotionData.pAnimInfo->PropsStreamsPresent & (1<<PropIndex) )
		{
			// Get ptrs to the frames
			GetFrameDataPtrs( rMotionData, &DataPtr0, &DataPtr1, &Frac );
			PSO = rMotionData.PropStream[PropIndex];

			PropPos.X = ((f32)DataPtr0[PSO+3] + Frac * (DataPtr1[PSO+3] - DataPtr0[PSO+3])) * VALUE_TO_INCHES;
			PropPos.Y = ((f32)DataPtr0[PSO+4] + Frac * (DataPtr1[PSO+4] - DataPtr0[PSO+4])) * VALUE_TO_INCHES;
			PropPos.Z = ((f32)DataPtr0[PSO+5] + Frac * (DataPtr1[PSO+5] - DataPtr0[PSO+5])) * VALUE_TO_INCHES;

			Rot0.Pitch	= (f32)(DataPtr0[PSO+0]) * rMotionData.AngleToRadians;
			Rot0.Yaw	= (f32)(DataPtr0[PSO+1]) * rMotionData.AngleToRadians;
			Rot0.Roll	= (f32)(DataPtr0[PSO+2]) * rMotionData.AngleToRadians;
			Rot1.Pitch	= (f32)(DataPtr1[PSO+0]) * rMotionData.AngleToRadians;
			Rot1.Yaw	= (f32)(DataPtr1[PSO+1]) * rMotionData.AngleToRadians;
			Rot1.Roll	= (f32)(DataPtr1[PSO+2]) * rMotionData.AngleToRadians;

			//---	calculate the prop roation
			if (rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_PROPQUATBLEND)
				PropRot = BlendUsingQuaternions( Rot0, Rot1, Frac );
			else
				BlendRadian3( PropRot, Rot0, Rot1, Frac );
		}
		else
		{
			PropRot = pPropDef->Rot;
			PropPos	= pPropDef->Pos;
		}

		//---	blend the prop values into the parameter as perscribed by the caller.
		if( Blend == 1.0f )
		{
			Rot._r	= PropRot;
			Pos		= PropPos;
		}
		else
		{
			if (rMotionData.pAnimInfo->Flags & ANIMINFO_FLAG_PROPQUATBLEND)
				Rot._r	= BlendUsingQuaternions( Rot._r, PropRot, Blend );
			else
			{
				BlendRadian3( TempRot, Rot._r, PropRot, Blend );
				Rot._r		= TempRot;
			}

			Weight0	= 1.0f - Blend;
			Pos		= Pos*Weight0 + PropPos*Blend;
		}
	}

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
xbool mocap_player::GetPropData( s32				PropIndex,
								 SPropData&			rPropData )
{
	SPlaybackData*	pMotionData;
	SPlaybackData*	pEnd;
	vector3			vPos;
	URotData		Rot;
	prop_def*		pPropDef;

	ASSERT((PropIndex>=0) && (PropIndex<ANIM_MAX_PROPS));
	
	pPropDef = &m_AnimGroup->PropDefs[PropIndex];
	if( !pPropDef->Flags&prop_def::ACTIVE )
		return FALSE;

	//========================
	AMSTATS_PROPINTERP_START//==== PROFILING
	//========================

	VERIFY(GetAnimPropInfo( m_MMM[0], PropIndex, pPropDef, vPos, Rot, 1.0f ));

	//---	if there is only one motion, get the data and go
	if( m_nMMM > 1 )
	{
		//---	there are 2 or more blended motions
		pMotionData	= &m_MMM[0];
		pEnd		= &m_MMM[m_nMMM];
		ASSERT((m_nMMM>=1)&&(m_nMMM<MAX_MOTION_BLEND_COUNT));

		while( ++pMotionData != pEnd )
			VERIFY(GetAnimPropInfo( *pMotionData, PropIndex, pPropDef, vPos, Rot, pMotionData->BlendOrderRatio ));
	}

	//---	fill out the rPropData structure
	rPropData.Pos		= vPos;

    if( m_MMM[0].pAnimInfo->Flags & ANIMINFO_FLAG_QUATERNION )
	    rPropData.Rot._q	= Rot._q;
	else
	    rPropData.Rot._r	= Rot._r;
	
	rPropData.pPropDef	= pPropDef;

	//=======================
	AMSTATS_PROPINTERP_END;//==== PROFILING
	//=======================

	return TRUE;
}

//==========================================================================
void mocap_player::PropDataToPropPosRot( SPropData&		rData,
										 xbool			bBoneRelative,
										 const matrix4&	rBoneMat,
										 vector3&		Pos,
										 radian3&		Rot )
{
	vector3 vPos;
	radian3	vRot;
	s32		PropFlags		= (s32)rData.pPropDef->Flags;

	//---	get the rotation from the rotation data
	if( m_AnimFlags & ANIMINFO_FLAG_QUATERNION )
		QuatToRot( rData.Rot._q, vRot );
	else
		vRot = rData.Rot._r;

	//---	get the position information
	vPos = rData.Pos;
	if( !bBoneRelative )
		vPos -= m_CycleRawPos;

	//---	mirror the position if mirrored
	if( m_bMirror && (PropFlags&prop_def::MIRRORWITHANIM) )
	{
		vPos.X		= -vPos.X;
		vRot.Yaw	= -vRot.Yaw;
		vRot.Roll	= -vRot.Roll;
	}

	if( PropFlags&prop_def::SCALED )
		vPos *= m_WorldScale;

	//---	if the prop isn't relative to any bone, get the prop pos/rot relative to the player's root node and
	//		rotate the pos/rot about that location
	if( !bBoneRelative )
	{
		if( PropFlags&prop_def::STATIC_POS )
		{
			//---	if the position is relative to the character's facing, apply the facing now
			if( PropFlags&prop_def::FACEREL_POS )
				vPos.RotateY( m_WorldRootDir );
		}
		else
		{
			//---	rotate the position which should be relative to the player's root node, around the player
			vPos.RotateY( m_WorldYaw );
		}

		//---	we must move the position to the world location of the mocap_player.
		vPos += m_WorldPos;

		if( PropFlags&prop_def::STATIC_ROT )
		{
			//---	if the rotation is relative to the character's facing, apply the facing now
			if( PropFlags&prop_def::FACEREL_ROT )
				vRot.Yaw += m_WorldRootDir;
		}
		else
		{
			vRot.Yaw += m_WorldYaw;
		}

		//---	convert the rotation to ZXY order
	//	RotXZYToRotZXY( vRot, vRot );

		Pos = vPos;
		Rot = vRot;

		return;
	}

	//---	we are keeping position and rotation separate so that the game application can decide whether and how
	//		to scale the prop independantly of the shape and size of the mocap_player.  Props such as a baseball bat
	//		would need to have their position scaled, but wouldn't be scaled themselves while props such as hair would
	//		scale in size as well as positionally.

	if( PropFlags&prop_def::STATIC_POS && rData.pPropDef->Flags&prop_def::STATIC_ROT )
	{
		//---	if the position is relative to the character's facing, apply the facing now
		if( PropFlags&prop_def::FACEREL_POS )
			vPos.RotateY( m_WorldRootDir );

		//---	we must move the position to the world location of the mocap_player.
		vPos += rBoneMat.GetTranslation();

		//---	if the rotation is relative to the character's facing, apply the facing now
		if( PropFlags&prop_def::FACEREL_ROT )
			vRot.Yaw += m_WorldRootDir;

	}
	else if( PropFlags&prop_def::STATIC_POS )
	{
		//---	if the position is relative to the character's facing, apply the facing now
		if( PropFlags&prop_def::FACEREL_POS )
			vPos.RotateY( m_WorldRootDir );

		//---	we must move the position to the world location of the mocap_player.
		vPos += rBoneMat.GetTranslation();

		//---	transform the rotation
#if 1 // two methods, the second one is faster, but I am not sure if it will always work
		matrix4 m;
		RotXZYToMat( m, vRot );
		m.Transform( rBoneMat );
		MatToRotXZY( rBoneMat, vRot );
#else
		radian3 r;
		MatToRotXZY( rBoneMat, r );
		vRot += r;
#endif
	}
	else if( PropFlags&prop_def::STATIC_ROT )
	{
		//---	transform the position only
		rBoneMat.Transform( vPos );
		
		//---	if the rotation is relative to the character's facing, apply the facing now
		if( PropFlags&prop_def::FACEREL_ROT )
			vRot.Yaw += m_WorldRootDir;
	}
	else // !rData.pPropDef->bStaticPos && !rData.pPropDef->bStaticRot
	{
		//---	put the position and rotation into matrix form
		matrix4 m;
		RotXZYToMat( m, vRot );
		m.Translate( vPos );
		
		//---	transform the position and rotation
		m.Transform( rBoneMat );

		//---	get the position and rotation values out of matrix form
		vPos = m.GetTranslation();
		MatToRotXZY( m, vRot );
	}

	Pos = vPos;
	Rot = vRot;
}


//==========================================================================
void mocap_player::PropDataToPropPosRot( SPropData&		rData,
										 vector3&		Pos,
										 radian3&		Rot )
{
	matrix4 BoneMatrix;
	s32		BoneID;
	xbool	bBoneRelative;

	ASSERT(m_AnimGroup);

	//---	get the bone's index, if the prop has a bone index, get the bone's matrix
	BoneID = rData.pPropDef->BoneID;
	if( BoneID != -1 )
	{
		//---	if the player is mirrored, the prop will be based on the mirrored bone matrix
		if( m_bMirror )
			BoneID = m_AnimGroup->Bone[BoneID].MirrorID;

		//---	get the bone's matrix
		GetBoneMatrix( BoneID, BoneMatrix );
		bBoneRelative	= TRUE;
	}
	else
		bBoneRelative	= FALSE;

	PropDataToPropPosRot( rData, bBoneRelative, BoneMatrix, Pos, Rot );
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
    ASSERT(rMotionData.pAnimInfo);
    ASSERT((PointIndex>=0)&&(PointIndex<8));
    ASSERT(rMotionData.pAnimInfo->PointsPresent & (1<<PointIndex));
    ASSERT(((s32)rMotionData.CurrentExportFrame>=rMotionData.Block.Frame0) && 
           ((s32)rMotionData.CurrentExportFrame<=rMotionData.Block.FrameN));

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
    if( m_bMirror )
    {
        P0.X = -P0.X;
    }

    // Scale based on player size
    P0 *= m_WorldScale;
    //V3_Scale(&P0,&P0,m_WorldScale);

    // Rotate into world
    P0.RotateY( m_WorldYaw );

    // Translate to world position
    P0 += m_WorldPos;

    // Return position
    *WorldPos = P0;
}
*/

///////////////////////////////////////////////////////////////////////////
void mocap_player::GetEventPos( s32                Index,
                                vector3&           EventPos)
{
    anim_event* EV;
    vector3     P;
    vector3     R;

    ASSERT((Index>=0) && (Index<m_NEventsQueued));

    EV = &m_AnimGroup->AnimEvent[ m_EventQueue[Index] + m_MMM[0].pAnimInfo->EventIndex ];
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
    if( m_bMirror )
    {
        P.X = -P.X;
    }

    // Scale to player scale
    P *= m_WorldScale;

    // Rotate Y by base rotation
    P.RotateY( m_WorldYaw );

    // Translate out to player's position
    P += m_WorldPos;

    EventPos = P;
}


///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

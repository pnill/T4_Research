///////////////////////////////////////////////////////////////////////////
//
// AM_PLAY.C
//
///////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_math.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "AM_Play.h"
#include "AM_MotionPlayer.h"
#include "AM_Skel.h"
#include "AM_Stats.h"

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
motion_player::motion_player()
{
	m_MocapFlags.All				= 0;
	m_MocapFlags.Ind.BlendingOn		= TRUE;

	m_BoneRotationData				= NULL;
	m_pBoneMatrices					= NULL;

	SetRotationsDirty( TRUE );
	SetMatricesDirty( TRUE );

	m_bQuatBlend					= FALSE;
	m_PropBlendOnFlags				= 0;

    m_nBlendFrames					= 0.0f;
    m_NextBlendAmount				= 0.0f;

	m_fRenderRootDir				= 0.0f;
	m_fRenderRootDirBlendTime		= 0.0f;
	m_fRenderRootDirLastTime		= 0.0f;

	//---	clear the prop data
	x_memset( m_PropData, 0, ANIM_MAX_PROPS*sizeof(SPropData) );

	//---	PrepBoneBlending() and PrepPropBlending() assume that these flags
	//		are in these locations when chosing a function to convert data
	//		from one type to another.
	ASSERT(ANIMINFO_FLAG_GLOBALROTS==1);
	ASSERT(ANIMINFO_FLAG_WORLDROTS==2);
	ASSERT(ANIMINFO_FLAG_QUATERNION==4);
}

///////////////////////////////////////////////////////////////////////////
motion_player::~motion_player()
{
	Kill();
}

//=============================================================================
void motion_player::Init( anim_group* pAnimGroup, xbool bStoreMatrices )
{
	//---	unless the number of bones has changed or no buffers
	//		have yet been allocated for the motion_player, don't
	//		allocate new buffers.
	if( (bStoreMatrices && !m_pBoneMatrices) ||
		!m_BoneRotationData ||
		!m_AnimGroup ||
		(m_AnimGroup->NBones != pAnimGroup->NBones) )
	{
		Kill();

		//---	allocate storage space for the rotation and matrix data.
		//		Because the matrix data is going to be DMAed often, align
		//		the data to improve performance.  Rather than creating a
		//		new member variable to store the alloced address for the
		//		bone matrices, lets use the rotation data pointer to hold
		//		the allocated buffer for both rotation data and matrix data.
		//
		const s32 ALIGN_SIZE			= 64;	// aligning the data in RAM will improve (and/or permit)
												// DMA transfers on some target platforms specifically the PS2.
		s32 RotationDataSize			= pAnimGroup->NBones*sizeof( URotData );
		s32	MatrixDataSize				= 0;
		
		if( bStoreMatrices )
			MatrixDataSize	= pAnimGroup->NBones*sizeof( matrix4 ) + ALIGN_SIZE;

		u8*	pData						= (u8*) x_malloc( RotationDataSize+MatrixDataSize );
		x_memset( pData, 0, RotationDataSize+MatrixDataSize );

		m_BoneRotationData				= (URotData*) pData;

		if( bStoreMatrices )
		{
			m_pBoneMatrices					= bStoreMatrices ? (matrix4*) &pData[RotationDataSize] : NULL;
			if( (u32)m_pBoneMatrices & (ALIGN_SIZE-1) )
				m_pBoneMatrices				= (matrix4*) (((u32)m_pBoneMatrices) + ALIGN_SIZE - (((u32)m_pBoneMatrices)&(ALIGN_SIZE-1)));
		}
		else
			m_pBoneMatrices	= NULL;
	}

	ASSERT(m_BoneRotationData);
	ASSERT(!bStoreMatrices || m_pBoneMatrices);

	//
	//	initilalize the mocap player portion of the player
	//
	mocap_player::Init( pAnimGroup );

	//
	//	initialize the player into the zero pose
	//
	SetZeroPose();
}

//=============================================================================
void motion_player::Kill( void )
{
	mocap_player::Kill();

	if( m_BoneRotationData )
		x_free( m_BoneRotationData );

	m_pBoneMatrices		= NULL;
	m_BoneRotationData	= NULL;
}

//=============================================================================
void motion_player::SetAnimation( s32 AnimID, xbool bMirror, xbool bBlend )
{
	//=====================
	AMSTATS_SETANIM_START//====  PROFILING
	//=====================

	//---	remember if the last animation had a prop
	u32			LastAnimProps	= m_MMM[0].pAnimInfo ? m_MMM[0].pAnimInfo->PropsStreamsPresent : 0;
	anim_info*	pOldAnimInfo	= m_MMM[0].pAnimInfo;
	xbool		MirrorBlend		= ( !m_bMirror != !bMirror ) ? TRUE : FALSE;

	//---	set the new motion into the motion player
	mocap_player::SetAnimation( AnimID, bMirror );
	
	//---	if either the last animation or the next animation have prop streams, turn on prop blending
	//		for that prop.
	m_PropBlendOnFlags = m_MMM[0].pAnimInfo ? (LastAnimProps | m_MMM[0].pAnimInfo->PropsStreamsPresent) : 0;

	//---   
	// 
	// prepare the animation for blending
	// 
	//---   
	if( pOldAnimInfo )
	{
		URotData BoneRots[50];
		ASSERT(GetNumBones()<50);
		GetBoneData( BoneRots );

		f32 BestBlendFrames = PrepBlending( m_BoneRotationData,
											BoneRots,
											m_PropData,
											m_AnimFlags, //m_MMM[0].pAnimInfo->Flags,
											pOldAnimInfo->Flags,
											MirrorBlend,
											FALSE );

		if( m_MocapFlags.Ind.BlendingOn && m_MocapFlags.Ind.DefaultBlendingOn )
			m_nBlendFrames = BestBlendFrames;
	}

	SetRotationsDirty( TRUE );

	//===================
	AMSTATS_SETANIM_END//====  PROFILING
	//===================
}

//==================================================================================
void motion_player::AdvNFrames( f32              NFrames,
								vector3&         NewWorldPos,
								radian&          NewWorldRootDir,
								radian&          NewWorldMoveDir,
								radian&          NewWorldTotalMoveDir)
{
	//========================
	AMSTATS_ADVNFRAMES_START//====	PROFILING
	//========================

	//---	advance the actual motion
	mocap_player::AdvNFrames( NFrames,
							   NewWorldPos,
							   NewWorldRootDir,
							   NewWorldMoveDir,
							   NewWorldTotalMoveDir );

	//---	we must update the bone rotations if we are currently blending
	SetRotationsDirty( TRUE );

	if( m_nBlendFrames > 0.0f )
	{
		//---	calculate the blend amount
		IncrementBlending( ABS(NFrames) );
		UpdateBoneRotationData();

		NewWorldPos.Y = m_WorldPos.Y + (NewWorldPos.Y - m_WorldPos.Y)*m_NextBlendAmount;
	}

	//========================
	AMSTATS_ADVNFRAMES_END//====	PROFILING
	//========================
}

//=========================================================================
void motion_player::JumpToFrame( f32 DestFrame, xbool bCalcSetBlend )
{
	//========================
	AMSTATS_JUMPFRAME_START//====	PROFILING
	//========================

    ASSERT(m_AnimGroup);
	ASSERT((DestFrame < 10000.0f) && (DestFrame > -10000.0f));

	//---	if the anim object is not in an animation, return now
	if( m_MMM[0].pAnimInfo == NULL )
		return;

    //---	Advance the animation
	mocap_player::JumpToFrame( DestFrame );

	//---	set the advanced blending ammount
	SetRotationsDirty( TRUE );

	if( m_MocapFlags.Ind.DefaultBlendingOn && bCalcSetBlend )
	{
		URotData BoneRots[50];
		ASSERT(GetNumBones()<50);
		GetBoneData( BoneRots );

		f32 BestBlendFrames = PrepBlending( m_BoneRotationData,
											BoneRots,
											m_PropData,
											m_AnimFlags, //m_MMM[0].pAnimInfo->Flags,
											m_AnimFlags, //m_MMM[0].pAnimInfo->Flags,
											FALSE,
											FALSE );

		if( m_MocapFlags.Ind.BlendingOn && m_MocapFlags.Ind.DefaultBlendingOn )
			m_nBlendFrames = BestBlendFrames;

		IncrementBlending( 0.0f );
	}

	//========================
	AMSTATS_JUMPFRAME_END//====	PROFILING
	//========================
}

//=========================================================================
static URotData     s_BoneRots[50];
static SPropData    s_PropData[ANIM_MAX_PROPS];
xbool motion_player::UpdateBoneRotationData( void )
{
	ASSERT(m_AnimGroup->NBones <= 50);

	//---	if the bone rotations are not dirty, return now
	if( !GetRotationsDirty() )
		return FALSE;

	/////////////////////////////////////////////////////////////////////////
	// CHECK IF WE SHOULD BLEND
	/////////////////////////////////////////////////////////////////////////
	
    if( m_nBlendFrames != 0.0f )
    {
		////////////////////////////////////////////////////////////////////
		// BLEND OLD FRAME INTO NEW FRAME
		////////////////////////////////////////////////////////////////////
		mocap_player::GetBoneData( s_BoneRots );
		BlendBones( m_BoneRotationData, s_BoneRots, m_NextBlendAmount );

		if( m_PropBlendOnFlags )
		{
			GetPropData( s_PropData, ANIM_MAX_PROPS );
			BlendProps( m_PropData, s_PropData, m_NextBlendAmount );
		}
		else
			GetPropData( m_PropData, ANIM_MAX_PROPS );

		//---   
		// 
		// In debug we can draw the axis if desired
		// 
		//---   
//		SE_DEBUG( UpdateBoneRots_DrawDebugAxis( BoneRots ); )
    }
	else
	{
		mocap_player::GetBoneData( m_BoneRotationData );
		GetPropData( m_PropData, ANIM_MAX_PROPS );
	}

	//---	clear the dirty flag for the bone rotations, and set the dirty flag for the bone matrices
	SetRotationsDirty( FALSE );

	return TRUE;
}


//==========================================================================
//
//	GetRenderFacing
//
//		Returns the direction that the motion should be faced when rendering.
//	if the motion data was exported using motion relative facing, the world yaw
//	of the player will need to be returned.  The world yaw doesn't change during
//	the full motion playback unless the application wants to rotate the player.  If
//	on the other hand, the frames of the motion were exported without facing information
//	then the players root direction must be added to each frame.
//
//==========================================================================
radian motion_player::GetRenderFacing( void )
{
#ifdef ROOT_DIR_BLENDING

	f32 dT;
	f32	dR;

	//---	if the current animation is based on world rotations, the motions base Yaw value
	//		is used to convert its rotations into world rotations.
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_WORLDROTS )
		return GetWorldYaw();

	//---	blend the root node over time
	if( m_fRenderRootDirBlendTime > 0.0f )
	{
		dT = Time() - m_fRenderRootDirLastTime;
		if( dT >= m_fRenderRootDirBlendTime )
		{
			m_fRenderRootDirBlendTime	= 0.0f;
			m_fRenderRootDir			= GetWorldRootDir();
		}
		else if( dT == 0.0f )
		{
			return m_fRenderRootDir;
		}
		else
		{
			dR = GetWorldRootDir() - m_fRenderRootDir;
			while( dR < -R_180 )	dR += R_360;
			while( dR > R_180 )		dR -= R_360;
			m_fRenderRootDir			= m_fRenderRootDir + dR*dT/m_fRenderRootDirBlendTime;
			m_fRenderRootDirBlendTime	-= dT;
		}
	}
	else
		m_fRenderRootDir	= GetWorldRootDir();

	m_fRenderRootDirLastTime = Time();

	return m_fRenderRootDir;

#else

	return mocap_player::GetRenderFacing();

#endif
}


//void QuatToRot( const quaternion& Q, radian3& R );

//==========================================================================
void motion_player::GetBoneOrient( s32 BoneIndex, radian3& rBoneRot )
{
    ASSERT(m_AnimGroup);
    ASSERT((BoneIndex>=0) && (BoneIndex<m_AnimGroup->NBones));

	// get this function to work off of the AMSKL_GetBoneOrient() function for speed
#if 1
	matrix4 M;
	GetBoneMatrix( BoneIndex, M );
	MatToRotXZY( M, rBoneRot );
#else

    // Load rotations into temp buffer
	URotData RotData;
	AMSKL_GetBoneOrient( m_AnimGroup,
						 RotData,
						 BoneIndex,
						 GetBoneData(),
						 GetBoneDataFormat(),
						 m_bMirror,
						 GetRenderFacing(),
						 m_bTransform ? &m_Transform : NULL );

	if( GetAnimInfoFlags()&ANIMINFO_FLAG_QUATERNION )
		QuatToRot( RotData._q, rBoneRot  );
	else
		rBoneRot = RotData._r;
#endif
}

//==========================================================================
void motion_player::GetBoneOrient( s32 BoneIndex, quaternion& rBoneQuat )
{
//	radian3		TempRot;

    ASSERT(m_AnimGroup);
    ASSERT((BoneIndex>=0) && (BoneIndex<m_AnimGroup->NBones));

	// get this function to work off of the AMSKL_GetBoneOrient() function for speed
#if 1

	matrix4 M;
	GetBoneMatrix( BoneIndex, M );
	rBoneQuat.SetupMatrix( M );

#else

    // Load rotations into temp buffer
	URotData RotData;
	AMSKL_GetBoneOrient( m_AnimGroup,
						 RotData,
						 BoneIndex,
						 GetBoneData(),
						 GetBoneDataFormat(),
						 m_bMirror,
						 GetRenderFacing(),
						 m_bTransform ? &m_Transform : NULL );

	if( GetAnimInfoFlags()&ANIMINFO_FLAG_QUATERNION )
		rBoneQuat = RotData._q;
	else
		RotToQuat( RotData._r, rBoneQuat );

#endif
}

//==========================================================================
// *** FUNCTION NOT YET TESTED
void motion_player::SetBoneOrient( s32 BoneIndex, const radian3& rBoneRot )
{
	if( GetAnimInfoFlags()&ANIMINFO_FLAG_GLOBALROTS )
	{
		radian3 r;
		r.Pitch	= rBoneRot.Pitch;
		r.Yaw	= rBoneRot.Yaw - GetRenderFacing();
		r.Roll	= rBoneRot.Roll;

		if( IsMirrored() )
		{
			r.Yaw	= -r.Yaw;
			r.Roll	= -r.Roll;
			SetBoneRot( m_AnimGroup->Bone[BoneIndex].MirrorID, r );
		}
		else
			SetBoneRot( BoneIndex, r );
	}
	else
	{
		quaternion BoneOrient;
		quaternion ParentOrient;
		
		RotXZYToQuat( rBoneRot, BoneOrient );
		GetBoneOrient( m_AnimGroup->Bone[BoneIndex].ParentID, ParentOrient );

		BoneOrient = (-ParentOrient) * BoneOrient;

		if( IsMirrored() )
		{
			MirrorQuat( BoneOrient, BoneOrient );
			SetBoneQuat( m_AnimGroup->Bone[BoneIndex].MirrorID, BoneOrient );
		}
		else
			SetBoneQuat( BoneIndex, BoneOrient );
	}
}

//==========================================================================
void motion_player::SetBoneOrient( s32 BoneIndex, const quaternion& rBoneRot )
{
	ASSERTS(0, "Not yet implemented, use the SetBoneOrient() function for radian3 to get this one going" );
}


#if 0
//==========================================================================
// Function:	GetMaxYPRDif ( GetMaxYawPitchRollDif )
// 
// Description: Compares the yaw, pitch and roll of two radian3s and returns
//				the absolute value of the one determined to have the greatest 
//				difference
//==========================================================================
inline radian GetMaxYPRDif ( radian3* From, radian3* To )
{
	radian DR;
	radian MaxYPRDif ( 0.0f );

	DR = DirDif( From->Pitch, To->Pitch );
	DR = ABS(DR);
	if( DR > MaxYPRDif )
		MaxYPRDif = DR;

	DR = DirDif( From->Yaw, To->Yaw );
	DR = ABS(DR);
	if( DR > MaxYPRDif )
		MaxYPRDif = DR;	

	DR = DirDif( From->Roll, To->Roll );
	DR = ABS(DR);
	if( DR > MaxYPRDif )
		MaxYPRDif = DR;

	return MaxYPRDif;
}
#endif


void motion_player::ConvertData( URotData* A, SPropData* pPropData, u32 AFlags, u32 BFlags )
{
		ASSERT(0); // not yet implemented
#if 0
		//---	convert between quaternian and radain3
		if( (AFlags&ANIMINFO_FLAG_QUATERNION) != (BFlags&ANIMINFO_FLAG_QUATERNION) )
		{
			if( BFlags&ANIMINFO_FLAG_QUATERNION )
				PrepBlend_RotToQuat( A, pPropData );
			else
				PrepBlend_QuatToRot( A, pPropData );
		}

		//---	convert between local and global rotations if required
		if( (AFlags&ANIMINFO_FLAG_GLOBALROTS) != (BFlags&ANIMINFO_FLAG_GLOBALROTS) )
		{
			if( BFlags&ANIMINFO_FLAG_GLOBALROTS )
				PrepBlend_LocalToGlobal( A, pPropData );
			else
				PrepBlend_GlobalToLocal( A, pPropData );
				
		}

		//---	convert between world and frame yaw
		if( (AFlags&ANIMINFO_FLAG_WORLDROTS) != (BFlags&ANIMINFO_FLAG_WORLDROTS) )
		{
			if( BFlags&ANIMINFO_FLAG_WORLDROTS )
				PrepBlend_FrameToWorld( A, pPropData );
			else
				PrepBlend_WorldToFrame( A, pPropData );
		}
#endif
}


//==========================================================================
//
//	PrepBlending
//
//		Changes the bone rotations from animation A to be reletive to the bone
//	rotations for animation B.  It can also calculate a good blend value be
//	determining the maximum difference in any bone as it is processing.
//
//		If the bone rotations are in quaternions/euler, world/frame, local/global
//	doesn't matter.  The old data will be converted to work in the new animation's
//	system.
//
//	void* A is the destination buffer to hold the converted bone rotations.
//	void* B is the buffer containing the old bone rotations in their old format.
//	s32	AFlags are the flags which describe the desired format for the output bone rotations.
//	s32 BFlags are the flags which describe the old bone rotation format.
//
//	return    f32  - amount of blending (only returned if CalcBestBlend == TRUE)
//
//==========================================================================
f32 motion_player::PrepBlending( URotData* A, URotData* B, SPropData* pPropData, u32 AFlags, u32 BFlags, xbool Mirror, xbool CalcBestBlend )
{
	//=======================
	AMSTATS_PREPBLEND_START//==== PROFILING
	//=======================

	radian		OldYawToNewYaw;
	anim_bone*	pBone;
	radian3*	pMirRot;
	radian3		TempRot;
	quaternion*	pMirQuat;
	quaternion	TempQuat;
	URotData*	pA;
	URotData*	pB;
	URotData*	pEnd;

	//---	convert the format
	if( (AFlags&(ANIMINFO_FLAG_QUATERNION|ANIMINFO_FLAG_WORLDROTS|ANIMINFO_FLAG_GLOBALROTS)) !=
		(BFlags&(ANIMINFO_FLAG_QUATERNION|ANIMINFO_FLAG_WORLDROTS|ANIMINFO_FLAG_GLOBALROTS)) )
	{
		ConvertData( A, pPropData, AFlags, BFlags );
	}

    ////////////////////////////////////////////////////////////////////////
	//	INITIALIZE THE STEPPING POINTERS
    ////////////////////////////////////////////////////////////////////////
	pA		= A;
	pB		= B;
	pEnd	= &pA[GetNumBones()];

	if( AFlags&ANIMINFO_FLAG_QUATERNION )
	{
		if( AFlags&ANIMINFO_FLAG_WORLDROTS )
		{
			//---   
			// 
			// Determine the value necessary to transform the bones of the old anim
			// into the space of the new animation
			// 
			//---   
			if( Mirror )	OldYawToNewYaw = QuatToYaw( pB->_q ) + QuatToYaw( pA->_q );
			else			OldYawToNewYaw = QuatToYaw( pB->_q ) - QuatToYaw( pA->_q );

			if( AFlags&ANIMINFO_FLAG_GLOBALROTS )
			{
				//===	EULER/WORLD/GLOBAL

				if( Mirror )
				{
					//===	EULER/WORLD/GLOBAL/MIRROR

					//---   
					// 
					// Get a pointer to the bones array for mirroring
					// 
					//---   
					pBone = &m_AnimGroup->Bone[0];

					////////////////////////////////////////////////////////////////////////
					// FOR EACH BONE GET THE DELTA BLEND AND COMPARE IT WITH THE FINAL ROTATION
					////////////////////////////////////////////////////////////////////////
					for (; pA != pEnd; ++pA, ++pBone )
					{
						//---	mirror the bone
						if( pBone->BoneID < pBone->MirrorID )
						{
							pMirQuat	= &A[pBone->MirrorID]._q;
							TempQuat	= pA->_q;
							pA->_q		= *pMirQuat;
							*pMirQuat	= TempQuat;
						}

						//---	Mirror the old animation to fit the required blend into the new animation
						MirrorQuat( pA->_q, pA->_q );

						//---   
						// 
						// Update the current bones yaw to the space of the new anims yaw
						// 
						//---   
						pA->_q.RotateY( OldYawToNewYaw );
					}
				}
				else
				{
					//===	EULER/WORLD/GLOBAL/NO MIRROR

					////////////////////////////////////////////////////////////////////////
					// FOR EACH BONE GET THE DELTA BLEND AND COMPARE IT WITH THE FINAL ROTATION
					////////////////////////////////////////////////////////////////////////
					for (; pA != pEnd; ++pA )
					{
						//---   
						// 
						// Update the current bones yaw to the space of the new anims yaw
						// 
						//---   
						pA->_q.RotateY( OldYawToNewYaw );
					}
				}
			}
			else // LOCAL ROTS
			{
				//===	EULER/WORLD/LOCAL

				if( Mirror )
				{
					//---   
					// 
					// Get a pointer to the bones array for mirroring
					// 
					//---   
					pBone = &m_AnimGroup->Bone[0];

					//---	initially calculate a value for the root yaw which will be correct
					//		once it is negated.
					MirrorQuat( pA->_q, pA->_q );
					pA->_q.RotateY( OldYawToNewYaw );
					++pA;
					++pBone;

					////////////////////////////////////////////////////////////////////////
					// FOR EACH BONE GET THE DELTA BLEND AND COMPARE IT WITH THE FINAL ROTATION
					////////////////////////////////////////////////////////////////////////
					for (; pA != pEnd; ++pA, ++pBone )
					{
						//---	mirror the bone
						if( pBone->BoneID < pBone->MirrorID )
						{
							pMirQuat	= &A[pBone->MirrorID]._q;
							TempQuat	= pA->_q;
							pA->_q		= *pMirQuat;
							*pMirQuat	= TempQuat;
						}

						MirrorQuat( pA->_q, pA->_q );
					}
				}
				else
				{
					//===	EULER/WORLD/LOCAL/NO MIRROR

					//---	get the old bones YAW to a value relative to the new motion bone's yaw
					pA->_q.RotateY( OldYawToNewYaw );
				}
			}
		}
		else
		{
			if( Mirror )
			{
				//---   
				// 
				// Get a pointer to the bones array for mirroring
				// 
				//---   
				pBone = &m_AnimGroup->Bone[0];

				////////////////////////////////////////////////////////////////////////
				// FOR EACH BONE GET THE DELTA BLEND AND COMPARE IT WITH THE FINAL ROTATION
				////////////////////////////////////////////////////////////////////////
				for (; pA != pEnd; ++pA, ++pBone )
				{
					//---	mirror the bone
					if( pBone->BoneID < pBone->MirrorID )
					{
						pMirQuat	= &A[pBone->MirrorID]._q;
						TempQuat	= pA->_q;
						pA->_q		= *pMirQuat;
						*pMirQuat	= TempQuat;
					}

					MirrorQuat( pA->_q, pA->_q );
				}
			}
			else
			{
				//===	EULER/FRAME/LOCAL/NO MIRROR

				//---	nothing to be done here.
			}
		}
	}
	else
	{
		if( AFlags&ANIMINFO_FLAG_WORLDROTS )
		{
			//---   
			// 
			// Determine the value necessary to transform the bones of the old anim
			// into the space of the new animation
			// 
			//---   
			if( Mirror )	OldYawToNewYaw = pB->_r.Yaw + pA->_r.Yaw;
			else			OldYawToNewYaw = pB->_r.Yaw - pA->_r.Yaw;

			if( AFlags&ANIMINFO_FLAG_GLOBALROTS )
			{
				//===	EULER/WORLD/GLOBAL

				if( Mirror )
				{
					//===	EULER/WORLD/GLOBAL/MIRROR

					//---   
					// 
					// Get a pointer to the bones array for mirroring
					// 
					//---   
					pBone = &m_AnimGroup->Bone[0];
					
					////////////////////////////////////////////////////////////////////////
					// FOR EACH BONE GET THE DELTA BLEND AND COMPARE IT WITH THE FINAL ROTATION
					////////////////////////////////////////////////////////////////////////
					for (; pA != pEnd; ++pA, ++pBone )
					{
						//---	mirror the bone
						if( pBone->BoneID < pBone->MirrorID )
						{
							pMirRot		= &A[pBone->MirrorID]._r;
							TempRot		= pA->_r;
							pA->_r		= *pMirRot;
							*pMirRot	= TempRot;
						}

						//---   
						// 
						// Update the current bones yaw to the space of the new anims yaw
						// 
						//---   
						pA->_r.Yaw	= OldYawToNewYaw - pA->_r.Yaw;
						pA->_r.Roll	= -pA->_r.Roll;	// Negated for mirroring
					}
				}
				else
				{
					//===	EULER/WORLD/GLOBAL/NO MIRROR

					////////////////////////////////////////////////////////////////////////
					// FOR EACH BONE GET THE DELTA BLEND AND COMPARE IT WITH THE FINAL ROTATION
					////////////////////////////////////////////////////////////////////////
					for (; pA != pEnd; ++pA )
					{
						//---   
						// 
						// Update the current bones yaw to the space of the new anims yaw
						// 
						//---   
						pA->_r.Yaw += OldYawToNewYaw;
					}
				}
			}
			else // LOCAL ROTS
			{
				//===	EULER/WORLD/LOCAL

				if( Mirror )
				{
					//===	EULER/WORLD/LOCAL/MIRROR
					pBone = &m_AnimGroup->Bone[0];

					//---	initially calculate a value for the root yaw which will be correct
					//		once it is negated.
					pA->_r.Yaw	= -(OldYawToNewYaw - pA->_r.Yaw);

					////////////////////////////////////////////////////////////////////////
					// FOR EACH BONE GET THE DELTA BLEND AND COMPARE IT WITH THE FINAL ROTATION
					////////////////////////////////////////////////////////////////////////
					for (; pA != pEnd; ++pA, ++pBone )
					{
						//---	mirror the bone if necessary
						if( pBone->BoneID < pBone->MirrorID )
						{
							pMirRot		= &A[pBone->MirrorID]._r;
							TempRot		= pA->_r;
							pA->_r	= *pMirRot;
							*pMirRot	= TempRot;
						}

						//---	get the old bones YAW to a value relative to the new motion bone's yaw
						pA->_r.Yaw	= -pA->_r.Yaw;
						pA->_r.Roll	= -pA->_r.Roll;
					}
				}
				else
				{
					//===	EULER/WORLD/LOCAL/NO MIRROR

					//---	get the old bones YAW to a value relative to the new motion bone's yaw
					pA->_r.Yaw += OldYawToNewYaw;
				}
			}
		}
		else
		{

			if( Mirror )
			{
				//===	EULER/FRAME/LOCAL/MIRROR
				pBone = &m_AnimGroup->Bone[0];
				
				////////////////////////////////////////////////////////////////////////
				// FOR EACH BONE GET THE DELTA BLEND AND COMPARE IT WITH THE FINAL ROTATION
				////////////////////////////////////////////////////////////////////////
				for (; pA != pEnd; ++pA, ++pBone )
				{
					//---	mirror the bone
					if( pBone->BoneID < pBone->MirrorID )
					{
						pMirRot		= &A[pBone->MirrorID]._r;
						TempRot		= pA->_r;
						pA->_r	= *pMirRot;
						*pMirRot	= TempRot;
					}

					//---	get the old bones YAW to a value relative to the new motion bone's yaw
					pA->_r.Yaw	= -pA->_r.Yaw;
					pA->_r.Roll	= -pA->_r.Roll;
				}
			}
			else
			{
				//===	EULER/FRAME/LOCAL/NO MIRROR

				//---	nothing to be done here.
			}

		}
	}

	//=====================
	AMSTATS_PREPBLEND_END//==== PROFILING
	//=====================
	return 0.0f;
}

#ifdef X_DEBUG
s32 MaxQuatBlends = 0;
#endif

radian3 BlendUsingQuaternions( const radian3& From, const radian3& To, f32 t );


//==========================================================================
//
//	BlendBones
//
//		Blends two sets of bone rotations by a specified amount. The blended
//		bones are stored in the AnimActor's m_pBoneRotations member variable.
// 
//==========================================================================
void motion_player::BlendBones( URotData* A, URotData* B, f32 BlendAmount )
{
	//===================
	AMSTATS_BLEND_START//==== PROFILING
	//===================

	URotData*	pA;
	URotData*	pB;
	URotData*	pEnd;


    ////////////////////////////////////////////////////////////////////////
    //  PARAMETER CHECKING
    ////////////////////////////////////////////////////////////////////////

    ASSERT(A);
    ASSERT(B);
    ASSERT((BlendAmount >= 0.0f) && (BlendAmount <= 1.0f));

    ////////////////////////////////////////////////////////////////////////
    //  BONE ROTS CLEAN!!
    ////////////////////////////////////////////////////////////////////////

    SetRotationsDirty( FALSE );

	//---	INITIALIZE THE STEPPING POINTERS
	pA		= A;
	pB		= B;
	pEnd	= &pA[GetNumBones()];

    ////////////////////////////////////////////////////////////////////////
    // FOR EACH BONE BLEND THE ROTATION FROM SNAPSHOT TO CURRENT 
    ////////////////////////////////////////////////////////////////////////
	if( m_AnimFlags & ANIMINFO_FLAG_QUATERNION )
	{
		for (; pA != pEnd; ++pA, ++pB )
			pA->_q	= BlendLinear( pA->_q, pB->_q, BlendAmount, FALSE );
	}
	else
	{
		//---	if it is quaternion blended, perform the slow blend
		if( m_AnimFlags & ANIMINFO_FLAG_QUATBLEND )
		{
			for (; pA != pEnd; ++pA, ++pB )
				pA->_r = BlendUsingQuaternions( pA->_r, pB->_r, BlendAmount );
		}
		else
		{
			for (; pA != pEnd; ++pA, ++pB )
				BlendRadian3( pA->_r, pA->_r, pB->_r, BlendAmount );
		}
	}

	//=================
	AMSTATS_BLEND_END//==== PROFILING
	//=================
}

//==========================================================================
void motion_player::BlendProps( SPropData* A, SPropData* B, f32 BlendAmount )
{
	//=======================
	AMSTATS_PROPBLEND_START//==== PROFILING
	//=======================

	s32	PropFlag = 1;
	SPropData*	pA;
	SPropData*	pB;
	SPropData*	pEnd;

	pA		= &A[0];
	pB		= &B[0];
	pEnd	= &A[ANIM_MAX_PROPS];

	while( pA != pEnd )
	{
		if( m_PropBlendOnFlags & PropFlag )
		{
			if( m_AnimFlags & ANIMINFO_FLAG_QUATERNION )
				pA->Rot._q = BlendLinear( pA->Rot._q, pB->Rot._q, BlendAmount, FALSE );
			else
			{
				if( m_AnimFlags & ANIMINFO_FLAG_QUATBLEND )
					pA->Rot._r = BlendUsingQuaternions( pA->Rot._r, pB->Rot._r, BlendAmount );
				else
					BlendRadian3( pA->Rot._r, pA->Rot._r, pB->Rot._r, BlendAmount );
			}

			pA->Pos += BlendAmount*(pB->Pos - pA->Pos);
		}
		else
		{
			pA->Rot._q	= pB->Rot._q;
			pA->Pos		= pB->Pos;
		}

		PropFlag <<= 1;
		++pA;
		++pB;
	}

	//======================
	AMSTATS_PROPBLEND_END;//==== PROFILING
	//======================
}


//==========================================================================
//
//	IncrementBlending
//
//		Increment the blending by an amount. This will turn off blending
//		automatically when the full number of blend frames has been reached.
// 
//==========================================================================
void motion_player::IncrementBlending( f32 IncrementAmount )
{
	//---	if blending is paused or if there is no current blending, return
	if (m_nBlendFrames == 0.0f)
		return;

	//---	make sure that the number of frames to increment the blending is positive
	ASSERT((IncrementAmount >= 0.0f) && (IncrementAmount < 1000.0f));

	//---	if the increment amount is zero, no blending increment
	if (IncrementAmount <= 0.0f)
	{
		m_NextBlendAmount = 0.0f;
		return;
	}

	//---	if the number of blend frames has been used up, return
	if (m_nBlendFrames <= IncrementAmount)
	{
		m_nBlendFrames = 0.0f;
		m_NextBlendAmount = 1.0f;
		return;
	}

	//---	calculate the amount of blending for the next update
	m_NextBlendAmount = (1.0f / m_nBlendFrames) * IncrementAmount;

	//---	set the new number of blend frames and set the fraction
	m_nBlendFrames    -= IncrementAmount;
}


///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////


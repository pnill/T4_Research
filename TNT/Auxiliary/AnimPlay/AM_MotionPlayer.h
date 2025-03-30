///////////////////////////////////////////////////////////////////////////
//
//  AM_MotionPlayer.hpp
//
//
//	FURTURE WORK IDEAS:
//		Props work well, but they are always exported from the CED reletive
//	to the world location.  Perhaps they should be exported reletive to a
//	chosen bone.  Then, blending in the game could move them appropriately.
//	Currently, when blending, props translations are linearly blended.  This
//	linear blend probably won't match the rotational displacement of the
//	skeletal structure out to the bone wo which the prop is supposed to be
//	visually tied.
//
///////////////////////////////////////////////////////////////////////////

#ifndef _AM_MOTIONPLAYER_HPP_
#define _AM_MOTIONPLAYER_HPP_

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "AM_MocapPlayer.h"
#include "AM_Skel.h"

//==========================================================================
//	Rotation data union (storing rotations as radian3 and quaternion
//==========================================================================

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
struct SMocapFlags
{
	u32	BlendingOn:1;
	u32	DefaultBlendingOn:1;
};

///////////////////////////////////////////////////////////////////////////
union UMocapFlags
{
	u32			All;
	SMocapFlags	Ind;
};

//==========================================================================
//	the class
//==========================================================================
class motion_player : public mocap_player
{
public:
	motion_player();
	virtual ~motion_player();

	//------------------------------------------------------------------------
	//	The Init function must be called before anything can be done with this
	//	motion_player.
	//
	//	anim_group*		pAnimGroup		- animation group used by this motion player.
	//	xbool			bStoreMatrices	- T/F should buffer space be allocated to
	//									  store the matrices so that they are only
	//									  recalculated when dirty?.
	//
	void				Init					( anim_group* pAnimGroup, xbool bStoreMatrices=FALSE );
	void				Kill					( void );

	void				SetZeroPose				( xbool bMirror=FALSE );

	void				SetAnimation			( s32	AnimID,
												  xbool	bMirror,
												  xbool bBlend=FALSE );

	void				AdvNFrames				( f32		NFrames,
												  vector3&	NewWorldPos,
												  radian&	NewWorldRootDir,
												  radian&	NewWorldMoveDir,
												  radian&	NewWorldTotalMoveDir);

	void				JumpToFrame				( f32		DestFrame,
												  xbool		bCalcSetBlend=FALSE );

	void				SetQuatBlending			( xbool Setting );
	void				SetQuatInterpolation	( xbool Setting );

	//---	rotations dirty
	xbool				GetRotationsDirty		( void );
	void				SetRotationsDirty		( xbool Setting );

	//---	matricies dirty
	xbool				GetMatricesDirty		( void );
	void				SetMatricesDirty		( xbool Setting );

	//---	
	void				SetWorldPosition		( const vector3& WorldPos );
	void				SetWorldScale			( f32 WorldScale );


	///////////////////////////////////////////////////////////////////////////
	// BONE ROTATION INFO
	///////////////////////////////////////////////////////////////////////////
	//
	//	NOTE:	For the following GetBoneRot()/GetBoneQuat()/GetBoneData() functions
	//			given data is based on the exported rotation data modified by any
	//			blending that may have been done on them.  It is not relative to
	//			the bone's final world orientation.  To get the bone's final world
	//			orientation call GetBoneOrient().
	//
	//	GetBoneRots() gets the bone rotation values from the motion data.  This
	//	might be calculation intensive if the data is in quaternion format, or
	//	quite simple if the data is in euler format.
	//
	//	GetBoneQuats() get the bone quaternions.  This might be trivial if the data
	//	is stored in quaternion format, or calculation intensive if the data is
	//	stored in euler format.
	//
	//	GetBoneData() gets the bone data which might be quaternion or rotation
	//	into the format that the game is using.
	//
	/////////////////////////////////////////////////////////////////////////////

	//---	get rotation data
	radian3&		GetBoneRot			( s32			Bone,
										  radian3&		Dest );
	radian3*		GetBoneRots			( radian3*		Dest );

	quaternion&		GetBoneQuat			( s32			Bone,
										  quaternion&	Dest );
	quaternion*		GetBoneQuats		( quaternion*	Dest );

	//---	get bone data (rotation or quaternion data, whichever is available)
	const URotData&	GetBoneData			( s32			Bone );
	URotData&		GetBoneData			( s32			Bone,
										  URotData&		Dest );

	const URotData*	GetBoneData			( void );
	URotData*		GetBoneData			( URotData*		Dest );

	//	set rotation data
	void			SetBoneRot			( s32				Bone,
										  const radian3&	Src );
	void			SetBoneRots			( const radian3*	Src );
	void			SetBoneQuat			( s32				Bone,
										  const quaternion&	Src );
	void			SetBoneQuats		( const quaternion*	Src );
	void			SetBoneData			( s32				Bone,
										  const URotData&	Src );
	void			SetBoneData			( const URotData*	Src );

	//==========================================================================
	//
	//	GET BONE POSITION, ROTATIONS, and MATRICES
    //
    //  A NOTE ABOUT COORDINATE FRAMES, ETC...
    //
    //  The Bone Positions are in world space and have already had WorldYaw,
    //  WorldPos, WorldScale, and mirroring applied to them.
    //
    //  The Bone Rotations are in world space (NOT relative to parent bone)
    //  and have already had WorldYaw and mirroring applied to them.
    //
    //  The Bone Matrices are in world space and have already had WorldYaw,
    //  WorldPos, WorldScale, and mirroring applied to them.
    //  They have NOT had individual bone scales applied, and for rendering
    //  purposes you will also want to translate them by -OriginToBone.
	//
	//==========================================================================

	void			GetBonePosition			( s32 BoneIndex, vector3& rBonePos );

	void			GetBoneOrient			( s32 BoneIndex, radian3& rBoneRot );
	void			GetBoneOrient			( s32 BoneIndex, quaternion& rBoneRot );

	//	function to set the rotation of a bone with world rotations
	void			SetBoneOrient			( s32 BoneIndex, const radian3& rBoneRot );
	void			SetBoneOrient			( s32 BoneIndex, const quaternion& rBoneRot );

	//	these versions work only if the motion_player is set to cache matrices
	const matrix4&	GetBoneMatrix			( s32 BoneIndex );
	const matrix4*	GetBoneMatrices			( void );

	//	these work regardless of caching
	matrix4&		GetBoneMatrix			( s32 BoneIndex, matrix4& rBoneL2W );
	matrix4*		GetBoneMatrices			( matrix4* pBoneL2Ws );

	s32				GetBoneIndex			( char* BoneName );
	s32				GetNumBones				( void );

	//----------------------------------------------------------------------
	// FUNCTIONS FOR UPDATING ROTATIONS /W BLENDING
	//----------------------------------------------------------------------

	//==========================================================================
	//
	//	SetBlendFrames
	//	GetBlendFrames
	//
	//		Sets the number of frames that the player should blend for.  This
	//		blending is one directional.  His current bone rotations are blended
	//		toward the current animation's bone rotations slowly over the number
	//		of frames given.
	//		
	// 
	//==========================================================================
	void			SetBlendFrames			( f32 BlendFrames );
	f32				GetBlendFrames			( void );

	void			SetBlendingOn			( xbool bSetting );
	void			SetDefaultBlendingOn	( xbool bSetting );

	xbool			UpdateBoneRotationData	( void );

	void			ConvertData				( URotData* A, SPropData* pPropData, u32 AFlags, u32 BFlags );
	f32				PrepBlending			( URotData* A, URotData* B, SPropData* pPropData, u32 AFlags, u32 BFlags, xbool Mirror, xbool CalcBestBlend );
    void			BlendBones				( URotData* A, URotData* B, f32 BlendAmount );
    void			BlendProps				( SPropData* A, SPropData* B, f32 BlendAmount );

    void			IncrementBlending		( f32 IncrementAmount );


	///////////////////////////////////////////////////////////////////////////
	// PROP INFORMATION
	///////////////////////////////////////////////////////////////////////////

	xbool			GetPropInfo				( s32				PropIndex,
											  vector3&			Pos,
											  radian3&			Rot );

	xbool			GetPropInfo				( vector3*			Pos,
											  radian3*			Rot,
											  s32				Len );

	//////////////////////////////////////////////////////////////////////////
	//	Global Transform functions
	//////////////////////////////////////////////////////////////////////////
	void			SetTransform			( const matrix4&	rTransform );
	void			ApplyTransform			( xbool bSetting=TRUE );

    //----------------------------------------------------------------------
    //  FUNCTIONS FOR ROOT DIRECTION BLENDING
    //----------------------------------------------------------------------
	void			SetBlendRootDirTime		( f32 Secs );

protected:
	//---	must render based on the RenderRootDir if root node blending is to be used.
	radian			GetRenderFacing			( void );


    //----------------------------------------------------------------------
    //  FUNCTIONS FOR BUILDING MATRICES
    //----------------------------------------------------------------------
	void			UpdateBoneMatricies		( void );

	void			BuildBoneMatrix			( s32 BoneIndex, matrix4& rBoneL2W );
	void			BuildBoneMatrices		( matrix4* pBoneL2Ws );

protected:
///////////////////////////////////////////////////////////////////////////
//	MEMBER DATA
///////////////////////////////////////////////////////////////////////////

	UMocapFlags			m_MocapFlags;

//    UBoneRotationData	m_BoneRotationData;			// Rotation values for bones
	URotData*			m_BoneRotationData;
    xbool				m_bBoneRotationsDirty;		// TRUE = update BoneRotations

	SPropData			m_PropData[ANIM_MAX_PROPS];	// prop data to be blended
						
	matrix4*			m_pBoneMatrices;			// for PS2 make sure that the buffer is alligned to 16 byte.
	xbool				m_bBoneMatricesDirty;		// bitset where each bit sais whether a bone is dirty

	//---   Linear Single-directional bone blending
    f32					m_nBlendFrames;				// Num of seconds to blend from the current place to the destination
	f32					m_NextBlendAmount;			// next amount to blend
	xbool				m_bQuatBlend;				// use quaternion blending
	u32					m_PropBlendOnFlags;			// flag is turned on for each prop which should be blended

	//---	RootDir blending
	f32					m_fRenderRootDir;			// world root dir to use for rendering.
	f32					m_fRenderRootDirBlendTime;	// number of frames remaining in the blend operation
	f32					m_fRenderRootDirLastTime;	// last time the render root dir blending was updated
};





///////////////////////////////////////////////////////////////////////////
//	INLINED FUNCTIONS
///////////////////////////////////////////////////////////////////////////


//==========================================================================
inline void motion_player::SetZeroPose( xbool bMirror )
{
	//
	//	set the motion to -1
	//
	SetAnimation( -1, bMirror, FALSE );
}

//=============================================================================
inline void motion_player::SetBlendRootDirTime( f32 Secs )
{
	m_fRenderRootDirBlendTime	= Secs;
}

//=============================================================================
inline void motion_player::SetBlendFrames( f32 BlendFrames )
{
	if ( m_MocapFlags.Ind.BlendingOn && !m_MocapFlags.Ind.DefaultBlendingOn )
	{
		m_nBlendFrames	  = BlendFrames;
		m_NextBlendAmount = 0.0f;
	}
}

//==========================================================================
inline f32 motion_player::GetBlendFrames( void )
{
	return m_nBlendFrames;
}

//=============================================================================
inline void motion_player::SetBlendingOn( xbool bSetting )
{
	m_MocapFlags.Ind.DefaultBlendingOn = bSetting ? TRUE : FALSE;
}

//=============================================================================
inline void motion_player::SetDefaultBlendingOn( xbool bSetting )
{
	m_MocapFlags.Ind.BlendingOn = bSetting ? TRUE : FALSE;
}

//==========================================================================
inline void	motion_player::SetQuatBlending( xbool Setting )
{
	m_bQuatBlend = Setting ? TRUE : FALSE;
}

//==========================================================================
inline void motion_player::SetQuatInterpolation( xbool Setting )
{
	QuaternionBlend( Setting );
}

//==========================================================================
inline radian3& motion_player::GetBoneRot( s32		Bone,
										   radian3&	Dest )
{
	ASSERT((Bone>=0) && m_AnimGroup && (Bone<m_AnimGroup->NBones) );
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
		QuatToRot( m_BoneRotationData[Bone]._q, Dest );
	else
		Dest = m_BoneRotationData[Bone]._r;

	return Dest;
}

//==========================================================================
inline radian3* motion_player::GetBoneRots( radian3* Dest )
{
	s32			i;

	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			QuatToRot( m_BoneRotationData[i]._q, Dest[i] );
	}
	else
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			Dest[i] = m_BoneRotationData[i]._r;
	}

	return Dest;
}

//==========================================================================
inline quaternion& motion_player::GetBoneQuat(	s32			Bone,
											    quaternion&	Dest )
{
	ASSERT((Bone>=0) && m_AnimGroup && (Bone<m_AnimGroup->NBones) );
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
		Dest = m_BoneRotationData[Bone]._q;
	else
		RotToQuat( m_BoneRotationData[Bone]._r, Dest );

	return Dest;
}

//==========================================================================
inline quaternion* motion_player::GetBoneQuats( quaternion* Dest )
{
	s32			i;
//	radian3		Q;

	ASSERT(m_AnimGroup);

	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			Dest[i] = m_BoneRotationData[i]._q;
	}
	else
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			RotXZYToQuat( m_BoneRotationData[i]._r, Dest[i] );
	}

	return Dest;
}

//==========================================================================
inline const URotData& motion_player::GetBoneData( s32 Bone )
{
	ASSERT((Bone>=0) && m_AnimGroup && (Bone<m_AnimGroup->NBones) );

	if( m_bBoneRotationsDirty )
		UpdateBoneRotationData();

	return m_BoneRotationData[Bone];
}

//==========================================================================
inline URotData& motion_player::GetBoneData( s32		Bone,
											 URotData&	Dest )
{
	return Dest = GetBoneData(Bone);
}

//==========================================================================
inline const URotData* motion_player::GetBoneData( void )
{
	if( m_bBoneRotationsDirty )
		UpdateBoneRotationData();

	return m_BoneRotationData;
}

//==========================================================================
inline URotData* motion_player::GetBoneData( URotData* Dest )
{
	x_memcpy( Dest, GetBoneData(), GetNumBones()*sizeof(URotData) );
	return Dest;
}


//==========================================================================
inline void motion_player::SetBoneRot( s32				Bone,
									   const radian3&	Src )
{
	//	make sure the rotations have been set before changing the value of one of them
	if( m_bBoneRotationsDirty )
		UpdateBoneRotationData();

	ASSERT((Bone>=0) && m_AnimGroup && (Bone<m_AnimGroup->NBones) );
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
		RotToQuat( Src, m_BoneRotationData[Bone]._q );
	else
		m_BoneRotationData[Bone]._r = Src;
}

//==========================================================================
inline void motion_player::SetBoneRots( const radian3* Src )
{
	s32			i;

	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			RotToQuat( Src[i], m_BoneRotationData[i]._q );
	}
	else
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			m_BoneRotationData[i]._r = Src[i];
	}

	//	the rotation data has been completely set externally
	m_bBoneRotationsDirty = FALSE;
}

//==========================================================================
inline void motion_player::SetBoneQuat(	s32					Bone,
									    const quaternion&	Src )
{
	//	make sure the rotations have been set before changing the value of one of them
	if( m_bBoneRotationsDirty )
		UpdateBoneRotationData();

	ASSERT((Bone>=0) && m_AnimGroup && (Bone<m_AnimGroup->NBones) );
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
		m_BoneRotationData[Bone]._q = Src;
	else
		QuatToRot( Src, m_BoneRotationData[Bone]._r );
}

//==========================================================================
inline void motion_player::SetBoneQuats( const quaternion* Src )
{
	s32			i;
//	radian3		Q;

	ASSERT(m_AnimGroup);

	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			m_BoneRotationData[i]._q = Src[i];
	}
	else
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			QuatToRot( Src[i], m_BoneRotationData[i]._r );
	}

	//	the rotation data has been completely set externally
	m_bBoneRotationsDirty	= FALSE;
}

//==========================================================================
inline void motion_player::SetBoneData( s32 Bone, const URotData& Src )
{
	ASSERT((Bone>=0) && m_AnimGroup && (Bone<m_AnimGroup->NBones) );

	//	make sure the rotations have been set before changing the value of one of them
	if( m_bBoneRotationsDirty )
		UpdateBoneRotationData();

	m_BoneRotationData[Bone] = Src;
}

//==========================================================================
inline void motion_player::SetBoneData( const URotData* Src )
{
	x_memcpy( m_BoneRotationData, Src, GetNumBones()*sizeof(URotData) );

	//	the rotation data has been completely set externally
	m_bBoneRotationsDirty	= FALSE;
}

//==========================================================================
inline void motion_player::GetBonePosition( s32 BoneIndex, vector3& rBonePos )
{
	matrix4 BM; // needed in case the matrices arent cached.  Remove this if we make the caching a requirement.
	rBonePos = GetBoneMatrix( BoneIndex, BM ).GetTranslation();
}

//==========================================================================
inline void motion_player::SetWorldPosition( const vector3& WorldPos )
{
	mocap_player::SetWorldPosition( WorldPos );
	SetMatricesDirty( TRUE );
}

//==========================================================================
inline void motion_player::SetWorldScale( f32 WorldScale )
{
	mocap_player::SetWorldScale( WorldScale );
	SetMatricesDirty( TRUE );
}

//==========================================================================
inline s32 motion_player::GetNumBones( void )
{
	return ANIM_GetNBones( m_AnimGroup );
}

//==========================================================================
inline s32 motion_player::GetBoneIndex( char* BoneName )
{
	return ANIM_GetBoneIndex( BoneName, m_AnimGroup );
}

//==========================================================================
inline xbool motion_player::GetRotationsDirty( void )
{
	return m_bBoneRotationsDirty;
}

//==========================================================================
inline void motion_player::SetRotationsDirty( xbool Setting )
{
	m_bBoneRotationsDirty	= Setting;
	if( m_bBoneRotationsDirty )
		SetMatricesDirty( TRUE );
}

//==========================================================================
inline xbool motion_player::GetMatricesDirty( void )
{
	return m_bBoneMatricesDirty;
}

//==========================================================================
inline void motion_player::SetMatricesDirty( xbool Setting )
{
	m_bBoneMatricesDirty = Setting;
}

//==========================================================================
inline void motion_player::BuildBoneMatrices( matrix4* pBoneL2Ws )
{
    // build the matricies
	ASSERT(pBoneL2Ws);
    AMSKL_GetBoneMatrices ( m_AnimGroup,
                            pBoneL2Ws,
                            GetBoneData(),
							GetBoneDataFormat(),
                            m_bMirror,
							FALSE,
                            m_WorldScale,
                            GetRenderFacing(),
                            m_WorldPos,
							m_bTransform ? &m_Transform : NULL );
}

//==========================================================================
inline void motion_player::BuildBoneMatrix( s32 BoneID, matrix4& rBoneL2W )
{
    // build the matricies
    AMSKL_GetBoneMatrix ( m_AnimGroup,
                          rBoneL2W,
						  BoneID,
                          GetBoneData(),
						  GetBoneDataFormat(),
                          m_bMirror,
						  FALSE,
                          m_WorldScale,
                          GetRenderFacing(),
                          m_WorldPos,
						  m_bTransform ? &m_Transform : NULL );
}

//==========================================================================
inline void motion_player::UpdateBoneMatricies( void )
{
	ASSERTS(m_pBoneMatrices, "Bone matrices are not stored.  This function cannot be used to get them.");

	//---	build the matrix data if it is out of date
	if( GetMatricesDirty() )
	{
		BuildBoneMatrices( m_pBoneMatrices );
		SetMatricesDirty( FALSE );
	}
}

//==========================================================================
inline const matrix4& motion_player::GetBoneMatrix( s32 BoneIndex )
{
	ASSERTS(m_pBoneMatrices, "Bone matrices are not stored.  This function cannot be used to get them.");
	ASSERT(BoneIndex < GetNumBones());
	ASSERT(BoneIndex < 32);

	//---	update the matrix data
	UpdateBoneMatricies();

	return m_pBoneMatrices[ BoneIndex ];
}

//==========================================================================
inline matrix4& motion_player::GetBoneMatrix( s32 BoneIndex, matrix4& Dest )
{
	ASSERT(BoneIndex < GetNumBones());
	ASSERT(BoneIndex < 32);
	ASSERT(BoneIndex >= 0);

	//	if the bone matrices are cached, get the matrix from the cached data
	if( m_pBoneMatrices )
	{
		//---	update the matrix data
		UpdateBoneMatricies();

		Dest = m_pBoneMatrices[ BoneIndex ];
	}
	else
		BuildBoneMatrix( BoneIndex, Dest );

	return Dest;
}

//==========================================================================
inline const matrix4* motion_player::GetBoneMatrices( void )
{
	ASSERTS(m_pBoneMatrices, "Bone matrices are not stored.  This function cannot be used to get them.");

	//---	update the matrix data
	UpdateBoneMatricies();
	
	return m_pBoneMatrices;
}

//==========================================================================
inline matrix4* motion_player::GetBoneMatrices( matrix4* pDest )
{
	if( m_pBoneMatrices )
	{
		//---	update the matrix data
		UpdateBoneMatricies();
		
		x_memcpy( pDest, m_pBoneMatrices, GetNumBones()*sizeof(matrix4) );
	}
	else
		BuildBoneMatrices( pDest );

	return pDest;
}

//==========================================================================
inline xbool motion_player::GetPropInfo( s32				PropIndex,
										  vector3&			Pos,
										  radian3&			Rot )
{
    ASSERT((PropIndex>=0) && (PropIndex<=ANIM_MAX_PROPS));
	SPropData& rPropData = m_PropData[PropIndex];

	//---	prop data is updated with the bone rotations, make sure they are updated
	if( GetRotationsDirty() )
		UpdateBoneRotationData();

	//---	if there is no prop definition, return FALSE that the prop data hasn't yet been initialized
	if( !rPropData.pPropDef )
		return FALSE;

	//---	get the bone's index, if the prop has a bone index, get the bone's matrix
	xbool	bBoneRelative;
	s32 BoneID = rPropData.pPropDef->BoneID;
	matrix4 BM; // needed in case the matrices arent cached.  Remove this if we make the caching a requirement.
	if( BoneID != -1 )
	{
		//---	if the player is mirrored, the prop will be based on the mirrored bone matrix
		if( m_bMirror && (rPropData.pPropDef->Flags&prop_def::MIRRORWITHANIM))
			BoneID = m_AnimGroup->Bone[BoneID].MirrorID;

		GetBoneMatrix( BoneID, BM ),
		bBoneRelative	= TRUE;
	}
	else
		bBoneRelative	= FALSE;

	//---	convert the prop data into the requested prop information
	PropDataToPropPosRot(	m_PropData[PropIndex],
							bBoneRelative,
							BM,
							Pos,
							Rot );

	return TRUE;
}

//==========================================================================
inline s32 motion_player::GetPropInfo( vector3* pPos,
										 radian3* pRot,
										 s32	  Len )
{
	s32		i;
	xbool	ret = FALSE;

    ASSERT((Len>=0) && (Len<=ANIM_MAX_PROPS));
	if( Len > ANIM_MAX_PROPS )
		Len = ANIM_MAX_PROPS;

	for( i=0; i<Len; i++ )
		if( GetPropInfo( i, pPos[i], pRot[i] ) )
			ret = TRUE;

	return i;
}

//==========================================================================
inline void motion_player::SetTransform( const matrix4&	rTransform )
{
	mocap_player::SetTransform( rTransform );
	SetMatricesDirty( TRUE );
}

//==========================================================================
inline void motion_player::ApplyTransform( xbool bSetting )
{
	mocap_player::ApplyTransform( bSetting );
	SetMatricesDirty( TRUE );
}

#endif // _AM_MOTIONPLAYER_HPP_

///////////////////////////////////////////////////////////////////////////
//
//  AM_MocapPlayer.hpp		
//
///////////////////////////////////////////////////////////////////////////

#ifndef _AM_MOCAPPLAYER_HPP_
#define _AM_MOCAPPLAYER_HPP_

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "AM_Export.h"
#include "AM_Cache.h"

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define	ANIM_FLAGS_QUATERNION	(1<<0)


//---	if the maximum number of blended motions
#ifndef	MAX_MOTION_BLEND_COUNT
#define	MAX_MOTION_BLEND_COUNT	1
#endif

enum
{
	ANIM_MAX_EVENTS			= 8,
};

///////////////////////////////////////////////////////////////////////////
// STRUCTS
///////////////////////////////////////////////////////////////////////////

//---	struct for setting multiple animation blending.
struct SMultAnim
{
	s32		AnimID;		// motion ID
	f32		fBlend;		// amout of blend weight given to this motion
};

//---	roation structure for holding bone and prop rotations
struct URotData
{
	radian3		_r;
	quaternion	_q;
};

//---	prop data structure
struct SPropData
{
	prop_def*	pPropDef;	// prop definition
	URotData	Rot;		// prop rotation
	vector3		Pos;		// prop position
};

struct SPlaybackData
{
	anim_info*      pAnimInfo;						// If no anim is set == NULL
//	xbool			bMirror;						// is this motion mirrored before the blend
	f32				Blend;							// ratio of this motion which goes into the motion mix.
	f32				BlendOrderRatio;				// blend used to combine this animation with the previously blended motions to produce the propper end result.
	f32				FrameRatio;						// converts m_CurrentFrame into 60fps frame reletive to this motion (which can then be converted to an export frame)

	SMotionBlock	Block;

	//---	static motion data
	f32				CurrentExportFrame;				// Current frame within the exported data (not the same as the frames of the animaiton)
	f32				FrameToExportFrame;				// converts a current frame to a frame number matching the exported frames
	f32				ExportFrameToFrame;				// converts an exported frame number to the regular animation frame number
												
	vector3         RawPos;							// Trans in cycle 0 on current frame
	radian          RawMoveDir;						// MoveDir in cycle 0 on current frame
	radian          RawRootDir;						// RootDir in cycle 0 on current frame
												
//		radian          RootDirAtFrame0;			// Horiz angle of root node zaxis
	radian          RootDirAtFrameN;				// Horiz angle of root node zaxis
	radian          MoveDirAtFrame0;				// MoveDir on first frame
	radian          MoveDirAtFrameN;				// MoveDir on last frame
	radian          TotalMoveDir;					// Horiz angle of total trans
//	vector3         TransAtFrame0;					// Translation at frame 0
//	vector3         TransAtFrameN;					// Translation at frame N
	f32				TransYAtFrame0;					// Translation Y at frame 0
	f32				TransYAtFrameN;					// Translation Y at frame N
	vector3         TotalTrans;						// TransN-Trans0 plus some modifiers
									
	s32				Angle360;						// angle values for this animation
	f32				AngleToRadians;					// conversion from the exported angle data to radians
	f32				ValueToQuat;					// conversion from the exported quaternion value to a real quaternion value
	f32				ValueToQuatW;					// conversion from the exported quaternion W value to a real quaternion W value

	u8				PropStream[ANIM_MAX_PROPS];		// Stream that each prop starts on
	u8				PointStream[ANIM_MAX_POINTS];	// Stream that each pt starts on
	u8				UserStream[ANIM_MAX_USER_STREAMS];         
};


///////////////////////////////////////////////////////////////////////////
// CLASS
///////////////////////////////////////////////////////////////////////////

class mocap_player
{
///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////
public:
				mocap_player           ( void );
				~mocap_player          ();
            
	///////////////////////////////////////////////////////////////////////////
	// INSTANCE INIT AND KILL
	///////////////////////////////////////////////////////////////////////////

	void    Init                    ( anim_group*		AnimGroup );
	void    Kill                    ( void );                  

	///////////////////////////////////////////////////////////////////////////
	// SET ANIMATION
	///////////////////////////////////////////////////////////////////////////
	//
	// This call will switch over to a new animation at cycle 0, frame 0.  At
	// this point it is necessary to know whether the animation will be played
	// mirrored or not.  You may pass in -1 as the AnimID to select a NULL 
	// animation.
	//
	///////////////////////////////////////////////////////////////////////////

	void    SetAnimation            ( s32   AnimID,
									  xbool bMirror=FALSE );

	///////////////////////////////////////////////////////////////////////////
	//	BIDERECTIONAL BLENDING FUNCTIONS
	///////////////////////////////////////////////////////////////////////////
	//
	// These functions allow the application to set another motion and a blend
	// amount between the primary motion and the secondary motion.  The blend
	// ratio can be changed real time to cause the playback to blend from one
	// motion to the other.  This second motion can be set at any time and will
	// automatically align its from zeror with the primary motion's frame zero
	// so that the primary motion's current frame will match the correct location
	// within this secondary motion.
	//
	// A call to SetAnimation() will automatically change the ratio to 0.0f (in
	// favor of the motion just set.  This will effectively end the bi-directional
	// blending.  However, another call to SetAnim2Blend() will start it up again
	// using the AnimID and Mirror flag set on the last call to SetAnimation2().
	//
	// NOTE: The mirror flag used for the first motion will mirror the overall blend
	//		 affects.  For all motions after the first, the mirror only affects that
	//		 specified motion.  There is no way to mirror the first motion before the
	//		 blend is processed. If you think about it, you can see why this system is
	//		 easier, faster, and has no limitations.  Thet GetMirror() function returns
	//		 the status of this overall mirror setting.
	//
	// NOTE: Both motions must be exported in the same data format.  This is to
	//		 keep from wasting time with constant motion data conversions.  If
	//		 any are not, an ASSERT will be triggered on a debug build.
	//
	///////////////////////////////////////////////////////////////////////////
	void    SetAnimation2			( s32   AnimID,
									  f32	Blend = 0.0f );
	void	SetAnim2Blend			( f32	Blend );


	///////////////////////////////////////////////////////////////////////////
	//	MULTIPLE MOTION BLENDING FUNCTION
	///////////////////////////////////////////////////////////////////////////
	//
	// The SetMultAnimation() function lets the application set many motions
	// at once with blend amounts between the given animations.  The motion
	// ID's, mirror values and blend ratios are set into an array of SMultAnim
	// structures which get passed in.  The data will be copied out of this array
	// so the calling function doesn't need to keep the data persistant.
	//
	// If more motions are passed in that the system allows for, an ASSERT will be
	// triggered on a debug build, and the remaining motions above the limit will
	// be ignored.
	//
	// A call to SetAnimation() will terminate the playback of the multiple motion.
	// A NULL anim list is not allowed, nor is a list length of less than 1.
	//
	// If the given blend amounts are less than one, the remaining blend amount is
	// assumed to be for the currently set animation.  The motions will combine with
	// that motion in a fasion similar to a call to SetAnimation2().  In other words, 
	// the primary motion will not be reset to frame zero.  For this reason, a call
	// to SetMultiAnimation() with a list length of one is identical to a call to
	// SetAnimation2(), however a bit slower at run-time.
	//
	// SetMultAnimBlend(), SetMultAnimMirror() is assumed to be passed a list of the
	// correct length for the animations.
	//
	// NOTE: The mirror flag used for the first motion (if blend amounts are less
	//		 that zero, the first motion is the one set with the previous call to
	//		 SetAnimation()) will mirror the overall blend affects.  For all motions
	//		 after the first, the mirror only affects that specified motion.  There
	//		 is no way to mirror the first motion before the blend is processed. If
	//		 you think about it, you can see why this system is easier, faster, and
	//		 has no limitations.  Thet GetMirror() function returns the status of this
	//		 overall mirror setting.
	//
	// NOTE: All motions must be exported in the same data format.  This is to
	//		 keep from wasting time with constant motion data conversions.  If
	//		 any are not, an ASSERT will be triggered on a debug build.
	//
	///////////////////////////////////////////////////////////////////////////
	void    SetMultAnimation		( s32*			pAnimIDList,
									  f32*			pBlendList,
									  s32			ListLength,
									  xbool			bMirror );
	void	SetMultAnimBlend		( f32*			pBlendList );


	///////////////////////////////////////////////////////////////////////////
	// ADVANCE N FRAMES
	///////////////////////////////////////////////////////////////////////////
	// 
	// You can pass in a positive or negative number to step forward or 
	// backwards in the current animation.  As the animation loops over itself
	// it will link together using the rootdir.  This gives the appearance of
	// of a continuous animation.  The current cycle will be incremented and 
	// current frame will be modulated.  NFrames is in (1/60th) of a second.
	// After an animation has been advanced the animation system can tell you
	// where the actor should be if he was playing the animation exactly.  The
	// values returned take into account the actor's current position, 
	// direction, and scale in the world.  If you take the values returned and
	// passed them back to the anim_player directly using the set functions,
	// the actor will play the animation unmodified.  
	//
	///////////////////////////////////////////////////////////////////////////

	void    AdvNFrames              ( f32                NFrames,
									  vector3&           NewWorldPos,
									  radian&            NewWorldRootDir,
									  radian&            NewWorldMoveDir,
									  radian&            NewWorldTotalMoveDir );

	///////////////////////////////////////////////////////////////////////////
	// JUMP TO FRAME
	///////////////////////////////////////////////////////////////////////////
	// 
	// By specifying the frame you wish to jump to the animation system will
	// move to the requested frame and keep the rootdirs of the original and 
	// new poses aligned.  The other world dirs will be recomputed. The position
	// of the character will not be altered.
	//
	///////////////////////////////////////////////////////////////////////////

	void    JumpToFrame             ( f32                DestFrame );


	///////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	// Get information functions
	///////////////////////////////////////////////////////////////////////////
	// 
	//	Functions to retreive general information from the mocap_player
	//
	///////////////////////////////////////////////////////////////////////////
	s32					GetLastFrame			( void ) const;
	s32					GetTotalFrames          ( void ) const;
	const vector3&		GetTotalTrans			( void ) const;
	radian				GetTotalDeltaRootDir	( void ) const;
	f32					GetCurrentFrame         ( void ) const;
	s32					GetCurrentCycle         ( void ) const;
	xbool				NoAnimSet				( void ) const;
	s32					GetAnimID               ( void ) const;
	xbool				GetMirror				( void ) const;
	anim_group*			GetAnimGroup			( void ) const;
	const anim_info*	GetAnimInfo				( void ) const;
	const anim_info*	GetAnimInfo				( s32 iBlendMotion ) const;
	u32					GetAnimInfoFlags		( void ) const;

	//////////////////////////////////////////////////////////////////////////
	// MOTION CONTROL -SETS
	//////////////////////////////////////////////////////////////////////////
	//
	// The WorldPosition is the location in the world of the character's 
	// root node.  This includes the character's altitude or Y-value.  The
	// WorldScale is the general scale of the character.  If the person 
	// motion captured was 66 inches and the character you are rendering is
	// 72 inches.  You need to tell the animation system a world scale of
	// 72/66.  The scale allows your character to take the correct length
	// strides, and for events and props to position themselves correctly.
	// The Dir functions allow you to orient the character using a 'handle'
	// in the animation and pointing it in some direction in the world.  For
	// instance, if you want the character to run in a direction of 45 degrees
	// and you know that the RootDir points in the direction he's going you
	// can use SetWorldRootDir(R_45) to point him in that direction.
	//
	//////////////////////////////////////////////////////////////////////////

	void    SetWorldPosition        ( const vector3&     WorldPos );
	void    SetWorldMoveDir         ( radian             WorldMoveDir );
	void    SetWorldFinalMoveDir    ( radian             WorldFinalMoveDir );
	void    SetWorldTotalMoveDir    ( radian             WorldTotalMoveDir );
	void    SetWorldRootDir         ( radian             WorldRootDir );
	void    SetWorldFinalRootDir    ( radian             WorldFinalRootDir );
	void    SetWorldScale           ( f32                WorldScale );  

    void    SetYaw                  ( radian            Yaw);

	//////////////////////////////////////////////////////////////////////////
	// MOTION CONTROL -GETS
	//////////////////////////////////////////////////////////////////////////
	//
	// These return the current values of the motion control listed above
	//
	//////////////////////////////////////////////////////////////////////////

	radian  GetWorldMoveDir         ( void );
	radian  GetWorldFinalMoveDir    ( void );
	radian  GetWorldTotalMoveDir    ( void );
	radian  GetWorldRootDir         ( void );
	radian  GetWorldFinalRootDir    ( void );
	f32		GetRawY					( void ) const;
	f32     GetWorldScale           ( void ) const;
	xbool   IsMirrored              ( void ) const { return GetMirror(); }

	void    GetWorldPosition        ( vector3&           WorldPos );

	void    GetFirstLastMoveDir     ( s32                AnimID,
									  radian&            FirstMoveDir,
									  radian&            LastMoveDir );

	void    GetFirstLastRootDir     ( s32                AnimID,
									  radian&            FirstRootDir,
									  radian&            LastRootDir );

	///////////////////////////////////////////////////////////////////////////
	// RENDER INFO
	///////////////////////////////////////////////////////////////////////////
	//
	//	GetWorldYaw() is the angle calculated to apply to the motion so that it
	//	face the direction requested by the application.  This rotation must be
	//	applied when building the matrices for the animation.
	//
	radian		GetWorldYaw             ( void ) const;

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
	//	GetBoneDataFormat()	flags to describe the format of the bone data.  These
	//	flags match the exported flags which describe things like whether or not
	//	this animation uses quaternions or eurlor angles, whether it is local or
	//	global rotations, or whether the root node's yaw is relative to the first
	//	frame of the motion, or simply zero and requires that the first frame yaw
	//	gets applied.
	//
	/////////////////////////////////////////////////////////////////////////////

	//---	get rotation data
	radian3&	GetBoneRot				( s32				Bone,
										  radian3&			Dest );
	radian3*	GetBoneRots				( radian3*			Dest );

	quaternion&	GetBoneQuat				( s32				Bone,
										  quaternion&		Dest );
	quaternion*	GetBoneQuats			( quaternion*		Dest );

	//---	get rotation or quaternion data, whichever is available
	URotData&	GetBoneData				( s32				Bone,
										  URotData&			Dest );
	URotData*	GetBoneData				( URotData*			Dest );

	//---	get the format of the bone data to use in interpreting the URotData array
	s32			GetBoneDataFormat		( void ) const;
	xbool		IsBoneDataQuat			( void ) const; // is the bone data quaternion


	///////////////////////////////////////////////////////////////////////////
	// PROJECT END OF ANIM
	///////////////////////////////////////////////////////////////////////////
	//
	// Projects all of the logistics to the end of an animation
	//
	///////////////////////////////////////////////////////////////////////////
	void ProjectEndOfAnim( s32                AnimID,
						   f32                WorldScale,
						   xbool              Mirror,
						   radian             StartWorldRootDir,
						   vector3*           StartWorldPos,
						   radian*            FinalWorldRootDir,
						   vector3*           FinalWorldPos,
						   s32*               AnimFrames);

	//////////////////////////////////////////////////////////////////////////

	void ProjectEvent(		s32                AnimID,
							s32                EventID,
							f32                WorldScale,
							xbool              Mirror,
							radian             StartWorldRootDir,
							vector3*           StartWorldPos,
							vector3*           EventWorldPos,
							vector3*           RootWorldPos,
							s32*               EventFrame);

	///////////////////////////////////////////////////////////////////////////
	// TIME TILL EVENT
	///////////////////////////////////////////////////////////////////////////
	//
	// Returns the number of frames between the current frame of the player
	// and the next time the requested event occurs.  This assumes you are
	// looking forward in the animation.
	//
	///////////////////////////////////////////////////////////////////////////

	xbool   TimeTillEvent           ( s16                EventID,
									  f32&               Frame );

                                     
	///////////////////////////////////////////////////////////////////////////
	// EVENT INFO
	///////////////////////////////////////////////////////////////////////////
	//
	// Returns different pieces of info on any events that were accumulated
	// during an AdvNFrames.  The queue is cleared each AdvNFrames.  GetEventPos
	// takes into account the character's world position, direction, and scale.
	//
	///////////////////////////////////////////////////////////////////////////

	s32     GetNEventsQueued        ( void )	const;

	s32     GetEventID              ( s32                Index );

	f32     GetEventFrame           ( s32                Index );

	void    GetEventPos             ( s32                Index,
									  vector3&           EventPos );


	///////////////////////////////////////////////////////////////////////////
	// GET BONE INFO
	///////////////////////////////////////////////////////////////////////////
	//
	//	These functions return the final world position/rotation values.  They
	//	require different amounts of calculations depending on which export format
	//	the animation data is stored in.
	//
	///////////////////////////////////////////////////////////////////////////

	void    GetBonePosition         ( s32                BoneIndex,
									  vector3&           WorldPos );

	void    GetBoneOrient           ( s32                BoneIndex,
									  radian3&           WorldRot );

	void    GetBoneOrient           ( s32                BoneIndex,
									  quaternion&        WorldQuat );

	void    GetBoneMatrix           ( s32                BoneIndex,
									  matrix4&           LocalToWorldM );

	void    GetBoneMatrices			( matrix4*           LocalToWorldMs );



	///////////////////////////////////////////////////////////////////////////
	// PROP INFORMATION
	///////////////////////////////////////////////////////////////////////////
	//
	// IsPropPresent returns whether the requested prop is present at any time
	// in the animation.  GetPropInfo returns the world position and orientation
	// of the prop for the current frame.  If no prop is present on this frame,
	// the position and orientation is set to 0.  Note also that GetPropInfo 
	// will return wether the prop was present at that particular frame.  If
	// the logic needs two know when a prop starts or stops being present, 
	// consider using events.
	//
	// NOTE: We are keeping position and rotation separate so that the game application
	//		 can decide whether and how to scale the prop independantly of the shape and
	//		 size of the mocap_player.  Props such as a baseball bat would need to have
	//		 their position scaled, but wouldn't be scaled themselves while props such
	//		 as hair would scale in size as well as positionally.
	//
	//	GetPropData()		returns the raw prop information for that animation or
	//						set of blended animations.  It is not positioned in world
	//						space, but is reletive to the mocap_player.
	//
	//	IsPropPresent()		Is a prop active for this motion.
	//	IsPropStreamed()	Is the active prop streamed with motion data (was a motion linked to it at export)
	//
	//	PropDataToPropInfo()	This function will convert prop data reletive to the
	//							mocap player into a position and rotation in world space.
	//
	///////////////////////////////////////////////////////////////////////////

	xbool   GetPropInfo             ( s32				PropIndex,
									  vector3&			Pos,
									  radian3&			Rot );

	xbool   GetPropInfo             ( vector3*			Pos,
									  radian3*			Rot,
									  s32				Len );

	xbool	GetPropData				( s32				PropIndex,
									  SPropData&		rPropData );
	s32		GetPropData				( SPropData*		pPropData,
									  s32				Len );

	xbool   IsPropPresent			( void );
	xbool   IsPropPresent           ( s32				PropIndex );
	xbool   IsPropStreamed          ( s32				PropIndex );


	void	PropDataToPropInfo		( SPropData&		rData,
									  vector3&			Pos,
									  radian3&			Rot );

	//////////////////////////////////////////////////////////////////////////
	//
	//		Global Transformation of the actor which is applied completely
	//	outside of the regular animation playback.  This can be used to place
	//	the actor in the world without changing the nature of the motion playback.
	//	It has been added here so that the matrix building functions can be
	//	optimized.
	//
	//		This transform affects all/only matricies retreived from this class.
	//
	//////////////////////////////////////////////////////////////////////////
	void			SetTransform			( const matrix4&	rTransform );
	const matrix4&	GetTransform			( void );
	void			ApplyTransform			( xbool bSetting=TRUE );

	///////////////////////////////////////////////////////////////////////////
	// USER FLAGS
	///////////////////////////////////////////////////////////////////////////
	//
	// User defined flags can be queried individually or the entire flags set
	// can be requested.
	//
	///////////////////////////////////////////////////////////////////////////

	xbool   GetUserFlag             ( s32                FlagIndex );

	u32     GetUserFlags            ( void );

	void	QuaternionBlend			( xbool bSetting = TRUE );


///////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
///////////////////////////////////////////////////////////////////////////
protected:

	//==========================================================================
	//
	//	GetRenderFacing
	//
	//		Returns the direction that the motion should be faced when rendering.
	//	if the motion data was exported using motion relative facing, the world yaw
	//	of the player will be returned.  The world yaw doesn't change during the
	//	full motion playback unless the application wants to rotate the player.  If
	//	on the other hand, the frames of the motion were exported without facing
	//	information (facing zero) then the player's root direction will be returned
	//	so that it will	be rendered facing the correct direction for this frame.
	//
	//==========================================================================
	radian	GetRenderFacing			( void );

	void	PropDataToPropPosRot	( SPropData&		rData,
									  vector3&			Pos,
									  radian3&			Rot );

	void	PropDataToPropPosRot	( SPropData&		rData,
									  xbool				bBoneRelative,
									  const matrix4&	rBoneMatrix,
									  vector3&			Pos,
									  radian3&			Rot );


///////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////

private:
	void	SetExportFrame			( SPlaybackData&	rMotionData,
									  f32				fExportFrame );

	void	CalcSetExportFrame		( void );

	void	CalcSetAnimRawMotion	( SPlaybackData&	rMotionData );
	void	CalcSetRawMotion		( void );

	radian	GetMotionBaseYaw		( void );
    
    void    GetFrameDataPtrs        ( SPlaybackData&	rMotionData,
									  s16**             DataPtr0,
                                      s16**             DataPtr1,
                                      f32*              Frac );

    void    QueueEvents             ( f32               StartFrame,
                                      f32               EndFrame );

	void	SetMotionPlaybackData	( SPlaybackData&	rMotionData,
									  s32				AnimID );

	void	CalcSetAnim2FrameRatio		( void );
	void	CalcSetMultiAnimFrameRatio	( void );

	void	SetCycleFrame			( s32				Cycle,
									  f32				Frame );

	void	GetAnimBoneData			( SPlaybackData&	rMotionData,
									  URotData*			Dest,
									  f32				Blend=1.0f );

	void	GetAnimBoneData			( SPlaybackData&	rMotionData,
									  URotData&			Dest,
									  s32				Bone,
									  f32				Blend=1.0f );

	xbool	GetAnimPropInfo			( SPlaybackData&	rMotionData,
									  s32				PropIndex,
									  prop_def*			pPropDef,
									  vector3&			Pos,
									  URotData&			Rot,
									  f32				Blend );

///////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////
public:

    anim_group*     m_AnimGroup;					// Group that these anims come from

	xbool			m_bMirror;
	xbool			m_bFramesConst;					// flag that all of the set animations have the same number of frames													
    f32             m_CurrentFrame;					// Modulated frame number
    s32             m_CurrentCycle;					// Cycle of animation we are in
	s32				m_LastFrameInCycle;				// last valid frame in the cycle (m_FramesPerCycle-1);
	s32				m_FramesPerCycle;				// number of frames in the motion (if the motion is a blend, the number of frames may be blended as well).

	vector3			m_CycleTotalTrans;				// Total translation of all blended motions
	radian			m_CycleTotalMoveDir;			// Total move direction of the cycle translation
	radian			m_CycleDeltaRootDir;			// Combined change in root direction for all blended motion
	radian			m_CycleMoveDirAtFrame0;			// Raw MoveDir at frame 0
	radian			m_CycleMoveDirAtFrameN;			// Raw MoveDir at frame N
	vector3			m_CycleRawPos;					// Raw position within the blended cycle
	radian			m_CycleRawRootDir;				// Raw root dir within the blended cycle.
	radian			m_CycleRawMoveDir;				// Raw move dir within the blended cycle

    u32             m_AnimFlags;					// Flags
													
    vector3         m_WorldPos;						// Current position in world
    radian          m_WorldMoveDir;					// Current movedir in world
    radian          m_WorldTotalMoveDir;			// Current total movedir in world
    radian          m_WorldRootDir;					// Current rootdir in world
    radian          m_WorldYaw;						// Current alignment yaw
    f32             m_WorldScale;					// Current scale

	xbool			m_bTransform;					// T/F has a transformation matrix been given?
	matrix4			m_Transform;					// Universal transform to be applied to any requested information

    s16             m_NEventsQueued;				// Num events accumulated
    u8              m_EventQueue[ANIM_MAX_EVENTS];

	s32				m_nMMM;							// multiple motion count.
	SPlaybackData	m_MMM[MAX_MOTION_BLEND_COUNT];	// Multiple motion mix. (the primary motion uses the regular system interface.
};






////////////////////////////////////////////////////////////////////////////
//	INLINED FUNCTIONS
////////////////////////////////////////////////////////////////////////////

//==========================================================================
inline void mocap_player::SetTransform( const matrix4& rTransform )
{
	m_bTransform = TRUE;
	m_Transform = rTransform;
}

//==========================================================================
inline const matrix4& mocap_player::GetTransform( void )
{
	return m_Transform;
}

//==========================================================================
inline void mocap_player::ApplyTransform( xbool bSetting )
{
	m_bTransform = bSetting;
}

//==========================================================================
inline const anim_info* mocap_player::GetAnimInfo( void ) const
{
	return m_MMM[0].pAnimInfo;
}

//==========================================================================
inline const anim_info* mocap_player::GetAnimInfo( s32 iBlendMotion ) const
{
	ASSERT((iBlendMotion>=0)&&(iBlendMotion<MAX_MOTION_BLEND_COUNT));
	return m_MMM[iBlendMotion].pAnimInfo;
}

//==========================================================================
inline s32 mocap_player::GetLastFrame( void ) const
{
    
    ASSERT(m_MMM[0].pAnimInfo);
	return m_LastFrameInCycle;
}

//==========================================================================
inline s32 mocap_player::GetTotalFrames( void ) const
{
    
    ASSERT(m_MMM[0].pAnimInfo);
	return m_FramesPerCycle;
}

//==========================================================================
inline const vector3& mocap_player::GetTotalTrans( void ) const
{
	return m_CycleTotalTrans;
}

//==========================================================================
inline radian mocap_player::GetTotalDeltaRootDir( void ) const
{
	return m_CycleDeltaRootDir;
}

//==========================================================================
inline f32 mocap_player::GetCurrentFrame( void ) const
{
    return m_CurrentFrame;
}

//==========================================================================
inline s32 mocap_player::GetCurrentCycle( void ) const
{
    return m_CurrentCycle;
}

//==========================================================================
inline anim_group* mocap_player::GetAnimGroup( void ) const
{
	return m_AnimGroup;
}

//==========================================================================
inline xbool mocap_player::NoAnimSet( void ) const
{
	return m_MMM[0].pAnimInfo ? FALSE : TRUE;
}

//==========================================================================
inline s32 mocap_player::GetAnimID( void ) const
{
	if( NoAnimSet() )
		return -1;

	return m_MMM[0].pAnimInfo->ID;
}

//==========================================================================
inline void mocap_player::GetWorldPosition( vector3& WorldPos )
{
    WorldPos = m_WorldPos;
}

//==========================================================================
inline radian mocap_player::GetWorldMoveDir( void )
{
    
    return m_WorldMoveDir;
}

//==========================================================================
inline radian mocap_player::GetWorldFinalMoveDir( void )
{
    radian RawFinalMoveDir;
    
    ASSERT(m_MMM[0].pAnimInfo);
	ASSERTS(m_nMMM==1, "No multi-anim blend version of this function yet.");

    RawFinalMoveDir = m_MMM[0].pAnimInfo->MoveDirAtFrameN * m_MMM[0].AngleToRadians;
    if ( m_bMirror ) 
        RawFinalMoveDir = -RawFinalMoveDir;

    return m_WorldYaw + RawFinalMoveDir;
}

//==========================================================================
inline radian mocap_player::GetWorldTotalMoveDir( void )
{
    return m_WorldTotalMoveDir;
}

//==========================================================================
inline radian mocap_player::GetWorldRootDir( void )
{
    return m_WorldRootDir;
}

//==========================================================================
inline radian mocap_player::GetWorldFinalRootDir( void )
{
    radian RawFinalRootDir;
    
    ASSERT(m_MMM[0].pAnimInfo);
	ASSERTS(m_nMMM==1, "No multi-anim blend version of this function yet.");

    RawFinalRootDir = m_MMM[0].pAnimInfo->RootDirAtFrameN * m_MMM[0].AngleToRadians;
    if ( m_bMirror ) 
        RawFinalRootDir = -RawFinalRootDir;

    return m_WorldYaw + RawFinalRootDir;
}

//==========================================================================
inline f32 mocap_player::GetRawY( void ) const
{
	return m_CycleRawPos.Y;
}

//==========================================================================
inline f32 mocap_player::GetWorldScale( void ) const
{
    return m_WorldScale;
}

//==========================================================================
inline xbool mocap_player::GetMirror( void ) const
{
	return m_bMirror;
}

//==========================================================================
inline void mocap_player::SetWorldScale( f32 WorldScale )
{
    
    ASSERT((WorldScale >= 0.0f) && (WorldScale < 10.0f));
    m_WorldScale = WorldScale;
}

//==========================================================================
inline void mocap_player::SetWorldPosition( const vector3& WorldPos )
{
    ASSERT((WorldPos.X >= -100000.0f) && (WorldPos.X <= 100000.0f));
    ASSERT((WorldPos.Y >= -100000.0f) && (WorldPos.Y <= 100000.0f));
    ASSERT((WorldPos.Z >= -100000.0f) && (WorldPos.Z <= 100000.0f));
    m_WorldPos = WorldPos;
}

//==========================================================================
inline radian mocap_player::GetWorldYaw( void ) const
{
    return m_WorldYaw;
}

///////////////////////////////////////////////////////////////////////////
inline radian3& mocap_player::GetBoneRot( s32		Bone,
										  radian3&	Dest )
{
	ASSERT(m_AnimGroup);
	URotData Rot;

	GetBoneData( Bone, Rot );
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
		QuatToRot( Rot._q, Dest );
	else
		Dest = Rot._r;

	return Dest;
}

///////////////////////////////////////////////////////////////////////////
inline radian3* mocap_player::GetBoneRots( radian3* Dest )
{
	URotData	Rots[50];
	s32			i;

	ASSERT(m_AnimGroup);
	ASSERT(m_AnimGroup->NBones <= 50);

	GetBoneData( Rots );
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			QuatToRot( Rots[i]._q, Dest[i] );
	}
	else
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
			Dest[i] = Rots[i]._r;
	}

	return Dest;
}

///////////////////////////////////////////////////////////////////////////
inline quaternion& mocap_player::GetBoneQuat(	s32			Bone,
												quaternion&	Dest )
{
	ASSERT(m_AnimGroup);
	URotData Rot;

	GetBoneData( Bone, Rot );
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION )
		Dest = Rot._q;
	else
		RotToQuat( Rot._r, Dest );

	return Dest;
}

///////////////////////////////////////////////////////////////////////////
inline quaternion* mocap_player::GetBoneQuats( quaternion* Dest )
{
	s32			i;
	radian3		R;

	ASSERT(m_AnimGroup);
	ASSERT(m_AnimGroup->NBones <= 50);

	GetBoneData( (URotData*)Dest );
	if( !(GetAnimInfoFlags()&ANIMINFO_FLAG_QUATERNION) )
	{
		for( i=0; i<m_AnimGroup->NBones; i++ )
		{
			R = ((URotData*)&Dest[i])->_r;
			RotXZYToQuat( R, Dest[i] );
		}
	}

	return Dest;
}

///////////////////////////////////////////////////////////////////////////
inline void mocap_player::GetBonePosition( s32 BoneID, vector3& WorldPos )
{
    matrix4 M;
    GetBoneMatrix(BoneID,M);
    WorldPos = M.GetTranslation();
}

///////////////////////////////////////////////////////////////////////////
inline void mocap_player::GetBoneOrient( s32 BoneID, radian3& WorldRot )
{
//	radian3		TempRot;

    ASSERT(m_AnimGroup);
    ASSERT((BoneID>=0) && (BoneID<m_AnimGroup->NBones));

	// get this function to work off of the AMSKL_GetBoneOrient() function for speed
#if 1
	matrix4 M;
	GetBoneMatrix( BoneID, M );
	MatToRotXZY( M, WorldRot );
#else

    // Load rotations into temp buffer
    URotData    TempRotArray[50];
    ASSERT(m_AnimGroup->NBones <= 50);
    GetBoneData( TempRotArray );

	URotData RotData;
	AMSKL_GetBoneOrient( m_AnimGroup,
						 RotData,
						 BoneID,
						 TempRotArray,
						 GetBoneDataFormat(),
						 m_bMirror,
						 GetRenderFacing(),
						 m_bTransform ? &m_Transform : NULL );

	if( GetAnimInfoFlags()&ANIMINFO_FLAG_QUATERNION )
		QuatToRot( RotData._q, WorldRot  );
	else
		WorldRot = RotData._r;
#endif
}

///////////////////////////////////////////////////////////////////////////
inline void mocap_player::GetBoneOrient( s32 BoneID, quaternion& WorldQuat )
{
//	radian3		TempRot;

    ASSERT(m_AnimGroup);
    ASSERT((BoneID>=0) && (BoneID<m_AnimGroup->NBones));

	// get this function to work off of the AMSKL_GetBoneOrient() function for speed
#if 1

	matrix4 M;
	GetBoneMatrix( BoneID, M );
	WorldQuat.SetupMatrix( M );

#else

    // Load rotations into temp buffer
    URotData    TempRotArray[50];
    ASSERT(m_AnimGroup->NBones <= 50);
    GetBoneData( TempRotArray );

	URotData RotData;
	AMSKL_GetBoneOrient( m_AnimGroup,
						 RotData,
						 BoneID,
						 TempRotArray,
						 GetBoneDataFormat(),
						 m_bMirror,
						 GetRenderFacing(),
						 m_bTransform ? &m_Transform : NULL );

	if( GetAnimInfoFlags()&ANIMINFO_FLAG_QUATERNION )
		WorldQuat = RotData._q;
	else
		RotToQuat( RotData._r, WorldQuat );

#endif
}

//==========================================================================
inline s32 mocap_player::GetBoneDataFormat( void ) const
{
	return GetAnimInfoFlags() & (ANIMINFO_FLAG_QUATERNION|ANIMINFO_FLAG_GLOBALROTS|ANIMINFO_FLAG_WORLDROTS);
}

//==========================================================================
inline xbool mocap_player::IsBoneDataQuat( void ) const
{
	return (GetAnimInfoFlags() & ANIMINFO_FLAG_QUATERNION) ? TRUE : FALSE;
}

//==========================================================================
inline xbool mocap_player::IsPropStreamed( s32 PropIndex )
{
	s32 i;

    ASSERT((PropIndex>=0) && (PropIndex<ANIM_MAX_PROPS));
	if( m_MMM[0].PropStream[PropIndex] )
		return TRUE;

	if( m_nMMM > 1 )
	{
		for( i=1; i<m_nMMM; i++ )
			if( m_MMM[i].PropStream[PropIndex] )
				return TRUE;
	}

	return FALSE;
}

//==========================================================================
inline xbool mocap_player::IsPropPresent( s32 PropIndex )
{
	s32 i;

	//
	//	if there is no motion, return FALSE
	//
	if( !m_MMM[0].pAnimInfo )
		return FALSE;

    ASSERT((PropIndex>=0) && (PropIndex<ANIM_MAX_PROPS));
	if( m_MMM[0].pAnimInfo->PropsActive & (1<<PropIndex) )
		return TRUE;

	if( m_nMMM > 1 )
	{
		for( i=1; i<m_nMMM; i++ )
			if( m_MMM[i].pAnimInfo->PropsActive & (1<<PropIndex) )
				return TRUE;
	}

	return FALSE;
}

//==========================================================================
inline xbool mocap_player::IsPropPresent( void )
{
	s32 i;

	//
	//	if there is no motion, return FALSE
	//
	if( !m_MMM[0].pAnimInfo )
		return FALSE;

	ASSERT(m_MMM[0].pAnimInfo);
	if( m_MMM[0].pAnimInfo->PropsActive )
		return TRUE;

	if( m_nMMM > 1 )
	{
		ASSERT(m_MMM[0].pAnimInfo);
		for( i=1; i<m_nMMM; i++ )
			if( m_MMM[i].pAnimInfo->PropsActive )
				return TRUE;
	}

	return FALSE;
}

//==========================================================================
inline s32 mocap_player::GetPropData( SPropData* pPropData, s32 Len )
{
	s32		i;

    ASSERT((Len>=0) && (Len<=ANIM_MAX_PROPS));
	if( Len > ANIM_MAX_PROPS )
		Len = ANIM_MAX_PROPS;

	for( i=0; i<Len; i++ )
		GetPropData( i, pPropData[i] );

	return i;
}

//==========================================================================
inline xbool mocap_player::GetPropInfo( s32				PropIndex,
										vector3&		Pos,
										radian3&		Rot )
{
	SPropData		PropData;

	//---	get the prop data
	if( !GetPropData( PropIndex, PropData ) )
		return FALSE;

	//---	convert the prop data into the requested prop information
	PropDataToPropPosRot( PropData, Pos, Rot );

	return TRUE;
}

//==========================================================================
inline s32 mocap_player::GetPropInfo( vector3*	pPos,
									  radian3*	pRot,
									  s32		Len )
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
inline xbool mocap_player::GetUserFlag( s32 FlagIndex )
{
    ASSERT(m_MMM[0].pAnimInfo);
    ASSERT((FlagIndex>=0) && (FlagIndex<16));
    return (m_MMM[0].pAnimInfo->UserFlags & (1<<FlagIndex)) ? (TRUE):(FALSE);
}

//==========================================================================
inline u32 mocap_player::GetUserFlags( void )
{
    
    ASSERT(m_MMM[0].pAnimInfo);
    return (u32)m_MMM[0].pAnimInfo->UserFlags;
}

//==========================================================================
inline s32 mocap_player::GetNEventsQueued( void ) const
{
    return m_NEventsQueued;
}

//==========================================================================
inline s32 mocap_player::GetEventID( s32 Index )
{
    anim_event* EV;
    ASSERT((Index>=0) && (Index<m_NEventsQueued));
    EV = &m_AnimGroup->AnimEvent[ m_EventQueue[Index] + m_MMM[0].pAnimInfo->EventIndex ];
    return EV->EventID;
}

//==========================================================================
inline f32 mocap_player::GetEventFrame( s32 Index )
{
    anim_event* EV;
    ASSERT((Index>=0) && (Index<m_NEventsQueued));
    EV = &m_AnimGroup->AnimEvent[ m_EventQueue[Index] + m_MMM[0].pAnimInfo->EventIndex ];
    return EV->Frame;
}

//==========================================================================
inline u32 mocap_player::GetAnimInfoFlags( void ) const
{
	return m_AnimFlags;
}

//==========================================================================
inline radian mocap_player::GetRenderFacing( void )
{
	//---	if the current animation is based on world rotations, the motions base Yaw value
	//		is used to convert its rotations into world rotations.
	if( GetAnimInfoFlags() & ANIMINFO_FLAG_WORLDROTS )
		return GetWorldYaw();

	return GetWorldRootDir() ;
}

#endif // _AM_MOCAPPLAYER_HPP_

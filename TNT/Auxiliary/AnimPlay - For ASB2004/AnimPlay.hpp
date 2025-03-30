///////////////////////////////////////////////////////////////////////////
//
//  ANIMPLAY.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef _ANIMPLAY_H_
#define _ANIMPLAY_H_

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_math.hpp"
#include "AM_Group.hpp"

///////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATION
///////////////////////////////////////////////////////////////////////////

struct mocap_player;

///////////////////////////////////////////////////////////////////////////
// INIT MODULE
///////////////////////////////////////////////////////////////////////////

void		ANIM_InitModule			( s32 NumCacheStaticBuffers=32, s32 NumCacheDynamicBuffers=0 );
void		ANIM_KillModule			( void );                                     
//anim_group*	ANIM_GetDefAnimGroup	( void );
anim_group*	ANIM_GetAnimGroup		( mocap_player* pPlayer=NULL );
anim_group* ANIM_UseAnimGroup		( char* FileName );
void		ANIM_DoneWithAnimGroup	( char* FileName );
void		ANIM_DoneWithAnimGroup	( anim_group* pAnimGroup );
void		ANIM_DoneWithAnimGroup	( u32 ID );

char*		ANIM_GetAnimName		( anim_group* pAnimGroup, s32 AnimID );

//////////////////////////////////////////////////////////////////////////
// MOTION CONTROL -GETS
//////////////////////////////////////////////////////////////////////////
//
// These return the current values of the motion control listed above
//
//////////////////////////////////////////////////////////////////////////

void    ANIM_GetFirstLastMoveDir   ( s32                AnimID,
                                     radian*            FirstMoveDir,
                                     radian*            LastMoveDir,
									 anim_group*		AnimGroup);

void    ANIM_GetFirstLastRootDir   ( s32                AnimID,
                                     radian*            FirstRootDir,
                                     radian*            LastRootDir,
									 anim_group*		AnimGroup);

///////////////////////////////////////////////////////////////////////////
// GENERAL ANIMGROUP QUESTIONS
///////////////////////////////////////////////////////////////////////////
//
// WhenIsEvent will return the frame that the first instance of an event in
// an animation rests at.  It also returns an err value where success means
// the event was present.
//
///////////////////////////////////////////////////////////////////////////

s32         ANIM_GetNFrames        ( s32                AnimID,
									 anim_group*		AnimGroup);

xbool       ANIM_WhenIsEvent       ( s32                AnimID,
                                     s16                EventID,
                                     f32*               Frame,
									 anim_group*		AnimGroup);

s32         ANIM_GetNumAnimations  ( anim_group*		AnimGroup );

///////////////////////////////////////////////////////////////////////////
// PROJECT END OF ANIMATION
///////////////////////////////////////////////////////////////////////////
//
// Rotates and aligns the total translation of the requested animation to
// the starting conditions.  Takes into account whether animation is 
// mirrored or the player is scaled.  Can be used to build lookup tables or
// predicting position of player at end of animation.  
//
///////////////////////////////////////////////////////////////////////////

void    ANIM_ProjectEndOfAnim      ( s32                AnimID,
                                     f32                Scale,
                                     xbool              Mirror,
                                     radian             StartWorldRootDir,
                                     vector3*           StartWorldPos,
                                     radian*            DestFinalRootDir,
                                     vector3*           DestFinalWorldPos,
                                     s32*               DestAnimFrames,
									 anim_group*		AnimGroup );

///////////////////////////////////////////////////////////////////////////
// PROJECT EVENT POSITION ONTO ANIMATION
///////////////////////////////////////////////////////////////////////////
//
// Rotates and aligns the translation from the beginning of the requested
// animation to the time of the event.  Takes into account whether anim
// is mirrored or the player is scaled.  Can be used to build lookup tables
// or predicting position of player at time of event.
//
///////////////////////////////////////////////////////////////////////////

void    ANIM_ProjectEvent          ( s32                AnimID,
                                     s32                EventID,
                                     f32                Scale,
                                     xbool              Mirror,
                                     radian             StartWorldRootDir,
                                     vector3*           StartWorldPos,
                                     vector3*           DestEventWorldPos,
                                     vector3*           DestRootWorldPos,
                                     s32*               DestEventFrame,
									 anim_group*		AnimGroup );

///////////////////////////////////////////////////////////////////////////
// GET BONE INFO
///////////////////////////////////////////////////////////////////////////
//
// These functions come in different levels of CPU time.  GetAllBoneMatrices
// does a ton of math usually reserved for rendering the player.  
// GetBoneMatrix computes only the part of the skeleton that is needed and
// is not nearly as hefty as computing all the matrices but is still pretty
// heavy.  GetBonePosition is identical to GetBoneMatrix except that it
// returns only the position of the bone.  GetBoneOrient is actually the
// cheapest of the four and is useful for head tracking or firing of a 
// breath in the direction the head is currently facing.
//
///////////////////////////////////////////////////////////////////////////

s32     ANIM_GetNBones             ( anim_group*		AnimGroup);

s32     ANIM_GetBoneIndex          ( char*              BoneName,
									 anim_group*		AnimGroup );

char*   ANIM_GetBoneName           ( s32                BoneIndex,
									 anim_group*		AnimGroup );

void    ANIM_GetBonePos            ( s32 BoneIndex, vector3& BonePos,
									 anim_group*		AnimGroup );


///////////////////////////////////////////////////////////////////////////
// CLASS
///////////////////////////////////////////////////////////////////////////

struct mocap_player
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
                                  xbool MirrorIsOn );

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

s32     GetTotalFrames          ( void );
f32     GetCurrentFrame         ( void );
s32     GetCurrentCycle         ( void );
s32     GetAnimID               ( void );           

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
f32     GetWorldScale           ( void );
xbool   IsMirrored              ( void );

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
// The WorldYaw is the angle the animation system needed to rotate the
// character in order to have him face the direction you requested.  This
// must be applied at render time.  GetRotations copies the rotation values
// for the current frame into a buffer of your choice.
//
///////////////////////////////////////////////////////////////////////////

radian  GetWorldYaw             ( void );

radian	GetRotation				( s32				Bone,
								  radian3*			Dest );
radian	GetRotations            ( radian3*          Dest );


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

s32     GetNEventsQueued        ( void );

s32     GetEventID              ( s32                Index );

f32     GetEventFrame           ( s32                Index );

void    GetEventPos             ( s32                Index,
                                  vector3&           EventPos );

///////////////////////////////////////////////////////////////////////////
// GET BONE INFO
///////////////////////////////////////////////////////////////////////////
//
// These functions come in different levels of CPU time.  GetAllBoneMatrices
// does a ton of math usually reserved for rendering the player.  
// GetBoneMatrix computes only the part of the skeleton that is needed and
// is not nearly as hefty as computing all the matrices but is still pretty
// heavy.  GetBonePosition is identical to GetBoneMatrix except that it
// returns only the position of the bone.  GetBoneOrient is actually the
// cheapest of the four and is useful for head tracking or firing of a 
// breath in the direction the head is currently facing.
//
///////////////////////////////////////////////////////////////////////////

void    GetBonePosition         ( s32                BoneIndex,
                                  vector3&           WorldPos );

void    GetBoneOrient           ( s32                BoneIndex,
                                  radian3&           WorldRot );

void    GetBoneMatrix           ( s32                BoneIndex,
                                  matrix4&           LocalToWorldM );

void    GetAllBoneMatrices      ( matrix4*           LocalToWorldMs );

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
///////////////////////////////////////////////////////////////////////////

xbool   GetPropInfo             ( s32                PropIndex,
                                  vector3&           Pos,
                                  radian3&           Rot );

xbool   IsPropPresent           ( s32                PropIndex );

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

///////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////
public:

#define ANIM_MAX_EVENTS         8
#define ANIM_MAX_PROPS          8
#define ANIM_MAX_POINTS         8
#define ANIM_MAX_USER_STREAMS   8

    anim_group*     m_AnimGroup;					// Group that these anims come from
    anim_info*      m_AnimInfo;						// If no anim is set == NULL
													
    f32             m_CurrentFrame;					// Modulated frame number
    s32             m_CurrentCycle;					// Cycle of animation we are in
													
    s32             m_BlockID;						// ID of block we are in
    s32             m_BlockFrame0;					// First frame this block (in 60 f/s values).
    s32             m_BlockFrameN;					// Last frame this block (in 60 f/s values).
    s16*            m_BlockData;					// Ptr to decompressed anim data
													
	f32				m_CurrentExportFrame;			// Current frame within the exported data (not the same as the frames of the animaiton)
	f32				m_FrameToExportFrame;			// converts a current frame to a frame number matching the exported frames
	f32				m_ExportFrameToFrame;			// converts an exported frame number to the regular animation frame number

    u32             m_Flags;						// Flags
													
    vector3         m_WorldPos;						// Current position in world
    radian          m_WorldMoveDir;					// Current movedir in world
    radian          m_WorldTotalMoveDir;			// Current total movedir in world
    radian          m_WorldRootDir;					// Current rootdir in world
    radian          m_WorldYaw;						// Current alignment yaw
    f32             m_WorldScale;					// Current scale
													
    vector3         m_RawPos;						// Trans in cycle 0 on current frame
    radian          m_RawMoveDir;					// MoveDir in cycle 0 on current frame
    radian          m_RawRootDir;					// RootDir in cycle 0 on current frame

#ifdef REMOVE_FRAME_YAW
	radian			m_MotionFrameYaw;				// The yaw of the current motion frame relative to the base yaw of the motion
#endif
													
    radian          m_RootDirAtFrame0;				// Horiz angle of root node zaxis
    radian          m_RootDirAtFrameN;				// Horiz angle of root node zaxis
    radian          m_MoveDirAtFrame0;				// MoveDir on first frame
    radian          m_MoveDirAtFrameN;				// MoveDir on last frame
    radian          m_TotalMoveDir;					// Horiz angle of total trans
    vector3         m_TransAtFrame0;				// Translation at frame 0
    vector3         m_TransAtFrameN;				// Translation at frame N
    vector3         m_TotalTrans;					// TransN-Trans0 plus some modifiers
									
	s32				m_Angle360;						// angle values for this animation
	f32				m_AngleToRadians;				// conversion from the exported angle data to radians
													
    s16             m_NEventsQueued;				// Num events accumulated
    u8              m_EventQueue[ANIM_MAX_EVENTS];

    u8              m_PropStream[ANIM_MAX_PROPS];	// Stream that each prop starts on
    u8              m_PointStream[ANIM_MAX_POINTS];	// Stream that each pt starts on
    u8              m_UserStream[ANIM_MAX_USER_STREAMS];         
    
	f32				m_MaxBlend;						// maximum amount of blending for this update

    void    SetYaw                  ( radian            Yaw);
private:

    
    radian  GetRawMotion            ( vector3*          Trans,
                                      radian*           MoveDir,
                                      radian*           RootDir);

	radian	GetMotionBaseYaw		( void );
    
    void    GetFrameDataPtrs        ( s16**             DataPtr0,
                                      s16**             DataPtr1,
                                      f32*              Frac);

    void    QueueEvents             ( f32               StartFrame,
                                      f32               EndFrame);

};


#endif

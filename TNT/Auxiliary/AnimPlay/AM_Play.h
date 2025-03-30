///////////////////////////////////////////////////////////////////////////
//
//  AM_PLAY.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef _AM_PLAY_H_
#define _AM_PLAY_H_

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////
#include "x_math.hpp"
#include "x_plus.hpp"
#include "AM_Export.h"

///////////////////////////////////////////////////////////////////////////
// DEFINES / MACROS
///////////////////////////////////////////////////////////////////////////

#define STREAM_TX_OFFSET        0
#define STREAM_TY_OFFSET        1
#define STREAM_TZ_OFFSET        2
#define STREAM_MOVEDIR_OFFSET   3
#define STREAM_ROOTDIR_OFFSET   4

#define	VALUE_TO_INCHES			(1.0f/16.0f)

#define	NORMALIZE_QUAT_ON_BLEND	TRUE

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
// PRIVATE PROTOTYPES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
inline
s32 ANIM_GetNFrames( s32 AnimID, anim_group* AnimGroup)
{
    ASSERT(AnimGroup);
    ASSERT((AnimID >= 0) && (AnimID < AnimGroup->NAnimations));
    return AnimGroup->AnimInfo[AnimID].NFrames;
}

///////////////////////////////////////////////////////////////////////////
inline
s32 ANIM_GetNBones( anim_group*	AnimGroup )
{
    ASSERT(AnimGroup);
    return AnimGroup->NBones;
}

///////////////////////////////////////////////////////////////////////////
inline
void ANIM_GetBonePos( s32 BoneIndex, vector3& BonePos, anim_group* AnimGroup )
{
    ASSERT(AnimGroup);
    ASSERT((BoneIndex>=0) && (BoneIndex<AnimGroup->NBones));
    BonePos = AnimGroup->Bone[ BoneIndex ].OriginToBone;
}

///////////////////////////////////////////////////////////////////////////
inline
char* ANIM_GetBoneName( s32 BoneIndex, anim_group* AnimGroup )
{
    ASSERT(AnimGroup);
    ASSERT((BoneIndex>=0) && (BoneIndex<AnimGroup->NBones));
    return AnimGroup->Bone[BoneIndex].BoneName;
}

///////////////////////////////////////////////////////////////////////////
inline
s32 ANIM_GetBoneIndex( char* BoneName, anim_group* AnimGroup )
{
    s32 i;
    char BN[20];                     

    ASSERT(AnimGroup);
    ASSERT(BoneName);

    x_strcpy(BN,BoneName);
    x_strtoupper(BN);
    for (i=0; i<AnimGroup->NBones; i++)
    if (x_stricmp(BN,AnimGroup->Bone[i].BoneName)==0) return i;

    return -1;
}

///////////////////////////////////////////////////////////////////////////
inline
s32 ANIM_GetMaxStreams( anim_group*	AnimGroup)
{
    ASSERT(AnimGroup);
    return AnimGroup->MaxStreams;
}
 
///////////////////////////////////////////////////////////////////////////
inline
s32 ANIM_GetNumAnimations( anim_group* AnimGroup)
{
    ASSERT(AnimGroup);
    return AnimGroup->NAnimations;
}

///////////////////////////////////////////////////////////////////////////
inline
void ANIM_GetFirstLastMoveDir( s32				AnimID,
							   radian*			FirstMoveDir,
							   radian*			LastMoveDir,
							   anim_group*		AnimGroup )
{
	anim_info* pAnim;
    f32 C;

    ASSERT(AnimGroup);
    ASSERT((AnimID>=0) && (AnimID<AnimGroup->NAnimations));

	pAnim = &AnimGroup->AnimInfo[AnimID];
	C = EXPORTBITS_GETANGLETORADIANS(pAnim->ExportBits);

    *FirstMoveDir = pAnim->MoveDirAtFrame0*C; 
    *LastMoveDir  = pAnim->MoveDirAtFrameN*C; 
}

///////////////////////////////////////////////////////////////////////////
inline
void ANIM_GetFirstLastRootDir( s32				AnimID,
							   radian*			FirstRootDir,
							   radian*			LastRootDir,
							   anim_group*		AnimGroup )
{
	anim_info* pAnim;
    f32 C;

    ASSERT(AnimGroup);
    ASSERT((AnimID>=0) && (AnimID<AnimGroup->NAnimations));

	pAnim = &AnimGroup->AnimInfo[AnimID];
	C = EXPORTBITS_GETANGLETORADIANS(pAnim->ExportBits);

    *FirstRootDir = 0.0f; //pAnim->RootDirAtFrame0*C; 
    *LastRootDir  = pAnim->RootDirAtFrameN*C; 
}


///////////////////////////////////////////////////////////////////////////
// INLINED ANIMATION BLENDING FUNCTIONS
///////////////////////////////////////////////////////////////////////////
void	BlendRadian( radian& rDest, radian& rFrom, radian& rTo, f32 BlendAmount );
void	BlendRadian3( radian3& rDest, radian3& rFrom, radian3& rTo, f32 BlendAmount );
radian3	BlendUsingQuaternions( const radian3& From, const radian3& To, f32 t );
void	MatToRotXZY( const matrix4& M, radian3& R );
void	RotToQuat( const radian3& R, quaternion& Q );
void	QuatToRot( const quaternion& Q, radian3& R );
f32		QuatToYaw( const quaternion& Q );
void	MirrorQuat( const quaternion& Q, quaternion& MirQuat );
void	MirrorRot( const radian3& R, radian3& MirRot );
void	MirrorTrans( const vector3& T, vector3& MirTrans );


///////////////////////////////////////////////////////////////////////////
//
//	MirrorQuat
//
//		This function mirros a quaternions rotation on the YZ plane of X=0.
//
inline void MirrorQuat( const quaternion& Q, quaternion& MirQuat )
{
	MirQuat.W		= Q.W;
	MirQuat.X		= Q.X;
	MirQuat.Y		= -Q.Y;
	MirQuat.Z		= -Q.Z;
}

///////////////////////////////////////////////////////////////////////////
inline void MirrorRot( const radian3& R, radian3& MirRot )
{
	MirRot.Pitch	=  R.Pitch;
	MirRot.Yaw		= -R.Yaw;
	MirRot.Roll		= -R.Roll;
}

///////////////////////////////////////////////////////////////////////////
inline void MirrorTrans( const vector3& T, vector3& MirTrans )
{
	MirTrans.X		= -T.X;
	MirTrans.Y		= T.Y;
	MirTrans.Z		= T.Z;
}

//==========================================================================
inline void BlendRadian( radian& rDest, radian& rFrom, radian& rTo, f32 BlendAmount )
{
    radian DR; // Radian delta

	DR = rTo - rFrom;
	while( DR >  R_180 )	DR -= R_360;
	while( DR < -R_180 )	DR += R_360;
	rDest = rFrom + ( BlendAmount*DR );
}

//==========================================================================
inline void BlendRadian3( radian3& rDest, radian3& rFrom, radian3& rTo, f32 BlendAmount )
{
	BlendRadian( rDest.Pitch, rFrom.Pitch, rTo.Pitch, BlendAmount );
	BlendRadian( rDest.Yaw, rFrom.Yaw, rTo.Yaw, BlendAmount );
	BlendRadian( rDest.Roll, rFrom.Roll, rTo.Roll, BlendAmount );
}

//==========================================================================
inline void MatToRotXZY( const matrix4& M, radian3& R )
{
	// compute RZ
	f32 s	= M.M[0][1];
	if (s > 1.0f) s= 1.0f;
	if (s <-1.0f) s=-1.0f;
	R.Roll  = (f32)x_asin(s);

	// compute RX,RY
	f32 CZ = x_cos(R.Roll);
	if ( (CZ>0.0001f) || (CZ<-0.0001f) )
	{
		R.Pitch	= x_atan2( -M.M[2][1], M.M[1][1] );
		R.Yaw	= x_atan2( -M.M[0][2], M.M[0][0] );
	}
	else
	{
		R.Pitch	= x_atan2( M.M[1][2], -M.M[1][0] );
		R.Yaw	= 0;
	}
}

//==========================================================================
inline void RotXZYToMat( matrix4& M, const radian3& R )
{
    register f32 sx, sy, sz;
    register f32 cx, cy, cz;
    register f32 cxcy,sxcy,cxsy,sxsy;

    // calc basic values
	x_sincos( R.Pitch, sx, cx );
	x_sincos( R.Yaw, sy, cy );
	x_sincos( R.Roll, sz, cz );

    cxcy = cx*cy;
    sxcy = sx*cy;
    sxsy = sx*sy;
    cxsy = cx*sy;

    M.M[0][0] = cy*cz;
    M.M[1][0] = -cxcy*sz + sxsy;
    M.M[2][0] = sxcy*sz + cxsy;
    M.M[0][1] = sz;
    M.M[1][1] = cx*cz;
    M.M[2][1] = -sx*cz;
    M.M[0][2] = -sy*cz;
    M.M[1][2] = cxsy*sz + sxcy;
    M.M[2][2] = -sxsy*sz + cxcy;

    //---	set the translation
    M.M[3][0] = M.M[3][1] = M.M[3][2] =
    M.M[0][3] = M.M[1][3] = M.M[2][3] = 0.0f;
    M.M[3][3] = 1.0f;    
}

//==========================================================================
inline void RotZXYToQuat( const radian3& R, quaternion& Q )
{
#if 1

	f32 Sp, Cp;
	f32 Sy, Cy;
	f32 Sr, Cr;
	f32 CpCr;
	f32 SpSr;
	f32 SrCp;
	f32 SpCr;

	x_sincos( R.Pitch*0.5f, Sp, Cp );
	x_sincos( R.Yaw*0.5f, Sy, Cy );
	x_sincos( R.Roll*0.5f, Sr, Cr );

	CpCr = Cp*Cr;
	SpSr = Sp*Sr;
	SrCp = Sr*Cp;
	SpCr = Sp*Cr;

	Q.X = Cy*SpCr + Sy*SrCp;
	Q.Y = Sy*CpCr - Cy*SpSr;
	Q.Z = Cy*SrCp - Sy*SpCr;
	Q.W = Cy*CpCr + Sy*SpSr;

#else

	Q.Identity();
	Q.RotateZ( R.Roll );
	Q.RotateX( R.Pitch );
	Q.RotateY( R.Yaw );

#endif
}

//==========================================================================
inline void RotXZYToQuat( const radian3& R, quaternion& Q )
{
#if 1

	f32 sx, cx;
	f32	sy, cy;
	f32 sz, cz;
	f32	cycz;
	f32	cysz;
	f32	sycz;
	f32	sysz;

	x_sincos( R.Pitch*0.5f, sx, cx );
	x_sincos( R.Yaw*0.5f, sy, cy );
	x_sincos( R.Roll*0.5f, sz, cz );

	cycz = cy*cz;
	cysz = cy*sz;
	sycz = sy*cz;
	sysz = sy*sz;

	Q.X = sx*cycz + cx*sysz;
	Q.Y = sx*cysz + cx*sycz;
	Q.Z = cx*cysz - sx*sycz;
	Q.W = cx*cycz - sx*sysz;

#else

	Q.Identity();
	Q.RotateX( R.Pitch );
	Q.RotateZ( R.Roll );
	Q.RotateY( R.Yaw );

#endif
}

//==========================================================================
inline void QuatToRotZXY( const quaternion& Q, radian3& R )
{
#if 1
	f32	S, C;
	f32	tx, ty, tz;

	tx = 2.0f*Q.X;
	ty = 2.0f*Q.Y;
	tz = 2.0f*Q.Z;

	S = ty*Q.Z - tx*Q.W;
	if		( S < -1.0f )	S = -1.0f;
	else if	( S >  1.0f )	S =  1.0f;
	R.Pitch = x_asin( -S );

	C = x_cos( R.Pitch );
	if( (C>0.0001f) || (C<-0.0001f) )
	{
		R.Yaw	= x_atan2( tx*Q.Z + ty*Q.W, 1.0f - (tx*Q.X + ty*Q.Y) );
		R.Roll	= x_atan2( tx*Q.Y + tz*Q.W, 1.0f - (tx*Q.X + tz*Q.Z) );
	}
	else
	{
		R.Yaw	= 0.0f;
		R.Roll	= x_atan2( tx*Q.Z - ty*Q.W, 1.0f - (ty*Q.Y + tx*Q.Z) );
	}
#else
	matrix4 M4;
	Q.BuildMatrix( M4 );
	R = M4.GetRotation();
#endif
}

//==========================================================================
inline void QuatToRotXZY( const quaternion& Q, radian3& R )
{
#if 1
	f32	S, C;
	f32	tx, ty, tz;

	tx	= 2.0f*Q.X;
	ty	= 2.0f*Q.Y;
	tz	= 2.0f*Q.Z;

	// compute RZ
	S	= tx*Q.Y+tz*Q.W;
	if (S > 1.0f) S= 1.0f;
	if (S <-1.0f) S=-1.0f;
	R.Roll  = (f32)x_asin(S);

	// compute RX,RY
	C = x_cos(R.Roll);
	if ( (C>0.0001f) || (C<-0.0001f) )
	{
		R.Pitch	= x_atan2( tx*Q.W-ty*Q.Z, 1.0f-(tx*Q.X+tz*Q.Z) );
		R.Yaw	= x_atan2( ty*Q.W-tx*Q.Z, 1.0f-(ty*Q.Y+tz*Q.Z) );
	}
	else
	{
		R.Pitch	= x_atan2( ty*Q.Z+tx*Q.W, tz*Q.W-tx*Q.Y );
		R.Yaw	= 0;
	}
#else
	matrix4 M4;
	Q.BuildMatrix( M4 );
	MatToRotXZY( M4, R );
#endif
}

//==========================================================================
inline void RotToQuat( const radian3& R, quaternion& Q )
{
	RotXZYToQuat( R, Q );
}

//==========================================================================
inline void QuatToRot( const quaternion& Q, radian3& R )
{
	QuatToRotXZY( Q, R );
}


//==========================================================================
inline void RotXZYToRotZXY( const radian3& R, radian3& R2 )
{
	quaternion Q;
	RotXZYToQuat( R, Q );
	QuatToRotZXY( Q, R2 );
}

//==========================================================================
inline f32 QuatToYaw( const quaternion& Q )
{
	radian3 R;

#if 0 // ZXY

	f32	S, C;
	f32	tx, ty;

	tx = 2.0f*Q.X;
	ty = 2.0f*Q.Y;

	S = ty*Q.Z - tx*Q.W;
	if		( S < -1.0f )	S = -1.0f;
	else if	( S >  1.0f )	S =  1.0f;
	R.Pitch = x_asin( -S );

	C = x_cos( R.Pitch );
	if( (C>0.0001f) || (C<-0.0001f) )
		R.Yaw	= x_atan2( tx*Q.Z + ty*Q.W, 1.0f - (tx*Q.X + ty*Q.Y) );
	else
		R.Yaw	= 0.0f;

#else // XZY

	f32	S, C;
	f32	tx, ty, tz;

	tx	= 2.0f*Q.X;
	ty	= 2.0f*Q.Y;
	tz	= 2.0f*Q.Z;

	// compute RZ
	S	= tx*Q.Y+tz*Q.W;
	if (S > 1.0f) S= 1.0f;
	if (S <-1.0f) S=-1.0f;
	R.Roll  = (f32)x_asin(S);

	// compute RX,RY
	C = x_cos(R.Roll);
	if ( (C>0.0001f) || (C<-0.0001f) )
		R.Yaw	= x_atan2( ty*Q.W-tx*Q.Z, 1.0f-(ty*Q.Y+tz*Q.Z) );
	else
		R.Yaw	= 0;

#endif

	return R.Yaw;
}

//==========================================================================
inline radian3 BlendUsingQuaternions( const radian3& From, const radian3& To, f32 t )
{
    quaternion  Q1;
    quaternion  Q2;
    quaternion  slerp;
//    matrix4     M4;
    radian3     result;

    ASSERT( (t >= 0.0f) && (t <= 1.0f) );

	//---	convert the rotations to quaternions
	RotToQuat( From, Q1 );
	RotToQuat( To, Q2 );

	//---	interpolate the quats
	slerp = BlendLinear( Q1, Q2, t, NORMALIZE_QUAT_ON_BLEND );

	//---	convert the interpolated value back to rotations
	QuatToRot( slerp, result );

    return result;
}

///////////////////////////////////////////////////////////////////////////
//  FINISHED
///////////////////////////////////////////////////////////////////////////


#endif

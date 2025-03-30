///////////////////////////////////////////////////////////////////////////
//
//  AM_Export.h
//
///////////////////////////////////////////////////////////////////////////
#ifndef _AM_EXPORT_H_
#define _AM_EXPORT_H_

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

//---	uncompressed data type flags
#define	ANIMINFO_FLAG_GLOBALROTS	0x01	// rotations are local/global (global do not need to be combined with parent rotations) (root is always local)
#define	ANIMINFO_FLAG_WORLDROTS		0x02	// rotations are frame/world relative to this frame's face dir OR are global rotations.
#define	ANIMINFO_FLAG_QUATERNION	0x04	// the exported rotations are quaternion values rather than euler - the W values are not exported unless the ANIMINFO_FLAG_QUAT_W flag is also set
#define	ANIMINFO_FLAG_QUAT_W		0x08	// there are 4 quat streams (X,Y,Z,W) instead of 3 quat streams (X,Y,Z) streams per bone.

//---	other flags
#define	ANIMINFO_FLAG_UNCOMPRESSED	0x10	// the data is compressed
#define	ANIMINFO_FLAG_QUATBLEND		0x20	// force quaternion blending on non-quaternion export.
#define	ANIMINFO_FLAG_PROPQUATBLEND	0x40	// force quaternion blending for props on non-quaternion export

enum
{
	ANIM_MAX_PROPS			= 8,
	ANIM_MAX_POINTS			= 8,
	ANIM_MAX_USER_STREAMS	= 8,
};

///////////////////////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////////////////////

#define	EXPORTBITS_GETANGLETORADIANS(bits)		(R_360 / (f32)(1L<<(bits)))
#define	EXPORTBITS_GETVALUETOQUATERNION(bits)	(1.0f / (f32)((1L<<(s32)(bits))/2 - 1))
#define	EXPORTBITS_GETVALUETOQUATERNION_W(bits)	(1.0f / (f32)((1L<<(s32)(bits)) - 1))


///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
struct anim_info
{
    s32 BlockOffset;                // Offset in bytes to beginning of comp data
    s16 NFrames;                    // Num frames in the animation
    s16 EventIndex;                 // Index into the event array to first event
    s16 BlockSizeIndex;             // Index into BlockSize array to first block
//    s16 RootDirAtFrame0;            // RootDir for first frame of animation
    s16 RootDirAtFrameN;            // RootDir for last frame of animation
    s16 MoveDirAtFrame0;            // MoveDir for first frame of animation
    s16 MoveDirAtFrameN;            // MoveDir for last frame of animation
//    s16 TransXAtFrame0;             // TransX at first frame of animation in 12.4
    s16 TransYAtFrame0;             // TransY at first frame of animation in 12.4
//    s16 TransZAtFrame0;             // TransZ at first frame of animation in 12.4
    s16 TransXAtFrameN;             // TransX at last  frame of animation in 12.4
    s16 TransYAtFrameN;             // TransY at last  frame of animation in 12.4
    s16 TransZAtFrameN;             // TransZ at last  frame of animation in 12.4
    s16 ID;                         // Index of this animation in the array
    u16 UserFlags;                  // User defined flags
	u16	ExportNFrames;				// Number of exported frames
	u8	ExportBits;					// Bits of resolution in exported angles AND quaternion values 4:4
    u8  NEvents;                    // Num Events in animation
	u8	PropsActive;				// Props intended to be used with this character
    u8  PropsStreamsPresent;        // Which of the 8 props are present in anim
    u8  PointsPresent;              // Which of the 8 points are present in anim
    u8  UserStreamsPresent;         // Which user streams are present in anim
    u8  NStreams;                   // Total number of streams this anim has
	u8	Flags;						// Animation export flags
	u8	pad2;
	u8	pad3;
}; // anim_info

///////////////////////////////////////////////////////////////////////////
struct anim_event
{
    s16 Frame;                      // Frame event occurs on in (1/60s)
    s16 EventID;                    // Event ID
    s16 PX,PY,PZ;                   // Position of event in 12.4 fixed
    s16 RPX,RPY,RPZ;                // Position of root in 12.4 fixed
}; // anim_event

///////////////////////////////////////////////////////////////////////////
struct anim_bone
{
    char        BoneName[20];       // Bone name, all uppercase
    vector3     OriginToBone;       // Global translation from origin to bone pos
    vector3     ParentToBone;       // Local translation from parent to bone pos
    s16         BoneID;             // Index of this bone in array
    s16         ParentID;           // Index of parent bone in array or -1
    s16         MirrorID;           // This bone's mirror image lhand = rhand
}; // anim_bone

///////////////////////////////////////////////////////////////////////////
struct prop_def
{
	enum EFlags
	{
		ACTIVE			= (1<<0),
		SCALED			= (1<<1),
		MIRRORED		= (1<<2),
		STATIC_POS		= (1<<3),
		STATIC_ROT		= (1<<4),
		FACEREL_POS		= (1<<5),
		FACEREL_ROT		= (1<<6),
		MIRRORWITHANIM	= (1<<7),
	};

//	xbool		bActive;
//	xbool		bScaled;
//	xbool		bStaticPos;
//	xbool		bStaticRot;
//	xbool		bFacingRelPos;
//	xbool		bFacingRelRot;

	u16			Flags;
	s16			BoneID;
	vector3		Pos;
	radian3		Rot;
	quaternion	Quat;
}; // prop_def

///////////////////////////////////////////////////////////////////////////
struct anim_group
{
    void*       AnimCompData;       // Ptr to compressed binary data. FilePtr for non-primary storage
    s32         Version;            // Version number
    char        Name[32];           // Group name
    u8*         AnimBlockSize;      // In multiples of 16 bytes
    anim_event* AnimEvent;          // Events
    anim_info*  AnimInfo;           // Info on each animation
    anim_bone*  Bone;               // Skeleton animations use
    s32         NAnimBytes;         // Size of compressed animation data
    s16         NAnimations;        // Num animations in group
    s16         NAnimEvents;        // Total events in group
    s16         NAnimBlocks;        // Total compressed anim blocks in group
    s16         NAnimStreams;       // Total streams each anim has
    s16         NBones;             // Total bones each anim uses
    s16         MaxStreams;         // Max streams ever needed
	s16			FramesPerBlock;		// frames per compressed block in the characters compressed motion
	s16			pad;

	prop_def	PropDefs[ANIM_MAX_PROPS];
}; // anim_group


///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////
#endif // _AM_EXPORT_H_

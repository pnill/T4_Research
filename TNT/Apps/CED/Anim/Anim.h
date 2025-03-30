///////////////////////////////////////////////////////////////////////////
//
//  ANIM.H
//
///////////////////////////////////////////////////////////////////////////
#ifndef _ANIM_H_
#define _ANIM_H_

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////
#include "skel.h"

///////////////////////////////////////////////////////////////////////////
// DEFINES / MACROS
///////////////////////////////////////////////////////////////////////////

//---	resolution of angled exported
#define	ANGLE_BITS			16
#define	ANGLE_360			(1<<ANGLE_BITS)
#define	ANGLE_180			(ANGLE_360/2)

#define ANIM_FLAG_DIRTY                 (((u32)1)<<0)
#define ANIM_FLAG_INTERRUPTABLE         (((u32)1)<<1)
#define ANIM_FLAG_MIRROR_LEFT_HANDED    (((u32)1)<<2)
#define ANIM_FLAG_MIRROR_RIGHT_HANDED   (((u32)1)<<3)
#define	ANIM_FLAG_QUATERNION			(((u32)1)<<4)

#define	MAX_STREAMS			256

#define	STREAM_TYPE_INDEX(s)	((s)/32)
#define	STREAM_TYPE_MASK(s)		(1<<((s)&31))

///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

//typedef struct _skel skel;

typedef struct
{
    char Name[32];
    s16  Frame;     //  Which 1/60th does the event land on.
    s16  PosX;      //  X Pos * 16  -   Gives 1/16th inch accuracy
    s16  PosY;      //  Y Pos * 16  -   and 170ft range from beginning
    s16  PosZ;      //  Z Pos * 16  -   pos of animation.
} SAnimEvent;

struct anim
{
    char            Name[32];										// Animation name
    u32             Flags;											// Always need flags
    s32             StartFrame;										// Time of first frame in anim
    s32             NFrames;										// Number of 1/60th frames in anim
    s32             NBones;											// Number of bones in anim
    s32             NStreams;										// Total streams of info in anim
	u32				StreamType[STREAM_TYPE_INDEX(MAX_STREAMS-1)+1];	// Bitset, one bit per stream.  T/F is is a rotation stream.
																	// this is needed for blending purposes.
    s16*            Stream;											// Ptr to raw stream data
    s32             NEvents;										// Number of events in this anim
    SAnimEvent*     Event;											// Ptr to events.  Events are in frame order.
    s16             Version;										// Version of this animation

	anim() : Stream(NULL),
			 NBones(0) {}
};

///////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
///////////////////////////////////////////////////////////////////////////
//
//  InitInstance - Unused anim structures should be cleared to a valid state.
//  CopyAnim     - A deep copy is available for undo or duplication.  
//  KillInstance - Frees any malloced data and places anim in valid state.
//
//  The dirty bit is set anytime an operation changes animation data.
//  The dirty bit must be cleared manually.
//
///////////////////////////////////////////////////////////////////////////

// BASIC OPERATIONS
void ANIM_InitInstance          (anim* Anim);
void ANIM_KillInstance          (anim* Anim);

void ANIM_CopyAnim              (anim* AnimDst, anim* AnimSrc);
void ANIM_CopyAnimHeader        (anim* AnimDst, anim* AnimSrc);
void ANIM_CopyAnimStreams       (anim* AnimDst, anim* AnimSrc);

// IMPORT / EXPORT
err  ANIM_ImportAMC             (anim* Anim, 
                                 const char* FileName, 
                                 CSkel* Skel, 
                                 s16 Version);
err  ANIM_ExportAMC             (anim* Anim, CSkel* Skel, const char* FileName);

// EVENTS
void ANIM_AddEvent              (anim* Anim, s32 Frame, const char* EventName, vector3  P);
void ANIM_GetEvent              (anim* Anim, const char* EventName, SAnimEvent* EventDst);
void ANIM_DelEvent              (anim* Anim, const char* EventName);


// STREAM VALUES
s32		ANIM_AddStream			(anim* Anim, xbool bRotationStream);
void	ANIM_AddStream			(anim* Anim, s32 NewStreamIndex, xbool bRotationStream);
s32		ANIM_AddStreams			(anim* Anim, s32 Count, xbool bRotationStream);
void	ANIM_AddStreams			(anim* Anim, s32 NewStreamIndex, s32 NumStreams, xbool bRotationStreams);
void	ANIM_DelStream			(anim* Anim, s32 Index );
void	ANIM_DelStreams			(anim* Anim, s32 Index, s32 Count );

void	ANIM_ChangeNumFrames	(anim* Anim, s32 NewNumFrames );

inline
xbool	ANIM_StreamIsRotation	(anim* Anim, s32 Stream)
	{ return Anim->StreamType[STREAM_TYPE_INDEX(Stream)]&STREAM_TYPE_MASK(Stream) ? TRUE : FALSE; }
inline
void	ANIM_SetStreamType		( anim* Anim, s32 Stream, xbool bRotation )
{ 
	if( bRotation )
		Anim->StreamType[STREAM_TYPE_INDEX(Stream)] |= STREAM_TYPE_MASK(Stream);
	else
		Anim->StreamType[STREAM_TYPE_INDEX(Stream)] &= ~STREAM_TYPE_MASK(Stream);
}

void	ANIM_ConvertToQuat		(anim* Anim);
void	ANIM_ConvertToEuler		(anim* Anim);

void	ANIM_SetStreamValue		(anim* Anim, s32 Frame, s32 Stream, s16  DataValue);
void	ANIM_GetStreamValue		(anim* Anim, s32 Frame, s32 Stream, s16* DataValue);
void	ANIM_SetRotation		(anim* Anim, s32 Frame, s32 Stream, f32  RotValue);
f32		ANIM_GetRotation		(anim* Anim, s32 Frame, s32 Stream);
void	ANIM_SetQuatValue		(anim* Anim, s32 Frame, s32 Stream, f32  QuatValue);
f32		ANIM_GetQuatValue		(anim* Anim, s32 Frame, s32 Stream);
void	ANIM_SetRotValue		(anim* Anim, s32 Frame, s32 Stream, s32  RotValue);
s32		ANIM_GetRotValue		(anim* Anim, s32 Frame, s32 Stream);
void	ANIM_SetTransValue		(anim* Anim, s32 Frame, vector3 * Trans);
void	ANIM_GetTransValue		(anim* Anim, s32 Frame, vector3 * Trans);
void	ANIM_SetMiscValue		(anim* Anim, s32 Frame, s32 Stream, s32 MiscDataValue);
s32		ANIM_GetMiscValue		(anim* Anim, s32 Frame, s32 Stream);

void	ANIM_MinimizeAngleDiff	(anim* Anim, s32 AngleStream, s32 Angle360);
void	ANIM_DuplicateLastFrame	(anim* Anim);

// FLAGS
void ANIM_SetFlag               (anim* Anim, u32 FlagType, xbool FlagOn);
xbool ANIM_GetFlag               (anim* Anim, u32 FlagType);

// HAS THE ANIM BEEN CHANGED
xbool ANIM_IsDirty               (anim* Anim);
void ANIM_ClearDirtyBit         (anim* Anim);
void ANIM_SetDirtyBit           (anim* Anim);


///////////////////////////////////////////////////////////////////////////
//  FINISHED
///////////////////////////////////////////////////////////////////////////
#endif
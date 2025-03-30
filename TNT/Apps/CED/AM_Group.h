///////////////////////////////////////////////////////////////////////////
//
//  AM_GROUP.h
//
///////////////////////////////////////////////////////////////////////////
#ifndef _AM_GROUP_H_
#define _AM_GROUP_H_

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_math.h"


///////////////////////////////////////////////////////////////////////////
// C PLUS PLUS
///////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

typedef struct
{
    s32 BlockOffset;                // Offset in bytes to beginning of comp data
    s16 NFrames;                    // Num frames in the animation
    s16 EventIndex;                 // Index into the event array to first event
    s16 BlockSizeIndex;             // Index into BlockSize array to first block
    s16 RootDirAtFrame0;            // RootDir for first frame of animation
    s16 RootDirAtFrameN;            // RootDir for last frame of animation
    s16 MoveDirAtFrame0;            // MoveDir for first frame of animation
    s16 MoveDirAtFrameN;            // MoveDir for last frame of animation
    s16 TransXAtFrame0;             // TransX at first frame of animation in 12.4
    s16 TransYAtFrame0;             // TransY at first frame of animation in 12.4
    s16 TransZAtFrame0;             // TransZ at first frame of animation in 12.4
    s16 TransXAtFrameN;             // TransX at last  frame of animation in 12.4
    s16 TransYAtFrameN;             // TransY at last  frame of animation in 12.4
    s16 TransZAtFrameN;             // TransZ at last  frame of animation in 12.4
    s16 ID;                         // Index of this animation in the array
    u16 UserFlags;                  // User defined flags
    u8  NEvents;                    // Num Events in animation
    u8  PropsPresent;               // Which of the 8 props are present in anim
    u8  PointsPresent;              // Which of the 8 points are present in anim
    u8  UserStreamsPresent;         // Which user streams are present in anim
    u8  NStreams;                   // Total number of streams this anim has
    u8  PAD[1];
} anim_info;

typedef struct
{
    s16 Frame;                      // Frame event occurs on in (1/60s)
    s16 EventID;                    // Event ID
    s16 PX,PY,PZ;                   // Position of event in 12.4 fixed
    s16 RPX,RPY,RPZ;                // Position of root in 12.4 fixed
} anim_event;

typedef struct
{
    char        BoneName[20];       // Bone name, all uppercase
    vector3d    OriginToBone;       // Global translation from origin to bone pos
    vector3d    ParentToBone;       // Local translation from parent to bone pos
    s16         BoneID;             // Index of this bone in array
    s16         ParentID;           // Index of parent bone in array or -1
    s16         MirrorID;           // This bone's mirror image lhand = rhand
} anim_bone;

typedef struct
{
    void*       AnimCompData;       // Ptr to compressed binary data. FilePtr on N64
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

} anim_group;

///////////////////////////////////////////////////////////////////////////
// C PLUS PLUS
///////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
} ;
#endif

///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////
#endif

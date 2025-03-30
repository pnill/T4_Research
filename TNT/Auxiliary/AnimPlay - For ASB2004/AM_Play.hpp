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
#include "AM_Group.hpp"
#include "AnimPlay.hpp"

///////////////////////////////////////////////////////////////////////////
// DEFINES / MACROS
///////////////////////////////////////////////////////////////////////////

#define STREAM_TX_OFFSET        0
#define STREAM_TY_OFFSET        1
#define STREAM_TZ_OFFSET        2
#define STREAM_MOVEDIR_OFFSET   3
#define STREAM_ROOTDIR_OFFSET   4

#define ANIM_FRAMES_PER_BLOCK   32

#define ANIM_FLAGS_INTERPOLATE	(1<<0)
#define ANIM_FLAGS_MIRROR		(1<<1)
#define ANIM_FLAGS_BLENDING		(1<<2)

//#define ANGLE_360               65536
//#define ANGLE_180               (ANGLE_360/2)

//#define ANGLE_TO_RAD            (R_360/(f32)ANGLE_360)
//#define RAD_TO_ANGLE            ((f32)ANGLE_360/R_360)

#define	VALUE_TO_INCHES			(1.0f/16.0f)

///////////////////////////////////////////////////////////////////////////
// PRIMARY OR SECONDARY STORAGE
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PS2
    #define ANIMPLAY_PRIMARY_STORAGE
#endif

#ifdef TARGET_DOLPHIN
    #define ANIMPLAY_PRIMARY_STORAGE
#endif

#ifdef TARGET_XBOX
    #define ANIMPLAY_PRIMARY_STORAGE
#endif

#ifdef TARGET_PC
    #define ANIMPLAY_PRIMARY_STORAGE
#endif


///////////////////////////////////////////////////////////////////////////
// PRIVATE PROTOTYPES
///////////////////////////////////////////////////////////////////////////

void    AMCACHE_InitModule          (s32            TotalStaticBuffers, 
                                     s32            TotalDynamicBuffers, 
                                     s32            MaxStreams,
									 s32			MaxFramesPerBlock);

void    AMCACHE_KillModule          (void);

void    AMCACHE_LockBlock           (mocap_player* Anim);

void    AMCACHE_UnlockBlock         (mocap_player* Anim);

void    AMSKL_GetBoneMatrices       (anim_group*    AnimGroup,
                                     matrix4*       DestMs,
                                     radian3*       BoneRots,
                                     xbool          Mirror,
                                     f32            WorldScale,
                                     radian         WorldYaw,
                                     vector3*       WorldPos);

void    AMSKL_GetBoneMatrix         (anim_group*    AnimGroup,
                                     matrix4*       DestM,
                                     radian3*       BoneRots,
                                     xbool          Mirror,
                                     s32            BoneIndex,
                                     f32            WorldScale,
                                     radian         WorldYaw,
                                     vector3*       WorldPos);

void    ANIM_GetFrameDataPtrs       (mocap_player* Anim,
                                     s16**        DataPtr0,
                                     s16**        DataPtr1,
                                     f32*         Frac);        

void    ANIM_CompRelativePt         (f32            Scale,
                                     vector3*       StartWorldPos,
                                     radian         StartWorldRootDir,
                                     vector3*       AnimTargetPos,
                                     radian         AnimTargetRootDir,
                                     vector3*       AnimBasePos,
                                     radian         AnimBaseRootDir,
                                     vector3*       FinalWorldPos,
                                     radian*        FinalWorldRootDir);

void    ANIM_SetYaw                 (mocap_player*       This,
                                     radian             Yaw);

///////////////////////////////////////////////////////////////////////////
//  FINISHED
///////////////////////////////////////////////////////////////////////////


#endif

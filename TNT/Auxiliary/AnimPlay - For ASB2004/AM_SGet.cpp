///////////////////////////////////////////////////////////////////////////
//
// AM_SGET.C
//
///////////////////////////////////////////////////////////////////////////

#include "AM_Play.hpp"
#include "AM_Group.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"

///////////////////////////////////////////////////////////////////////////

mocap_player::mocap_player ( void )
{
	Init( NULL );
}

///////////////////////////////////////////////////////////////////////////

mocap_player::~mocap_player ( void )
{
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::Init   ( anim_group* pAnimGroup )
{
    x_memset(this,0,sizeof(mocap_player));
    m_AnimInfo    = NULL;
    m_AnimGroup   = pAnimGroup;
    m_BlockFrame0 = 1;
    m_BlockFrameN = -1;
    m_WorldScale  = 1.0f;
    m_Flags       = ANIM_FLAGS_INTERPOLATE;
    m_CurrentCycle  = 0;
    m_CurrentFrame  = 0;
	m_CurrentExportFrame = 0;
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::Kill           (void)
{
    SetAnimation(-1,FALSE);
    x_memset(this,0,sizeof(mocap_player));
}

///////////////////////////////////////////////////////////////////////////

s32     mocap_player::GetTotalFrames         (void)
{
    
    ASSERT(m_AnimInfo);
    return m_AnimInfo->NFrames;
}

///////////////////////////////////////////////////////////////////////////

f32     mocap_player::GetCurrentFrame        (void)
{
    
    return m_CurrentFrame;
}

///////////////////////////////////////////////////////////////////////////

s32     mocap_player::GetCurrentCycle        (void)
{
    
    return m_CurrentCycle;
}

///////////////////////////////////////////////////////////////////////////

s32     mocap_player::GetAnimID              (void)
{
    
    ASSERT(m_AnimInfo);
    return m_AnimInfo->ID;
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::GetWorldPosition       (vector3&           WorldPos)
{
    WorldPos = m_WorldPos;
}

///////////////////////////////////////////////////////////////////////////

radian  mocap_player::GetWorldMoveDir        (void)
{
    
    return m_WorldMoveDir;
}

///////////////////////////////////////////////////////////////////////////

radian  mocap_player::GetWorldFinalMoveDir        (void)
{
    radian RawFinalMoveDir;
    
    ASSERT(m_AnimInfo);

    RawFinalMoveDir = m_AnimInfo->MoveDirAtFrameN * m_AngleToRadians;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        RawFinalMoveDir = -RawFinalMoveDir;

    return m_WorldYaw + RawFinalMoveDir;
}

///////////////////////////////////////////////////////////////////////////

radian  mocap_player::GetWorldTotalMoveDir   (void)
{
    
    return m_WorldTotalMoveDir;
}

///////////////////////////////////////////////////////////////////////////

radian  mocap_player::GetWorldRootDir        (void)
{
    
    return m_WorldRootDir;
}

///////////////////////////////////////////////////////////////////////////

radian  mocap_player::GetWorldFinalRootDir        (void)
{
    radian RawFinalRootDir;
    
    ASSERT(m_AnimInfo);

    RawFinalRootDir = m_AnimInfo->RootDirAtFrameN * m_AngleToRadians;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        RawFinalRootDir = -RawFinalRootDir;

    return m_WorldYaw + RawFinalRootDir;
}

///////////////////////////////////////////////////////////////////////////

void    ANIM_GetFirstLastMoveDir   ( s32                AnimID,
                                     radian*            FirstMoveDir,
                                     radian*            LastMoveDir,
									 anim_group*		AnimGroup )
{
	anim_info* pAnim;
    f32 C;

    ASSERT(AnimGroup);
    ASSERT((AnimID>=0) && (AnimID<AnimGroup->NAnimations));

	pAnim = &AnimGroup->AnimInfo[AnimID];
	C = R_360/(f32)(1L<<pAnim->ExportBits);

    *FirstMoveDir = pAnim->MoveDirAtFrame0*C; 
    *LastMoveDir  = pAnim->MoveDirAtFrameN*C; 
}

///////////////////////////////////////////////////////////////////////////

void    ANIM_GetFirstLastRootDir   ( s32                AnimID,
                                     radian*            FirstRootDir,
                                     radian*            LastRootDir,
									 anim_group*		AnimGroup )
{
	anim_info* pAnim;
    f32 C;

    ASSERT(AnimGroup);
    ASSERT((AnimID>=0) && (AnimID<AnimGroup->NAnimations));

	pAnim = &AnimGroup->AnimInfo[AnimID];
	C = R_360/(f32)(1L<<pAnim->ExportBits);

    *FirstRootDir = pAnim->RootDirAtFrame0*C; 
    *LastRootDir  = pAnim->RootDirAtFrameN*C; 
}

///////////////////////////////////////////////////////////////////////////

f32     mocap_player::GetWorldScale          (void)
{
    
    return m_WorldScale;
}

///////////////////////////////////////////////////////////////////////////

xbool   mocap_player::IsMirrored             (void)
{
    
    return (m_Flags & ANIM_FLAGS_MIRROR) ? (TRUE):(FALSE);
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetYaw                 (radian             Yaw)
{
    // Move yaw into good range
    Yaw = x_ModAngle(Yaw);
    //while (Yaw >= R_360) Yaw -= R_360;
    //while (Yaw <  0)     Yaw += R_360;
    m_WorldYaw = Yaw;

    // Update Angles
    if (m_Flags & ANIM_FLAGS_MIRROR) 
    {
        m_WorldMoveDir      = Yaw - m_RawMoveDir;
        m_WorldTotalMoveDir = Yaw - m_TotalMoveDir;
        m_WorldRootDir      = Yaw - m_RawRootDir;
    }
    else
    {
        m_WorldMoveDir      = Yaw + m_RawMoveDir;
        m_WorldTotalMoveDir = Yaw + m_TotalMoveDir;
        m_WorldRootDir      = Yaw + m_RawRootDir;
    }
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetWorldMoveDir        (radian             WorldMoveDir)
{
    radian RawMoveDir;
    
    
    WorldMoveDir = x_ModAngle( WorldMoveDir );
    //while (WorldMoveDir >= R_360) WorldMoveDir -= R_360;
    //while (WorldMoveDir <  0)     WorldMoveDir += R_360;

    RawMoveDir = m_RawMoveDir;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        RawMoveDir = -RawMoveDir;

    SetYaw( WorldMoveDir - RawMoveDir);
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetWorldFinalMoveDir   (radian             WorldFinalMoveDir)
{
    radian RawFinalMoveDir;
    
    ASSERT(m_AnimInfo);
    
    WorldFinalMoveDir = x_ModAngle( WorldFinalMoveDir );
    //while (WorldFinalMoveDir >= R_360) WorldFinalMoveDir -= R_360;
    //while (WorldFinalMoveDir <  0)     WorldFinalMoveDir += R_360;

    RawFinalMoveDir = m_AnimInfo->MoveDirAtFrameN * m_AngleToRadians;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        RawFinalMoveDir = -RawFinalMoveDir;

    SetYaw( WorldFinalMoveDir - RawFinalMoveDir);
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetWorldTotalMoveDir   (radian             WorldTotalMoveDir)
{
    radian RawTotalMoveDir;
    
    ASSERT(m_AnimInfo);
    
    WorldTotalMoveDir = x_ModAngle( WorldTotalMoveDir );
    //while (WorldTotalMoveDir >= R_360) WorldTotalMoveDir -= R_360;
    //while (WorldTotalMoveDir <  0)     WorldTotalMoveDir += R_360;

    RawTotalMoveDir = m_TotalMoveDir;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        RawTotalMoveDir = -RawTotalMoveDir;

    SetYaw( WorldTotalMoveDir - RawTotalMoveDir );
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetWorldRootDir        (radian             WorldRootDir)
{
    radian RawRootDir;
    
    
    WorldRootDir = x_ModAngle( WorldRootDir );
    //while (WorldRootDir >= R_360) WorldRootDir -= R_360;
    //while (WorldRootDir <  0)     WorldRootDir += R_360;

    RawRootDir = m_RawRootDir;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        RawRootDir = -RawRootDir;

    SetYaw( WorldRootDir - RawRootDir );
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetWorldFinalRootDir   (radian             WorldFinalRootDir)
{
    radian RawFinalRootDir;
    
    
    WorldFinalRootDir = x_ModAngle( WorldFinalRootDir );
    //while (WorldFinalRootDir >= R_360) WorldFinalRootDir -= R_360;
    //while (WorldFinalRootDir <  0)     WorldFinalRootDir += R_360;

    RawFinalRootDir = m_AnimInfo->RootDirAtFrameN * m_AngleToRadians;
    if (m_Flags & ANIM_FLAGS_MIRROR) 
        RawFinalRootDir = -RawFinalRootDir;

    SetYaw( WorldFinalRootDir - RawFinalRootDir);
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetWorldScale          (f32                WorldScale)
{
    
    ASSERT((WorldScale >= 0.0f) && (WorldScale < 10.0f));
    m_WorldScale = WorldScale;
}

///////////////////////////////////////////////////////////////////////////

void    mocap_player::SetWorldPosition       (const vector3&           WorldPos)
{
    ASSERT((WorldPos.X >= -100000.0f) && (WorldPos.X <= 100000.0f));
    ASSERT((WorldPos.Y >= -100000.0f) && (WorldPos.Y <= 100000.0f));
    ASSERT((WorldPos.Z >= -100000.0f) && (WorldPos.Z <= 100000.0f));
    m_WorldPos = WorldPos;
}


///////////////////////////////////////////////////////////////////////////

radian  mocap_player::GetWorldYaw           (void)
{
    return m_WorldYaw;
}

///////////////////////////////////////////////////////////////////////////

s32         ANIM_GetNFrames         (s32                AnimID,
									 anim_group*		AnimGroup)
{
    ASSERT(AnimGroup);
    ASSERT((AnimID >= 0) && (AnimID < AnimGroup->NAnimations));
    return AnimGroup->AnimInfo[AnimID].NFrames;
}

///////////////////////////////////////////////////////////////////////////

s32         ANIM_GetNBones			( anim_group*		AnimGroup )
{
    ASSERT(AnimGroup);
    return AnimGroup->NBones;
}

///////////////////////////////////////////////////////////////////////////

void    ANIM_GetBonePos            ( s32 BoneIndex, vector3& BonePos, anim_group* AnimGroup )
{
    ASSERT(AnimGroup);
    ASSERT((BoneIndex>=0) && (BoneIndex<AnimGroup->NBones));
    BonePos = AnimGroup->Bone[ BoneIndex ].OriginToBone;
}

///////////////////////////////////////////////////////////////////////////

char*   ANIM_GetBoneName           ( s32                BoneIndex,
									 anim_group*		AnimGroup )
{
    ASSERT(AnimGroup);
    ASSERT((BoneIndex>=0) && (BoneIndex<AnimGroup->NBones));
    return AnimGroup->Bone[BoneIndex].BoneName;
}

///////////////////////////////////////////////////////////////////////////

s32    ANIM_GetBoneIndex      ( char*			BoneName,
								anim_group*		AnimGroup)
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
xbool  ANIM_WhenIsEvent        (s32				AnimID,
                                s16				EventID,
                                f32*			Frame,
								anim_group*		AnimGroup )
{
	anim_info* pAnim = &AnimGroup->AnimInfo[AnimID];
    anim_event* EV;
    anim_event* EVEnd;
    
    ASSERT(AnimGroup);
    ASSERT((AnimID >= 0) && (AnimID < AnimGroup->NAnimations));

    EV = AnimGroup->AnimEvent + pAnim->EventIndex;
	EVEnd = &EV[pAnim->NEvents];
    while (EV != EVEnd)
	{
		if (EV->EventID == EventID)
		{
			*Frame = (f32)(EV->Frame);
			return TRUE;
		}
		EV++;
	}

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////

s32    ANIM_GetMaxStreams      ( anim_group*		AnimGroup)
{
    ASSERT(AnimGroup);
    return AnimGroup->MaxStreams;
}
 
///////////////////////////////////////////////////////////////////////////

xbool   mocap_player::IsPropPresent          (s32                PropIndex)
{
    
    ASSERT((PropIndex>=0) && (PropIndex<8));
    ASSERT(m_AnimInfo);
    return (((u32)(m_AnimInfo->PropsPresent)) & (1<<PropIndex)) ? (TRUE):(FALSE);
}

///////////////////////////////////////////////////////////////////////////

xbool   mocap_player::GetUserFlag            (s32                FlagIndex)
{
    
    ASSERT(m_AnimInfo);
    ASSERT((FlagIndex>=0) && (FlagIndex<16));
    return (m_AnimInfo->UserFlags & (1<<FlagIndex)) ? (TRUE):(FALSE);
}

///////////////////////////////////////////////////////////////////////////

u32     mocap_player::GetUserFlags           (void)
{
    
    ASSERT(m_AnimInfo);
    return (u32)m_AnimInfo->UserFlags;
}

///////////////////////////////////////////////////////////////////////////
s32  ANIM_GetNumAnimations   ( anim_group*		AnimGroup)
{
    ASSERT(AnimGroup);
    return AnimGroup->NAnimations;
}
///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

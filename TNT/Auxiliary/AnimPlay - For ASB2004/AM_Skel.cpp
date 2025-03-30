///////////////////////////////////////////////////////////////////////////
//
//  AM_SKEL.C
//
///////////////////////////////////////////////////////////////////////////
#include "x_debug.hpp"
#include "am_play.hpp"

///////////////////////////////////////////////////////////////////////////

static inline void BuildLocalMatrix   (matrix4*    M, 
                                       radian3*    R, 
                                       vector3*    T, 
                                       matrix4*   PM,
                                       radian     WY)
{
    register f32 sx, sy, sz;
    register f32 cx, cy, cz;
    register f32 cxcy,sxcy,sysz;

    // calc basic values
    sx = x_fastsin(R->Pitch);
    cx = x_fastcos(R->Pitch);
    sy = x_fastsin(R->Yaw+WY);
    cy = x_fastcos(R->Yaw+WY);
    sz = x_fastsin(R->Roll);
    cz = x_fastcos(R->Roll);

    cxcy = cx*cy;
    sxcy = sx*cy;
    sysz = sy*sz;
    
    // fill out 3x3 rotations
    M->M[0][0] = (cz*cy);
    M->M[1][0] = (-sz*cxcy + sy*sx);
    M->M[2][0] = (sz*sxcy + sy*cx);
    M->M[0][1] = (sz);
    M->M[1][1] = (cz*cx);
    M->M[2][1] = (-cz*sx);
    M->M[0][2] = (-sy*cz);
    M->M[1][2] = (sysz*cx + sxcy);
    M->M[2][2] = (-sysz*sx + cxcy);

    // solve translations
    M->M[3][0] = (PM->M[0][0]*T->X) + (PM->M[1][0]*T->Y) + (PM->M[2][0]*T->Z) + PM->M[3][0];
    M->M[3][1] = (PM->M[0][1]*T->X) + (PM->M[1][1]*T->Y) + (PM->M[2][1]*T->Z) + PM->M[3][1];
    M->M[3][2] = (PM->M[0][2]*T->X) + (PM->M[1][2]*T->Y) + (PM->M[2][2]*T->Z) + PM->M[3][2];

    // fill out edges
    M->M[0][3] = M->M[1][3] = M->M[2][3] = 0;
    M->M[3][3] = 1;
}

///////////////////////////////////////////////////////////////////////////

void AMSKL_GetBoneMatrices  (anim_group*    AnimGroup,
                             matrix4*       DestMs,
                             radian3*       BoneRots,
                             xbool          Mirror,
                             f32            WorldScale,
                             radian         WorldYaw,
                             vector3*       WorldPos)
{
    matrix4*    ParentMatrix;
    radian3     Rot;
    vector3     Trans;
    matrix4     WorldOrientM;
    s32         i;
    matrix4*    M;

    // Build WorldOrientM 
    WorldOrientM.Identity();
    //M4_Identity(&WorldOrientM);

    // Loop through all bones hitting parents before children
    for (i=0; i<AnimGroup->NBones; i++)
    {
        // Decide on rotation and translation values for bone
        if (Mirror)
        {
            Rot.Pitch	=  BoneRots[i].Pitch;
            Rot.Yaw		= -BoneRots[i].Yaw;
            Rot.Roll	= -BoneRots[i].Roll;
            Trans	= AnimGroup->Bone[ AnimGroup->Bone[i].MirrorID ].ParentToBone;
        }
        else
        {
			Rot		= BoneRots[i];
            Trans   = AnimGroup->Bone[ i ].ParentToBone;
        }

        // Reach back to earlier bones and decide on parent matrix
        if (AnimGroup->Bone[i].ParentID != -1)
            ParentMatrix = (DestMs) + AnimGroup->Bone[i].ParentID;
        else
            ParentMatrix = &WorldOrientM;

        // Build matrix for this bone
        BuildLocalMatrix( (DestMs) + i, &Rot, &Trans, ParentMatrix, WorldYaw );
    }

    for (i=0; i<AnimGroup->NBones; i++)
    {
        M = DestMs + i;
//        M->M[0][0] *= WorldScale;  M->M[1][0] *= WorldScale;  M->M[2][0] *= WorldScale;
//        M->M[0][1] *= WorldScale;  M->M[1][1] *= WorldScale;  M->M[2][1] *= WorldScale;
//        M->M[0][2] *= WorldScale;  M->M[1][2] *= WorldScale;  M->M[2][2] *= WorldScale;

        M->M[3][0] = (M->M[3][0]*WorldScale) + WorldPos->X;
        M->M[3][1] = (M->M[3][1]*WorldScale) + WorldPos->Y;
        M->M[3][2] = (M->M[3][2]*WorldScale) + WorldPos->Z;
    }
}

///////////////////////////////////////////////////////////////////////////

void AMSKL_GetBoneMatrix    (anim_group*    AnimGroup,
                             matrix4*       DestM,
                             radian3*      BoneRots,
                             xbool          Mirror,
                             s32            BoneIndex,
                             f32            WorldScale,
                             radian         WorldYaw,
                             vector3*      WorldPos)
{
    s32         i;
    matrix4     M[2];
    radian3    R;
    s32         MIndex;
    s32         BoneStack[30];
    s32         BoneStackIndex;
    s32         BoneID;

    ASSERT(AnimGroup && DestM && WorldPos && BoneRots);
    ASSERT(AnimGroup->NBones < 30);

    // Build path from bone to root
	i = BoneIndex;
    BoneStackIndex=0;
    while (i!=-1)
    {
        // Push bone on stack
        BoneStack[BoneStackIndex] = i;
        BoneStackIndex++;
        i = AnimGroup->Bone[i].ParentID;
    }

    // Build WorldOrientM into M[0]
    //M4_Identity(&M[0]);
    M[0].Identity();

    // Traverse path from root to bone and build matrices
    MIndex = 0;
    for (i=0; i<BoneStackIndex; i++)
    {
        // Get bone we are talking about
        BoneID = BoneStack[BoneStackIndex-1-i];

        // Get rotation values
        if (Mirror)
        {
            R.Pitch	=  BoneRots[AnimGroup->Bone[BoneID].MirrorID].Pitch;
            R.Yaw	= -BoneRots[AnimGroup->Bone[BoneID].MirrorID].Yaw;
            R.Roll	= -BoneRots[AnimGroup->Bone[BoneID].MirrorID].Roll;
        }
        else
        {
            R = BoneRots[BoneID];
        }

        // Build local matrix
        BuildLocalMatrix(&M[1-MIndex],&R,&AnimGroup->Bone[BoneID].ParentToBone,&M[MIndex],WorldYaw);
        MIndex = 1-MIndex;
    }

    *DestM = M[MIndex];

//    DestM->M[0][0] *= WorldScale;  DestM->M[1][0] *= WorldScale;  DestM->M[2][0] *= WorldScale;
//    DestM->M[0][1] *= WorldScale;  DestM->M[1][1] *= WorldScale;  DestM->M[2][1] *= WorldScale;
//    DestM->M[0][2] *= WorldScale;  DestM->M[1][2] *= WorldScale;  DestM->M[2][2] *= WorldScale;
    DestM->M[3][0] = (DestM->M[3][0]*WorldScale) + WorldPos->X;
    DestM->M[3][1] = (DestM->M[3][1]*WorldScale) + WorldPos->Y;
    DestM->M[3][2] = (DestM->M[3][2]*WorldScale) + WorldPos->Z;
}

///////////////////////////////////////////////////////////////////////////

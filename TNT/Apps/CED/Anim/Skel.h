///////////////////////////////////////////////////////////////////////////
//
//  SKEL.H
//
///////////////////////////////////////////////////////////////////////////
#ifndef _SKEL_H_
#define _SKEL_H_

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////
#include "x_math.hpp"


///////////////////////////////////////////////////////////////////////////
// DEFINES / MACROS
///////////////////////////////////////////////////////////////////////////

#define BONE_DOF_RX     (1<<0)
#define BONE_DOF_RY     (1<<1)
#define BONE_DOF_RZ     (1<<2)
#define BONE_DOF_TX     (1<<3)
#define BONE_DOF_TY     (1<<4)
#define BONE_DOF_TZ     (1<<5)

///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

struct anim;

struct SSkelBone
{
    char        BoneName[32];
    char        ParentName[32];
    s32         BoneID;
    s32         ParentID;
    s32         MirrorID;
    u32         Flags;
    vector3     OriginToBone;       //  Bone - Origin
    vector3     ParentToBone;       //  Bone - Parent


    // VALUES FROM ASF
    radian3     Axis;               //  ASF axis alignment values
    vector3     BoneVector;         //  Bonelength and direction
    u32         DOFOrder[6];

};

////////////////////////////////////////////////////////////////////////
// The CLASS
////////////////////////////////////////////////////////////////////////
class CSkel
{
public:
    ////////////////////////////////////////////////////////////////////////
    // Basic operations
    ////////////////////////////////////////////////////////////////////////
             CSkel( void );
    virtual ~CSkel( void );
    
    void CopySkel       ( CSkel* SkelSrc );

    ////////////////////////////////////////////////////////////////////////
    // Import/export
    ////////////////////////////////////////////////////////////////////////
    xbool       ImportASF       ( const char* FileName );

    ////////////////////////////////////////////////////////////////////////
    // Modifications
    ////////////////////////////////////////////////////////////////////////
    s32         FindBone        ( const char* BoneName );
    const char* GetBoneName     ( s32 BoneID );
    void        DelBone         ( s32 BoneID );
    void        DelDummyBones   ( void );
    void        ClearAxis       ( void );

    ////////////////////////////////////////////////////////////////////////
    // Misc. routines
    ////////////////////////////////////////////////////////////////////////
    void        CalcGlobalPosFromLocal  ( void );
    void        CalcLocalPosFromGlobal  ( void );
    void        DecideMirrorBones       ( void );

    ////////////////////////////////////////////////////////////////////////
    // Data
    ////////////////////////////////////////////////////////////////////////
    s32			m_NBones;
    SSkelBone*	m_pBone;
    f32			m_TransScale;
};

/*
///////////////////////////////////////////////////////////////////////////
// PROTOTYPES
///////////////////////////////////////////////////////////////////////////

// BASIC OPERATIONS
void SKEL_InitInstance  (skel* Skel);
void SKEL_CopySkel      (skel* SkelDst, skel* SkelSrc);
void SKEL_KillInstance  (skel* Skel);

// IMPORT / EXPORT
err	SKEL_ImportASF		(skel* Skel, const char* FileName);
err	SKEL_ExportASF		(skel* Skel, const char* FileName);

// MODIFICATIONS
s32  SKEL_FindBone           (skel* Skel, const char* BoneName);
char* SKEL_GetBoneName       (skel* Skel, s32 BoneID);
void SKEL_DelBone            (skel* Skel, s32 BoneID);
void SKEL_DelDummyBones      (skel* Skel);
void SKEL_ClearAxis          (skel* Skel);

// MATRICES
void SKEL_BuildWorldOrient  (matrix4*   M, 
                             f32        Scale, 
                             f32        Tx, 
                             f32        Ty, 
                             f32        Tz,
                             radian     Rx, 
                             radian     Ry, 
                             radian     Rz);

void SKEL_BuildMatricesXZY  (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM);

void SKEL_BuildMatrices     (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM);

void SKEL_BuildMatricesBlend(skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame0,
                             s32        AnimFrame1,
                             f32        AnimFrameFrac,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM);

void SKEL_BuildMatrices2    (skel*      Skel,
                             radian3*   Rotations,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM);

void SKEL_GetBoneMatrix     (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrix,
                             matrix4*   WorldOrientM,
                             s32        BoneIndex);

void SKEL_BuildMatricesMir  (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM);

void SKEL_BuildMatricesMir2 (skel*      Skel,
                             radian3*   Rotations,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM);

void SKEL_GetBoneMatrixMir  (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrix,
                             matrix4*   WorldOrientM,
                             s32        BoneIndex);

void SKEL_DecideMirrorBones (skel*      Skel);
*/
///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////
#endif

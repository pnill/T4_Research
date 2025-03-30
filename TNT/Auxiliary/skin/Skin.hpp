////////////////////////////////////////////////////////////////////////////
//
// Skin.hpp
//
// Header for QSkin class
//
////////////////////////////////////////////////////////////////////////////

#ifndef SKIN_HPP_INCLUDED
#define SKIN_HPP_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_math.hpp"
#include "x_bitmap.hpp"

#include "AnimPlay.h"

#include "CommonSkinDefines.hpp"


////////////////////////////////////////////////////////////////////////////
// PLATFORM-SPECIFIC INCLUDES
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PS2 )
    #include "PS2/PS2_Skin.hpp"
#elif defined( TARGET_DOLPHIN )
    #include "GameCube/GC_Skin.hpp"
#elif defined( TARGET_XBOX )
    #include "XBOX/XBOX_Skin.hpp"
#elif defined( TARGET_PC ) && !defined( _CONSOLE )
    #include "PC/PC_Skin.hpp"
#endif


////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef xbool (*skin_prerender_fnptr)(s32 MeshID, u32 SkinData, u32 UserData1, u32 UserData2, u32 UserData3 );
typedef void  (*skin_finished_fnptr)( void );


////////////////////////////////////////////////////////////////////////////
// The QSkin class
////////////////////////////////////////////////////////////////////////////

class QSkin
{
public:
    ////////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    ////////////////////////////////////////////////////////////////////////
             QSkin( void );
             QSkin( char* SkinFile );
    virtual ~QSkin( void );

    ////////////////////////////////////////////////////////////////////////
    // Misc. Setup Functions
    ////////////////////////////////////////////////////////////////////////
    void SetupFromResource( char* filename );
    void SetupFromResource( X_FILE* pFilePtr );
    void SetupFromResource( byte* pByteStream );

    ////////////////////////////////////////////////////////////////////////
    // Skin info routines
    ////////////////////////////////////////////////////////////////////////
    const char* GetName( void );
    void        SetLightingLevel( s32 iLevel );

    ////////////////////////////////////////////////////////////////////////
    // Bone info routines
    ////////////////////////////////////////////////////////////////////////
    s32         GetNBones   ( void );
    s32         GetBoneIndex( const char* BoneName );
    const char* GetBoneName ( s32 BoneIndex );

    ////////////////////////////////////////////////////////////////////////
    // Mesh Info routines
    ////////////////////////////////////////////////////////////////////////
    s32         GetNMeshes  ( void );
    const char* GetMeshName ( s32 MeshIndex );
    s32         GetMeshIndex( const char* MeshName );

    ////////////////////////////////////////////////////////////////////////
    // Visibility info
    ////////////////////////////////////////////////////////////////////////
    void SetMeshVisibility   ( s32 MeshIndex, xbool bVisible );
    void SetTextureVisibility( s32 MeshIndex, s32 TexID, xbool bVisible );

    ////////////////////////////////////////////////////////////////////////
    // Skin functions
    //
    //      It is expected that you will be responsible for setting the
    //      engine's render flags and settings. In order to help facilitate
    //      that, there are a couple of callback functions that will be
    //      called by FlushSkinBuffer().
    //
    //      The PreRender callback is where you should place any engine
    //      settings. If your callback function returns NULL, the mesh will
    //      be culled out, so this is also a way for disabling certain
    //      meshes in special-case situations.
    //
    //      The Finished callback is called at the end of FlushSkinBuffer
    //      to inform your app that the skin rendering is done, in case
    //      you need to restore engine settings or the like.
    //
    //      If you leave PreRender and Finished callbacks NULL, the skin
    //      will attempt to make an educated guess and set the render
    //      flags and blend modes for you. I wouldn't rely on this too much
    //      though...
    ////////////////////////////////////////////////////////////////////////
    static void SetPreRenderCallback( skin_prerender_fnptr Callback );
    static void SetFinishedCallback ( skin_finished_fnptr Callback );
    static void RenderSubMesh       ( u32 SkinData );

#ifdef TARGET_PS2
    static void ResendMatrices ( u32 SkinData );
#endif

    static void OpenSkinBuffer ( s32 MaxNMatrixSets, s32 MaxNSubMeshes );
    static void FlushSkinBuffer( void );

    void RenderSkin( matrix4*    pBoneMatrices,
                     vector3*    pBoneScales,
                     xbool       bMirror,
                     anim_group* pAnimGroup,
                     u32         UserData1,	// UserData1_AnimPlayerData
                     u32         UserData2, // UserData2_ShadowMatrices
                     u32         UserData3	// UserData3_AnimatedHandData
               );

    ////////////////////////////////////////////////////////////////////////
    // Morph target functions
    ////////////////////////////////////////////////////////////////////////
    s32     GetNMorphTargets   ( s32 MeshID );
    char*   GetMorphTargetName ( s32 MeshID, s32 TargetID );
    s32     GetMorphTargetIndex( s32 MeshID, const char* Name );
    void    SetActiveTargets   ( s32 MeshID, s32 NTargets, s32 Targets[], f32 Weights[] );

    ////////////////////////////////////////////////////////////////////////
    // Texture routines
    ////////////////////////////////////////////////////////////////////////
    s32             GetNTextures      ( void );
    const char*     GetTextureName    ( s32 TextureIndex );
    s32             GetTextureIndex   ( const char* TextureName );
    void            SetTexturePtrArray( x_bitmap** pTextureArray );

    ////////////////////////////////////////////////////////////////////////
    // Stat tracking
    ////////////////////////////////////////////////////////////////////////
    static void SetStatTrackers( s32* pNVerts, s32* pNTris, s32* pNBytes );

protected:
    ////////////////////////////////////////////////////////////////////////
    // Internal routines
    ////////////////////////////////////////////////////////////////////////
    void InitData( void );
    void KillData( void );

    ////////////////////////////////////////////////////////////////////////
    // Data common to all skins
    ////////////////////////////////////////////////////////////////////////
    static skin_prerender_fnptr     s_PreRenderCallback;
    static skin_finished_fnptr      s_FinishedCallback;

    ////////////////////////////////////////////////////////////////////////
    // Data for the skin
    ////////////////////////////////////////////////////////////////////////
    char                m_Name[SKIN_NAME_LENGTH];
    s32                 m_NBones;
    char*               m_pBoneNames;
    u32                 m_Flags;
    s32                 m_NMeshes;
    t_SkinMesh*         m_pMeshes;
    s32                 m_NSubMeshes;
    t_SkinSubMesh*      m_pSubMeshes;
    s32                 m_NMorphTargets;
    t_SkinMorphTarget*  m_pMorphTargets;
    s32                 m_NTextures;
    char*               m_pTextureNames;
    x_bitmap**          m_pTextureArray;

    byte*               m_pRawData;     //Raw data from skin file

    ////////////////////////////////////////////////////////////////////////
    // Stat tracking
    ////////////////////////////////////////////////////////////////////////

    static s32*         s_pStatNVerts;
    static s32*         s_pStatNTris;
    static s32*         s_pStatNBytes;


    ////////////////////////////////////////////////////////////////////////
    // PS2-specific functions and data
    ////////////////////////////////////////////////////////////////////////
#ifdef TARGET_PS2

protected:
    s32                 m_NCacheFrames;
    t_SkinCacheFrame*   m_pCacheFrames;
    s32                 m_NVertLoads;
    t_SkinVertLoad*     m_pVertLoads;
    s32                 m_NDeltaLocations;
    t_SkinDeltaLoc*     m_pDeltaLocations;
    s32                 m_NDeltaValues;
    t_SkinDeltaValue*   m_pDeltaValues;
    byte*               m_pPacketData;
    byte*               m_pDeltaData;

    static byte*        s_pSkinBuffer;
    static s32          s_NMtxGroupsInBuffer;
    static s32          s_MaxMtxGroups;
    static s32          s_MaxSubMeshes;
    static s32          s_SkinMicroCodeID;

    static void ActivateMicroCode ( void );
    static void RenderSubMeshGroup( t_SubMeshGroup* pGroup );

    byte* BuildMorphTargetData( s32 MeshID );
    void  BuildLightingMatrix( matrix4* pLightMatrix, matrix4* pColorMatrix );

public:
    void  RenderSkin( t_MatrixGroup* pMatrixGroup,
                      u32            UserData1  = 0, // UserData1_AnimPlayerData
                      u32            UserData2  = 0, // UserData2_ShadowMatrices
                      u32            UserData3  = 0, // UserData3_AnimatedHandData
                      s32            NShadowMtx = 0,
                      matrix4*       pShadowMtx = NULL );


	void  BuildMatrices( t_MatrixGroup*		pMatrixGroup,
						 const matrix4*		pBoneMatrices,
						 const vector3*		pBoneScales,
						 xbool				bMirrored,
						 const anim_group*	pAnimGroup );

protected:

	void BuildFinalBoneMatrices(	t_MatrixGroup*		pMatrixGroup,
									const matrix4*		pBoneMatrices,		
									const vector3*		pBoneScales,
									xbool				Mirrored,
									const anim_group*	pAnimGroup );
/*
    void  BuildMatrices( t_MatrixGroup* pMatrixGroup,
                         radian3*       pBoneRots,
                         vector3&       WorldPos,
                         radian         WorldYaw,
                         xbool          bMirrored,
                         f32            WorldScale,
                         vector3*       pBoneScales,
                         void*          pAnimGroupPtr );

    void  BuildMatrices( t_MatrixGroup* pMatrixGroup,
                         radian3*       pBoneRots,
                         vector3&       WorldPos,
                         matrix4&       WorldOrientation,
                         xbool          bMirrored,
                         f32            WorldScale,
                         vector3*       pBoneScales,
                         void*          pAnimGroupPtr );
*/

    ////////////////////////////////////////////////////////////////////////
    // PC/XBOX-specific functions and data
    ////////////////////////////////////////////////////////////////////////
#elif defined( TARGET_XBOX ) || (defined( TARGET_PC ) && !defined( _CONSOLE ))

protected:
    static s32 s_VertexShaderID;
    static s32 s_ShadowShaderID;
    static s32 s_PixelShaderID;
    static s32 s_PixelShader_CheatID;

    static void ActivateVertexShader( void );
    static void ActivateShadowShader( void );
    static void ActivatePixelShader( void );
    static void DeActivatePixelShader( void );

    void BuildLightingMatrix ( matrix4* pLightMatrix, matrix4* pColorMatrix );

public:

    void  RenderSkin( t_MatrixGroup* pMatrixGroup,
                      u32            UserData1  = 0, // UserData1_AnimPlayerData
                      u32            UserData2  = 0, // UserData2_ShadowMatrices
                      u32            UserData3  = 0, // UserData3_AnimatedHandData
                      s32            NShadowMtx = 0,
                      matrix4*       pShadowMtx = NULL,
                      u8*            AlphaLevels = NULL );

	void  BuildMatrices( t_MatrixGroup*		pMatrixGroup,
						 const matrix4*		pBoneMatrices,
						 const vector3*		pBoneScales,
						 xbool				bMirrored,
						 const anim_group*	pAnimGroup );

protected:

	void BuildFinalBoneMatrices(	t_MatrixGroup*		pMatrixGroup,
									const matrix4*		pBoneMatrices,		
									const vector3*		pBoneScales,
									xbool				bMirrored,
									const anim_group*	pAnimGroup );

/*
    void  BuildMatrices( t_MatrixGroup* pMatrixGroup,
                         radian3*       pBoneRots,
                         vector3&       WorldPos,
                         radian         WorldYaw,
                         xbool          bMirrored,
                         f32            WorldScale,
                         vector3*       pBoneScales,
                         void*          pAnimGroupPtr );

    void BuildMatrices(  t_MatrixGroup* pMatrixGroup,
                         radian3*       pBoneRots,
                         vector3&       WorldPos,
                         matrix4&       WorldOrientation,
                         xbool          bMirrored,
                         f32            WorldScale,
                         vector3*       pBoneScales,
                         void*          pAnimGroupPtr );
*/
#if defined( TARGET_XBOX )
    void  DrawSkeleton( SXBOXMatrixGroup* pMatrixGroup, xbool bMirror, void* pAnimGroupPtr, f32 JointScale = 1.0f );
#endif

    ////////////////////////////////////////////////////////////////////////
    // GameCube-specific functions and data
    ////////////////////////////////////////////////////////////////////////
#elif defined( TARGET_DOLPHIN )

protected:
    u32                 m_DeltaDataSize;    // # of morph delta values for m_pDeltaValues
    byte*               m_pDeltaValues;     // X Y Z delta offsets for morph targets

    u32                 m_VertexDataSize;
    u32                 m_DListDataSize;
    byte*               m_pVertexData;
    byte*               m_pDListData;

    static void RenderSubMeshGroup( t_SubMeshGroup* pGroup );
    static void SetupTEV( u32 SubMeshFlags );

public:

    void RenderSkin( GCMtxGroup*    pMtxData,
                     u32            UserData1       = 0,
                     u32            UserData2       = 0,
                     u32            UserData3       = 0,
                     s32            NShadowMtx      = 0,
                     GCPosMtxArray* pShadowMtxData  = NULL );


	void  BuildMatrices( GCMtxGroup*		pMtxData,
						 const matrix4*		pBoneMatrices,
						 const vector3*		pBoneScales,
						 xbool				bMirrored,
						 const anim_group*	pAnimGroup,
                         s32				NShadowMtx      = 0,
                         matrix4*			pShadowMatrices = NULL,
                         GCPosMtxArray*		pShadowMtxData  = NULL );

protected:

	void BuildFinalBoneMatrices(	GCMtxGroup*			pMtxData,
									const matrix4*		pBoneMatrices,		
									const vector3*		pBoneScales,
									xbool				Mirrored,
									const anim_group*	pAnimGroup );

/*    void BuildMatrices( GCMtxGroup*    pMtxData,
                        radian3*       pBoneRots,
                        vector3&       WorldPos,
                        radian         WorldYaw,
                        xbool          bMirrored,
                        f32            WorldScale,
                        vector3*       pBoneScales,
                        void*          pAnimGroupPtr,
                        s32            NShadowMtx      = 0,
                        matrix4*       pShadowMatrices = NULL,
                        GCPosMtxArray* pShadowMtxData  = NULL );

    void BuildMatrices( GCMtxGroup*    pMtxData,
                        radian3*       pBoneRots,
                        vector3&       WorldPos,
                        GCPosMtx*      pWorldOrientation,
                        xbool          bMirrored,
                        f32            WorldScale,
                        vector3*       pBoneScales,
                        void*          pAnimGroupPtr,
                        s32            NShadowMtx      = 0,
                        matrix4*       pShadowMatrices = NULL,
                        GCPosMtxArray* pShadowMtxData  = NULL );
*/
#endif
};


////////////////////////////////////////////////////////////////////////////
// INLINES
////////////////////////////////////////////////////////////////////////////

inline void QSkin::SetPreRenderCallback( skin_prerender_fnptr Callback )
{
    s_PreRenderCallback = Callback;
}

//==========================================================================

inline void QSkin::SetFinishedCallback( skin_finished_fnptr Callback )
{
    s_FinishedCallback = Callback;
}

////////////////////////////////////////////////////////////////////////////

#endif // SKIN_HPP_INCLUDED

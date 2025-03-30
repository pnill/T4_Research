////////////////////////////////////////////////////////////////////////////
//
// Skin_Common.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "Skin.hpp"


////////////////////////////////////////////////////////////////////////////
// QSkin STATIC MEMBER VARAIBLES
////////////////////////////////////////////////////////////////////////////

skin_prerender_fnptr    QSkin::s_PreRenderCallback   = NULL;
skin_finished_fnptr     QSkin::s_FinishedCallback    = NULL;

s32*                    QSkin::s_pStatNVerts = NULL;
s32*                    QSkin::s_pStatNTris  = NULL;
s32*                    QSkin::s_pStatNBytes = NULL;


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

const char* QSkin::GetName( void )
{
    return m_Name;
}

//==========================================================================

void QSkin::SetLightingLevel( s32 iLevel )
{
	// temporarily disabling lighting levels
    m_Flags = iLevel;
}

//==========================================================================

s32 QSkin::GetNBones( void )
{
    return m_NBones;
}

//==========================================================================

s32 QSkin::GetBoneIndex( const char* BoneName )
{
    s32 i;

    ASSERT( BoneName );

    for( i = 0; i < m_NBones; i++ )
    {
        if( !x_stricmp( &m_pBoneNames[SKIN_BONE_NAME_LENGTH * i], BoneName ) )
        {
            return i;
        }
    }

    return -1;
}

//==========================================================================

const char* QSkin::GetBoneName( s32 BoneIndex )
{
    ASSERT( (BoneIndex >= 0) && (BoneIndex < m_NBones) );

    return &m_pBoneNames[SKIN_BONE_NAME_LENGTH * BoneIndex];
}

//==========================================================================

s32 QSkin::GetNMeshes( void )
{
    return m_NMeshes;
}

//==========================================================================

const char* QSkin::GetMeshName( s32 MeshIndex )
{
    ASSERT( (MeshIndex >= 0) && (MeshIndex < m_NMeshes) );

    return m_pMeshes[MeshIndex].Name;
}

//==========================================================================

s32 QSkin::GetMeshIndex( const char* MeshName )
{
    s32 i;

    ASSERT( MeshName );

    for( i = 0; i < m_NMeshes; i++ )
    {
        if( !x_stricmp( m_pMeshes[i].Name, MeshName ) )
            return i;
    }

    return -1;
}

//==========================================================================

void QSkin::SetMeshVisibility( s32 MeshIndex, xbool bVisible )
{
//    x_printf( "MeshIndex: %d\n", MeshIndex );
    if (!(MeshIndex >= 0 && MeshIndex < m_NMeshes))
        return;

    ASSERT( (MeshIndex >= 0) && (MeshIndex < m_NMeshes) );

    if( bVisible )
        m_pMeshes[MeshIndex].Flags |= SKIN_MESH_FLAG_VISIBLE;
    else
        m_pMeshes[MeshIndex].Flags &= ~SKIN_MESH_FLAG_VISIBLE;
//    x_printf( "MeshIndex: %d\n", MeshIndex );
}

//==========================================================================

void QSkin::SetTextureVisibility( s32 MeshIndex, s32 TexID, xbool bVisible )
{
    ASSERT( (MeshIndex >= 0) && (MeshIndex < m_NMeshes) );
    ASSERT( (TexID >= 0) && (TexID < m_NTextures) );

    s32 i;
    s32 LastSubMesh;

    LastSubMesh = (m_pMeshes[MeshIndex].FirstSubMesh + m_pMeshes[MeshIndex].NSubMeshes) - 1;

    if( bVisible )
    {
        for( i = m_pMeshes[MeshIndex].FirstSubMesh; i <= LastSubMesh; i++ )
        {
            if( m_pSubMeshes[i].TextureID == TexID )
            {
                m_pSubMeshes[i].Flags |= SKIN_SUBMESH_FLAG_VISIBLE;
            }
        }
    }
    else
    {
        for( i = m_pMeshes[MeshIndex].FirstSubMesh; i <= LastSubMesh; i++ )
        {
            if( m_pSubMeshes[i].TextureID == TexID )
            {
                m_pSubMeshes[i].Flags &= ~SKIN_SUBMESH_FLAG_VISIBLE;
            }
        }
    }
}

//==========================================================================

s32 QSkin::GetNMorphTargets( s32 MeshID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    return m_pMeshes[MeshID].NMorphTargets;
}

//==========================================================================

char* QSkin::GetMorphTargetName( s32 MeshID, s32 TargetID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (TargetID >= 0) && (TargetID < m_pMeshes[MeshID].NMorphTargets) );

    return m_pMorphTargets[ m_pMeshes[MeshID].FirstMorphTarget + TargetID ].Name;
}

//==========================================================================

s32 QSkin::GetMorphTargetIndex( s32 MeshID, const char* Name )
{
    s32 i;

    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    for( i = 0; i < m_pMeshes[MeshID].NMorphTargets; i++ )
    {
        if( !x_stricmp( m_pMorphTargets[m_pMeshes[MeshID].FirstMorphTarget + i].Name, Name ) )
        {
            return i;
        }
    }

    return -1;
}

//==========================================================================

void QSkin::SetActiveTargets( s32 MeshID,
                              s32 NTargets,
                              s32 Targets[],
                              f32 Weights[] )
{
    s32                 i;
    t_SkinMorphTarget*  pTarget;
    xbool               Default = TRUE;

    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & SKIN_MESH_FLAG_MORPH );

    //--- by default make all targets inactive
    for( i = 0; i < m_pMeshes[MeshID].NMorphTargets; i++ )
    {
        pTarget = &m_pMorphTargets[ m_pMeshes[MeshID].FirstMorphTarget + i ];
        pTarget->Active = FALSE;
        pTarget->Weight = 0.0f;
    }

    //--- loop through the targets, marking them as active or
    //    inactive and specifying the weights
    for( i = 0; i < NTargets; i++ )
    {
        ASSERT( (Targets[i] >= 0) && (Targets[i] < m_pMeshes[MeshID].NMorphTargets) );

        if( Weights[i] < 0.01f )
            continue;

        if( Targets[i] > 0 )
            Default = FALSE;

        pTarget = &m_pMorphTargets[ m_pMeshes[MeshID].FirstMorphTarget + Targets[i] ];
        pTarget->Active = TRUE;
        pTarget->Weight = Weights[i];
    }

    //--- try to do some optimization if we're just using the default morph
    if( Default )
    {
        pTarget = &m_pMorphTargets[ m_pMeshes[MeshID].FirstMorphTarget ];
        pTarget->Active = TRUE;
        pTarget->Weight = 1.0f;
    }
    else
    {
        //#### When we build the morph target data, we take a shortcut
        //     and point to the original morph target when the default target
        //     weight is 1.0f.
        //     This causes problems if you have a default target with
        //     a weight of 1.0f AND you have another target active with
        //     some weight. (Because morph target deltas can be additive).
        //     So...a hackish fix...if its not a default morph target,
        //     set the default weight to 0.0f.
        pTarget = &m_pMorphTargets[m_pMeshes[MeshID].FirstMorphTarget];
        pTarget->Active = FALSE;
        pTarget->Weight = 0.0f;
    }
}

//==========================================================================

s32 QSkin::GetNTextures( void )
{
    return m_NTextures;
}

//==========================================================================

const char* QSkin::GetTextureName( s32 TextureIndex )
{
    ASSERT( (TextureIndex >= 0) && (TextureIndex < m_NTextures) );

    return &m_pTextureNames[SKIN_TEXTURE_NAME_LENGTH];
}

//==========================================================================

s32 QSkin::GetTextureIndex( const char* TextureName )
{
    s32 i;

    ASSERT( TextureName );

    for( i = 0; i < m_NTextures; i++ )
    {
        if( !x_stricmp( &m_pTextureNames[SKIN_TEXTURE_NAME_LENGTH], TextureName ) )
        {
            return i;
        }
    }

    return -1;
}

//==========================================================================

void QSkin::SetTexturePtrArray( x_bitmap** pTextureArray )
{
    m_pTextureArray = pTextureArray;
}

//==========================================================================
// Stat tracking
//==========================================================================

void QSkin::SetStatTrackers( s32* pNVerts, s32* pNTris, s32* pNBytes )
{
    s_pStatNVerts = pNVerts;
    s_pStatNTris  = pNTris;
    s_pStatNBytes = pNBytes;
}

//==========================================================================

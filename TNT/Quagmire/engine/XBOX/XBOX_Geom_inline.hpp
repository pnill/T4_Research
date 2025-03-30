#ifndef __XBOX_GEOM_INLINE_HPP_INCLUDED__
#define __XBOX_GEOM_INLINE_HPP_INCLUDED__

#include "Q_XBOX.hpp"
#include "XBOX\XBOX_Video.hpp"

//==========================================================================
inline void QGeom::LockMesh( s32 MeshID )
{
    //Make sure that the mesh is valid.
    ASSERT( ( MeshID >= 0) && (MeshID < m_NMeshes ) );
    ASSERT( m_MeshLocked == -1 );

    // Lock the vertex buffer for this mesh.
    // Store the vertex buffer address in the local data pointer so you can manipulate it.
    DXWARN( m_pMeshes[MeshID].pD3DVerts->Lock(  0,
                                                m_pMeshes[MeshID].VertFormatSize  * m_pMeshes[MeshID].NVerts,
                                                (byte**)&m_pLockData,
                                                0) );

    m_MeshLocked = MeshID;
}

//==========================================================================
inline void QGeom::UnlockMesh( s32 MeshID )
{
    // Make sure that the mesh that you wish to unlock is locked already.
    ASSERT( m_MeshLocked != -1 );
    ASSERT( m_MeshLocked == MeshID );

    DXWARN( m_pMeshes[MeshID].pD3DVerts->Unlock() );
    
    // Clear the mesh locked maintenance variables.
    m_pLockData  = NULL;
    m_MeshLocked = -1;
}

//==========================================================================
inline void* QGeom::GetPosPtr( s32 MeshID, s32 VertIndex )
{
    SGeomMesh* pMesh = &m_pMeshes[MeshID];

    // Test to see if the mesh requested is locked first.
    ASSERT( m_MeshLocked != -1 );
    ASSERT( m_MeshLocked == MeshID );

    // Test to make sure that the vertex requested is valid for this mesh.
    ASSERT( (VertIndex >= 0) && (VertIndex < pMesh->NVerts) );

    // Depending on the vertex format for this mesh, grab the vertex position data.
    switch( pMesh->VertFormat )
    {
        case VERTEX_FORMAT_XYZ_ARGB_TEX1:
        {
            VertFormat_XYZ_ARGB_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_TEX1*)m_pLockData;
            return &pVertexBuffer[VertIndex].Pos;
            break;
        }

        case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1:
        {
            VertFormat_XYZ_ARGB_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1*)m_pLockData;
            return &pVertexBuffer[VertIndex].Pos;
            break;
        }

        case VERTEX_FORMAT_XYZ_NORM_TEX1:
        {
            VertFormat_XYZ_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_NORM_TEX1*)m_pLockData;
         
            return &pVertexBuffer[VertIndex].Pos;
            break;
        }

        case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3:
        {
            VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3*)m_pLockData;
         
            return &pVertexBuffer[VertIndex].Pos;
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }

    return NULL;
}

//==========================================================================

inline void* QGeom::GetUVPtr( s32 MeshID, s32 VertIndex )
{
    SGeomMesh* pMesh = &m_pMeshes[MeshID];

    // Test to see if the mesh requested is locked first.
    ASSERT( m_MeshLocked != -1 );
    ASSERT( m_MeshLocked == MeshID );

    // Test to make sure that the vertex requested is valid for this mesh.
    ASSERT( (VertIndex >= 0) && (VertIndex < pMesh->NVerts) );

    // Depending on the vertex format for this mesh, grab the vertex position data.
    switch( pMesh->VertFormat )
    {
        case VERTEX_FORMAT_XYZ_ARGB_TEX1:
        {
            VertFormat_XYZ_ARGB_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_TEX1*)m_pLockData;
            return &pVertexBuffer[VertIndex].UV;
            break;
        }

        case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1:
        {
            VertFormat_XYZ_ARGB_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1*)m_pLockData;
            return &pVertexBuffer[VertIndex].UV;
            break;
        }

        case VERTEX_FORMAT_XYZ_NORM_TEX1:
        {
            VertFormat_XYZ_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_NORM_TEX1*)m_pLockData;
         
            return &pVertexBuffer[VertIndex].UV;
            break;
        }

        case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3:
        {
            VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3*)m_pLockData;
         
            return &pVertexBuffer[VertIndex].UV1;
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }

    return NULL;
}

//==========================================================================
inline void* QGeom::GetColorPtr( s32 MeshID, s32 VertIndex )
{
    SGeomMesh* pMesh = &m_pMeshes[MeshID];

    // Test to see if the mesh requested is locked first.
    ASSERT( m_MeshLocked != -1 );
    ASSERT( m_MeshLocked == MeshID );

    // Test to make sure that the vertex requested is valid for this mesh.
    ASSERT( (VertIndex >= 0) && (VertIndex < pMesh->NVerts) );

    // Depending on the vertex format for this mesh, grab the vertex position data.
    switch( pMesh->VertFormat )
    {
        case VERTEX_FORMAT_XYZ_ARGB_TEX1:
        {
            VertFormat_XYZ_ARGB_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_TEX1*)m_pLockData;
            return &pVertexBuffer[VertIndex].Color;
            break;
        }

        case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1:
        {
            VertFormat_XYZ_ARGB_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1*)m_pLockData;
            return &pVertexBuffer[VertIndex].Color;
            break;
        }

        // There is an assert if you are using the VERTEX_FORMAT_XYZ_NORM_TEX1 since there is no color data in the
        // vertex data structure.
        case VERTEX_FORMAT_XYZ_NORM_TEX1:
        {
            ASSERT( 0 );
            break;
        }

        case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3:
        {
            VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3*)m_pLockData;
            return &pVertexBuffer[VertIndex].Color;
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }

    return NULL;
}

//==========================================================================
inline void* QGeom::GetNormalPtr( s32 MeshID, s32 VertIndex )
{
    SGeomMesh* pMesh = &m_pMeshes[MeshID];

    // Test to see if the mesh requested is locked first.
    ASSERT( m_MeshLocked != -1 );
    ASSERT( m_MeshLocked == MeshID );

    // Test to make sure that the vertex requested is valid for this mesh.
    ASSERT( (VertIndex >= 0) && (VertIndex < pMesh->NVerts) );

    // Depending on the vertex format for this mesh, grab the vertex position data.
    switch( pMesh->VertFormat )
    {
        case VERTEX_FORMAT_XYZ_ARGB_TEX1:
        {
            ASSERTS( FALSE, "Format does not have a Normal value." );
            break;
        }

        case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1:
        {
            VertFormat_XYZ_ARGB_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1*)m_pLockData;
         
            return &pVertexBuffer[VertIndex].Normal;
            break;
        }

        case VERTEX_FORMAT_XYZ_NORM_TEX1:
        {
            VertFormat_XYZ_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_NORM_TEX1*)m_pLockData;
         
            return &pVertexBuffer[VertIndex].Normal;
            break;
        }

        case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3:
        {
            VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3*)m_pLockData;
         
            return &pVertexBuffer[VertIndex].Normal;
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }

    return NULL;
}

inline s32 QGeom::GetTextureID( s32 MeshID, s32 SubMeshID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) && (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );
    return m_pSubMeshes[m_pMeshes[MeshID].FirstSubMesh+SubMeshID].TextureID[0];
}

//==========================================================================

inline void QGeom::SetTextureID( s32 MeshID, s32 SubMeshID, s32 TextureID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) && (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );
    m_pSubMeshes[m_pMeshes[MeshID].FirstSubMesh+SubMeshID].TextureID[0] = TextureID;
}

#endif // __XBOX_GEOM_INLINE_HPP_INCLUDED__
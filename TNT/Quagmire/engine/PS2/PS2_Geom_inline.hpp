#ifndef __PS2_GEOM_INLINE_HPP_INCLUDED__
#define __PS2_GEOM_INLINE_HPP_INCLUDED__
#else
#error "File " __FILE__ " has been included twice!"
#endif // __PS2_GEOM_INLINE_HPP_INCLUDED__

//==========================================================================
// Mesh data
//==========================================================================

//==========================================================================

inline void QGeom::LockMesh( s32 MeshID )
{
    ASSERT( ( MeshID >= 0) && (MeshID < m_NMeshes ) );
    ASSERT( !(m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED)  );

    ASSERT( ( m_pMeshes[MeshID].Flags & MESH_FLAG_CONTAINS_POSPTRS    ) ||
            ( m_pMeshes[MeshID].Flags & MESH_FLAG_CONTAINS_UVPTRS     ) ||
            ( m_pMeshes[MeshID].Flags & MESH_FLAG_CONTAINS_RGBAPTRS   ) ||
            ( m_pMeshes[MeshID].Flags & MESH_FLAG_CONTAINS_NORMALPTRS ) );

    m_pMeshes[MeshID].Flags |= MESH_FLAG_LOCKED;
}

//==========================================================================

inline void QGeom::UnlockMesh( s32 MeshID )
{
    ASSERT( ( MeshID >= 0) && (MeshID < m_NMeshes ) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );

    m_pMeshes[MeshID].Flags &= ~MESH_FLAG_LOCKED;
}

//==========================================================================

inline void* QGeom::GetPosPtr( s32 MeshID, s32 VertIndex )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );
    ASSERT( (VertIndex >= 0) && (VertIndex < m_pMeshes[MeshID].NVerts) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_CONTAINS_POSPTRS );

    return m_pMeshes[MeshID].pPositions[VertIndex];
}

//==========================================================================

inline void* QGeom::GetUVPtr( s32 MeshID, s32 VertIndex )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );
    ASSERT( (VertIndex >= 0) && (VertIndex < m_pMeshes[MeshID].NVerts) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_CONTAINS_UVPTRS );

    return m_pMeshes[MeshID].pUVs[VertIndex];
}

//==========================================================================

inline void* QGeom::GetColorPtr( s32 MeshID, s32 VertIndex )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );
    ASSERT( (VertIndex >= 0) && (VertIndex < m_pMeshes[MeshID].NVerts) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_CONTAINS_RGBAPTRS );

    return m_pMeshes[MeshID].pRGBAs[VertIndex];
}

//==========================================================================

inline void* QGeom::GetNormalPtr( s32 MeshID, s32 VertIndex )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );
    ASSERT( (VertIndex >= 0) && (VertIndex < m_pMeshes[MeshID].NVerts) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_CONTAINS_NORMALPTRS );

    return m_pMeshes[MeshID].pNormals[VertIndex];
}

//==========================================================================

inline s32 QGeom::GetTextureID( s32 MeshID, s32 SubMeshID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) && (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );
    return m_pSubMeshes[m_pMeshes[MeshID].FirstSubMesh+SubMeshID].TextureID;
}

//==========================================================================

inline void QGeom::SetTextureID( s32 MeshID, s32 SubMeshID, s32 TextureID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) && (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );
    m_pSubMeshes[m_pMeshes[MeshID].FirstSubMesh+SubMeshID].TextureID = TextureID;
}

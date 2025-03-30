////////////////////////////////////////////////////////////////////////////
//
// GC_Geom_inline.hpp
//
// inline functions for QGeom specific to GameCube
//
////////////////////////////////////////////////////////////////////////////

#ifndef GC_GEOM_INLINE_HPP_INCLUDED
#define GC_GEOM_INLINE_HPP_INCLUDED


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

inline void* QGeom::GetPosPtr( s32 MeshID, s32 VertIndex )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );
    ASSERT( (VertIndex >= 0) && (VertIndex < m_pMeshes[MeshID].NVerts) );

    if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_POS_ONLY )
    {
        return &((vector3*)m_pMeshes[MeshID].pVertData)[VertIndex];
    }
    else if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_NORMAL )
    {
        return &((t_GCVertNormal*)m_pMeshes[MeshID].pVertData)[VertIndex].Pos;
    }
    else if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_COLOR )
    {
        return &((t_GCVertColor*)m_pMeshes[MeshID].pVertData)[VertIndex].Pos;
    }
    else
    {   ASSERTS( FALSE, "Invalid Vertex format" );
        return NULL;
    }
}

//==========================================================================

inline void* QGeom::GetUVPtr( s32 MeshID, s32 VertIndex )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );
    ASSERT( (VertIndex >= 0) && (VertIndex < m_pMeshes[MeshID].NVerts) );

    if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_NORMAL )
    {
        return &((t_GCVertNormal*)m_pMeshes[MeshID].pVertData)[VertIndex].UV;
    }
    else if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_COLOR )
    {
        return &((t_GCVertColor*)m_pMeshes[MeshID].pVertData)[VertIndex].UV;
    }
    else
    {   ASSERTS( FALSE, "Vertex format has no UV data" );
        return NULL;
    }
}

//==========================================================================

inline void* QGeom::GetColorPtr( s32 MeshID, s32 VertIndex )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );
    ASSERT( (VertIndex >= 0) && (VertIndex < m_pMeshes[MeshID].NVerts) );

    if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_COLOR )
    {
        return &((t_GCVertColor*)m_pMeshes[MeshID].pVertData)[VertIndex].Color;
    }
    else
    {   ASSERTS( FALSE, "Vertex format has no Color data" );
        return NULL;
    }
}

//==========================================================================

inline void* QGeom::GetNormalPtr( s32 MeshID, s32 VertIndex )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );
    ASSERT( (VertIndex >= 0) && (VertIndex < m_pMeshes[MeshID].NVerts) );

    if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_NORMAL )
    {
        return &((t_GCVertNormal*)m_pMeshes[MeshID].pVertData)[VertIndex].Normal;
    }
    else
    {   ASSERTS( FALSE, "Vertex format has no Normal data" );
        return NULL;
    }
}

//==========================================================================

inline s32 QGeom::GetTextureID( s32 MeshID, s32 SubMeshID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) && (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );

    return m_pSubMeshes[ m_pMeshes[MeshID].FirstSubMesh + SubMeshID ].TextureID;
}

//==========================================================================

inline void QGeom::SetTextureID( s32 MeshID, s32 SubMeshID, s32 TextureID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) && (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );

    m_pSubMeshes[ m_pMeshes[MeshID].FirstSubMesh + SubMeshID ].TextureID = TextureID;
}


////////////////////////////////////////////////////////////////////////////

#endif // GC_GEOM_INLINE_HPP_INCLUDED

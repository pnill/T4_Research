#ifndef __Q_GEOM_INLINE_HPP_INCLUDED__
#define __Q_GEOM_INLINE_HPP_INCLUDED__
#else
#error "File " __FILE__ " has been included twice!"
#endif // __Q_GEOM_INLINE_HPP_INCLUDED__

//==========================================================================
// Mesh data
//==========================================================================

inline s32 QGeom::GetNVertsInMesh( s32 MeshID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    return m_pMeshes[MeshID].NVerts;
}


//==========================================================================
// Geometry Flags
//==========================================================================

inline xbool QGeom::GetFlag( s32 GeomFlagID )
{
    return ((m_Flags & GeomFlagID) ? TRUE : FALSE);
}

//==========================================================================

inline void QGeom::SetFlag( s32 GeomFlagID, xbool FlagValue )
{
    if ( FlagValue )
        m_Flags |= GeomFlagID;
    else
        m_Flags &= ~GeomFlagID;
}

//==========================================================================

inline void QGeom::SetFlags( s32 FlagMask )
{
    m_Flags = FlagMask;
}

//==========================================================================
// Mesh Flags
//==========================================================================

inline xbool QGeom::GetMeshFlag( s32 MeshID, s32 MeshFlagID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    return ((m_pMeshes[MeshID].Flags & MeshFlagID) ? TRUE : FALSE);
}

//==========================================================================

inline void QGeom::SetMeshFlag( s32 MeshID,
                                s32 MeshFlagID,
                                xbool FlagValue )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    if ( FlagValue )
        m_pMeshes[MeshID].Flags |= MeshFlagID;
    else
        m_pMeshes[MeshID].Flags &= ~MeshFlagID;
}

//==========================================================================

inline void QGeom::SetMeshFlags( s32 MeshID, s32 MeshFlagMask )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    m_pMeshes[MeshID].Flags = MeshFlagMask;
}

//==========================================================================
// SubMesh Flags
//==========================================================================

inline xbool QGeom::GetSubMeshFlag( s32 MeshID,
                                    s32 SubMeshID,
                                    s32 SubMeshFlagID )
{
    t_GeomSubMesh*  pSubMesh;

    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) &&
            (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );

    pSubMesh = &m_pSubMeshes[SubMeshID + m_pMeshes[MeshID].FirstSubMesh];

    return ((pSubMesh->Flags & SubMeshFlagID) ? TRUE : FALSE);
}

//==========================================================================

inline void QGeom::SetSubMeshFlag( s32 MeshID,
                                   s32 SubMeshID,
                                   s32 SubMeshFlagID,
                                   xbool FlagValue )
{
    t_GeomSubMesh*  pSubMesh;

    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) &&
            (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );

    pSubMesh = &m_pSubMeshes[SubMeshID + m_pMeshes[MeshID].FirstSubMesh];

    if ( FlagValue )
        pSubMesh->Flags |= SubMeshFlagID;
    else
        pSubMesh->Flags &= ~SubMeshFlagID;
}

//==========================================================================

inline void QGeom::SetSubMeshFlags( s32 MeshID,
                                    s32 SubMeshID,
                                    s32 SubMeshFlagMask )
{
    t_GeomSubMesh*  pSubMesh;

    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );
    ASSERT( (SubMeshID >= 0) &&
            (SubMeshID < m_pMeshes[MeshID].NSubMeshes) );

    pSubMesh = &m_pSubMeshes[SubMeshID + m_pMeshes[MeshID].FirstSubMesh];
    pSubMesh->Flags = SubMeshFlagMask;
}

//==========================================================================
// General Purpose Info
//==========================================================================

inline const char* QGeom::GetName( void )
{
    return m_Name;
}

//==========================================================================

inline s32 QGeom::GetNMeshes( void )
{
    return m_NMeshes;
}

//==========================================================================

inline s32 QGeom::GetNSubMeshes( void )
{
    return m_NSubMeshes;
}

//==========================================================================

inline const char* QGeom::GetMeshName( s32 MeshID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    return m_pMeshes[MeshID].Name;
}

//==========================================================================

inline s32 QGeom::GetMeshIndex( const char* MeshName )
{
    s32 i;
    s32 index = -1;

    ASSERT( MeshName );

    for ( i = 0; i < m_NMeshes; i++ )
    {
        if ( !x_strcmp( MeshName, m_pMeshes[i].Name ) )
        {
            index = i;
            break;
        }
    }

    return index;
}

//==========================================================================

inline f32 QGeom::GetMeshRadius( s32 MeshID )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    return m_pMeshes[MeshID].Radius;
}

//==========================================================================
// L2W matrix
//==========================================================================

inline matrix4 QGeom::GetL2W( void )
{
    return m_L2W;
}

//==========================================================================

inline void QGeom::SetL2W( matrix4& L2W )
{
    m_L2W = L2W;
}

inline void QGeom::SetMeshTexture( s32 MeshID, x_bitmap* Texture )
{
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    m_MeshTextureArray[MeshID] = Texture;
}

inline x_bitmap* QGeom::GetMeshTexture( s32 MeshID )
{
    return m_MeshTextureArray[MeshID];
}


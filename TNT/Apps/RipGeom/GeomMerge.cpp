////////////////////////////////////////////////////////////////////////////
// GeomMerge.cpp
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////

#include "x_plus.hpp"
#include "GeomMerge.h"
#include "AsciiMesh.h"

mesh::material_set *GetMaterialSet( mesh::object* rGeom )
{
    s32 MaterialSetID = rGeom->GetMaterialSetId( mesh::MESH_TARGET_XBOX );
    if( MaterialSetID == -1 )
    {
        MaterialSetID = rGeom->GetMaterialSetId( mesh::MESH_TARGET_GENERIC );
        ASSERT( MaterialSetID != -1 );
    }
    return &rGeom->m_pMaterialSets[ MaterialSetID ];
}

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

QGeomMerge::QGeomMerge( void )
{
    m_NGeoms = 0;
}

//==========================================================================

QGeomMerge::~QGeomMerge( void )
{
    s32 i;
    for ( i = 0; i < m_NGeoms; i++ )
    {
        delete m_pGeoms[i];
    }
}

//==========================================================================

void QGeomMerge::AddGeom( mesh::object* pObject )
{
    ASSERT( pObject );
    ASSERT( (m_NGeoms - 1) < MAX_NUM_MERGES );

    m_pGeoms[m_NGeoms] = new mesh::object();
    ASSERT( m_pGeoms[m_NGeoms] );

    CopyGeom( m_pGeoms[m_NGeoms], pObject );
    m_NGeoms++;
}

//==========================================================================

static
void RemoveTexture( mesh::object* pObj, s32 TexID )
{
    s32             i;
    mesh::texture*  pNewTexture = NULL;
    mesh::material_set* pMatSet = GetMaterialSet(pObj);

    //---   allocate space for the new texture array
    if ( (pMatSet->m_nTextures - 1) != 0 )
    {
        pNewTexture = new mesh::texture[pMatSet->m_nTextures - 1];
        ASSERT( pNewTexture );
    }

    //---   copy the textures over from the old array
    for ( i = 0; i < TexID; i++ )
        pNewTexture[i] = pMatSet->m_pTextures[i];
    for ( i = TexID + 1; i < pMatSet->m_nTextures; i++ )
        pNewTexture[i - 1] = pMatSet->m_pTextures[i];

    //---   remove the old array
    if ( pMatSet->m_nTextures )
    {
        delete []pMatSet->m_pTextures;
    }

    //---   assign the new array
    pMatSet->m_pTextures = pNewTexture;
    pMatSet->m_nTextures--;
}

//==========================================================================

static
void ConsolidateTextures( mesh::object* pObj )
{
    s32     i, j;
    s32     StageID;
    xbool   Done = FALSE;

    mesh::material_set* pMatSet = GetMaterialSet(pObj);

    while ( !Done )
    {
        Done = TRUE;

        //---   attempt to find two matching textures
        for ( i = 0; i < pMatSet->m_nTextures; i++ )
        {
            for ( j = 0; j < pMatSet->m_nTextures; j++ )
            {
                if ( i == j )
                    continue;

                if ( !x_stricmp( pMatSet->m_pTextures[i].m_Filename, pMatSet->m_pTextures[j].m_Filename ) &&
                     (pMatSet->m_pTextures[i].m_Height == pMatSet->m_pTextures[j].m_Height) &&
                     (pMatSet->m_pTextures[i].m_Width == pMatSet->m_pTextures[j].m_Width) )
                {
                    //---   we have found a spot where there are two identical textures, so
                    //      remove one of them
                    
                    //---   first, re-arrange the appropriate indices
                    for ( StageID = 0; StageID < pMatSet->m_nStages; StageID++ )
                    {
                        //---   remove any references to the j-th texture
                        if ( pMatSet->m_pStages[StageID].m_iTexture == j )
                            pMatSet->m_pStages[StageID].m_iTexture = i;

                        //---   since we'll be removing the j-th texture, drop
                        //      down any indices greater than j
                        if ( pMatSet->m_pStages[StageID].m_iTexture > j )
                            pMatSet->m_pStages[StageID].m_iTexture--;
                    }

                    //---   now, remove the j-th texture
                    RemoveTexture( pObj, j );

                    Done = FALSE;
                }

                if ( Done == FALSE )
                    break;
            }

            if ( Done == FALSE )
                break;
        }
    }
}

//==========================================================================

static
void RemoveStage( mesh::object* pObj, s32 StageID )
{
    s32                     i;
    mesh::stage*     pNewStage = NULL;

    mesh::material_set* pMatSet = GetMaterialSet(pObj);

    //---   allocate space for the new sub-material array
    if ( (pMatSet->m_nStages - 1) != 0 )
    {
        pNewStage = new mesh::stage[pMatSet->m_nStages - 1];
        ASSERT( pNewStage );
    }

    //---   copy the sub-materials over from the old array
    for ( i = 0; i < StageID; i++ )
        pNewStage[i] = pMatSet->m_pStages[i];
    for ( i = StageID + 1; i < pMatSet->m_nStages; i++ )
        pNewStage[i - 1] = pMatSet->m_pStages[i];

    //---   remove the old array
    if ( pMatSet->m_nStages )
    {
        delete []pMatSet->m_pStages;
    }

    //---   assign the new array
    pMatSet->m_pStages = pNewStage;
    pMatSet->m_nStages--;
}

//==========================================================================

static
void ConsolidateStages( mesh::object* pObj )
{
    s32     i, j;
    s32     MatID, PassID;
    xbool   Done = FALSE;

    mesh::material_set* pMatSet = GetMaterialSet(pObj);

    while ( !Done )
    {
        Done = TRUE;

        //---   attempt to find two matching sub-materials
        for ( i = 0; i < pMatSet->m_nStages; i++ )
        {
            for ( j = 0; j < pMatSet->m_nStages; j++ )
            {
                if ( i == j )
                    continue;

                if ( /*(pMatSet->m_pStages[i].m_Intensity == pMatSet->m_pStages[j].m_Intensity) && */
                     (pMatSet->m_pStages[i].m_iTexture == pMatSet->m_pStages[j].m_iTexture) &&
                     (pMatSet->m_pStages[i].m_Op == pMatSet->m_pStages[j].m_Op) && 
                     (pMatSet->m_pStages[i].m_WrapU == pMatSet->m_pStages[j].m_WrapU) &&
                     (pMatSet->m_pStages[i].m_WrapV == pMatSet->m_pStages[j].m_WrapV) )
                {
                    //---   we have found a spot where there exists a duplicate sub-material

                    //---   first, re-arrange the appropriate indices
                    for ( MatID = 0; MatID < pMatSet->m_nMaterials; MatID++ )
                    {
                        for ( PassID = 0; PassID < pMatSet->m_pMaterials[MatID].m_nStages; PassID++ )
                        {
                            //---   remove all references to the j-th sub-material
                            if ( pMatSet->m_pMaterials[MatID].m_iStages[PassID] == j )
                                pMatSet->m_pMaterials[MatID].m_iStages[PassID] = i;

                            //---   since we'll be removing the j-th sub-material, drop
                            //      down any indices greater than j
                            if ( pMatSet->m_pMaterials[MatID].m_iStages[PassID] > j )
                                pMatSet->m_pMaterials[MatID].m_iStages[PassID]--;
                        }
                    }

                    //---   now, remove the j-th sub-material
                    RemoveStage( pObj, j );

                    Done = FALSE;
                }

                if ( Done == FALSE )
                    break;
            }

            if ( Done == FALSE )
                break;
        }
    }
}

//==========================================================================

static
void RemoveMaterial( mesh::object* pObj, s32 MatID )
{
    s32                 i;
    mesh::material*     pNewMat = NULL;

    mesh::material_set* pMatSet = GetMaterialSet(pObj);

    //---   allocate space for the new material array
    if ( (pMatSet->m_nMaterials - 1) != 0 )
    {
        pNewMat = new mesh::material[pMatSet->m_nMaterials - 1];
        ASSERT( pNewMat );
    }

    //---   copy the materials over from the old array
    for ( i = 0; i < MatID; i++ )
        pNewMat[i] = pMatSet->m_pMaterials[i];
    for ( i = MatID + 1; i < pMatSet->m_nMaterials; i++ )
        pNewMat[i - 1] = pMatSet->m_pMaterials[i];

    //---   remove the old array
    if ( pMatSet->m_nMaterials )
    {
        delete []pMatSet->m_pMaterials;
    }

    //---   assign the new array
    pMatSet->m_pMaterials = pNewMat;
    pMatSet->m_nMaterials--;
}

//==========================================================================

static
void ConsolidateMaterials( mesh::object* pObj )
{
    s32     i, j;
    s32     ChunkID, PassID, FaceID;
    xbool   Done = FALSE;

    mesh::material_set* pMatSet = GetMaterialSet(pObj);

    while ( !Done )
    {
        Done = TRUE;

        //---   attempt to find two matching materials
        for ( i = 0; i < pMatSet->m_nMaterials; i++ )
        {
            for ( j = 0; j < pMatSet->m_nMaterials; j++ )
            {
                if ( i == j )
                    continue;

                if ( !x_stricmp( pMatSet->m_pMaterials[i].m_Name, pMatSet->m_pMaterials[j].m_Name) &&
                     (pMatSet->m_pMaterials[i].m_nStages == pMatSet->m_pMaterials[j].m_nStages) )
                {
                    //---   the names and NPasses are the same, what about the Stage
                    //      indices and UV indices?
                    for ( PassID = 0; PassID < pMatSet->m_pMaterials[i].m_nStages; PassID++ )
                    {
                        if ( pMatSet->m_pMaterials[i].m_iStages[PassID] != pMatSet->m_pMaterials[j].m_iStages[PassID] )
                            break;
                        if ( pMatSet->m_pStages[pMatSet->m_pMaterials[i].m_iStages[PassID]].m_iUV != 
                             pMatSet->m_pStages[pMatSet->m_pMaterials[j].m_iStages[PassID]].m_iUV )
                            break;
                    }
                    if ( PassID == pMatSet->m_pMaterials[i].m_nStages )
                    {
                        Done = FALSE;

                        //---   we found a duplicate material

                        //---   first, re-arrange the appropriate indices
                        for ( ChunkID = 0; ChunkID < pObj->m_nChunks; ChunkID++ )
                        {
                            for ( FaceID = 0; FaceID < pObj->m_pChunks[ChunkID].m_nFaces; FaceID++ )
                            {
                                //---   remove all references to the j-th material
                                if ( pObj->m_pChunks[ChunkID].m_pFaces[FaceID].m_iMaterial == j )
                                    pObj->m_pChunks[ChunkID].m_pFaces[FaceID].m_iMaterial = i;

                                //---   since we'll be removing the j-th material, drop
                                //      down any indices greater than j
                                if ( pObj->m_pChunks[ChunkID].m_pFaces[FaceID].m_iMaterial > j )
                                    pObj->m_pChunks[ChunkID].m_pFaces[FaceID].m_iMaterial--;
                            }
                        }

                        //---   now, remove the j-th material
                        RemoveMaterial( pObj, j );
                    }
                }

                if ( Done == FALSE )
                    break;
            }

            if ( Done == FALSE )
                break;
        }
    }
}

//==========================================================================

void QGeomMerge::MergeObjects( mesh::object* pDst )
{
    s32 GeomID, BoneID, ChunkID, VertID, FaceID, MatID, StageID, TexID, PassID;

    s32 ChunkOffset = 0;
    s32 MaterialOffset = 0;
    s32 StageOffset = 0;
    s32 TextureOffset = 0;

    pDst->m_nMaterialSets = 1;
    pDst->m_pMaterialSets = new mesh::material_set;
    mesh::material_set* pMatSet = pDst->m_pMaterialSets;
    pMatSet->m_Target = mesh::MESH_TARGET_GENERIC;

    pDst->m_nChunks = 0;
    pMatSet->m_nMaterials = 0;
    pMatSet->m_nStages = 0;
    pMatSet->m_nTextures = 0;

    //---   tally up some numbers
    ASSERT( m_NGeoms );
    pDst->m_nBones = m_pGeoms[0]->m_nBones;
    for ( GeomID = 0; GeomID < m_NGeoms; GeomID++ )
    {
        pDst->m_nChunks += m_pGeoms[GeomID]->m_nChunks;
        pMatSet->m_nMaterials += GetMaterialSet(m_pGeoms[GeomID])->m_nMaterials;
        pMatSet->m_nStages += GetMaterialSet(m_pGeoms[GeomID])->m_nStages;
        pMatSet->m_nTextures += GetMaterialSet(m_pGeoms[GeomID])->m_nTextures;

        //---   the # of bones for all of the meshes to be merged have to
        //      be the same, so let's verify that...
        ASSERT( m_pGeoms[0]->m_nBones == m_pGeoms[GeomID]->m_nBones );
    }

    //---   the bones have to be the same for all meshes for this to work..
    //      let's do a verification of that!
    for ( BoneID = 0; BoneID < pDst->m_nBones; BoneID++ )
    {
        mesh::bone  TheBone;

        TheBone = m_pGeoms[0]->m_pBones[BoneID];
        for ( GeomID = 1; GeomID < m_NGeoms; GeomID++ )
        {
            ASSERT( TheBone.m_Id == m_pGeoms[GeomID]->m_pBones[BoneID].m_Id );
            ASSERT( TheBone.m_nChildren == m_pGeoms[GeomID]->m_pBones[BoneID].m_nChildren );
            ASSERT( !x_stricmp( TheBone.m_Name, m_pGeoms[GeomID]->m_pBones[BoneID].m_Name ) );
        }
    }

    //---   now, allocate the space we need for everything
    if ( pDst->m_nBones )
    {
        pDst->m_pBones = new mesh::bone[pDst->m_nBones];
        ASSERT( pDst->m_pBones );
    }
    if ( pDst->m_nChunks )
    {
        pDst->m_pChunks = new mesh::chunk[pDst->m_nChunks];
        ASSERT( pDst->m_pChunks );
    }
    if ( pMatSet->m_nMaterials )
    {
        pMatSet->m_pMaterials = new mesh::material[pMatSet->m_nMaterials];
        ASSERT( pMatSet->m_pMaterials );
    }
    if ( pMatSet->m_nStages )
    {
        pMatSet->m_pStages = new mesh::stage[pMatSet->m_nStages];
        ASSERT( pMatSet->m_pStages );
    }
    if ( pMatSet->m_nTextures )
    {
        pMatSet->m_pTextures = new mesh::texture[pMatSet->m_nTextures];
        ASSERT( pMatSet->m_pTextures );
    }

    //---   the dest just takes its name from the first geom
    x_strcpy( pDst->m_Name, m_pGeoms[0]->m_Name );

    //---   now merge all of the together
    for ( GeomID = 0; GeomID < m_NGeoms; GeomID++ )
    {
        mesh::object* pSrc;

        pSrc = m_pGeoms[GeomID];
        mesh::material_set* pMatSetSrc = GetMaterialSet(pSrc);

        //---   add the bones
        for ( BoneID = 0; BoneID < pSrc->m_nBones; BoneID++ )
        {
            pDst->m_pBones[BoneID] = pSrc->m_pBones[BoneID];
        }

        //---   add the chunks
        for ( ChunkID = 0; ChunkID < pSrc->m_nChunks; ChunkID++ )
        {
            mesh::chunk* pDstChunk;
            mesh::chunk* pSrcChunk;

            pDstChunk = &pDst->m_pChunks[ChunkID + ChunkOffset];
            pSrcChunk = &pSrc->m_pChunks[ChunkID];

            //---   copy the name
            x_strcpy( pDstChunk->m_Name, pSrcChunk->m_Name );

            //---   copy the verts
            pDstChunk->m_nVerts = pSrcChunk->m_nVerts;
            if ( pDstChunk->m_nVerts )
            {
                pDstChunk->m_pVerts = new mesh::vertex[pDstChunk->m_nVerts];
                ASSERT( pDstChunk->m_pVerts );
                for ( VertID = 0; VertID < pDstChunk->m_nVerts; VertID++ )
                    pDstChunk->m_pVerts[VertID] = pSrcChunk->m_pVerts[VertID];
            }

            //---   copy the faces
            pDstChunk->m_nFaces = pSrcChunk->m_nFaces;
            if ( pDstChunk->m_nFaces )
            {
                pDstChunk->m_pFaces = new mesh::face[pDstChunk->m_nFaces];
                ASSERT( pDstChunk->m_pFaces );
                for ( FaceID = 0; FaceID < pDstChunk->m_nFaces; FaceID++ )
                {
                    for ( VertID = 0; VertID < 4; VertID++ )
                    {
                        pDstChunk->m_pFaces[FaceID].m_Index[VertID] =
                            pSrcChunk->m_pFaces[FaceID].m_Index[VertID];
                    }
                    pDstChunk->m_pFaces[FaceID].m_bQuad = pSrcChunk->m_pFaces[FaceID].m_bQuad;
                    pDstChunk->m_pFaces[FaceID].m_iMaterial =
                        pSrcChunk->m_pFaces[FaceID].m_iMaterial + MaterialOffset;
                }
            }
        }

        //---   Add the materials
        for ( MatID = 0; MatID < pMatSetSrc->m_nMaterials; MatID++ )
        {
            mesh::material* pDstMat;
            mesh::material* pSrcMat;

            pDstMat = &pMatSet->m_pMaterials[MatID + MaterialOffset];
            pSrcMat = &pMatSetSrc->m_pMaterials[MatID];

            for ( PassID = 0; PassID < pSrcMat->m_nStages; PassID++ )
            {
                pDstMat->m_iStages[PassID] = pSrcMat->m_iStages[PassID] + StageOffset;
                pMatSet->m_pStages[pDstMat->m_iStages[PassID]].m_iUV = pMatSetSrc->m_pStages[pSrcMat->m_iStages[PassID]].m_iUV;
            }
            x_strcpy( pDstMat->m_Name, pSrcMat->m_Name );
            pDstMat->m_nStages = pSrcMat->m_nStages;
        }

        //---   Add the sub-materials
        for ( StageID = 0; StageID < pMatSetSrc->m_nStages; StageID++ )
        {
            mesh::stage* pDstStage;
            mesh::stage* pSrcStage;

            pDstStage = &pMatSet->m_pStages[StageID + StageOffset];
            pSrcStage = &pMatSetSrc->m_pStages[StageID];

//            pDstStage->m_Intensity = pSrcStage->m_Intensity;
            pDstStage->m_iTexture  = pSrcStage->m_iTexture + TextureOffset;
            pDstStage->m_Op        = pSrcStage->m_Op;
            pDstStage->m_WrapU     = pSrcStage->m_WrapU;
            pDstStage->m_WrapV     = pSrcStage->m_WrapV;
        }

        //---   Add the textures
        for ( TexID = 0; TexID < pMatSetSrc->m_nTextures; TexID++ )
        {
            pMatSet->m_pTextures[TexID + TextureOffset] = pMatSetSrc->m_pTextures[TexID];
        }

        //---   keep our offsets current
        ChunkOffset += pSrc->m_nChunks;
        MaterialOffset += pMatSetSrc->m_nMaterials;
        StageOffset += pMatSetSrc->m_nStages;
        TextureOffset += pMatSetSrc->m_nTextures;
    }

    //---   consolidate the textures
    ConsolidateTextures( pDst );

    //---   consolidate the sub-materials
    ConsolidateStages( pDst );

    //---   consolidate the materials
    ConsolidateMaterials( pDst );
}

//==========================================================================

void QGeomMerge::CopyGeom( mesh::object* pDst, mesh::object* pSrc )
{
    s32 i, j;

    ASSERT( pDst && pSrc );

    //---   copy the name
    x_strcpy( pDst->m_Name, pSrc->m_Name );

    //---   copy the material sets
    mesh::material_set* pMatSetSrc = GetMaterialSet(pSrc);
    pDst->m_nMaterialSets = pSrc->m_nMaterialSets;
    pDst->m_pMaterialSets = new mesh::material_set[pSrc->m_nMaterialSets];
    for( i = 0; i < pSrc->m_nMaterialSets; i++ )
        pDst->m_pMaterialSets[i] = pSrc->m_pMaterialSets[i];
    mesh::material_set* pMatSet = GetMaterialSet(pDst);

    //---   copy the bones
    pDst->m_nBones = pSrc->m_nBones;
    if ( pDst->m_nBones )
    {
        pDst->m_pBones = new mesh::bone[pDst->m_nBones];
        ASSERT( pDst->m_pBones );

        for ( i = 0; i < pDst->m_nBones; i++ )
            pDst->m_pBones[i] = pSrc->m_pBones[i];
    }

    //---   copy the chunks
    pDst->m_nChunks = pSrc->m_nChunks;
    if ( pDst->m_nChunks )
    {
        pDst->m_pChunks = new mesh::chunk[pDst->m_nChunks];
        ASSERT( pDst->m_pChunks );

        for ( i = 0; i < pDst->m_nChunks; i++ )
        {
            //---   copy the name
            x_strcpy( pDst->m_pChunks[i].m_Name, pSrc->m_pChunks[i].m_Name );

            //---   copy the verts
            pDst->m_pChunks[i].m_nVerts = pSrc->m_pChunks[i].m_nVerts;
            if ( pDst->m_pChunks[i].m_nVerts )
            {
                pDst->m_pChunks[i].m_pVerts = new mesh::vertex[pDst->m_pChunks[i].m_nVerts];
                ASSERT( pDst->m_pChunks[i].m_pVerts );
                for ( j = 0; j < pDst->m_pChunks[i].m_nVerts; j++ )
                    pDst->m_pChunks[i].m_pVerts[j] = pSrc->m_pChunks[i].m_pVerts[j];
            }

            //---   copy the faces
            pDst->m_pChunks[i].m_nFaces = pSrc->m_pChunks[i].m_nFaces;
            if ( pDst->m_pChunks[i].m_nFaces )
            {
                pDst->m_pChunks[i].m_pFaces = new mesh::face[pDst->m_pChunks[i].m_nFaces];
                ASSERT( pDst->m_pChunks[i].m_pFaces );
                for ( j = 0; j < pDst->m_pChunks[i].m_nFaces; j++ )
                    pDst->m_pChunks[i].m_pFaces[j] = pSrc->m_pChunks[i].m_pFaces[j];
            }
        }

    }

    //---   copy the materials
    pMatSet->m_nMaterials = pMatSetSrc->m_nMaterials;
    if ( pMatSet->m_nMaterials )
    {
        pMatSet->m_pMaterials = new mesh::material[pMatSet->m_nMaterials];
        ASSERT( pMatSet->m_pMaterials );

        for ( i = 0; i < pMatSet->m_nMaterials; i++ )
            pMatSet->m_pMaterials[i] = pMatSetSrc->m_pMaterials[i];
    }

    //---   copy the sub-materials
    pMatSet->m_nStages = pMatSetSrc->m_nStages;
    if ( pMatSet->m_nStages )
    {
        pMatSet->m_pStages = new mesh::stage[pMatSet->m_nStages];
        ASSERT( pMatSet->m_pStages );

        for ( i = 0; i < pMatSet->m_nStages; i++ )
            pMatSet->m_pStages[i] = pMatSetSrc->m_pStages[i];
    }

    //---   copy the textures
    pMatSet->m_nTextures = pMatSetSrc->m_nTextures;
    if ( pMatSet->m_nTextures )
    {
        pMatSet->m_pTextures = new mesh::texture[pMatSet->m_nTextures];
        ASSERT( pMatSet->m_pTextures );

        for ( i = 0; i < pMatSet->m_nTextures; i++ )
            pMatSet->m_pTextures[i] = pMatSetSrc->m_pTextures[i];
    }
}

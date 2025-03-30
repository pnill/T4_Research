#include "x_plus.hpp"
#include "x_memory.hpp"

#include "skin_mesh.h"
#include "MeshUtils.h"


////////////////////////////////////////////////////////////////////////////
// Utilities for modifying meshes
////////////////////////////////////////////////////////////////////////////

void MESHUTIL_ChunkCopy( mesh::chunk* pDst, mesh::chunk* pSrc, char* NewName )
{
    s32 i;

    ASSERT( pDst && pSrc );

    //---   set the name
    if ( NewName )
        x_strcpy( pDst->m_Name, NewName );
    else
        x_strcpy( pDst->m_Name, pSrc->m_Name );

    //---   copy the faces
    pDst->m_nFaces = pSrc->m_nFaces;
    if ( pDst->m_nFaces )
    {
        pDst->m_pFace = new mesh::face[pDst->m_nFaces];
        ASSERT( pDst->m_pFace );

        for ( i = 0; i < pDst->m_nFaces; i++ )
        {
            pDst->m_pFace[i] = pSrc->m_pFace[i];
        }
    }

    //---   copy the verts
    pDst->m_nVerts = pSrc->m_nVerts;
    if ( pDst->m_nVerts )
    {
        pDst->m_pVertex = new mesh::vertex[pDst->m_nVerts];
        ASSERT( pDst->m_pVertex );

        for ( i = 0; i < pDst->m_nVerts; i++ )
        {
            pDst->m_pVertex[i] = pSrc->m_pVertex[i];
        }
    }
}

//==========================================================================

void MESHUTIL_BoneCopy( mesh::bone* pDst, mesh::bone* pSrc )
{
    x_strcpy( pDst->m_Name, pSrc->m_Name );
    pDst->m_Id = pSrc->m_Id;
    pDst->m_nChildren = pSrc->m_nChildren;
}

//==========================================================================

void MESHUTIL_TextureCopy( mesh::texture* pDst, mesh::texture* pSrc )
{
    x_strcpy( pDst->m_FileName, pSrc->m_FileName );
    pDst->m_Height = pSrc->m_Height;
    pDst->m_Width = pSrc->m_Width;
}

//==========================================================================

void MESHUTIL_SubMaterialCopy( mesh::sub_material* pDst, mesh::sub_material* pSrc )
{
    pDst->m_Intensity = pSrc->m_Intensity;
    pDst->m_iTexture = pSrc->m_iTexture;
    pDst->m_Operation = pSrc->m_Operation;
    pDst->m_WrapU = pSrc->m_WrapU;
    pDst->m_WrapV = pSrc->m_WrapV;
}

//==========================================================================

void MESHUTIL_MaterialCopy( mesh::material* pDst, mesh::material* pSrc )
{
    s32 i;

    x_strcpy( pDst->m_Name, pSrc->m_Name );
    pDst->m_nPasses = pSrc->m_nPasses;

    for ( i = 0; i < pDst->m_nPasses; i++ )
    {
        pDst->m_iSubMat[i] = pSrc->m_iSubMat[i];
        pDst->m_iUV[i] = pSrc->m_iUV[i];
    }
}

//==========================================================================

void MESHUTIL_BaseInfoCopy( mesh::object* pDst, mesh::object* pSrc )
{
    s32 i;

    //---   copy the name
    x_strcpy( pDst->m_Name, pSrc->m_Name );

    //---   copy the bones
    pDst->m_nBones = pSrc->m_nBones;
    pDst->m_pBone = new mesh::bone[pSrc->m_nBones];
    ASSERT( pDst->m_pBone );
    for ( i = 0; i < pDst->m_nBones; i++ )
        MESHUTIL_BoneCopy( &pDst->m_pBone[i], &pSrc->m_pBone[i] );

    //---   don't copy the chunks
    pDst->m_nChunks = 0;
    pDst->m_pChunk = NULL;

    //---   copy the textures
    pDst->m_nTextures = pSrc->m_nTextures;
    pDst->m_pTexture = new mesh::texture[pSrc->m_nTextures];
    ASSERT( pDst->m_pTexture );
    for ( i = 0; i < pDst->m_nTextures; i++ )
        MESHUTIL_TextureCopy( &pDst->m_pTexture[i], &pSrc->m_pTexture[i] );

    //---   copy the sub-materials
    pDst->m_nSubMaterials = pSrc->m_nSubMaterials;
    pDst->m_pSubMaterial = new mesh::sub_material[pSrc->m_nSubMaterials];
    ASSERT( pDst->m_pSubMaterial );
    for ( i = 0; i < pDst->m_nSubMaterials; i++ )
        MESHUTIL_SubMaterialCopy( &pDst->m_pSubMaterial[i], &pSrc->m_pSubMaterial[i] );

    //---   copy the materials
    pDst->m_nMaterials = pSrc->m_nMaterials;
    pDst->m_pMaterial = new mesh::material[pSrc->m_nMaterials];
    ASSERT( pDst->m_pMaterial );
    for ( i = 0; i < pDst->m_nMaterials; i++ )
        MESHUTIL_MaterialCopy( &pDst->m_pMaterial[i], &pSrc->m_pMaterial[i] );
}

//==========================================================================

s32 MESHUTIL_GetMaterialID( mesh::object* pMesh, char* MatName )
{
    s32 i;
    
    ASSERT( pMesh && MatName );

    //---   find the material
    for ( i = 0; i < pMesh->m_nMaterials; i++ )
    {
        if ( !x_stricmp( pMesh->m_pMaterial[i].m_Name, MatName ) )
        {
            return i;
        }
    }

    return -1;
}

//==========================================================================

s32 MESHUTIL_GetChunkID( mesh::object* pMesh, char* ChunkName )
{
    s32 i;

    ASSERT( pMesh && ChunkName );

    //---   find the chunk
    for ( i = 0; i < pMesh->m_nChunks; i++ )
    {
        if ( !x_stricmp( pMesh->m_pChunk[i].m_Name, ChunkName ) )
        {
            return i;
        }
    }

    return -1;
}

//==========================================================================

#define NEWLINE "\r\n"

static
void RecurseSaveBone( mesh::object* pMesh, char** pTemp, s32& rBone, s32 Parent, s32 Indent)
{
    s32 ChildID;
    s32 IndentID;

    for( ChildID = 0; ChildID < pMesh->m_pBone[Parent].m_nChildren; ChildID++ )
    {
        //---   output child count
        *pTemp += x_sprintf( *pTemp, "\t\t%d, ", pMesh->m_pBone[rBone].m_nChildren );

        //---   indent
        *pTemp += x_sprintf( *pTemp, ",\t" );
        for( IndentID = 0; IndentID < Indent; IndentID++)
            *pTemp += x_sprintf( *pTemp, " " );

        //---   output name
        *pTemp += x_sprintf( *pTemp, "\"%s\"" NEWLINE, pMesh->m_pBone[rBone].m_Name );

        //---   recurse on children
        rBone++;
        RecurseSaveBone( pMesh, pTemp, rBone, (rBone - 1), (Indent + 1));
    }
}

//==========================================================================

xbool MESHUTIL_SaveMTF( mesh::object* pMesh, char* filename, QRipMorphTargetSet* pMorphSet )
{
    char*   tempbuf;
    char*   pTemp;
    s32     i, j, k;
    s32     BoneIndent = 0;
    X_FILE* fh;

    tempbuf = (char*)x_malloc( 4 * 1024 * 1024 );   // #### try to avoid hard-coding this!!!!
    ASSERT( tempbuf );
    pTemp = tempbuf;

    //---   print out the header
    pTemp += x_sprintf( pTemp, "*MESH = \"%s\"" NEWLINE, pMesh->m_Name );
    pTemp += x_sprintf( pTemp, "{" NEWLINE );

    if ( pMesh->m_nBones )
    {
        //---   print out the skeleton
        pTemp += x_sprintf( pTemp, "\t*HIERARCHY[ %d ] =" NEWLINE, pMesh->m_nBones );
        pTemp += x_sprintf( pTemp, "\t{" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t\t; nChildren\tName" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t\t%d, ,\t", pMesh->m_pBone[0].m_nChildren );
        pTemp += x_sprintf( pTemp, "\"%s\"" NEWLINE, pMesh->m_pBone[0].m_Name );
        i = 1;
        RecurseSaveBone( pMesh, &pTemp, i, 0, 1 );
        pTemp += x_sprintf( pTemp, "\t}" NEWLINE NEWLINE );
    }

    //---   save out the chunks
    pTemp += x_sprintf( pTemp, "\t*NUM_CHUNKS = %d" NEWLINE, pMesh->m_nChunks );
    for ( i = 0; i < pMesh->m_nChunks; i++ )
    {
        //---   chunk name
        pTemp += x_sprintf( pTemp, "\t*CHUNK = \"%s\"" NEWLINE, pMesh->m_pChunk[i].m_Name );
        pTemp += x_sprintf( pTemp, "\t{" NEWLINE );
        
        //---   vertices
        pTemp += x_sprintf( pTemp, "\t\t*VERTEX[ %d ] =" NEWLINE, pMesh->m_pChunk[i].m_nVerts );
        pTemp += x_sprintf( pTemp, "\t\t{" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t\t\t; x\ty\tz" NEWLINE );
        for ( j = 0; j < pMesh->m_pChunk[i].m_nVerts; j++ )
        {
            pTemp += x_sprintf( pTemp, "\t\t\t%1.6f, %1.6f, %1.6f," NEWLINE,
                                pMesh->m_pChunk[i].m_pVertex[j].m_vPos.X,
                                pMesh->m_pChunk[i].m_pVertex[j].m_vPos.Y,
                                pMesh->m_pChunk[i].m_pVertex[j].m_vPos.Z );
        }
        pTemp += x_sprintf( pTemp, "\t\t}" NEWLINE NEWLINE );

        //---   normals
        pTemp += x_sprintf( pTemp, "\t\t*NORMAL[ %d ] =" NEWLINE, pMesh->m_pChunk[i].m_nVerts );
        pTemp += x_sprintf( pTemp, "\t\t{" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t\t\t; x\ty\tz" NEWLINE );
        for ( j = 0; j < pMesh->m_pChunk[i].m_nVerts; j++ )
        {
            pTemp += x_sprintf( pTemp, "\t\t\t%1.6f, %1.6f, %1.6f," NEWLINE,
                                pMesh->m_pChunk[i].m_pVertex[j].m_vNormal.X,
                                pMesh->m_pChunk[i].m_pVertex[j].m_vNormal.Y,
                                pMesh->m_pChunk[i].m_pVertex[j].m_vNormal.Z );
        }
        pTemp += x_sprintf( pTemp, "\t\t}" NEWLINE NEWLINE );

        //---   colors
        pTemp += x_sprintf( pTemp, "\t\t*COLOR[ %d ] =" NEWLINE, pMesh->m_pChunk[i].m_nVerts );
        pTemp += x_sprintf( pTemp, "\t\t{" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t\t\t; r\tg\tb" NEWLINE );
        for ( j = 0; j < pMesh->m_pChunk[i].m_nVerts; j++ )
        {
            pTemp += x_sprintf( pTemp, "\t\t\t%1.6f, %1.6f, %1.6f, %1.6f," NEWLINE,
                                pMesh->m_pChunk[i].m_pVertex[j].m_Color.X,
                                pMesh->m_pChunk[i].m_pVertex[j].m_Color.Y,
                                pMesh->m_pChunk[i].m_pVertex[j].m_Color.Z,
                                pMesh->m_pChunk[i].m_pVertex[j].m_Color.W );
        }
        pTemp += x_sprintf( pTemp, "\t\t}" NEWLINE NEWLINE );

        //---   UV map
        pTemp += x_sprintf( pTemp, "\t\t*UVMAP[ %d ] =" NEWLINE, pMesh->m_pChunk[i].m_nVerts );
        pTemp += x_sprintf( pTemp, "\t\t{" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t\t\t; VIndex\tPassNum\tU\tV" NEWLINE );
        for ( j = 0; j < pMesh->m_pChunk[i].m_nVerts; j++ )
        {
            for ( k = 0; k < pMesh->m_pChunk[i].m_pVertex[j].m_nPasses; k++ )
            {
                pMesh->m_pChunk[i].m_pVertex[j].m_UV[k];
                pTemp += x_sprintf( pTemp, "\t\t\t%d,\t%d,\t%1.6f, %1.6f," NEWLINE,
                    j,
                    k,
                    pMesh->m_pChunk[i].m_pVertex[j].m_UV[k].X,
                    pMesh->m_pChunk[i].m_pVertex[j].m_UV[k].Y );
            }
        }
        pTemp += x_sprintf( pTemp, "\t\t}" NEWLINE NEWLINE );

        //---   Weights
        pTemp += x_sprintf( pTemp, "\t\t*WEIGHT[ %d ] =" NEWLINE, pMesh->m_pChunk[i].m_nVerts );
        pTemp += x_sprintf( pTemp, "\t\t{" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t\t\t; NMatrices\tMIndex\t%%\t..." NEWLINE );
        for ( j = 0; j < pMesh->m_pChunk[i].m_nVerts; j++ )
        {
            pTemp += x_sprintf( pTemp, "\t\t\t%d", pMesh->m_pChunk[i].m_pVertex[j].m_nWeights );
            for ( k = 0; k < pMesh->m_pChunk[i].m_pVertex[j].m_nWeights; k++ )
            {
                pTemp += x_sprintf( pTemp, ",\t%d,\t%1.6f",
                    pMesh->m_pChunk[i].m_pVertex[j].m_Weight[k].m_iMatrix,
                    pMesh->m_pChunk[i].m_pVertex[j].m_Weight[k].m_Weight );
            }
            pTemp += x_sprintf( pTemp, NEWLINE );
        }
        pTemp += x_sprintf( pTemp, "\t\t}" NEWLINE NEWLINE );

        //---   Tris
        pTemp += x_sprintf( pTemp, "\t\t*TRI[ %d ] =" NEWLINE, pMesh->m_pChunk[i].m_nFaces );
        pTemp += x_sprintf( pTemp, "\t\t{" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t\t\t; VIndex\tVIndex\tVIndex\tMaterialIndex" NEWLINE );
        for ( j = 0; j < pMesh->m_pChunk[i].m_nFaces; j++ )
        {
            pTemp += x_sprintf( pTemp, "\t\t\t%d, ,\t%d, ,\t%d, ,\t%d," NEWLINE,
                pMesh->m_pChunk[i].m_pFace[j].m_Index[0],
                pMesh->m_pChunk[i].m_pFace[j].m_Index[1],
                pMesh->m_pChunk[i].m_pFace[j].m_Index[2],
                pMesh->m_pChunk[i].m_pFace[j].m_iMaterial );
        }
        pTemp += x_sprintf( pTemp, "\t\t}" NEWLINE NEWLINE );

        pTemp += x_sprintf( pTemp, "\t}" NEWLINE NEWLINE );
    }

    //---   save out the textures
    pTemp += x_sprintf( pTemp, "\t*TEXTURE[ %d ] =" NEWLINE, pMesh->m_nTextures );
    pTemp += x_sprintf( pTemp, "\t{" NEWLINE );
    pTemp += x_sprintf( pTemp, "\t\t; W\tH\tTextureName" NEWLINE );
    for ( i = 0; i < pMesh->m_nTextures; i++ )
    {
        pTemp += x_sprintf( pTemp, "\t\t%d, ,\t%d, ,\t\"%s\"" NEWLINE,
            pMesh->m_pTexture[i].m_Width,
            pMesh->m_pTexture[i].m_Height,
            pMesh->m_pTexture[i].m_FileName );
    }
    pTemp += x_sprintf( pTemp, "\t}" NEWLINE NEWLINE );

    //---   save out the sub-materials
    pTemp += x_sprintf( pTemp, "\t*SUBMATERIAL[ %d ] =" NEWLINE, pMesh->m_nSubMaterials );
    pTemp += x_sprintf( pTemp, "\t{" NEWLINE );
    pTemp += x_sprintf( pTemp, "\t\t; UWrap VWrap TextureID Intensity Operation" NEWLINE );
    for ( i = 0; i < pMesh->m_nSubMaterials; i++ )
    {
        pTemp += x_sprintf( pTemp, "\t\t" );
        switch ( pMesh->m_pSubMaterial[i].m_WrapU )
        {
        case mesh::WMODE_NONE:    pTemp += x_sprintf( pTemp, "\"NONE\", " );    break;
        case mesh::WMODE_CLAMP:   pTemp += x_sprintf( pTemp, "\"CLAMP\", " );   break;
        case mesh::WMODE_TILE:    pTemp += x_sprintf( pTemp, "\"TILE\", " );    break;
        case mesh::WMODE_MIRROR:  pTemp += x_sprintf( pTemp, "\"MIRROR\", " );  break;
        }
        switch ( pMesh->m_pSubMaterial[i].m_WrapV )
        {
        case mesh::WMODE_NONE:    pTemp += x_sprintf( pTemp, "\"NONE\",\t" );    break;
        case mesh::WMODE_CLAMP:   pTemp += x_sprintf( pTemp, "\"CLAMP\",\t" );   break;
        case mesh::WMODE_TILE:    pTemp += x_sprintf( pTemp, "\"TILE\",\t" );    break;
        case mesh::WMODE_MIRROR:  pTemp += x_sprintf( pTemp, "\"MIRROR\",\t" );  break;
        }
        pTemp += x_sprintf( pTemp, "%d, ,\t%1.6f, ,\t",
            pMesh->m_pSubMaterial[i].m_iTexture,
            pMesh->m_pSubMaterial[i].m_Intensity );
        switch ( pMesh->m_pSubMaterial[i].m_Operation )
        {
        case mesh::OP_OPAQUE:         pTemp += x_sprintf( pTemp, "\"OPAQUE\"" NEWLINE );          break;
        case mesh::OP_PUNCH_THROUGH:  pTemp += x_sprintf( pTemp, "\"PUNCH_THROUGH\"" NEWLINE );   break;
        case mesh::OP_ALPHA:          pTemp += x_sprintf( pTemp, "\"ALPHA\"" NEWLINE );           break;
        case mesh::OP_EMBOSS:         pTemp += x_sprintf( pTemp, "\"EMBOSS\"" NEWLINE );          break;
        case mesh::OP_DOT3:           pTemp += x_sprintf( pTemp, "\"DOT3\"" NEWLINE );            break;
        case mesh::OP_SPECULAR:       pTemp += x_sprintf( pTemp, "\"SPECULAR\"" NEWLINE );        break;
        case mesh::OP_DETAIL:         pTemp += x_sprintf( pTemp, "\"DETAIL\"" NEWLINE );          break;
        case mesh::OP_LIGHTMAP:       pTemp += x_sprintf( pTemp, "\"LIGHTMAP\"" NEWLINE );        break;
        case mesh::OP_MULT:           pTemp += x_sprintf( pTemp, "\"MULT\"" NEWLINE );            break;
        case mesh::OP_ADD:            pTemp += x_sprintf( pTemp, "\"ADD\"" NEWLINE );             break;
        case mesh::OP_SUB:            pTemp += x_sprintf( pTemp, "\"SUB\"" NEWLINE );             break;
        }
    }
    pTemp += x_sprintf( pTemp, "\t}" NEWLINE NEWLINE );

    //---   save out the materials
    pTemp += x_sprintf( pTemp, "\t*MATERIAL[ %d ] =" NEWLINE, pMesh->m_nMaterials );
    pTemp += x_sprintf( pTemp, "\t{" NEWLINE );
    pTemp += x_sprintf( pTemp, "\t\t; MaterialName   NPasses SubmaterialIDs  UVPass" NEWLINE );
    for ( i = 0; i < pMesh->m_nMaterials; i++ )
    {
        pTemp += x_sprintf( pTemp, "\t\t\"%s\",\t%d, ",
            pMesh->m_pMaterial[i].m_Name,
            pMesh->m_pMaterial[i].m_nPasses );
        for ( j = 0; j < pMesh->m_pMaterial[i].m_nPasses; j++ )
        {
            pTemp += x_sprintf( pTemp, ",\t%d, ,\t%d, ",
                pMesh->m_pMaterial[i].m_iSubMat[j],
                pMesh->m_pMaterial[i].m_iUV[j] );
        }
        pTemp += x_sprintf( pTemp, NEWLINE );
    }
    pTemp += x_sprintf( pTemp, "\t}" NEWLINE NEWLINE );

    pTemp += x_sprintf( pTemp, "}" NEWLINE );

    //---   if we have morph targets, save those out
    if ( pMorphSet && pMorphSet->m_NTargets )
    {
        pTemp += x_sprintf( pTemp, NEWLINE "*MORPH_TARGET_SET[ %d ] = ", pMorphSet->m_NTargets );
        pTemp += x_sprintf( pTemp, "\"blahblah - dunno where this comes from !?\"" );
        pTemp += x_sprintf( pTemp, NEWLINE "{" NEWLINE );
        pTemp += x_sprintf( pTemp, "\t*REFERENCE = \"dunno\"" NEWLINE );

        for ( i = 0; i < pMorphSet->m_NTargets; i++ )
        {
            pTemp += x_sprintf( pTemp, "\t*TARGET[ %d ] = ", pMorphSet->m_pTargets[i].m_NDeltas );
            pTemp += x_sprintf( pTemp, "\"%s\"" NEWLINE, pMorphSet->m_pTargets[i].m_Name );
            pTemp += x_sprintf( pTemp, "\t{" NEWLINE );
            pTemp += x_sprintf( pTemp, "\t\t*REFERENCE = " );
            pTemp += x_sprintf( pTemp, "\"%s\"" NEWLINE, pMorphSet->m_pTargets[i].m_ChunkName );
            pTemp += x_sprintf( pTemp, NEWLINE "\t\t;\tiVert,\tvDelta" NEWLINE );

            for ( j = 0; j < pMorphSet->m_pTargets[i].m_NDeltas; j++ )
            {
                pTemp += x_sprintf( pTemp, "\t\t%d, \t%1.6f, %1.6f, %1.6f, " NEWLINE,
                    pMorphSet->m_pTargets[i].m_pDeltas[j].m_VertID,
                    pMorphSet->m_pTargets[i].m_pDeltas[j].m_Delta.X,
                    pMorphSet->m_pTargets[i].m_pDeltas[j].m_Delta.Y,
                    pMorphSet->m_pTargets[i].m_pDeltas[j].m_Delta.Z );
            }

            pTemp += x_sprintf( pTemp, "\t}" NEWLINE NEWLINE );
        }

        pTemp += x_sprintf( pTemp, "}" NEWLINE );
    }

    //---   write out the final file
    fh = x_fopen( filename, "wb" );
    if ( !fh )
    {
        x_printf( "ERROR: Unable to open temp file for writing.\n" );
        return FALSE;
    }
    x_fwrite( tempbuf, sizeof(char), pTemp - tempbuf, fh );
    x_fclose( fh );

    //---   clean up
    x_free( tempbuf );

    return TRUE;
}

//==========================================================================

void MESHUTIL_MergeChunks(  mesh::chunk* pDst, char* NewName,
                            mesh::chunk* Chunk1, mesh::chunk* Chunk2,
                            mesh::chunk* Chunk3, mesh::chunk* Chunk4,
                            mesh::chunk* Chunk5, mesh::chunk* Chunk6 )
{
    mesh::chunk*    SrcList[10];
    s32             NSources = 0;

    if ( Chunk1 )
        SrcList[NSources++] = Chunk1;
    if ( Chunk2 )
        SrcList[NSources++] = Chunk2;
    if ( Chunk3 )
        SrcList[NSources++] = Chunk3;
    if ( Chunk4 )
        SrcList[NSources++] = Chunk4;
    if ( Chunk5 )
        SrcList[NSources++] = Chunk5;
    if ( Chunk6 )
        SrcList[NSources++] = Chunk6;

    MESHUTIL_MergeChunks( pDst, NewName, NSources, SrcList );

}


//==========================================================================

void MESHUTIL_MergeChunks( mesh::chunk* pDst, char* NewName, s32 NChunksToMerge, mesh::chunk** pChunks )
{
    s32             i, j;
    s32             FaceID, VertID;
    s32             NewVertBase;

    //---   copy the name
    x_strcpy( pDst->m_Name, NewName );

    //---   how many faces and verts should we allocate?
    pDst->m_nFaces = 0;
    pDst->m_nVerts = 0;
    for ( i = 0; i < NChunksToMerge; i++ )
    {
        pDst->m_nFaces += pChunks[i]->m_nFaces;
        pDst->m_nVerts += pChunks[i]->m_nVerts;
    }

    //---   allocate the faces and verts
    pDst->m_pFace = new mesh::face[pDst->m_nFaces];
    pDst->m_pVertex = new mesh::vertex[pDst->m_nVerts];

    //---   copy the face and vertex information over
    FaceID = 0;
    VertID = 0;
    NewVertBase = 0;
    for ( i = 0; i < NChunksToMerge; i++ )
    {
        for ( j = 0; j < pChunks[i]->m_nFaces; j++ )
        {
            pDst->m_pFace[FaceID] = pChunks[i]->m_pFace[j];
            pDst->m_pFace[FaceID].m_Index[0] += NewVertBase;
            pDst->m_pFace[FaceID].m_Index[1] += NewVertBase;
            pDst->m_pFace[FaceID].m_Index[2] += NewVertBase;
            pDst->m_pFace[FaceID].m_Index[3] += NewVertBase;
            FaceID++;
        }

        for ( j = 0; j < pChunks[i]->m_nVerts; j++ )
            pDst->m_pVertex[VertID++] = pChunks[i]->m_pVertex[j];

        NewVertBase = VertID;
    }
}

//==========================================================================

void MESHUTIL_RotateY180( mesh::object* pMesh )
{
    s32             ChunkID;
    s32             VertID;
    mesh::chunk*    pChunk;

    for ( ChunkID = 0; ChunkID < pMesh->m_nChunks; ChunkID++ )
    {
        pChunk = &pMesh->m_pChunk[ChunkID];

        for ( VertID = 0; VertID < pChunk->m_nVerts; VertID++ )
        {
            pChunk->m_pVertex[VertID].m_vPos.X = -pChunk->m_pVertex[VertID].m_vPos.X;
            pChunk->m_pVertex[VertID].m_vPos.Z = -pChunk->m_pVertex[VertID].m_vPos.Z;
        }
    }
}

//==========================================================================

static
s32 CountASFBoneChildren( QSkel& rSkel, s32 GeomBoneID )
{
    s32 i;
    s32 count = 0;

    for ( i = 0; i < rSkel.m_NBones; i++ )
    {
        if ( (i != GeomBoneID) &&
             (rSkel.m_pBone[i].ParentID == GeomBoneID) )
        {
            count++;
        }
    }

    return count;
}

//==========================================================================

static
void RecurseBuildParentList( mesh::object* pGeom, s32& rBone, s32* ParentList, s32 Parent )
{
    s32 i;
    s32 NewParent;

    if ( rBone >= pGeom->m_nBones )
        return;

    ParentList[rBone] = Parent;

    NewParent = rBone;
    for ( i = 0; i < pGeom->m_pBone[NewParent].m_nChildren; i++ )
    {
        rBone++;
        RecurseBuildParentList( pGeom, rBone, ParentList, NewParent );
    }
}

//==========================================================================

static
s32 FindParentBone( mesh::object* pGeom, s32 MtfBoneID )
{
    s32 CurrBone = 0;
    s32 Parents[100];

    RecurseBuildParentList( pGeom, CurrBone, Parents, 0 );

    return Parents[MtfBoneID];
}

//==========================================================================

static
s32 FindASFBoneID( QSkel& rSkel, mesh::object* pGeom, s32 MtfBoneID )
{
    s32     j;
    s32     ASFBoneID;

    //---   Given a bone name find the .ASF bone equivalent
    ASFBoneID = -1;

    while( ASFBoneID == -1 )
    {
        //---   Search for bone in asf bones
        for( j = 0; j < rSkel.m_NBones; j++ )
        {
            if( x_stricmp( pGeom->m_pBone[MtfBoneID].m_Name, rSkel.m_pBone[j].BoneName ) == 0 )
            {
                ASFBoneID = j;
                break;
            }
        }

        //---   if no bone found try the parent bone
        if( j == rSkel.m_NBones )
        {
            //---   find the parent...the mtf bone layout isn't very nice, but we can
            //      still find the parent if we try...
            MtfBoneID = FindParentBone( pGeom, MtfBoneID );
        }
    }

    return ASFBoneID;
}

//==========================================================================

void MESHUTIL_RemapVertWeights( QSkel& rSkel, mesh::object* pGeom )
{
    s32 ChunkID;
    s32 VertID;
    s32 WeightID;
    
    s32 GeomBoneID;
    s32 ASFBoneID;

    mesh::vertex*   pVert;
    xbool           done;
    s32             i, j, k;

    ASSERT( pGeom );

    //---   re-map the vert weights
    for ( ChunkID = 0; ChunkID < pGeom->m_nChunks; ChunkID++ )
    {
        for ( VertID = 0; VertID < pGeom->m_pChunk[ChunkID].m_nVerts; VertID++ )
        {
            if ( pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_nWeights == 0 )
            {
                //---   no weights assigned, assume its 100% to the root
                pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_nWeights = 1;
                pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_Weight[0].m_iMatrix = 0;
                pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_Weight[1].m_Weight = 1.0f;
            }
            else
            {
                //---   re-map all of the matrix indices
                for ( WeightID = 0; WeightID < pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_nWeights; WeightID++ )
                {
                    GeomBoneID = pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_Weight[WeightID].m_iMatrix;
                    ASFBoneID = FindASFBoneID( rSkel, pGeom, GeomBoneID );

                    if ( ASFBoneID == -1 )
                        ASFBoneID = 0;

                    pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_Weight[WeightID].m_iMatrix = ASFBoneID;
                }
            }
        }
    }

    //---   because we've removed dummy bones, there is a chance that verts
    //      are weighted to the same bone more than once, remove that
    
    for ( ChunkID = 0; ChunkID < pGeom->m_nChunks; ChunkID++ )
    {
        for ( VertID = 0; VertID < pGeom->m_pChunk[ChunkID].m_nVerts; VertID++ )
        {
            pVert = &pGeom->m_pChunk[ChunkID].m_pVertex[VertID];

            done = FALSE;
            while ( !done )
            {
                done = TRUE;
                for ( i = 0; i < pVert->m_nWeights; i++ )
                {
                    for ( j = i + 1; j < pVert->m_nWeights; j++ )
                    {
                        if ( pVert->m_Weight[i].m_iMatrix == pVert->m_Weight[j].m_iMatrix )
                        {
                            done = FALSE;

                            //---   the i-th weight and the j-th weight should be added together
                            pVert->m_Weight[i].m_Weight += pVert->m_Weight[j].m_Weight;

                            //---   shift the list of weights down
                            for ( k = j; k < pVert->m_nWeights-1; k++ )
                                pVert->m_Weight[k] = pVert->m_Weight[k+1];

                            //---   we have removed an un-necessary weight
                            pVert->m_nWeights--;

                            break;
                        }
                    }

                    if ( !done )
                        break;
                }
            }
        }
    }

    //---   the geom's skeleton now matches the asf skeleton, so make a new version of the skeleton for the geom
    mesh::bone* pBone;

    pBone = new mesh::bone[rSkel.m_NBones];
    for ( GeomBoneID = 0; GeomBoneID < rSkel.m_NBones; GeomBoneID++ )
    {
        pBone[GeomBoneID].m_Id = GeomBoneID;
        x_strcpy( pBone[GeomBoneID].m_Name, rSkel.GetBoneName(GeomBoneID) );

        pBone[GeomBoneID].m_nChildren = CountASFBoneChildren( rSkel, GeomBoneID );
    }

    pGeom->m_nBones = rSkel.m_NBones;
    delete []pGeom->m_pBone;
    pGeom->m_pBone = pBone;
}

//==========================================================================

void MESHUTIL_RemovePrefixes( mesh::object* pMesh, char* Prefix )
{
    s32     i;
    char    Temp1[256];
    char    Temp2[256];

    x_strcpy( Temp1, Prefix );
    x_strtoupper( Temp1 );

    for ( i = 0; i < pMesh->m_nChunks; i++ )
    {
        x_strcpy( Temp2, pMesh->m_pChunk[i].m_Name );
        x_strtoupper( Temp2 );
        if ( !x_strncmp( Temp1, Temp2, x_strlen( Temp1 ) ) )
        {
            x_strcpy( Temp2, pMesh->m_pChunk[i].m_Name );
            x_strcpy( pMesh->m_pChunk[i].m_Name, &Temp2[x_strlen(Temp1)] );
        }
    }
}

//==========================================================================

void MESHUTIL_AddChunk( mesh::object* pDst, mesh::chunk* pSrc )
{
    s32             i;
    mesh::chunk*    pNewChunkList;

    ASSERT( pDst && pSrc );

    //---   create some storage for the new chunk list
    pNewChunkList = new mesh::chunk[pDst->m_nChunks+1];
    ASSERT( pNewChunkList );

    //---   copy the original chunks plus the new chunk into a new list
    for ( i = 0; i < pDst->m_nChunks; i++ )
    {
        MESHUTIL_ChunkCopy( &pNewChunkList[i], &pDst->m_pChunk[i] );
    }
    MESHUTIL_ChunkCopy( &pNewChunkList[pDst->m_nChunks], pSrc );
    
    //---   point the destination objects chunk list to our new one
    pDst->m_nChunks++;
    if ( pDst->m_pChunk )
        delete []pDst->m_pChunk;
    pDst->m_pChunk = pNewChunkList;
}

//==========================================================================

void MESHUTIL_AddMaterial( mesh::object* pDst, mesh::material* pMat )
{
    s32             i;
    mesh::material* pNewMatList;

    ASSERT( pDst && pMat );

    //---   create some storage for the new material list
    pNewMatList = new mesh::material[pDst->m_nMaterials+1];
    ASSERT( pNewMatList );

    //---   copy the original materials plus the new material into a new list
    for ( i = 0; i < pDst->m_nMaterials; i++ )
    {
        MESHUTIL_MaterialCopy( &pNewMatList[i], &pDst->m_pMaterial[i] );
    }
    MESHUTIL_MaterialCopy( &pNewMatList[i], pMat );

    //---   point the destination objects material list to our new one
    pDst->m_nMaterials++;
    if ( pDst->m_pMaterial )
        delete []pDst->m_pMaterial;
    pDst->m_pMaterial = pNewMatList;
}

//==========================================================================

void MESHUTIL_AddSubMaterial( mesh::object* pDst, mesh::sub_material* pSubMat )
{
    s32                 i;
    mesh::sub_material* pNewSubMatList;

    ASSERT( pDst && pSubMat );

    //---   create some storage for the new sub-material list
    pNewSubMatList = new mesh::sub_material[pDst->m_nSubMaterials+1];
    ASSERT( pNewSubMatList );

    //---   copy the original sub-materials plus the new sub-material into a new list
    for ( i = 0; i < pDst->m_nSubMaterials; i++ )
    {
        MESHUTIL_SubMaterialCopy( &pNewSubMatList[i], &pDst->m_pSubMaterial[i] );
    }
    MESHUTIL_SubMaterialCopy( &pNewSubMatList[i], pSubMat );

    //---   point the destination object's sub-material list to our new one
    pDst->m_nSubMaterials++;
    if ( pDst->m_pSubMaterial )
        delete []pDst->m_pSubMaterial;
    pDst->m_pSubMaterial = pNewSubMatList;
}

//==========================================================================

void MESHUTIL_AddTexture( mesh::object* pDst, mesh::texture* pTexture )
{
    s32             i;
    mesh::texture*  pNewTexList;

    ASSERT( pDst && pTexture );

    //---   create some storage for the new texture list
    pNewTexList = new mesh::texture[pDst->m_nTextures+1];
    ASSERT( pNewTexList );

    //---   copy the original textures plust the new texture into a new list
    for ( i = 0; i < pDst->m_nTextures; i++ )
    {
        MESHUTIL_TextureCopy( &pNewTexList[i], &pDst->m_pTexture[i] );
    }
    MESHUTIL_TextureCopy( &pNewTexList[i], pTexture );

    //---   point the destination object's texture list to our new one
    pDst->m_nTextures++;
    if ( pDst->m_pTexture )
        delete []pDst->m_pTexture;
    pDst->m_pTexture = pNewTexList;
}

//==========================================================================

s32 MESHUTIL_GetNFacesUsingMat( mesh::chunk* pChunk, s32 MatID )
{
    s32 i;
    s32 count = 0;

    for ( i = 0; i < pChunk->m_nFaces; i++ )
    {
        if ( pChunk->m_pFace[i].m_iMaterial == MatID )
            count++;
    }
    return count;
}

//==========================================================================

static
s32 GetNValidFaces( mesh::chunk* pChunk )
{
    s32 i;
    s32 count = 0;
    for ( i = 0; i < pChunk->m_nFaces; i++ )
    {
        if ( pChunk->m_pFace[i].m_Index[0] >= 0 )
            count++;
    }
    return count;
}

//==========================================================================

void MESHUTIL_PullNamedMaterial( mesh::object* pMesh,
                                 char* MeshName,
                                 char* MatName,
                                 char* NewMeshName )
{
    s32             i;
    s32             ChunkID;
    s32             MatID;
    s32             NFaces;
    s32             NFacesAdded;
    s32             NTempVertIDs;
    s32*            TempVertIDs;
    mesh::chunk*    pNewChunk;
    mesh::face*     pNewFaces;

    ASSERT( pMesh && MeshName && MatName && NewMeshName );

    //---   find the texture index we are mucking around with
    MatID = MESHUTIL_GetMaterialID( pMesh, MatName );
    if ( MatID < 0 )
        return;
    
    //---   find the mesh ID we are mucking around with
    ChunkID = MESHUTIL_GetChunkID( pMesh, MeshName );
    if ( ChunkID < 0 )
        return;

    //---   count the number of faces that use this texture
    NFaces = MESHUTIL_GetNFacesUsingMat( &pMesh->m_pChunk[ChunkID], MatID );

    //---   try to bail out early if we can
    if ( NFaces == 0 )
        return;

    if ( NFaces == pMesh->m_pChunk[ChunkID].m_nFaces )
    {
        x_strcpy( pMesh->m_pChunk[ChunkID].m_Name, NewMeshName );
        return;
    }

    //---   create a new chunk to hold the pulled-out tris
    pNewChunk = new mesh::chunk;
    ASSERT( pNewChunk );
    x_strcpy( pNewChunk->m_Name, NewMeshName );

    //---   allocate space for the faces
    pNewChunk->m_nFaces = NFaces;
    pNewChunk->m_pFace = new mesh::face[NFaces];
    ASSERT( pNewChunk->m_pFace );

    //---   allocate some temporary storage for vert indices
    NTempVertIDs = 0;
    TempVertIDs = (s32*)x_malloc( 10 * 1024 * sizeof(s32) );
    ASSERT( TempVertIDs );

    //---   find faces that use this texture, and copy the vert indices,
    //      then mark the face as invalid by setting the indices to -1
    //      Invalid faces will later be removed
    NFacesAdded = 0;
    for ( i = 0; i < pMesh->m_pChunk[ChunkID].m_nFaces; i++ )
    {
        if ( pMesh->m_pChunk[ChunkID].m_pFace[i].m_iMaterial == MatID )
        {
            s32 N, V;
            s32 VertID;
            s32 NIndices;

            //---   add this face to our new chunk
            pNewChunk->m_pFace[NFacesAdded].m_bQuad =
                pMesh->m_pChunk[ChunkID].m_pFace[i].m_bQuad;
            pNewChunk->m_pFace[NFacesAdded].m_iMaterial =
                pMesh->m_pChunk[ChunkID].m_pFace[i].m_iMaterial;

            //---   how many indices should we copy over?
            if ( pMesh->m_pChunk[ChunkID].m_pFace[i].m_bQuad )
                NIndices = 4;
            else
                NIndices = 3;

            //---   copy each vert index, adding vert IDs into our list as appropriate
            for ( N = 0; N < NIndices; N++ )
            {
                VertID = pMesh->m_pChunk[ChunkID].m_pFace[i].m_Index[N];
                ASSERT( (VertID >= 0) && (VertID < pMesh->m_pChunk[ChunkID].m_nVerts) );

                //---   is the vert index already in our list?
                for ( V = 0; V < NTempVertIDs; V++ )
                {
                    if ( TempVertIDs[V] == VertID )
                        break;
                }
                if ( V == NTempVertIDs )
                {
                    //---   add the vert to our temp list
                    TempVertIDs[V] = VertID;
                    NTempVertIDs++;
                }

                //---   'V' now contains where the vert will appear in the new vertex list.
                //      reconcile that with the triangle
                pNewChunk->m_pFace[NFacesAdded].m_Index[N] = V;

                //---   mark the source tri as invalid, so it can be removed later
                pMesh->m_pChunk[ChunkID].m_pFace[i].m_Index[N] = -1;
            }
            //---   we have just copied a face over...
            NFacesAdded++;
        }
    }
    ASSERT( NFacesAdded == NFaces );

    //---   at this point all of the faces using this material have been copied over,
    //      copy over the verts this new chunk will need
    pNewChunk->m_nVerts = NTempVertIDs;
    pNewChunk->m_pVertex = new mesh::vertex[NTempVertIDs];
    ASSERT( pNewChunk->m_pVertex );
    for ( i = 0; i < NTempVertIDs; i++ )
    {
        pNewChunk->m_pVertex[i] = pMesh->m_pChunk[ChunkID].m_pVertex[TempVertIDs[i]];
    }

    //---   we no longer need the temporary storage for vert indices
    x_free( TempVertIDs );

    //---   remove the invalid faces from the src chunk
    NFaces = GetNValidFaces( &pMesh->m_pChunk[ChunkID] );
    ASSERT( NFaces );

    pNewFaces = new mesh::face[NFaces];
    ASSERT( pNewFaces );
    NFacesAdded = 0;
    for ( i = 0; i < pMesh->m_pChunk[ChunkID].m_nFaces; i++ )
    {
        if ( pMesh->m_pChunk[ChunkID].m_pFace[i].m_Index[0] >= 0 )
        {
            pNewFaces[NFacesAdded++] = pMesh->m_pChunk[ChunkID].m_pFace[i];
        }
    }
    ASSERT( NFacesAdded == NFaces );

    if ( pMesh->m_pChunk[ChunkID].m_pFace )
        delete []pMesh->m_pChunk[ChunkID].m_pFace;
    pMesh->m_pChunk[ChunkID].m_pFace = pNewFaces;
    pMesh->m_pChunk[ChunkID].m_nFaces = NFaces;

    //---   add the new chunk
    MESHUTIL_AddChunk( pMesh, pNewChunk );

    //---   We undoubtedly have unused vertices now, get rid of them
//    MESHUTIL_RemoveUnusedVerts( pMesh );  //####
}

//==========================================================================

static
xbool IsVertUsed( mesh::chunk* pChunk, s32 VertID )
{
    s32 i;
    s32 N, NIndices;

    for ( i = 0; i < pChunk->m_nFaces; i++ )
    {
        if ( pChunk->m_pFace[i].m_bQuad )
            NIndices = 4;
        else
            NIndices = 3;

        for ( N = 0; N < NIndices; N++ )
        {
            if ( pChunk->m_pFace[i].m_Index[N] == VertID )
                return TRUE;
        }
    }
    return FALSE;
}

//==========================================================================

void MESHUTIL_RemoveUnusedVerts( mesh::object* pMesh )
{
    s32             i;
    s32             ChunkID;
    s32             NVertsAdded;
    s32             NVertsToAdd;
    mesh::vertex*   pNewVerts;

    for ( ChunkID = 0; ChunkID < pMesh->m_nChunks; ChunkID++ )
    {
        //---   how many verts are used?
        NVertsToAdd = 0;
        for ( i = 0; i < pMesh->m_pChunk[ChunkID].m_nVerts; i++ )
        {
            if ( IsVertUsed( &pMesh->m_pChunk[ChunkID], i ) )
            {
                NVertsToAdd++;
            }
        }

        //---   bail out early if we can
        if ( NVertsToAdd == pMesh->m_pChunk[ChunkID].m_nVerts )
            continue;

        //---   copy the used verts over into a new list
        pNewVerts = new mesh::vertex[NVertsToAdd];
        ASSERT( pNewVerts );
        NVertsAdded = 0;
        for ( i = 0; i < pMesh->m_pChunk[ChunkID].m_nVerts; i++ )
        {
            if ( IsVertUsed( &pMesh->m_pChunk[ChunkID], i ) == FALSE )
            {
                pNewVerts[NVertsAdded++] = pMesh->m_pChunk[ChunkID].m_pVertex[i];
            }
        }
        ASSERT( NVertsAdded = NVertsToAdd );

        //---   make the chunk point to our new list
        if ( pMesh->m_pChunk[ChunkID].m_pVertex )
            delete []pMesh->m_pChunk[ChunkID].m_pVertex;
        pMesh->m_pChunk[ChunkID].m_pVertex = pNewVerts;
        pMesh->m_pChunk[ChunkID].m_nVerts = NVertsAdded;
    }
}

//==========================================================================

void MESHUTIL_DeleteChunk( mesh::object* pMesh, s32 ChunkID )
{
    mesh::chunk*    pNewChunkList;
    s32             i;

    ASSERT( (ChunkID >= 0) && (ChunkID < pMesh->m_nChunks) );

    //---   try to bail early
    if ( pMesh->m_nChunks == 1 )
    {
        delete []pMesh->m_pChunk;
        pMesh->m_pChunk = NULL;
        pMesh->m_nChunks = 0;
        return;
    }

    //---   allocate space for a new list
    pNewChunkList = new mesh::chunk[pMesh->m_nChunks-1];
    
    //---   copy the chunks to our new list, sans the deleted one
    for ( i = 0; i < ChunkID; i++ )
    {
        MESHUTIL_ChunkCopy( &pNewChunkList[i], &pMesh->m_pChunk[i] );
    }
    for ( i = ChunkID + 1; i < pMesh->m_nChunks; i++ )
    {
        MESHUTIL_ChunkCopy( &pNewChunkList[i-1], &pMesh->m_pChunk[i] );
    }

    //---   set the mesh to point to our new chunk list
    if ( pMesh->m_pChunk )
        delete []pMesh->m_pChunk;
    pMesh->m_pChunk = pNewChunkList;
    pMesh->m_nChunks--;
}


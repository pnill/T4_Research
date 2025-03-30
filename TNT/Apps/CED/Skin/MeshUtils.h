#ifndef __MESHUTILS_H_INCLUDED__
#define __MESHUTILS_H_INCLUDED__

#include "Skel.h"

////////////////////////////////////////////////////////////////////////////
// Utilities for modifying a mesh
////////////////////////////////////////////////////////////////////////////

void    MESHUTIL_ChunkCopy          ( mesh::chunk* pDst,
                                      mesh::chunk* pSrc,
                                      char* NewName = NULL );
void    MESHUTIL_BoneCopy           ( mesh::bone* pDst, mesh::bone* pSrc );
void    MESHUTIL_TextureCopy        ( mesh::texture* pDst, mesh::texture* pSrc );
void    MESHUTIL_SubMaterialCopy    ( mesh::sub_material* pDst, mesh::sub_material* pSrc );
void    MESHUTIL_MaterialCopy       ( mesh::material* pDst, mesh::material* pSrc );
void    MESHUTIL_BaseInfoCopy       ( mesh::object* pDst, mesh::object* pSrc );
s32     MESHUTIL_GetMaterialID      ( mesh::object* pMesh, char* MatName );
s32     MESHUTIL_GetChunkID         ( mesh::object* pMesh, char* ChunkName );
//xbool   MESHUTIL_SaveMTF            ( mesh::object* pMesh, char* filename, QRipMorphTargetSet* pMorphSet = NULL );
void    MESHUTIL_MergeChunks        ( mesh::chunk* pDst, char* NewName,
                                      mesh::chunk* Chunk1,
                                      mesh::chunk* Chunk2 = NULL,
                                      mesh::chunk* Chunk3 = NULL,
                                      mesh::chunk* Chunk4 = NULL,
                                      mesh::chunk* Chunk5 = NULL,
                                      mesh::chunk* Chunk6 = NULL );
void    MESHUTIL_MergeChunks        ( mesh::chunk* pDst, char* NewName,
                                      s32 NChunksToMerge,
                                      mesh::chunk** pChunks );
void    MESHUTIL_RotateY180         ( mesh::object* pMesh );
void    MESHUTIL_RemapVertWeights   ( QSkel& rSkel, mesh::object* pGeom );
void    MESHUTIL_RemovePrefixes     ( mesh::object* pMesh, char* Prefix );
void    MESHUTIL_AddChunk           ( mesh::object* pDst, mesh::chunk* pSrc );
void    MESHUTIL_AddMaterial        ( mesh::object* pDst, mesh::material* pMat );
void    MESHUTIL_AddSubMaterial     ( mesh::object* pDst, mesh::sub_material* pSubMat );
void    MESHUTIL_AddTexture         ( mesh::object* pDst, mesh::texture* pTexture );
s32     MESHUTIL_GetNFacesUsingMat  ( mesh::chunk* pChunk, s32 MatID );
void    MESHUTIL_PullNamedMaterial  ( mesh::object* pMesh,
                                       char* MeshName,
                                       char* MatName,
                                       char* NewMeshName );
void    MESHUTIL_RemoveUnusedVerts  ( mesh::object* pMesh );
void    MESHUTIL_DeleteChunk        ( mesh::object* pMesh, s32 ChunkID );

#endif // __MESHUTILS_H_INCLUDED__

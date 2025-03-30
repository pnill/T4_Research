// INCLUDES
//=====================================================================================================================================
#include "x_files.hpp"
#include "XBOX_RipSkin.h"

// External Globals
//=====================================================================================================================================
extern xbool g_Verbose;

// Interal Globals
//=====================================================================================================================================
static char     gOutputBuffer[200];
static X_FILE*  gfp = NULL;

// Local Defines
//=====================================================================================================================================
//#define EXPORT_DETAILED_SKIN_INFO
#define BIG_S32_VALUE               65535
#define SKIN_MAX_NUM_TRIS           65535
#define SKIN_MAX_NUM_INDICES        65535
#define SKIN_MAX_NUM_VERTS_IN_STRIP 65535
#define SAFE_RELEASE_MESH_DATA( x ) { if( x != NULL ){ x_free( x ); x = NULL; }}
#define EPSILON                     0.0001f
#define FASTSWAP( a, b )            ((a) ^= (b) ^= (a) ^= (b))
#define FASTSWAP_FLOAT( a, b )      ((*(u32*)(&a)) ^= (*(u32*)(&b)) ^= (*(u32*)(&a)) ^= (*(u32*)(&b)) )

#define DISPLAY_FREE_MEM( x ) {\
\
    x_sprintf( gOutputBuffer, "\nFree Mem:%s, %d", #x, x_TotalMemoryFree( ) );\
    OutputDebugString( gOutputBuffer );\
    x_printf( gOutputBuffer );\
    x_fprintf( gfp, gOutputBuffer );\
}

#define OUTPUT_TO_USER_AND_FILE( x ) {\
\
    OutputDebugString( gOutputBuffer );\
    x_printf( gOutputBuffer );\
    x_fprintf( gfp, gOutputBuffer );\
}


// Local functions.
//=====================================================================================================================================
static s32 TriSortCompare( const void* Left, const void* Right );

//=====================================================================================================================================
QXBOXRipSkin::QXBOXRipSkin( void )
{
    mSkinName[0] = NULL;
    mActiveMesh = FALSE;
    
    mpMesh = NULL;
    mNMeshes = 0;

    mpSubMesh = NULL;
    mNSubMeshes = 0;

    mpTexture = NULL;
    mNTextures = 0;

    mpBone = NULL;
    mNBones = 0;

    if( gfp == NULL )
    {
        gfp = x_fopen( "XBOXRipSkinOutput.txt", "w" );
        ASSERT( gfp );
    }
}


//=====================================================================================================================================
QXBOXRipSkin::~QXBOXRipSkin( void )
{
    Clear();  // Destroy all resources associated to a SkinMesh.

    if( gfp != NULL )
    {
        x_fclose( gfp );
        gfp = NULL;
    }
}


//=====================================================================================================================================
void QXBOXRipSkin::Clear( void )
{
    s32 MeshID;

    mSkinName[0] = NULL;
    mActiveMesh = FALSE;
    
    // Destroy the meshes.
    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        SAFE_RELEASE_MESH_DATA( mpMesh[MeshID].mpD3DVerts );
        SAFE_RELEASE_MESH_DATA( mpMesh[MeshID].mpIndices );
        SAFE_RELEASE_MESH_DATA( mpMesh[MeshID].mpTris );
    }

    SAFE_RELEASE_MESH_DATA( mpMesh );
    mNMeshes = 0;

    SAFE_RELEASE_MESH_DATA( mpSubMesh );
    mNSubMeshes = 0;

    SAFE_RELEASE_MESH_DATA( mpTexture );
    mpTexture = 0;

    SAFE_RELEASE_MESH_DATA( mpBone );
    mNBones = 0;
}


//=====================================================================================================================================
void QXBOXRipSkin::SetSkinName( char* Name )
{
    x_strncpy( mSkinName, Name, SKIN_NAME_LENGTH );
}


//=====================================================================================================================================
void QXBOXRipSkin::AddBone( char* Name )
{
    XBOXSkinBone* pTempBonePointer = NULL;

    // Allocate the memory to add the extra bone to the skin file.
    pTempBonePointer = (XBOXSkinBone*)x_realloc( mpBone, sizeof( XBOXSkinBone ) * (mNBones + 1));
    ASSERT( pTempBonePointer );

    // Copy the new bone information.
    x_strncpy( pTempBonePointer[mNBones].mName, Name, SKIN_BONE_NAME_LENGTH );

    if( g_Verbose )
    {
        x_sprintf( gOutputBuffer, "Adding Bone %d --> %s\n", mNBones, pTempBonePointer[mNBones].mName );
        OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    }

    // Add the new bone to the skin mesh.
    mpBone = pTempBonePointer;

    // Increment the number of bones in the SkinMesh
    mNBones++;
}


//=====================================================================================================================================
void QXBOXRipSkin::NewTexture( char* TexturePath, s32 Width, s32 Height )
{
    XBOXSkinTexture* pTempTexture = NULL;

    // Width and Height don't seem to be needed anywhere, so ignore them.
    Width  = 0;
    Height = 0;

    // Allocate the memory to add the extra texture to the skin file.
    pTempTexture = (XBOXSkinTexture*)x_realloc( mpTexture, sizeof( XBOXSkinTexture ) * (mNTextures + 1) );
    ASSERT( pTempTexture );

    // Copy the new texture data to the new texture.
    x_strncpy( pTempTexture[mNTextures].mName, TexturePath, X_MAX_PATH );

    if( g_Verbose )
    {
        x_sprintf( gOutputBuffer, "Adding Texture %d --> %s\n", mNTextures, pTempTexture[mNTextures].mName );
        OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    }
    
    // Add the new texture to the Skin Mesh.
    mpTexture = pTempTexture;

    // Increment the number of textures in the SkinMesh
    mNTextures++;
}


//=====================================================================================================================================
void QXBOXRipSkin::NewMesh( char* MeshName, xbool EnvMapped, xbool Shadow, xbool Alpha )
{
    // Allocate enough space to store all previously recorded meshes as well as one more.
    XBOXRipSkinMesh* pTempMeshes;
    XBOXRipSkinMesh* pNewMeshStorage;
    s32            MeshFlags;
    s32            PreviousMeshSize;
    s32            TotalMeshSize;
    s32            i;

    // Calculate the actual size of the previously created meshes.
    PreviousMeshSize = 0;
    for( i = 0; i < mNMeshes; i++ )
    {
        PreviousMeshSize += sizeof( XBOXRipSkinMesh );
        PreviousMeshSize += sizeof( XBOXSkinD3DVert ) * mpMesh[i].mNVerts;
        PreviousMeshSize += sizeof( XBOXSkinMorphTarget ) * mpMesh[i].mNMorphTargets;
        PreviousMeshSize += sizeof( XBOXSkinMorphDelta ) * mpMesh[i].mNMorphDeltas;
    }
    TotalMeshSize = PreviousMeshSize + sizeof( XBOXRipSkinMesh );

    pTempMeshes = (XBOXRipSkinMesh*)x_realloc( mpMesh, TotalMeshSize );
    ASSERT( pTempMeshes );

    // Now Build the new mesh from the data passed in.
    pNewMeshStorage = &pTempMeshes[mNMeshes];

    MeshFlags = SKIN_MESH_FLAG_VISIBLE                         | 
                ((EnvMapped) ? (SKIN_MESH_FLAG_ENVMAPPED) : 0) |
                ((Shadow)    ? (SKIN_MESH_FLAG_SHADOW)    : 0) |
                ((Alpha)     ? (SKIN_MESH_FLAG_ALPHA)     : 0) ;

    x_strncpy( pNewMeshStorage->mName, MeshName, SKIN_MESH_NAME_LENGTH );
    pNewMeshStorage->mFlags         = MeshFlags;
    pNewMeshStorage->mRadius        = 0;
    pNewMeshStorage->mFirstSubMesh  = 0;
    pNewMeshStorage->mNSubMeshes    = 0;
    pNewMeshStorage->mpD3DVerts     = NULL;
    pNewMeshStorage->mNVerts        = 0;
    pNewMeshStorage->mpIndices      = NULL;
    pNewMeshStorage->mNIndices      = 0;
    pNewMeshStorage->mpTris         = NULL;
    pNewMeshStorage->mNTris         = 0;
    pNewMeshStorage->mpMorphTarget  = NULL;
    pNewMeshStorage->mNMorphTargets = 0;
    pNewMeshStorage->mpMorphDelta   = NULL;
    pNewMeshStorage->mNMorphDeltas  = 0;
    
    if( g_Verbose )
    {
        x_sprintf( gOutputBuffer, "Adding Mesh: %d --> %s\n", mNMeshes, pNewMeshStorage->mName  );
        OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    }

    // Attach the new meshes to the Skins mesh pointer.
    mpMesh = pTempMeshes;

    // Increment the mesh count in the Skin.
    mNMeshes++;
}


//=====================================================================================================================================
void QXBOXRipSkin::AddVert( t_RipVert& rRipVert )
{
    XBOXSkinD3DVert*  pTempVerts = NULL;
    XBOXSkinD3DVert*  pLastVert;
    XBOXRipSkinMesh*  pLastCreatedMesh;
    s32             CurrentVertCount;

    ASSERT( mNMeshes );
    pLastCreatedMesh = &mpMesh[mNMeshes - 1];

    // Allocate the memory required for the new vertex data.
    CurrentVertCount = pLastCreatedMesh->mNVerts;

    pTempVerts = (XBOXSkinD3DVert*)x_realloc( pLastCreatedMesh->mpD3DVerts, sizeof( XBOXSkinD3DVert ) * (CurrentVertCount + 1) );
    ASSERT( pTempVerts );

    // Copy the vertex data to the new vertex.
    pLastVert = &pTempVerts[CurrentVertCount];

    pLastVert->mPos             = rRipVert.Pos;
    pLastVert->mBlendWeights[0] = rRipVert.MatrixWeight[0];
    pLastVert->mBlendWeights[1] = rRipVert.MatrixWeight[1];
    pLastVert->mBlendWeights[2] = rRipVert.MatrixWeight[2];
    pLastVert->mMatrixIDs[0]    = static_cast<u8>(rRipVert.MatrixID[0]);
    pLastVert->mMatrixIDs[1]    = static_cast<u8>(rRipVert.MatrixID[1]);
    pLastVert->mMatrixIDs[2]    = static_cast<u8>(rRipVert.MatrixID[2]);
    pLastVert->mNormal          = rRipVert.Normal;
//    pLastVert->mDiffuse         = rRipVert.Color;
    pLastVert->mUV              = rRipVert.UV;

    // Add the new vertex to the last mesh.
    pLastCreatedMesh->mpD3DVerts = pTempVerts;

    // Increment the number of textures in the SkinMesh
    pLastCreatedMesh->mNVerts++;
}


//=====================================================================================================================================
void QXBOXRipSkin::AddTri( s32 Vert0, s32 Vert1, s32 Vert2, char* pTextureName )
{
    s32             i;
    s32             TriID;
    s32             CurrentTriCount;
    XBOXRipSkinTri    NewTri;
    XBOXRipSkinTri*   pTempTris = NULL;
    XBOXRipSkinMesh*  pLastCreatedMesh;
    XBOXSkinD3DVert*  pCurrentVertex;
    vector3         Edge1, Edge2;
    s32             VertTestLeft[3], VertTestRight[3];

    ASSERT( mNMeshes );
    pLastCreatedMesh = &mpMesh[mNMeshes - 1];
    pCurrentVertex   = pLastCreatedMesh->mpD3DVerts;

    // Test to see if this triangle is just a line, if it is, nuke it.  It's no good to us.
    if( (pCurrentVertex[Vert0].mPos == pCurrentVertex[Vert1].mPos) ||
        (pCurrentVertex[Vert0].mPos == pCurrentVertex[Vert2].mPos) ||
        (pCurrentVertex[Vert1].mPos == pCurrentVertex[Vert2].mPos) )
    {
        return;
    }

    // Allocate the memory required for the new tri data.
    CurrentTriCount = pLastCreatedMesh->mNTris;

    // Copy the triangle data to the new triangle.
    NewTri.mIndex[0] = Vert0;
    NewTri.mIndex[1] = Vert1;
    NewTri.mIndex[2] = Vert2;

    // Calculate the Area, and normal of the tri to help with the re-calculateions of the normal.
    Edge1 = pCurrentVertex[NewTri.mIndex[1]].mPos - pCurrentVertex[NewTri.mIndex[0]].mPos;
    Edge2 = pCurrentVertex[NewTri.mIndex[2]].mPos - pCurrentVertex[NewTri.mIndex[0]].mPos;

    NewTri.mNormal = Edge1.Cross( Edge2 );
    NewTri.mArea   = NewTri.mNormal.Length() * 0.5f;
    NewTri.mNormal.Normalize( );

    // Search through the list of textures that this Skin has to find which texture to use.
    NewTri.mMaterialID = -1; // Set to -1 if to default to no texture.
    for( i = 0; i < mNTextures; i++ )
    {
        char* pCurrentTestTextureName;
        pCurrentTestTextureName = mpTexture[i].mName;

        // Compare the names of the textures to see if you find a match, when you do, that offset will
        // become the material ID for this triangle.
        if( x_strcmp( pCurrentTestTextureName, pTextureName ) == 0 )
        {
            NewTri.mMaterialID = i;
            break;
        }
    }

    // Keep a temporary copy of the indexes that make up the triangle in numerical order to make the
    // following tests less complex.
    VertTestLeft[0] = NewTri.mIndex[0];
    VertTestLeft[1] = NewTri.mIndex[1];
    VertTestLeft[2] = NewTri.mIndex[2];

    if( VertTestLeft[0] > VertTestLeft[1] ) FASTSWAP( VertTestLeft[0], VertTestLeft[1] );
    if( VertTestLeft[1] > VertTestLeft[2] ) FASTSWAP( VertTestLeft[1], VertTestLeft[2] );
    if( VertTestLeft[0] > VertTestLeft[1] ) FASTSWAP( VertTestLeft[0], VertTestLeft[1] );

    // Determine if this triangle is a duplicate, if it is, remove it.
    for( TriID = 0; TriID < CurrentTriCount; TriID++ )
    {
        // Make a temporary copy of the indexes that make up the triangle from this mesh's triangle array 
        // then sort them in numerical order to make the following tests less complex.
        VertTestRight[0] = pLastCreatedMesh->mpTris[TriID].mIndex[0];
        VertTestRight[1] = pLastCreatedMesh->mpTris[TriID].mIndex[1];
        VertTestRight[2] = pLastCreatedMesh->mpTris[TriID].mIndex[2];

        if( VertTestRight[0] > VertTestRight[1] ) FASTSWAP( VertTestRight[0], VertTestRight[1] );
        if( VertTestRight[1] > VertTestRight[2] ) FASTSWAP( VertTestRight[1], VertTestRight[2] );
        if( VertTestRight[0] > VertTestRight[1] ) FASTSWAP( VertTestRight[0], VertTestRight[1] );

        if( (VertTestLeft[0] == VertTestRight[0]  ) && 
            (VertTestLeft[1] == VertTestRight[1]  ) && 
            (VertTestLeft[2] == VertTestRight[2]  ) && 
            (pLastCreatedMesh->mpTris[TriID].mMaterialID == NewTri.mMaterialID) )
        {
            // This is a duplicate triangle, get rid of it.
            return;
        }
    }

    // Expand the tri array, then add the new try to it.
    pTempTris = (XBOXRipSkinTri*)x_realloc( pLastCreatedMesh->mpTris, sizeof( XBOXRipSkinTri ) * (CurrentTriCount + 1) );
    ASSERT( pTempTris );

    pTempTris[CurrentTriCount] = NewTri;

    // Have the mesh get access to the new extended tri array.
    pLastCreatedMesh->mpTris = pTempTris;

    // Increment the number of textures in the SkinMesh
    pLastCreatedMesh->mNTris++;
}


//=====================================================================================================================================
void QXBOXRipSkin::NewMorphTarget( char* TargetName )
{
    XBOXRipSkinMesh*      pLastCreatedMesh;
    XBOXSkinMorphTarget*  pTempMorphTargets;
    XBOXSkinMorphTarget*  pLastMorphTarget;
    s32                 CurrentMorphTargetCount;

    // Get a pointer to the last mesh in the Skin.
    ASSERT( mNMeshes );
    pLastCreatedMesh = &mpMesh[mNMeshes - 1];

    // Allocate the memory required for the MorphTarget.
    CurrentMorphTargetCount = pLastCreatedMesh->mNMorphTargets;
    pTempMorphTargets = (XBOXSkinMorphTarget*)x_realloc( pLastCreatedMesh->mpMorphTarget, sizeof( XBOXSkinMorphTarget ) * (CurrentMorphTargetCount + 1) );
    ASSERT( pTempMorphTargets );

    // Copy the MorphTarget data passed in to the new allocated morph target.
    pLastMorphTarget = &pTempMorphTargets[CurrentMorphTargetCount];

    x_strncpy( pLastMorphTarget->Name, TargetName, SKIN_MORPH_NAME_LENGTH );
    pLastMorphTarget->FirstDeltaID = pLastCreatedMesh->mNMorphDeltas;
    pLastMorphTarget->NDeltas = 0;

    // Add the Morph Target to the last Mesh.
    pLastCreatedMesh->mpMorphTarget = pTempMorphTargets;

    // Increment the Morph Target count.
    pLastCreatedMesh->mNMorphTargets++;
}


//=====================================================================================================================================
void QXBOXRipSkin::AddMorphDelta( s32 VertID, vector3 Delta )
{
    XBOXRipSkinMesh*      pLastCreatedMesh;
    XBOXSkinMorphDelta*   pTempMorphDeltas;
    XBOXSkinMorphDelta*   pLastMorphDelta;
    s32                 CurrentMorphDeltaCount;

    // Get a pointer to the last mesh in the Skin.
    ASSERT( mNMeshes );
    pLastCreatedMesh = &mpMesh[mNMeshes - 1];

    // Allocate the memory required for the new MorphTargetDelta.
    CurrentMorphDeltaCount = pLastCreatedMesh->mNMorphDeltas;
    pTempMorphDeltas = (XBOXSkinMorphDelta*)x_realloc( pLastCreatedMesh->mpMorphDelta, sizeof( XBOXSkinMorphDelta ) * (CurrentMorphDeltaCount + 1) );
    ASSERT( pTempMorphDeltas );

    // Copy the MorphTargetDelta data passed in to the new allocated morph target delta.
    pLastMorphDelta = &pTempMorphDeltas[CurrentMorphDeltaCount];
    pLastMorphDelta->mDelta  = Delta; 
    pLastMorphDelta->mVertID = VertID;

    // Assign the newly allocated morph target deltas to the last mesh.
    pLastCreatedMesh->mpMorphDelta = pTempMorphDeltas;

    // Increment the Morph Target Delta count.
    pLastCreatedMesh->mNMorphDeltas++;
}


//=====================================================================================================================================
void QXBOXRipSkin::ForceSingleMatrix( void )
{
    u16             MeshID;
    s32             VertexID;
    XBOXRipSkinMesh*  pCurrentMesh  = NULL;
    XBOXSkinD3DVert*  pCurrentVerts = NULL;

    // Go through each mesh.
    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh  = &mpMesh[MeshID];
        pCurrentMesh->mVertexFormat = NULL;
        pCurrentVerts = pCurrentMesh->mpD3DVerts;
        
        // Go through each vert in the mesh.
        for( VertexID = 0; VertexID < pCurrentMesh->mNVerts; VertexID++ )
        {
            ReWeightToSingleMatrix( &pCurrentVerts[VertexID] );
        }
    }
}


//=====================================================================================================================================
void QXBOXRipSkin::ForceDoubleMatrix( void )
{
    u16             MeshID;
    s32             VertexID;
    XBOXRipSkinMesh*  pCurrentMesh  = NULL;
    XBOXSkinD3DVert*  pCurrentVerts = NULL;

    // Go through each mesh.
    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh  = &mpMesh[MeshID];
        pCurrentMesh->mVertexFormat = NULL;//SKIN_VERTFORMAT_BLEND3;
        pCurrentVerts = pCurrentMesh->mpD3DVerts;
        
        // Go through each vert in the mesh.
        for( VertexID = 0; VertexID < pCurrentMesh->mNVerts; VertexID++ )
        {
            ReWeightToDoubleMatrix( &pCurrentVerts[VertexID] );
        }
    }
}


//=====================================================================================================================================
void QXBOXRipSkin::ForceTripleMatrix( void )
{
    u16             MeshID;
    s32             VertexID;
    XBOXRipSkinMesh*  pCurrentMesh  = NULL;
    XBOXSkinD3DVert*  pCurrentVerts = NULL;

    // Go through each mesh.
    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh  = &mpMesh[MeshID];
        pCurrentMesh->mVertexFormat = NULL;//SKIN_VERTFORMAT_BLEND3;
        pCurrentVerts = pCurrentMesh->mpD3DVerts;
        
        // Go through each vert in the mesh.
        for( VertexID = 0; VertexID < pCurrentMesh->mNVerts; VertexID++ )
        {
            ReWeightToTripleMatrix( &pCurrentVerts[VertexID] );
        }
    }
}



//=====================================================================================================================================
void QXBOXRipSkin::PrepareStructures( void )
{
    // Re-compute the normals for each chunk, the normals provided by the .MTF file can be incorrect.
    ReComputeNormals( );

    // Go through all of the vertex points in the entire skin, and blend any normals that share the same vertex position.
    //
    // The reason this needs to be done is because some vertices are copied to allow the skin to be broken up into 
    // several parts, such and hands, and legs etc...  These copied vertices, will not share the same polygon connections, 
    // and because of that, even though the vertices share the same space, they don't share the same vertex normal.
    // As a result you will see a distinct difference in the two connecting pieces because the lighting will emphasize the
    // different vertex normals.
    BlendVertexNormals( );

    // Determine the number of submeshes that are created for ALL the meshes used in this Skin.
    DetermineTheTotalSubMeshCount( );

    // Fill out the data for each submesh in each mesh
    SeparateMeshesIntoSubmeshes( );

    // Calculate the radius of all of the meshes.
    CalcalateMeshRadius( );
}


//=====================================================================================================================================
void QXBOXRipSkin::DisplayStats( void )
{
    s32             MeshID;
    s32             SubmeshID;
    XBOXRipSkinMesh*  pCurrentMesh;
    XBOXSkinSubmesh*  pCurrentSubmesh;

    x_sprintf( gOutputBuffer, "  \n" );                                                          OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  =======================================================\n");    OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  SKIN STATS    \n");                                             OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  =======================================================\n");    OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  Skin Name:  %s\n", mSkinName );                                 OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  NMeshes:    %d\n", mNSubMeshes );                               OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  NSubMeshes: %d\n", mNMeshes );                                  OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  NTextures:  %d\n", mNTextures );                                OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  NBones:     %d\n", mNBones );                                   OUTPUT_TO_USER_AND_FILE( gOutputBuffer );

    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh = &mpMesh[MeshID];

        x_sprintf( gOutputBuffer, "  \n" );                                                          OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        x_sprintf( gOutputBuffer, "  \n" );                                                          OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        x_sprintf( gOutputBuffer, "  =======================================================\n");    OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        x_sprintf( gOutputBuffer, "  Mesh %d --> %s\n", MeshID, pCurrentMesh->mName );               OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        x_sprintf( gOutputBuffer, "  =======================================================\n");    OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        x_sprintf( gOutputBuffer, "  Total Submeshes: %d\n", pCurrentMesh->mNSubMeshes );            OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        x_sprintf( gOutputBuffer, "  Total Verts: %d\n", pCurrentMesh->mNVerts );                    OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        x_sprintf( gOutputBuffer, "  Total Indices: %d\n", pCurrentMesh->mNIndices );                OUTPUT_TO_USER_AND_FILE( gOutputBuffer );

        for( SubmeshID = 0; SubmeshID < pCurrentMesh->mNSubMeshes; SubmeshID++ )
        {
            pCurrentSubmesh = &mpSubMesh[pCurrentMesh->mFirstSubMesh + SubmeshID];

            x_sprintf( gOutputBuffer, "  \n" );                                                                      OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            x_sprintf( gOutputBuffer, "  -------------------------------------------------------\n");                OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            x_sprintf( gOutputBuffer, "  Submesh %d of %d\n", SubmeshID + 1, pCurrentMesh->mNSubMeshes );            OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            x_sprintf( gOutputBuffer, "  -------------------------------------------------------\n");                OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            x_sprintf( gOutputBuffer, "  MaterialID:         %d\n", pCurrentSubmesh->TextureID );                   OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            x_sprintf( gOutputBuffer, "  Number of Polygons: %d\n", pCurrentSubmesh->mNPrims );                      OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            x_sprintf( gOutputBuffer, "  Smallest Vertex ID: %d\n", pCurrentSubmesh->mVertexMin );                   OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            x_sprintf( gOutputBuffer, "  Total Verts Used:   %d\n", pCurrentSubmesh->mVertexCount );                 OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            x_sprintf( gOutputBuffer, "  First Index in Mesh Indice array: %d\n", pCurrentSubmesh->mIndexStart );    OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        }
    }

    x_sprintf( gOutputBuffer, "  \n" );                                                         OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    x_sprintf( gOutputBuffer, "  =======================================================\n");   OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
}


//=====================================================================================================================================
void QXBOXRipSkin::Save( char* pFileName )
{
    X_FILE* SkinFile;

    SkinFile = x_fopen( pFileName, "wb" ); ASSERT( SkinFile );

    BuildAndWriteSkinFileHeader( SkinFile );
    WriteMeshData( SkinFile );
    WriteSubmeshData( SkinFile );
    WriteTextureData( SkinFile );
    WriteSkeletonData( SkinFile );

    x_fclose( SkinFile );
}


//=====================================================================================================================================
void QXBOXRipSkin::ReWeightToSingleMatrix( XBOXSkinD3DVert* pCurrentVert )
{
    u32 i;
    s8  MaxIndex = 0;

    // Now test each matrix attached to this mesh to find the one which has the greatest influence on this vertex.
    for( i = 0; i < MAX_XBOX_MATRICES_PER_VERT; i++ )
    {
        // If the matrix is being used, then test to see if that matrix has the largest blend weight.
        if( pCurrentVert->mMatrixIDs[i] != UNUSED_MATRIX_ID )
        {
            if( pCurrentVert->mBlendWeights[i] > pCurrentVert->mBlendWeights[MaxIndex] )
            {
                MaxIndex = i;
            }
        }
        // Break at the first unused matrix.
        else
        {
            break;
        }
    }

    // Make sure that there was atleast 1 matrix.
    ASSERT( i );

    // Now force the vertex to use the matrix with the largest blend weight, then force the 
    // vertex to only use that blend weight.
    pCurrentVert->mMatrixIDs[0]    = pCurrentVert->mMatrixIDs[MaxIndex];
    pCurrentVert->mMatrixIDs[1]    = pCurrentVert->mMatrixIDs[0];
    pCurrentVert->mMatrixIDs[2]    = pCurrentVert->mMatrixIDs[0];
    pCurrentVert->mBlendWeights[0] = 1.0f;
    pCurrentVert->mBlendWeights[1] = 0.0f;
}

//=====================================================================================================================================
void QXBOXRipSkin::ReWeightToDoubleMatrix( XBOXSkinD3DVert* pCurrentVert )
{
    s8  i;
    s8  MaxIndex;
    s8  NextMaxIndex;
    s8  Matrix1;
    s8  Matrix2;
    f32 Weight1;
    f32 Weight2;
    s8  NMatrices;
    f32 BlendWeights[MAX_XBOX_MATRICES_PER_VERT];

    // Make a copy of the current verts blend matrices for testing purposes.
    BlendWeights[0] = pCurrentVert->mBlendWeights[0];
    BlendWeights[1] = pCurrentVert->mBlendWeights[1];
    BlendWeights[2] = 1.0f - (BlendWeights[0] + BlendWeights[1]);

    NMatrices = 0;
    for( i = 0; i < MAX_XBOX_MATRICES_PER_VERT; i++ )
    {
        if( pCurrentVert->mMatrixIDs[i] != UNUSED_MATRIX_ID )
            NMatrices++;
    }
    ASSERT( NMatrices );

    // Pre sort the first 1 or 2 matrices based on their blending weights.
    if( NMatrices > 1 )
    {
        if( BlendWeights[0] <= BlendWeights[1] )
        {
            MaxIndex = 0;
            NextMaxIndex = 1;
        }
        else
        {
            MaxIndex = 1;
            NextMaxIndex = 0;
        }

        for( i = 0; i < NMatrices; i++ )
        {
            if( BlendWeights[i] > BlendWeights[MaxIndex] )
            {
                // If you are not going to set up the Next best value to a garbage matrix, have the
                // next max index set to whatever the old max was.
                if( pCurrentVert->mMatrixIDs[MaxIndex] != UNUSED_MATRIX_ID )
                    NextMaxIndex = MaxIndex;

                MaxIndex = i;
            }
        }
    }
    // There's only one matrix, it must be the first one.  so set it up as the max and next max.
    else
    {
        MaxIndex = 0;
        NextMaxIndex = 0;
    }

    Matrix1 = (s8)pCurrentVert->mMatrixIDs[MaxIndex];
    Matrix2 = (s8)pCurrentVert->mMatrixIDs[NextMaxIndex];
    pCurrentVert->mMatrixIDs[0] = Matrix1;
    pCurrentVert->mMatrixIDs[1] = Matrix2;
    pCurrentVert->mMatrixIDs[2] = Matrix2;

    // As far as the vertex weighting is concerned, since we are only using 2 vertex weights, instead of using
    // the blend weight of the most influential matrix, have use 1.0f - the next largest blend weight.
    // In doing this, the most influential matrix remains just that.  If there was a case where there were
    // originally 3 matrices assigned to this vertex, and the most blending weights were assigned as
    // 0.40, 0.35, 0.25, and we assigned 0.4 to the first matrix since it was had the highest blend weight
    // then we would incorrectly leave 0.6 blend weight for the 2nd highest by default, If we use 1.0 - 0.35 
    // the most influential blend weight will become 0.65.  Though it's not correct, it's more correct than
    // using the default value.

    // If there is only 1 matrix attached to this vertex, just set the weight to be 1.0f on the first weight and 0.0f 
    // on the others.
    if( NextMaxIndex != MaxIndex )
    {
        Weight1 = 1.0f - BlendWeights[NextMaxIndex];
        Weight2 = 1.0f - Weight1; // Calculate the 2nd weight to ensure that they both add up to 1.0f exactly.
    }
    else
    {
        Weight1 = 1.0f;
        Weight2 = 0.0f;
    }

    pCurrentVert->mBlendWeights[0] = Weight1;
    pCurrentVert->mBlendWeights[1] = Weight2;
}

//=====================================================================================================================================
void QXBOXRipSkin::ReWeightToTripleMatrix( XBOXSkinD3DVert* pCurrentVert )
{
    u32 i;
    u32 OriginalMatrixCount;
    s8  MatrixOrderByWeight[MAX_XBOX_MATRICES_PER_VERT];
    s8  TempMatrixIDs[MAX_XBOX_MATRICES_PER_VERT];
    f32 BlendWeightsIn[MAX_XBOX_MATRICES_PER_VERT];
    f32 BlendWeightsOut[MAX_XBOX_MATRICES_PER_VERT];

    OriginalMatrixCount = 0;
    
    // Make a copy of the current verts blend matrices for testing purposes.
    BlendWeightsIn[0] = pCurrentVert->mBlendWeights[0];
    BlendWeightsIn[1] = pCurrentVert->mBlendWeights[1];
    BlendWeightsIn[2] = 1.0f - (BlendWeightsIn[0] + BlendWeightsIn[1]);

    // Determine how many matrices are assigned to this vertex.
    for( i = 0; i < MAX_XBOX_MATRICES_PER_VERT; i++ )
    {
        if( pCurrentVert->mMatrixIDs[i] != UNUSED_MATRIX_ID )
        {
            OriginalMatrixCount++;
        }
        // Break at the first unused matrix.
        else
        {
            break;
        }
    }
    // Make sure that there was atleast 1 matrix.
    ASSERT( OriginalMatrixCount );

    // Force the vertex to use 3 matrix values
    switch( OriginalMatrixCount )
    {
        case 1:
        {
            pCurrentVert->mMatrixIDs[0] = pCurrentVert->mMatrixIDs[0]; // Yes all 3 should be matrix 0.
            pCurrentVert->mMatrixIDs[1] = pCurrentVert->mMatrixIDs[0];
            pCurrentVert->mMatrixIDs[2] = pCurrentVert->mMatrixIDs[0];

            // Since there was only 1 original matrix, divide the blend weights evenly, 0.34, 0.33, and 0.33.
            BlendWeightsOut[0] = 0.34f;
            BlendWeightsOut[1] = 0.33f;
            BlendWeightsOut[2] = 1.0f - (BlendWeightsOut[0] + BlendWeightsOut[1]);
            break;
        }

        case 2:
        {
            if( BlendWeightsIn[0] > BlendWeightsIn[1] )
            {
                MatrixOrderByWeight[0] = 0;
                MatrixOrderByWeight[1] = 1;
            }
            else
            {
                MatrixOrderByWeight[0] = 1;
                MatrixOrderByWeight[1] = 0;
            }

            // Copy the matrix ID's according to their weighting value.
            TempMatrixIDs[0] = (s8)pCurrentVert->mMatrixIDs[MatrixOrderByWeight[0]];
            TempMatrixIDs[1] = (s8)pCurrentVert->mMatrixIDs[MatrixOrderByWeight[1]];
            TempMatrixIDs[2] = (s8)pCurrentVert->mMatrixIDs[MatrixOrderByWeight[1]]; // Yes this 2nd 1 is correct.

            // Reassign the vertex's matrix ID's
            pCurrentVert->mMatrixIDs[0] = TempMatrixIDs[0];
            pCurrentVert->mMatrixIDs[1] = TempMatrixIDs[1];
            pCurrentVert->mMatrixIDs[2] = TempMatrixIDs[2];

            // Determine the blending weights.  Make sure that the most influential remains the most influential.
            BlendWeightsOut[0] = 1.0f - BlendWeightsIn[MatrixOrderByWeight[1]];
            BlendWeightsOut[1] = BlendWeightsIn[MatrixOrderByWeight[1]] * 0.5f;
            BlendWeightsOut[2] = 1.0f - (BlendWeightsOut[0] + BlendWeightsOut[1]);
            break;
        }

        case 3:
        {
            // Since there were 3 matricies applied to this vertex in the first place, there really isn't much
            // to do with this vertex, but to remain in the same spirit as the other 2 cases,  the matrix ID's
            // and blending weights will be reordered by highest weight first.
            xbool Done;
            s8    TempMatrixID;
            f32   TempWeight;


            // Using a bubble sort, just sort the matrices and weights within the 3 values attached to this vert.
            do
            {
                Done = TRUE;

                for( i = 0; i < 2; i++ )
                {
                    // If the next vertex blend weight is larger than the current, swap them.
                    if( BlendWeightsIn[i] < BlendWeightsIn[i + 1] )
                    {
                        TempWeight                    = BlendWeightsIn[i+1];
                        TempMatrixID                  = (s8)pCurrentVert->mMatrixIDs[i+1];

                        BlendWeightsIn[i+1]           = BlendWeightsIn[i];
                        pCurrentVert->mMatrixIDs[i+1] = pCurrentVert->mMatrixIDs[i];

                        BlendWeightsIn[i]             = TempWeight;
                        pCurrentVert->mMatrixIDs[i]   = TempMatrixID;

                        Done = FALSE;
                    }
                }

            }while( !Done );

            // Copy the Sorted BlendWeightsIn to the BlendWeightsOut.
            BlendWeightsOut[0] = BlendWeightsIn[0];
            BlendWeightsOut[1] = BlendWeightsIn[1];
            BlendWeightsOut[2] = 1.0f - (BlendWeightsIn[0] + BlendWeightsIn[1]);
            break;
        }

        default:
            ASSERT( 0 );
    }

    // Reassign the blending weights.
    pCurrentVert->mBlendWeights[0] = BlendWeightsOut[0];
    pCurrentVert->mBlendWeights[1] = BlendWeightsOut[1];
}


//=====================================================================================================================================
typedef struct SNormalVert
{
    vector3 Pos;
    vector3 Normal;
} t_NormalVert;

void QXBOXRipSkin::ReComputeNormals( void )
{
    s32             MeshID, VertID, TempVertID, TriID, TriIndex;
    s32             nTempVerts;
    SNormalVert*    pTempVerts;

    s32             nCurrentMeshVerts;
    s32             nCurrentMeshTris;
    XBOXRipSkinMesh*  pCurrentMesh;
    XBOXSkinD3DVert*  pCurrentVerts;
    XBOXRipSkinTri*   pCurrentTris;


    // Re-calculate the normals for each vertex used in each mesh.
    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh      = &mpMesh[MeshID];
        pCurrentVerts     = pCurrentMesh->mpD3DVerts;
        nCurrentMeshVerts = pCurrentMesh->mNVerts;
        nCurrentMeshTris  = pCurrentMesh->mNTris;
        pCurrentTris      = pCurrentMesh->mpTris;

        // Clear the normal used in each vert.
        for( VertID = 0; VertID < mpMesh[MeshID].mNVerts; VertID++ )
            pCurrentVerts[VertID].mNormal.Zero( );

        // Allocate some special temporary verts.
        pTempVerts = new SNormalVert[nCurrentMeshVerts];
        ASSERT( pTempVerts );
        nTempVerts = 0;

        // To avoid normals looking weird across texture seams, do them based on position (NOT INDEX)
        // Create a list of vertices with nothing but position.
        for( VertID = 0; VertID < nCurrentMeshVerts; VertID++ )
        {
            // Go through the temp list and compare it to the current REAL mesh vertex list.
            // Determine if the Current REAL vertex list is not already in the temp list.
            // If you find a match, exit early, and don't record the second instance of the vert from the
            // REAL list to the temp list.  We are trying to remove verts that are almost identical.
            for( TempVertID = 0; TempVertID < nTempVerts; TempVertID++ )
            {
                if( ( x_abs(pTempVerts[TempVertID].Pos.X - pCurrentVerts[VertID].mPos.X) < EPSILON ) &&
                    ( x_abs(pTempVerts[TempVertID].Pos.Y - pCurrentVerts[VertID].mPos.Y) < EPSILON ) &&
                    ( x_abs(pTempVerts[TempVertID].Pos.Z - pCurrentVerts[VertID].mPos.Z) < EPSILON ) )
                {
                    break;
                }
            }

            // Add the unique vertex to the temporary list.
            if( TempVertID == nTempVerts )
            {
                // The current vert wasn't found, so add it
                pTempVerts[nTempVerts].Normal.Zero();
                pTempVerts[nTempVerts].Pos = pCurrentVerts[VertID].mPos;
                nTempVerts++;
            }
        }

        // For each vert that a triangle uses, add in the triangle's normal, weighted by its area
        for( TriID = 0; TriID < nCurrentMeshTris; TriID++ )
        {
            for( TriIndex = 0; TriIndex < 3; TriIndex++ )
            {
                // Find each of the verts in the temporary list
                for( TempVertID = 0; TempVertID < nTempVerts; TempVertID++ )
                {
                    if( ( x_abs(pTempVerts[TempVertID].Pos.X - pCurrentVerts[pCurrentTris[TriID].mIndex[TriIndex]].mPos.X) < EPSILON ) &&
                        ( x_abs(pTempVerts[TempVertID].Pos.Y - pCurrentVerts[pCurrentTris[TriID].mIndex[TriIndex]].mPos.Y) < EPSILON ) &&
                        ( x_abs(pTempVerts[TempVertID].Pos.Z - pCurrentVerts[pCurrentTris[TriID].mIndex[TriIndex]].mPos.Z) < EPSILON ) )
                    {
                        break;
                    }
                }
                ASSERT( TempVertID < nTempVerts );

                pTempVerts[TempVertID].Normal += pCurrentTris[TriID].mNormal * pCurrentTris[TriID].mArea;
            }
        }

        // Now copy the new vertex normals back into the current meshes vertex list.
        for( TempVertID = 0; TempVertID < nTempVerts; TempVertID++ )
        {
            // Go through each vertex in this mesh to find a simlar vertex, then copy it normal to it.
            for( VertID = 0; VertID < nCurrentMeshVerts; VertID++ )
            {
                if( (x_abs(pTempVerts[TempVertID].Pos.X - pCurrentVerts[VertID].mPos.X) < EPSILON) &&
                    (x_abs(pTempVerts[TempVertID].Pos.Y - pCurrentVerts[VertID].mPos.Y) < EPSILON) &&
                    (x_abs(pTempVerts[TempVertID].Pos.Z - pCurrentVerts[VertID].mPos.Z) < EPSILON) )
                {
                    pCurrentVerts[VertID].mNormal = pTempVerts[TempVertID].Normal;
                }
            }
        }

        // Release the temporary vertex array.
        delete []pTempVerts;

        // Re-Normalize the new vertex normals
        for( VertID = 0; VertID < nCurrentMeshVerts; VertID++ )
        {
            pCurrentVerts[VertID].mNormal.Normalize();
        }
    }
}

//=====================================================================================================================================
void QXBOXRipSkin::BlendVertexNormals( void )
{
    VertList        VList;
    VertList*       pCurrentList;
    VertNode*       pTestNode;

    vector3*        pvTestPosition;
    vector3*        pvArchivedPosition;
    vector3*        pvTestNormal;
    vector3*        pvArchivedNormal;
    f32             NormalDot;

    s32             MeshID;
    s32             VertID;

    s32             CommonVerts;
    vector3         AvgNormal;
    
    // Go through the entire skin to find duplicate points.    
    if( g_Verbose )
    {
        x_sprintf( gOutputBuffer, "Blending the vertex normals.\n" );
        OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    }

    // Go through each of the meshes, and each vertex within each mesh to find any vertex position that shares the
    // same space as any other vertex, and whos normal shares the same half space.
    // (meaning the normals aren't facing in completely opposite direction )
    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        if( g_Verbose )
        {
            x_sprintf( gOutputBuffer, "Comparing verts from MeshID %d: ", MeshID );
            OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        }

        for( VertID = 0; VertID < mpMesh[MeshID].mNVerts; VertID++ )
        {
            if( g_Verbose )
            {
                if( VertID % 25 == 0 )
                {
                    x_sprintf( gOutputBuffer, "." );
                    OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
                }
            }

            // Start at the top of the list.
            pCurrentList = &VList;

            // Grab the vertex to test against.
            pvTestPosition = &mpMesh[MeshID].mpD3DVerts[VertID].mPos;
            pvTestNormal   = &mpMesh[MeshID].mpD3DVerts[VertID].mNormal;

            // Loop through the list of points until something is found.
            while( TRUE )
            {
                pTestNode = pCurrentList->mpHead;

                // If there is a node there, test the stored vertex with the current vert.
                if( pTestNode )
                {
                    pvArchivedPosition = &mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mPos;
                    pvArchivedNormal   = &mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mNormal;

                    NormalDot = pvTestNormal->Dot( *pvArchivedNormal );

                    // Test to see if this vertex is a duplicate.
                    if( (x_abs( pvTestPosition->X - pvArchivedPosition->X) < EPSILON) &&
                        (x_abs( pvTestPosition->Y - pvArchivedPosition->Y) < EPSILON) &&
                        (x_abs( pvTestPosition->Z - pvArchivedPosition->Z) < EPSILON) && 
                        NormalDot > 0.0f )
                    {
                        // Add the duplicate node to the current list, then break from the while loop
                        pCurrentList->AddNode( MeshID, VertID );
                        break;
                    }
                    // Not a duplicate, so walk the lists of lists to the next linked list.
                    else
                    {
                        // If you can walk the list, then go to the next list.
                        if( pCurrentList->mpNextList )
                        {
                            pCurrentList = pCurrentList->mpNextList;
                        }
                        // If there are no more lists, and you haven't found a match, this must be a unique
                        // vertex, so create a new Linked list.
                        else
                        {
                            VList.AttachList( MeshID, VertID );
                            break;
                        }
                    }
                }
                // There are no nodes in this list, so just add this one.
                else
                {
                    pCurrentList->AddNode( MeshID, VertID );
                    break;
                }
            }
        }        

        if( g_Verbose )
        {
            x_sprintf( gOutputBuffer, "\n" );
            OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
        }
    }


    // Now we have a linked list of linked lists where each 2nd generation list has a chain of verts that share the same
    // space.  Time to average these normals and shove them back into the correct place.
    //-------------------------------------------------------------------------------------------------------------------------------------
    pCurrentList = &VList;
    
    while( pCurrentList )
    {
        // Find how many verts share this point, and add average any normals that are different.
        CommonVerts = 0;
        AvgNormal.Zero( );

        pTestNode = pCurrentList->mpHead;
        while( pTestNode )
        {
            CommonVerts++;

            AvgNormal.X += mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mNormal.X;
            AvgNormal.Y += mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mNormal.Y;
            AvgNormal.Z += mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mNormal.Z;

            pTestNode = pTestNode->pNext;
        }
        

        // No find the average and set that average for all of those normals.
        AvgNormal.X /= CommonVerts;
        AvgNormal.Y /= CommonVerts;
        AvgNormal.Z /= CommonVerts;

        AvgNormal.Normalize( );

        pTestNode = pCurrentList->mpHead;
        while( pTestNode )
        {
            mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mNormal.X = AvgNormal.X;
            mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mNormal.Y = AvgNormal.Y;
            mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mNormal.Z = AvgNormal.Z;
            pTestNode = pTestNode->pNext;
        }

        pCurrentList = pCurrentList->mpNextList;
    }
}



//=====================================================================================================================================
void QXBOXRipSkin::DetermineTheTotalSubMeshCount( void )
{
    s32               nTris;
    s32               MeshID, TriID;
    s32               CurrentMaterialID;
    XBOXRipSkinMesh*    pCurrentMesh;
    XBOXRipSkinTri*     pCurrentTris;

    mNSubMeshes = 0;

    if( g_Verbose )
    {
        x_sprintf( gOutputBuffer, "Determining Submesh Count.\n" );
        OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    }

    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh = &mpMesh[MeshID];
        pCurrentTris = pCurrentMesh->mpTris; 
        nTris        = pCurrentMesh->mNTris;

        // First thing to do is sort the triangles by texture ID.  That way we have a method of breaking up the submeshes
        // by the texture that their common tris share.
        x_qsort( pCurrentTris, nTris, sizeof( XBOXRipSkinTri ), TriSortCompare );

        // Now each mesh has it's tris sorted by materialID (texture).  Go through the tris for this mesh,
        // and determine how many specific textures are used.  This number will represent how many submeshes
        // there will be for this mesh.  Therefore a submesh is nothing more than a part of a mesh which shares
        // the same texture.
        pCurrentMesh->mNSubMeshes = 0;
        CurrentMaterialID = -1;

        for( TriID = 0; TriID < nTris; TriID++ )
        {
            if( CurrentMaterialID != pCurrentTris[TriID].mMaterialID )
            {
                // Determine if this is the first submesh for this mesh.
                if( pCurrentMesh->mNSubMeshes == 0 )
                    pCurrentMesh->mFirstSubMesh = mNSubMeshes;

                CurrentMaterialID = pCurrentTris[TriID].mMaterialID;
                pCurrentMesh->mNSubMeshes++;
                mNSubMeshes++;
            }
        }
    }

    // We now know how many submeshes are to be used in each mesh, so lets build them.
    mpSubMesh = (XBOXSkinSubmesh*)x_malloc( sizeof( XBOXSkinSubmesh ) * mNSubMeshes );
    ASSERT( mpSubMesh );

    // Output some info.
    if( g_Verbose )
    {   
        x_sprintf( gOutputBuffer, "\nSkin contains %d Meshes and %d Submeshes.\n",mNMeshes ,mNSubMeshes );
        OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    }
}


//=====================================================================================================================================
void QXBOXRipSkin::SeparateMeshesIntoSubmeshes( void )
{
    s32               LastUsedMaterialID;
    s32               VertID;
    s32               NIndices;
    s32               MaxIndice;

    s32               TriID;
    s32               TrisInSubmesh;
    s32               FirstTriInSubmesh;
    t_Triangle*       pTrisForStrip;
    XBOXRipSkinTri*     pCurrentTris;

    s32               MeshID;
    s32               SubMeshID;
    s32               FirstSubMeshID;
    s32               MaxSubMeshID;
    XBOXRipSkinMesh*    pCurrentMesh;
    XBOXSkinSubmesh*    pCurrentSubMesh;

    s32               StripID;
    s32               NStrips;
    t_TriangleStrip*  pStrips;
    xbool             ShadowMesh, EnvMappedMesh, AlphaMesh;

    // Output some info.
    if( g_Verbose )
    {   
        x_sprintf( gOutputBuffer, "\nSeparating meshes into submeshes\n" );
        OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
    }
    
    // Allocate a dummy array of the triangles in a format that the triangle stripper can use.
    pTrisForStrip = (t_Triangle*)x_malloc( sizeof( t_Triangle ) * SKIN_MAX_NUM_TRIS );
    ASSERT( pTrisForStrip );

    // Go through each mesh and determine how to assemble each sub-mesh.
    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        // Get the current mesh, and the native triangle list for this mesh.
        pCurrentMesh = &mpMesh[MeshID];
        pCurrentTris = pCurrentMesh->mpTris; 

        // Get some basic information about the mesh.
        EnvMappedMesh = pCurrentMesh->mFlags & SKIN_MESH_FLAG_ENVMAPPED;
        ShadowMesh    = pCurrentMesh->mFlags & SKIN_MESH_FLAG_SHADOW;
        AlphaMesh     = pCurrentMesh->mFlags & SKIN_MESH_FLAG_ALPHA;

        // Determine the indexes for the submeshes that are to be used by this mesh.
        FirstSubMeshID = pCurrentMesh->mFirstSubMesh;
        MaxSubMeshID   = FirstSubMeshID + pCurrentMesh->mNSubMeshes;

        // Allocate the working Indices array for the current mesh.
        pCurrentMesh->mpIndices = (s16*)x_malloc( sizeof( s16 ) * SKIN_MAX_NUM_INDICES );
        ASSERT( pCurrentMesh->mpIndices );

        // Reset the indices count.
        pCurrentMesh->mNIndices = 0;

        // Set the Material types for each submesh.
        //=====================================================================================================================================
        // Reset the current Material to an invalid material number.
        LastUsedMaterialID = -1;

        for( SubMeshID = FirstSubMeshID; SubMeshID < MaxSubMeshID; SubMeshID++ )
        {
            pCurrentSubMesh = &mpSubMesh[SubMeshID];

            // Get the next valid MaterialID.
            for( TriID = 0; TriID < pCurrentMesh->mNTris; TriID++ )
            {
                // If the current material ID is higher than the last one used, set the current submesh to this
                // material ID.
                if( pCurrentTris[TriID].mMaterialID > LastUsedMaterialID )
                {
                    pCurrentSubMesh->TextureID = pCurrentTris[TriID].mMaterialID;
                    LastUsedMaterialID = pCurrentTris[TriID].mMaterialID;
                    break;
                }
            }
        }

        // Now each submesh has it's material assigned to it, time to build the strips for each submesh.
        //=====================================================================================================================================
        for( SubMeshID = FirstSubMeshID; SubMeshID < MaxSubMeshID; SubMeshID++ )
        {
            // Get the current submesh pointer.
            pCurrentSubMesh = &mpSubMesh[SubMeshID];

            // Set the submesh type flags based on it's parent mesh.
            pCurrentSubMesh->Flags = EnvMappedMesh ? SKIN_SUBMESH_FLAG_ENVPASS : 0 |
                                      ShadowMesh    ? SKIN_SUBMESH_FLAG_SHADOW  : 0 |
                                      AlphaMesh     ? SKIN_SUBMESH_FLAG_ALPHA   : 0;

            // Determine how many triangles are in this submesh by counting the triangles in this 
            // mesh that share the same MaterialID as the submesh textureID.
            TrisInSubmesh = 0;
            for( TriID = 0; TriID < pCurrentMesh->mNTris; TriID++ )
            {
                if( pCurrentSubMesh->TextureID == pCurrentTris[TriID].mMaterialID )
                {
                    if( TrisInSubmesh == 0 )
                        FirstTriInSubmesh = TriID;

                    // Include the matching Triangle in the list of matches.
                    TrisInSubmesh++;
                }
            }

            // Now, copy all of the tris that are in this submesh over to a structure that is suited for the
            // triangle stripper.  
            for( TriID = 0; TriID < TrisInSubmesh; TriID++ )
            {
                // Just copy over the indices that make up the triangle, the material is not required.
                pTrisForStrip[TriID].Index[0] = pCurrentTris[TriID + FirstTriInSubmesh].mIndex[0];
                pTrisForStrip[TriID].Index[1] = pCurrentTris[TriID + FirstTriInSubmesh].mIndex[1];
                pTrisForStrip[TriID].Index[2] = pCurrentTris[TriID + FirstTriInSubmesh].mIndex[2];
            }
            
            // Build the strips needed to render this submesh.
            if( g_Verbose )
            {   
                x_sprintf( gOutputBuffer, "Building Mesh:%d, Submesh:%d\n", MeshID, SubMeshID );
                OUTPUT_TO_USER_AND_FILE( gOutputBuffer );
            }

            BuildStrips( pTrisForStrip, TrisInSubmesh, &pStrips, NStrips, SKIN_MAX_NUM_VERTS_IN_STRIP );

            // Prepare to parse the strip array and build one big ass strip.
            pCurrentSubMesh->mPrimType   = D3DPT_TRIANGLESTRIP;
            pCurrentSubMesh->mIndexStart = pCurrentMesh->mNIndices;
            pCurrentSubMesh->mVertexMin  = BIG_S32_VALUE;
            MaxIndice = 0;
            NIndices = 0;

            // Now take that strip and add it to the main meshes indices array.
            for( StripID = 0; StripID < NStrips; StripID++ )
            {
                // Add each vertex used in the strip to the meshes indices array.
                for( VertID = 0; VertID < pStrips[StripID].NVerts; VertID++ )
                {
                    // Copy the vert an increment the count.
                    pCurrentMesh->mpIndices[pCurrentMesh->mNIndices] = pStrips[StripID].pVerts[VertID];
                    pCurrentMesh->mNIndices++;
                    NIndices++;
                    ASSERT( pCurrentMesh->mNIndices < SKIN_MAX_NUM_INDICES );

                    // Update the minimum and maximum VERTEX that was used by this submesh.
                    pCurrentSubMesh->mVertexMin = MIN( pCurrentSubMesh->mVertexMin, pStrips[StripID].pVerts[VertID] );
                    MaxIndice                   = MAX( MaxIndice,                   pStrips[StripID].pVerts[VertID] );
                }

                // If this isn't the last strip, connect this strip to the next one with 2 garbage triangles.
                if( StripID < (NStrips - 1) )
                {
                    ASSERT((pCurrentMesh->mNIndices + 2) < SKIN_MAX_NUM_INDICES);

                    // Connect that last vertex of this strip to the previous vertex.
                    pCurrentMesh->mpIndices[pCurrentMesh->mNIndices] = pCurrentMesh->mpIndices[pCurrentMesh->mNIndices - 1];
                    pCurrentMesh->mNIndices++;

                    // Now connect this strip to the first vert of the next strip.
                    pCurrentMesh->mpIndices[pCurrentMesh->mNIndices] = pStrips[StripID + 1].pVerts[0];
                    pCurrentMesh->mNIndices++;

                    NIndices += 2;
                }
            }

            // Complete the submesh structure.
            pCurrentSubMesh->mNPrims      = NIndices - 2;
            pCurrentSubMesh->mVertexCount = 1 + (MaxIndice - pCurrentSubMesh->mVertexMin);

            // Free the strips that were just created.
            SAFE_RELEASE_MESH_DATA( pStrips );
        }
    }


    // Free all the memory left over.
    SAFE_RELEASE_MESH_DATA( pTrisForStrip );
}


//=====================================================================================================================================
void QXBOXRipSkin::CalcalateMeshRadius( void )
{
    s32             MeshID;
    s32             VertID;
    f32             CurrentLength;
    f32             MaxLength;
    XBOXRipSkinMesh*  pCurrentMesh;
    XBOXSkinD3DVert*  pCurrentVerts;

    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh = &mpMesh[MeshID];
        MaxLength    = 0.0f;

        pCurrentVerts = pCurrentMesh->mpD3DVerts;

        for( VertID = 0; VertID < pCurrentMesh->mNVerts; VertID++ )
        {
            CurrentLength = pCurrentVerts[VertID].mPos.Length();

            if( CurrentLength > MaxLength )
                MaxLength = CurrentLength;
        }

        pCurrentMesh->mRadius = MaxLength;
    }
}


//=====================================================================================================================================
void QXBOXRipSkin::BuildAndWriteSkinFileHeader( X_FILE* pSkinFile )
{
    XBOXSkinFileHeader  SkinFileHeader;

    x_strncpy( SkinFileHeader.mName, mSkinName, SKIN_NAME_LENGTH );
    SkinFileHeader.mFlags      = 0;
    SkinFileHeader.mNBones     = mNBones;
    SkinFileHeader.mNMeshes    = mNMeshes;
    SkinFileHeader.mNSubMeshes = mNSubMeshes;
    SkinFileHeader.mNTextures  = mNTextures;

    VERIFY( x_fwrite( &SkinFileHeader, sizeof( XBOXSkinFileHeader ), 1, pSkinFile ) == 1 );
}

//=====================================================================================================================================
void QXBOXRipSkin::WriteMeshData( X_FILE* pSkinFile )
{
    s32                 MeshID;
    XBOXRipSkinMesh*      pCurrentMesh;

    XBOXSkinMeshHeader    MeshFileHeader;

    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh = &mpMesh[MeshID];

        x_strncpy( MeshFileHeader.mName,  pCurrentMesh->mName, SKIN_MESH_NAME_LENGTH );
        MeshFileHeader.mFlags           = pCurrentMesh->mFlags;
        MeshFileHeader.mRadius          = pCurrentMesh->mRadius;
        MeshFileHeader.mFirstSubMesh    = pCurrentMesh->mFirstSubMesh;
        MeshFileHeader.mNSubMeshes      = pCurrentMesh->mNSubMeshes;
        MeshFileHeader.mVertexFormat    = pCurrentMesh->mVertexFormat;
        MeshFileHeader.mNVerts          = pCurrentMesh->mNVerts;
        MeshFileHeader.mVertexDataSize  = sizeof( XBOXSkinD3DVert ) * pCurrentMesh->mNVerts;
        MeshFileHeader.mNIndices        = pCurrentMesh->mNIndices;
        MeshFileHeader.mIndexDataSize   = sizeof( s16 ) * pCurrentMesh->mNIndices;

        VERIFY( x_fwrite( &MeshFileHeader,           sizeof( XBOXSkinMeshHeader ),     1, pSkinFile ) );
        
        if( MeshFileHeader.mVertexDataSize )
            VERIFY( x_fwrite( pCurrentMesh->mpD3DVerts,  MeshFileHeader.mVertexDataSize, 1, pSkinFile ) );

        if( MeshFileHeader.mIndexDataSize )
            VERIFY( x_fwrite( pCurrentMesh->mpIndices,   MeshFileHeader.mIndexDataSize,  1, pSkinFile ) );
    }
}

//=====================================================================================================================================
void QXBOXRipSkin::WriteSubmeshData( X_FILE* pSkinFile )
{
    VERIFY( x_fwrite( mpSubMesh, sizeof( XBOXSkinSubmesh ), mNSubMeshes, pSkinFile ) );
}


//=====================================================================================================================================
void QXBOXRipSkin::WriteTextureData( X_FILE* pSkinFile )
{
    VERIFY( x_fwrite( mpTexture, sizeof( XBOXSkinTexture ), this->mNTextures, pSkinFile ) );
}


//=====================================================================================================================================
void QXBOXRipSkin::WriteSkeletonData( X_FILE* pSkinFile )
{
    VERIFY( x_fwrite( mpBone, sizeof( XBOXSkinBone ), mNBones, pSkinFile ) );
}


//=====================================================================================================================================
s32 TriSortCompare( const void* Left, const void* Right )
{
    const XBOXRipSkinTri* LeftTri;
    const XBOXRipSkinTri* RightTri;

    // Get a local copy of each pointer that's the correct data type.
    LeftTri  = reinterpret_cast< const XBOXRipSkinTri* >( Left  );
    RightTri = reinterpret_cast< const XBOXRipSkinTri* >( Right );

    if( LeftTri->mMaterialID < RightTri->mMaterialID )
    {
        return -1;
    }
    else if( LeftTri->mMaterialID == RightTri->mMaterialID )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
////////////////////////////////////////////////////////////////////////////
//
// PC_Skin.cpp - implementation of the skin file for the PC
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "Q_PC.hpp"
#include "Q_Engine.hpp"
#include "Q_SMEM.hpp"
#include "Q_VRAM.hpp"
#include "Q_Draw.hpp"

#include "Skin.hpp"
#include "PC_SkinBuffer.hpp"
#include "AnimPlay.h" //###


////////////////////////////////////////////////////////////////////////////
// QSkin(PC) Vertex Shader info
////////////////////////////////////////////////////////////////////////////

#include "PC_VBShaderDefines.h"
#include "PC_VBShader.h"
#include "PC_VBShadowShader.h"

u32 s_SKIN_VertexFormat[] =
{
    D3DVSD_STREAM   (0),
    D3DVSD_REG      (0, D3DVSDT_FLOAT3),            // v.xyz = vPos
    D3DVSD_REG      (1, D3DVSDT_FLOAT3),            // v.xyz = matrix weights
    D3DVSD_REG      (2, D3DVSDT_FLOAT3),            // v.xyz = matrix indices
    D3DVSD_REG      (3, D3DVSDT_FLOAT3),            // v.xyz = vNormal
//  D3DVSD_REG      (4, D3DVSDT_SHORT4),            // v.xyzw = color
    D3DVSD_REG      (4, D3DVSDT_FLOAT2),            // v.xy  = uv
    D3DVSD_END      ()
};


////////////////////////////////////////////////////////////////////////////
// LOCAL STRUCTS
////////////////////////////////////////////////////////////////////////////

struct SSortSubMesh
{
    s16 SortKey;
    s16 Index;
};


////////////////////////////////////////////////////////////////////////////
// QSkin STATICS
////////////////////////////////////////////////////////////////////////////

s32 QSkin::s_VertexShaderID = -1;
s32 QSkin::s_ShadowShaderID = -1;


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

void QSkin::ActivateVertexShader( void )
{
    if( s_VertexShaderID == -1 )
        s_VertexShaderID = PC_RegisterVertexShader( s_SKIN_ObjShader, s_SKIN_VertexFormat, 0 );

    PC_ActivateVertexShader( s_VertexShaderID );
}

//==========================================================================

void QSkin::ActivateShadowShader( void )
{
    if( s_ShadowShaderID == -1 )
        s_ShadowShaderID = PC_RegisterVertexShader( s_SKIN_ShadowShader, s_SKIN_VertexFormat, 0 );

    PC_ActivateVertexShader( s_ShadowShaderID );
}

//==========================================================================

QSkin::QSkin( void )
{
    InitData();
}

//==========================================================================

QSkin::QSkin( char* SkinFile )
{
    InitData();
    SetupFromResource( SkinFile );
}

//==========================================================================

QSkin::~QSkin( void )
{
    KillData();
}

//==========================================================================

void QSkin::SetupFromResource( char* filename )
{
    X_FILE* fp = x_fopen( filename, "rb" );
    ASSERT(fp);

    SetupFromResource( fp );

    x_fclose(fp);
}

//==========================================================================

void QSkin::SetupFromResource( X_FILE* fp )
{
    LPDIRECT3DDEVICE8   pD3DDevice;
    PCSkinD3DVert*    pBufferData;
    PCSkinFileHeader  SkinFileHeader;
    PCSkinMeshHeader  MeshHeader;
    s32                 MeshID;
    s32                 MeshVertDataSize  = 0;
    s32                 MeshIndexDataSize = 0;

    // Make sure that there is a file to deal with at all.
    ASSERT( fp );

    // Get access to the main D3DDevice.
    pD3DDevice = PC_GetDevice();

    // Read the header out of the file.
    VERIFY( x_fread( &SkinFileHeader, sizeof( PCSkinFileHeader ), 1, fp ) == 1 );
    
    // Copy the header information into the skin
    x_strcpy( m_Name, SkinFileHeader.mName );
    m_Flags      = SkinFileHeader.mFlags;
    m_NBones     = SkinFileHeader.mNBones;
    m_NMeshes    = SkinFileHeader.mNMeshes;
    m_NSubMeshes = SkinFileHeader.mNSubMeshes;
    m_NTextures  = SkinFileHeader.mNTextures;

    // Allocate Resources
    ////////////////////////////////////////////////////////////////////////
    m_pMeshes       = new PCSkinMesh[m_NMeshes];
    m_pSubMeshes    = new PCSkinSubmesh[m_NSubMeshes];
    m_pBoneNames    = new char[m_NBones * SKIN_BONE_NAME_LENGTH];
    m_pTextureNames = new char[m_NTextures * X_MAX_PATH];
        
    //  Read Mesh Data.
    ////////////////////////////////////////////////////////////////////////
    for( MeshID = 0; MeshID < m_NMeshes; MeshID++ )
    {
        // Read Mesh Header
        VERIFY( x_fread(&MeshHeader, sizeof(PCSkinMeshHeader), 1, fp ) == 1 );
        x_strncpy( m_pMeshes[MeshID].Name, MeshHeader.mName, SKIN_MESH_NAME_LENGTH );
        m_pMeshes[MeshID].Flags          = MeshHeader.mFlags;
        m_pMeshes[MeshID].Radius         = MeshHeader.mRadius;
        m_pMeshes[MeshID].FirstSubMesh   = MeshHeader.mFirstSubMesh;
        m_pMeshes[MeshID].NSubMeshes     = MeshHeader.mNSubMeshes;
        
        m_pMeshes[MeshID].VertexFormat   = MeshHeader.mVertexFormat;
        m_pMeshes[MeshID].NVerts         = MeshHeader.mNVerts;
        MeshVertDataSize                 = MeshHeader.mVertexDataSize;
        m_pMeshes[MeshID].pVertexBuffer  = NULL;
        
        m_pMeshes[MeshID].NIndices       = MeshHeader.mNIndices;
        MeshIndexDataSize                = MeshHeader.mIndexDataSize;
        m_pMeshes[MeshID].pIndexBuffer   = NULL;
        
        m_pMeshes[MeshID].NMorphTargets  = 0;  // Not supported on PC yet.
        m_pMeshes[MeshID].pMorphTarget   = NULL;
        m_pMeshes[MeshID].NMorphDeltas   = 0;  // Not supported on PC yet.
        m_pMeshes[MeshID].pMorphDelta    = NULL;

        // If there is data to get, then build the vertex and index buffers.
        if( MeshVertDataSize && MeshIndexDataSize )
        {
            // Create the D3D Vertex buffer
            DXWARN( pD3DDevice->CreateVertexBuffer( MeshVertDataSize,
                                                    D3DUSAGE_WRITEONLY,
                                                    m_pMeshes[MeshID].VertexFormat,
                                                    D3DPOOL_DEFAULT,
                                                    &m_pMeshes[MeshID].pVertexBuffer ));

            // Lock the entire vertex buffer and fill it directly from the Skin file.
            DXWARN( m_pMeshes[MeshID].pVertexBuffer->Lock( 0, 0, (byte**)&pBufferData, 0 ));

            VERIFY( x_fread( pBufferData, MeshVertDataSize, 1, fp ) == 1 );

            s32 CheckVert;
            for( CheckVert = 0; CheckVert < m_pMeshes[MeshID].NVerts; CheckVert++ )
            {
                if( pBufferData[CheckVert].mBlendWeights[2] && (pBufferData[CheckVert].mBlendWeights[0] + 
                    pBufferData[CheckVert].mBlendWeights[1] + 
                    pBufferData[CheckVert].mBlendWeights[2] > 1.0f) )
                {  
                    pBufferData[CheckVert].mBlendWeights[2] -= (pBufferData[CheckVert].mBlendWeights[0] + 
                                                                pBufferData[CheckVert].mBlendWeights[1] + 
                                                                pBufferData[CheckVert].mBlendWeights[2]) - 1.0f;
                }
            }


            DXWARN( m_pMeshes[MeshID].pVertexBuffer->Unlock());
         
            // Create the index buffer.
            DXWARN( pD3DDevice->CreateIndexBuffer( MeshIndexDataSize,
                                                   D3DUSAGE_WRITEONLY,
                                                   D3DFMT_INDEX16,
                                                   D3DPOOL_DEFAULT,
                                                   &m_pMeshes[MeshID].pIndexBuffer ));

            // Lock the buffer and grab the data directly from the Skin File
            DXWARN( m_pMeshes[MeshID].pIndexBuffer->Lock( 0, MeshIndexDataSize, (byte**)&pBufferData, 0 ));

            VERIFY( x_fread( pBufferData, MeshIndexDataSize, 1, fp ) == 1 );

            DXWARN( m_pMeshes[MeshID].pIndexBuffer->Unlock() );
        }
        else
        {
            ASSERT( 0 );
        }
    }

    // Time to read in the rest of the file.
    x_fread(m_pSubMeshes,    sizeof(PCSkinSubmesh), m_NSubMeshes, fp );
    x_fread(m_pTextureNames, sizeof(PCSkinTexture), m_NTextures, fp );
    x_fread(m_pBoneNames,    sizeof(PCSkinBone),    m_NBones, fp );

    //### TEMP: This flag should be set in RipSkin, there's a possibility that
    //          it shouldn't be set on every submesh.
    for( s32 i = 0; i < m_NSubMeshes; i++ )
    {
        m_pSubMeshes[i].Flags |= SKIN_SUBMESH_FLAG_VISIBLE;
    }
}


//=====================================================================================================================================
void QSkin::SetupFromResource( byte* pByteStream )
{
    // NOT IMPLEMENTED AS OF YET UNTIL FILE VERSIONS ARE FINE.
    ASSERT( FALSE );
}


//=====================================================================================================================================
void QSkin::RenderSubMesh( u32 SkinData )
{
//  SkinData = 0;
}


//=====================================================================================================================================
void QSkin::OpenSkinBuffer( s32 NumberOfSkins, s32 MaxSubMeshCountPerSkin )
{
    s32 MatrixDataSize;
    s32 SubmeshDataSize;

    // Set the limits to this skin buffer.
    PCSkinBuffer::mMaxNumberOfSkins     = NumberOfSkins;
    PCSkinBuffer::mMaxNumberOfSubmeshes = NumberOfSkins * MaxSubMeshCountPerSkin;

    // Determine how much scratch memory is going to be required, then take it.
    MatrixDataSize        = sizeof( SPCMatrixGroup ) * NumberOfSkins;
    SubmeshDataSize       = sizeof( PCSubMeshGroup ) * PCSkinBuffer::mMaxNumberOfSubmeshes;
    PCSkinBuffer::mScratchDataSize = MatrixDataSize + SubmeshDataSize + 32;// add 32 for alignment purposes 

    PCSkinBuffer::mpSMEMData = SMEM_BufferAlloc( PCSkinBuffer::mScratchDataSize );
    ASSERT( PCSkinBuffer::mpSMEMData );

    // Allign the Scratch Memory pointer to a 32 byte value.
    PCSkinBuffer::mpSMEMData = (byte*)ALIGN_32( (u32)PCSkinBuffer::mpSMEMData );

    // Set the matrix pointer to the start of the Scratch memory space.
    PCSkinBuffer::mpMatrices = (SPCMatrixGroup*)PCSkinBuffer::mpSMEMData;

    // Set the SubmeshGroup pointer to wherever the end of the matricies are going to be.
    PCSkinBuffer::mpSubMeshGroup = (PCSubMeshGroup*)(PCSkinBuffer::mpSMEMData + MatrixDataSize);

    // Initialize the skin and submesh counts.
    PCSkinBuffer::mNSkinsInBuffer     = 0;
    PCSkinBuffer::mNSubmeshesInBuffer = 0;
}


//=====================================================================================================================================
void QSkin::RenderSkin( radian3* pBoneRots,
                        vector3& WorldPos,
                        radian   WorldYaw,
                        void*    pAnimGroupPtr,
                        xbool    Mirrored,
                        f32      WorldScale,
                        vector3* pBoneScales,
                        u32      UserData1,
                        u32      UserData2,
                        u32      UserData3)

{
    // Confirm that the game is ready to render the skins.
    ASSERT( ENG_GetRenderMode() );
    ASSERT( PCSkinBuffer::IsValid( ) );

    t_MatrixGroup* pMatrices;   // The next available spot in scratch memory to store matrices for a skin.

    pMatrices = PCSkinBuffer::GetMatrixGroup( );
    ASSERT( pMatrices );

    // Build the matrices for this Skin
    BuildMatrices( pMatrices, // Location to store the Final matrices data for this skin.
                   pBoneRots,               // Rotatations for the bones from the animation frame.
                   WorldPos,                // World position of the object using this skin.
                   WorldYaw,                // World Y axis rotation of this skinned object.
                   Mirrored,                // Identify that this model is mirrored.
                   WorldScale,              // Value to use as a general scale of the entire model.
                   pBoneScales,             // Scaling values for particular bones.
                   pAnimGroupPtr );         // Animation information.

    RenderSkin( pMatrices, UserData1, UserData2, UserData3, 0, NULL, NULL );
}

//==========================================================================

void QSkin::RenderSkin( t_MatrixGroup* pMatrices,
                        u32            UserData1,
                        u32            UserData2,
                        u32            UserData3,
                        s32            NShadowMtx,
                        matrix4*       pShadowMtx,
                        u8*            AlphaLevels )
{
    s32              MeshID;
    s32              SubMeshID;
    PCSubMeshGroup SubMeshGroup;

    ASSERT( pMatrices != NULL );
    ASSERT( (NShadowMtx <= 0) || ((NShadowMtx > 0) && (pShadowMtx != NULL)) );

    //---   figure out the pointers to the data stuff

    // Look through all of the meshes in this skin and add the submeshes to the buffer.
    for( MeshID = 0; MeshID < m_NMeshes; MeshID++ )
    {
        // If the mesh isn't visible, just go to the next one.
        if( (m_pMeshes[MeshID].Flags & SKIN_MESH_FLAG_VISIBLE) == 0 )
            continue;

//        if( (m_pMeshes[MeshID].Flags & SKIN_MESH_FLAG_SHADOW ) )
//            continue;

        // Add each submesh for this mesh to the Skins Scratch Buffer.
        for( SubMeshID = m_pMeshes[MeshID].FirstSubMesh; SubMeshID < (m_pMeshes[MeshID].FirstSubMesh + m_pMeshes[MeshID].NSubMeshes); SubMeshID++ )
        {
            //--- check if submesh is marked as visible
            if( (m_pSubMeshes[SubMeshID].Flags & SKIN_SUBMESH_FLAG_VISIBLE) == 0 )
                continue;

            SubMeshGroup.Flags            = m_pSubMeshes[SubMeshID].Flags;
            SubMeshGroup.MeshID           = MeshID;
            SubMeshGroup.pMesh            = &m_pMeshes[MeshID];
            SubMeshGroup.pSubmesh         = &m_pSubMeshes[SubMeshID];
            SubMeshGroup.NBones           = m_NBones;
            SubMeshGroup.pMatrices        = pMatrices;
            SubMeshGroup.UserData1        = UserData1;
            SubMeshGroup.UserData2        = UserData2;
            SubMeshGroup.UserData3        = UserData3;
            SubMeshGroup.mpShadowMatrices = pShadowMtx;
            SubMeshGroup.NumShadows       = NShadowMtx;
            SubMeshGroup.pAlphaLevels     = AlphaLevels;


            // if this is a shadow mesh, make sure that the bitmap is NULL
            if( SubMeshGroup.Flags & SKIN_SUBMESH_FLAG_SHADOW )
            {
                SubMeshGroup.pBitmap = NULL;
            }
            else
            {
                ASSERT( m_pTextureArray );
                ASSERT( m_pTextureArray[m_pSubMeshes[SubMeshID].TextureID] );
                SubMeshGroup.pBitmap = m_pTextureArray[m_pSubMeshes[SubMeshID].TextureID];
            }

            if( SubMeshGroup.Flags & SKIN_SUBMESH_FLAG_ENVPASS )
            {
                SubMeshGroup.pBitmap2 = m_pTextureArray[ m_NTextures-1 ];
                ASSERT( SubMeshGroup.pBitmap2 != NULL );
            }
            else
            {
                SubMeshGroup.pBitmap2 = NULL;
            }

            PCSkinBuffer::AddSubmeshGroup( &SubMeshGroup );
        }
    }

    // Mark the addition of the skin to the buffer.
    PCSkinBuffer::UpdateSkinCount( );
}


//=====================================================================================================================================
static void RadixSort( s32 NSubMeshes, SSortSubMesh* pSubMeshes )
{
    //--- NEED: 1) Sort key for every submesh
    //          2) Values to sort (either ptrs or indices) for each submesh

    #define NUM_BUCKETS     16
    #define BUCKET_AND      0x0F
    #define BITS_PER_PASS   4
    #define NUM_PASSES      4

    s32*            pBucketCount;
    SSortSubMesh*   pBuckets;
    s32             Pass;
    s32             BucketID;
    s32             Shifter;
    s32             SubMeshID;
    s32             Count;
    s32             BucketOffset;

    //--- allocate temporary space for the buckets and bucket counts
    SMEM_StackPushMarker();
    pBuckets     = (SSortSubMesh*)SMEM_StackAlloc( sizeof(SSortSubMesh) * NUM_BUCKETS * NSubMeshes );
    pBucketCount = (s32*)         SMEM_StackAlloc( sizeof(s32) * NUM_BUCKETS );
    ASSERT( (pBuckets != NULL) && (pBucketCount != NULL) );

    //--- do each pass
    Shifter = 0;
    for( Pass = 0; Pass < NUM_PASSES; ++Pass )
    {
        //--- empty the buckets
        for( BucketID = 0; BucketID < NUM_BUCKETS; ++BucketID )
            pBucketCount[BucketID] = 0;

        //--- put the submeshes in their buckets
        for( SubMeshID = 0; SubMeshID < NSubMeshes; ++SubMeshID )
        {
            BucketID     = (pSubMeshes[SubMeshID].SortKey >> Shifter) & BUCKET_AND;
            BucketOffset = BucketID * NSubMeshes;

            pBuckets[ BucketOffset + pBucketCount[BucketID] ] = pSubMeshes[SubMeshID];
            pBucketCount[BucketID]++;

            ASSERT( pBucketCount[BucketID] <= NSubMeshes );
        }

        //--- remerge the list from the buckets
        Count = 0;
        for( BucketID = 0; BucketID < NUM_BUCKETS; ++BucketID )
        {
            BucketOffset = BucketID*NSubMeshes;
            for( SubMeshID = 0; SubMeshID < pBucketCount[BucketID]; ++SubMeshID )
            {
                pSubMeshes[Count] = pBuckets[BucketOffset + SubMeshID];
                Count++;
            }
        }
        ASSERT( Count == NSubMeshes );

        //--- change shifter to inspect next bits
        Shifter += BITS_PER_PASS;
    }

    //--- free temporary space
    SMEM_StackPopToMarker();
}

//=====================================================================================================================================
void QSkin::FlushSkinBuffer( void )
{
    // Confirm that the game is ready to render the skins.
    ASSERT( ENG_GetRenderMode() );
    ASSERT( PCSkinBuffer::IsValid( ) );

    s32                 SubMeshID, BoneID;
    PCSkinMesh*       pActiveMesh     = NULL;
    matrix4*            pActiveMatrices = NULL;
    x_bitmap*           pActiveTexture  = (x_bitmap*)0xffffffff; // Don't use NULL to initialize this 
                                                                 // variable because a NULL texture 
    SSortSubMesh*       pSubMeshOrder;

    s8                  CurrentShadowAlpha = -1;
    s8                  CurrentShadowOrder = -1;
    s8                  BaseShadowAlpha = -1;
    s32                 TopBasePointer = -1;

    PCSubMeshGroup*   pCurrentSubMeshGroup;

    LPDIRECT3DDEVICE8   pD3DDevice;

    // If there are no submeshes in the buffer, just leave now.
    if( PCSkinBuffer::mNSubmeshesInBuffer == 0 )
        return;

    // Prepare D3D for the rendering.
    // Get access to the main D3DDevice.
    pD3DDevice = PC_GetDevice();

    // Set the render states that are required for the skin to be rendered using index buffers with vertex blending.

    // Set vertex blending to use 3 weights
    DXWARN( pD3DDevice->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_2WEIGHTS ));

    //--- create a temp buffer to store the submesh order
    SMEM_StackPushMarker();
    pSubMeshOrder = (SSortSubMesh*)SMEM_StackAlloc( sizeof(SSortSubMesh) * PCSkinBuffer::mNSubmeshesInBuffer );
    ASSERT( pSubMeshOrder != NULL );

    //--- init sort structures
    for( SubMeshID = 0; SubMeshID < PCSkinBuffer::mNSubmeshesInBuffer; ++SubMeshID )
    {
        pCurrentSubMeshGroup = &PCSkinBuffer::mpSubMeshGroup[SubMeshID];

        if( pCurrentSubMeshGroup->pBitmap != NULL )
        {
            ASSERT( pCurrentSubMeshGroup->pBitmap->GetVRAMID() < 2048 );
            pSubMeshOrder[SubMeshID].SortKey = pCurrentSubMeshGroup->pBitmap->GetVRAMID();
        }
        else
        {
            pSubMeshOrder[SubMeshID].SortKey  = (1 << 11);
        }

        //if( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_ )
        //    pSubMeshOrder[i].SortKey |= (1 << 12);

        if( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_ALPHA )
            pSubMeshOrder[SubMeshID].SortKey |= (1 << 13);

        if( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_MORPH )
            pSubMeshOrder[SubMeshID].SortKey |= (1 << 14);

        if( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_SHADOW )
            pSubMeshOrder[SubMeshID].SortKey |= (1 << 15);

        pSubMeshOrder[SubMeshID].Index = SubMeshID;
    }

    //--- sort the submeshes by texture and render flags
    RadixSort( PCSkinBuffer::mNSubmeshesInBuffer, pSubMeshOrder );


    // Set up the default texture stages.
    DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
    DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));

    DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
    DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));

    DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE ));

    DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ));


    // setup the base constant register
    vector4 vConst;
    vConst.Set(3.0f, 0.5f, -0.5f, 1.0f);
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_1, &vConst.X, 1) );

    vConst.Set(1.0f, 1.0f, 1.0f, 1.0f);
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_MAX, &vConst.X, 1) );

    vConst.Set(0.0f, 0.0f, 0.0f, 0.0f);
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_MIN, &vConst.X, 1) );

    D3DXMATRIX matView, matProj, matViewProj, matViewProjTranspose, matShadowProj;
    pD3DDevice->GetTransform( D3DTS_VIEW,       &matView );
    pD3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
    D3DXMatrixMultiply( &matViewProj, &matView, &matProj );
    D3DXMatrixTranspose( &matViewProjTranspose, &matViewProj );


    // Go through each sub mesh in the skin that has been sorted and draw it.
    for( SubMeshID = 0; SubMeshID < PCSkinBuffer::mNSubmeshesInBuffer; SubMeshID++ )
    {
        pCurrentSubMeshGroup = &PCSkinBuffer::mpSubMeshGroup[ pSubMeshOrder[SubMeshID].Index ];

        // If there isn't a pre callback function, do our best to set up the system to render the skin properly.
        if( QSkin::s_PreRenderCallback == NULL )
        {
            //---   set the render flags based on the sub-mesh flags
            if( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_SHADOW )
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
                ENG_SetBlendMode( ENG_BLEND_NORMAL | ENG_BLEND_FIXED_ALPHA, 32 );
            }
            else if ( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_ENVPASS )
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
                ENG_SetBlendMode( ENG_BLEND_FIXED_ALPHA | ENG_BLEND_ADDITIVE, 50 );
            }
            else if ( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_ALPHA )
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
                ENG_SetBlendMode( ENG_BLEND_NORMAL );
            }
            else
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF );
            }
        }
        else
        {
            // Otherwise call the pre call back function.
            if( QSkin::s_PreRenderCallback(  pCurrentSubMeshGroup->MeshID,
                                             (u32)pCurrentSubMeshGroup->pSubmesh,
                                             pCurrentSubMeshGroup->UserData1,
                                             pCurrentSubMeshGroup->UserData2,
                                             pCurrentSubMeshGroup->UserData3 ) == FALSE )
            {
                continue;
            }
        }

        if( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_ENVPASS && (pCurrentSubMeshGroup->pBitmap2) )
        {
            VRAM_PC_Activate( *pCurrentSubMeshGroup->pBitmap2, 1 );
            VRAM_PC_SetWrapMode( WRAP_MODE_TILE, WRAP_MODE_TILE, 1 );

            DXWARN(pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR , D3DCOLOR_ARGB( 50, 50, 50, 50 ) ));

            // setup the second stage texture blending mode
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG0, D3DTA_CURRENT ));
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TFACTOR ));
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE ));

            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD ));
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1  ));
        }
        else
        {
            // turn off the second texture stage
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ));
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ));
        }

        // Activate new textures as needed.
        if( (pCurrentSubMeshGroup->pBitmap != pActiveTexture) && (pCurrentSubMeshGroup->pBitmap) )
        {
            pActiveTexture = pCurrentSubMeshGroup->pBitmap;
            VRAM_Activate( *pActiveTexture );
        }

        //---------------------------------------
        //#### OPTIMIZE THIS!!!!!  This should be done on the build matrix step before rendering,
        //                         it will save a lot of operations that way!
        D3DXMATRIX TransMatrix;
        // If this  Load in a different set of matrices if necessary
        if( pCurrentSubMeshGroup->pMatrices->BoneMatrices != pActiveMatrices )
        {
            pActiveMatrices = pCurrentSubMeshGroup->pMatrices->BoneMatrices;

            // Copy the matrix used by each bone into the matrix array of D3D.
            for( BoneID = 0; BoneID < pCurrentSubMeshGroup->NBones; BoneID++ )
            {
                D3DXMatrixTranspose( &TransMatrix, (D3DXMATRIX *)&pActiveMatrices[BoneID].M );
                DXWARN( pD3DDevice->SetVertexShaderConstant( (3*BoneID), &TransMatrix, 3 ));
            }
        }
        //---------------------------------------

        if( pCurrentSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_SHADOW )
        {
            s32 ShadowPass;
            for( ShadowPass = 0; ShadowPass < pCurrentSubMeshGroup->NumShadows; ShadowPass++ )
            {
                // turn on the stencil buffer
//                PC_SetStencilMode( ENG_STENCIL_ON, pCurrentSubMeshGroup->pAlphaLevels[ShadowPass] );
                // set the ZBias so each overlap shadow will not fight
//                PC_SetShadowBiasMode( ENG_SHADOW_BIAS_ON, 16 - ShadowPass );
                // set the current shadow matrix
                pCurrentSubMeshGroup->pShadowMtx = &pCurrentSubMeshGroup->mpShadowMatrices[ShadowPass];

                // fill the projection matrix in constant registers
                D3DXMatrixMultiply( &matShadowProj, (D3DXMATRIX *)pCurrentSubMeshGroup->pShadowMtx, &matViewProj );
                D3DXMatrixTranspose( &matShadowProj, &matShadowProj );
                DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_PROJECT_1, &matShadowProj, 4) );

                // If the mesh is different, make sure to reset the vertex stream and the primitive index stream.
                if( pCurrentSubMeshGroup->pMesh != pActiveMesh )
                {
                    pActiveMesh = pCurrentSubMeshGroup->pMesh;

                    // Set the vertex buffer stream, then the index buffer, and finally identify the vertex shader.
                    ActivateShadowShader();
                    DXWARN( pD3DDevice->SetStreamSource( 0, pActiveMesh->pVertexBuffer, sizeof(PCSkinD3DVert) ));
                    DXWARN( pD3DDevice->SetIndices( pActiveMesh->pIndexBuffer, 0 ));
                }


                DXWARN( pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP,
                                                          pCurrentSubMeshGroup->pSubmesh->mVertexMin,
                                                          pCurrentSubMeshGroup->pSubmesh->mVertexCount,
                                                          pCurrentSubMeshGroup->pSubmesh->mIndexStart,
                                                          pCurrentSubMeshGroup->pSubmesh->mNPrims ));

//                PC_SetStencilMode( ENG_STENCIL_OFF );
//                PC_SetShadowBiasMode( ENG_SHADOW_BIAS_OFF );
            }
        }
        else
        {
            DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_LIGHTDIR_1, &pCurrentSubMeshGroup->pMatrices->LightDirMatrix, 4) );
            DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_LIGHTCOL_1, &pCurrentSubMeshGroup->pMatrices->LightColMatrix, 4) );

            // fill the projection matrix in constant registers
            DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_PROJECT_1, &matViewProjTranspose, 4) );

            // If the mesh is different, make sure to reset the vertex stream and the primitive index stream.
            if( pCurrentSubMeshGroup->pMesh != pActiveMesh )
            {
                pActiveMesh = pCurrentSubMeshGroup->pMesh;

                // Set the vertex buffer stream, then the index buffer, and finally identify the vertex shader.
                ActivateVertexShader();
                DXWARN( pD3DDevice->SetStreamSource( 0, pActiveMesh->pVertexBuffer, sizeof(PCSkinD3DVert) ));
                DXWARN( pD3DDevice->SetIndices( pActiveMesh->pIndexBuffer, 0 ));
            }

            DXWARN( pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP,
                                                      pCurrentSubMeshGroup->pSubmesh->mVertexMin,
                                                      pCurrentSubMeshGroup->pSubmesh->mVertexCount,
                                                      pCurrentSubMeshGroup->pSubmesh->mIndexStart,
                                                      pCurrentSubMeshGroup->pSubmesh->mNPrims )); 

        }

//        ENG_SetBlendMode( ENG_BLEND_NORMAL | ENG_BLEND_FIXED_ALPHA, 128 );
//        VRAM_PC_Deactivate( 1 );
    }

    //--- free any allocated SMEM
    SMEM_StackPopToMarker();

    // turn off the second texture stage
    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ));
    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ));
    VRAM_PC_Deactivate( 1 );

    // Disable the D3D states that were set previously.
    pD3DDevice->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE );

    // Kill the skin buffer it's now obsolete.
    PCSkinBuffer::Kill( );

    // Let the user know we are finished
    if( QSkin::s_FinishedCallback )
    {
        QSkin::s_FinishedCallback();
    }
}


//=====================================================================================================================================
void QSkin::InitData( void )
{
    m_NBones        = 0;
    m_Flags         = 0;
    m_NMeshes       = 0;
    m_NSubMeshes    = 0;
    m_NTextures     = 0;
    m_NMorphTargets = 0;
    m_pBoneNames    = NULL;
    m_pMeshes       = NULL;
    m_pSubMeshes    = NULL;
    m_pTextureNames = NULL;
    m_pTextureArray = NULL;
    m_pMorphTargets = NULL;
}


//=====================================================================================================================================
void QSkin::KillData( void )
{
    s32 i;
    PCSkinMesh* pCurrentMesh;

    //Destroy the meshes vertex buffer and index buffer assigned to this mesh.
    for( i = 0; i < m_NMeshes; i++ )
    {
        pCurrentMesh = &m_pMeshes[i];
        ASSERT( pCurrentMesh );

        if( pCurrentMesh )
        {
            SAFE_RELEASE(pCurrentMesh->pVertexBuffer);
            SAFE_RELEASE(pCurrentMesh->pIndexBuffer);
        }
    }
    
    SAFE_DELETE_ARRAY(m_pMeshes);
    SAFE_DELETE_ARRAY(m_pSubMeshes);
    SAFE_DELETE_ARRAY(m_pBoneNames);
    SAFE_DELETE_ARRAY(m_pTextureNames);

    m_NBones        = 0;
    m_Flags         = 0;
    m_NMeshes       = 0;
    m_NSubMeshes    = 0;
    m_NTextures     = 0;
    m_pTextureArray = NULL;
}


//-------------------------------------------------------------------------------------------------------------------------------------
inline void BuildRotationXZY( matrix4& RotMatrix, radian3 &Rotation )
{
    //=====================================================================================================================================
    //  This function takes Pitch, Roll and Yaw rotations and 
    //  applies those rotations into a matrix in that order, ( Rotations are around X, then Z, then Y axis )
    //
    //  In matrix form, this would be A' = RyRzRx * A, where:
    //
    //  Rx = | 1      0      0      0 |
    //       | 0      CosX   -SinX  0 |
    //       | 0      SinZ   CosX   0 |
    //       | 0      0      0      1 |
    //
    //  Rz = | CosZ   -SinZ  0      0 |
    //       | SinZ   CosZ   0      0 |
    //       | 0      0      1      0 |
    //       | 0      0      0      1 |
    //
    //  Ry = | CosY   0      SinY   0 |
    //       | 0      1      0      0 |
    //       | -SinY  0      CosY   0 |
    //       | 0      0      0      1 |
    //
    //
    //  So...the Rotatation XZY rotation transformation matrix (RyRzRx) is:
    //
    //  R = |  CosYCosZ     -CosXCosYSinZ + SinXSinY    CosYSinXSinZ + CosXSinY     0 |
    //      |  SinZ         CosXCosZ                    -CosZSinX                   0 |
    //      |  -CosZSinY    CosXSinYSinZ + CosYSinX     -SinXSinYSinZ + CosXCosY    0 |
    //      |  0            0                           0                           1 |
    //
    //=====================================================================================================================================
    register f32 SinX, SinY, SinZ;
    register f32 CosX, CosY, CosZ;
    register f32 CosXCosY,SinXCosY,SinYSinZ;

    x_sincos( Rotation.Pitch, SinX, CosX );
    x_sincos( Rotation.Yaw,   SinY, CosY );
    x_sincos( Rotation.Roll,  SinZ, CosZ );

    CosXCosY = CosX * CosY;
    SinXCosY = SinX * CosY;
    SinYSinZ = SinY * SinZ;

    RotMatrix.Identity( );
    RotMatrix.M[0][0] = ( CosY * CosZ );
    RotMatrix.M[1][0] = (-CosXCosY * SinZ + SinX * SinY );
    RotMatrix.M[2][0] = ( SinXCosY * SinZ + CosX * SinY );
    RotMatrix.M[0][1] = ( SinZ );
    RotMatrix.M[1][1] = ( CosX * CosZ );
    RotMatrix.M[2][1] = (-CosZ * SinX );
    RotMatrix.M[0][2] = (-CosZ * SinY );
    RotMatrix.M[1][2] = ( CosX * SinYSinZ + SinXCosY );
    RotMatrix.M[2][2] = (-SinX * SinYSinZ + CosXCosY );
}

//=====================================================================================================================================
// The core matrix is basically the rotations that are required for this bone, and a translation matrix
// that attaches this bone to it's parent.
inline void BuildCoreBoneMatrix(matrix4* pBoneMatrix, radian3* pBoneRotations, vector3* pParentToBone, matrix4* pParentMatrix, matrix4* pWorldOrientation )
{
    vector3  vTransformedPlayerToBone;  // ParentToBone vector Transformed by the bones parent matrix.

    // Building the CORE matrix. (Local Matrix)
    ////////////////////////////////////////////////////////////////
    //  The bone rotations are in XZY order. That is, you are
    //  expected to pitch, roll, and yaw, in that order.
    //
    //  In matrix form, this would be A' = RyRzRx * A, where:
    //
    //  Rx = | 1      0      0      0 |
    //       | 0      CosX   -SinX  0 |
    //       | 0      SinZ   CosX   0 |
    //       | 0      0      0      1 |
    //
    //  Rz = | CosZ   -SinZ  0      0 |
    //       | SinZ   CosZ   0      0 |
    //       | 0      0      1      0 |
    //       | 0      0      0      1 |
    //
    //  Ry = | CosY   0      SinY   0 |
    //       | 0      1      0      0 |
    //       | -SinY  0      CosY   0 |
    //       | 0      0      0      1 |
    //
    //
    //  We then add the translation. The translation is equal to
    //  the bone's local translation run through the parent's
    //  transformation matrix.  OR ...
    //
    //  P' = Tp * P,    where Tp is the parent's transform matrix
    //
    //  Now, the transformation looks like:
    //      A' = TRyRzRx * A
    //
    //  So...the transformation matrix (TRyRzRx) is:
    //
    //  T = |  CosYCosZ     -CosXCosYSinZ + SinXSinY    CosYSinXSinZ + CosXSinY     Tx |
    //      |  SinZ         CosXCosZ                    -CosZSinX                   Ty |
    //      |  -CosZSinY    CosXSinYSinZ + CosYSinX     -SinXSinYSinZ + CosXCosY    Tz |
    //      |  0            0                           0                           1  |
    //
    ////////////////////////////////////////////////////////////////
    
    // Start the bone matrix off by applying the rotations of the bone that were provided by the animation.
    BuildRotationXZY( *pBoneMatrix, *pBoneRotations );

    // Now add the additional rotations to the bone matrix to orient the bone properly into the world.
    pBoneMatrix->Transform( *pWorldOrientation );
    
    // Transform the BoneToParent vector by the parent matrix.
    vTransformedPlayerToBone.X = (pParentMatrix->M[0][0] * pParentToBone->X) + (pParentMatrix->M[1][0] * pParentToBone->Y) +
                                 (pParentMatrix->M[2][0] * pParentToBone->Z) +  pParentMatrix->M[3][0];

    vTransformedPlayerToBone.Y = (pParentMatrix->M[0][1] * pParentToBone->X) + (pParentMatrix->M[1][1] * pParentToBone->Y) +
                                 (pParentMatrix->M[2][1] * pParentToBone->Z) +  pParentMatrix->M[3][1];

    vTransformedPlayerToBone.Z = (pParentMatrix->M[0][2] * pParentToBone->X) + (pParentMatrix->M[1][2] * pParentToBone->Y) +
                                 (pParentMatrix->M[2][2] * pParentToBone->Z) +  pParentMatrix->M[3][2];

    // Set those translation values into the core bone matrix.
    pBoneMatrix->M[3][0] = vTransformedPlayerToBone.X;
    pBoneMatrix->M[3][1] = vTransformedPlayerToBone.Y;
    pBoneMatrix->M[3][2] = vTransformedPlayerToBone.Z;

    // Fill out edges
    pBoneMatrix->M[0][3] = pBoneMatrix->M[1][3] = pBoneMatrix->M[2][3] = 0;
    pBoneMatrix->M[3][3] = 1;
}



//=====================================================================================================================================
void QSkin::BuildMatrices( t_MatrixGroup*   pMatrixGroup,
                           radian3*         pBoneRots,
                           vector3&         WorldPos,
                           radian           WorldYaw,
                           xbool            bMirrored,
                           f32              WorldScale,
                           vector3*         pBoneScales,
                           void*            pAnimGroupPtr )
{
    // Starting rotations of the model.
    matrix4 WorldOrientation;
    WorldOrientation.Identity( );
    WorldOrientation.RotateY( WorldYaw );

    BuildMatrices( pMatrixGroup,
                   pBoneRots,
                   WorldPos,
                   WorldOrientation,
                   bMirrored,
                   WorldScale,
                   pBoneScales,
                   pAnimGroupPtr );
}


//=====================================================================================================================================
void QSkin::BuildMatrices( t_MatrixGroup* pMatrixGroup,
                           radian3*       pBoneRots,
                           vector3&       WorldPos,
                           matrix4&       WorldOrientation,
                           xbool          bMirrored,
                           f32            WorldScale,
                           vector3*       pBoneScales,
                           void*          pAnimGroupPtr )
{
    s32         BoneID;             // Current bone that is being processed.
    s32         MaxBones;           // Max bones in the skin.
    matrix4*    pParentMatrix;      // Pointer to the parent matrix.
    matrix4*    pCurrentMatrix;
    matrix4     WorldTranslation;   // Starting point of the model.
    radian3     Rot;                // XYZ rotation values.
    vector3     Trans;              // POSition translation value.              
    anim_group* pAnimGroup;         // Animation applied to this bone.
    vector3*    pCurrentBoneScale;
    vector3     OriginToBone;

    matrix4* pMatrices;
    pMatrices = &pMatrixGroup->BoneMatrices[0];

    // Make sure it's all good.
    ASSERT( pMatrices );
    ASSERT( pBoneRots );
    ASSERT( pAnimGroupPtr );

    BuildLightingMatrix( &pMatrixGroup->LightDirMatrix, &pMatrixGroup->LightColMatrix );

    // Get a pointer to the animation data, make sure that the animation has the same bone count as this skin.
    pAnimGroup = (anim_group*)pAnimGroupPtr;
    ASSERT( pAnimGroup->NBones == m_NBones );
    MaxBones = pAnimGroup->NBones;

    // Build the world translation matrix which identifies where in world space the skin will be located.
    WorldTranslation.Identity();
    WorldTranslation.Translate( WorldPos );

    // Loop through all bones (The parent bones will AND must be processed prior to the children bones)
    // Determine the rotation and translation values for the bones.
    for( BoneID = 0; BoneID < MaxBones; BoneID++)
    {
        //////////////////////////////////////////////////////////////////
        // ABOUT ANIMATION MIRRORING:                                               Y
        //                                                                          |
        //  The skeleton is assumed to be aligned facing positive Z axis,     X     |     Z
        //  with the left side on positive X axis, which means mirroring       \    |    /
        //  for left-right is done using the YZ plane.                           \  |  /
        //                                                                         \|/
        //  To get the mirrored animation, instead of using the current
        //  bone's ParentToBone offset, we need to get that offset from
        //  the bone's mirrored counterpart.  That offset is mirrored
        //  across the YZ plane by simply negating the X component.
        //
        //  The rotation values are handled in a similar fashion, getting
        //  the mirrored couterpart's rotation, and mirroring it across
        //  the YZ plane as well.  To do that we just negate the Yaw and
        //  Roll(Y and Z) values.
        //
        ////////////////////////////////////////////////////////////////////
        // Decide on rotation and translation values for bone
        if( bMirrored )
        {
            Rot.Pitch =  pBoneRots[pAnimGroup->Bone[BoneID].MirrorID].Pitch;
            Rot.Yaw   = -pBoneRots[pAnimGroup->Bone[BoneID].MirrorID].Yaw;
            Rot.Roll  = -pBoneRots[pAnimGroup->Bone[BoneID].MirrorID].Roll;
            Trans     =  pAnimGroup->Bone[pAnimGroup->Bone[BoneID].MirrorID].ParentToBone;

            Trans.X   = -Trans.X;
        }
        else
        {
            Rot       = pBoneRots[BoneID];
            Trans     = pAnimGroup->Bone[BoneID].ParentToBone;
        }

        // Translation of the bones should be scaled by the global scaling value.
        Trans.X *= WorldScale;
        Trans.Y *= WorldScale;
        Trans.Z *= WorldScale;


        // Get the this bones parent matrix.  If there isn't a parent, then this matrix is for the 
        // ROOT node.  So give it the plain world translation.
        if( pAnimGroup->Bone[BoneID].ParentID != -1)
            pParentMatrix = &pMatrices[pAnimGroup->Bone[BoneID].ParentID];
        else
            pParentMatrix = &WorldTranslation;

        // Build the core matrix for this bone.
        BuildCoreBoneMatrix( &pMatrices[BoneID], &Rot, &Trans, pParentMatrix, &WorldOrientation );
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Complete the rest of the bones matrix.
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // At this point there is a CORE BONE matrix calculated for each bone in the skeleton.
    // These matrices would work fine if the points in the model were mapped to each 
    // bone directly (assuming that each bone was the origin to a particular point on the model).
    //
    // In reality the model's points are all mapped in relation to the origin in model space.
    // So before these matrices are of any use to the model, each bone matrix must have another 
    // translation added to it.  This translation is a constant that is provided in each bone.
    // It's called the origin to bone offset.  To translate the bone back to model space so
    // the other transforms are performed correctly, just translate by -OriginToBone
    //
    //  NOTE: for mirroring, OriginToBone must be retrieved from the
    //  bone's mirror counterpart, and then mirrored across the YZ plane.
    //  This is easily done by negating the X component, but since we are
    //  already using -OriginToBone, we negate the Y and Z for optimization.
    //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( BoneID = 0; BoneID < pAnimGroup->NBones; BoneID++ )
    {
        pCurrentMatrix = &pMatrices[BoneID];

        // Scale the Bone by the values set for this particular bone.
        if( pBoneScales )
        {
            pCurrentBoneScale = &pBoneScales[BoneID];
            pCurrentMatrix->M[0][0] *= pCurrentBoneScale->X;
            pCurrentMatrix->M[0][1] *= pCurrentBoneScale->X;
            pCurrentMatrix->M[0][2] *= pCurrentBoneScale->X;
            pCurrentMatrix->M[1][0] *= pCurrentBoneScale->Y;
            pCurrentMatrix->M[1][1] *= pCurrentBoneScale->Y;
            pCurrentMatrix->M[1][2] *= pCurrentBoneScale->Y;
            pCurrentMatrix->M[2][0] *= pCurrentBoneScale->Z;
            pCurrentMatrix->M[2][1] *= pCurrentBoneScale->Z;
            pCurrentMatrix->M[2][2] *= pCurrentBoneScale->Z;
        }

        // Translate the point by -OriginToBone to get the point with respect to the bone.
        // Adding this last translation to the current matrix will complete the final
        // Local to World matrix for that bone.
        if( bMirrored )
        {
            OriginToBone = pAnimGroup->Bone[pAnimGroup->Bone[BoneID].MirrorID].OriginToBone;
            OriginToBone.Y = -OriginToBone.Y;
            OriginToBone.Z = -OriginToBone.Z;
        }
        else
        {
            OriginToBone = -pAnimGroup->Bone[BoneID].OriginToBone;
        }

        // Normally you would create a separate translation matrix using the OriginToBone vector, and 
        // multiply the current bone matrix to add the translation into the transform.
        // If you work out all of the math, the only value that is adjusted is the translation values
        // in the current matrix.  Since you know this, cut out the rest of the matrix calculations.
        //
        // M is the matrix before adding the last Origin to bone translation.
        // T is a matrix that you could build using the Origin to bone Translations.
        //
        //  M = | M00  M01  M02  M03 |  T = | 1  0  0  O2BX |
        //      | M10  M11  M12  M13 |      | 0  1  0  O2BY |
        //      | M20  M21  M22  M23 |      | 0  0  1  O2BZ |
        //      |   0    0    0    1 |      | 0  0  0  1    |
        //
        //
        // MFinal = T * M
        //
        // MFinal = | (M00+0+0+0)  (M01+0+0+0)  (M02+0+0+0)  (M00*02BX+M01*02BY+M02*02BZ+M03) |
        //          | (M10+0+0+0)  (M11+0+0+0)  (M12+0+0+0)  (M10*02BX+M11*02BY+M12*02BZ+M13) |
        //          | (M20+0+0+0)  (M21+0+0+0)  (M22+0+0+0)  (M20*02BX+M21*02BY+M22*02BZ+M23) |
        //          |   (0+0+0+0)    (0+0+0+0)    (0+0+0+0)                         (0+0+0+1) |
        //
        //
        // CPU Version:
        pCurrentMatrix->M[3][0] += pCurrentMatrix->M[0][0] * OriginToBone.X +
                                   pCurrentMatrix->M[1][0] * OriginToBone.Y +
                                   pCurrentMatrix->M[2][0] * OriginToBone.Z;

        pCurrentMatrix->M[3][1] += pCurrentMatrix->M[0][1] * OriginToBone.X +
                                   pCurrentMatrix->M[1][1] * OriginToBone.Y +
                                   pCurrentMatrix->M[2][1] * OriginToBone.Z;
        
        pCurrentMatrix->M[3][2] += pCurrentMatrix->M[0][2] * OriginToBone.X +
                                   pCurrentMatrix->M[1][2] * OriginToBone.Y +
                                   pCurrentMatrix->M[2][2] * OriginToBone.Z;

#ifdef RENDER_BONE_CENTERS
        BoneCenter[BoneID] = pCurrentMatrix->Transform( pAnimGroup->Bone[BoneID].OriginToBone );
#endif
    }
}

//==========================================================================

void QSkin::BuildLightingMatrix( matrix4* pLightMatrix, matrix4* pColorMatrix )
{
    matrix4         LightDir;
    matrix4         LightColors;
    lighting        EngLighting;

    //---   Figure out the Light Dir matrix
    ENG_GetActiveLighting( EngLighting );
    if ( EngLighting.Directional[0].Active )
    {
        LightDir.M[0][0] = -EngLighting.Directional[0].Direction.X;
        LightDir.M[0][1] = -EngLighting.Directional[0].Direction.Y;
        LightDir.M[0][2] = -EngLighting.Directional[0].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero...
        LightDir.M[0][0] = 0.0f;
        LightDir.M[0][1] = 0.0f;
        LightDir.M[0][2] = 0.0f;
    }
    LightDir.M[0][3] = 0.0f;
    if ( EngLighting.Directional[1].Active )
    {
        LightDir.M[1][0] = -EngLighting.Directional[1].Direction.X;
        LightDir.M[1][1] = -EngLighting.Directional[1].Direction.Y;
        LightDir.M[1][2] = -EngLighting.Directional[1].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero
        LightDir.M[1][0] = 0.0f;
        LightDir.M[1][1] = 0.0f;
        LightDir.M[1][2] = 0.0f;
    }
    LightDir.M[1][3] = 0.0f;
    if ( EngLighting.Directional[2].Active )
    {
        LightDir.M[2][0] = -EngLighting.Directional[2].Direction.X;
        LightDir.M[2][1] = -EngLighting.Directional[2].Direction.Y;
        LightDir.M[2][2] = -EngLighting.Directional[2].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero
        LightDir.M[2][0] = 0.0f;
        LightDir.M[2][1] = 0.0f;
        LightDir.M[2][2] = 0.0f;
    }
    LightDir.M[2][3] = 0.0f;
    LightDir.M[3][0] = 0.0f;
    LightDir.M[3][1] = 0.0f;
    LightDir.M[3][2] = 0.0f;
    LightDir.M[3][3] = 1.0f;

    //---   Figure out the Light Colors matrix
    if ( EngLighting.Directional[0].Active )
    {
        LightColors.M[0][0] = (f32)(EngLighting.Directional[0].Color.R/255.0f);
        LightColors.M[1][0] = (f32)(EngLighting.Directional[0].Color.G/255.0f);
        LightColors.M[2][0] = (f32)(EngLighting.Directional[0].Color.B/255.0f);
        LightColors.M[3][0] = (f32)(EngLighting.Directional[0].Color.A/255.0f);
    }
    else
    {
        LightColors.M[0][0] = 0.0f;
        LightColors.M[1][0] = 0.0f;
        LightColors.M[2][0] = 0.0f;
        LightColors.M[3][0] = 0.0f;
    }
    if ( EngLighting.Directional[1].Active )
    {
        LightColors.M[0][1] = (f32)(EngLighting.Directional[1].Color.R/255.0f);
        LightColors.M[1][1] = (f32)(EngLighting.Directional[1].Color.G/255.0f);
        LightColors.M[2][1] = (f32)(EngLighting.Directional[1].Color.B/255.0f);
        LightColors.M[3][1] = (f32)(EngLighting.Directional[1].Color.A/255.0f);
    }
    else
    {
        LightColors.M[0][1] = 0.0f;
        LightColors.M[1][1] = 0.0f;
        LightColors.M[2][1] = 0.0f;
        LightColors.M[3][1] = 0.0f;
    }
    if ( EngLighting.Directional[2].Active )
    {
        LightColors.M[0][2] = (f32)(EngLighting.Directional[2].Color.R/255.0f);
        LightColors.M[1][2] = (f32)(EngLighting.Directional[2].Color.G/255.0f);
        LightColors.M[2][2] = (f32)(EngLighting.Directional[2].Color.B/255.0f);
        LightColors.M[3][2] = (f32)(EngLighting.Directional[2].Color.A/255.0f);
    }
    else
    {
        LightColors.M[0][2] = 0.0f;
        LightColors.M[1][2] = 0.0f;
        LightColors.M[2][2] = 0.0f;
        LightColors.M[3][2] = 0.0f;
    }
    LightColors.M[0][3] = (f32)(EngLighting.Ambient.R/255.0f);
    LightColors.M[1][3] = (f32)(EngLighting.Ambient.G/255.0f);
    LightColors.M[2][3] = (f32)(EngLighting.Ambient.B/255.0f);
    LightColors.M[3][3] = (f32)(EngLighting.Ambient.A/255.0f);

    //---   set up the lighting matrix
    *pLightMatrix = LightDir;
    *pColorMatrix = LightColors;
}

//==========================================================================
void DrawJoint( vector3& point, f32 Scale, matrix4* pMatrix )
{
    s32     i;
    vector3 line[2];
    color   Color[2];
        
    // Draw a geometric shape around the joint if the matrix isn't provided.
    if( pMatrix == NULL )
    {
        vector3 points[6];
        for( i = 0; i < 6; i++ )
        {
            points[i] = point;
        }

        Color[0].Set( 0, 0, 0, 255 );
        Color[1] = Color[0];

        points[0].X -= Scale;
        points[1].Z += Scale;
        points[2].X += Scale;
        points[3].Z -= Scale;
        points[4].Y -= Scale;
        points[5].Y += Scale;

        line[0] = points[0];    line[1] = points[1];    DRAW_Lines( 2, line, Color );
        line[0] = points[1];    line[1] = points[2];    DRAW_Lines( 2, line, Color );
        line[0] = points[2];    line[1] = points[3];    DRAW_Lines( 2, line, Color );
        line[0] = points[3];    line[1] = points[0];    DRAW_Lines( 2, line, Color );
        line[0] = points[0];    line[1] = points[5];    DRAW_Lines( 2, line, Color );
        line[0] = points[5];    line[1] = points[2];    DRAW_Lines( 2, line, Color );
        line[0] = points[2];    line[1] = points[4];    DRAW_Lines( 2, line, Color );
        line[0] = points[4];    line[1] = points[0];    DRAW_Lines( 2, line, Color );
        line[0] = points[1];    line[1] = points[5];    DRAW_Lines( 2, line, Color );
        line[0] = points[5];    line[1] = points[3];    DRAW_Lines( 2, line, Color );
        line[0] = points[3];    line[1] = points[4];    DRAW_Lines( 2, line, Color );
        line[0] = points[4];    line[1] = points[1];    DRAW_Lines( 2, line, Color );
    }
    else
    {
        f32 AxisScale = Scale * 1.5f;
        matrix4 RotationOnlyMatrix(*pMatrix);
        
        RotationOnlyMatrix.ClearTranslation( );

        Color[1].Set( 255, 0, 0 );
        Color[0] = Color[1];
        line[0].Set( 0.0f,  0.0f,  0.0f );
        line[1].Set( AxisScale, 0.0f,  0.0f );
        line[0] = RotationOnlyMatrix.Transform( line[0] );
        line[1] = RotationOnlyMatrix.Transform( line[1] );

        line[0] += point;
        line[1] += point;
        DRAW_Lines( 2, line, Color );

        Color[1].Set( 0, 255, 0 );
        Color[0] = Color[1];
        line[0].Set( 0.0f,  0.0f,  0.0f );
        line[1].Set( 0.0f,  AxisScale, 0.0f );
        line[0] = RotationOnlyMatrix.Transform( line[0] );
        line[1] = RotationOnlyMatrix.Transform( line[1] );
        line[0] += point;
        line[1] += point;
        DRAW_Lines( 2, line, Color );

        Color[1].Set( 0, 0, 255 );
        Color[0] = Color[1];
        line[0].Set( 0.0f,  0.0f,  0.0f );
        line[1].Set( 0.0f,  0.0f,  AxisScale );
        line[0] = RotationOnlyMatrix.Transform( line[0] );
        line[1] = RotationOnlyMatrix.Transform( line[1] );
        line[0] += point;
        line[1] += point;
        DRAW_Lines( 2, line, Color );
    }

    // Draw the axis.
}




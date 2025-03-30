////////////////////////////////////////////////////////////////////////////
//
// XBOX_Geom.cpp - geom routines specific to the XBOX
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"
#include "x_memory.hpp"
#include "x_time.hpp"

#include "Q_XBOX.hpp"
#include "Q_Engine.hpp"
#include "Q_View.hpp"
#include "Q_VRAM.hpp"
#include "Q_Geom.hpp"

#include "Q_Draw.hpp"

////////////////////////////////////////////////////////////////////////////
// Note of vertex shaders:
//   We are only going to use a vertex shader if we are using dynamic(vertex)
//   lighting on the object.  Otherwise we use the built in diffuse color.
//   Also, only these objects can have environment mapping.
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// QGeom(XBOX) Vertex Shader info
////////////////////////////////////////////////////////////////////////////

#include "XBOX_GeomShaderDefines.h"
#include "XBOX_GeomObjShader.h"
#include "XBOX_GeomMultiTexShader.h"
#include "XBOX_GeomShadowShader.h"
#include "XBOX_PSMultiTex.h"
#include "XBOX_GeomEnvMapShader.h"
#include "XBOX_PSEnvMap.h"
#include "XBOX_Geom2DiffuseShader.h"
#include "XBOX_PS_2Diffuse.h"
#include "XBox_PSSimpleDiffuseNoLight.h"
#include "XBOX_PSMultiTex_Cheat.h"
#include "XBOX_PSEnvMap_Cheat.h"
#include "XBOX_PS_2Diffuse_Cheat.h"
#include "XBox_PSSimpleDiffuseNoLight_Cheat.h"

#define SAFE_FREE(p) if (p) { x_free(p); p=NULL;}

u32 s_GEOM_VertexFormat[] =
{
    D3DVSD_STREAM   (0),
    D3DVSD_REG      (0, D3DVSDT_FLOAT3),            // v.xyz = vPos
    D3DVSD_REG      (1, D3DVSDT_FLOAT3),            // v.xyz = vNormal
    D3DVSD_REG      (2, D3DVSDT_FLOAT2),            // v.xy  = uv
    D3DVSD_END      ()
};

u32 s_GEOM_VertexFormatMultiTex[] =
{
    D3DVSD_STREAM   (0),
    D3DVSD_REG      (0, D3DVSDT_FLOAT3),            // v.xyz = vPos
    D3DVSD_REG      (5, D3DVSDT_D3DCOLOR),          // v.xyzw = ARGB
    D3DVSD_REG      (2, D3DVSDT_FLOAT2),            // v.xy  = uv1
    D3DVSD_END      ()
};

u32 s_GEOM_VertexFormatEnvMap[] =
{
    D3DVSD_STREAM   (0),
    D3DVSD_REG      (0, D3DVSDT_FLOAT3),            // v.xyz = vPos
    D3DVSD_REG      (5, D3DVSDT_D3DCOLOR),          // v.xyzw = ARGB
    D3DVSD_REG      (1, D3DVSDT_FLOAT3),            // v.xyz = vNormal
    D3DVSD_REG      (2, D3DVSDT_FLOAT2),            // v.xy  = uv1
    D3DVSD_END      ()
};

////////////////////////////////////////////////////////////////////////////
// QGeom STATICS
////////////////////////////////////////////////////////////////////////////

/****************************************************************************
*                                                                           *
*  If you add a Pixel Shader, update this count and the 3 tables below it.  *
*                                                                           *
****************************************************************************/
static const s32 kPixelShaderCount = 8;
static const s32 kToonCheatOffset = 4;
static xbool s_bPSActive[ kPixelShaderCount ] = 
{
    FALSE,                      // Multi Tex is not active
    FALSE,                      // Environment Mapped is not active
    FALSE,                      // 2 Diffuse is not active
    FALSE,                      // single diffuse with no lighting ramp
    FALSE,                      // Multi Tex Cheat is not active
    FALSE,                      // Environment Mapped Cheat is not active
    FALSE,                      // 2 Diffuse Cheat is not active
    FALSE,                      // single diffuse cheat with no lighting ramp
};
static s32 s_i_PS_ID[ kPixelShaderCount ] =
{
    -1,                         // Multi Tex is unregistered
    -1,                         // Environment Mapped is unregistered
    -1,                         // 2 Diffuse is unregistered
    -1,                         // single diffuse with no lighting ramp
    -1,                         // Multi Tex Cheat is unregistered
    -1,                         // Environment Mapped Cheat is unregistered
    -1,                         // 2 Diffuse Cheat is unregistered
    -1,                         // single diffuse cheat with no lighting ramp
};
static s32 *s_i_PS_Addresses[ kPixelShaderCount ] =
{
    (s32*)&s_GEOM_PS_MultiTexShader[ 0 ],                   // Multi Tex address from XBOX_PSMultiTex.h
    (s32*)&s_GEOM_PS_EnvMapShader[ 0 ],                     // Env Map address from XBOX_PSEnvMap.h
    (s32*)&s_GEOM_PS_2DiffuseShader[ 0 ],                   // 2 Diffuse address from XBOX_PS_2Diffuse.h
    (s32*)&s_GEOM_PS_SimpleDiffuseNoLightShader[ 0 ],       // single diffuse address from XBOX_PSSimpleDiffuseNoLight.h
    (s32*)&s_GEOM_PS_MultiTex_CheatShader[ 0 ],             // Multi Tex Cheat address from XBOX_PSMultiTex_Cheat.h
    (s32*)&s_GEOM_PS_EnvMap_CheatShader[ 0 ],               // Env Map Cheat address from XBOX_PSEnvMap_Cheat.h
    (s32*)&s_GEOM_PS_2Diffuse_CheatShader[ 0 ],             // 2 Diffuse Cheat address from XBOX_PS_2Diffuse_Cheat.h
    (s32*)&s_GEOM_PS_SimpleDiffuseNoLight_CheatShader[ 0 ], // single diffuse Cheat address from XBOX_PSSimpleDiffuseNoLight_Cheat.h
};

static const s32 kPSMultiTexID                  = 0;
static const s32 kPSEnvMapID                    = 1;
static const s32 kPS2DiffuseID                  = 2;
static const s32 kPSSimpleDiffuseNoLight        = 3;
static const s32 kPSMultiTex_CheatID            = 4;
static const s32 kPSEnvMap_CheatID              = 5;
static const s32 kPS2Diffuse_CheatID            = 6;
static const s32 kPSSimpleDiffuseNoLight_Cheat  = 7;


/****************************************************************************
*                                                                           *
*  If you add a Vertex Shader, update this count and the 2 tables below it. *
*                                                                           *
****************************************************************************/
static const s32 kVertexShaderCount = 5;
static xbool s_bVSActive[ kVertexShaderCount ] =
{
    FALSE,                      // generic?
    FALSE,                      // multi tex
    FALSE,                      // env map
    FALSE,                      // 2 diffuse
    FALSE,                      // shadow
};
static s32 s_i_VS_ID[ kVertexShaderCount ] =
{
    -1,                         // generic
    -1,                         // multi tex
    -1,                         // env map
    -1,                         // 2 diffuse
    -1,                         // shadow
};
static s32 *s_i_VS_Addresses[ kVertexShaderCount ] =
{
    (s32*)&s_GEOM_ObjShader[ 0 ],       // generic address from XBOX_GeomObjShader.h
    (s32*)&s_GEOM_MultiTexShader[ 0 ],  // multi tex address from XBOX_GeomMultiTexShader.h
    (s32*)&s_GEOM_EnvMapShader[ 0 ],    // env map address from XBOX_GeomEnvMapShader.h
    (s32*)&s_GEOM_2DiffuseShader[ 0 ],  // 2 diffuse from XBOX_Geom2DiffuseShader.h
    (s32*)&s_GEOM_ShadowShader[ 0 ],    // shadow from XBOX_GeomShadowShader.h
};

static const s32 kVSGenericID   = 0;
static const s32 kVSMultiTexID  = 1;
static const s32 kVSEnvMapID    = 2;
static const s32 kVS2DiffuseID  = 3;
static const s32 kVSShadowID    = 4;

static s32 s_VertexShaderID = -1;
static s32 s_VertexShaderMultiTexID = -1;
static s32 s_VertexShaderEnvMapID = -1;
static s32 s_VertexShader2PassDiffuseID = -1;
static s32 s_PixelShaderMultiTexID = -1;
static s32 s_PixelShaderEnvMapID = -1;
static s32 s_PixelShader2DiffuseID = -1;
static s32 s_ShadowShaderID = -1;

static xbool s_bMultiTexPSActive = FALSE;
static xbool s_bEnvMapPSActive = FALSE;
static xbool s_b2PassDiffusePSActive = FALSE;
static xbool s_bIsMultiTextured = FALSE;
static xbool s_bUseEnvMapping = FALSE;



////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////
QGeom::QGeom( void )
{
    InitData( );
}

//-------------------------------------------------------------------------------------------------------------------------------------
QGeom::QGeom( char* filename )
{
    InitData( );
    SetupFromResource( filename );
}

//-------------------------------------------------------------------------------------------------------------------------------------
QGeom::~QGeom( void )
{
    KillData();
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::InitData( void )
{
    m_Name[0]           = '\0';
    m_Flags             = 0;
    m_L2W.Identity();

    m_NTextures         = 0;
    m_NTexturesOwned    = 0;
    m_pTextureNames     = NULL;
    m_pTextures         = NULL;
    m_pTexturePtrs      = NULL;

    m_NMeshes           = 0;
    m_pMeshes           = NULL;
    m_MeshLocked        = -1;

    m_NSubMeshes        = 0;
    m_pSubMeshes        = NULL;

    m_pRawData          = NULL;

    m_MeshTextureArray  = NULL;

    m_NMaterials        = 0;
    m_NMatStages        = 0;
    m_pMaterials        = NULL;
    m_pMatStages        = NULL;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::KillData( void )
{
    s32 i;
    for( i = 0; i < m_NMeshes; i++ )
    {
        SAFE_RELEASE(m_pMeshes[i].pD3DVerts);
        SAFE_RELEASE(m_pMeshes[i].pD3DIndices);
    }

    SAFE_FREE( m_pRawData );
    SAFE_FREE( m_pMeshes );
    SAFE_FREE( m_pSubMeshes );
    SAFE_FREE( m_pTextureNames );
    SAFE_DELETE( m_pMaterials );
    SAFE_DELETE( m_pMatStages );

    m_NMaterials = 0;
    m_NMatStages = 0;

    // delete the mesh texture array
    SAFE_DELETE_ARRAY( m_MeshTextureArray )

    KillOwnedTextures();
}

//-------------------------------------------------------------------------------------------------------------------------------------

static xbool s_bMultipassOverride = FALSE;

void QGeom::PrepareForMultipass( void )
{
    s_bMultipassOverride = TRUE;
}

//==========================================================================

void QGeom::RecoverFromMultipass( void )
{
    s_bMultipassOverride = FALSE;
}

//==========================================================================

////////////////////////////////////////////////////////////////////////
// VERY IMPORTANT NOTE: THE SetupFromResource ROUTINE IS OVERLOADED A
// COUPLE OF TIMES. IF THE DATA FORMAT CHANGES AT ALL, MAKE SURE THE
// OVERLOADED ROUTINES GET CHANGED AS WELL.
////////////////////////////////////////////////////////////////////////
//
// A XBOX GEOM file is laid out like this:
//
//      Header          (t_GeomFileHeader)
//
//      Geometry
//          Mesh Data       (array of t_GeomMesh)
//          Vert Data       (array of D3D Flexible Vertex Format(FVF) Verts.
//          Index Data      (array of D3D Indices (16-bit signed int))
//          SubMesh Data    (array of t_GeomSubMesh)
//
//      Textures
//          Texture names   (array of texture names 32 chars/name)
//          Texture BMP's   (array of texture xbitmaps
//
////////////////////////////////////////////////////////////////////////////
void QGeom::SetupFromResource( char* pFilename )
{
    X_FILE* pFilePtr;

    //--- open the file
    pFilePtr = x_fopen( pFilename, "rb" );
    ASSERT( pFilePtr != NULL );

    SetupFromResource( pFilePtr );

    //--- close the file
    x_fclose( pFilePtr );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::SetupFromResource( X_FILE* pFilePtr )
{
    if (s_bMultipassOverride)
    {
        SetupFromResource_Multipass( pFilePtr );
        return;
    }
    ASSERT( pFilePtr );

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    s32     i, MeshIndex;
    byte*   pTempData;

    t_GeomFileHeader hdr;

    // Waste any previous data on the setup.
    KillData();
    InitData();

    // Load in the file header
    x_fread( &hdr, sizeof(t_GeomFileHeader), 1, pFilePtr );

    // Fill in the basic Geom data from the header block.
    x_strcpy( m_Name, hdr.Name );
    m_Flags         = hdr.Flags;
    m_NTextures     = hdr.NTextures;
    m_NMeshes       = hdr.NMeshes;
    m_NSubMeshes    = hdr.NSubMeshes;

    // Allocate space for all of the data
    m_pMeshes    = (t_GeomMesh*)   x_malloc( sizeof(t_GeomMesh)    * m_NMeshes );    ASSERT( m_pMeshes );
    m_pSubMeshes = (t_GeomSubMesh*)x_malloc( sizeof(t_GeomSubMesh) * m_NSubMeshes ); ASSERT( m_pSubMeshes );

    // Load the mesh data
    for( MeshIndex = 0; MeshIndex < m_NMeshes; MeshIndex++ )
    {
        // Read the mesh struct out, then deal with the vertices.
        x_fread( &m_pMeshes[MeshIndex], sizeof(t_GeomMesh), 1, pFilePtr );
        m_pMeshes[MeshIndex].pD3DVerts   = NULL;
        m_pMeshes[MeshIndex].pD3DIndices = NULL;

        // Create the Vertex buffer, then read the data into it.
        DXWARN( pD3DDevice->CreateVertexBuffer( m_pMeshes[MeshIndex].VertFormatSize * m_pMeshes[MeshIndex].NVerts,
                                                m_pMeshes[MeshIndex].VertUsage,
                                                m_pMeshes[MeshIndex].VertFormat,
                                                D3DPOOL_MANAGED,
                                                &m_pMeshes[MeshIndex].pD3DVerts ));

        // Lock the vertex buffer.
        DXWARN( m_pMeshes[MeshIndex].pD3DVerts->Lock( 0,
                                                      m_pMeshes[MeshIndex].VertFormatSize * m_pMeshes[MeshIndex].NVerts,
                                                      (byte**)&pTempData,
                                                      0 ));

        // Read in the vertex information for this mesh.
        x_fread( pTempData, m_pMeshes[MeshIndex].VertFormatSize, m_pMeshes[MeshIndex].NVerts, pFilePtr );

        // Unlock the vertex buffer.
        DXWARN( m_pMeshes[MeshIndex].pD3DVerts->Unlock());

        // Create the display list (index array) for this mesh.
        DXWARN( pD3DDevice->CreateIndexBuffer( sizeof(u16) * m_pMeshes[MeshIndex].NIndices,
                                               m_pMeshes[MeshIndex].VertUsage,
                                               D3DFMT_INDEX16,
                                               D3DPOOL_MANAGED,
                                               &m_pMeshes[MeshIndex].pD3DIndices ));

        // Lock the display list.
        DXWARN( m_pMeshes[MeshIndex].pD3DIndices->Lock( 0,
                                                        sizeof(s16) * m_pMeshes[MeshIndex].NIndices,
                                                        (byte**)&pTempData,
                                                        0 ));

        // Read the information from the mesh.
        x_fread( pTempData, sizeof(s16), m_pMeshes[MeshIndex].NIndices, pFilePtr );

        // Unlock the list.
        DXWARN( m_pMeshes[MeshIndex].pD3DIndices->Unlock() );
    }
    
    // Load the SubMesh data
    x_fread( m_pSubMeshes, sizeof(t_GeomSubMesh), m_NSubMeshes, pFilePtr );

    // Allocate enough space to hold the strings for the texture names.
    if( hdr.NTextures )
    {
        m_pTextureNames = (char*)x_malloc( sizeof(char) * TEXTURE_NAME_LENGTH * hdr.NTextures );  ASSERT( m_pTextureNames );
    }

    // Load in the texture names
    x_fread( m_pTextureNames, sizeof(char) * TEXTURE_NAME_LENGTH, hdr.NTextures, pFilePtr );
    
    // Allocate bitmap memory if textures stored in file
    if( hdr.NTexturesInFile > 0 )
    {
        m_Flags             |= GEOM_FLAG_OWNS_TEXTURES;
        m_NTexturesOwned    = hdr.NTexturesInFile;
        m_pTextures         = new x_bitmap[ m_NTexturesOwned ];
        ASSERT( m_pTextures != NULL );

        //--- Load the texture data
        for( i = 0; i < hdr.NTexturesInFile; i++ )
        {
            VERIFY( m_pTextures[i].Load( pFilePtr ) );
            VRAM_Register( m_pTextures[i] );
        }
    }

    m_MeshTextureArray = (x_bitmap**)x_malloc( sizeof(x_bitmap*) * m_NMeshes );
    ASSERT( m_MeshTextureArray != NULL );
    x_memset( m_MeshTextureArray, NULL, sizeof(x_bitmap*) * m_NMeshes );

//    x_fclose( pFilePtr );
}



//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::SetupFromResource_Multipass( X_FILE* pFilePtr )
{
    ASSERT( pFilePtr );

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    s32     i, MeshIndex;
    byte*   pTempData;

    t_GeomFileHeader_Multipass hdr;

    // Waste any previous data on the setup.
    KillData();
    InitData();

    // Load in the file header
    x_fread( &hdr, sizeof(t_GeomFileHeader_Multipass), 1, pFilePtr );

    // Fill in the basic Geom data from the header block.
    x_strcpy( m_Name, hdr.Name );
    m_Flags         = hdr.Flags;
    m_NTextures     = hdr.NTextures;
    m_NMeshes       = hdr.NMeshes;
    m_NSubMeshes    = hdr.NSubMeshes;

    // Allocate space for all of the data
    m_pMeshes    = (t_GeomMesh*)   x_malloc( sizeof(t_GeomMesh)    * m_NMeshes );    ASSERT( m_pMeshes );
    m_pSubMeshes = (t_GeomSubMesh*)x_malloc( sizeof(t_GeomSubMesh) * m_NSubMeshes ); ASSERT( m_pSubMeshes );

    // Load the mesh data
    for( MeshIndex = 0; MeshIndex < m_NMeshes; MeshIndex++ )
    {
        // Read the mesh struct out, then deal with the vertices.
        x_fread( &m_pMeshes[MeshIndex], sizeof(t_GeomMesh), 1, pFilePtr );
        m_pMeshes[MeshIndex].pD3DVerts   = NULL;
        m_pMeshes[MeshIndex].pD3DIndices = NULL;

        // Create the Vertex buffer, then read the data into it.
        DXWARN( pD3DDevice->CreateVertexBuffer( m_pMeshes[MeshIndex].VertFormatSize * m_pMeshes[MeshIndex].NVerts,
                                                m_pMeshes[MeshIndex].VertUsage,
                                                m_pMeshes[MeshIndex].VertFormat,
                                                D3DPOOL_MANAGED,
                                                &m_pMeshes[MeshIndex].pD3DVerts ));

        // Lock the vertex buffer.
        DXWARN( m_pMeshes[MeshIndex].pD3DVerts->Lock( 0,
                                                      m_pMeshes[MeshIndex].VertFormatSize * m_pMeshes[MeshIndex].NVerts,
                                                      (byte**)&pTempData,
                                                      0 ));

        // Read in the vertex information for this mesh.
        x_fread( pTempData, m_pMeshes[MeshIndex].VertFormatSize, m_pMeshes[MeshIndex].NVerts, pFilePtr );

        // Unlock the vertex buffer.
        DXWARN( m_pMeshes[MeshIndex].pD3DVerts->Unlock());

        // Create the display list (index array) for this mesh.
        DXWARN( pD3DDevice->CreateIndexBuffer( sizeof(u16) * m_pMeshes[MeshIndex].NIndices,
                                               m_pMeshes[MeshIndex].VertUsage,
                                               D3DFMT_INDEX16,
                                               D3DPOOL_MANAGED,
                                               &m_pMeshes[MeshIndex].pD3DIndices ));

        // Lock the display list.
        DXWARN( m_pMeshes[MeshIndex].pD3DIndices->Lock( 0,
                                                        sizeof(s16) * m_pMeshes[MeshIndex].NIndices,
                                                        (byte**)&pTempData,
                                                        0 ));

        // Read the information from the mesh.
        x_fread( pTempData, sizeof(s16), m_pMeshes[MeshIndex].NIndices, pFilePtr );

        // Unlock the list.
        DXWARN( m_pMeshes[MeshIndex].pD3DIndices->Unlock() );
    }
    
    // Load the SubMesh data
    x_fread( m_pSubMeshes, sizeof(t_GeomSubMesh), m_NSubMeshes, pFilePtr );

    // Allocate enough space to hold the strings for the texture names.
    if( hdr.NTextures )
    {
        m_pTextureNames = (char*)x_malloc( sizeof(char) * TEXTURE_NAME_LENGTH * hdr.NTextures );  ASSERT( m_pTextureNames );
    }

    // Load in the texture names
    x_fread( m_pTextureNames, sizeof(char) * TEXTURE_NAME_LENGTH, hdr.NTextures, pFilePtr );
    
    // Allocate bitmap memory if textures stored in file
    if( hdr.NTexturesInFile > 0 )
    {
        m_Flags             |= GEOM_FLAG_OWNS_TEXTURES;
        m_NTexturesOwned    = hdr.NTexturesInFile;
        m_pTextures         = new x_bitmap[ m_NTexturesOwned ];
        ASSERT( m_pTextures != NULL );

        //--- Load the texture data
        for( i = 0; i < hdr.NTexturesInFile; i++ )
        {
            VERIFY( m_pTextures[i].Load( pFilePtr ) );
            VRAM_Register( m_pTextures[i] );
        }
    }

    m_MeshTextureArray = (x_bitmap**)x_malloc( sizeof(x_bitmap*) * m_NMeshes );
    ASSERT( m_MeshTextureArray != NULL );
    x_memset( m_MeshTextureArray, NULL, sizeof(x_bitmap*) * m_NMeshes );


    //---   Load the Materials
    if (hdr.NMaterials)
    {
        m_NMaterials        = hdr.NMaterials;
        m_pMaterials        = new t_GeomMaterial[ m_NMaterials ];
        for (i=0; i<m_NMaterials; ++i)
        {
            x_fread( &m_pMaterials[ i ], sizeof( t_GeomMaterial ), 1, pFilePtr );
        }
    }

    //---   Load the Stages associated with the Materials
    if (hdr.NMatStages)
    {
        m_NMatStages        = hdr.NMatStages;
        m_pMatStages        = new t_GeomMatStage[ m_NMatStages ];
        for (i=0; i<m_NMatStages; ++i)
        {
            x_fread( &m_pMatStages[ i ], sizeof( t_GeomMatStage ), 1, pFilePtr );
        }
    }

    for (i=0; i<m_NMaterials; ++i)
    {
        s32 j;
        for (j=0; j<m_pMaterials[ i ].nStages; ++j)
        {
            ASSERT( j < kMaxNumStages);
            s32 iTexture = -1;
            if (m_pMaterials[ i ].nStages > 0)
                iTexture = m_pMatStages[ m_pMaterials[ i ].iStages[ j ] ].iTexture;
            if (iTexture >= 0)
            {
                if (m_pTextures)
                {
                    if (m_pTextures[ iTexture ].GetVRAMID() == -1)
                    {
                        VRAM_Register( m_pTextures[ iTexture ], j );
                    }
                }
                else if (m_pTexturePtrs)
                {
                    if (m_pTexturePtrs[ iTexture ]->GetVRAMID() == -1)
                    {
                        VRAM_Register( *m_pTexturePtrs[ iTexture ], j );
                    }
                }
            }
        }
    }
//    x_fclose( pFilePtr );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::SetupFromResource( byte* pByteStream )
{
    // Later
    ASSERT( FALSE );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateVertexShader( void )
{
    if( s_VertexShaderID == -1 )
        s_VertexShaderID = XBOX_RegisterVertexShader( s_GEOM_ObjShader, s_GEOM_VertexFormat, 0 );

    XBOX_ActivateVertexShader( s_VertexShaderID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateVertexShaderMultiTex( void )
{
    if( s_VertexShaderMultiTexID == -1 )
        s_VertexShaderMultiTexID = XBOX_RegisterVertexShader( s_GEOM_MultiTexShader, s_GEOM_VertexFormatMultiTex, 0 );

    XBOX_ActivateVertexShader( s_VertexShaderMultiTexID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateVertexShaderEnvMap( void )
{
    if (s_VertexShaderEnvMapID == -1)
        s_VertexShaderEnvMapID = XBOX_RegisterVertexShader( s_GEOM_EnvMapShader, s_GEOM_VertexFormatEnvMap, 0 );

    XBOX_ActivateVertexShader( s_VertexShaderEnvMapID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateVertexShader2DiffuseTex( void )
{
    if (s_VertexShader2PassDiffuseID == -1)
        s_VertexShader2PassDiffuseID = XBOX_RegisterVertexShader( s_GEOM_2DiffuseShader, s_GEOM_VertexFormatMultiTex, 0 );

    XBOX_ActivateVertexShader( s_VertexShader2PassDiffuseID );
}

//-------------------------------------------------------------------------------------------------------------------------------------

void QGeom::ActivateShadowShader( void )
{
    if( s_ShadowShaderID == -1 )
        s_ShadowShaderID = XBOX_RegisterVertexShader( s_GEOM_ShadowShader, s_GEOM_VertexFormat, 0 );

    XBOX_ActivateVertexShader( s_ShadowShaderID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivatePixelShaderMultiTex( void )
{
    if( s_PixelShaderMultiTexID == -1 )
        s_PixelShaderMultiTexID = XBOX_RegisterPixelShader( s_GEOM_PS_MultiTexShader );

    XBOX_ActivatePixelShader( s_PixelShaderMultiTexID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivatePixelShaderEnvMap( void )
{
    if( s_PixelShaderEnvMapID == -1 )
        s_PixelShaderEnvMapID = XBOX_RegisterPixelShader( s_GEOM_PS_EnvMapShader );

    XBOX_ActivatePixelShader( s_PixelShaderEnvMapID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivatePixelShader2DiffuseTex( void )
{
    if( s_PixelShader2DiffuseID == -1 )
        s_PixelShader2DiffuseID = XBOX_RegisterPixelShader( s_GEOM_PS_2DiffuseShader );

    XBOX_ActivatePixelShader( s_PixelShader2DiffuseID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivatePixelShader( s32 iIndex )
{
    if (XBOX_IsToonCheatActive())
        iIndex += kToonCheatOffset;

    if (iIndex < 0 || iIndex>kPixelShaderCount)
        return;

    if (s_i_PS_ID[ iIndex ] == -1)
        s_i_PS_ID[ iIndex ] = XBOX_RegisterPixelShader( s_i_PS_Addresses[ iIndex ] );

    XBOX_ActivatePixelShader( s_i_PS_ID[ iIndex ] );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::DeActivatePixelShaderMultiTex( void )
{
    if( s_PixelShaderMultiTexID != -1 )
        XBOX_DeActivatePixelShader( s_PixelShaderMultiTexID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::DeActivatePixelShader2DiffuseTex( void )
{
    if( s_PixelShader2DiffuseID != -1 )
        XBOX_DeActivatePixelShader( s_PixelShader2DiffuseID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::DeActivatePixelShaderEnvMap( void )
{
    if( s_PixelShaderEnvMapID != -1 )
        XBOX_DeActivatePixelShader( s_PixelShaderEnvMapID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::DeActivatePixelShader( s32 iIndex )
{
    if (XBOX_IsToonCheatActive())
        iIndex += kToonCheatOffset;

    if (iIndex < 0 || iIndex>kPixelShaderCount)
        return;

    if (s_i_PS_ID[ iIndex ] != -1)
        XBOX_DeActivatePixelShader( s_i_PS_ID[ iIndex ] );

}

//-------------------------------------------------------------------------------------------------------------------------------------

void QGeom::SetBlendSettings_FixedAlpha( LPDIRECT3DDEVICE8 pD3DDevice,
                                         u32 &CurrentBlendMode,
                                         u8 &CurrentFixedAlpha,
                                         s32 TextureCount,
                                         s32 *ColorOperations,
                                         s32 *AlphaOperations,
                                         s32 iMaterial,
                                         xbool bMeshHasVertexColor )
{
    ENG_GetBlendMode( CurrentBlendMode, CurrentFixedAlpha );

    // Test to see if a fixed alpha blending mode is to be used.
    if( CurrentBlendMode & ENG_BLEND_FIXED_ALPHA )
    {
        // If this draw mode is using a texture, then set the color arg to the texture, otherwise, just to the
        // diffuse color.
        if( TextureCount && bMeshHasVertexColor )
        {
            ColorOperations[ 0 ] = D3DTOP_MODULATE;
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));

            AlphaOperations[ 0 ] = D3DTOP_MODULATE;
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ));

            ColorOperations[ 1 ] = D3DTOP_SELECTARG1;
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT ));

            AlphaOperations[ 1 ] = D3DTOP_BLENDFACTORALPHA;
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT ));
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ));
        }
        else if( TextureCount )
        {
            ColorOperations[ 0 ] = D3DTOP_MODULATE;
            AlphaOperations[ 0 ] = D3DTOP_MODULATE;
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR ));

            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ));
        }
        else if( bMeshHasVertexColor )
        {
            ColorOperations[ 0 ] = D3DTOP_SELECTARG1;
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE ));

            AlphaOperations[ 0 ] = D3DTOP_BLENDFACTORALPHA;
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE ));
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ));
        }
        else
        {
            ColorOperations[ 0 ] = D3DTOP_DISABLE;
            AlphaOperations[ 0 ] = D3DTOP_DISABLE;
        }
    }
    else
    {
        if (m_pMaterials && m_pMatStages && m_pMaterials[ iMaterial ].nStages > 1)
        {
            ColorOperations[ 0 ] = D3DTOP_MODULATE;
            AlphaOperations[ 0 ] = D3DTOP_MODULATE;
            ColorOperations[ 1 ] = D3DTOP_MODULATE;
            AlphaOperations[ 1 ] = D3DTOP_MODULATE;
            ColorOperations[ 2 ] = D3DTOP_MODULATE;
            AlphaOperations[ 2 ] = D3DTOP_MODULATE;
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
//            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));

//            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
//            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));

            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE ));

//            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
//            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));
        }
        else if( TextureCount && bMeshHasVertexColor )
        {
            ColorOperations[ 0 ] = D3DTOP_MODULATE;
            AlphaOperations[ 0 ] = D3DTOP_MODULATE;
//            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
//            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));

//            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
//            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));
        }
        else if( TextureCount )
        {
            ColorOperations[ 0 ] = D3DTOP_SELECTARG1;
            AlphaOperations[ 0 ] = D3DTOP_SELECTARG1;
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
        }
        else if( bMeshHasVertexColor )
        {
            ColorOperations[ 0 ] = D3DTOP_SELECTARG2;
            AlphaOperations[ 0 ] = D3DTOP_SELECTARG2;
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));
            DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));
        }
        else
        {
            ColorOperations[ 0 ] = D3DTOP_DISABLE;
            AlphaOperations[ 0 ] = D3DTOP_DISABLE;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------

void QGeom::ActivateShaders_DynamicLighting( LPDIRECT3DDEVICE8 pD3DDevice, view *pView, s32 SubMeshID, vector3 *pos )
{
    vector4 vConst;
    if( s_bIsMultiTextured )
    {
        ActivateShaders_MultiTexture( pD3DDevice, pView, SubMeshID );
    }
    else
    {
        ActivateShaders_NoMultiTexture( pD3DDevice );
    }

    // setup the base constant register
    vConst.Set(3.0f, 0.5f, -0.5f, 1.0f);
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_1, &vConst.X, 1) );

    // fill the projection matrix in constant registers
    D3DXMATRIX matView, matProj, matViewProj, matViewProjTranspose;
    pD3DDevice->GetTransform( D3DTS_VIEW,       &matView );
    if (pos)
    {
        D3DXMATRIX temp;
        D3DXMatrixTranslation( &temp, pos->X, pos->Y, pos->Z );
        D3DXMatrixMultiply( &matView, &temp, &matView );
    }
    pD3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
    D3DXMatrixMultiply( &matViewProj, &matView, &matProj );
    D3DXMatrixTranspose( &matViewProjTranspose, &matViewProj );
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_PROJECT_1, &matViewProjTranspose, 4) );
}

//-------------------------------------------------------------------------------------------------------------------------------------

void QGeom::ActivateShaders_MultiTexture( LPDIRECT3DDEVICE8 pD3DDevice, view *pView, s32 SubMeshID )
{
    if (s_bUseEnvMapping)
    {
        s32 iPS = kPSEnvMapID;
        if (XBOX_IsToonCheatActive())
            iPS += kToonCheatOffset;
        if (s_i_VS_ID[ kVSEnvMapID ] == -1)
            s_i_VS_ID[ kVSEnvMapID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVSEnvMapID ], s_GEOM_VertexFormatEnvMap, 0 );
        XBOX_ActivateVertexShader( s_i_VS_ID[ kVSEnvMapID ] );

        if (s_i_PS_ID[ iPS ] == -1)
            s_i_PS_ID[ iPS ] = XBOX_RegisterPixelShader( s_i_PS_Addresses[ iPS ] );
        if (!s_bPSActive[ iPS ]) XBOX_ActivatePixelShader( s_i_PS_ID[ iPS ] );
        s_bPSActive[ iPS ] = TRUE;

        // send Eye vector
        vector3 Eye;
        pView->GetPosition( Eye );
        DXWARN( pD3DDevice->SetVertexShaderConstant( VSH_V_WORLD_EYE, &Eye, 1 ) );

        // send L2W matrix
        matrix4 L2W = m_L2W;
        L2W.ClearTranslation();
        DXWARN( pD3DDevice->SetVertexShaderConstant( VSH_M_L2W_1, (D3DXMATRIX*)&L2W, 3 ) );
    }
    else
    {
        if (m_pMaterials && m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].nStages > 1)
        {
            s32 iPS = kPS2DiffuseID;
            if (XBOX_IsToonCheatActive())
                iPS += kToonCheatOffset;
            if (s_i_VS_ID[ kVS2DiffuseID ] == -1)
                s_i_VS_ID[ kVS2DiffuseID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVS2DiffuseID ], s_GEOM_VertexFormatMultiTex, 0 );
            XBOX_ActivateVertexShader( s_i_VS_ID[ kVS2DiffuseID ] );

            if (s_i_PS_ID[ iPS ] == -1)
                s_i_PS_ID[ iPS ] = XBOX_RegisterPixelShader( s_i_PS_Addresses[ iPS ] );
            if (!s_bPSActive[ iPS ]) XBOX_ActivatePixelShader( s_i_PS_ID[ iPS ] );
            s_bPSActive[ iPS ] = TRUE;
        }
        else
        {
            s32 iPS = kPSMultiTexID;
            if (XBOX_IsToonCheatActive())
                iPS += kToonCheatOffset;
            if (s_i_VS_ID[ kVSMultiTexID ] == -1)
                s_i_VS_ID[ kVSMultiTexID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVSMultiTexID ], s_GEOM_VertexFormatMultiTex, 0 );
            XBOX_ActivateVertexShader( s_i_VS_ID[ kVSMultiTexID ] );

            if (s_i_PS_ID[ iPS ] == -1)
                s_i_PS_ID[ iPS ] = XBOX_RegisterPixelShader( s_i_PS_Addresses[ iPS ] );
            if (!s_bPSActive[ iPS ]) XBOX_ActivatePixelShader( s_i_PS_ID[ iPS ] );
            s_bPSActive[ iPS ] = TRUE;
        }
//        DXWARN( pD3DDevice->SetPixelShaderConstant( 3, &Brightness, 1 ) );
//        DXWARN( pD3DDevice->SetPixelShaderConstant( 2, &Contrast, 1 ) );
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateShaders_NoMultiTexture( LPDIRECT3DDEVICE8 pD3DDevice )
{
    s32 iPS = kPSMultiTexID;
    if (XBOX_IsToonCheatActive())
        iPS += kToonCheatOffset;
    if (s_i_VS_ID[ kVSMultiTexID ] == -1)
        s_i_VS_ID[ kVSMultiTexID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVSMultiTexID ], s_GEOM_VertexFormatMultiTex, 0 );
    XBOX_ActivateVertexShader( s_i_VS_ID[ kVSMultiTexID ] );

    if (s_i_PS_ID[ iPS ] == -1)
        s_i_PS_ID[ iPS ] = XBOX_RegisterPixelShader( s_i_PS_Addresses[ iPS ] );
    if (!s_bPSActive[ iPS ]) 
    {
        XBOX_ActivatePixelShader( s_i_PS_ID[ iPS ] );
        s_bPSActive[ iPS ] = TRUE;
    }

    vector4 vConst;
    vConst.Set( 1.0f, 1.0f, 1.0f, 1.0f );
    DXWARN( pD3DDevice->SetVertexShaderConstant( VSH_CONST_MAX, &vConst.X, 1 ) );

    vConst.Set( 0.0f, 0.0f, 0.0f, 0.0f );
    DXWARN( pD3DDevice->SetVertexShaderConstant( VSH_CONST_MIN, &vConst.X, 1 ) );

    matrix4 LightDirMatrix, LightColMatrix;
    BuildLightingMatrix( &LightDirMatrix, &LightColMatrix );
    DXWARN( pD3DDevice->SetVertexShaderConstant( VSH_M_LIGHTDIR_1, &LightDirMatrix, 4 ) );
    DXWARN( pD3DDevice->SetPixelShaderConstant( 0, &LightDirMatrix.M[2], 1 ) );
    DXWARN( pD3DDevice->SetVertexShaderConstant( VSH_M_LIGHTCOL_1, &LightColMatrix, 4 ) );

    // fill the L2W transformation matrices
    D3DXMATRIX matWorld;
    pD3DDevice->GetTransform( D3DTS_WORLD, &matWorld );
    D3DXMatrixTranspose( &matWorld, &matWorld );
    DXWARN( pD3DDevice->SetVertexShaderConstant( VSH_M_MOVE_1, &matWorld, 4 ) );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateTexture( LPDIRECT3DDEVICE8 pD3DDevice, s32 CurrentMeshID, s32 SubMeshID, s32 *ColorOperations )
{
    // Let VRAM know that this texture is needed.
    if( m_pTextures )
    {
        ActivateTexture_Owned( pD3DDevice, CurrentMeshID, SubMeshID, ColorOperations );
    }
    else if( m_pTexturePtrs )
    {
        ActivateTexture_Pointed( pD3DDevice, CurrentMeshID, SubMeshID, ColorOperations );
    }
    else
    {
        VRAM_XBOX_Deactivate( 0 );
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateTexture_Owned( LPDIRECT3DDEVICE8 pD3DDevice, s32 CurrentMeshID, s32 SubMeshID, s32 *ColorOperations )
{
    if( m_MeshTextureArray[ CurrentMeshID ] == NULL )
    {
        //VRAM_XBOX_Deactivate( 0 );
        VRAM_XBOX_Deactivate( 1 );
        VRAM_XBOX_Deactivate( 2 );
        VRAM_XBOX_Deactivate( 3 );
        VRAM_XBOX_Activate( m_pTextures[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ], 0 );
    }

    if( m_pSubMeshes[ SubMeshID ].TextureID[ 1 ] != -1 )
    {
        if (ColorOperations)
            DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperations[ 1 ] ) );
        VRAM_XBOX_Activate( m_pTextures[ m_pSubMeshes[ SubMeshID ].TextureID[ 1 ] ], 1 );
    }

    if( m_pSubMeshes[ SubMeshID ].TextureID[ 2 ] != -1 )
        VRAM_XBOX_Activate( m_pTextures[ m_pSubMeshes[ SubMeshID ].TextureID[ 2 ] ], 2 );

    if (m_pMaterials && m_pMatStages)
    {
        s32 mat_loop;
        for (mat_loop=0; mat_loop<m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].nStages; ++mat_loop)
        {
            VRAM_XBOX_Activate( m_pTextures[ m_pMatStages[ m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].iStages[ mat_loop ] ].iTexture ], mat_loop );
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateTexture_Pointed( LPDIRECT3DDEVICE8 pD3DDevice, s32 CurrentMeshID, s32 SubMeshID, s32 *ColorOperations )
{
    if (m_pMaterials && m_pMatStages)
    {
        if (m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].nStages > 1)
        {
            VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pMatStages[ m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].iStages[ 0 ] ].iTexture ], 0 );

            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperations[ 1 ] ));
            VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pMatStages[ m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].iStages[ 1 ] ].iTexture ], 1 );
        }
        else 
        {
            if( m_MeshTextureArray[ CurrentMeshID ] == NULL )
                VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pMatStages[ m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].iStages[ 0 ] ].iTexture ], 0 );

            if( m_pSubMeshes[ SubMeshID ].TextureID[ 1 ] != -1 )
            {
                if (ColorOperations)
                    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperations[ 1 ] ));
                VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pSubMeshes[ SubMeshID ].TextureID[ 1 ] ], 1 );
            }

            if( m_pSubMeshes[ SubMeshID ].TextureID[ 2 ] != -1 )
                VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pSubMeshes[ SubMeshID ].TextureID[ 2 ] ], 2 );
        }
/*
        s32 mat_loop;
        for (mat_loop=0; mat_loop<m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].nStages; ++mat_loop)
        {
            DXWARN(pD3DDevice->SetTextureStageState( mat_loop, D3DTSS_COLOROP, ColorOperations[ mat_loop ] ));
            VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pMatStages[ m_pMaterials[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ].iStages[ mat_loop ] ].iTexture ], mat_loop );
        }

        if( m_pSubMeshes[ SubMeshID ].TextureID[ 2 ] != -1 )
            VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pSubMeshes[ SubMeshID ].TextureID[ 2 ] ], 2 );
*/
    }
    else
    {
        if( m_MeshTextureArray[ CurrentMeshID ] == NULL )
            VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pSubMeshes[ SubMeshID ].TextureID[ 0 ] ], 0 );

        if( m_pSubMeshes[ SubMeshID ].TextureID[ 1 ] != -1 )
        {
            DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperations[ 1 ] ));
            VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pSubMeshes[ SubMeshID ].TextureID[ 1 ] ], 1 );
        }

        if( m_pSubMeshes[ SubMeshID ].TextureID[ 2 ] != -1 )
            VRAM_XBOX_Activate( *m_pTexturePtrs[ m_pSubMeshes[ SubMeshID ].TextureID[ 2 ] ], 2 );
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
static f32 CalcScaleMagnitude( matrix4& L2W )
{
    s32     LargestScaleAxis;
    f32     Scale[3];

    //---   calculate the scale on each axis...w/o the square root
    Scale[0] = L2W.M[0][0] * L2W.M[0][0] + L2W.M[0][1] * L2W.M[0][1] + L2W.M[0][2] * L2W.M[0][2];
    Scale[1] = L2W.M[1][0] * L2W.M[1][0] + L2W.M[1][1] * L2W.M[1][1] + L2W.M[1][2] * L2W.M[1][2];
    Scale[2] = L2W.M[2][0] * L2W.M[2][0] + L2W.M[2][1] * L2W.M[2][1] + L2W.M[2][2] * L2W.M[2][2];

    //---   now we can figure out which axis has the largest scale
    LargestScaleAxis = 0;
    if( Scale[1] > Scale[LargestScaleAxis] )
        LargestScaleAxis = 1;
    if( Scale[2] > Scale[LargestScaleAxis] )
        LargestScaleAxis = 2;

    //---   apply the square root to get the proper magnitude
    return x_sqrt( Scale[LargestScaleAxis] );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::PreRender( matrix4* pShadowMatrix )
{

    view*   pView;

    // Gather info for visibility checks
    pView = ENG_GetActiveView();

    // setup the fast clip data for SubMeshInView function
    pView->GetFrustum( m_FastClipPack.FX, m_FastClipPack.FY );
    pView->GetW2VMatrix( m_FastClipPack.W2V );
    m_XBOXFastClipW2V[0].a = m_FastClipPack.W2V.M[0][0];
    m_XBOXFastClipW2V[0].b = m_FastClipPack.W2V.M[1][0];
    m_XBOXFastClipW2V[0].c = m_FastClipPack.W2V.M[2][0];
    m_XBOXFastClipW2V[0].d = m_FastClipPack.W2V.M[3][0];
    m_XBOXFastClipW2V[1].a = m_FastClipPack.W2V.M[0][1];
    m_XBOXFastClipW2V[1].b = m_FastClipPack.W2V.M[1][1];
    m_XBOXFastClipW2V[1].c = m_FastClipPack.W2V.M[2][1];
    m_XBOXFastClipW2V[1].d = m_FastClipPack.W2V.M[3][1];
    m_XBOXFastClipW2V[2].a = m_FastClipPack.W2V.M[0][2];
    m_XBOXFastClipW2V[2].b = m_FastClipPack.W2V.M[1][2];
    m_XBOXFastClipW2V[2].c = m_FastClipPack.W2V.M[2][2];
    m_XBOXFastClipW2V[2].d = m_FastClipPack.W2V.M[3][2];
    pView->GetZLimits( m_FastClipPack.NZ, m_FastClipPack.FZ );
    pView->GetViewPlanes( view::V_WORLD, m_FastClipPack.PlaneN[3], m_FastClipPack.PlaneD[3],
                                         m_FastClipPack.PlaneN[4], m_FastClipPack.PlaneD[4],
                                         m_FastClipPack.PlaneN[1], m_FastClipPack.PlaneD[1],
                                         m_FastClipPack.PlaneN[2], m_FastClipPack.PlaneD[2],
                                         m_FastClipPack.PlaneN[0], m_FastClipPack.PlaneD[0],
                                         m_FastClipPack.PlaneN[5], m_FastClipPack.PlaneD[5]);

    m_Location = m_L2W.GetTranslation();



    // Determine the scale radius
    m_RadiusScale = CalcScaleMagnitude( m_L2W );

//    D3DXMATRIX L2V;
    matrix4 Q_L2V;
//    matrix4 Q_W2V;
//    ENG_GetActiveView()->GetW2VMatrix( Q_W2V );

    // Apply shadow matrix if supplied
    if( pShadowMatrix != NULL )
    {
        // CPU multiply:
        Q_L2V = (*pShadowMatrix) * m_L2W;
//        Q_L2V = Q_W2V * Q_L2V;
        XBOX_GetDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX *)&Q_L2V );
    }
    else
        XBOX_GetDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m_L2W.M );

}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::RenderMesh( s32 MeshID, xbool SkipVisCheck )
{
    ASSERT( ENG_GetRenderMode() );

    // Prepare for Rendering.
    PreRender();

    // Render the specific mesh.
    if( m_pMeshes[MeshID].Flags & MESH_FLAG_VISIBLE || SkipVisCheck )
        MeshRender( MeshID );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::Render( xbool SkipVisCheck )
{
    s32 MeshID;

    // Render the mesh
    ASSERT( ENG_GetRenderMode() );

    // Pre-Render Phase
    PreRender();

    // Render all of the meshes
    for ( MeshID = 0; MeshID < m_NMeshes; MeshID++ )
    {
        // Try to trivially reject the mesh.
        if( m_pMeshes[MeshID].Flags & MESH_FLAG_VISIBLE || SkipVisCheck )
            MeshRender( MeshID );
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
static vector4 Brightness( 0.87f, 0.87f, 0.87f, 1.0f );
static vector4 Contrast( 0.5f, 0.5f, 0.5f, 1.0f );

void QGeom::MeshRender( s32 MeshID )
{
    s32         SubMeshID;
    s32         CurrentMeshID = -1;
    s32         ColorOperation0;
    s32         AlphaOperation0;
    s32         ColorOperation1;
    s32         AlphaOperation1;
    s32         ColorOperation2;
    s32         AlphaOperation2;
    u32         CurrentRenderFlags;
    u32         CurrentBlendMode;
    u8          CurrentFixedAlpha;
    s32         TextureCount;
    xbool       bMeshHasVertexColor;
    t_GeomMesh* pMesh = &m_pMeshes[MeshID];
    view*       pView = ENG_GetActiveView();
    s32         TotalSubMeshes;
    xbool       bUseDynamicLighting;
    xbool       bSimpleDiffusePSActive = FALSE;

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    if( !pView->SphereInView( view::V_WORLD, m_Location, pMesh->Radius * m_RadiusScale ) )
        return;

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );

    // Determine if there is are any textures in this mesh.
    TextureCount = GetNTextures();

    // Determine if this mesh is using vertex colors.
    if( pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1 || 
        pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1 ||
        pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3 )
    {
        bMeshHasVertexColor = TRUE;
    }

    // Determine if this is a multitextured object (not environment mapped though)
    s_bIsMultiTextured = ((pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3)
                        || (pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1)
                        || (pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1));

    // Determine the color and alpha blending types.
    // Default the 2nd texture stage to disabled.
    ColorOperation1 = D3DTOP_DISABLE;
    AlphaOperation1 = D3DTOP_DISABLE;
    ColorOperation2 = D3DTOP_DISABLE;
    AlphaOperation2 = D3DTOP_DISABLE;

    if( s_bIsMultiTextured )
    {
        VRAM_XBOX_SetWrapMode( WRAP_MODE_CLAMP, WRAP_MODE_CLAMP, 1 );
    }
    if( CurrentRenderFlags & ENG_ALPHA_BLEND_ON )
    {
        ENG_GetBlendMode( CurrentBlendMode, CurrentFixedAlpha );

        // Test to see if a fixed alpha blending mode is to be used.
        if( CurrentBlendMode & ENG_BLEND_FIXED_ALPHA )
        {
            // If this draw mode is using a texture, then set the color arg to the texture, otherwise, just to the
            // diffuse color.
            if( TextureCount && bMeshHasVertexColor )
            {
                ColorOperation0 = D3DTOP_MODULATE;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));

                AlphaOperation0 = D3DTOP_MODULATE;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ));

                ColorOperation1 = D3DTOP_SELECTARG1;
                DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT ));

                AlphaOperation1 = D3DTOP_BLENDFACTORALPHA;
                DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT ));
                DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ));
            }
            else if( TextureCount )
            {
                ColorOperation0 = D3DTOP_MODULATE;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR ));

                AlphaOperation0 = D3DTOP_MODULATE;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ));
            }
            else if( bMeshHasVertexColor )
            {
                ColorOperation0 = D3DTOP_SELECTARG1;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE ));

                AlphaOperation0 = D3DTOP_BLENDFACTORALPHA;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ));
            }
            else
            {
                ColorOperation0 = D3DTOP_DISABLE;
                AlphaOperation0 = D3DTOP_DISABLE;
            }
        }
        else
        {
            if( TextureCount && bMeshHasVertexColor )
            {
                ColorOperation0 = D3DTOP_MODULATE;
                AlphaOperation0 = D3DTOP_MODULATE;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));

                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));
            }
            else if( TextureCount )
            {
                ColorOperation0 = D3DTOP_SELECTARG1;
                AlphaOperation0 = D3DTOP_SELECTARG1;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
            }
            else if( bMeshHasVertexColor )
            {
                ColorOperation0 = D3DTOP_SELECTARG2;
                AlphaOperation0 = D3DTOP_SELECTARG2;
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));
            }
            else
            {
                ColorOperation0 = D3DTOP_DISABLE;
                AlphaOperation0 = D3DTOP_DISABLE;
            }
        }
    }
    else
    {
        // Disable alpha blending altogether.
        ColorOperation0 = D3DTOP_MODULATE;
        AlphaOperation0 = D3DTOP_DISABLE;

        // Set the colors of the pixel to be a combination of the texture and vertex color. 
        DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
        DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));
    }


    // Set the color and alpha operations that were just determined.
    DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOperation0 ));
    DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, AlphaOperation0 ));
    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperation1 ));
    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, AlphaOperation1 ));
    DXWARN(pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, ColorOperation2 ));
    DXWARN(pD3DDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, AlphaOperation2 ));

    // Determine if dynamic lighting should be used.
    bUseDynamicLighting = (pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING) > 0;

//    if(    pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1 
//        || pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1)
//		bUseDynamicLighting = FALSE;

    if( bUseDynamicLighting )
    {
        if( s_bIsMultiTextured )
        {
            ActivateVertexShaderMultiTex();
            ActivatePixelShader( kPSSimpleDiffuseNoLight );
            bSimpleDiffusePSActive = TRUE;
//                    DXWARN( pD3DDevice->SetPixelShaderConstant( 3, &Brightness, 1 ) );
//                    DXWARN( pD3DDevice->SetPixelShaderConstant( 2, &Contrast, 1 ) );
        }
        else
        {
            ActivateVertexShader();
            ActivatePixelShader( kPSSimpleDiffuseNoLight );
            bSimpleDiffusePSActive = TRUE;
        }

        // setup the base constant register
        vector4 vConst;
        vConst.Set(3.0f, 0.5f, -0.5f, 1.0f);
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_1, &vConst.X, 1) );

        vConst.Set(1.0f, 1.0f, 1.0f, 1.0f);
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_MAX, &vConst.X, 1) );

        vConst.Set(0.0f, 0.0f, 0.0f, 0.0f);
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_MIN, &vConst.X, 1) );

        matrix4 LightDirMatrix, LightColMatrix;
        BuildLightingMatrix( &LightDirMatrix, &LightColMatrix );
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_LIGHTDIR_1, &LightDirMatrix, 4) );
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_LIGHTCOL_1, &LightColMatrix, 4) );

        // fill the L2W transformation matrices
        D3DXMATRIX matWorld;
        pD3DDevice->GetTransform( D3DTS_WORLD, &matWorld );
        D3DXMatrixTranspose( &matWorld, &matWorld );
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_MOVE_1, &matWorld, 4) );

        // fill the projection matrix in constant registers
        D3DXMATRIX matView, matProj, matViewProj, matViewProjTranspose;
        pD3DDevice->GetTransform( D3DTS_VIEW,       &matView );
        pD3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
        D3DXMatrixMultiply( &matViewProj, &matView, &matProj );
        D3DXMatrixTranspose( &matViewProjTranspose, &matViewProj );
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_PROJECT_1, &matViewProjTranspose, 4) );

    }
    else
    {
        XBOX_ActivateVertexShader( pMesh->VertFormat );
        ActivatePixelShader( kPSSimpleDiffuseNoLight );
        bSimpleDiffusePSActive = TRUE;
    }

    // Set the source streams for DirectX
    DXWARN( pD3DDevice->SetStreamSource( 0, pMesh->pD3DVerts, pMesh->VertFormatSize ));
    DXWARN( pD3DDevice->SetIndices( pMesh->pD3DIndices, 0 ));

    // check for a over ride texture and set if there
    if( m_MeshTextureArray[MeshID] != NULL )
        VRAM_XBOX_Activate( *m_MeshTextureArray[MeshID], 0 );

    // Loop through the sub-meshes in this mesh.
    TotalSubMeshes = (pMesh->FirstSubMesh + pMesh->NSubMeshes);
    s32 NewRenderFlags;
    for( SubMeshID = pMesh->FirstSubMesh; SubMeshID < TotalSubMeshes; SubMeshID++ )
    {
        CurrentMeshID = m_pSubMeshes[SubMeshID].MeshID;

        if( m_pMeshes[ CurrentMeshID ].Flags & MESH_FLAG_SPECIAL ) continue;

        NewRenderFlags = CurrentRenderFlags;
        if( m_pSubMeshes[SubMeshID].Flags & SUBMESH_FLAG_ALPHA_P )
        {
            // turn the alpha test mode on
            NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
            NewRenderFlags |= ENG_ALPHA_TEST_ON;
        }
        else if( m_pSubMeshes[SubMeshID].Flags & SUBMESH_FLAG_ALPHA_T )
        {
            // turn the alpha blend mode on
            NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
            NewRenderFlags |= ENG_ALPHA_BLEND_ON;
        }

        // special submeshes get a special render "somewhere else"

//        ENG_SetRenderFlags( CurrentRenderFlags ); 
        // If there are textures used for this submesh, set the texture stage accordingly.
        
        if( m_pSubMeshes[SubMeshID].TextureID >= 0 )
        {
            ActivateTexture( pD3DDevice, CurrentMeshID, SubMeshID, NULL );
        }


        // Set shadowbuffer state
        pD3DDevice->SetRenderState( D3DRS_SHADOWFUNC, D3DCMP_GREATEREQUAL );
        // Render the Submesh
        if( bSimpleDiffusePSActive ) 
            ActivatePixelShader( kPSSimpleDiffuseNoLight );
        pD3DDevice->DrawIndexedPrimitive( (_D3DPRIMITIVETYPE)m_pSubMeshes[SubMeshID].PrimType,
                                           m_pSubMeshes[SubMeshID].IndexMin, 
                                           m_pSubMeshes[SubMeshID].IndexCount,
                                           m_pSubMeshes[SubMeshID].IndexStart,
                                           m_pSubMeshes[SubMeshID].NTris );


        // Set shadowbuffer state
        pD3DDevice->SetRenderState( D3DRS_SHADOWFUNC, D3DCMP_NEVER );

        if( m_pSubMeshes[SubMeshID].Flags ) ENG_SetRenderFlags( CurrentRenderFlags );  // reset display mode
    }

    if( bSimpleDiffusePSActive )
    {
        DeActivatePixelShader( kPSSimpleDiffuseNoLight );
        bSimpleDiffusePSActive = FALSE;
    }
    // turn lighting back off since most stuff doesn't want it
    DXWARN( pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE) );
    // turn fogging off, too.
    DXWARN( pD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE ) );
}

static void DeactivateAllPixelShaders( void )
{
    for (s32 i=0; i<kPixelShaderCount; ++i)
    {
        if (s_bPSActive[ i ])
        {
            if (s_i_PS_ID[ i ] != -1)
                XBOX_DeActivatePixelShader( s_i_PS_ID[ i ] );
            s_bPSActive[ i ] = FALSE;
        }
    }
}

s32 QGeom::RenderSubMeshes ( xbool* SubMeshes, xbool SkipVisCheck, s32 SubMeshFlags, xbool SkipInViewCheck )
{
    ASSERT( ENG_GetRenderMode() );

    // Prepare for Rendering.
    PreRender();

    s32         i;
    s32         SubMeshID;
    s32         CurrentMeshID = -1;
    s32         ColorOperations[ 4 ];
    s32         AlphaOperations[ 4 ];
    u32         CurrentRenderFlags;
    s32         NewRenderFlags;
    u32         CurrentBlendMode;
    u32         NewBlendMode;
    u8          CurrentFixedAlpha;
    u8          NewFixedAlpha;
    s32         TextureCount;
    xbool       bMeshHasVertexColor;
    t_GeomMesh* pMesh;
    view*       pView = ENG_GetActiveView();
    s32         TotalSubMeshes;
    xbool       bUseDynamicLighting;
    s32         NSubMeshesRendered = 0;
    static s32  SPreviousMaterialID = -1;

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );
    ENG_GetBlendMode( CurrentBlendMode, CurrentFixedAlpha );

    // Determine if there is are any textures in this mesh.
    TextureCount = GetNTextures();


    // loop through all the submeshes and see if the SubMeshes array has it turned on
    for( SubMeshID = 0; SubMeshID < m_NSubMeshes; SubMeshID++ )
    {
        s_bUseEnvMapping = FALSE;
        // make sure this submesh is in the display list
        if( SubMeshes[ SubMeshID ] == FALSE ) continue;
        if( SubMeshFlags == -1 ) SubMeshes[ SubMeshID ] = FALSE;

        if( SubMeshFlags == SUBMESH_FLAG_NONE && m_pSubMeshes[SubMeshID].Flags != SUBMESH_FLAG_NONE ) continue;
        if( SubMeshFlags > SUBMESH_FLAG_NONE && !(m_pSubMeshes[SubMeshID].Flags & SubMeshFlags) ) continue;

        // RENDER THE SUBMESH
        // check to see if this submesh is within the view and skip if not
        if( !SkipVisCheck && !SkipInViewCheck )
            if( !SubMeshInView( &m_pSubMeshes[SubMeshID] ) ) continue;

        SubMeshes[SubMeshID] = TRUE;
        NSubMeshesRendered++;

        // special submeshes get a special render "somewhere else"

        s32 iMat = m_pSubMeshes[ SubMeshID ].TextureID[ 0 ];
        if (   ((m_pMaterials && m_pMatStages) && iMat != SPreviousMaterialID)
            || (m_pSubMeshes[ SubMeshID ].MeshID != CurrentMeshID))
        {
            s32 nStages = 1;
            if (m_pMaterials && m_pMatStages)
            {
                if (iMat != SPreviousMaterialID)
                    SPreviousMaterialID = iMat;
                nStages = m_pMaterials[ iMat ].nStages;
            }

            CurrentMeshID = m_pSubMeshes[SubMeshID].MeshID;
            pMesh = &m_pMeshes[ CurrentMeshID ];

            if( pMesh->Flags & MESH_FLAG_SPECIAL ) continue;

            if( pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1 || 
                pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1 ||
                pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3 )
            {
                bMeshHasVertexColor = TRUE;
            }
            bUseDynamicLighting = ((pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING) > 0);
            s_bUseEnvMapping = (pMesh->Flags & MESH_FLAG_ENVMAPPED) > 0;
            s_bIsMultiTextured = ((pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3)
                                || (pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1)
                                || (pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1));
            if( s_bIsMultiTextured )
                VRAM_XBOX_SetWrapMode( WRAP_MODE_CLAMP, WRAP_MODE_CLAMP, 1 );

            // Determine the color and alpha blending types.
            // Default the 2nd texture stage to disabled.
            ColorOperations[ 1 ] = D3DTOP_DISABLE;
            AlphaOperations[ 1 ] = D3DTOP_DISABLE;
            ColorOperations[ 2 ] = D3DTOP_DISABLE;
            AlphaOperations[ 2 ] = D3DTOP_DISABLE;
            ColorOperations[ 3 ] = D3DTOP_DISABLE;
            AlphaOperations[ 3 ] = D3DTOP_DISABLE;


            if( CurrentRenderFlags & ENG_ALPHA_BLEND_ON || CurrentRenderFlags & ENG_ALPHA_TEST_ON)
            {
                SetBlendSettings_FixedAlpha( pD3DDevice, 
                                             CurrentBlendMode, 
                                             CurrentFixedAlpha,
                                             TextureCount, 
                                             &ColorOperations[ 0 ],
                                             &AlphaOperations[ 0 ],
                                             m_pSubMeshes[ SubMeshID ].TextureID[ 0 ],
                                             bMeshHasVertexColor );
            }
            else
            {
                // Disable alpha blending altogether.
                ColorOperations[ 0 ] = D3DTOP_MODULATE;
                AlphaOperations[ 0 ] = D3DTOP_DISABLE;

                // Set the colors of the pixel to be a combination of the texture and vertex color. 
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));
            }


            // Set the color and alpha operations that were just determined.
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOperations[ 0 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, AlphaOperations[ 0 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperations[ 1 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, AlphaOperations[ 1 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, ColorOperations[ 2 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, AlphaOperations[ 2 ] ) );

            if (nStages > 1)
            {
                s32 iPS = kPS2DiffuseID;
                if (XBOX_IsToonCheatActive())
                    iPS += kToonCheatOffset;
                if (s_i_VS_ID[ kVS2DiffuseID ] == -1)
                    s_i_VS_ID[ kVS2DiffuseID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVS2DiffuseID ], s_GEOM_VertexFormatMultiTex, 0 );
                XBOX_ActivateVertexShader( s_i_VS_ID[ kVS2DiffuseID ] );

                if (s_i_PS_ID[ iPS ] == -1)
                    s_i_PS_ID[ iPS ] = XBOX_RegisterPixelShader( s_i_PS_Addresses[ iPS ] );
                if (!s_bPSActive[ iPS ])  XBOX_ActivatePixelShader( s_i_PS_ID[ iPS ] );
                s_bPSActive[ iPS ] = TRUE;
                if (bUseDynamicLighting)
                {
                    vector4 vConst;
                    // setup the base constant register
                    vConst.Set(3.0f, 0.5f, -0.5f, 1.0f);
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_1, &vConst.X, 1) );

                    // fill the projection matrix in constant registers
                    D3DXMATRIX matView, matProj, matViewProj, matViewProjTranspose;
                    pD3DDevice->GetTransform( D3DTS_VIEW,       &matView );
                    pD3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
                    D3DXMatrixMultiply( &matViewProj, &matView, &matProj );
                    D3DXMatrixTranspose( &matViewProjTranspose, &matViewProj );
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_PROJECT_1, &matViewProjTranspose, 4) );
                }
                VRAM_XBOX_SetWrapMode( WRAP_MODE_TILE, WRAP_MODE_TILE, 1 );
            }
            else if( bUseDynamicLighting )
            {
                ActivateShaders_DynamicLighting( pD3DDevice, pView, SubMeshID );
            }
            else if (s_bUseEnvMapping)
            {
                DeactivateAllPixelShaders();

//                if (s_i_VS_ID[ kVSEnvMapID ] == -1)
//                    s_i_VS_ID[ kVSEnvMapID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVSEnvMapID ], s_GEOM_VertexFormatMultiTex, 0 );
//                XBOX_ActivateVertexShader( s_i_VS_ID[ kVSEnvMapID ] );
                XBOX_ActivateVertexShader( pMesh->VertFormat );
            }
            else
            {
                s32 iPS = kPSMultiTexID;
                if (XBOX_IsToonCheatActive())
                    iPS += kToonCheatOffset;
                if (s_i_VS_ID[ kVSGenericID ] == -1)
                    s_i_VS_ID[ kVSGenericID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVSGenericID ], s_GEOM_VertexFormatMultiTex, 0 );
                XBOX_ActivateVertexShader( s_i_VS_ID[ kVSGenericID ] );

                if( s_i_PS_ID[ iPS ] == -1 )
                    s_i_PS_ID[ iPS ] = XBOX_RegisterPixelShader( s_i_PS_Addresses[ iPS ] );
                if (!s_bPSActive[ iPS ])  XBOX_ActivatePixelShader( s_i_PS_ID[ iPS ] );

                s_bPSActive[ iPS ] = TRUE;
            }
            // Set the source streams for DirectX
            DXWARN( pD3DDevice->SetStreamSource( 0, pMesh->pD3DVerts, pMesh->VertFormatSize ));
            DXWARN( pD3DDevice->SetIndices( pMesh->pD3DIndices, 0 ));

            // check for a over ride texture and set if there
            if( m_MeshTextureArray[CurrentMeshID] != NULL )
            {
                VRAM_XBOX_Activate( *m_MeshTextureArray[CurrentMeshID], 0 );
            }

            // Loop through the sub-meshes in this mesh.
            TotalSubMeshes = (pMesh->FirstSubMesh + pMesh->NSubMeshes);
        }  // done setting up the new pMesh settings


        if( !(m_pMeshes[CurrentMeshID].Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck ) continue;

        NewRenderFlags = CurrentRenderFlags;
        NewBlendMode = CurrentBlendMode;
        NewFixedAlpha = CurrentFixedAlpha;

        if( m_pSubMeshes[SubMeshID].Flags )
        {
            NewRenderFlags = CurrentRenderFlags;
            if( m_pSubMeshes[SubMeshID].Flags & SUBMESH_FLAG_ALPHA_P )
            {
                // turn the alpha test mode on
                NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                NewRenderFlags |= ENG_ALPHA_TEST_ON;
            }
            if( m_pSubMeshes[SubMeshID].Flags & SUBMESH_FLAG_ALPHA_T )
            {
                // turn the alpha blend mode on
				NewRenderFlags &= ~ENG_ZBUFFER_FILL_ON;
				NewRenderFlags |=  ENG_ZBUFFER_FILL_OFF;
                NewRenderFlags &= ~ENG_ALPHA_TEST_ON;
                NewRenderFlags |= ENG_ALPHA_TEST_OFF;
                NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                NewRenderFlags |= ENG_ALPHA_BLEND_ON;
                NewBlendMode = ENG_BLEND_NORMAL | ENG_BLEND_FIXED_ALPHA;
                NewFixedAlpha = 255;
            }
        }
        if (NewRenderFlags != CurrentRenderFlags)
            ENG_SetRenderFlags( NewRenderFlags );
        if (NewBlendMode != CurrentBlendMode ||
            ((NewBlendMode & ENG_BLEND_FIXED_ALPHA) && (NewFixedAlpha != CurrentFixedAlpha)))
            ENG_SetBlendMode( NewBlendMode, NewFixedAlpha );

        // If there are textures used for this submesh, set the texture stage accordingly.
        if( m_pSubMeshes[ SubMeshID ].TextureID >= 0 )
            ActivateTexture( pD3DDevice, CurrentMeshID, SubMeshID, &ColorOperations[ 0 ] );

        // Set shadowbuffer state
        pD3DDevice->SetRenderState( D3DRS_SHADOWFUNC, D3DCMP_GREATEREQUAL );
        // Render the Submesh
        pD3DDevice->DrawIndexedPrimitive( (_D3DPRIMITIVETYPE)m_pSubMeshes[SubMeshID].PrimType,
                                           m_pSubMeshes[SubMeshID].IndexMin, 
                                           m_pSubMeshes[SubMeshID].IndexCount,
                                           m_pSubMeshes[SubMeshID].IndexStart,
                                           m_pSubMeshes[SubMeshID].NTris );

        // Set shadowbuffer state
        pD3DDevice->SetRenderState( D3DRS_SHADOWFUNC, D3DCMP_NEVER );

        /*
        if( m_pSubMeshes[SubMeshID].Flags ) 
        {
            ENG_SetRenderFlags( CurrentRenderFlags );  // reset display mode
        }
        else if (NewRenderFlags != CurrentRenderFlags)
        {
            ENG_SetRenderFlags( CurrentRenderFlags );
        }
        if( m_pSubMeshes[SubMeshID].Flags ) 
        {
            ENG_SetBlendMode( CurrentBlendMode, CurrentFixedAlpha );
        }
        else if (   NewBlendMode != CurrentBlendMode
                 || NewFixedAlpha != CurrentFixedAlpha)
        {
            ENG_SetBlendMode( CurrentBlendMode, CurrentFixedAlpha );
        }
        */
    }

    DeactivateAllPixelShaders();

    // turn lighting back off since most stuff doesn't want it
    DXWARN( pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE) );
    DXWARN( pD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE ) );
    return NSubMeshesRendered;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::RenderSubMeshAtPos( SGeomSubMesh *pSubMesh, s32 iSubMeshIndex, vector3 &pos )
{
    ASSERT( ENG_GetRenderMode() );

    // Prepare for Rendering.
//    PreRender();
    {
        view*   pView;

        // Gather info for visibility checks
        pView = ENG_GetActiveView();

        // setup the fast clip data for SubMeshInView function
        pView->GetFrustum( m_FastClipPack.FX, m_FastClipPack.FY );
        pView->GetW2VMatrix( m_FastClipPack.W2V );
        m_XBOXFastClipW2V[0].a = m_FastClipPack.W2V.M[0][0];
        m_XBOXFastClipW2V[0].b = m_FastClipPack.W2V.M[1][0];
        m_XBOXFastClipW2V[0].c = m_FastClipPack.W2V.M[2][0];
        m_XBOXFastClipW2V[0].d = m_FastClipPack.W2V.M[3][0];
        m_XBOXFastClipW2V[1].a = m_FastClipPack.W2V.M[0][1];
        m_XBOXFastClipW2V[1].b = m_FastClipPack.W2V.M[1][1];
        m_XBOXFastClipW2V[1].c = m_FastClipPack.W2V.M[2][1];
        m_XBOXFastClipW2V[1].d = m_FastClipPack.W2V.M[3][1];
        m_XBOXFastClipW2V[2].a = m_FastClipPack.W2V.M[0][2];
        m_XBOXFastClipW2V[2].b = m_FastClipPack.W2V.M[1][2];
        m_XBOXFastClipW2V[2].c = m_FastClipPack.W2V.M[2][2];
        m_XBOXFastClipW2V[2].d = m_FastClipPack.W2V.M[3][2];
        pView->GetZLimits( m_FastClipPack.NZ, m_FastClipPack.FZ );
        pView->GetViewPlanes( view::V_WORLD, m_FastClipPack.PlaneN[3], m_FastClipPack.PlaneD[3],
                                            m_FastClipPack.PlaneN[4], m_FastClipPack.PlaneD[4],
                                            m_FastClipPack.PlaneN[1], m_FastClipPack.PlaneD[1],
                                            m_FastClipPack.PlaneN[2], m_FastClipPack.PlaneD[2],
                                            m_FastClipPack.PlaneN[0], m_FastClipPack.PlaneD[0],
                                            m_FastClipPack.PlaneN[5], m_FastClipPack.PlaneD[5]);

        m_Location = m_L2W.GetTranslation();



        // Determine the scale radius
        m_RadiusScale = CalcScaleMagnitude( m_L2W );

        matrix4 L2W = m_L2W;
        L2W.Translate( pos );

        XBOX_GetDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W.M );
    }

    s32         i;
    s32         CurrentMeshID = -1;
    s32         ColorOperations[ 4 ];
    s32         AlphaOperations[ 4 ];
    u32         CurrentRenderFlags;
    u32         CurrentBlendMode;
    u8          CurrentFixedAlpha;
    s32         TextureCount;
    xbool       bMeshHasVertexColor;
    t_GeomMesh* pMesh;
    view*       pView = ENG_GetActiveView();
    s32         TotalSubMeshes;
    xbool       bUseDynamicLighting;
    s32         NewRenderFlags;
    s32         NSubMeshesRendered = 0;
    static s32  SPreviousMaterialID = -1;

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );

    // Determine if there is are any textures in this mesh.
    TextureCount = GetNTextures();


    // loop through all the submeshes and see if the SubMeshes array has it turned on
    {
        s_bUseEnvMapping = FALSE;

        s32 iMat = pSubMesh->TextureID[ 0 ];
        if (   ((m_pMaterials && m_pMatStages) && iMat != SPreviousMaterialID)
            || (pSubMesh->MeshID != CurrentMeshID))
        {
            s32 nStages = 1;
            if (m_pMaterials && m_pMatStages)
            {
                if (iMat != SPreviousMaterialID)
                    SPreviousMaterialID = iMat;
                nStages = m_pMaterials[ iMat ].nStages;
            }

            // check to see if the MeshID has changed and reset the settings for it
            for (i=0; i<kPixelShaderCount; ++i)
            {
                if (s_bPSActive[ i ])
                {
                    if (s_i_PS_ID[ i ] != -1)
                        XBOX_DeActivatePixelShader( s_i_PS_ID[ i ] );
                    s_bPSActive[ i ] = FALSE;
                }
            }

            CurrentMeshID = pSubMesh->MeshID;
            pMesh = &m_pMeshes[ CurrentMeshID ];

            if( pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1 || 
                pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1 ||
                pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3 )
            {
                bMeshHasVertexColor = TRUE;
            }
            bUseDynamicLighting = ((pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING) > 0);
            s_bUseEnvMapping = (pMesh->Flags & MESH_FLAG_ENVMAPPED) > 0;
            s_bIsMultiTextured = ((pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3)
                                || (pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1)
                                || (pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1));
            if( s_bIsMultiTextured )
                VRAM_XBOX_SetWrapMode( WRAP_MODE_CLAMP, WRAP_MODE_CLAMP, 1 );

            // Determine the color and alpha blending types.
            // Default the 2nd texture stage to disabled.
            ColorOperations[ 1 ] = D3DTOP_DISABLE;
            AlphaOperations[ 1 ] = D3DTOP_DISABLE;
            ColorOperations[ 2 ] = D3DTOP_DISABLE;
            AlphaOperations[ 2 ] = D3DTOP_DISABLE;
            ColorOperations[ 3 ] = D3DTOP_DISABLE;
            AlphaOperations[ 3 ] = D3DTOP_DISABLE;


            if( CurrentRenderFlags & ENG_ALPHA_BLEND_ON || CurrentRenderFlags & ENG_ALPHA_TEST_ON)
            {
                SetBlendSettings_FixedAlpha( pD3DDevice, 
                                             CurrentBlendMode, 
                                             CurrentFixedAlpha,
                                             TextureCount, 
                                             &ColorOperations[ 0 ],
                                             &AlphaOperations[ 0 ],
                                             pSubMesh->TextureID[ 0 ],
                                             bMeshHasVertexColor );
            }
            else
            {
                // Disable alpha blending altogether.
                ColorOperations[ 0 ] = D3DTOP_MODULATE;
                AlphaOperations[ 0 ] = D3DTOP_DISABLE;

                // Set the colors of the pixel to be a combination of the texture and vertex color. 
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
                DXWARN(pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));
            }


            // Set the color and alpha operations that were just determined.
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOperations[ 0 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, AlphaOperations[ 0 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperations[ 1 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, AlphaOperations[ 1 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, ColorOperations[ 2 ] ) );
            DXWARN( pD3DDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, AlphaOperations[ 2 ] ) );

            if (nStages > 1)
            {
                s32 iPS = kPSEnvMapID;
                if (XBOX_IsToonCheatActive())
                    iPS += kToonCheatOffset;
                if (s_i_VS_ID[ kVS2DiffuseID ] == -1)
                    s_i_VS_ID[ kVS2DiffuseID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVS2DiffuseID ], s_GEOM_VertexFormatMultiTex, 0 );
                XBOX_ActivateVertexShader( s_i_VS_ID[ kVS2DiffuseID ] );

                if (s_i_PS_ID[ iPS ] == -1)
                    s_i_PS_ID[ iPS ] = XBOX_RegisterPixelShader( s_i_PS_Addresses[ iPS ] );
                XBOX_ActivatePixelShader( s_i_PS_ID[ iPS ] );
                s_bPSActive[ iPS ] = TRUE;
                if (bUseDynamicLighting)
                {
                    vector4 vConst;
                    // setup the base constant register
                    vConst.Set(3.0f, 0.5f, -0.5f, 1.0f);
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_1, &vConst.X, 1) );

                    // fill the projection matrix in constant registers
                    D3DXMATRIX matView, matProj, matViewProj, matViewProjTranspose;
                    pD3DDevice->GetTransform( D3DTS_VIEW,       &matView );
                    D3DXMATRIX temp;
                    D3DXMatrixTranslation( &temp, pos.X, pos.Y, pos.Z );
                    D3DXMatrixMultiply( &matView, &temp, &matView );
                    pD3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
                    D3DXMatrixMultiply( &matViewProj, &matView, &matProj );
                    D3DXMatrixTranspose( &matViewProjTranspose, &matViewProj );
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_PROJECT_1, &matViewProjTranspose, 4) );
                }
                VRAM_XBOX_SetWrapMode( WRAP_MODE_TILE, WRAP_MODE_TILE, 1 );
            }
            else if( bUseDynamicLighting )
            {
                ActivateShaders_DynamicLighting( pD3DDevice, pView, pSubMesh->MeshID, &pos );
            }
            else if (s_bUseEnvMapping)
            {
//                if (s_i_VS_ID[ kVSEnvMapID ] == -1)
//                    s_i_VS_ID[ kVSEnvMapID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVSEnvMapID ], s_GEOM_VertexFormatMultiTex, 0 );
//                XBOX_ActivateVertexShader( s_i_VS_ID[ kVSEnvMapID ] );
                XBOX_ActivateVertexShader( pMesh->VertFormat );
            }
            else
            {
                s32 iPS = kPSEnvMapID;
                if (XBOX_IsToonCheatActive())
                    iPS += kToonCheatOffset;
                if (s_i_VS_ID[ kVSGenericID ] == -1)
                    s_i_VS_ID[ kVSGenericID ] = XBOX_RegisterVertexShader( s_i_VS_Addresses[ kVSGenericID ], s_GEOM_VertexFormatMultiTex, 0 );
                XBOX_ActivateVertexShader( s_i_VS_ID[ kVSGenericID ] );

                if( s_i_PS_ID[ iPS ] == -1 )
                    s_i_PS_ID[ iPS ] = XBOX_RegisterPixelShader( s_i_VS_Addresses[ iPS ] );
                XBOX_ActivatePixelShader( s_i_PS_ID[ iPS ] );

                s_bPSActive[ iPS ] = TRUE;
            }

            // Set the source streams for DirectX
            DXWARN( pD3DDevice->SetStreamSource( 0, pMesh->pD3DVerts, pMesh->VertFormatSize ));
            DXWARN( pD3DDevice->SetIndices( pMesh->pD3DIndices, 0 ));

            // check for a over ride texture and set if there
            if( m_MeshTextureArray[CurrentMeshID] != NULL )
            {
                VRAM_XBOX_Activate( *m_MeshTextureArray[CurrentMeshID], 0 );
            }

            // Loop through the sub-meshes in this mesh.
            TotalSubMeshes = (pMesh->FirstSubMesh + pMesh->NSubMeshes);
        }  // done setting up the new pMesh settings


        // RENDER THE SUBMESH
        // check to see if this submesh is within the view and skip if not
//        if( !SubMeshInView( pSubMesh ) ) return;

        NewRenderFlags = CurrentRenderFlags;
        if( pSubMesh->Flags )
        {
            NewRenderFlags = CurrentRenderFlags;
            if( pSubMesh->Flags & SUBMESH_FLAG_ALPHA_P )
            {
                // turn the alpha test mode on
                NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                NewRenderFlags |= ENG_ALPHA_TEST_ON;
            }
            if( pSubMesh->Flags & SUBMESH_FLAG_ALPHA_T )
            {
                // turn the alpha blend mode on
                NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                NewRenderFlags |= ENG_ALPHA_BLEND_ON;
            }
        }
        ENG_SetRenderFlags( NewRenderFlags );
        // If there are textures used for this submesh, set the texture stage accordingly.
        if( pSubMesh->TextureID >= 0 )
            ActivateTexture( pD3DDevice, pSubMesh->MeshID, iSubMeshIndex, &ColorOperations[ 0 ] );

        // Set shadowbuffer state
        pD3DDevice->SetRenderState( D3DRS_SHADOWFUNC, D3DCMP_GREATEREQUAL );
        // Render the Submesh
        pD3DDevice->DrawIndexedPrimitive( (_D3DPRIMITIVETYPE)pSubMesh->PrimType,
                                           pSubMesh->IndexMin, 
                                           pSubMesh->IndexCount,
                                           pSubMesh->IndexStart,
                                           pSubMesh->NTris );

        // Set shadowbuffer state
        pD3DDevice->SetRenderState( D3DRS_SHADOWFUNC, D3DCMP_NEVER );
        if( pSubMesh->Flags ) ENG_SetRenderFlags( CurrentRenderFlags );  // reset display mode
    }

    for (i=0; i<kPixelShaderCount; ++i)
    {
        if (s_bPSActive[ i ])
        {
            if (s_i_PS_ID[ i ] != -1)
                XBOX_DeActivatePixelShader( s_i_PS_ID[ i ] );
            s_bPSActive[ i ] = FALSE;
        }
    }

    // turn lighting back off since most stuff doesn't want it
    DXWARN( pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE) );
    DXWARN( pD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE ) );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::RenderMeshAsShadow( s32 MeshID, matrix4 ShadowMatrix, xbool SkipVisCheck )
{
//    return;
    s32         SubMeshID;
    s32         LastSubMeshID;
    t_GeomMesh* pMesh;
    view*       pView;
    vector3     Location;

    ASSERT( ENG_GetRenderMode() );
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    //--- get pointer to mesh
    pMesh = &m_pMeshes[MeshID];
    //ASSERT( pMesh->Flags & MESH_FLAG_SHADOW );
//    if( !(pMesh->Flags & MESH_FLAG_SHADOW) ) return;

    //--- perform trivial rejections first
    if( !(pMesh->Flags & MESH_FLAG_VISIBLE) )
        return;

    //--- get visibility info
    pView = ENG_GetActiveView();
    Location = m_L2W.GetTranslation();

    if( SkipVisCheck == FALSE )
    {
        f32 Radius = pMesh->Radius * CalcScaleMagnitude( m_L2W );

        if( !pView->SphereInView( view::V_WORLD, Location, Radius ) )
            return;
    }

    //--- Do pre-rendering setup
    PreRender( &ShadowMatrix );
    ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );

    u32         CurrentBlendMode;
    u8          CurrentFixedAlpha;
//    vector4     ShadowClr;

    ENG_GetBlendMode( CurrentBlendMode, CurrentFixedAlpha );
//    ShadowClr.Set( 0.0f, 0.0f, 0.0f, CurrentFixedAlpha/255.0f );
//    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_LIGHTCOL_1, &ShadowClr, 1) );

    XBOX_SetStencilMode( ENG_STENCIL_ON, CurrentFixedAlpha );
    ENG_SetBlendMode( ENG_BLEND_STENCIL );// | ENG_BLEND_FIXED_ALPHA, CurrentFixedAlpha>>1 );
//    XBOX_SetShadowBiasMode( ENG_SHADOW_BIAS_ON );

    // setup the base constant register
    vector4 vConst;
    vConst.Set(3.0f, 0.5f, -0.5f, 1.0f);
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_1, &vConst.X, 1) );

    vConst.Set(1.0f, 1.0f, 1.0f, 1.0f);
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_MAX, &vConst.X, 1) );

    vConst.Set(0.0f, 0.0f, 0.0f, 0.0f);
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_MIN, &vConst.X, 1) );

    // fill the L2W transformation matrices
    D3DXMATRIX matWorld;
    pD3DDevice->GetTransform( D3DTS_WORLD, &matWorld );
    D3DXMatrixTranspose( &matWorld, &matWorld );
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_MOVE_1, &matWorld, 4) );

    // fill the projection matrix in constant registers
    D3DXMATRIX matView, matProj, matViewProj, matViewProjTranspose;
    pD3DDevice->GetTransform( D3DTS_VIEW,       &matView );
    pD3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
    D3DXMatrixMultiply( &matViewProj, &matView, &matProj );
    D3DXMatrixTranspose( &matViewProjTranspose, &matViewProj );
    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_PROJECT_1, &matViewProjTranspose, 4) );

    // Set the source streams for DirectX
    DXWARN( pD3DDevice->SetStreamSource( 0, pMesh->pD3DVerts, pMesh->VertFormatSize ));
    DXWARN( pD3DDevice->SetIndices( pMesh->pD3DIndices, 0 ));

    ActivateShadowShader();

    // turn off texture and set black color

    LastSubMeshID = pMesh->FirstSubMesh + pMesh->NSubMeshes;

    //--- loop through the sub-meshes
    for( SubMeshID = pMesh->FirstSubMesh; SubMeshID < LastSubMeshID; SubMeshID++ )
    {
        //--- Draw the primitive
        pD3DDevice->DrawIndexedPrimitive( (_D3DPRIMITIVETYPE)m_pSubMeshes[SubMeshID].PrimType,
                                           m_pSubMeshes[SubMeshID].IndexMin, 
                                           m_pSubMeshes[SubMeshID].IndexCount,
                                           m_pSubMeshes[SubMeshID].IndexStart,
                                           m_pSubMeshes[SubMeshID].NTris );

        //--- Update geom stats
//        if( s_pStatNVerts ) *s_pStatNVerts += m_pSubMeshes[SubMeshID].NVerts;
        if( s_pStatNTris )  *s_pStatNTris  += m_pSubMeshes[SubMeshID].NTris;
        //if( s_pStatNBytes )   *s_pStatNBytes += m_pSubMeshes[SubMeshID].???;
    } 

    XBOX_SetStencilMode( ENG_STENCIL_OFF );
    ENG_SetBlendMode( CurrentBlendMode, CurrentFixedAlpha );
}


//-------------------------------------------------------------------------------------------------------------------------------------
static f32 s_fLightingTweak1 = 2.0f;
static f32 s_fLightingTweak2 = 2.0f;
static f32 s_fLightingTweak3 = 3.75f;
static f32 s_fLightingTweak4 = 3.0f;
void QGeom::BuildLightingMatrix( matrix4* pLightMatrix, matrix4* pColorMatrix )
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
        s32 i;
        for (i=0; i<4; ++i)
        {
            LightColors.M[ i ][ 0 ] *= s_fLightingTweak1;
        }
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
        s32 i;
        for (i=0; i<4; ++i)
        {
            LightColors.M[ i ][ 1 ] *= s_fLightingTweak2;
        }
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

        s32 i;
        for (i=0; i<4; ++i)
        {
            LightColors.M[ i ][ 2 ] *= s_fLightingTweak3;
        }
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
    s32 i;
    for (i=0; i<4; ++i)
    {
        LightColors.M[ i ][ 3 ] *= s_fLightingTweak4;
    }

    //---   set up the lighting matrix
    *pLightMatrix = LightDir;
    *pColorMatrix = LightColors;
}

xbool QGeom::SubMeshInView( SGeomSubMesh *pSubMesh )
{
#if defined( GEOM_SHOW_BOUNDING_BOXES_ONLY )
    {  // draw all the bounding boxes and midpoints
        matrix4 NewMatrix;
        NewMatrix.Identity();
        XBOX_GetDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&NewMatrix.M );

        DRAW_SetMode( DRAW_3D | 
                      DRAW_NO_FILL | 
                      DRAW_NO_TEXTURE | 
                      DRAW_NO_ALPHA | 
                      DRAW_NO_ZBUFFER |
                      DRAW_NO_CLIP |
                      DRAW_NO_LIGHT );
        
        vector3 PT[24];
        PT[0] = pSubMesh->Corner;
        PT[1] = pSubMesh->Corner + pSubMesh->Axis[2];
        PT[2] = pSubMesh->Corner;
        PT[3] = pSubMesh->Corner + pSubMesh->Axis[1];
        PT[4] = pSubMesh->Corner + pSubMesh->Axis[2];
        PT[5] = pSubMesh->Corner + pSubMesh->Axis[1] + pSubMesh->Axis[2];
        PT[6] = pSubMesh->Corner + pSubMesh->Axis[1];
        PT[7] = pSubMesh->Corner + pSubMesh->Axis[1] + pSubMesh->Axis[2];
        PT[8] = pSubMesh->Corner + pSubMesh->Axis[1];
        PT[9] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[1];
        PT[10] = pSubMesh->Corner + pSubMesh->Axis[1] + pSubMesh->Axis[2];
        PT[11] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[1] + pSubMesh->Axis[2];
        PT[12] = pSubMesh->Corner + pSubMesh->Axis[0];
        PT[13] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[2];
        PT[14] = pSubMesh->Corner + pSubMesh->Axis[0];
        PT[15] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[1];
        PT[16] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[2];
        PT[17] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[1] + pSubMesh->Axis[2];
        PT[18] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[1];
        PT[19] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[1] + pSubMesh->Axis[2];
        PT[20] = pSubMesh->Corner;
        PT[21] = pSubMesh->Corner + pSubMesh->Axis[0];
        PT[22] = pSubMesh->Corner + pSubMesh->Axis[2];
        PT[23] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[2];
/*
        PT[0] = pSubMesh->Corner;
        PT[1] = pSubMesh->Corner + pSubMesh->Axis[2];
        PT[2] = pSubMesh->Corner + pSubMesh->Axis[1];
        PT[3] = pSubMesh->Corner + pSubMesh->Axis[1] + pSubMesh->Axis[2];
        PT[4] = pSubMesh->Corner + pSubMesh->Axis[0];
        PT[5] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[2];
        PT[6] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[1];
        PT[7] = pSubMesh->Corner + pSubMesh->Axis[0] + pSubMesh->Axis[1] + pSubMesh->Axis[2];
*/
        color Color;
        Color.Set(255,0,0,255);
        DRAW_Points( 1, &pSubMesh->MidPoint, &Color );
        Color.Set(0,255,0,255);
        DRAW_Lines( 24, (vector3*)&PT, &Color );

        XBOX_GetDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m_L2W.M );
    }
    return FALSE; 
#endif
    ////////////////////////////////////////////////////////////////////
    // Check if midpt is in view
    ////////////////////////////////////////////////////////////////////
    vector3     V;

    // XGraphics version:
    V.X = XGPlaneDotCoord( &m_XBOXFastClipW2V[0], (XGVECTOR3*)&pSubMesh->MidPoint );
    V.Y = XGPlaneDotCoord( &m_XBOXFastClipW2V[1], (XGVECTOR3*)&pSubMesh->MidPoint );
    V.Z = XGPlaneDotCoord( &m_XBOXFastClipW2V[2], (XGVECTOR3*)&pSubMesh->MidPoint );

	
    if ((V.Z >= m_FastClipPack.NZ) &&
        (V.Z <= m_FastClipPack.FZ))
	{
		f32	pfTemp	= V.X * m_FastClipPack.FX;

		if ((pfTemp <= V.Z) && (-pfTemp <= V.Z))
		{
			pfTemp	= V.Y * m_FastClipPack.FY;

			if ((pfTemp <= V.Z) && (-pfTemp <= V.Z))
			{
				return TRUE;
			}
		}
	}

    // Pick best and worst candidate for being in planes
    s32 NWorstIn = 0;
    s32 i;
    vector3 BestPt;
    vector3 WorstPt;

    for( i=0; i<6; i++ )
    {
        BestPt  = pSubMesh->Corner;
        WorstPt = pSubMesh->Corner;

        if( XGVec3Dot((XGVECTOR3*)&pSubMesh->Axis[0],(XGVECTOR3*)&m_FastClipPack.PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[0];
        else
            WorstPt += pSubMesh->Axis[0];

        if( XGVec3Dot((XGVECTOR3*)&pSubMesh->Axis[1],(XGVECTOR3*)&m_FastClipPack.PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[1];
        else
            WorstPt += pSubMesh->Axis[1];

        if( XGVec3Dot((XGVECTOR3*)&pSubMesh->Axis[2],(XGVECTOR3*)&m_FastClipPack.PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[2];
        else
            WorstPt += pSubMesh->Axis[2];

        // Check if best point is outside
        if( (XGVec3Dot((XGVECTOR3*)&m_FastClipPack.PlaneN[i],(XGVECTOR3*)&BestPt) + m_FastClipPack.PlaneD[i]) < 0 )
            return FALSE; 

    }

    return TRUE;
}

//==========================================================================
void QGeom::VerifyMaterials( void )
{
    // for now, do nothing
    // This function MAY need to Unregister and reRegister all textures for this object.
}


//==========================================================================
t_GeomMesh *QGeom::GetMesh( s32 MeshID )
{
    if (MeshID < 0 || MeshID >= m_NMeshes)
        return NULL;

    return &m_pMeshes[ MeshID ];
}

//==========================================================================
t_GeomSubMesh *QGeom::GetSubMesh( s32 SubMeshID )
{
    if (SubMeshID < 0 || SubMeshID >= m_NSubMeshes)
        return NULL;

    return &m_pSubMeshes[ SubMeshID ];
}

////////////////////////////////////////////////////////////////////////////
//
// PC_Geom.cpp - geom routines specific to the PC
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"
#include "x_memory.hpp"
#include "x_time.hpp"

#include "Q_PC.hpp"
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
// QGeom(PC) Vertex Shader info
////////////////////////////////////////////////////////////////////////////

#include "Shaders/PC_GeomShaderDefines.h"
#include "Shaders/PC_GeomObjShader.h"
#include "Shaders/PC_GeomMultiTexShader.h"
#include "Shaders/PC_GeomShadowShader.h"
#include "Shaders/PC_PSMultiTex.h"

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
    D3DVSD_REG      (1, D3DVSDT_FLOAT3),            // v.xyz = vNormal
    D3DVSD_REG      (2, D3DVSDT_FLOAT2),            // v.xy  = uv1
    D3DVSD_REG      (3, D3DVSDT_FLOAT2),            // v.xy  = uv2
    D3DVSD_REG      (4, D3DVSDT_FLOAT2),            // v.xy  = uv3
    D3DVSD_END      ()
};

////////////////////////////////////////////////////////////////////////////
// QGeom STATICS
////////////////////////////////////////////////////////////////////////////
s32 QGeom::s_VertexShaderID = -1;
s32 QGeom::s_VertexShaderMultiTexID = -1;
s32 QGeom::s_PixelShaderMultiTexID = -1;
s32 QGeom::s_ShadowShaderID = -1;


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

    if( m_pRawData != NULL )
    {
        x_free( m_pRawData );
        m_pRawData = NULL;
    }

    if( m_pMeshes != NULL )
    {
        x_free( m_pMeshes );
        m_pMeshes = NULL;
    }

    if( m_pSubMeshes != NULL )
    {
        x_free( m_pSubMeshes );
        m_pSubMeshes = NULL;
    }

    if( m_pTextureNames != NULL )
    {
        x_free( m_pTextureNames );
        m_pTextureNames = NULL;
    }

    // delete the mesh texture array
    SAFE_DELETE_ARRAY( m_MeshTextureArray )

    KillOwnedTextures();
}

//-------------------------------------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
// VERY IMPORTANT NOTE: THE SetupFromResource ROUTINE IS OVERLOADED A
// COUPLE OF TIMES. IF THE DATA FORMAT CHANGES AT ALL, MAKE SURE THE
// OVERLOADED ROUTINES GET CHANGED AS WELL.
////////////////////////////////////////////////////////////////////////
//
// A PC GEOM file is laid out like this:
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
    pFilePtr = x_fopen( pFilename, "r" );
    ASSERT( pFilePtr != NULL );

    SetupFromResource( pFilePtr );

    //--- close the file
    x_fclose( pFilePtr );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::SetupFromResource( X_FILE* pFilePtr )
{
    ASSERT( pFilePtr );

    LPDIRECT3DDEVICE8 pD3DDevice = PC_GetDevice();

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
                                                      NULL ));

        // Read in the vertex information for this mesh.
        VERIFY( x_fread( pTempData, m_pMeshes[MeshIndex].VertFormatSize, m_pMeshes[MeshIndex].NVerts, pFilePtr ) == m_pMeshes[MeshIndex].NVerts );

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
                                                        NULL ));

        // Read the information from the mesh.
        VERIFY( x_fread( pTempData, sizeof(s16), m_pMeshes[MeshIndex].NIndices, pFilePtr ) == m_pMeshes[MeshIndex].NIndices );

        // Unlock the list.
        DXWARN( m_pMeshes[MeshIndex].pD3DIndices->Unlock() );
        x_MemSanityCheck();
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

        x_MemSanityCheck();
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
        s_VertexShaderID = PC_RegisterVertexShader( s_GEOM_ObjShader, s_GEOM_VertexFormat, 0 );

    PC_ActivateVertexShader( s_VertexShaderID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateVertexShaderMultiTex( void )
{
    if( s_VertexShaderMultiTexID == -1 )
        s_VertexShaderMultiTexID = PC_RegisterVertexShader( s_GEOM_MultiTexShader, s_GEOM_VertexFormatMultiTex, 0 );

    PC_ActivateVertexShader( s_VertexShaderMultiTexID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivateShadowShader( void )
{
    if( s_ShadowShaderID == -1 )
        s_ShadowShaderID = PC_RegisterVertexShader( s_GEOM_ShadowShader, s_GEOM_VertexFormat, 0 );

    PC_ActivateVertexShader( s_ShadowShaderID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::ActivatePixelShaderMultiTex( void )
{
    if( s_PixelShaderMultiTexID == -1 )
        s_PixelShaderMultiTexID = PC_RegisterPixelShader( s_GEOM_PS_MultiTexShader );

    PC_ActivatePixelShader( s_PixelShaderMultiTexID );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::DeActivatePixelShaderMultiTex( void )
{
    if( s_PixelShaderMultiTexID != -1 )
        PC_DeActivatePixelShader( s_PixelShaderMultiTexID );
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
//    m_PCFastClipW2V[0].a = m_FastClipPack.W2V.M[0][0];
//    m_PCFastClipW2V[0].b = m_FastClipPack.W2V.M[1][0];
//    m_PCFastClipW2V[0].c = m_FastClipPack.W2V.M[2][0];
//    m_PCFastClipW2V[0].d = m_FastClipPack.W2V.M[3][0];
//    m_PCFastClipW2V[1].a = m_FastClipPack.W2V.M[0][1];
//    m_PCFastClipW2V[1].b = m_FastClipPack.W2V.M[1][1];
//    m_PCFastClipW2V[1].c = m_FastClipPack.W2V.M[2][1];
//    m_PCFastClipW2V[1].d = m_FastClipPack.W2V.M[3][1];
//    m_PCFastClipW2V[2].a = m_FastClipPack.W2V.M[0][2];
//    m_PCFastClipW2V[2].b = m_FastClipPack.W2V.M[1][2];
//    m_PCFastClipW2V[2].c = m_FastClipPack.W2V.M[2][2];
//    m_PCFastClipW2V[2].d = m_FastClipPack.W2V.M[3][2];
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
        PC_GetDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX *)&Q_L2V );
    }
    else
        PC_GetDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m_L2W.M );

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
void QGeom::MeshRender( s32 MeshID )
{
    s32         SubMeshID;
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
    xbool       bIsMultiTextured;
    xbool       bMultiTexPSActive = FALSE;

    LPDIRECT3DDEVICE8 pD3DDevice = PC_GetDevice();

    if( !pView->SphereInView( view::V_WORLD, m_Location, pMesh->Radius * m_RadiusScale ) )
        return;

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );

    // Determine if there is are any textures in this mesh.
    TextureCount = GetNTextures();

    // Determine if this mesh is using vertex colors.
    if( pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1 || 
        pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3 )
    {
        bMeshHasVertexColor = TRUE;
    }

    // Determine if this is a multitextured object (not environment mapped though)
    bIsMultiTextured = pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3;

    // Determine the color and alpha blending types.
    // Default the 2nd texture stage to disabled.
    ColorOperation1 = D3DTOP_DISABLE;
    AlphaOperation1 = D3DTOP_DISABLE;
    ColorOperation2 = D3DTOP_DISABLE;
    AlphaOperation2 = D3DTOP_DISABLE;

    if( bIsMultiTextured )
    {
        VRAM_PC_SetWrapMode( WRAP_MODE_CLAMP, WRAP_MODE_CLAMP, 1 );
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

    if( pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1 )
		bUseDynamicLighting = FALSE;

    if( bUseDynamicLighting )
    {
        if( bIsMultiTextured )
        {
            ActivateVertexShaderMultiTex();
            ActivatePixelShaderMultiTex();
            bMultiTexPSActive = TRUE;
        }
        else
            ActivateVertexShader();

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
        DXWARN( pD3DDevice->SetVertexShader( pMesh->VertFormat ));
    }

    if (g_EngineFog.bEnabled)
    {
        // set the fog values for the shader
        vector3 fog( g_EngineFog.fDensity, g_EngineFog.fStart, 30000.0f );
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_FOG, &fog, 1) );
    }
    else
    {
        // set fog values to generically off values
        vector3 fog( 0.0f, 0.0f, 1.0f );
        DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_FOG, &fog, 1) );
    }

    // Set the source streams for DirectX
    DXWARN( pD3DDevice->SetStreamSource( 0, pMesh->pD3DVerts, pMesh->VertFormatSize ));
    DXWARN( pD3DDevice->SetIndices( pMesh->pD3DIndices, 0 ));

    // check for a over ride texture and set if there
    if( m_MeshTextureArray[MeshID] != NULL )
        VRAM_PC_Activate( *m_MeshTextureArray[MeshID], 0 );

    // Loop through the sub-meshes in this mesh.
    TotalSubMeshes = (pMesh->FirstSubMesh + pMesh->NSubMeshes);
    s32 NewRenderFlags;
    for( SubMeshID = pMesh->FirstSubMesh; SubMeshID < TotalSubMeshes; SubMeshID++ )
    {
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
        ENG_SetRenderFlags( CurrentRenderFlags ); 
        // If there are textures used for this submesh, set the texture stage accordingly.
        if( m_pSubMeshes[SubMeshID].TextureID >= 0 )
        {
            // Let VRAM know that this texture is needed.
            if( m_pTextures )
            {
                if( m_MeshTextureArray[MeshID] == NULL )
                    VRAM_PC_Activate( m_pTextures[m_pSubMeshes[SubMeshID].TextureID[0]], 0 );

                if( m_pSubMeshes[SubMeshID].TextureID[1] != -1 )
                {
                    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperation1 ));
                    VRAM_PC_Activate( m_pTextures[m_pSubMeshes[SubMeshID].TextureID[1]], 1 );
                }
                else
                    if( bMultiTexPSActive ) DeActivatePixelShaderMultiTex();

                if( m_pSubMeshes[SubMeshID].TextureID[2] != -1 )
                    VRAM_PC_Activate( m_pTextures[m_pSubMeshes[SubMeshID].TextureID[2]], 2 );
            }
            else
            if( m_pTexturePtrs )
            {
                if( m_MeshTextureArray[MeshID] == NULL )
                    VRAM_PC_Activate( *m_pTexturePtrs[m_pSubMeshes[SubMeshID].TextureID[0]], 0 );

                if( m_pSubMeshes[SubMeshID].TextureID[1] != -1 )
                {
                    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperation1 ));
                    VRAM_PC_Activate( *m_pTexturePtrs[m_pSubMeshes[SubMeshID].TextureID[1]], 1 );
                }
                else
                    if( bMultiTexPSActive ) DeActivatePixelShaderMultiTex();

                if( m_pSubMeshes[SubMeshID].TextureID[2] != -1 )
                    VRAM_PC_Activate( *m_pTexturePtrs[m_pSubMeshes[SubMeshID].TextureID[2]], 2 );
            }
            else
            {
                VRAM_PC_Deactivate( 0 );
            }
        }

        // Render the Submesh
        pD3DDevice->DrawIndexedPrimitive( (_D3DPRIMITIVETYPE)(m_pSubMeshes[SubMeshID].PrimType),
                                           m_pSubMeshes[SubMeshID].IndexMin, 
                                           m_pSubMeshes[SubMeshID].IndexCount,
                                           m_pSubMeshes[SubMeshID].IndexStart,
                                           m_pSubMeshes[SubMeshID].NTris );

        if( bMultiTexPSActive ) ActivatePixelShaderMultiTex();

        if( m_pSubMeshes[SubMeshID].Flags ) ENG_SetRenderFlags( CurrentRenderFlags );  // reset display mode
    }

    if( bMultiTexPSActive )
    {
        DeActivatePixelShaderMultiTex();
        bMultiTexPSActive = FALSE;
    }
    // turn lighting back off since most stuff doesn't want it
    DXWARN( pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE) );
}

s32 QGeom::RenderSubMeshes ( xbool* SubMeshes, xbool SkipVisCheck, s32 SubMeshFlags, xbool SkipInViewCheck )
{
    ASSERT( ENG_GetRenderMode() );

    // Prepare for Rendering.
    PreRender();

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
    t_GeomMesh* pMesh;
    view*       pView = ENG_GetActiveView();
    s32         TotalSubMeshes;
    xbool       bUseDynamicLighting;
    xbool       bIsMultiTextured;
    xbool       bMultiTexPSActive = FALSE;
    s32         NewRenderFlags;
    s32         NSubMeshesRendered = 0;

    LPDIRECT3DDEVICE8 pD3DDevice = PC_GetDevice();

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );

    // Determine if there is are any textures in this mesh.
    TextureCount = GetNTextures();

    // loop through all the submeshes and see if the SubMeshes array has it turned on
    for( SubMeshID = 0; SubMeshID < m_NSubMeshes; SubMeshID++ )
    {
        // make sure this submesh is in the display list
        if( SubMeshes[SubMeshID] == FALSE ) continue;
        if( SubMeshFlags == -1 ) SubMeshes[SubMeshID] = FALSE;

        if( SubMeshFlags == SUBMESH_FLAG_NONE && m_pSubMeshes[SubMeshID].Flags != SUBMESH_FLAG_NONE ) continue;
        if( SubMeshFlags > SUBMESH_FLAG_NONE && !(m_pSubMeshes[SubMeshID].Flags & SubMeshFlags) ) continue;

        // check to see if the MeshID has changed and reset the settings for it
        if( m_pSubMeshes[SubMeshID].MeshID != CurrentMeshID )
        {
            if( bMultiTexPSActive )
            {
                DeActivatePixelShaderMultiTex();
                bMultiTexPSActive = FALSE;
            }

            CurrentMeshID = m_pSubMeshes[SubMeshID].MeshID;
            pMesh = &m_pMeshes[CurrentMeshID];

            // Determine if this mesh is using vertex colors.
            if( pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1 || 
                pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3 )
            {
                bMeshHasVertexColor = TRUE;
            }

            // Determine if this is a multitextured object (not environment mapped though)
            bIsMultiTextured = pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3;

            // Determine the color and alpha blending types.
            // Default the 2nd texture stage to disabled.
            ColorOperation1 = D3DTOP_DISABLE;
            AlphaOperation1 = D3DTOP_DISABLE;
            ColorOperation2 = D3DTOP_DISABLE;
            AlphaOperation2 = D3DTOP_DISABLE;

            if( bIsMultiTextured )
            {
                VRAM_PC_SetWrapMode( WRAP_MODE_CLAMP, WRAP_MODE_CLAMP, 1 );
//                if( m_pTexturePtrs )
//                    VRAM_PC_Activate( *m_pTexturePtrs[m_NTextures], 2 );
            }
            if( CurrentRenderFlags & ENG_ALPHA_BLEND_ON || CurrentRenderFlags & ENG_ALPHA_TEST_ON)
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

            if( pMesh->VertFormat == VERTEX_FORMAT_XYZ_ARGB_TEX1 )
		        bUseDynamicLighting = FALSE;

            if( bUseDynamicLighting )
            {
                vector4 vConst;
                if( bIsMultiTextured )
                {
                    ActivateVertexShaderMultiTex();
                    ActivatePixelShaderMultiTex();
                    bMultiTexPSActive = TRUE;
                }
                else
                {
                    ActivateVertexShader();

                    vConst.Set(1.0f, 1.0f, 1.0f, 1.0f);
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_MAX, &vConst.X, 1) );

                    vConst.Set(0.0f, 0.0f, 0.0f, 0.0f);
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_CONST_MIN, &vConst.X, 1) );

                    matrix4 LightDirMatrix, LightColMatrix;
                    BuildLightingMatrix( &LightDirMatrix, &LightColMatrix );
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_LIGHTDIR_1, &LightDirMatrix, 4) );
                    DXWARN( pD3DDevice->SetPixelShaderConstant( 0, &LightDirMatrix.M[2], 1 ) );
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_LIGHTCOL_1, &LightColMatrix, 4) );

                    // fill the L2W transformation matrices
                    D3DXMATRIX matWorld;
                    pD3DDevice->GetTransform( D3DTS_WORLD, &matWorld );
                    D3DXMatrixTranspose( &matWorld, &matWorld );
                    DXWARN( pD3DDevice->SetVertexShaderConstant(VSH_M_MOVE_1, &matWorld, 4) );

                }

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
            else
            {
                DXWARN( pD3DDevice->SetVertexShader( pMesh->VertFormat ));
            }

            // Set the source streams for DirectX
            DXWARN( pD3DDevice->SetStreamSource( 0, pMesh->pD3DVerts, pMesh->VertFormatSize ));
            DXWARN( pD3DDevice->SetIndices( pMesh->pD3DIndices, 0 ));

            // check for a over ride texture and set if there
            if( m_MeshTextureArray[CurrentMeshID] != NULL )
                VRAM_PC_Activate( *m_MeshTextureArray[CurrentMeshID], 0 );

            // Loop through the sub-meshes in this mesh.
            TotalSubMeshes = (pMesh->FirstSubMesh + pMesh->NSubMeshes);
        }  // done setting up the new pMesh settings

        if( !(m_pMeshes[CurrentMeshID].Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck ) continue;

        // RENDER THE SUBMESH
        // check to see if this submesh is within the view and skip if not
        if( !SkipVisCheck && !SkipInViewCheck )
            if( !SubMeshInView( &m_pSubMeshes[SubMeshID] ) ) continue;

        SubMeshes[SubMeshID] = TRUE;
        NSubMeshesRendered++;

        NewRenderFlags = CurrentRenderFlags;
        if( m_pSubMeshes[SubMeshID].Flags )
        {
            NewRenderFlags = CurrentRenderFlags;
            NewRenderFlags &= ~ENG_ALPHA_TEST_ON;
            NewRenderFlags |= ENG_ALPHA_TEST_OFF;
            NewRenderFlags &= ~ENG_ALPHA_BLEND_ON;
            NewRenderFlags |= ENG_ALPHA_BLEND_OFF;
            if( m_pSubMeshes[SubMeshID].Flags & SUBMESH_FLAG_ALPHA_P )
            {
                // turn the alpha test mode on
                NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                NewRenderFlags |= ENG_ALPHA_TEST_ON;
            }
            if( m_pSubMeshes[SubMeshID].Flags & SUBMESH_FLAG_ALPHA_T )
            {
                // turn the alpha blend mode on
                NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                NewRenderFlags |= ENG_ALPHA_BLEND_ON;
            }
        }
        ENG_SetRenderFlags( NewRenderFlags );
        // If there are textures used for this submesh, set the texture stage accordingly.
        if( m_pSubMeshes[SubMeshID].TextureID >= 0 )
        {
            // Let VRAM know that this texture is needed.
            if( m_pTextures )
            {
                if( m_MeshTextureArray[CurrentMeshID] == NULL )
                    VRAM_PC_Activate( m_pTextures[m_pSubMeshes[SubMeshID].TextureID[0]], 0 );

                if( m_pSubMeshes[SubMeshID].TextureID[1] != -1 )
                {
                    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperation1 ));
                    VRAM_PC_Activate( m_pTextures[m_pSubMeshes[SubMeshID].TextureID[1]], 1 );
                }
                else
                    if( bMultiTexPSActive ) DeActivatePixelShaderMultiTex();

                if( m_pSubMeshes[SubMeshID].TextureID[2] != -1 )
                    VRAM_PC_Activate( m_pTextures[m_pSubMeshes[SubMeshID].TextureID[2]], 2 );
            }
            else
            if( m_pTexturePtrs )
            {
                if( m_MeshTextureArray[CurrentMeshID] == NULL )
                    VRAM_PC_Activate( *m_pTexturePtrs[m_pSubMeshes[SubMeshID].TextureID[0]], 0 );

                if( m_pSubMeshes[SubMeshID].TextureID[1] != -1 )
                {
                    DXWARN(pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOperation1 ));
                    VRAM_PC_Activate( *m_pTexturePtrs[m_pSubMeshes[SubMeshID].TextureID[1]], 1 );
                }
                else
                    if( bMultiTexPSActive ) DeActivatePixelShaderMultiTex();

                if( m_pSubMeshes[SubMeshID].TextureID[2] != -1 )
                    VRAM_PC_Activate( *m_pTexturePtrs[m_pSubMeshes[SubMeshID].TextureID[2]], 2 );
            }
            else
            {
                VRAM_PC_Deactivate( 0 );
            }
        }

        // Render the Submesh
        pD3DDevice->DrawIndexedPrimitive( (_D3DPRIMITIVETYPE)(m_pSubMeshes[SubMeshID].PrimType),
                                           m_pSubMeshes[SubMeshID].IndexMin, 
                                           m_pSubMeshes[SubMeshID].IndexCount,
                                           m_pSubMeshes[SubMeshID].IndexStart,
                                           m_pSubMeshes[SubMeshID].NTris );

        if( bMultiTexPSActive ) ActivatePixelShaderMultiTex();
        if( m_pSubMeshes[SubMeshID].Flags ) ENG_SetRenderFlags( CurrentRenderFlags );  // reset display mode
    }

    if( bMultiTexPSActive )
    {
        DeActivatePixelShaderMultiTex();
        bMultiTexPSActive = FALSE;
    }

    // turn lighting back off since most stuff doesn't want it
    DXWARN( pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE) );
    return NSubMeshesRendered;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void QGeom::RenderMeshAsShadow( s32 MeshID, matrix4 ShadowMatrix, xbool SkipVisCheck )
{
    s32         SubMeshID;
    s32         LastSubMeshID;
    t_GeomMesh* pMesh;
    view*       pView;
    vector3     Location;

    ASSERT( ENG_GetRenderMode() );
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    LPDIRECT3DDEVICE8 pD3DDevice = PC_GetDevice();

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

//    PC_SetStencilMode( ENG_STENCIL_ON, CurrentFixedAlpha );
//    PC_SetShadowBiasMode( ENG_SHADOW_BIAS_ON );

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

//    PC_SetStencilMode( ENG_STENCIL_OFF );
}


//-------------------------------------------------------------------------------------------------------------------------------------
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

xbool QGeom::SubMeshInView( SGeomSubMesh *pSubMesh )
{
    s32     i;
    view*   pView;
    matrix4 W2V;
    f32     FX,FY;
    f32     NZ,FZ;
    vector3 PlaneN[6];
    f32     PlaneD[6];

    // Get view info
    pView = ENG_GetActiveView();
    pView->GetFrustum( FX, FY );
    pView->GetW2VMatrix( W2V );
    pView->GetZLimits( NZ, FZ );
    pView->GetViewPlanes( view::V_WORLD, PlaneN[3], PlaneD[3],
                                         PlaneN[4], PlaneD[4],
                                         PlaneN[1], PlaneD[1],
                                         PlaneN[2], PlaneD[2],
                                         PlaneN[0], PlaneD[0],
                                         PlaneN[5], PlaneD[5]);

    ////////////////////////////////////////////////////////////////////
    // Check if midpt is in view
    ////////////////////////////////////////////////////////////////////
    vector4     V;

    ////////////////////////////////////////////////////////////////
    // Convert pt into camera space
    ////////////////////////////////////////////////////////////////

    // CPU version:
    V.X  = (W2V.M[0][0]*pSubMesh->MidPoint.X) + (W2V.M[1][0]*pSubMesh->MidPoint.Y) + (W2V.M[2][0]*pSubMesh->MidPoint.Z) + W2V.M[3][0];
    V.Y  = (W2V.M[0][1]*pSubMesh->MidPoint.X) + (W2V.M[1][1]*pSubMesh->MidPoint.Y) + (W2V.M[2][1]*pSubMesh->MidPoint.Z) + W2V.M[3][1];
    V.Z  = (W2V.M[0][2]*pSubMesh->MidPoint.X) + (W2V.M[1][2]*pSubMesh->MidPoint.Y) + (W2V.M[2][2]*pSubMesh->MidPoint.Z) + W2V.M[3][2];

    if((  V.Z      >= NZ  ) &&
       (  V.Z      <= FZ  ) &&
       (  V.X * FX <= V.Z ) && 
       ( -V.X * FX <= V.Z ) && 
       (  V.Y * FY <= V.Z ) && 
       ( -V.Y * FY <= V.Z ))
    {
        return TRUE;;
    }

    for( i=0; i<6; i++ )
    {
        vector3 BestPt;
        vector3 WorstPt;

        BestPt  = pSubMesh->Corner;
        WorstPt = pSubMesh->Corner;

        if( Dot(pSubMesh->Axis[0],PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[0];
        else
            WorstPt += pSubMesh->Axis[0];

        if( Dot(pSubMesh->Axis[1],PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[1];
        else
            WorstPt += pSubMesh->Axis[1];

        if( Dot(pSubMesh->Axis[2],PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[2];
        else
            WorstPt += pSubMesh->Axis[2];

        // Check if best point is outside
        if( (Dot(PlaneN[i],BestPt) + PlaneD[i]) < 0 )
        {
            return FALSE;
        }

    }
    return TRUE;
}


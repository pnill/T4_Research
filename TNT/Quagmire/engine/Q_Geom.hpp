////////////////////////////////////////////////////////////////////////////
//
// Q_Geom.hpp
//
////////////////////////////////////////////////////////////////////////////

#ifndef Q_GEOM_HPP_INCLUDED
#define Q_GEOM_HPP_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_math.hpp"
#include "x_bitmap.hpp"

#include "Q_View.hpp"

#if defined( TARGET_PS2 )
	#include "PS2/PS2_Geom.hpp"
#elif defined( TARGET_XBOX )
	#include "XBOX/XBOX_Geom.hpp"
    #include "xgmath.h"
#elif ( defined( TARGET_PC ) && !defined( _CONSOLE ) )
	#include "PC/PC_Geom.hpp"
#elif defined( TARGET_DOLPHIN )
	#include "GameCube/GC_Geom.hpp"
#endif


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define TEXTURE_NAME_LENGTH				32

#define GEOM_FLAG_OWNS_TEXTURES         0x01

#define MESH_FLAG_CONTAINS_POSPTRS      0x0001
#define MESH_FLAG_CONTAINS_UVPTRS       0x0002
#define MESH_FLAG_CONTAINS_RGBAPTRS     0x0004
#define MESH_FLAG_CONTAINS_NORMALPTRS   0x0008
#define MESH_FLAG_VISIBLE               0x0010
#define MESH_FLAG_USE_MIPS              0x0020
#define MESH_FLAG_DYNAMIC_LIGHTING      0x0040
#define MESH_FLAG_SHADOW                0x0080
#define MESH_FLAG_ENVMAPPED             0x0100
#define MESH_FLAG_SPECIAL               0x0200
#define MESH_FLAG_LOCKED                0x8000

#define SUBMESH_FLAG_NONE               0x0000
#define SUBMESH_FLAG_ENVMAP             0x0001
#define SUBMESH_FLAG_ALPHA_P            0x0002  // this submesh should be blended in test mode
#define SUBMESH_FLAG_ALPHA_T            0x0004  // this submesh should be blended with transparent mode
#define SUBMESH_FLAG_NO_BILINEAR        0x0008  // this submesh should NOT use bilinear filtering

////////////////////////////////////////////////////////////////////////////
// This structure is the data used in the SubMeshInView function.
// You need to setup m_FastClipPack data in the PreRender function.
////////////////////////////////////////////////////////////////////////////
typedef struct
{
    matrix4         W2V;
    f32             FX,FY;
    f32             NZ,FZ;
    vector3         PlaneN[6];
    f32             PlaneD[6];
} t_FastClipPack;

#if defined (TARGET_PS2)
typedef struct t_TexData
{
    s32 CurrentMeshID;
    s32 SubMeshID;
    x_bitmap *pTexture;
    x_bitmap *pLastTexture;
    x_bitmap *pEnvMapTexture;
    view *pView;
    vector3 MipPlaneN;
    f32 MipPlaneD;
    t_GeomMesh *pMesh;
} t_TexData;
#endif

#if defined( TARGET_PS2 )
const s32 kMaxNumStages = 2;
#else
const s32 kMaxNumStages = 4;
#endif

typedef struct SGeomMaterial
{
    s32             nStages;
    s32             iStages[ kMaxNumStages ];
} t_GeomMaterial;

typedef struct SGeomMatStage
{
    s32             iTexture;
} t_GeomMatStage;


////////////////////////////////////////////////////////////////////////////
// QGeom Class
////////////////////////////////////////////////////////////////////////////

class QGeom
{
public:
    ////////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    ////////////////////////////////////////////////////////////////////////
             QGeom( void );
             QGeom( char* filename );
    virtual ~QGeom( void );

    ////////////////////////////////////////////////////////////////////////
    // Misc. setup functions
    ////////////////////////////////////////////////////////////////////////
    //
    //  SetupFromResource - use this routine to load in a new geometry file
    //
    //  PS2 SPECIFIC:
    //  ActivateMicroCode - Other places in your game may wish to share the
    //                      geometry's micro code. Use this static routine
    //                      if you would like to do this
    //
    //  XBOX SPECIFIC:
    //  ActivateVertexShader - Similar to PS2 microcode, use this to activate the
	//						   QGeom vertex shader.
    //
    //  ActivateVertexShaderMultiTex - Same as normal vertex shader, with multi
    //                                 texture support
	//
	//  ActivateShadowShader - Activates the vertex shader to do geometry shadows.
    //
    //  ActivatePixelShaderMultiTex - Activates the pixel shader to handle 3 TEX
    //  DeActivatePixelShaderMultiTex - DEActivates the pixel shader to handle 3 TEX
    //
    ////////////////////////////////////////////////////////////////////////

    void SetupFromResource( char* pFilename );
    void SetupFromResource( X_FILE* pFilePtr );
    void SetupFromResource( byte* pByteStream );

    void SetupFromResource_Multipass( X_FILE* pFilePtr );

    void PrepareForMultipass    ( void );
    void RecoverFromMultipass   ( void );

#if defined(TARGET_PS2)
    static void ActivateMicroCode( void );


#elif defined( TARGET_XBOX ) || defined( TARGET_PC )
    static void  ActivateVertexShader( void );
    static void  ActivateVertexShaderMultiTex( void );
    static void  ActivateShadowShader( void );
    static void  ActivateVertexShaderEnvMap( void );
    static void  ActivateVertexShader2DiffuseTex( void );
    static void  ActivatePixelShader( void );
    static void  ActivatePixelShaderMultiTex( void );
    static void  ActivatePixelShaderEnvMap( void );
    static void  ActivatePixelShader2DiffuseTex( void );
    static void  DeActivatePixelShaderMultiTex( void );
    static void  DeActivatePixelShaderEnvMap( void );
    static void  DeActivatePixelShader2DiffuseTex( void );

    static void  ActivatePixelShader( s32 iIndex = 0 );
    static void  DeActivatePixelShader( s32 iIndex = 0 );

#if defined( TARGET_XBOX )
    void SetBlendSettings_FixedAlpha( LPDIRECT3DDEVICE8 pD3DDevice,
                                      u32   &CurrentBlendMode,
                                      u8    &CurrentFixedAlpha,
                                      s32   TextureCount,
                                      s32   *ColorOperations,
                                      s32   *AlphaOperations,
                                      s32   iMaterial,
                                      xbool bMeshHasVertexColor );
    void ActivateShaders_DynamicLighting( LPDIRECT3DDEVICE8 pD3DDevice, view *pView, s32 SubMeshID, vector3 *pos = NULL );
    void ActivateShaders_MultiTexture( LPDIRECT3DDEVICE8 pD3DDevice, view *pView, s32 SubMeshID );
    void ActivateShaders_NoMultiTexture( LPDIRECT3DDEVICE8 pD3DDevice );

    void ActivateTexture( LPDIRECT3DDEVICE8 pD3DDevice, s32 CurrentMeshID, s32 SubMeshID, s32 *ColorOperations );
    void ActivateTexture_Owned( LPDIRECT3DDEVICE8 pD3DDevice, s32 CurrentMeshID, s32 SubMeshID, s32 *ColorOperations );
    void ActivateTexture_Pointed( LPDIRECT3DDEVICE8 pD3DDevice, s32 CurrentMeshID, s32 SubMeshID, s32 *ColorOperations );
#endif
#endif

    ////////////////////////////////////////////////////////////////////////
    // Texture functions
    ////////////////////////////////////////////////////////////////////////
    //
    //  To find out what order the geometry expects the textures to be in,
    //  use GetTextureIndex or GetTextureName.
    //
    //  There are two methods for you to specify which texture the geometry
    //  should use. If your textures are contiguous in memory, you can use
    //  a call to SetTextureArray. If your texture are not contiguous in
    //  memory, you can create an array of texture pointers, and call
    //  SetTexturePtrArray.
    //
    //  A call to either SetTextureArray or SetTexturePtrArray will NULL
    //  the other one. Use only one of these functions.
    //
    //  SetMeshTexture will over ride the texture used within the mesh.
    //  ALL SUBMESHES WITHIN THE MESH WILL USE THIS SAME TEXTURE.
    //
    //  VERY IMPORTANT NOTE ABOUT MEMORY USAGE:
    //  If you use SetTextureArray or SetTexturePtrArray, you will then
    //  be responsible for freeing your own memory.
    //
    ////////////////////////////////////////////////////////////////////////

    s32         GetNTextures        ( void );
    s32         GetTextureIndex     ( const char* TextureName );
    const char* GetTextureName      ( s32 TextureIndex );
    void        SetTextureArray     ( x_bitmap* TextureArray );
    x_bitmap*   GetTextureArray     ( void );
    void        SetTexturePtrArray  ( x_bitmap** TexturePtrArray );
    x_bitmap**  GetTexturePtrArray  ( void );

    void        VerifyMaterials     ( void );

    void        SetMeshTexture      ( s32 MeshID, x_bitmap* Texture );
    x_bitmap*   GetMeshTexture      ( s32 MeshID );

    s32         GetTextureID        ( s32 MeshID, s32 SubMeshID );
    void        SetTextureID        ( s32 MeshID, s32 SubMeshID, s32 TextureID );

    ////////////////////////////////////////////////////////////////////////
    // Render functions
    ////////////////////////////////////////////////////////////////////////
    //
    //  Render - Renders all meshes in the geometry
    //  RenderMesh - Renders only one mesh in the geometry
    //  RenderSubMeshes - Renders a display list of submeshes (see function definition for parameters), returns the number rendered
    //
    ////////////////////////////////////////////////////////////////////////

    void    Render              ( xbool SkipVisCheck = FALSE );
    void    RenderMesh          ( s32 MeshID, xbool SkipVisCheck = FALSE );
    s32     RenderSubMeshes     ( xbool* SubMeshes, xbool SkipVisCheck = FALSE, s32 SubMeshFlags = -1, xbool SkipInViewCheck = FALSE );
    void    RenderSubMeshAtPos  ( SGeomSubMesh *pSubMesh, s32 iSubMeshIndex, vector3 &pos );

    ////////////////////////////////////////////////////////////////////////
    // Rendering Shadows
    ////////////////////////////////////////////////////////////////////////
    //
    //  CalcShadowMatrix - Calculates a shadow matrix given a light
    //      direction and a plane to cast the shadow onto. This should be
    //      done as little as possible, so if you need to cast multiple
    //      shadows to one plane from a single light source, only do this
    //      calculation once, passing the result into RenderMeshAsShadow
    //      as much as needed.
    //  RenderMeshAsShadow - Renders a mesh as a shadow mesh. In order to
    //      optimize this operation, try to use a mesh that is not
    //      dynamically lit, and if you can cut down on poly counts, do so.
    //
    ////////////////////////////////////////////////////////////////////////
    static matrix4 CalcShadowMatrix  ( vector3 LightDir, vector4 Plane );
           void    RenderMeshAsShadow( s32 MeshID, matrix4 ShadowMatrix, xbool SkipVisCheck = FALSE );

    ////////////////////////////////////////////////////////////////////////
    // Mesh data
    ////////////////////////////////////////////////////////////////////////
    //
    //  You can get pointers to the mesh data to alter them. Because the
    //  format of the data is platform specific, and build specific, they
    //  are returned to you as void*, which can then be typecast to the
    //  appropriate data type.
    //
    //  PS2 SPECIFIC:
    //  Because the Playstation 2 uses a double-buffering scheme, it is
    //  possible that the previous frame may use data that you have just
    //  altered. Be prepared for this.
    //
    ////////////////////////////////////////////////////////////////////////

    s32     GetNVertsInMesh     ( s32 MeshID );

    void    LockMesh            ( s32 MeshID );
    void    UnlockMesh          ( s32 MeshID );

    void*   GetPosPtr           ( s32 MeshID, s32 VertIndex );
    void*   GetUVPtr            ( s32 MeshID, s32 VertIndex );
    void*   GetColorPtr         ( s32 MeshID, s32 VertIndex );
    void*   GetNormalPtr        ( s32 MeshID, s32 VertIndex );

    ////////////////////////////////////////////////////////////////////////
    // Geometry Flags
    ////////////////////////////////////////////////////////////////////////

    xbool   GetFlag     ( s32 GeomFlagID );
    void    SetFlag     ( s32 GeomFlagID, xbool FlagValue );
    void    SetFlags    ( s32 FlagMask );

    ////////////////////////////////////////////////////////////////////////
    // Mesh Flags
    ////////////////////////////////////////////////////////////////////////

    xbool   GetMeshFlag     ( s32 MeshID, s32 MeshFlagID );
    void    SetMeshFlag     ( s32 MeshID, s32 MeshFlagID, xbool FlagValue );
    void    SetMeshFlags    ( s32 MeshID, s32 MeshFlagMask );

    ////////////////////////////////////////////////////////////////////////
    // SubMesh Flags
    ////////////////////////////////////////////////////////////////////////

    xbool   GetSubMeshFlag  ( s32 MeshID,
                              s32 SubMeshID,
                              s32 SubMeshFlagID );
    void    SetSubMeshFlag  ( s32 MeshID,
                              s32 SubMeshID,
                              s32 SubMeshFlagID,
                              xbool FlagValue );
    void    SetSubMeshFlags ( s32 MeshID,
                              s32 SubMeshID,
                              s32 SubMeshFlagMask );

    ////////////////////////////////////////////////////////////////////////
    // General Purpose Info
    ////////////////////////////////////////////////////////////////////////

    const char* GetName       ( void );
    s32         GetNMeshes    ( void );
    s32         GetNSubMeshes ( void );
    const char* GetMeshName   ( s32 MeshID );
    s32         GetMeshIndex  ( const char* MeshName );
    f32         GetMeshRadius ( s32 MeshID );


    t_GeomMesh      *GetMesh( s32 MeshID );
    t_GeomSubMesh   *GetSubMesh( s32 SubMeshID );

    ////////////////////////////////////////////////////////////////////////
    // L2W matrix
    ////////////////////////////////////////////////////////////////////////

    matrix4     GetL2W      ( void );
    void        SetL2W      ( matrix4& L2W );

    ////////////////////////////////////////////////////////////////////////
    // Stat functions
    ////////////////////////////////////////////////////////////////////////
    static void SetStatTrackers( s32* pNVerts, s32* pNTris, s32* pNBytes );

protected:

    ////////////////////////////////////////////////////////////////////////
    // Internal routines
    ////////////////////////////////////////////////////////////////////////
    void InitData           ( void );
    void KillData           ( void );
    void KillOwnedTextures  ( void );

    void PreRender      ( matrix4* ShadowMatrix = NULL );

    xbool SubMeshInView     ( SGeomSubMesh* pSubMesh );
    xbool SubMeshOnView     ( SGeomSubMesh* pSubMesh );

    ////////////////////////////////////////////////////////////////////////
    // Member variables
    ////////////////////////////////////////////////////////////////////////

    char            m_Name[16];         // Name of geom
    u32             m_Flags;            // GEOM_FLAG_xxx bits
#ifdef TARGET_PS2
    matrix4         m_L2W __attribute__ ((aligned(16))); // Local to World matrix
#else
    matrix4         m_L2W;              // Local to World matrix
#endif

    s32             m_NTextures;
	s32				m_NTexturesOwned;
    char*           m_pTextureNames;    // Texture names (16 chars/name)
    x_bitmap*       m_pTextures;        // Array of textures
    x_bitmap**      m_pTexturePtrs;     // Array of texture ptrs

    s32             m_NMaterials;
    t_GeomMaterial  *m_pMaterials;
    s32             m_NMatStages;
    t_GeomMatStage  *m_pMatStages;

    s32             m_NMeshes;          // Number of meshes
    t_GeomMesh*     m_pMeshes;          // Array of meshes
    s32             m_MeshLocked;       // Mesh locked (only one mesh can be locked at a time )

    s32             m_NSubMeshes;       // Number of sub-meshes
    t_GeomSubMesh*  m_pSubMeshes;       // Array of sub-meshes

    byte*           m_pRawData;         // Pointer to raw data of geom

    static s32*     s_pStatNVerts;      // user-specified stat-tracking stuff
    static s32*     s_pStatNTris;
    static s32*     s_pStatNBytes;

    x_bitmap**      m_MeshTextureArray; // array of textures that over ride each mesh texture (if set)

    t_FastClipPack  m_FastClipPack;     // structure that contains the data needed for SubMeshInView function
#if defined( TARGET_XBOX )
    XGPLANE         m_XBOXFastClipW2V[3]; // this should be equated to the W2V matrix as needed in the SubMeshInView function
#endif

	///////////////////////////////////////////////////////////////////////////
	//  Platform Specific
	///////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PS2 )
    ////////////////////////////////////////////////////////////////////////
    // PS2 SPECIFIC
    ////////////////////////////////////////////////////////////////////////

    void SendLightingMatrix     ( void );   // sends the lighting info to the VU
    void SendNL2VMatrix         ( void );   // sends the normalized L2V to VU for env-mapping

    // sends flags which tell VU how to behave for a specific submesh render
    void SendSubmeshFlags       ( xbool bPerformClip, xbool bPerformLight, xbool bPerformEnvmap, xbool bFirst = FALSE, xbool bShadow = FALSE );
    void SetEnvMapContext       ( x_bitmap* pBitmap );

    void ActivateTexture        ( t_TexData &Data, s32 iPass = 1 );
    void ActivateTextureMIPs    ( t_TexData &Data );
    void ActivateTextureNoMIPs  ( t_TexData &Data );
    f32  ComputeMipK            ( s32 SubMeshIndex, view* pView );
    void ComputeLOD             ( s32 MeshIndex,
                                  f32 MipK,
                                  vector3 &PlaneN,
                                  f32 PlaneD,
                                  f32& MinLOD,
                                  f32& MaxLOD );
    s32 RenderSubMeshes_MultiPass( xbool* SubMeshes, t_TexData &Data, s32 SubMeshFlags, xbool SkipVisCheck, s32 nStages, u32 RenderFlags, u32 BlendMode, u8 FixedAlpha );
    xbool PokeNewSubmeshData( t_TexData &Data, xbool bPerformLight, xbool bPerformEnvmap, xbool SkipVisCheck );

protected:
    static s32      s_MicroCodeHandle;
    s32             m_PacketSize;
    byte*           m_Packets;

    u32*            m_pRawPtrData;

    s32*            m_pContiguousMaterials;


#elif defined( TARGET_XBOX ) || defined( TARGET_PC )
    ////////////////////////////////////////////////////////////////////////
    // X-BOX SPECIFIC
    ////////////////////////////////////////////////////////////////////////

    void    MeshRender  ( s32 MeshID );

    f32     m_RadiusScale;
    vector3 m_Location;
    s32     m_ActiveTextureID;
    void*   m_pLockData;

    void  BuildLightingMatrix ( matrix4* pLightMatrix, matrix4* pColorMatrix );


#elif defined( TARGET_DOLPHIN )
    ////////////////////////////////////////////////////////////////////////
    // GAMECUBE SPECIFIC
    ////////////////////////////////////////////////////////////////////////

	byte*		m_pVertexData;
	byte*		m_pDispListData;
	u32			m_VertexDataSize;
	u32			m_DispListDataSize;

#endif

};

////////////////////////////////////////////////////////////////////////////
// Inline funcs
////////////////////////////////////////////////////////////////////////////

#include "Q_Geom_inline.hpp"

#if defined( TARGET_PS2 )
	#include "PS2/PS2_Geom_inline.hpp"
#elif defined( TARGET_XBOX )
	#include "XBOX/XBOX_Geom_inline.hpp"
#elif ( defined( TARGET_PC ) && !defined( _CONSOLE ) )
	#include "PC/PC_Geom_inline.hpp"
#elif defined( TARGET_DOLPHIN )
	#include "GameCube/GC_Geom_inline.hpp"
#endif

////////////////////////////////////////////////////////////////////////////

#endif  // Q_GEOM_INCLUDED_HPP
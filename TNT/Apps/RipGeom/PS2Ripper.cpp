#include <stdlib.h>

#include "x_plus.hpp"
#include "x_memory.hpp"

#include "PS2/PS2_Geom.hpp"
#include "Q_Geom.hpp"
#include "AUX_Bitmap.hpp"

#include "PS2Ripper.h"
#include "RipGeom.h"
#include "PS2Help.h"
#include "Strip.h"


////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////

//
// We have 861 qwords to use for vertex buffers
//
// Triple buffers require 720/3 qwords = 240 qwords
//  A 3-component triple vertex buffer has 79 verts, 1 flag var, and 2 GIFtags - 240 qwords
//  A 4-component triple vertex buffer has 59 verts, 1 flag var, and 2 GIFtags - 240 qwords
//
// Double buffers require 720/2 qwords = 360 qwords
//  A 3-component double vertex buffer has 119 verts, 1 flag var, and 2 GIFtags - 360 qwords
//  A 4-component double vertex buffer has 89 verts, 1 flag var, and 2 GIFtags - 360 qwords
//

static const u32 kBufferSize = 240;

static const u32 kMaxVertsInPackage[2] = {79, 119};
static const u32 kMaxVertsInPackageEnvMap[2] = {59, 89};

#define MAX_VERTS_IN_PACKAGE    119
#define MAX_VERTS_IN_PACKAGE_SHADOW 117
#define MAX_VERTS_IN_PACKAGE_ENVMAP 89
#define MAX_PACKET_SIZE         (12 * 1024 * 1024)
#define MAX_NUM_MESHES          512
#define MAX_NUM_SUBMESHES       1024
#define MAX_NUM_VERTS           100000
#define MAX_NUM_PACKAGES        500
#define MAX_NUM_TRIS_IN_SUBMESH 100000

////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////

typedef struct SStripVert
{
    s32     VertIndex;
    vector2 UV;
    xbool   KickEnabled;
} t_StripVert;

typedef struct SStripPackage
{
    s32             NVerts;
    s32             NTris;  // for stats tracking
    t_StripVert     Verts[MAX_VERTS_IN_PACKAGE];
} t_StripPackage;

////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////

static byte*            s_PacketData = NULL;
static s32              s_CurPacketSize = 0;
static t_GeomMesh*      s_MeshData = NULL;
static t_GeomSubMesh*   s_SubMeshData = NULL;
static s32              s_CurSubMesh = 0;
static u32*             s_PosPtrs = NULL;
static s32              s_NPosPtrs = 0;
static u32*             s_UVPtrs = NULL;
static s32              s_NUVPtrs = 0;
static u32*             s_RGBAPtrs = NULL;
static s32              s_NRGBAPtrs = 0;
static u32*             s_NormalPtrs = NULL;
static s32              s_NNormalPtrs = 0;
static byte*            s_pD = NULL;

static vector4          s_CompressColor;

static t_Triangle*      s_TriData = NULL;
static vector2*         s_UV = NULL;
static s32              s_NTris = 0;

static u16              s_SubMeshTriData[MAX_NUM_TRIS_IN_SUBMESH];

static mesh::material_set*    s_pMaterialSet = NULL;

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

static s32 GetAlphaMode( s32 TriID, mesh::object& rGeom )
{
    s32 MaterialID;

    MaterialID = s_TriData[s_SubMeshTriData[0]].MaterialID;
    if ( MaterialID >= 0 )
    {
        mesh::material*         pMaterial;
        pMaterial = &s_pMaterialSet->m_pMaterials[MaterialID];

        char NewDrive[5];
        char NewDir[255];
        char NewFileName[80];
        char NewExtension[80];
        s32 iIndex = s_pMaterialSet->m_pStages[pMaterial->m_iStages[0]].m_iTexture;
        if (iIndex == -1)
            return -1;
        x_splitpath( s_pMaterialSet->m_pTextures[iIndex].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
        char AlphaValue = NewFileName[2];
        if( AlphaValue >= '0' && AlphaValue <= '9' ) 
            return mesh::ALPHA_MODE_TRANSPARENT;
        return -1;
    }
    else
    {
        return -1;
    }
}

//==========================================================================

static
void FillName( char* Name, char* src, s32 NChars )
{
    s32 length = x_strlen( src );
    s32 start;

    if ( length < NChars )
        start = 0;
    else
        start = length - NChars + 1;

    x_strcpy( Name, &src[start] );
}

//==========================================================================

static
void MakeUV( s32 Index, mesh::chunk* rMesh )
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    //  UV Adjustments
    //    This code was copied from the original StadRip... let's hope it has a desired effect
    ///////////////////////////////////////////////////////////////////////////////////////////////
    s32 MinU;
    s32 MinV;
    s32 BreakLoopCounter = 0;
    const s32 MaxUVShuffleSize = 8000;

    vector2 UV[3];

    UV[0] = rMesh->m_pVerts[s_TriData[Index].Index[0]].m_UVs[0];
    UV[1] = rMesh->m_pVerts[s_TriData[Index].Index[1]].m_UVs[0];
    UV[2] = rMesh->m_pVerts[s_TriData[Index].Index[2]].m_UVs[0];

    ///////////////////////////////////////////////////////////////////
    // find minimum U
    MinU = 0;
    if (UV[1].X < UV[MinU].X) MinU = 1;
    if (UV[2].X < UV[MinU].X) MinU = 2;

    ////////////////////////////////////////
    // slide U values over
    if (UV[MinU].X<=0)
    {
        BreakLoopCounter = 0;
        while ( UV[MinU].X < 0 )
        {
            UV[0].X++;
            UV[1].X++;
            UV[2].X++;
            BreakLoopCounter++;
            if( BreakLoopCounter == MaxUVShuffleSize ) break;
        }
    }
    else
    {
        BreakLoopCounter = 0;
        while ( (UV[MinU].X >= 1.0f) && (UV[MinU].X+1 >= 0.0f))
        {
            UV[0].X--;
            UV[1].X--;
            UV[2].X--;
            BreakLoopCounter++;
            if( BreakLoopCounter == MaxUVShuffleSize ) break;
        }
    }

    ///////////////////////////////////////////////////////////////////
    // find minimum V
    MinV = 0;
    if (UV[1].Y < UV[MinV].Y) MinV = 1;
    if (UV[2].Y < UV[MinV].Y) MinV = 2;

    ////////////////////////////////////////
    // slide V values over
    if (UV[MinV].Y<=0)
    {
        BreakLoopCounter = 0;
        while ( UV[MinV].Y < 0 )
        {
            UV[0].Y++;
            UV[1].Y++;
            UV[2].Y++;
            BreakLoopCounter++;
            if( BreakLoopCounter == MaxUVShuffleSize ) break;
        }
    }
    else
    {
        BreakLoopCounter = 0;
        while ( (UV[MinV].Y >= 1.0f) && (UV[MinV].Y+1 >= 0.0f))
        {
            UV[0].Y--;
            UV[1].Y--;
            UV[2].Y--;
            BreakLoopCounter++;
            if( BreakLoopCounter == MaxUVShuffleSize ) break;
        }
    }
    ASSERT( UV[0].X >= 0.0f );
    ASSERT( UV[1].X >= 0.0f );
    ASSERT( UV[2].X >= 0.0f );
    ASSERT( UV[0].Y >= 0.0f );
    ASSERT( UV[1].Y >= 0.0f );
    ASSERT( UV[2].Y >= 0.0f );
    // END OF UV Adjustment Code

    s32 TextureID = 0;
    if ( s_TriData[Index].MaterialID >= 0 )
    {
        mesh::material*         pMaterial;
        mesh::stage*     pStage;

        pMaterial = &s_pMaterialSet->m_pMaterials[s_TriData[Index].MaterialID];
        pStage = &s_pMaterialSet->m_pStages[pMaterial->m_iStages[0]];

        if( pStage->m_iTexture < 0 )
            TextureID = 0;
        else
            TextureID = pStage->m_iTexture;
    }


    // check if we need to rotate the UVs
    if( s_pMaterialSet->m_pTextures[TextureID].m_Height > s_pMaterialSet->m_pTextures[TextureID].m_Width )
    {
        UV[0].X = rMesh->m_pVerts[s_TriData[Index].Index[0]].m_UVs[0].Y;
        UV[0].Y = 1.0f - rMesh->m_pVerts[s_TriData[Index].Index[0]].m_UVs[0].X;
        UV[1].X = rMesh->m_pVerts[s_TriData[Index].Index[1]].m_UVs[0].Y;
        UV[1].Y = 1.0f - rMesh->m_pVerts[s_TriData[Index].Index[1]].m_UVs[0].X;
        UV[2].X = rMesh->m_pVerts[s_TriData[Index].Index[2]].m_UVs[0].Y;
        UV[2].Y = 1.0f - rMesh->m_pVerts[s_TriData[Index].Index[2]].m_UVs[0].X;
    }

    s_TriData[Index].UVs[0] = UV[0];
    s_TriData[Index].UVs[1] = UV[1];
    s_TriData[Index].UVs[2] = UV[2];
}

//==========================================================================

static
void CopyTriData( mesh::chunk& rMesh )
{
    s32 i;
    s32 count = 0;

    //---   count the number of triangles we need
    for ( i = 0; i < rMesh.m_nFaces; i++ )
    {
        if ( rMesh.m_pFaces[i].m_bQuad )
            count += 2;
        else
            count++;
    }

    //---   allocate space for the triangle data
    s_TriData = (t_Triangle*)x_malloc( sizeof(t_Triangle) * count );
    ASSERT( s_TriData );

    //---   copy the triangle data
    count = 0;
    for ( i = 0; i < rMesh.m_nFaces; i++ )
    {
        if ( rMesh.m_pFaces[i].m_bQuad )
        {
            //---   copy a quad, splitting it into two tris
            s_TriData[count].Index[0] = rMesh.m_pFaces[i].m_Index[0];
            s_TriData[count].Index[1] = rMesh.m_pFaces[i].m_Index[1];
            s_TriData[count].Index[2] = rMesh.m_pFaces[i].m_Index[2];
            s_TriData[count].MaterialID = rMesh.m_pFaces[i].m_iMaterial;
            MakeUV( count, &rMesh );
            count++;
            s_TriData[count].Index[0] = rMesh.m_pFaces[i].m_Index[0];
            s_TriData[count].Index[1] = rMesh.m_pFaces[i].m_Index[2];
            s_TriData[count].Index[2] = rMesh.m_pFaces[i].m_Index[3];
            s_TriData[count].MaterialID = rMesh.m_pFaces[i].m_iMaterial;
            MakeUV( count, &rMesh );
            count++;
        }
        else
        {
            if ( (rMesh.m_pFaces[i].m_Index[0] == rMesh.m_pFaces[i].m_Index[1]) ||
                 (rMesh.m_pFaces[i].m_Index[0] == rMesh.m_pFaces[i].m_Index[2]) ||
                 (rMesh.m_pFaces[i].m_Index[1] == rMesh.m_pFaces[i].m_Index[2]) )
            {
                //---   skip degenerate triangles
                continue;
            }
            //---   just copy the tri
            s_TriData[count].Index[0] = rMesh.m_pFaces[i].m_Index[0];
            s_TriData[count].Index[1] = rMesh.m_pFaces[i].m_Index[1];
            s_TriData[count].Index[2] = rMesh.m_pFaces[i].m_Index[2];
            s_TriData[count].MaterialID = rMesh.m_pFaces[i].m_iMaterial;
            MakeUV( count, &rMesh );
            count++;
        }
    }

    s_NTris = count;
}

//==========================================================================

static
s32 CompareMaterialIDs( const void* Element1, const void* Element2 )
{
    if ( ((t_Triangle*)Element1)->MaterialID < ((t_Triangle*)Element2)->MaterialID )
        return -1;
    else
    if ( ((t_Triangle*)Element1)->MaterialID > ((t_Triangle*)Element2)->MaterialID )
        return 1;
    else
        return 0;
}

//==========================================================================

static
s32 CountTrisWithSameMaterial( s32 StartIndex )
{
    s32 i;
    s32 count = 1;
    s32 CurMaterial = s_TriData[StartIndex].MaterialID;

    for ( i = StartIndex + 1; i < s_NTris; i++ )
    {
        if ( s_TriData[i].MaterialID != CurMaterial )
            break;

        s_SubMeshTriData[count] = i;
        count++;
    }
    return count;
}

//==========================================================================

static
xbool StripsLeftInList( xbool* pBools, s32 NStrips )
{
    s32 i;
    for ( i = 0; i < NStrips; i++ )
    {
        if ( pBools[i] == FALSE )
            return TRUE;    // still a strip left that hasn't been added
    }
    return FALSE;           // no strips left in list
}

//==========================================================================

static
s32 BuildVertPackagesFromStrips( t_TriangleStrip*   pStrips,
                                 s32                NStrips,
                                 t_StripPackage**   pPackages,
                                 s32&               rNPackages,
                                 xbool              EnvMapped )
{
    s32                 i;
    xbool*              pStripsAdded = NULL;
    s32                 NVertsAvailable;
    t_StripPackage*     PackageArray;

    //---   create temporary storage for our boolean list
    pStripsAdded = (xbool*)x_malloc( NStrips * sizeof(xbool) );
    ASSERT( pStripsAdded );
    for ( i = 0; i < NStrips; i++ )
        pStripsAdded[i] = FALSE;

    //---   create storage for our package list
    *pPackages = (t_StripPackage*)x_malloc( MAX_NUM_PACKAGES * sizeof(t_StripPackage) );
    ASSERT( *pPackages );
    PackageArray = *pPackages;

    //---   initialize some variables before we begin
    rNPackages = 0;
    NVertsAvailable = 0;

    // Look through all of the strips first to get a general count of the number of verts we'll be adding
    s32 nTotalVerts = 0;
    for (i=0; i<NStrips; ++i)
    {
        nTotalVerts += pStrips[ i ].NVerts;
    }
    // now use that total to calculate whether this object will be double-buffered or triple-buffered
    f32 fTotalBuffers = (f32)nTotalVerts * (1.0f / (f32)kMaxVertsInPackage[ 1 ]);

    s32 nBufferIndex = 1;   // assume we are double buffering
    if (fTotalBuffers > 6)
    {
        nBufferIndex = 0;   // change it to triple buffering
    }

    //---   use a greedy algorithm to add the verts to packages
    s32 CurrentQuad;
    for( CurrentQuad = 0; CurrentQuad < NStrips; CurrentQuad++ )
    {
        s32 LongestStrip;
        s32 StripLength;

        //---   find the longest strip that will fit in the available space
        LongestStrip = CurrentQuad;
        StripLength = 4;
        // find the first strip that has yet to be added to the list

        for (i=0; i<NStrips; ++i)
        {
            if (pStripsAdded[ i ] == FALSE)
            {
                LongestStrip = i;
                StripLength = pStrips[ i ].NVerts;
                break;
            }
        }
        // now go back over the strips and see if there's a better one.
        for (i = 0; i < NStrips; ++i)
        {
            if ( (pStrips[i].NVerts <= NVertsAvailable) &&
                 (pStripsAdded[i] == FALSE) &&
                 (pStrips[i].NVerts > StripLength) )
            {
                LongestStrip = i;
                StripLength = pStrips[i].NVerts;
            }
        }


        //---   if we couldn't find a strip to fit into the available space,
        //      close out this package, create a new one, and wait for the
        //      loop to try again
        if (pStrips[ LongestStrip ].NVerts > NVertsAvailable)
        {
            rNPackages++;
            ASSERT( rNPackages < MAX_NUM_PACKAGES );
            if ( EnvMapped )
                NVertsAvailable = kMaxVertsInPackageEnvMap[ nBufferIndex ];
            else
                NVertsAvailable = kMaxVertsInPackage[ nBufferIndex ];
            PackageArray[rNPackages-1].NTris = 0;
            PackageArray[rNPackages-1].NVerts = 0;
            for ( i = 0; i < MAX_VERTS_IN_PACKAGE; i++ )
            {
                PackageArray[rNPackages-1].Verts[i].KickEnabled = TRUE;
                PackageArray[rNPackages-1].Verts[i].VertIndex = -1;
            }
        }

        //---   add the strip to the package
        s32             CurVert;
        t_StripVert*    VertArray;

        //---   get some variables to start off with
        CurVert = PackageArray[rNPackages-1].NVerts;
        VertArray = PackageArray[rNPackages-1].Verts;

        //---   disable the kick on the first two verts, to keep the
        //      previous strip from connecting to the current one
        VertArray[CurVert].KickEnabled = FALSE;
        VertArray[CurVert+1].KickEnabled = FALSE;

        //---   now add the verts
        for ( i = 0; i < pStrips[LongestStrip].NVerts; i++ )
        {
            VertArray[CurVert].VertIndex = pStrips[LongestStrip].pVerts[i];
            VertArray[CurVert].UV = pStrips[LongestStrip].UVs[i];
            CurVert++;
        }

        //---   store back our variables
        PackageArray[rNPackages-1].NVerts = CurVert;
        if ( EnvMapped )
        {
            ASSERT( PackageArray[rNPackages-1].NVerts <= (s32)kMaxVertsInPackageEnvMap[ nBufferIndex ] );
            NVertsAvailable = kMaxVertsInPackageEnvMap[ nBufferIndex ] - PackageArray[rNPackages-1].NVerts;
        }
        else
        {
            ASSERT( PackageArray[rNPackages-1].NVerts <= (s32)kMaxVertsInPackage[ nBufferIndex ] );
            NVertsAvailable = kMaxVertsInPackage[ nBufferIndex ] - PackageArray[rNPackages-1].NVerts;
        }

        //---   add to our stats
        PackageArray[rNPackages-1].NTris += pStrips[LongestStrip].NVerts - 2;

        //---   mark this strip as having been added, and move on
        pStripsAdded[LongestStrip] = TRUE;
    }

    //---   free up any temporary storage
    x_free( pStripsAdded );

    return nBufferIndex;
}

//==========================================================================

static
void ComputeMipValues( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex )
{
    s32                 i, j;
    f32                 MinPixelSize;
    f32                 MaxPixelSize;
    f32                 TotalPixelSize;
    s32                 MaterialID;
    mesh::material*     pMaterial;
    mesh::stage*        pStage;

    //---   grab some useful pointers
    MaterialID = s_TriData[StartTri].MaterialID;
    if ( MaterialID >= 0 )
    {
        // #### Change this to enable multi-pass rendering!
        pMaterial = &s_pMaterialSet->m_pMaterials[MaterialID];
        pStage = &s_pMaterialSet->m_pStages[0];
    }
    else
    {
        pMaterial = NULL;
        pStage = NULL;
    }

    //---   get some starting values
    MinPixelSize   =  99999999.0f;
    MaxPixelSize   = -99999999.0f;
    TotalPixelSize = 0.0f;

    //---   Loop through tris in submesh
    for ( i = StartTri; i < (StartTri + NTris); i++ )
    {
        f32             PixelSize;
        t_Triangle*     pTri;
        vector3         P[3];
        vector2         UV[3];
        f32             WArea;
        f32             PArea;
        vector3         E1,E2,N;
        
        //---   grab a ptr to the tri
        pTri = &s_TriData[i];

        //---   get the UV's and positions for this tri
        for ( j = 0; j < 3; j++ )
        {
            mesh::vertex* pVert;

            //---   geta ptr to the vert
            pVert = &rGeom.m_pChunks[MeshIndex].m_pVerts[pTri->Index[j]];

            P[j] = pVert->m_vPos;
            UV[j] = pVert->m_UVs[0];
            UV[j].X *= s_pMaterialSet->m_pTextures[pStage->m_iTexture].m_Width;
            UV[j].Y *= s_pMaterialSet->m_pTextures[pStage->m_iTexture].m_Height;
        }

        //---   calculate the squared area in inches
        E1 = P[1] - P[0];
        E2 = P[2] - P[0];
        N  = Cross(E1,E2);
        WArea = N.Length() * 0.5f;

        //---   calculate the squared area in pixels
        E1 = UV[1] - UV[0];
        E2 = UV[2] - UV[0];
        N  = Cross(E1,E2);
        PArea = N.Length() * 0.5f;

        //---   now we can get the pixel size
        if( PArea > 0.00001f )
        {
            PixelSize = x_sqrt(WArea) / x_sqrt(PArea);
        }
        else
        {
            PixelSize = 0;
        }

        //---   update statistics
        if( PixelSize > 0.0001f )
        {
            MinPixelSize = MIN(MinPixelSize,PixelSize);
            MaxPixelSize = MAX(MaxPixelSize,PixelSize);
            TotalPixelSize += PixelSize;
        }
    }

    //---   update the current sub mesh
    s_SubMeshData[s_CurSubMesh].MinPixelSize = MinPixelSize;
    s_SubMeshData[s_CurSubMesh].MaxPixelSize = MaxPixelSize;
    if ( NTris >= 0 )
        s_SubMeshData[s_CurSubMesh].AvgPixelSize = TotalPixelSize / NTris;
}

//==========================================================================

static
s32 GetTextureID( s32 TriID, mesh::object& rGeom )
{
    s32 MaterialID;

    MaterialID = s_TriData[TriID].MaterialID;
    if ( MaterialID >= 0 )
    {
        mesh::material*         pMaterial;
        mesh::stage*     pStage;

        pMaterial = &s_pMaterialSet->m_pMaterials[MaterialID];
        pStage = &s_pMaterialSet->m_pStages[pMaterial->m_iStages[0]];

        if( pStage->m_iTexture < 0 )
            return 0;
        return pStage->m_iTexture;
    }
    else
    {
        return -1;
    }
}

//==========================================================================
static s32 SaveVIFUnpackInfo1( byte* pD, s32 iIndex = 0, xbool bTriple = FALSE, xbool bEnvMap = FALSE, xbool bCompressedColors = FALSE, X_FILE *pDump = NULL );

static
s32 SaveVIFUnpackInfo1( byte* pD, s32 iIndex, xbool bTriple , xbool bEnvMap, xbool bCompressedColors, X_FILE *pDump )
{
    u32*    pVIF;

    //---   this vif unpack info is used for the CMD and GIF tags, and
    //      it also fills in the filling data used by any masks

// TODO::

    pVIF = (u32*)pD;
    pVIF[ 0 ] = VIFHELP_SET_STROW(0);
    if (bCompressedColors)
    {
        pVIF[ 1 ] = (u32)((s_CompressColor.X * s_CompressColor.X) * 192.0f);
        pVIF[ 2 ] = (u32)((s_CompressColor.Y * s_CompressColor.Y) * 192.0f);
        pVIF[ 3 ] = (u32)((s_CompressColor.Z * s_CompressColor.Z) * 192.0f);
        pVIF[ 4 ] = (u32)((s_CompressColor.W) * 128.0f);
    }
    else
    {
        *((f32*)(&pVIF[ 1 ])) = 1.0f;
        *((f32*)(&pVIF[ 2 ])) = 1.0f;
        *((f32*)(&pVIF[ 3 ])) = 1.0f;
        *((f32*)(&pVIF[ 4 ])) = 1.0f;
    }
    pVIF[ 5 ] = VIFHELP_SET_STCYCL( 1, 1, 0 );
    pVIF[ 6 ] = 0;
    u32 addr = (iIndex * kBufferSize);
    if (bCompressedColors)
    {
        PS2HELP_BuildVIFTagUnpack( &pVIF[ 7 ], addr, 5, VIFHELP_UNPACK_V4_32, FALSE, FALSE, bTriple );
    }
    else
    {
        PS2HELP_BuildVIFTagUnpack( &pVIF[ 7 ], addr, 3, VIFHELP_UNPACK_V4_32, FALSE, FALSE, bTriple );
    }



    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "STROW : 0x%08X  :  0x%08X, 0x%08X, 0x%08X, 0x%08X\n", pVIF[ 0 ], pVIF[ 1 ], pVIF[ 2 ], pVIF[ 3 ], pVIF[ 4 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "STCYCL : 0x%08X\n", pVIF[ 5 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "PAD : 0x%08X\n", pVIF[ 6 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "UNPACK : 0x%08X\n", pVIF[ 7 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }

    return (sizeof(u32) * 8);
}

//==========================================================================
static s32 SaveVIFUnpackInfo2( byte* pD, s32 NVerts, xbool MaskW, s32 iIndex = 0, xbool bTriple = FALSE, xbool bEnvMap = FALSE, s32 nOffset = 0, xbool bCompressedColors = FALSE, X_FILE *pDump = NULL );

static
s32 SaveVIFUnpackInfo2( byte* pD, s32 NVerts, xbool MaskW, s32 iIndex, xbool bTriple, xbool bEnvMap, s32 nOffset, xbool bCompressedColors, X_FILE *pDump )
{
    u32*    pVIF = (u32*)pD;

    //---   this vif unpack info is used for the position unpacking

    u32 bufferSize = 0;
    u32 addr = 5 + ((bEnvMap) ? 1 : 0) + (iIndex * kBufferSize) + nOffset;

    pVIF[1] = 0;
    if (bCompressedColors)
    {
        if (bEnvMap)
            pVIF[0] = VIFHELP_SET_STCYCL( 4, 1, 0 );
        else
            pVIF[0] = VIFHELP_SET_STCYCL( 3, 1, 0 );
        if ( MaskW )
        {
            PS2HELP_BuildVIFTagMask( &pVIF[1], 1, 0, 0, 0,
                                               1, 1, 1, 1,
                                               1, 1, 1, 1,
                                               1, 1, 1, 1 );
        }
    }
    else
    {
        if (bEnvMap)
            pVIF[0] = VIFHELP_SET_STCYCL( 1, 4, 0 );
        else
            pVIF[0] = VIFHELP_SET_STCYCL( 1, 3, 0 );
        if ( MaskW )
        {
            PS2HELP_BuildVIFTagMask( &pVIF[1], 1, 0, 0, 0,
                                               1, 0, 0, 0,
                                               1, 0, 0, 0,
                                               1, 0, 0, 0 );
        }
    }

    if ( MaskW )
    {
        PS2HELP_BuildVIFTagUnpack( &pVIF[3], addr, NVerts, VIFHELP_UNPACK_V3_32, FALSE, TRUE, bTriple );
    }
    else
    {
        PS2HELP_BuildVIFTagUnpack( &pVIF[3], addr, NVerts, VIFHELP_UNPACK_V4_32, FALSE, FALSE, bTriple );
    }

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "STCYCL : 0x%08X\n", pVIF[ 0 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "MASK | PAD : 0x%08X\n", pVIF[ 1 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "PAD | MASKDATA : 0x%08X\n", pVIF[ 2 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "UNPACK : 0x%08X\n", pVIF[ 3 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }
    return (sizeof(u32) * 4);
}

//==========================================================================
static s32 SaveVIFUnpackInfo2_5( byte* pD, s32 NVerts, xbool MaskW, X_FILE *pDump = NULL );

static
s32 SaveVIFUnpackInfo2_5( byte* pD, s32 NVerts, xbool MaskW, X_FILE *pDump )
{
    u32*    pVIF = (u32*)pD;

    //---   this vif unpack info is used for the position unpacking for SHADOW objects *ONLY*
    u32 addr = 4;

    pVIF[1] = 0;
    pVIF[0] = VIFHELP_SET_STCYCL( 1, 2, 0 );
    if ( MaskW )
    {
        PS2HELP_BuildVIFTagMask( &pVIF[1], 1, 0, 0, 0,
                                           1, 0, 0, 0,
                                           1, 0, 0, 0,
                                           1, 0, 0, 0 );
    }
    PS2HELP_BuildVIFTagUnpack( &pVIF[3], addr, NVerts, VIFHELP_UNPACK_V3_32, FALSE, MaskW, FALSE );

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "STCYCL : 0x%08X\n", pVIF[ 0 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "MASK | PAD : 0x%08X\n", pVIF[ 1 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "PAD | MASKDATA : 0x%08X\n", pVIF[ 2 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "UNPACK : 0x%08X\n", pVIF[ 3 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }
    return (sizeof(u32) * 4);
}

//==========================================================================

static s32 RecoverFromCompress( byte* pD, X_FILE *pDump=NULL );

static 
s32 RecoverFromCompress( byte* pD, X_FILE *pDump )
{
    u32*    pVIF;
    pVIF = (u32*)pD;
    pVIF[0] = VIFHELP_SET_STROW(0);
    *((f32*)(&pVIF[1])) = 1.0f;
    *((f32*)(&pVIF[2])) = 1.0f;
    *((f32*)(&pVIF[3])) = 1.0f;
    *((f32*)(&pVIF[4])) = 1.0f;

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "STROW : 0x%08X  :  0x%08X, 0x%08X, 0x%08X, 0x%08X\n", pVIF[ 0 ], pVIF[ 1 ], pVIF[ 2 ], pVIF[ 3 ], pVIF[ 4 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }

    return (sizeof( u32 ) * 5);
}

//==========================================================================
static s32 SaveVIFUnpackInfo2_5( byte* pD, s32 NVerts, xbool MaskW, s32 iIndex = 0, xbool bTriple = FALSE, xbool bEnvMap = FALSE, X_FILE *pDump = NULL );

static
s32 SaveVIFUnpackInfo2_5( byte* pD, s32 NVerts, s32 iIndex, xbool bTriple, xbool bEnvMap, X_FILE *pDump )
{
    u32*    pVIF = (u32*)pD;

    //---   this vif unpack info is used for the position ADC bit unpacking

    u32 bufferSize = 0;
    u32 addr = 5 + ((bEnvMap) ? 1 : 0) + (iIndex * kBufferSize);

    if (bEnvMap)
        pVIF[0] = VIFHELP_SET_STCYCL( 1, 4, 0 );
    else
        pVIF[0] = VIFHELP_SET_STCYCL( 1, 3, 0 );

    pVIF[1] = 0;
    pVIF[2] = 0;
    PS2HELP_BuildVIFTagMask( &pVIF[1], 0, 3, 3, 3,          // only write in W
                                       0, 3, 3, 3,          // everything else is masked out
                                       0, 3, 3, 3,
                                       0, 3, 3, 3 );
    PS2HELP_BuildVIFTagUnpack( &pVIF[3], addr, NVerts, VIFHELP_UNPACK_S_16, FALSE, TRUE, bTriple );









    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "MASK : 0x%08X\n", pVIF[ 0 ] );


        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "MASKDATA : 0x%08X\n", pVIF[ 1 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "UNPACK : 0x%08X\n", pVIF[ 2 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }
    return (sizeof(u32) * 4);
}

//==========================================================================
static s32 SaveVIFUnpackInfo3( byte* pD, s32 NVerts, s32 iIndex = 0, xbool bTriple = FALSE, X_FILE *pDump = NULL );

static
s32 SaveVIFUnpackInfo3( byte* pD, s32 NVerts, s32 iIndex, xbool bTriple, X_FILE *pDump )
{
    u32*    pVIF = (u32*)pD;

    //---   this vif unpack info is used for UVs

    u32 bufferSize = 0;
    u32 addr = 3 + (iIndex * kBufferSize);

    pVIF[0] = 0;
    pVIF[1] = 0;
    pVIF[2] = 0;
    PS2HELP_BuildVIFTagMask( &pVIF[0], 1, 1, 0, 0,
                                       1, 1, 0, 0,
                                       1, 1, 0, 0,
                                       1, 1, 0, 0 );
#if USE_16_BIT_UVS
    PS2HELP_BuildVIFTagUnpack( &pVIF[2], addr, NVerts, VIFHELP_UNPACK_V2_16, FALSE, TRUE, bTriple );
#else
    PS2HELP_BuildVIFTagUnpack( &pVIF[2], addr, NVerts, VIFHELP_UNPACK_V2_32, FALSE, TRUE, bTriple );
#endif

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "MASK : 0x%08X\n", pVIF[ 0 ] );


        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "MASKDATA : 0x%08X\n", pVIF[ 1 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "UNPACK : 0x%08X\n", pVIF[ 2 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }
    return (sizeof(u32) * 3);
}

//==========================================================================
static s32 SaveVIFUnpackInfo4( byte* pD, s32 NVerts, s32 iIndex = 0, xbool bTriple = FALSE, X_FILE *pDump = NULL );

static
s32 SaveVIFUnpackInfo4( byte* pD, s32 NVerts, s32 iIndex, xbool bTriple, X_FILE *pDump )
{
    u32*    pVIF = (u32*)pD;

    u32 bufferSize = 0;
    u32 addr = 4 + (iIndex * kBufferSize);

    //---   this vif unpack info is used for normals
    pVIF[0] = 0;
    pVIF[1] = 0;
    PS2HELP_BuildVIFTagMask( &pVIF[0], 1, 0, 0, 0,
                                       1, 0, 0, 0,
                                       1, 0, 0, 0,
                                       1, 0, 0, 0 );
    PS2HELP_BuildVIFTagUnpack( &pVIF[2], addr, NVerts, VIFHELP_UNPACK_V3_32, FALSE, TRUE, bTriple );

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "MASK : 0x%08X\n", pVIF[ 0 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "MASKDATA : 0x%08X\n", pVIF[ 1 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        x_sprintf( dump, "UNPACK : 0x%08X\n", pVIF[ 2 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );


    }
    return (sizeof(u32) * 3);
}

//==========================================================================
static s32 SaveVIFUnpackInfo5( byte* pD, s32 NVerts, s32 iIndex = 0, xbool bTriple = FALSE, xbool bEnvMap = FALSE, X_FILE *pDump = NULL );

static
s32 SaveVIFUnpackInfo5( byte* pD, s32 NVerts, s32 iIndex, xbool bTriple, xbool bEnvMap, X_FILE *pDump )
{
    u32*    pVIF = (u32*)pD;

    u32 bufferSize = 0;
    u32 addr = 4 + ((bEnvMap) ? 1 : 0) + (iIndex * kBufferSize);

    //---   this vif unpack info is used for RGBAs
    PS2HELP_BuildVIFTagUnpack( &pVIF[ 0 ], addr, NVerts, VIFHELP_UNPACK_V4_8, FALSE, FALSE, bTriple );




    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "UNPACK : 0x%08X\n", pVIF[ 0 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );






    }
    return (sizeof(u32));
}

//==========================================================================

static
s32 SaveCMDInfo( xbool bQuad, byte* pD, xbool Shadow, xbool Clipped, xbool Lit, xbool EnvMapped, s32 NVerts )
{
    u32*    pCMD;

    pCMD = (u32*)pD;
    if ( Shadow )
    {
        pCMD[0] = GEOM_OPCODE_TRI_SHADOW;
    }
    else
    if ( EnvMapped )
    {
        pCMD[0] = GEOM_OPCODE_TRI_ENVMAPPED;
    }
    else
    if ( Clipped )
    {
        if ( Lit )
        {
            ASSERT( !bQuad );
            pCMD[0] = GEOM_OPCODE_TRI_CLIP_LIGHT;     // opcode
        }
        else
        {
            if( bQuad )
                pCMD[0] = GEOM_OPCODE_QUAD_CLIP_NOLIGHT;
            else
                pCMD[0] = GEOM_OPCODE_TRI_CLIP_NOLIGHT;
        }
    }
    else
    {
        ASSERT( !bQuad );
        if ( Lit )
            pCMD[0] = GEOM_OPCODE_TRI_NOCLIP_LIGHT;     // opcode
        else
            pCMD[0] = GEOM_OPCODE_TRI_NOCLIP_NOLIGHT;
    }
    pCMD[1] = 0;                                    // offset
    pCMD[2] = NVerts;                               // Param1, # verts
    pCMD[3] = 0;                                    // Param2, unused

    return (sizeof(u32) * 4);
}

//==========================================================================

#define GIF_TYPE_CLIPPED    0
#define GIF_TYPE_STRIP      1
#define GIF_TYPE_SHADOW     2
#define GIF_TYPE_ENVMAP     3

static s32 SaveGIFInfo( byte* pD, s32 GifInfoType, s32 NVerts, xbool bCompressedColors = FALSE, X_FILE *pDump = NULL );

static
s32 SaveGIFInfo( byte* pD, s32 GifInfoType, s32 NVerts, xbool bCompressedColors, X_FILE *pDump )
{
    s32         NRegs;
    u32         *pCount = (u32*)pD;
    pCount[0] = NVerts;
    pCount[1] = 0;
    pCount[2] = 0;
    pCount[3] = 0;
    ASSERTS( NVerts>0, "Vert count for tri strip is 0!" );
    ASSERTS( NVerts <= (s32)kMaxVertsInPackage[1], "Vert count too large!" );

    if (NVerts <= 0)
        return 0;
    if (NVerts > (s32)kMaxVertsInPackage[1])
        return 0;

    pD += sizeof(u32)*4;
    sceGifTag*  pGIF = (sceGifTag*)pD;
    s32         PrimType;
    u32         PrimFlags;
    s32         Reg0;
    s32         Reg1;
    s32         Reg2;
    s32         Reg3;

    //---   get some information for setting up the gif tags
    switch ( GifInfoType )
    {
    case GIF_TYPE_CLIPPED:
        NRegs       = 3;
        PrimType    = VIFHELP_GIFPRIMTYPE_TRIANGLESTRIP;
        PrimFlags   = VIFHELP_GIFPRIMFLAGS_TEXTURE | VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE;
        Reg0        = VIFHELP_GIFREG_ST;
        Reg1        = VIFHELP_GIFREG_RGBAQ;
        Reg2        = VIFHELP_GIFREG_XYZ2;
        break;
    case GIF_TYPE_ENVMAP:
        NRegs       = 4;
        PrimType    = VIFHELP_GIFPRIMTYPE_TRIANGLESTRIP;
        PrimFlags   = VIFHELP_GIFPRIMFLAGS_TEXTURE | VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE;
        Reg0        = VIFHELP_GIFREG_ST;
        Reg1        = VIFHELP_GIFREG_NOP;
        Reg2        = VIFHELP_GIFREG_RGBAQ;
        Reg3        = VIFHELP_GIFREG_XYZ2;
        break;
    case GIF_TYPE_STRIP:
        NRegs       = 3;
        PrimType    = VIFHELP_GIFPRIMTYPE_TRIANGLESTRIP;
        PrimFlags   = VIFHELP_GIFPRIMFLAGS_TEXTURE | VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE;
        Reg0        = VIFHELP_GIFREG_ST;
        Reg1        = VIFHELP_GIFREG_RGBAQ;
        Reg2        = VIFHELP_GIFREG_XYZ2;
        break;
    case GIF_TYPE_SHADOW:
        NRegs       = 2;
        PrimType    = VIFHELP_GIFPRIMTYPE_TRIANGLE;
        PrimFlags   = VIFHELP_GIFPRIMFLAGS_ALPHA;
        Reg0        = VIFHELP_GIFREG_RGBAQ;
        Reg1        = VIFHELP_GIFREG_XYZ2;
        Reg2        = 0;
        break;
    default:
        ASSERT( FALSE );
        return 0;
    }

    //---   add the alpha and anti-alias flags if necessary
    if ( GifInfoType != GIF_TYPE_SHADOW )
    {
        if ( g_EnableAlpha )
            PrimFlags |= VIFHELP_GIFPRIMFLAGS_ALPHA;
        if ( g_EnableAA )
            PrimFlags |= VIFHELP_GIFPRIMFLAGS_ANTIALIAS;
    }

    if (GifInfoType == GIF_TYPE_ENVMAP)
    {
        //---   our "normal" GIFtag is now the SECOND tag
        //      env GIFtag
        //   ==>normal GIFtag       // Kick happens from here
        //       vert data
        //       vert data
        //       lots-o-vert data
        //
        //  Then the VU microcode swaps the two tags...
        //      normal GIFtag
        //   ==>env GIFtag          // Kick happens here
        //       same vert data
        //       nothing new here data
        //       lots-o-vert data

        // vert format:
        //  ST      for diffuse texture
        //  ST      for EnvMap texture
        //  RGBAQ
        //  XYZ2

        // flags are set for the normal, so stuff the normal into slot 1 first
        PS2HELP_BuildGIFTag1( &pGIF[1],
                            VIFHELP_GIFMODE_PACKED,
                            NRegs,
                            NVerts,
                            TRUE,
                            PrimType,
                            PrimFlags,
                            TRUE );
        // Setup regs to ignore EnvMap ST values
        PS2HELP_BuildGIFTag2( &pGIF[1], Reg0, Reg1, Reg2, Reg3 );

        // now do slot 0 with the Environment Map settings
//        PrimFlags |= VIFHELP_GIFPRIMFLAGS_ALPHA;
//        PrimFlags |= VIFHELP_GIFPRIMFLAGS_CONTEXT;
        PS2HELP_BuildGIFTag1( &pGIF[0],
                              VIFHELP_GIFMODE_PACKED,
                              NRegs,
                              NVerts,
                              TRUE,
                              PrimType,
                              PrimFlags,
                              TRUE );
        // Swap Reg0 and Reg1 
        //  forces GS to ignore Diffuse ST values in favor of EnvMap
        PS2HELP_BuildGIFTag2( &pGIF[0], Reg1, Reg0, Reg2, Reg3 );
    }
    else
    {
        //---   export the stall GIF tag   (used for stalling the GIF and for clipped tris)
        PS2HELP_BuildGIFTag1( &pGIF[0],
                              VIFHELP_GIFMODE_PACKED,
                              NRegs,
                              0,    // this MUST be zero for stalling the GS
                              TRUE,
                              PrimType,
                              PrimFlags,
                              TRUE );
        PS2HELP_BuildGIFTag2( &pGIF[0], Reg0, Reg1, Reg2, 0 );

        //---   export the strip's GIF tag    (used 99% of the time)
        PS2HELP_BuildGIFTag1( &pGIF[1],
                              VIFHELP_GIFMODE_PACKED,
                              NRegs,
                              NVerts,
                              TRUE,
                              PrimType,
                              PrimFlags,
                              TRUE );
        PS2HELP_BuildGIFTag2( &pGIF[1], Reg0, Reg1, Reg2, 0 );
    }

    pD += sizeof( sceGifTag ) * 2;

    if (bCompressedColors)
    {
        u32 *pColor = (u32*)pD;
        pColor[ 0 ] = (u32)((s_CompressColor.X * s_CompressColor.X) * 192.0f);
        pColor[ 1 ] = (u32)((s_CompressColor.Y * s_CompressColor.Y) * 192.0f);
        pColor[ 2 ] = (u32)((s_CompressColor.Z * s_CompressColor.Z) * 192.0f);
        pColor[ 3 ] = (u32)((s_CompressColor.W) * 128.0f);
        pColor[ 4 ] = (u32)((s_CompressColor.X * s_CompressColor.X) * 192.0f);
        pColor[ 5 ] = (u32)((s_CompressColor.Y * s_CompressColor.Y) * 192.0f);
        pColor[ 6 ] = (u32)((s_CompressColor.Z * s_CompressColor.Z) * 192.0f);
        pColor[ 7 ] = (u32)((s_CompressColor.W) * 128.0f);
    }

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "RAW_DATA : 0x%08X 0x%08X 0x%08X 0x%08X\n", pCount[ 0 ], pCount[ 1 ], pCount[ 2 ], pCount[ 3 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        pCount += 4;
        x_sprintf( dump, "GIFTAG1 : 0x%08X 0x%08X 0x%08X 0x%08X\n", pCount[ 0 ], pCount[ 1 ], pCount[ 2 ], pCount[ 3 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
        pCount += 4;
        x_sprintf( dump, "GIFTAG2 : 0x%08X 0x%08X 0x%08X 0x%08X\n", pCount[ 0 ], pCount[ 1 ], pCount[ 2 ], pCount[ 3 ] );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }
    
    if (bCompressedColors)
        return (sizeof(sceGifTag) * 2) + (sizeof( u32 ) * 12);
    else
        return (sizeof(sceGifTag) * 2) + (sizeof( u32 ) * 4);
}

//==========================================================================
static s32 SavePosition( byte* pD, mesh::vertex* pVert, xbool ExportADC, xbool KickEnabled, X_FILE *pDump = FALSE );

static
s32 SavePosition( byte* pD, mesh::vertex* pVert, xbool ExportADC, xbool KickEnabled, X_FILE *pDump )
{


    //---   save a ptr to the vert
    if ( g_ExportPosPtrs )
        s_PosPtrs[s_NPosPtrs++] = (u32)(pD - s_PacketData);

    vector3 TempVector;

    f32*    pPOS;

    //---   add the position
    pPOS = (f32*)pD;
    pPOS[0] = pVert->m_vPos.X;
    pPOS[1] = pVert->m_vPos.Y;
    pPOS[2] = pVert->m_vPos.Z;
    
    if ( ExportADC )
    {
        // "hide" the ADC bit in the W position, the microcode will take care of the rest
        if ( KickEnabled )
            *((u32*)(&pPOS[3])) = 0x0000;       // ADC bit is off
        else
            *((u32*)(&pPOS[3])) = 0x8005;       // ADC bit is on
    }

    if (pDump)
    {
        char dump[ 256 ];
        if (ExportADC)
            x_sprintf( dump, "POS : 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", *((u32*)(&pPOS[ 0 ])), *((u32 *)(&pPOS[ 1 ])), *((u32 *)(&pPOS[ 2 ])), *((u32 *)(&pPOS[ 3 ])) );
        else
            x_sprintf( dump, "POS : 0x%08X, 0x%08X, 0x%08X\n", *((u32*)(&pPOS[ 0 ])), *((u32 *)(&pPOS[ 1 ])), *((u32 *)(&pPOS[ 2 ])) );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }

    if ( ExportADC )
        return (sizeof(f32) * 4);
    else
        return (sizeof(f32) * 3);
}

//==========================================================================
static s32 SaveADC( byte* pD, mesh::vertex* pVert, xbool KickEnabled, X_FILE *pDump = FALSE );

static
s32 SaveADC( byte* pD, mesh::vertex* pVert, xbool KickEnabled, X_FILE *pDump )
{
    s16*    pADC;

    //---   add the position
    pADC = (s16*)pD;
    // "hide" the ADC bit in the W position, the microcode will take care of the rest
    if ( KickEnabled )
        *((u16*)(&pADC[0])) = 0x0000;       // ADC bit is off
    else
        *((u16*)(&pADC[0])) = 0x8005;       // ADC bit is on

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "ADC : 0x%04X\n", *((u16*)(&pADC[ 0 ])) );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }

    return (sizeof(s16));
}

//==========================================================================
static s32 SaveUV( byte* pD, vector2* UV, xbool bTriple = FALSE, X_FILE *pDump = NULL );

static
s32 SaveUV( byte* pD, vector2* UV, xbool bTriple, X_FILE *pDump )
{
#if USE_16_BIT
    s16*    pUV = (s16*)pD;
#else
    f32*    pUV = (f32*)pD;
#endif

    //---   save a ptr to the UV
    if ( g_ExportUVPtrs )
        s_UVPtrs[s_NUVPtrs++] = (u32)(pD - s_PacketData);

    //---   add the UVs
#if USE_16_BIT_UVS
    f32 temp = UV->X;
    temp *= 4096.0f;
    pUV[ 0 ] = (s16)temp;
    temp = UV->Y;
    temp *= 4096.0f;
    pUV[ 1 ] = (s16)temp;
#else
    pUV[ 0 ] = UV->X;
    pUV[ 1 ] = UV->Y;
#endif

    if (pDump)
    {
        char dump[ 256 ];
#if USE_16_BIT_UVS
        x_sprintf( dump, "UV : 0x%04X, 0x%04X\n", *((u16*)(&pUV[ 0 ])), *((u16 *)(&pUV[ 1 ])) );
#else
        x_sprintf( dump, "UV : 0x%08X, 0x%08X\n", *((u32*)(&pUV[ 0 ])), *((u32 *)(&pUV[ 1 ])) );
#endif
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }

#if USE_16_BIT_UVS
    return (sizeof(u16) * 2);
#else
    return (sizeof(f32) * 2);
#endif
}

//==========================================================================
static s32 SaveNormal( byte* pD, mesh::vertex* pVert, X_FILE *pDump = NULL );

static
s32 SaveNormal( byte* pD, mesh::vertex* pVert, X_FILE *pDump )
{
    f32*    pNormal;

    //---   save a ptr to the Normal
    if ( g_ExportNormalPtrs )
        s_NormalPtrs[s_NNormalPtrs++] = (u32)(pD - s_PacketData);

    //---   add the Normal
    pNormal = (f32*)pD;
    pNormal[0] = pVert->m_vNormal.X;
    pNormal[1] = pVert->m_vNormal.Y;
    pNormal[2] = pVert->m_vNormal.Z;

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "NORM : 0x%08X, 0x%08X, 0x%08X\n", *((u32*)(&pNormal[ 0 ])), *((u32 *)(&pNormal[ 1 ])), *((u32 *)(&pNormal[ 2 ])) );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }

    return (sizeof(f32) * 3);
}

//==========================================================================
static s32 SaveRGBA( byte* pD, mesh::vertex* pVert, xbool bTriple = FALSE, X_FILE *pDump = NULL );

static
s32 SaveRGBA( byte* pD, mesh::vertex* pVert, xbool bTriple, X_FILE *pDump )
{
    u8* pRGBA;

    //---   save a ptr to the RGBA
    if ( g_ExportRGBAPtrs )
        s_RGBAPtrs[s_NRGBAPtrs++] = (u32)(pD - s_PacketData);

    //---   add the RGBs
    pRGBA = (u8*)pD;
    pRGBA[0] = (u8)(pVert->m_Color.X * 128.0f);
    pRGBA[1] = (u8)(pVert->m_Color.Y * 128.0f);
    pRGBA[2] = (u8)(pVert->m_Color.Z * 128.0f);
    pRGBA[3] = (u8)(pVert->m_Color.W * 128.0f);

    if (pDump)
    {
        char dump[ 256 ];
        x_sprintf( dump, "COL : 0x%08X\n", *((u32*)(&pRGBA[ 0 ])) );
        x_fwrite( dump, x_strlen( dump ), 1, pDump );
    }
    
    return (sizeof(u8) * 4);
}

//==========================================================================
static s32 SaveVIFKick( byte* pD, u32 iIndex = 0, xbool bTriple = FALSE, xbool bFirst = FALSE, xbool bLast = FALSE, X_FILE *pDump = NULL );

static
s32 SaveVIFKick( byte* pD, u32 iIndex, xbool bTriple, xbool bFirst, xbool bLast, X_FILE *pDump )
{
    u32*    pVIF;
    pVIF = (u32*)pD;

    //---   set up the VIF kick
    if (bTriple)
    {
        pVIF[ 0 ] = VIFHELP_SET_BASE( iIndex * kBufferSize, 0 );
        pVIF[ 1 ] = VIFHELP_SET_OFFSET( iIndex * kBufferSize, 0 );
        pVIF[ 2 ] = VIFHELP_SET_ITOP( iIndex * kBufferSize, 0 );
        pVIF[ 3 ] = VIFHELP_SET_MSCAL( (0x1D8 >> 3), 0);

        if (bLast)
        {
            pVIF[ 4 ] = VIFHELP_SET_BASE( 0, 0 );
            pVIF[ 5 ] = VIFHELP_SET_OFFSET( 360, 0 );
            pVIF[ 6 ] = VIFHELP_SET_ITOP( 0, 0 );
            pVIF[ 7 ] = VIFHELP_SET_FLUSH( 0 );
        }
        else if (bFirst)
        {
            pVIF[ 4 ] = VIFHELP_SET_FLUSH( 0 );
        }
        if (pDump)
        {
            char dump[ 256 ];
            x_sprintf( dump, "BASE : 0x%08X\n", pVIF[ 0 ] );
            x_fwrite( dump, x_strlen( dump ), 1, pDump );
            x_sprintf( dump, "OFFSET : 0x%08X\n", pVIF[ 1 ] );
            x_fwrite( dump, x_strlen( dump ), 1, pDump );
            x_sprintf( dump, "ITOP : 0x%08X\n", pVIF[ 2 ] );
            x_fwrite( dump, x_strlen( dump ), 1, pDump );
            x_sprintf( dump, "MSCAL : 0x%08X\n", pVIF[ 3 ] );
            x_fwrite( dump, x_strlen( dump ), 1, pDump );
            if (bLast)
            {
                x_sprintf( dump, "BASE |PAD : 0x%08X\n", pVIF[ 4 ] );
                x_fwrite( dump, x_strlen( dump ), 1, pDump );
                x_sprintf( dump, "OFFSET | PAD : 0x%08X\n", pVIF[ 5 ] );
                x_fwrite( dump, x_strlen( dump ), 1, pDump );
                x_sprintf( dump, "ITOP | PAD : 0x%08X\n", pVIF[ 6 ] );
                x_fwrite( dump, x_strlen( dump ), 1, pDump );
                x_sprintf( dump, "FLUSH | PAD : 0x%08X\n", pVIF[ 7 ] );
                x_fwrite( dump, x_strlen( dump ), 1, pDump );
            }
            else if (bFirst)
            {
                x_sprintf( dump, "FLUSH | PAD : 0x%08X\n", pVIF[ 4 ] );
                x_fwrite( dump, x_strlen( dump ), 1, pDump );
            }
        }
    }
    else
    {
        pVIF[ 0 ] = VIFHELP_SET_MSCAL( (0x1D8 >> 3), 0);
        pVIF[ 1 ] = VIFHELP_SET_FLUSH( 0 );

        if (pDump)
        {
            char dump[ 256 ];
            x_sprintf( dump, "MSCAL : 0x%08X\n", pVIF[ 0 ] );
            x_fwrite( dump, x_strlen( dump ), 1, pDump );
            x_sprintf( dump, "FLUSH | PAD : 0x%08X\n", pVIF[ 1 ] );
            x_fwrite( dump, x_strlen( dump ), 1, pDump );
        }
    }

    if (bTriple)
    {
        if (bLast)
            return (sizeof(u32) * 8);
        else if (bFirst)
            return (sizeof(u32) * 5);
        else
            return (sizeof(u32) * 4);
    }
    else
        return (sizeof(u32) * 2);
}

//==========================================================================

static
s32 BuildSubMeshTris( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex, s32 CurMesh, t_Triangle* pTris = NULL )
{
    s32                 NTrisAdded = 0;
    byte*               pD;
    sceDmaTag*          pDMA;
    s32                 TextureID;

    //---   figure out which texture to use
    TextureID = GetTextureID( StartTri, rGeom );

    //---   grab a pointer to where we'll be storing the data
    pD = &s_PacketData[s_CurPacketSize];

    //---   set up the DMA tag
    pDMA = (sceDmaTag*)pD;
    PS2HELP_BuildDMATagCont( pDMA, 0 );  // we'll fill this in later!!!!!
    pD += sizeof(sceDmaTag);

    //---   now add the tris
    while ( NTrisAdded < NTris )
    {
        s32         i, j;

        //---   how many verts can we add to this package?
        s32 NVertsToAdd = (NTris - NTrisAdded) * 3;
        if ( NVertsToAdd > MAX_VERTS_IN_PACKAGE )
            NVertsToAdd = MAX_VERTS_IN_PACKAGE;

        //===   now build the PS2 package

        //---   set up the VIF Unpack info for the command and GifTags, and
        //      set up our filling data as well
        pD += SaveVIFUnpackInfo1( pD );

        //---   set up the render CMD
//        pD += SaveCMDInfo( FALSE, pD, FALSE, TRUE, g_DynamicLighting, FALSE, NVertsToAdd );

        //---   export the GIF tags
        pD += SaveGIFInfo( pD, GIF_TYPE_CLIPPED, NVertsToAdd );

        //===   add the vert data

        //---   set up the VIF Unpack info for the positions
        pD += SaveVIFUnpackInfo2( pD, NVertsToAdd, TRUE );

        //---   add the positions
        for ( i = 0; i < NVertsToAdd / 3; i++ )
        {
            t_Triangle* pTri;
            if( pTris )
                pTri = &pTris[NTrisAdded + i];
            else
                pTri = &s_TriData[s_SubMeshTriData[NTrisAdded + i]];

            for ( j = 0; j < 3; j++ )
            {
                mesh::vertex pVert;

                //---   get a ptr to the vert
                pVert = rGeom.m_pChunks[MeshIndex].m_pVerts[pTri->Index[j]];

                //---   save the position
                pD += SavePosition( pD, &pVert, FALSE, FALSE );
            }
        }

        //---   set up the VIF Unpack info for the UVs
        pD += SaveVIFUnpackInfo3( pD, NVertsToAdd );

        //---   add the UVs
        for ( i = 0; i < NVertsToAdd / 3; i++ )
        {
            t_Triangle* pTri;
            if( pTris )
                pTri = &pTris[NTrisAdded + i];
            else
                pTri = &s_TriData[s_SubMeshTriData[NTrisAdded + i]];

            for ( j = 0; j < 3; j++ )
            {
                //---   save the uv
                pD += SaveUV( pD, &pTri->UVs[j] );
            }
        }

        if ( g_DynamicLighting )
        {
            //---   set up the VIF Unpack info for the Normals
            pD += SaveVIFUnpackInfo4( pD, NVertsToAdd );

            //---   add the Normals
            for ( i = 0; i < NVertsToAdd / 3; i++ )
            {
                t_Triangle* pTri;
                if( pTris )
                    pTri = &pTris[NTrisAdded + i];
                else
                    pTri = &s_TriData[s_SubMeshTriData[NTrisAdded + i]];

                for ( j = 0; j < 3; j++ )
                {
                    mesh::vertex* pVert;

                    //---   get a ptr to the vert
                    pVert = &rGeom.m_pChunks[MeshIndex].m_pVerts[pTri->Index[j]];

                    //---   save the normal
                    pD += SaveNormal( pD, pVert );
                }
            }
        }
        else
        {
            //---   set up the VIF Unpack info for the RGBAs
            pD += SaveVIFUnpackInfo5( pD, NVertsToAdd );

            //---   add the RGBs
            for ( i = 0; i < NVertsToAdd / 3; i++ )
            {
                t_Triangle* pTri;
                if( pTris )
                    pTri = &pTris[NTrisAdded + i];
                else
                    pTri = &s_TriData[s_SubMeshTriData[NTrisAdded + i]];

                for ( j = 0; j < 3; j++ )
                {
                    mesh::vertex* pVert;

                    //---   get a ptr to the vert
                    pVert = &rGeom.m_pChunks[MeshIndex].m_pVerts[pTri->Index[j]];

                    //---   save the rgba
                    pD += SaveRGBA( pD, pVert );
                }
            }
        }

        //---   set up the VIF kick
        pD += SaveVIFKick( pD );

        //---   pad out the pD appropriately with NOPs
        while ( ((u32)(pD - &s_PacketData[s_CurPacketSize]) & 0xf) != 0x00 )
        {
            *((u32*)pD) = 0;
            pD += sizeof(u32);
        }

        //---   keep our totals current
        NTrisAdded += NVertsToAdd / 3;
    }

    //---   build a submesh
    s_SubMeshData[s_CurSubMesh].MeshID = CurMesh;
    s_SubMeshData[s_CurSubMesh].Flags = 0;
    s32 AlphaMode = GetAlphaMode( StartTri, rGeom );
    if( AlphaMode == mesh::ALPHA_MODE_PUNCHTHROUGH ) s_SubMeshData[s_CurSubMesh].Flags |= SUBMESH_FLAG_ALPHA_P;
    if( AlphaMode == mesh::ALPHA_MODE_TRANSPARENT ) s_SubMeshData[s_CurSubMesh].Flags |= SUBMESH_FLAG_ALPHA_T;
    s_SubMeshData[s_CurSubMesh].NTris = NTris;
    s_SubMeshData[s_CurSubMesh].NVertsTrans = NTris * 3;
    s_SubMeshData[s_CurSubMesh].TextureID = TextureID;
    s_SubMeshData[s_CurSubMesh].PacketOffset = s_CurPacketSize;
    s_SubMeshData[s_CurSubMesh].PacketSize = (u32)(pD - &s_PacketData[s_CurPacketSize]) + sizeof(sceDmaTag);
    s_SubMeshData[s_CurSubMesh].MinPixelSize = 0.0f;
    s_SubMeshData[s_CurSubMesh].MaxPixelSize = 0.0f;
    s_SubMeshData[s_CurSubMesh].AvgPixelSize = 0.0f;

//    BuildSubMeshRegionData( &s_SubMeshData[s_CurSubMesh], rGeom, MeshIndex, StartTri );

    //---   now assign the mip values for this sub mesh
    ComputeMipValues( pMesh, rGeom, StartTri, NTris, MeshIndex );

    //---   print out some stats
    if ( g_Verbose )
    {
        x_printf( "    SubMesh #%3d:  %4d Tris   %6d bytes   Texture %d\n",
                  s_CurSubMesh,
                  s_SubMeshData[s_CurSubMesh].NTris,
                  s_SubMeshData[s_CurSubMesh].PacketSize,
                  s_SubMeshData[s_CurSubMesh].TextureID );
        x_printf( "                : MinPelSize: %3.2f MaxPelSize: %3.2f AvgPelSize: %3.2f\n",
                  s_SubMeshData[s_CurSubMesh].MinPixelSize,
                  s_SubMeshData[s_CurSubMesh].MaxPixelSize,
                  s_SubMeshData[s_CurSubMesh].AvgPixelSize );
    }

    //---   fill in the DMA call, now that we have more data
    PS2HELP_BuildDMATagCont( pDMA, (u32)(pD - &s_PacketData[s_CurPacketSize]) - sizeof(sceDmaTag) );  // NOTE: pDMA was already set from above

    //---   set up a DMA return (because the packets will be called)
    pDMA = (sceDmaTag*)pD;
    ASSERT( (((u32)pDMA)&0x0F)==0 );
    PS2HELP_BuildDMATagRet( pDMA, 0 );
    pD += sizeof(sceDmaTag);

    //---   keep our totals current
    s_CurPacketSize += (u32)(pD - &s_PacketData[s_CurPacketSize]);
    ASSERT( s_CurPacketSize < MAX_PACKET_SIZE );
    pMesh->NVerts += NTris * 3;
    pMesh->NSubMeshes++;
    s_CurSubMesh++;
    ASSERT( s_CurSubMesh < MAX_NUM_SUBMESHES );

    return (NTrisAdded * 3);
}

//==========================================================================

static 
xbool PreProcessColorData( s32 iPackageIndex, s32 iMeshIndex, s32 nVerts, mesh::object &rGeom, t_StripPackage *pPackages )
{
    if (nVerts <= 0)
        return FALSE;

    //---   test the RGBs

    //---   get the first vert color as a test case
    mesh::vertex* pVert = &rGeom.m_pChunks[iMeshIndex].m_pVerts[pPackages[iPackageIndex].Verts[0].VertIndex];
    vector4 first = pVert->m_Color;
    s32 i;
    for (i=1; i<nVerts; ++i)
    {
        //---   get a ptr to the vert
        pVert = &rGeom.m_pChunks[iMeshIndex].m_pVerts[pPackages[iPackageIndex].Verts[i].VertIndex];

        //---   see if it's the same as the first
        if (pVert->m_Color != first)
            return FALSE;
    }

    //---   all is happy!
    return TRUE;
}

//==========================================================================

static
void BuildSubMeshStrips( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex, s32 CurMesh )
{
    s32                 i;
    s32                 NStrips;
    t_TriangleStrip*    pStrips = NULL;
    s32                 nTris = 0;
    t_Triangle*         pTris = NULL;
    t_StripPackage*     pPackages = NULL;
    s32                 NPackages;
    s32                 TotalDMASize = 0;
    s32                 NVerts;
    s32                 NVertsToAdd;
    byte*               pD;
    sceDmaTag*          pDMA;
    s32                 TextureID;

    //---   figure out which texture to use
    TextureID = GetTextureID( StartTri, rGeom );

    //---   Build the triangle strips
    if ( g_Verbose )
        x_printf( "Building triangle strips...(this may take a while)\n" );

    // check if we need to rotate the UVs
    s32 TWidth = s_pMaterialSet->m_pTextures[TextureID].m_Width;
    s32 THeight = s_pMaterialSet->m_pTextures[TextureID].m_Height;
    xbool bRotateUVs = ((THeight > TWidth) && (TWidth != 128 || THeight != 256));

    // Look through all of the strips first to get a general count of the number of verts we'll be adding
    s32 nTotalVerts = NTris * 3;
    // now use that total to calculate whether this object will be double-buffered or triple-buffered
    f32 fTotalBuffers = (f32)nTotalVerts * (1.0f / (f32)kMaxVertsInPackage[ 1 ]);

    s32 nBufferIndex = 1;   // assume we are double buffering
    if (fTotalBuffers > 6)
    {
        nBufferIndex = 0;   // change it to triple buffering
    }

    s32 nMaxVertsPerStrip = kMaxVertsInPackage[ nBufferIndex ];
    if (pMesh->Flags & MESH_FLAG_ENVMAPPED)
        nMaxVertsPerStrip = kMaxVertsInPackageEnvMap[ nBufferIndex ];

    // ask the tri stripper to build a linked list of strips for us
    BuildStrips( s_TriData, NTris, s_SubMeshTriData, &pStrips, NStrips, &pTris, nTris, nMaxVertsPerStrip, bRotateUVs );

    //---   grab a pointer to where we'll be storing the data
    pD = &s_PacketData[s_CurPacketSize];
    
    if ((NStrips == 0) && nTris)
    {
        // one triangle?
        // Just triangles that didn't strip?
        // We need to string them together into a strip using ADC bits...
//TODO:
        ASSERTS(FALSE, "Sonuvabeetch!");
        BuildSubMeshTris( pMesh, rGeom, StartTri, nTris, MeshIndex, CurMesh, pTris );
        return;
    }

    //---   set up the DMA tag
    pDMA = (sceDmaTag*)pD;
    PS2HELP_BuildDMATagCont( pDMA, 0 );  // we'll fill this in later!!!!!
    pD += sizeof(sceDmaTag);

    //---   Now build the vert lists based on the strip info
    nBufferIndex = BuildVertPackagesFromStrips( pStrips, NStrips, &pPackages, NPackages, (pMesh->Flags & MESH_FLAG_ENVMAPPED) );

    //---   now add the packages to the packet
    X_FILE *pDump = NULL;
#if defined( pmasters ) || defined( PMASTERS )
    pDump = x_fopen( "C:/geom.dump", "a+");
#endif
    NVerts = 0;
    for ( i = 0; i < NPackages; ++i )
    {
        s32         j;
        
        xbool bFirst = FALSE;

        s32 iIndex = 0;
        xbool bLast = FALSE;
        xbool bTriple = !nBufferIndex;
        if (bTriple)
        {
            iIndex = i%3;
            if (i == (NPackages - 1))
                bLast = TRUE;
        }
        else
        {
            iIndex = 0;//i%2;
            if (i==0)
                bFirst = TRUE;
        }

        //---   how many verts are we adding?
        NVertsToAdd = pPackages[i].NVerts;
        ASSERT( (NVertsToAdd >= 0) && (NVertsToAdd <= MAX_VERTS_IN_PACKAGE) );

        xbool bCompressColors = PreProcessColorData( i, MeshIndex, NVertsToAdd, rGeom, pPackages );
        if (g_ExportRGBAPtrs)
            bCompressColors = FALSE;
        s_CompressColor.Zero();

        xbool bEnvMap = (pMesh->Flags & MESH_FLAG_ENVMAPPED);

        if ( bEnvMap )
        {
            ASSERT( NVertsToAdd <= MAX_VERTS_IN_PACKAGE_ENVMAP );
        }


        //---   set up the VIF Unpack info for the command and GifTags, and
        //      set up our filling data as well
        if (bCompressColors)
        {
            mesh::vertex* pVert;

            //---   get a ptr to the vert
            pVert = &rGeom.m_pChunks[ MeshIndex ].m_pVerts[ pPackages[ i ].Verts[ 0 ].VertIndex ];

            //---   now grab it's color so we can stuff it in registers
            if (pVert)
                s_CompressColor = pVert->m_Color;
        }
        pD += SaveVIFUnpackInfo1( pD, iIndex, bTriple, bEnvMap, bCompressColors, pDump );


        //---   export the GIF tags
        if ( bEnvMap )
            pD += SaveGIFInfo( pD, GIF_TYPE_ENVMAP, NVertsToAdd, bCompressColors, pDump );
        else
            pD += SaveGIFInfo( pD, GIF_TYPE_STRIP, NVertsToAdd, bCompressColors, pDump );

        //===   add the vert data

        //---   set up the VIF Unpack info for the positions
        if (bCompressColors)
        {
            //---   may need to use multiple buffers
            s32 nWorkingCount = NVertsToAdd * 3;
            s32 nWorkingVertCount = NVertsToAdd;
            s32 nOffset = 0;    // address offset (steps by 3's)
            s32 iOffset = 0;    // index offset (steps by 1's)
            while (nWorkingCount > 0)
            {
                s32 nCount = nWorkingCount;
                s32 nVertCount = nWorkingVertCount;
                s32 nVertsPerBuffer = (bEnvMap?63:85);
                if (nWorkingVertCount > nVertsPerBuffer)
                {
                    nCount = (bEnvMap?0xFC:0xFF);
                    nVertCount = (bEnvMap?63:85);
                }

                pD += SaveVIFUnpackInfo2( pD, nCount, TRUE, iIndex, bTriple, bEnvMap, nOffset, bCompressColors, pDump );

                //---   add the positions
                for (j=iOffset; j<(nVertCount + iOffset); ++j)
                {
                    mesh::vertex* pVert;

                    //---   get a ptr to the vert
                    pVert = &rGeom.m_pChunks[MeshIndex].m_pVerts[pPackages[i].Verts[j].VertIndex];

                    //---   save the position
                    pD += SavePosition( pD, pVert, FALSE, pPackages[i].Verts[j].KickEnabled, pDump );
                }
                nOffset += nCount;
                iOffset += nVertsPerBuffer;

                nWorkingCount -= nCount;
                nWorkingVertCount -= nVertsPerBuffer;
            }
            pD += RecoverFromCompress( pD, pDump );
        }
        else
        {
            pD += SaveVIFUnpackInfo2( pD, NVertsToAdd, TRUE, iIndex, bTriple, bEnvMap, 0, bCompressColors, pDump );     // MASKing is ON for ADC connections

            //---   add the positions
            for ( j = 0; j < NVertsToAdd; j++ )
            {
                mesh::vertex* pVert;

                //---   get a ptr to the vert
                pVert = &rGeom.m_pChunks[MeshIndex].m_pVerts[pPackages[i].Verts[j].VertIndex];

                //---   save the position
                pD += SavePosition( pD, pVert, FALSE, pPackages[i].Verts[j].KickEnabled, pDump );
            }
        }
        //---   realign
        while ( ((u32)(pD - &s_PacketData[s_CurPacketSize]) & 0xf) != 0x00 )
        {
            *((u32*)pD) = 0;
            pD ++;
        }

        pD += SaveVIFUnpackInfo2_5( pD, NVertsToAdd, iIndex, bTriple, bEnvMap, pDump );

        //---   add the ADC data
        for (j = 0; j < NVertsToAdd; ++j)
        {
            mesh::vertex* pVert;

            //---   get a ptr to the vert
            pVert = &rGeom.m_pChunks[ MeshIndex ].m_pVerts[ pPackages[ i ].Verts[ j ].VertIndex ];

            //---   save the ADC info
            pD += SaveADC( pD, pVert, pPackages[ i ].Verts[ j ].KickEnabled, pDump );
        }
        //---   realign after the s16s
        while ( ((u32)(pD - &s_PacketData[s_CurPacketSize]) & 0xf) != 0x00 )
        {
            *((u32*)pD) = 0;
            pD ++;
        }

        if (!(pMesh->Flags & MESH_FLAG_SHADOW))
        {
            //---   set up the VIF Unpack info for the UVs
            pD += SaveVIFUnpackInfo3( pD, NVertsToAdd, iIndex, bTriple, pDump );

            //---   add the UVs
            for ( j = 0; j < NVertsToAdd; j++ )
            {
                //---   save the uv
                pD += SaveUV( pD, &pPackages[i].Verts[j].UV, bTriple, pDump );
            }
            //---   realign
            while ( ((u32)(pD - &s_PacketData[s_CurPacketSize]) & 0xf) != 0x00 )
            {
                *((u32*)pD) = 0;
                pD ++;
            }

            if ( g_DynamicLighting || bEnvMap )
            {
                //---   set up the VIF Unpack info for the Normals
                pD += SaveVIFUnpackInfo4( pD, NVertsToAdd, iIndex, bTriple, pDump );

                //---   add the Normals
                for ( j = 0; j < NVertsToAdd; j++ )
                {
                    mesh::vertex* pVert;

                    //---   get a ptr to the vert
                    pVert = &rGeom.m_pChunks[MeshIndex].m_pVerts[pPackages[i].Verts[j].VertIndex];

                    //---   save the normal
                    pD += SaveNormal( pD, pVert, pDump );
                }
            }
        }
        
        if (!bCompressColors && (pMesh->Flags & MESH_FLAG_SHADOW || !g_DynamicLighting))
        {
            //---   set up the VIF Unpack info for the RGBAs
            pD += SaveVIFUnpackInfo5( pD, NVertsToAdd, iIndex, bTriple, bEnvMap, pDump );

            //---   add the RGBs
            for ( j = 0; j < NVertsToAdd; j++ )
            {
                mesh::vertex* pVert;

                //---   get a ptr to the vert
                pVert = &rGeom.m_pChunks[MeshIndex].m_pVerts[pPackages[i].Verts[j].VertIndex];

                //---   save the rgba
                pD += SaveRGBA( pD, pVert, bTriple, pDump );
            }
        }

        //---   set up the VIF kick
        pD += SaveVIFKick( pD, iIndex, bTriple, bFirst, bLast, pDump );

        //---   pad out the pD appropriately with NOPs
        while ( ((u32)(pD - &s_PacketData[s_CurPacketSize]) & 0xf) != 0x00 )
        {
            *((u32*)pD) = 0;
            pD += sizeof(u32);
        }

        //---   keep our totals current
        NVerts += NVertsToAdd;
    }
    if (pDump)
        x_fclose( pDump );

    //---   build a submesh
    s_SubMeshData[s_CurSubMesh].MeshID = CurMesh;
    s_SubMeshData[s_CurSubMesh].Flags = 0;
    if ( pMesh->Flags & MESH_FLAG_ENVMAPPED )
        s_SubMeshData[s_CurSubMesh].Flags |= SUBMESH_FLAG_ENVMAP;
    s32 AlphaMode = GetAlphaMode( StartTri, rGeom );
    if( AlphaMode == mesh::ALPHA_MODE_PUNCHTHROUGH ) s_SubMeshData[s_CurSubMesh].Flags |= SUBMESH_FLAG_ALPHA_P;
    if( AlphaMode == mesh::ALPHA_MODE_TRANSPARENT ) s_SubMeshData[s_CurSubMesh].Flags |= SUBMESH_FLAG_ALPHA_T;
    s_SubMeshData[s_CurSubMesh].NTris = NTris;
    s_SubMeshData[s_CurSubMesh].NVertsTrans = NVerts;
    s_SubMeshData[s_CurSubMesh].TextureID = TextureID;
    s_SubMeshData[s_CurSubMesh].PacketOffset = s_CurPacketSize;
    s_SubMeshData[s_CurSubMesh].PacketSize =
        (u32)(pD - &s_PacketData[s_CurPacketSize]) + sizeof(sceDmaTag); // extra DMA is Return which hasn't been added yet
    s_SubMeshData[s_CurSubMesh].MinPixelSize = 0.0f;
    s_SubMeshData[s_CurSubMesh].MaxPixelSize = 0.0f;
    s_SubMeshData[s_CurSubMesh].AvgPixelSize = 0.0f;

//    BuildSubMeshRegionData( &s_SubMeshData[s_CurSubMesh], rGeom, MeshIndex, StartTri );

    //---   now assign the mip values for this sub mesh
    ComputeMipValues( pMesh, rGeom, StartTri, NTris, MeshIndex );

    //---   print out some stats
    if ( g_Verbose )
    {
        x_printf( "    SubMesh #%3d: %2d Strips %4d Tris  %1.2f Verts/Tri %5d bytes Tex. %d\n",
                  s_CurSubMesh,
                  NStrips,
                  s_SubMeshData[s_CurSubMesh].NTris,
                  (f32)NVerts / (f32)NTris,
                  s_SubMeshData[s_CurSubMesh].PacketSize,
                  s_SubMeshData[s_CurSubMesh].TextureID );
        x_printf( "                : MinPelSize: %3.2f MaxPelSize: %3.2f AvgPelSize: %3.2f\n",
                  s_SubMeshData[s_CurSubMesh].MinPixelSize,
                  s_SubMeshData[s_CurSubMesh].MaxPixelSize,
                  s_SubMeshData[s_CurSubMesh].AvgPixelSize );
    }

    //---   fill in the DMA call, now that we have more data
    PS2HELP_BuildDMATagCont( pDMA, (u32)(pD - &s_PacketData[s_CurPacketSize]) - sizeof(sceDmaTag) );  // NOTE: pDMA was already set from above

    //---   set up a DMA return (because the packets will be called)
    pDMA = (sceDmaTag*)pD;
    ASSERT( (((u32)pDMA)&0x0F)==0 );
    PS2HELP_BuildDMATagRet( pDMA, 0 );
    pD += sizeof(sceDmaTag);

    //---   keep our totals current
    s_CurPacketSize += (u32)(pD - &s_PacketData[s_CurPacketSize]);
    ASSERT( s_CurPacketSize < MAX_PACKET_SIZE );

    pMesh->NVerts += NVerts;
    pMesh->NSubMeshes++;
    s_CurSubMesh++;
    ASSERT( s_CurSubMesh < MAX_NUM_SUBMESHES );

    //---   free the data which was allocated for the strips
    for ( i = 0; i < NStrips; i++ )
    {
        x_free( pStrips[i].pVerts );
        x_free( pStrips[i].UVs );
    }
    x_free( pStrips );

    //---   free the data which was allocated for the packages
    x_free( pPackages );

    x_free( pTris );
}

//==========================================================================

static
void BuildSubMeshShadow( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex, s32 CurMesh )
{
    s32                 NTrisAdded = 0;
    byte*               pD;
    sceDmaTag*          pDMA;

    //---   grab a pointer to where we'll be storing the data
    pD = &s_PacketData[s_CurPacketSize];

    //---   set up the DMA tag
    pDMA = (sceDmaTag*)pD;
    PS2HELP_BuildDMATagCont( pDMA, 0 );  // we'll fill this in later!!!!!
    pD += sizeof(sceDmaTag);

    X_FILE *pDump = NULL;
#if defined( pmasters ) || defined( PMASTERS )
    pDump = x_fopen( "C:/geom.dump", "a+");
#endif

    xbool bCompressColors = PreProcessColorData( 0, MeshIndex, 0, rGeom, NULL );
    if (g_ExportRGBAPtrs)
        bCompressColors = FALSE;
    s_CompressColor.Zero();

    //---   now add the tris
    while ( NTrisAdded < NTris )
    {
        s32         i, j;

        //---   how many verts can we add to this package?
        s32 NVertsToAdd = (NTris - NTrisAdded) * 3;
        if ( NVertsToAdd > MAX_VERTS_IN_PACKAGE_SHADOW )
            NVertsToAdd = MAX_VERTS_IN_PACKAGE_SHADOW;

        //===   now build the PS2 package

        //---   set up the VIF Unpack info for the command and GifTags, and
        //      set up our filling data as well
//        pD += SaveVIFUnpackInfo1( pD );
        if (bCompressColors)
        {
            mesh::vertex* pVert;

            //---   get a ptr to the vert
            pVert = &rGeom.m_pChunks[ MeshIndex ].m_pVerts[ rGeom.m_pChunks[ MeshIndex ].m_pFaces[ 0 ].m_Index[ 0 ] ];

            //---   now grab it's color so we can stuff it in registers
            if (pVert)
                s_CompressColor = pVert->m_Color;
        }
        pD += SaveVIFUnpackInfo1( pD, 0, FALSE, FALSE, FALSE, pDump );

        //---   set up the render CMD
//        pD += SaveCMDInfo( pD, TRUE, TRUE, g_DynamicLighting, FALSE, NVertsToAdd );

        //---   export the GIF tags
        pD += SaveGIFInfo( pD, GIF_TYPE_SHADOW, NVertsToAdd, FALSE, pDump );

        //===   add the vert data

        //---   set up the VIF Unpack info for the positions
        pD += SaveVIFUnpackInfo2_5( pD, NVertsToAdd, TRUE, pDump );

        //---   add the positions
        for ( i = 0; i < NVertsToAdd / 3; i++ )
        {
            t_Triangle* pTri;
            pTri = &s_TriData[StartTri + NTrisAdded + i];

            for ( j = 0; j < 3; j++ )
            {
                mesh::vertex* pVert;

                //---   get a ptr to the vert
                pVert = &rGeom.m_pChunks[MeshIndex].m_pVerts[pTri->Index[j]];

                if (i==38)
                {
                    s32 debug = 0;
                    debug ++;
                }
                //---   save the position
                pD += SavePosition( pD, pVert, FALSE, FALSE, pDump );
            }
        }

        //---   set up the VIF kick
        pD += SaveVIFKick( pD );

        //---   pad out the pD appropriately with NOPs
        while ( ((u32)(pD - &s_PacketData[s_CurPacketSize]) & 0xf) != 0x00 )
        {
            *((u32*)pD) = 0;
            pD += sizeof(u32);
        }

        //---   keep our totals current
        NTrisAdded += NVertsToAdd / 3;
    }

    //---   build a submesh
    s_SubMeshData[s_CurSubMesh].MeshID = CurMesh;
    s_SubMeshData[s_CurSubMesh].Flags = 0;
    s_SubMeshData[s_CurSubMesh].NTris = NTris;
    s_SubMeshData[s_CurSubMesh].NVertsTrans = NTris * 3;
    s_SubMeshData[s_CurSubMesh].TextureID = -1;
    s_SubMeshData[s_CurSubMesh].PacketOffset = s_CurPacketSize;
    s_SubMeshData[s_CurSubMesh].PacketSize = (u32)(pD - &s_PacketData[s_CurPacketSize]) + sizeof(sceDmaTag);
    s_SubMeshData[s_CurSubMesh].MinPixelSize = 0.0f;
    s_SubMeshData[s_CurSubMesh].MaxPixelSize = 0.0f;
    s_SubMeshData[s_CurSubMesh].AvgPixelSize = 0.0f;

    //---   print out some stats
    if ( g_Verbose )
    {
        x_printf( "    SubMesh #%3d:  %4d Tris   %6d bytes\n",
                  s_CurSubMesh,
                  s_SubMeshData[s_CurSubMesh].NTris,
                  s_SubMeshData[s_CurSubMesh].PacketSize );
    }

    //---   fill in the DMA call, now that we have more data
    PS2HELP_BuildDMATagCont( pDMA, (u32)(pD - &s_PacketData[s_CurPacketSize]) - sizeof(sceDmaTag) );  // NOTE: pDMA was already set from above

    //---   set up a DMA return (because the packets will be called)
    pDMA = (sceDmaTag*)pD;
    ASSERT( (((u32)pDMA)&0x0F)==0 );
    PS2HELP_BuildDMATagRet( pDMA, 0 );
    pD += sizeof(sceDmaTag);

    //---   keep our totals current
    s_CurPacketSize += (u32)(pD - &s_PacketData[s_CurPacketSize]);
    ASSERT( s_CurPacketSize < MAX_PACKET_SIZE );
    pMesh->NVerts += NTris * 3;
    pMesh->NSubMeshes++;
    s_CurSubMesh++;
    ASSERT( s_CurSubMesh < MAX_NUM_SUBMESHES );

#if defined( pmasters ) || defined( PMASTERS )
    x_fclose( pDump );
#endif
}

//==========================================================================

static
void RipMeshData( s32 MeshIndex, t_GeomMesh* pMesh, mesh::object& rGeom, s32 CurMesh )
{
    s32     NTotalTris = 0;
    s32     TriCount;

    //---   create a workable copy of the triangle data
    CopyTriData( rGeom.m_pChunks[MeshIndex] );

    //---   sort the triangles by their material IDs
    x_qsort( s_TriData, s_NTris, sizeof(t_Triangle), CompareMaterialIDs );

    //---   each set of faces with a unique material becomes a submesh
    while ( NTotalTris < s_NTris )
    {
        //---   count the number of tris using the next material
        TriCount = CountTrisWithSameMaterial( NTotalTris );

        //---   build SubMeshes using this material out of the next set of tris
        if ( s_MeshData[MeshIndex].Flags & MESH_FLAG_SHADOW )
        {
            BuildSubMeshShadow( pMesh, rGeom, NTotalTris, TriCount, MeshIndex, CurMesh );
        }
        else
        if ( g_BuildTriStrips )
        {
            BuildSubMeshStrips( pMesh, rGeom, NTotalTris, TriCount, MeshIndex, CurMesh );
        }
        else
        {
            BuildSubMeshTris( pMesh, rGeom, NTotalTris, TriCount, MeshIndex, CurMesh );
        }
        NTotalTris += TriCount;
    }

    //---   free up the tri data
    x_free( s_TriData );
}

//==========================================================================

static
f32 CalculateMeshRadius( mesh::object& rGeom, s32 MeshID )
{
    s32 i;
    f32 Radius = 0.0f;

    for ( i = 0; i < rGeom.m_pChunks[MeshID].m_nVerts; i++ )
    {
        vector3*    v;
        f32         length;

        v = &rGeom.m_pChunks[MeshID].m_pVerts[i].m_vPos;
        length = v->Length();

        if ( length > Radius )
            Radius = length;
    }

    return Radius;
}

//==========================================================================

static
void GetTextureName( char* TextureName, mesh::object& rGeom, s32 Index )
{
    char    Drive[256];
    char    Path[256];
    char    File[256];
    char    Ext[256];
    
    x_splitpath( s_pMaterialSet->m_pTextures[Index].m_Filename,
                 Drive,
                 Path,
                 File,
                 Ext );
    x_strcpy( TextureName, File );
}

//==========================================================================

static
void PadStringWithZero( char *str, s32 length )
{
    s32 i;
    for ( i = x_strlen(str); i < length; i++ )
    {
        str[i] = '\0';
    }
}

//==========================================================================

static
xbool IsShadow( char* MeshName )
{
    s32 i;
    i = x_strlen( MeshName ) - x_strlen( "_shadow" );
    if ( i < 0 )
        return FALSE;

    if ( !x_stricmp( &MeshName[i], "_shadow" ) )
        return TRUE;

    return FALSE;
}

//==========================================================================

static
xbool IsEnvMapped( char* MeshName )
{
    s32 i;
    i = x_strlen( MeshName ) - x_strlen( "_env" );
    if ( i < 0 )
        return FALSE;

    if ( !x_stricmp( &MeshName[i], "_env" ) )
        return TRUE;

    return FALSE;
}

//==========================================================================

static
void ExportTextures( mesh::object& rGeom, X_FILE* outfile )
{
    s32 i;

    // Export the texture names to file
    if ( g_TextureNameFile )
    {

		//-- Texture File header
        x_fprintf( g_TextureNameFile, "[GLOBAL],GBL:1,SWZ:1,CMP:0,IMAP:0,4BIT:1,MIP:1,MIPCSTM:1,NUMMIPS:15,ALPHA:1,ALFNAME:1,ALFPUNCH:0,ALFGENC:0,CLRSCL:0:255:255:255:255\n");

        for ( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
        {
            char NewDrive[5];
            char NewDir[255];
            char NewFileName[80];
            char NewExtension[80];
            u8   AlphaPercent = 255;;
            x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
            
            // Attach PS2 folder to the end of the new dir
            x_strcat( NewDir, "PS2\\" );
            x_strcpy( NewExtension, "BMP" );

            x_makepath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
            x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
 
            x_sprintf( NewFileName, "%s", x_strtoupper(NewFileName) );

            if( NewFileName[2] >= '0' && NewFileName[2] <= '9' )
            {
                AlphaPercent = (u8)(255 * ((NewFileName[2] - '0') * .1));
            }

            // If there is a texture output file open, printf the current texture name to the output file.
            xbool AlphaFlag = ((!x_strncmp(&(NewFileName[3]),"CORONA",6)) || (!x_strncmp(&(NewFileName[4]),"CORONA",6)));

			if ( AlphaFlag || (AlphaPercent != 255)) 
			{
				x_fprintf( g_TextureNameFile, "%s,GBL:0,ALPHA:1,ALFNAME:%d,ALFGENC:%d,MIP:%d,CLRSCL:%d:255:255:255:%d\n",
						s_pMaterialSet->m_pTextures[i].m_Filename,   // File with full path.
						!AlphaFlag,                                  // ALFNAME:%d
						AlphaFlag,                                   // ALFGENC:%d
						g_UseMips,                                   // MIP:%d
						AlphaPercent != 255,                         // CLRSCL:%d
						AlphaPercent );                              // 255:255:255:%d
			}
			else
			{
				//-- Use global settings
				x_fprintf( g_TextureNameFile, "%s\n",s_pMaterialSet->m_pTextures[i].m_Filename);
			}
        }
    }

    if ( FALSE == g_ExportTextures )
        return;

    // Export the textures
    for ( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
    {
        x_bitmap    BMP;
        xbool       ret;

        if ( g_Verbose )
            x_printf( "Loading %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );

        if ( !x_stricmp( s_pMaterialSet->m_pTextures[i].m_Filename, "none" ) )
        {
            AUXBMP_SetupDefaultBitmap( BMP );
            ret = TRUE;
        }
        else
        {   ret = AUXBMP_LoadBitmap( BMP, s_pMaterialSet->m_pTextures[i].m_Filename );
        }

        if ( !ret )
        {
            x_printf( "ERROR: Unable to load %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
            PauseOnError();
        }
        else
        {
            // Added for alpha bitmaps ------------------------------------------------
            char Drive[X_MAX_DRIVE];
            char Dir[X_MAX_DIR];
            char Name[X_MAX_FNAME];
            char Ext[X_MAX_EXT];
            char AlphaName[X_MAX_FNAME];
            char PunchPath[X_MAX_PATH];
            char AlphaPath1[X_MAX_PATH];
            char AlphaPath2[X_MAX_PATH];
            char AlphaPath3[X_MAX_PATH];

            x_bitmap AlphaBMP;

            x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, Drive, Dir, Name, Ext );

            // Create possible alpha texture filenames
            x_sprintf( AlphaName, "%s_punch", Name );   x_makepath( PunchPath, Drive, Dir, AlphaName, Ext );
            x_sprintf( AlphaName, "%s_alpha", Name );   x_makepath( AlphaPath1, Drive, Dir, AlphaName, Ext );
            x_sprintf( AlphaName, "%s_mask", Name );    x_makepath( AlphaPath2, Drive, Dir, AlphaName, Ext );
            x_sprintf( AlphaName, "%sAlpha", Name );    x_makepath( AlphaPath3, Drive, Dir, AlphaName, Ext );

            // Clear Alpha present
            BMP.SetAllAlpha( 255 );

            // Load alpha map if possible
            if( AUXBMP_LoadBitmap( AlphaBMP, PunchPath ) )
            {
                if( !AUXBMP_ApplyAlpha( BMP, AlphaBMP, TRUE, TRUE ) )
                    x_printf( "--Could not apply punch using \"%s\" \n", PunchPath );
            }
            else if( AUXBMP_LoadBitmap( AlphaBMP, AlphaPath1 ) )
            {
                if( !AUXBMP_ApplyAlpha( BMP, AlphaBMP, FALSE, FALSE ) )
                    x_printf( "--Could not apply alpha using \"%s\" \n", AlphaPath1 );
            }
            else if( AUXBMP_LoadBitmap( AlphaBMP, AlphaPath2 ) )
            {
                if( !AUXBMP_ApplyAlpha( BMP, AlphaBMP, FALSE, FALSE ) )
                    x_printf( "--Could not apply alpha using \"%s\" \n", AlphaPath2 );
            }
            else if( AUXBMP_LoadBitmap( AlphaBMP, AlphaPath3 ) )
            {
                if( !AUXBMP_ApplyAlpha( BMP, AlphaBMP, FALSE, FALSE ) )
                    x_printf( "--Could not apply alpha using \"%s\" \n", AlphaPath3 );
            }

            AlphaBMP.KillBitmap();
            //---------------------------------------------------------------------------

            ret = AUXBMP_ConvertToPS2( BMP );

            if ( !ret )
            {
                x_printf( "ERROR: Unable to convert %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
                PauseOnError();
            }
            else
            {
                if( (BMP.GetWidth()  > 8)  &&
                    (BMP.GetHeight() > 8)  &&
                    g_UseMips )
                {
                    BMP.BuildMips();
                }

                if ( g_Verbose )
                    x_printf( "Saving %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
                ret = BMP.Save( outfile );
                if ( !ret )
                {
                    x_printf( "ERROR: Unable to save %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
                    PauseOnError();
                }
            }
        }
    }
}

//==========================================================================

void ExportPS2Geometry( mesh::object& rGeom, X_FILE* outfile )
{
    t_GeomFileHeader     hdr;
    s32                 i;

    s32 MaterialSetID = rGeom.GetMaterialSetId( mesh::MESH_TARGET_PS2 );
    if( MaterialSetID == -1 )
    {
        MaterialSetID = rGeom.GetMaterialSetId( mesh::MESH_TARGET_GENERIC );
        ASSERT( MaterialSetID != -1 );
    }
    s_pMaterialSet = &rGeom.m_pMaterialSets[ MaterialSetID ];

    ASSERT( outfile );

    //---   fill in some basic header info (not permanent, just a starting place!)
    FillName( hdr.Name, rGeom.m_Name, 16 );
    hdr.Flags           = 0;
    hdr.NMeshes         = 0;
    hdr.NSubMeshes      = 0;
    hdr.NTextures       = 0;
    hdr.NTexturesInFile = 0;
    hdr.PacketSize      = 0;

    //---   allocate some space for the data we will be building
    s_PacketData    = (byte*)x_malloc(MAX_PACKET_SIZE);
    s_MeshData      = (t_GeomMesh*)x_malloc(MAX_NUM_MESHES * sizeof(t_GeomMesh));
    s_SubMeshData   = (t_GeomSubMesh*)x_malloc(MAX_NUM_SUBMESHES * sizeof(t_GeomSubMesh));
    ASSERT( s_PacketData );
    ASSERT( s_MeshData );
    ASSERT( s_SubMeshData );
    if ( g_ExportPosPtrs )
    {
        s_PosPtrs = (u32*)x_malloc( MAX_NUM_VERTS * sizeof(u32) );
        ASSERT( s_PosPtrs );
    }
    if ( g_ExportUVPtrs )
    {
        s_UVPtrs = (u32*)x_malloc( MAX_NUM_VERTS * sizeof(u32) );
        ASSERT( s_UVPtrs );
    }
    if ( g_ExportRGBAPtrs )
    {
        s_RGBAPtrs = (u32*)x_malloc( MAX_NUM_VERTS * sizeof(u32) );
        ASSERT( s_RGBAPtrs );
    }
    if ( g_ExportNormalPtrs )
    {
        s_NormalPtrs = (u32*)x_malloc( MAX_NUM_VERTS * sizeof(u32) );
        ASSERT( s_NormalPtrs );
    }

    if ( g_Verbose )
    {
        x_printf( "Geometry Name: %s\n", hdr.Name );
    }

    //---   loop through each of the meshes adding them to the data
    s32 CurrentMesh = 0;
    for ( i = 0; i < rGeom.m_nChunks; i++ )
    {
        ASSERT( hdr.NMeshes < MAX_NUM_MESHES );
        ASSERT( hdr.NSubMeshes < MAX_NUM_SUBMESHES );

        //---   fill in the basic mesh data
        s_MeshData[CurrentMesh].FirstSubMesh = s_CurSubMesh;
        s_MeshData[CurrentMesh].Flags = MESH_FLAG_VISIBLE;
        if ( g_ExportPosPtrs )
            s_MeshData[CurrentMesh].Flags |= MESH_FLAG_CONTAINS_POSPTRS;
        if ( g_ExportUVPtrs )
            s_MeshData[CurrentMesh].Flags |= MESH_FLAG_CONTAINS_UVPTRS;
        if ( g_ExportRGBAPtrs )
            s_MeshData[CurrentMesh].Flags |= MESH_FLAG_CONTAINS_RGBAPTRS;
        if ( g_ExportNormalPtrs )
            s_MeshData[CurrentMesh].Flags |= MESH_FLAG_CONTAINS_NORMALPTRS;
        if ( g_UseMips )
            s_MeshData[CurrentMesh].Flags |= MESH_FLAG_USE_MIPS;
        if ( g_DynamicLighting )
            s_MeshData[CurrentMesh].Flags |= MESH_FLAG_DYNAMIC_LIGHTING;
        s_MeshData[CurrentMesh].Radius = CalculateMeshRadius( rGeom, i );
        s_MeshData[CurrentMesh].NSubMeshes = 0;
        s_MeshData[CurrentMesh].NVerts = 0;
        s_MeshData[CurrentMesh].pPositions = NULL;
        s_MeshData[CurrentMesh].pUVs = NULL;
        s_MeshData[CurrentMesh].pRGBAs = NULL;
        s_MeshData[CurrentMesh].pNormals = NULL;
        FillName( s_MeshData[CurrentMesh].Name, rGeom.m_pChunks[i].m_Name, 16 );

        if ( IsShadow( s_MeshData[CurrentMesh].Name ) )
        {
            s_MeshData[CurrentMesh].Flags |= MESH_FLAG_SHADOW;
            s_MeshData[CurrentMesh].Flags &= ~MESH_FLAG_CONTAINS_UVPTRS;
            s_MeshData[CurrentMesh].Flags &= ~MESH_FLAG_CONTAINS_RGBAPTRS;
            s_MeshData[CurrentMesh].Flags &= ~MESH_FLAG_CONTAINS_NORMALPTRS;
        }

        if ( IsEnvMapped( s_MeshData[CurrentMesh].Name ) )
        {
            s_MeshData[CurrentMesh].Flags |= MESH_FLAG_ENVMAPPED;
        }

        //---   print out some stats
        if ( g_Verbose )
            x_printf( "  Mesh Name: %s\n", s_MeshData[CurrentMesh].Name );

        //---   build the submeshes for this mesh
        RipMeshData( i, &s_MeshData[CurrentMesh], rGeom, CurrentMesh );

        //---   print out some more stats
        if ( g_Verbose )
            x_printf( "  NSubMeshes: %d\n", s_MeshData[CurrentMesh].NSubMeshes );

        //---   increase the mesh count
        hdr.NMeshes++;
        hdr.NSubMeshes += s_MeshData[i].NSubMeshes;
        CurrentMesh++;
    }

    //---   finish filling in the header
    hdr.PacketSize = s_CurPacketSize;
    hdr.NTextures = s_pMaterialSet->m_nTextures;
    if ( g_ExportTextures )
        hdr.NTexturesInFile = s_pMaterialSet->m_nTextures;
    else
        hdr.NTexturesInFile = 0;
    hdr.NPosPtrs = s_NPosPtrs;
    hdr.NUVPtrs = s_NUVPtrs;
    hdr.NRGBAPtrs = s_NRGBAPtrs;
    hdr.NNormalPtrs = s_NNormalPtrs;

    //---   print out some more stats
    if ( g_Verbose )
        x_printf( "Total Packet Memory Usage: %d\n", s_CurPacketSize );

    //===   now we can write out the data file

    //---   write out the header
    x_fwrite( &hdr, sizeof(t_GeomFileHeader), 1, outfile );

    //---   write out the texture names
    for ( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
    {
        char    TextureName[TEXTURE_NAME_LENGTH];
        GetTextureName( TextureName, rGeom, i );
        PadStringWithZero( TextureName, TEXTURE_NAME_LENGTH );
        x_fwrite( TextureName, sizeof(char), TEXTURE_NAME_LENGTH, outfile );
    }

    //---   write out the mesh data
    x_fwrite( s_MeshData, sizeof(t_GeomMesh), hdr.NMeshes, outfile );

    //---   write out the sub-mesh data
    x_fwrite( s_SubMeshData, sizeof(t_GeomSubMesh), hdr.NSubMeshes, outfile );

    //---   write out the packet data
    x_fwrite( s_PacketData, sizeof(byte), s_CurPacketSize, outfile );

    //---   write out the textures
    ExportTextures( rGeom, outfile );

    //---   write out the position pointers
    if ( g_ExportPosPtrs )
        x_fwrite( s_PosPtrs, sizeof(u32), s_NPosPtrs, outfile );

    //---   write out the UV pointers
    if ( g_ExportUVPtrs )
        x_fwrite( s_UVPtrs, sizeof(u32), s_NUVPtrs, outfile );

    //---   write out the RGBA pointers
    if ( g_ExportRGBAPtrs )
        x_fwrite( s_RGBAPtrs, sizeof(u32), s_NRGBAPtrs, outfile );

    //---   write out the Normal pointers
    if ( g_ExportNormalPtrs )
        x_fwrite( s_NormalPtrs, sizeof(u32), s_NNormalPtrs, outfile );

    //---   free up the data
    x_free( s_PacketData );
    x_free( s_MeshData );
    x_free( s_SubMeshData );
    if ( s_PosPtrs )
        x_free( s_PosPtrs );
    if ( s_UVPtrs )
        x_free( s_UVPtrs );
    if ( s_RGBAPtrs )
        x_free( s_RGBAPtrs );
    if ( s_NormalPtrs )
        x_free( s_NormalPtrs );
}

//==========================================================================

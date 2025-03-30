#include <windows.h>
#include <stdlib.h>
#include "AUX_Bitmap.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "XBOX\XBOX_Geom.hpp"  // THIS IS INTENDED... XBOX_Geam won't compile because of headers
#include "Q_Geom.hpp"
#include "RipStad.h"
#include "Strip.h"
#include "XBOXRipper.h"
#include "AsciiMesh.h"

// HACK! We are working with a PC tool, and we are building things for the XBOX, we must make sure that our defines
// used here are the same as the ones in D3D8Types-XBOX.h building stuff.
#define D3DFVF_XYZ              0x002
#define D3DFVF_NORMAL           0x010
#define D3DFVF_TEX1             0x100
#define D3DFVF_DIFFUSE          0x040
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
typedef DWORD D3DCOLOR;
#define D3DPT_POINTLIST              1
#define D3DPT_LINELIST               2
#define D3DPT_LINELOOP               3  // Xbox extension
#define D3DPT_LINESTRIP              4
#define D3DPT_TRIANGLELIST           5
#define D3DPT_TRIANGLESTRIP          6
#define D3DPT_TRIANGLEFAN            7
#define D3DPT_QUADLIST               8  // Xbox extension
#define D3DPT_QUADSTRIP              9  // Xbox extension
#define D3DPT_POLYGON                10 // Xbox extension
#define D3DPT_FORCE_DWORD            0x7fffffff /* force 32-bit size enum */
////////////////////////



////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////
#define MAX_NUM_MESHES          512
#define MAX_NUM_SUBMESHES       1500
#define MAX_NUM_TRIS_IN_SUBMESH 10000
#define MAX_NUM_VERTS           65536 //D3DMAXNUMVERTICES
#define MAX_NUM_PRIMS           65536 //D3DMAXNUMPRIMITIVES

#define MAX_GRIDPOINTS_PER_ROW  3
#define MAX_GRIDPOINTS          343  // 7 cubed
#define DISTANCE_B_POINTS       4000.0f // distance between the points
#define MAX_SUBMESHES_AT_POINT  2000

//=====================================================================================================================================
// Globals
//=====================================================================================================================================
// Mesh Data
static t_GeomMesh*      s_pMesh      = NULL;
static t_GeomSubMesh*   s_pSubMesh   = NULL;
static s32              s_CurSubMesh = 0;

// Triangle Builder Data
static t_Triangle*  s_TriData = NULL;
static s32          s_NTris   = 0;

static mesh::material_set*    s_pMaterialSet = NULL;
static u16              s_SubMeshTriData[MAX_NUM_TRIS_IN_SUBMESH];

// Point Grid Data and Region Data
struct rip_bounds
{
    vector3             Corner;
    vector3             Axis[3];
    f32                 Area;
};

// this data will create an axial aligned bounding box around the entire stadium and a point grid
struct point_grid
{
    vector3     GridBoundsMax;
    vector3     GridBoundsMin;
    rip_bounds  StadiumBBox;
    vector3     GridPoints[MAX_GRIDPOINTS];
    s32         SubMeshesPerGridPoint[MAX_GRIDPOINTS];
    s32         SubMeshesAtGridPoint[MAX_GRIDPOINTS][MAX_SUBMESHES_AT_POINT];
    f32         DistanceBPoints;
    s32         PointCount;
};
static point_grid       s_PGrid;

//=====================================================================================================================================
// Local Functions.
//=====================================================================================================================================
static void BuildTriData( mesh::chunk& Chunk );
static f32  CalculateMeshRadius( mesh::object& rGeom, s32 MeshID );
static s32  CompareMaterialIDs( const void* Element1, const void* Element2 );
static s32  CountTrisWithSameMaterial( s32 StartIndex );
static void FillName( char* Name, char* src, s32 NChars );
static void GetTextureName( char* TextureName, mesh::object& rGeom, s32 Index );
static void PadStringWithZero( char *str, s32 length );

//==========================================================================

static
xbool IsSpecialMesh( const char* SubMeshName )
{
    // HACK WARNING!!!
    // BLEH!!!
    
    if (x_strstr( SubMeshName, "HOMERUN_LM" ) )
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

static s32 GetTextureID( s32 TriID, mesh::object& rGeom, s32 Stage )
{
    s32 MaterialID;

    MaterialID = s_TriData[s_SubMeshTriData[0]].MaterialID;
    if ( MaterialID >= 0 )
    {
        mesh::material*         pMaterial;
        mesh::stage*     pStage;

        pMaterial = &s_pMaterialSet->m_pMaterials[MaterialID];
        if( pMaterial->m_nStages <= Stage ) 
        {
            if( Stage == 0 )
            {
                ASSERT( FALSE );
            }
            if( Stage == 0 ) 
                return 0;
            else
                return -1;
        }
        pStage = &s_pMaterialSet->m_pStages[pMaterial->m_iStages[Stage]];

        if( pStage->m_iTexture < 0 )
        {
            if( Stage == 0 ) 
                return 0;
            else
                return -1;
        }
        if (Stage == 0)
            return MaterialID;
        return pStage->m_iTexture;
    }
    else
    {
        if( Stage == 0 ) 
            return 0;
        else
            return -1;
    }
}

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

//=====================================================================================================================================
// Implementation
//=====================================================================================================================================
//-------------------------------------------------------------------------------------------------------------------------------------
//
// BuildTriData
//
// This function takes a 'Chunk' from the import file and copies out each
// face that is used to build the object.
//
// Once the faces are extracted and copied, the local version of the data
// is sorted (The triangles are sorted) by the material that is applied to
// them.  That way the polygons that share the same material are grouped
// in memory.
//
void BuildTriData( mesh::chunk& Chunk )
{
    s32 i;
    s32 count = 0;

    // Count the number of triangles we need
    for( i = 0; i < Chunk.m_nFaces; i++ )
    {
        // If this face is a quad, make sure to add 2 triangles.
        if ( Chunk.m_pFaces[i].m_bQuad )
            count += 2;
        else
            count++;
    }

    // Allocate a triangle buffer.
    s_TriData = new t_Triangle[count];
    ASSERT( s_TriData );

    // Copy the triangle data from the chunk into the triangle buffer.
    count = 0;
    for ( i = 0; i < Chunk.m_nFaces; i++ )
    {
        vector3 vPos0 = Chunk.m_pVerts[Chunk.m_pFaces[i].m_Index[0]].m_vPos;
        vector3 vPos1 = Chunk.m_pVerts[Chunk.m_pFaces[i].m_Index[1]].m_vPos;
        vector3 vPos2 = Chunk.m_pVerts[Chunk.m_pFaces[i].m_Index[2]].m_vPos;
        vector3 vMid0 = ( vPos0 + vPos1 ) * 0.5f;
        vector3 vMid1 = ( vPos1 + vPos2 ) * 0.5f;
        vector3 vMid2 = ( vPos0 + vPos2 ) * 0.5f;
        vector3 vCentroid = ( vMid0 + vMid1 + vMid2 ) * 0.33333f;

        // Is this face a quad?
        if ( Chunk.m_pFaces[i].m_bQuad )
        {
            // Copy a quad, splitting it into two tris
            s_TriData[count].Index[0]   = Chunk.m_pFaces[i].m_Index[0];
            s_TriData[count].Index[1]   = Chunk.m_pFaces[i].m_Index[1];
            s_TriData[count].Index[2]   = Chunk.m_pFaces[i].m_Index[2];
            s_TriData[count].MaterialID = Chunk.m_pFaces[i].m_iMaterial;
            s_TriData[count].Used = FALSE;
            s_TriData[count].Centroid = vCentroid;
            count++;

            s_TriData[count].Index[0]   = Chunk.m_pFaces[i].m_Index[0];
            s_TriData[count].Index[1]   = Chunk.m_pFaces[i].m_Index[2];
            s_TriData[count].Index[2]   = Chunk.m_pFaces[i].m_Index[3];
            s_TriData[count].MaterialID = Chunk.m_pFaces[i].m_iMaterial;
            s_TriData[count].Used = FALSE;
            s_TriData[count].Centroid = vCentroid;
            count++;
        }
        // Or just a regular triangle.
        else
        {
            // Remove degenerate triangles that are just lines.
            if ( (Chunk.m_pFaces[i].m_Index[0] == Chunk.m_pFaces[i].m_Index[1]) ||
                 (Chunk.m_pFaces[i].m_Index[0] == Chunk.m_pFaces[i].m_Index[2]) ||
                 (Chunk.m_pFaces[i].m_Index[1] == Chunk.m_pFaces[i].m_Index[2]) )
            {
                //---   skip degenerate triangles
                continue;
            }
            // Copy the triangle
            s_TriData[count].Index[0]   = Chunk.m_pFaces[i].m_Index[0];
            s_TriData[count].Index[1]   = Chunk.m_pFaces[i].m_Index[1];
            s_TriData[count].Index[2]   = Chunk.m_pFaces[i].m_Index[2];
            s_TriData[count].MaterialID = Chunk.m_pFaces[i].m_iMaterial;
            s_TriData[count].Used = FALSE;
            s_TriData[count].Centroid = vCentroid;
            count++;
        }
    }

    s_NTris = count;

    // Sort the triangles by their material IDs, from lowest to highest.
    x_qsort( s_TriData, s_NTris, sizeof(t_Triangle), CompareMaterialIDs );
}


//-------------------------------------------------------------------------------------------------------------------------------------
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

static s32 CompareDistFromOrigin( const void* Element1, const void* Element2 )
{
    const t_Triangle* pTri1 = (const t_Triangle*)Element1;
    const t_Triangle* pTri2 = (const t_Triangle*)Element2;

    if ( pTri1->Centroid.SquaredLength() < pTri2->Centroid.SquaredLength() ) return -1;
    if ( pTri1->Centroid.SquaredLength() > pTri2->Centroid.SquaredLength() ) return 1;
    return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 CompareMaterialIDs( const void* Element1, const void* Element2 )
{
    if( ((t_Triangle*)Element1)->MaterialID < ((t_Triangle*)Element2)->MaterialID )
        return -1;
    else 
    if( ((t_Triangle*)Element1)->MaterialID > ((t_Triangle*)Element2)->MaterialID )
        return 1;
    else
        return CompareDistFromOrigin( Element1, Element2 );
}

//-------------------------------------------------------------------------------------------------------------------------------------
static
s32 CountTrisWithSameGridPointAndMaterial( mesh::chunk* pChunk, s32 GridPoint, s32 StartIndex )
{
    s32 i;
    s32 count = 0;
    s32 CurMaterial = s_TriData[StartIndex].MaterialID;
    vector3 TempVector[3];

    for ( i = StartIndex; i < s_NTris; i++ )
    {
        if ( s_TriData[i].Used ) continue;

        TempVector[0] = pChunk->m_pVerts[s_TriData[i].Index[0]].m_vPos - s_PGrid.GridPoints[GridPoint];
        TempVector[1] = pChunk->m_pVerts[s_TriData[i].Index[1]].m_vPos - s_PGrid.GridPoints[GridPoint];
        TempVector[2] = pChunk->m_pVerts[s_TriData[i].Index[2]].m_vPos - s_PGrid.GridPoints[GridPoint];
        if ( TempVector[0].Length() <= s_PGrid.DistanceBPoints || 
             TempVector[1].Length() <= s_PGrid.DistanceBPoints ||
             TempVector[2].Length() <= s_PGrid.DistanceBPoints )
        {
            if ( s_TriData[i].MaterialID != CurMaterial )
                continue;

            if ( count == MAX_NUM_TRIS_IN_SUBMESH )
                break;

            s_TriData[i].Used = TRUE;
            s_SubMeshTriData[count] = i;
            count++;
        }
    }
    return count;
}

//-------------------------------------------------------------------------------------------------------------------------------------
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


//-------------------------------------------------------------------------------------------------------------------------------------
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


//-------------------------------------------------------------------------------------------------------------------------------------
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
void BuildStadiumBoundingBoxAndPGrid( mesh::object& rGeom )
{
    s32 i,j,CurrentChunk;

    if( g_Verbose )
        x_printf( "Finding stadium bounds...\n" );

    for( CurrentChunk = 0; CurrentChunk < rGeom.m_nChunks; CurrentChunk++ )
    {
        BuildTriData( rGeom.m_pChunks[CurrentChunk] );
        for ( i = 0; i < s_NTris; i++ )
        {
            for( j = 0; j < 3; j++ )
            {
                vector3 P = rGeom.m_pChunks[CurrentChunk].m_pVerts[s_TriData[i].Index[j]].m_vPos;
                if( P.X < s_PGrid.GridBoundsMin.X ) s_PGrid.GridBoundsMin.X = P.X;
                if( P.Y < s_PGrid.GridBoundsMin.Y ) s_PGrid.GridBoundsMin.Y = P.Y;
                if( P.Z < s_PGrid.GridBoundsMin.Z ) s_PGrid.GridBoundsMin.Z = P.Z;
                if( P.X > s_PGrid.GridBoundsMax.X ) s_PGrid.GridBoundsMax.X = P.X;
                if( P.Y > s_PGrid.GridBoundsMax.Y ) s_PGrid.GridBoundsMax.Y = P.Y;
                if( P.Z > s_PGrid.GridBoundsMax.Z ) s_PGrid.GridBoundsMax.Z = P.Z;
            }
        }
        delete[] s_TriData;
        s_TriData = NULL;
    }

    vector3     BBoxPoints[8];
    vector3     TempVector;
    s32         x,y,z;
    f32         BiggestDistance;
    s32         RealCount = 0;
    s32         XPasses, YPasses, ZPasses;
    s32         CurMeshID = -1;

    if( !g_GridPointFile ) return;

    if( g_Verbose )
        x_printf( "-- BUILDING GRID FILE --\n" );

    // setup the Bounds matrix
    s_PGrid.StadiumBBox.Axis[0].Set( 1.0f, 0.0f, 0.0f );
    s_PGrid.StadiumBBox.Axis[1].Set( 0.0f, 1.0f, 0.0f );
    s_PGrid.StadiumBBox.Axis[2].Set( 0.0f, 0.0f, 1.0f );

    // Slide corner to min pt
    s_PGrid.StadiumBBox.Corner = (s_PGrid.StadiumBBox.Axis[0] * s_PGrid.GridBoundsMin.X) +
                                 (s_PGrid.StadiumBBox.Axis[1] * s_PGrid.GridBoundsMin.Y) +
                                 (s_PGrid.StadiumBBox.Axis[2] * s_PGrid.GridBoundsMin.Z);

    // Scale axis by length of sides
    s_PGrid.StadiumBBox.Axis[0] *= (s_PGrid.GridBoundsMax.X-s_PGrid.GridBoundsMin.X);
    s_PGrid.StadiumBBox.Axis[1] *= (s_PGrid.GridBoundsMax.Y-s_PGrid.GridBoundsMin.Y);
    s_PGrid.StadiumBBox.Axis[2] *= (s_PGrid.GridBoundsMax.Z-s_PGrid.GridBoundsMin.Z);

    // now we have the bounding box the entire stadium... let's get the actual points for that box...
    BBoxPoints[0] = s_PGrid.StadiumBBox.Corner;
    BBoxPoints[1] = s_PGrid.StadiumBBox.Corner + s_PGrid.StadiumBBox.Axis[2];
    BBoxPoints[2] = s_PGrid.StadiumBBox.Corner + s_PGrid.StadiumBBox.Axis[1];
    BBoxPoints[3] = s_PGrid.StadiumBBox.Corner + s_PGrid.StadiumBBox.Axis[1] + s_PGrid.StadiumBBox.Axis[2];
    BBoxPoints[4] = s_PGrid.StadiumBBox.Corner + s_PGrid.StadiumBBox.Axis[0];
    BBoxPoints[5] = s_PGrid.StadiumBBox.Corner + s_PGrid.StadiumBBox.Axis[0] + s_PGrid.StadiumBBox.Axis[2];
    BBoxPoints[6] = s_PGrid.StadiumBBox.Corner + s_PGrid.StadiumBBox.Axis[0] + s_PGrid.StadiumBBox.Axis[1];
    BBoxPoints[7] = s_PGrid.StadiumBBox.Corner + s_PGrid.StadiumBBox.Axis[0] + s_PGrid.StadiumBBox.Axis[1] + s_PGrid.StadiumBBox.Axis[2];

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ok, so now here is the deal.  We're going to put a bunch of points equal distances apart within the box.
    // If we hit the bottom and we would need to go below it, we just snap it to the frame of the box.
    // These are our grid points...
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // first step, find the longest plane and figure distances from that
    TempVector = BBoxPoints[0] - BBoxPoints[4];
    BiggestDistance = s_PGrid.StadiumBBox.Axis[0].X;
    BiggestDistance = TempVector.Length();
    if( s_PGrid.StadiumBBox.Axis[1].Y > BiggestDistance ) BiggestDistance = s_PGrid.StadiumBBox.Axis[1].Y;
    if( s_PGrid.StadiumBBox.Axis[2].Z > BiggestDistance ) BiggestDistance = s_PGrid.StadiumBBox.Axis[2].Z;
    
    // now we know the distance between each point on all axis will be BiggestDistance / MAX_GRIDPOINTS_PER_ROW
    // so let's figure out where all the gridpoints are
    s_PGrid.DistanceBPoints = DISTANCE_B_POINTS;//BiggestDistance / (f32)MAX_GRIDPOINTS_PER_ROW;

    // figure out how many passes we need for each axis
    XPasses = (s32)((s_PGrid.StadiumBBox.Axis[0].X - s_PGrid.StadiumBBox.Corner.X) / s_PGrid.DistanceBPoints);
    if( ((s_PGrid.StadiumBBox.Axis[0].X - s_PGrid.StadiumBBox.Corner.X) / s_PGrid.DistanceBPoints) > XPasses ) XPasses++;
    YPasses = (s32)((s_PGrid.StadiumBBox.Axis[1].Y - s_PGrid.StadiumBBox.Corner.Y) / s_PGrid.DistanceBPoints);
    if( ((s_PGrid.StadiumBBox.Axis[1].Y - s_PGrid.StadiumBBox.Corner.Y) / s_PGrid.DistanceBPoints) > YPasses ) YPasses++;
    ZPasses = (s32)((s_PGrid.StadiumBBox.Axis[2].Z - s_PGrid.StadiumBBox.Corner.Z) / s_PGrid.DistanceBPoints);
    if( ((s_PGrid.StadiumBBox.Axis[2].Z - s_PGrid.StadiumBBox.Corner.Z) / s_PGrid.DistanceBPoints) > ZPasses ) ZPasses++;

    s_PGrid.PointCount = 0;
    for( x = 0; x < XPasses; x++ )
    {
        for( y = 0; y < YPasses; y++ )
        {
            for( z = 0; z < ZPasses; z++ )
            {
                s_PGrid.GridPoints[s_PGrid.PointCount].X = s_PGrid.StadiumBBox.Corner.X + (s_PGrid.DistanceBPoints * x);
                s_PGrid.GridPoints[s_PGrid.PointCount].Y = s_PGrid.StadiumBBox.Corner.Y + (s_PGrid.DistanceBPoints * y);
                s_PGrid.GridPoints[s_PGrid.PointCount].Z = s_PGrid.StadiumBBox.Corner.Z + (s_PGrid.DistanceBPoints * z);
                s_PGrid.PointCount++;
            }
        }
    }

    // initialize variables
    for( i = 0; i < s_PGrid.PointCount; i++ )
    {
        s_PGrid.SubMeshesPerGridPoint[i] = 0;
    }

    if( g_Verbose )
        x_printf( "%d GRID POINTS CREATED.  DISTANCE: %f\n", s_PGrid.PointCount, s_PGrid.DistanceBPoints );
}

//==========================================================================

static void BuildSubMeshRegionData( t_GeomSubMesh* pSubMesh, mesh::object& rGeom, s32 MeshIndex, s32 StartTri )
{
    s32         NYaws=10;
    s32         NPitches=10;
    s32         i,j,y,p;
    rip_bounds  BestBounds;
    
    // Make bad area
    BestBounds.Area = -1.0f;

    for( y=0; y<NYaws; y++ )
    for( p=0; p<NPitches; p++ )
    {
        matrix4 M;
        f32     Min[3];
        f32     Max[3];
        f32     Area;

        // Get matrix to bring object into axis-aligned world
        M.Identity();
        M.RotateX( R_90 * (f32)p / (f32)(NPitches-1) );
        M.RotateY( R_90 * (f32)y / (f32)(NYaws-1) );
    
        // Clear min and max
        Min[0] = Min[1] = Min[2] =  100000000.0f;
        Max[0] = Max[1] = Max[2] = -100000000.0f;
        for( i = 0; i < pSubMesh->NTris; i++ )
        {
            for( j = 0; j < 3; j++ )
            {
                vector3 BP;
                vector3 P = rGeom.m_pChunks[MeshIndex].m_pVerts[s_TriData[s_SubMeshTriData[i]].Index[j]].m_vPos;
                BP.X  = M.M[0][0]*P.X + M.M[1][0]*P.Y + M.M[2][0]*P.Z;
                BP.Y  = M.M[0][1]*P.X + M.M[1][1]*P.Y + M.M[2][1]*P.Z;
                BP.Z  = M.M[0][2]*P.X + M.M[1][2]*P.Y + M.M[2][2]*P.Z;
                // first setup min/max for the submesh
                if( BP.X < Min[0] ) Min[0] = BP.X;
                if( BP.Y < Min[1] ) Min[1] = BP.Y;
                if( BP.Z < Min[2] ) Min[2] = BP.Z;
                if( BP.X > Max[0] ) Max[0] = BP.X;
                if( BP.Y > Max[1] ) Max[1] = BP.Y;
                if( BP.Z > Max[2] ) Max[2] = BP.Z;

            }
        }
        // Compute area and see if it beats previous
        Area = (Max[0]-Min[0])*(Max[1]-Min[1]) +
               (Max[1]-Min[1])*(Max[2]-Min[2]) +
               (Max[0]-Min[0])*(Max[2]-Min[2]);

        if( (BestBounds.Area<0) || (Area < BestBounds.Area) )
        {
            BestBounds.Area = Area;

            // Compute bounds values
            M.GetRows(  BestBounds.Axis[0], 
                        BestBounds.Axis[1],
                        BestBounds.Axis[2] );

            // Slide corner to min pt
            BestBounds.Corner = (BestBounds.Axis[0] * Min[0]) +
                                (BestBounds.Axis[1] * Min[1]) +
                                (BestBounds.Axis[2] * Min[2]);

            // Scale axiis by length of sides
            BestBounds.Axis[0] *= (Max[0]-Min[0]);
            BestBounds.Axis[1] *= (Max[1]-Min[1]);
            BestBounds.Axis[2] *= (Max[2]-Min[2]);
        }
    }

    // Get center point
    vector3 MN,MX;
    MN = BestBounds.Corner;
    MX = BestBounds.Corner + BestBounds.Axis[0] + BestBounds.Axis[1] + BestBounds.Axis[2];
    pSubMesh->MidPoint.X = (MN.X + MX.X) * 0.5f;
    pSubMesh->MidPoint.Y = (MN.Y + MX.Y) * 0.5f;
    pSubMesh->MidPoint.Z = (MN.Z + MX.Z) * 0.5f;
    pSubMesh->MidPoint = pSubMesh->MidPoint;

//    pSubMesh->Area = BestBounds.Area;
    pSubMesh->Axis[0] = BestBounds.Axis[0];
    pSubMesh->Axis[1] = BestBounds.Axis[1];
    pSubMesh->Axis[2] = BestBounds.Axis[2];
    pSubMesh->Corner  = BestBounds.Corner;

}

static void SavePointGrid( )
{
    s32 i,j,RealCount = 0;

    // ok, so now we know our grid points and which submeshes are attached.  so it's time to save the file, but first
    // let's remove all the 0 submesh attached points so we save some processing time
    for( i = 0; i < s_PGrid.PointCount; i++ )
    {
        if( s_PGrid.SubMeshesPerGridPoint[i] != 0 )
            RealCount++;
    }

    // ok, now save
    if( g_Verbose )
        x_printf( "Saving Grid Point Data..." );

    x_fwrite( &RealCount, sizeof(s32), 1, g_GridPointFile );
    x_fwrite( &s_PGrid.DistanceBPoints, sizeof(f32), 1, g_GridPointFile );

    for( i = 0; i < s_PGrid.PointCount; i++ )
    {
        if( s_PGrid.SubMeshesPerGridPoint[i] == 0 ) continue;  // ignore empty grid points

        // write the grid point position
        x_fwrite( &s_PGrid.GridPoints[i], sizeof(vector3), 1, g_GridPointFile );

        // write out how many submeshes this point has
        x_fwrite( &s_PGrid.SubMeshesPerGridPoint[i], sizeof(s32), 1, g_GridPointFile );

        // now fill in the submesh IDs
        for( j = 0; j < s_PGrid.SubMeshesPerGridPoint[i]; j++ )
            x_fwrite( &s_PGrid.SubMeshesAtGridPoint[i][j], sizeof(s32), 1, g_GridPointFile );

    }

    if( g_Verbose )
        x_printf( "Done.\n" );

}

static
void ExportMaterials( mesh::object& rGeom, X_FILE* outfile )
{
    // NMaterials is saved in the hdr information
    // for each material write out the stage count and the stage indices
    // for each stage, write out a texture index
    //  put in placeholders for a crapload of other information, but (for now) all we need is the texture index
    //  the texture index should be in the same order as the texture names are listed in the output file

    if (!s_pMaterialSet)
        return;

    s32 i;

    for (i=0; i<s_pMaterialSet->m_nMaterials; ++i)
    {
        x_fwrite( &s_pMaterialSet->m_pMaterials[ i ].m_nStages, sizeof( s32 ), 1, outfile );

        s32 j;
        for (j=0; j<4; ++j)
            x_fwrite( &s_pMaterialSet->m_pMaterials[ i ].m_iStages[ j ], sizeof( s32 ), 1, outfile );
    }

    for (i=0; i<s_pMaterialSet->m_nStages; ++i)
    {
        x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_iTexture, sizeof( s32 ), 1, outfile );
        // below this point is more "advanced" material information.  It is not currently necessary
        //  in ASB, but could become so in future versions.
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_iUV, sizeof( s32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_iTransform, sizeof( s32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_Flags, sizeof( u32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_Alpha, sizeof( f32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_ActiveDistance, sizeof( f32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_Op, sizeof( u32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_BlendOp, sizeof( u32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_WrapU, sizeof( u32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_WrapV, sizeof( u32 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_UvScale, sizeof( vector2 ), 1, outfile );
        //x_fwrite( &s_pMaterialSet->m_pStages[ i ].m_UvMod, sizeof( mesh::uv_mod ), 1, outfile );
    }
}

static s32 IsCollisionData( char *Name )
{
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_FOULPOLE" ) )
        return COLL_TRI_FLAGS_FOULPOLE;
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_GROUND" ) )
        return COLL_TRI_FLAGS_GROUND;
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_LOW" ) )
        return COLL_TRI_FLAGS_LOW;
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_MEDIUM" ) )
        return COLL_TRI_FLAGS_MEDIUM;
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_HIGH" ) )
        return COLL_TRI_FLAGS_HIGH;
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_CROWD" ) )
        return COLL_TRI_FLAGS_STOP;
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_HR" ) )
        return COLL_TRI_FLAGS_HOMERUN;
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_FOUL" ) )
        return COLL_TRI_FLAGS_FOUL;
	if( !x_strcmp( x_strtoupper(Name), "A_WAREHOUSE" ) )
        return COLL_TRI_FLAGS_AUDIO_WAREHOUSE;
    if( !x_strcmp( x_strtoupper(Name), "A_WAVELAND" ) )
        return COLL_TRI_FLAGS_AUDIO_WAVELAND;
    if( !x_strcmp( x_strtoupper(Name), "A_COLL_SPLASH" ) )
        return COLL_TRI_FLAGS_AUDIO_SPLASH;
    if( !x_strcmp( x_strtoupper(Name), "A_POOL" ) )
        return COLL_TRI_FLAGS_AUDIO_POOL;
    if( !x_strcmp( x_strtoupper(Name), "A_TRAIN" ) )
        return COLL_TRI_FLAGS_AUDIO_TRAIN;
    if( !x_strcmp( x_strtoupper(Name), "A_HILL" ) )
        return COLL_TRI_FLAGS_AUDIO_HILL;

    return -1;
}

//==========================================================================
//  ExportPCGeometry()
//      Exports the PC geom to a file
//==========================================================================
void ExportXBOXGeometry( mesh::object& rGeom, X_FILE* outfile )
{
    SGeomFileHeader_Multipass   hdr;
    s32                         i, m, t;
    s32                         vIndex;
    s32                         NIndices;
    s32                         MaxIndice;
    u32                         Flags;
    s32                         TriID;
    vector3                     TempVector;
    xbool                       bEnvMap = FALSE;

    s32 MaterialSetID = rGeom.GetMaterialSetId( mesh::MESH_TARGET_XBOX );
    if( MaterialSetID == -1 )
    {
        MaterialSetID = rGeom.GetMaterialSetId( mesh::MESH_TARGET_GENERIC );
        ASSERT( MaterialSetID != -1 );
    }
    s_pMaterialSet = &rGeom.m_pMaterialSets[ MaterialSetID ];

    // Make sure there is an export file.
    ASSERT( outfile );

    // Fill the file header with the default info.
    FillName( hdr.Name, rGeom.m_Name, MAX_MESH_NAME );
    if( g_Verbose )
        x_printf( "Geometry Name: %s\n", hdr.Name );

    hdr.Flags           = 0;
    hdr.NMeshes         = 0;
    hdr.NSubMeshes      = 0;
    hdr.NTextures       = s_pMaterialSet->m_nTextures;
    hdr.NTexturesInFile = (g_ExportTextures) ? s_pMaterialSet->m_nTextures : 0;

    hdr.NMaterials      = s_pMaterialSet->m_nMaterials;
    hdr.NMatStages      = s_pMaterialSet->m_nStages;

    //  Allocate some space for the data we will be building
    s_pMesh      = new t_GeomMesh[MAX_NUM_MESHES];          ASSERT( s_pMesh );
    s_pSubMesh   = new t_GeomSubMesh[MAX_NUM_SUBMESHES];    ASSERT( s_pSubMesh );

    BuildStadiumBoundingBoxAndPGrid( rGeom );

    ////////////////////////////////////////////////////////////////////////
    //  BUILD MESHES
    //  Loop through each of the meshes adding them to the data
    ////////////////////////////////////////////////////////////////////////
    s32 NumChunks = rGeom.m_nChunks;
    s32 CurrentMesh = 0;
    for( i = 0; i < NumChunks; i++ )
    {
        // Test for mesh overflow.
        ASSERT( hdr.NMeshes < MAX_NUM_MESHES );
        ASSERT( hdr.NSubMeshes < MAX_NUM_SUBMESHES );

        //  Fill in the basic mesh data
        FillName( s_pMesh[CurrentMesh].Name, rGeom.m_pChunks[i].m_Name, MAX_MESH_NAME );

        bEnvMap = FALSE;
        if (IsEnvMapped( s_pMesh[ CurrentMesh ].Name ) )
        {
            s_pMesh[CurrentMesh].Flags |= MESH_FLAG_ENVMAPPED;
            bEnvMap = TRUE;
        }

        //  Check to see if this Mesh is collision data (if it is it will be saved to Collision and ignored in Geom
        s32 bCollision = IsCollisionData( s_pMesh[CurrentMesh].Name );
        if( !g_CollisionFile && (bCollision > -1) )
        {
            if ( g_Verbose ) x_printf( "Collision data skipped, no collision file specified.\n" );
            continue;
        }

        // Test for platform specific flags if alpha is used, and if Antialiasing is used.
        Flags  = 0;
        Flags |= g_EnableAlpha ? PRIVFLAG_MESH_ALPHA : 0;
        Flags |= g_EnableAA    ? PRIVFLAG_MESH_AA    : 0;
        s_pMesh[CurrentMesh].PrivateFlags  = Flags;

        // Determine the first (main) submesh, and it's size.
        s_pMesh[CurrentMesh].FirstSubMesh  = s_CurSubMesh;
        s_pMesh[CurrentMesh].Radius        = CalculateMeshRadius( rGeom, i );

        // Determine what the vertex format required to draw this mesh.
        s_pMesh[CurrentMesh].VertFormat     = bEnvMap ? (VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1)    : (VERTEX_FORMAT_XYZ_ARGB_TEX1);
        s_pMesh[CurrentMesh].VertFormatSize = bEnvMap ? sizeof(VertFormat_XYZ_ARGB_NORM_TEX1) : sizeof(VertFormat_XYZ_ARGB_TEX1);
        s_pMesh[CurrentMesh].VertUsage      = g_BuildTriStrips  ? 0               : 0;
        
        s_pMesh[CurrentMesh].NSubMeshes    = 0;
        s_pMesh[CurrentMesh].NVerts        = rGeom.m_pChunks[i].m_nVerts;
        s_pMesh[CurrentMesh].NIndices      = 0;
        s_pMesh[CurrentMesh].pVerts        = NULL;
        s_pMesh[CurrentMesh].pIndices      = NULL;

        // Fill the mesh specifics flags.
        Flags  = MESH_FLAG_VISIBLE;
        Flags |= g_DynamicLighting ? MESH_FLAG_DYNAMIC_LIGHTING : 0;
        Flags |= MESH_FLAG_DYNAMIC_LIGHTING;
        Flags |= g_UseMips         ? MESH_FLAG_USE_MIPS         : 0;
        Flags |= bEnvMap           ? MESH_FLAG_ENVMAPPED        : 0;
        s_pMesh[CurrentMesh].Flags = Flags;
        if (IsSpecialMesh( rGeom.m_pChunks[i].m_Name ))
            s_pMesh[CurrentMesh].Flags |= MESH_FLAG_SPECIAL;

        //  Create Empty Indice pool for submeshes in mesh
        s_pMesh[CurrentMesh].pIndices = (u16*)x_malloc( sizeof(u16) * MAX_NUM_PRIMS );
        ASSERT( s_pMesh[CurrentMesh].pIndices );

        // Print the mesh name
        if ( g_Verbose )
            x_printf( "  Mesh Name: %s\n", s_pMesh[CurrentMesh].Name );

        // Create a vertex pool and fill in the vertecies from the chunk.
        if( !g_CollisionFile || bCollision < 0 )
        {
            switch( s_pMesh[CurrentMesh].VertFormat )
            {
                case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3:
                {
                    // Allocate a block of memory large enough to store the vertex list from the Import File.
                    VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3*)x_malloc( sizeof(VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3) * s_pMesh[CurrentMesh].NVerts );
                    ASSERT( pVertexBuffer );

                    // For each vertex in the impore file, copy only the components that we need for this vertex format.
                    for ( vIndex = 0; vIndex < s_pMesh[CurrentMesh].NVerts; ++vIndex )
                    {
                        pVertexBuffer[vIndex].Pos    = rGeom.m_pChunks[i].m_pVerts[vIndex].m_vPos;
                        pVertexBuffer[vIndex].Color  = D3DCOLOR_ARGB( (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.W * 255.0f), 
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X) * 255.0f),
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y) * 255.0f),
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z) * 255.0f) );
                        pVertexBuffer[vIndex].Color  = D3DCOLOR_ARGB( (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.W * 255.0f), 
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X) * 148.0f),
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y) * 160.0f),
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z) * 160.0f) );
                        pVertexBuffer[vIndex].Normal = rGeom.m_pChunks[i].m_pVerts[vIndex].m_vNormal;
                        pVertexBuffer[vIndex].UV1    = rGeom.m_pChunks[i].m_pVerts[vIndex].m_UVs[0];
                        pVertexBuffer[vIndex].UV2    = rGeom.m_pChunks[i].m_pVerts[vIndex].m_UVs[1];
                        pVertexBuffer[vIndex].UV3    = rGeom.m_pChunks[i].m_pVerts[vIndex].m_UVs[2];
                    }

                    // Give this buffer over to the mesh object.
                    s_pMesh[CurrentMesh].pVerts = (void*)pVertexBuffer;
                    break;
                }

                case VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1:
                {
                    VertFormat_XYZ_ARGB_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_NORM_TEX1*)x_malloc( sizeof(VertFormat_XYZ_ARGB_NORM_TEX1) * s_pMesh[CurrentMesh].NVerts );
                    ASSERT( pVertexBuffer );

                    // For each vertex in the import file, copy only the components that we need for this vertex format.
                    for ( vIndex = 0; vIndex < s_pMesh[CurrentMesh].NVerts; ++vIndex )
                    {
                        pVertexBuffer[vIndex].Pos    = rGeom.m_pChunks[i].m_pVerts[vIndex].m_vPos;
                        pVertexBuffer[vIndex].Color  = D3DCOLOR_ARGB( (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.W * 255.0f), 
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X) * 255.0f),
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y) * 255.0f),
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z) * 255.0f) );
                        pVertexBuffer[vIndex].Color  = D3DCOLOR_ARGB( (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.W * 255.0f), 
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X) * 148.0f),
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y) * 160.0f),
                                                                      (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z) * 160.0f) );
                        pVertexBuffer[vIndex].Normal = rGeom.m_pChunks[i].m_pVerts[vIndex].m_vNormal;
                        pVertexBuffer[vIndex].UV     = rGeom.m_pChunks[i].m_pVerts[vIndex].m_UVs[0];
                    }

                    // Give this buffer over to the mesh object.
                    s_pMesh[CurrentMesh].pVerts = (void*)pVertexBuffer;
                    break;
                }

                case VERTEX_FORMAT_XYZ_ARGB_TEX1:
                {
                    // Allocate a block of memory large enough to store the vertex list from the Import File.
                    VertFormat_XYZ_ARGB_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_TEX1*)x_malloc( sizeof(VertFormat_XYZ_ARGB_TEX1) * s_pMesh[CurrentMesh].NVerts );
                    ASSERT( pVertexBuffer );

                    // For each vertex in the import file, copy only the components that we need for this vertex format.
                    for ( vIndex = 0; vIndex < s_pMesh[CurrentMesh].NVerts; ++vIndex )
                    {
                        pVertexBuffer[vIndex].Pos   = rGeom.m_pChunks[i].m_pVerts[vIndex].m_vPos;
                        pVertexBuffer[vIndex].Color = D3DCOLOR_ARGB( (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.W * 255.0f), 
                                                                     (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X) * 255.0f),
                                                                     (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y) * 255.0f),
                                                                     (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z * rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z) * 255.0f) );
                        pVertexBuffer[vIndex].Color = D3DCOLOR_ARGB( (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.W * 255.0f), 
                                                                     (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X) * 148.0f),
                                                                     (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y) * 160.0f),
                                                                     (u8)((rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z) * 160.0f) );
                        pVertexBuffer[vIndex].UV    = rGeom.m_pChunks[i].m_pVerts[vIndex].m_UVs[0];
                    }

                    // Give this buffer over to the mesh object.
                    s_pMesh[CurrentMesh].pVerts = (void*)pVertexBuffer;
                    break;
                }

                default:
                    ASSERT( 0 );
                    break;
           
            }
        }
        
        //  Build the submeshes
        {
            s32 NTotalTris = 0;
            s32 TriCount;

            // Create a copy of the triangle data that came from the import file.
            BuildTriData( rGeom.m_pChunks[i] );

            if( bCollision > -1 )
            {
                if( !g_CollisionFile ) return;
                for( TriID = 0; TriID < s_NTris; TriID++ )
                {
                    t_CollisionTri  CollisionTri;
                    CollisionTri.Type = bCollision;

                    //---   Save the position in collision data (may not write though)
                    CollisionTri.Verts[0] = rGeom.m_pChunks[i].m_pVerts[s_TriData[TriID].Index[0]].m_vPos;
                    CollisionTri.Verts[1] = rGeom.m_pChunks[i].m_pVerts[s_TriData[TriID].Index[1]].m_vPos;
                    CollisionTri.Verts[2] = rGeom.m_pChunks[i].m_pVerts[s_TriData[TriID].Index[2]].m_vPos;

                    x_fwrite( &CollisionTri, sizeof(t_CollisionTri), 1, g_CollisionFile );
                }
            }
            else
            {
                s32 StartTris[500];  // size of the array should be the number of materials possible, but I'm being lazy right now
                x_memset( StartTris, -1, sizeof(s32) * 500 );
                s32 GridNumber,CurMaterial = -1,HighestMaterial = -1;
                for ( TriID = 0; TriID < s_NTris; TriID++ )
                {
                    if( s_TriData[TriID].MaterialID != CurMaterial )
                    {
                        StartTris[s_TriData[TriID].MaterialID] = TriID;
                        CurMaterial = s_TriData[TriID].MaterialID;
                        if( CurMaterial > HighestMaterial ) HighestMaterial = CurMaterial;
                    }
                }
                //  Each set of faces with a unique material becomes a submesh
                CurMaterial = -1;
                while ( NTotalTris < s_NTris )
                {
                    CurMaterial++;
                    if( CurMaterial == 500 || CurMaterial > HighestMaterial ) 
                    {
                        for( TriID = 0; TriID < s_NTris; TriID++ )
                        {
                            if( s_TriData[TriID].Used == FALSE )
                            {
                                ASSERT(FALSE);
                            }
                        }
                        break;
                    }
                    if( StartTris[CurMaterial] == -1 ) continue;
                    for( GridNumber = 0; GridNumber < s_PGrid.PointCount; GridNumber++ )
                    {
                        // Count the number of tris using the next material
                        TriCount = CountTrisWithSameGridPointAndMaterial( &rGeom.m_pChunks[i], GridNumber, StartTris[CurMaterial] );

                        if( TriCount == 0 ) continue;

                        // Build SubMeshes out of the triangles that share the same material.
                        if( g_BuildTriStrips )
                        {
                            s32                 s;
                            s32                 NStrips = 0, nTris = 0;
                            t_TriangleStrip*    pStrips     = NULL;
                            u16*                pIndices    = (u16*)s_pMesh[CurrentMesh].pIndices;
							t_Triangle			*pTris;


                            // Build this submesh from triangle strips.
                            BuildStrips( s_TriData, TriCount, (u16*)s_SubMeshTriData/*pIndices*/, &pStrips, NStrips, &pTris, nTris, MAX_NUM_VERTS, FALSE );

                            ///////////////////////////////////////////////////////////////
                            //  Fill in submesh struct
                            ///////////////////////////////////////////////////////////////
                            s_pSubMesh[s_CurSubMesh].MeshID         = CurrentMesh;
                            s_pSubMesh[s_CurSubMesh].Flags          = 0;
                            s32 AlphaMode = GetAlphaMode( NTotalTris, rGeom );
                            if( AlphaMode == mesh::ALPHA_MODE_PUNCHTHROUGH ) s_pSubMesh[s_CurSubMesh].Flags |= SUBMESH_FLAG_ALPHA_P;
                            if( AlphaMode == mesh::ALPHA_MODE_TRANSPARENT ) s_pSubMesh[s_CurSubMesh].Flags |= SUBMESH_FLAG_ALPHA_T;
                            s_pSubMesh[s_CurSubMesh].IndexStart     = s_pMesh[CurrentMesh].NIndices;
                            s_pSubMesh[s_CurSubMesh].IndexMin       = MAX_NUM_PRIMS;
                            s_pSubMesh[s_CurSubMesh].IndexCount     = 0;
                            s_pSubMesh[s_CurSubMesh].NTris          = 0;
                            s_pSubMesh[s_CurSubMesh].TextureID[0]   = GetTextureID( NTotalTris, rGeom, 0 );
                            s_pSubMesh[s_CurSubMesh].TextureID[1]   = GetTextureID( NTotalTris, rGeom, 1 );
                            s_pSubMesh[s_CurSubMesh].TextureID[2]   = GetTextureID( NTotalTris, rGeom, 2 );

                            s_pSubMesh[s_CurSubMesh].AvgPixelSize   = 0;
                            s_pSubMesh[s_CurSubMesh].MaxPixelSize   = 0;
                            s_pSubMesh[s_CurSubMesh].MinPixelSize   = 0;
                            s_pSubMesh[s_CurSubMesh].NVertsTrans    = 0;

                            s_pSubMesh[s_CurSubMesh].PrimType       = D3DPT_TRIANGLESTRIP;

                            NIndices                                = 0;
                            MaxIndice                               = 0;

                            BuildSubMeshRegionData( &s_pSubMesh[s_CurSubMesh], rGeom, i, s_pMesh[CurrentMesh].NIndices );

                            // Now go through each of the triangle stips created and add the indeces used
                            // to the indece list for the mesh.
                            for( s = 0; s < NStrips; s++ )
                            {
                                for( vIndex = 0; vIndex < pStrips[s].NVerts; vIndex++ )
                                {
                                    // POINT GRID ATTACHMENT
                                    for( s32 GridID = 0; GridID < s_PGrid.PointCount; GridID++ )
                                    {
                                        if( s_PGrid.SubMeshesPerGridPoint[GridID] && s_PGrid.SubMeshesAtGridPoint[GridID][s_PGrid.SubMeshesPerGridPoint[GridID]-1] == s_CurSubMesh ) continue;
                                        TempVector = rGeom.m_pChunks[i].m_pVerts[pStrips[s].pVerts[vIndex]].m_vPos - s_PGrid.GridPoints[GridID];
                                        if( TempVector.Length() <= s_PGrid.DistanceBPoints )
                                        {
                                            s_PGrid.SubMeshesAtGridPoint[GridID][s_PGrid.SubMeshesPerGridPoint[GridID]] = s_CurSubMesh;
                                            s_PGrid.SubMeshesPerGridPoint[GridID]++;
                                            ASSERT( s_PGrid.SubMeshesPerGridPoint[GridID] < MAX_SUBMESHES_AT_POINT );
                                        }
                                    }
                                    
                                    //  Add indice to mesh indice pool
                                    ASSERT(s_pMesh[CurrentMesh].NIndices < MAX_NUM_PRIMS);
                                    pIndices[s_pMesh[CurrentMesh].NIndices] = pStrips[s].pVerts[vIndex];
                            
                                    // Imcrement the number of indeces in the mesh as well as the local counter.
                                    s_pMesh[CurrentMesh].NIndices++;
                                    NIndices++;

                                    //  Find the minimum and maximum Vertex index used for this submesh.
                                    s_pSubMesh[s_CurSubMesh].IndexMin = 
                                        MIN(s_pSubMesh[s_CurSubMesh].IndexMin, pStrips[s].pVerts[vIndex] );

                                    MaxIndice = MAX(MaxIndice, pStrips[s].pVerts[vIndex] );
                                }
                        
                                // If this isn't the last strip, connect this strip to the next one with 2
                                // garbage triangles.
                                if( s < (NStrips - 1) )
                                {
                                    ASSERT(s_pMesh[CurrentMesh].NIndices + 2 < MAX_NUM_PRIMS);

                                    pIndices[s_pMesh[CurrentMesh].NIndices] = pIndices[s_pMesh[CurrentMesh].NIndices-1];
                                    s_pMesh[CurrentMesh].NIndices++;
                            
                                    pIndices[s_pMesh[CurrentMesh].NIndices] = pStrips[s+1].pVerts[0];
                                    s_pMesh[CurrentMesh].NIndices++;

                                    NIndices += 2;
                                }
                            }

                            for( s = 0; s < nTris; s++ )
                            {
                                for( vIndex = 0; vIndex < 3; vIndex++ )
                                {
                                    // POINT GRID ATTACHMENT
                                    for( s32 GridID = 0; GridID < s_PGrid.PointCount; GridID++ )
                                    {
                                        if( s_PGrid.SubMeshesPerGridPoint[GridID] && s_PGrid.SubMeshesAtGridPoint[GridID][s_PGrid.SubMeshesPerGridPoint[GridID]-1] == s_CurSubMesh ) continue;
                                        TempVector = rGeom.m_pChunks[i].m_pVerts[pTris[s]. Index[vIndex]].m_vPos - s_PGrid.GridPoints[GridID];
                                        if( TempVector.Length() <= s_PGrid.DistanceBPoints )
                                        {
                                            s_PGrid.SubMeshesAtGridPoint[GridID][s_PGrid.SubMeshesPerGridPoint[GridID]] = s_CurSubMesh;
                                            s_PGrid.SubMeshesPerGridPoint[GridID]++;
                                            ASSERT( s_PGrid.SubMeshesPerGridPoint[GridID] < MAX_SUBMESHES_AT_POINT );
                                        }
                                    }

                                    //  Add indice to mesh indice pool
                                    ASSERT(s_pMesh[CurrentMesh].NIndices < MAX_NUM_PRIMS);
                                    pIndices[s_pMesh[CurrentMesh].NIndices] = pTris[s]. Index[vIndex];
                            
                                    // Imcrement the number of indeces in the mesh as well as the local counter.
                                    s_pMesh[CurrentMesh].NIndices++;
                                    NIndices++;

                                    //  Find the minimum and maximum Vertex index used for this submesh.
                                    s_pSubMesh[s_CurSubMesh].IndexMin = 
                                        MIN(s_pSubMesh[s_CurSubMesh].IndexMin, pTris[s].Index[vIndex] );

                                    MaxIndice = MAX(MaxIndice, pTris[s].Index[vIndex] );
                                }
                        
                                // If this isn't the last strip, connect this strip to the next one with 2
                                // garbage triangles.
                                if( s < (nTris - 1) )
                                {
                                    ASSERT(s_pMesh[CurrentMesh].NIndices + 2 < MAX_NUM_PRIMS);

                                    pIndices[s_pMesh[CurrentMesh].NIndices] = pIndices[s_pMesh[CurrentMesh].NIndices-1];
                                    s_pMesh[CurrentMesh].NIndices++;
                            
                                    pIndices[s_pMesh[CurrentMesh].NIndices] = pTris[s+1].Index[0];
                                    s_pMesh[CurrentMesh].NIndices++;

                                    NIndices += 2;
                                }
                            }

                            //  Clean up strip data if needed
                            if (pStrips) x_free(pStrips); 

                            //  Update the number of tris and indeces used by this submesh.
                            s_pSubMesh[s_CurSubMesh].NTris = NIndices - 2;
                            s_pSubMesh[s_CurSubMesh].IndexCount = MaxIndice + 1 - s_pSubMesh[s_CurSubMesh].IndexMin;

                            //  Increment submesh count
                            s_CurSubMesh++;
                            s_pMesh[CurrentMesh].NSubMeshes++;
                        }
                        else
                        {
                            u16* pIndices = (u16*)s_pMesh[CurrentMesh].pIndices;

                            //  Fill in submesh struct
                            s_pSubMesh[s_CurSubMesh].MeshID         = CurrentMesh;
                            s_pSubMesh[s_CurSubMesh].Flags          = 0;
                            s32 AlphaMode = GetAlphaMode( NTotalTris, rGeom );
                            if( AlphaMode == mesh::ALPHA_MODE_PUNCHTHROUGH ) s_pSubMesh[s_CurSubMesh].Flags |= SUBMESH_FLAG_ALPHA_P;
                            if( AlphaMode == mesh::ALPHA_MODE_TRANSPARENT ) 
                                s_pSubMesh[s_CurSubMesh].Flags |= SUBMESH_FLAG_ALPHA_T;
                            s_pSubMesh[s_CurSubMesh].IndexStart     = s_pMesh[CurrentMesh].NIndices;
                            s_pSubMesh[s_CurSubMesh].IndexMin       = MAX_NUM_PRIMS;
                            s_pSubMesh[s_CurSubMesh].IndexCount     = 0;
                            s_pSubMesh[s_CurSubMesh].NTris          = TriCount;
                            s_pSubMesh[s_CurSubMesh].TextureID[0]   = GetTextureID( NTotalTris, rGeom, 0 );
                            ASSERT( s_pSubMesh[s_CurSubMesh].TextureID[0] != -1 );
                            s_pSubMesh[s_CurSubMesh].TextureID[1]   = GetTextureID( NTotalTris, rGeom, 1 );
                            s_pSubMesh[s_CurSubMesh].TextureID[2]   = GetTextureID( NTotalTris, rGeom, 2 );

                            s_pSubMesh[s_CurSubMesh].AvgPixelSize   = 0;
                            s_pSubMesh[s_CurSubMesh].MaxPixelSize   = 0;
                            s_pSubMesh[s_CurSubMesh].MinPixelSize   = 0;
                            s_pSubMesh[s_CurSubMesh].NVertsTrans    = 0;

                            s_pSubMesh[s_CurSubMesh].PrimType       = D3DPT_TRIANGLELIST;

                            NIndices                                = 0;
                            MaxIndice                               = 0;

                            BuildSubMeshRegionData( &s_pSubMesh[s_CurSubMesh], rGeom, i, s_pMesh[CurrentMesh].NIndices );

                            // Go through each triangle that shares this material.
                            for ( t = 0; t < TriCount; t++ )
                            {
                                for ( vIndex = 0; vIndex < 3; vIndex++ )
                                {
                                    ASSERT(s_pMesh[CurrentMesh].NIndices < MAX_NUM_PRIMS);

                                    //  Add indice to mesh indice pool
                                    pIndices[s_pMesh[CurrentMesh].NIndices] = s_TriData[s_SubMeshTriData[t]].Index[vIndex];

                                    // POINT GRID ATTACHMENT
                                    for( s32 GridID = 0; GridID < s_PGrid.PointCount; GridID++ )
                                    {
                                        if( s_PGrid.SubMeshesPerGridPoint[GridID] && s_PGrid.SubMeshesAtGridPoint[GridID][s_PGrid.SubMeshesPerGridPoint[GridID]-1] == s_CurSubMesh ) continue;
                                        TempVector = rGeom.m_pChunks[i].m_pVerts[s_TriData[s_SubMeshTriData[t]].Index[vIndex]].m_vPos - s_PGrid.GridPoints[GridID];
                                        if( TempVector.Length() <= s_PGrid.DistanceBPoints )
                                        {
                                            s_PGrid.SubMeshesAtGridPoint[GridID][s_PGrid.SubMeshesPerGridPoint[GridID]] = s_CurSubMesh;
                                            s_PGrid.SubMeshesPerGridPoint[GridID]++;
                                            ASSERT( s_PGrid.SubMeshesPerGridPoint[GridID] < MAX_SUBMESHES_AT_POINT );
                                        }
                                    }

                                    s_pMesh[CurrentMesh].NIndices++;
                                    s_pSubMesh[s_CurSubMesh].IndexCount++;
                                    NIndices++;

                                    //  Find the minimum and maximum Vertex index used for this submesh.
                                    s_pSubMesh[s_CurSubMesh].IndexMin = 
                                        MIN(s_pSubMesh[s_CurSubMesh].IndexMin, s_TriData[s_SubMeshTriData[t]].Index[vIndex] );

                                    MaxIndice = 
                                        MAX(MaxIndice, s_TriData[s_SubMeshTriData[t]].Index[vIndex] );
                                }
                            }

                            //  Update the number of indeces used in this submesh.
        //                    s_pSubMesh[s_CurSubMesh].IndexCount = MaxIndice + 1 - s_pSubMesh[s_CurSubMesh].IndexMin;

                            //  Increment submesh count
                            s_CurSubMesh++;
                            s_pMesh[CurrentMesh].NSubMeshes++;
                        }

                        //  How many tris left to process
                        NTotalTris += TriCount;

//                        if( NTotalTris == s_NTris ) break;
                    }
                    ASSERT( s_pMesh[CurrentMesh].NSubMeshes );
                }
                ASSERT( s_pMesh[CurrentMesh].NSubMeshes );

                //  Free up the global tri data since it's been copied to the submeshes.
                delete[] s_TriData;
            }
        }

        if( g_CollisionFile && bCollision > -1 )
        {
            if ( g_Verbose )
                x_printf( " Saved as collision data.\n" );

            s_CurSubMesh -= s_pMesh[CurrentMesh].NSubMeshes;
            continue;
        }

        //  Display the number of Submeshes used.
        if ( g_Verbose )
            x_printf( "  NSubMeshes: %d\n", s_pMesh[CurrentMesh].NSubMeshes );

        //  Increase the mesh counts in the file header.
        hdr.NMeshes++;
        hdr.NSubMeshes += s_pMesh[CurrentMesh].NSubMeshes;
        CurrentMesh++;
    }

    //---   build the grid point data
    SavePointGrid( );

    //-------------------------------------------------------------------------------------------------------------------------------------
    //  Write the output file.
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Write the file header.
    x_fwrite( &hdr, sizeof(t_GeomFileHeader_Multipass), 1, outfile );

    // Write mesh struct with vert and indice pool
    for( m = 0; m < hdr.NMeshes; m++ )
    {
        x_fwrite( &s_pMesh[m], sizeof(t_GeomMesh), 1, outfile );    
        
        x_fwrite( s_pMesh[m].pVerts, s_pMesh[m].VertFormatSize, s_pMesh[m].NVerts, outfile );
        x_fwrite( s_pMesh[m].pIndices, sizeof(u16), s_pMesh[m].NIndices, outfile );

    }

    // Write out submeshes
    x_fwrite( s_pSubMesh, sizeof(t_GeomSubMesh), hdr.NSubMeshes, outfile );    

    // Write out the texture names
    x_printf("Exporting Texture Names\n");
    for( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
    {
        char TextureName[TEXTURE_NAME_LENGTH];
        GetTextureName( TextureName, rGeom, i );
        PadStringWithZero( TextureName, TEXTURE_NAME_LENGTH );
        x_fwrite( TextureName, sizeof(char), TEXTURE_NAME_LENGTH, outfile );
        x_printf( "TextureName[ %d ]: %s\n", i, TextureName );
    }

	//-- Print out the header of the texture file
    if( g_TextureNameFile )
    {
        x_fprintf( g_TextureNameFile, "[GLOBAL],GBL:1,SWZ:1,CMP:1,IMAP:0,4BIT:1,MIP:1,MIPCSTM:1,NUMMIPS:15,ALPHA:1,ALFNAME:1,ALFPUNCH:0,ALFGENC:0,CLRSCL:0:255:255:255:255\n");
    }

    // Write out the textures
    for( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
    {
        char NewDrive[5];
        char NewDir[255];
        char TestDir[255];
        char NewFileName[80];
        char NewExtension[10];
        u8   AlphaPercent = 255;
        xbool bImageIsALightmap = FALSE;
        

        x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );

        // Test to see if the word lightmaps is anywhere within the texture path.
        // If this image is a light map, then leave the image extension as is.
        x_strcpy( TestDir, NewDir );
        x_strtoupper( TestDir );

        bImageIsALightmap = x_strstr( TestDir, "LIGHTMAPS" ) != NULL;
        
        if( bImageIsALightmap == FALSE )
        {
            // Force the extension to be a BMP then rebuild, and re-split the path.
            x_strcpy( NewExtension, "BMP" );
            x_makepath(  s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
            x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
        }

        x_sprintf( NewFileName, "%s", x_strtoupper(NewFileName) );
        if( (NewFileName[2] == 'D') && g_NightVersion )
        {
            NewFileName[2] = 'N';
            x_makepath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
        }

        if( NewFileName[2] >= '0' && NewFileName[2] <= '9' )
        {
            AlphaPercent = (u8)(255 * ((NewFileName[2] - '0') * .1));
        }

        // now check to see if there is a 24-bit version of this texture so we can use that on the all mighty Xbox
        char Check24bit[80];
        char Check24bitFull[255];
        x_sprintf( Check24bit, "%s_24bit", NewFileName );
        x_makepath( Check24bitFull,  NewDrive, NewDir, Check24bit, NewExtension );
        X_FILE* CheckFile = NULL;
        CheckFile = x_fopen( Check24bitFull, "rb" );
        if( CheckFile != NULL )
        {
            x_strcpy( s_pMaterialSet->m_pTextures[i].m_Filename, Check24bitFull );
            x_fclose( CheckFile );
        }

        // If there is a texture output file open, printf the current texture name to the output file.
        if( g_TextureNameFile )
        {
            xbool AlphaFlag = ((!x_strncmp(&(NewFileName[3]),"CORONA",6)) || (!x_strncmp(&(NewFileName[4]),"CORONA",6)));

			if ( AlphaFlag || (AlphaPercent != 255)) 
			{
				x_fprintf( g_TextureNameFile, "%s,GBL:0,CMP:%d,ALPHA:1,ALFNAME:%d,ALFGENC:%d,MIP:%d,CLRSCL:%d:255:255:255:%d\n",
					s_pMaterialSet->m_pTextures[i].m_Filename,
					!AlphaFlag,//&& AlphaPercent == 255, // should we compress?
					!AlphaFlag,
					AlphaFlag,
					x_strncmp( "TEXTUREPAGE", NewFileName, x_strlen("TEXTUREPAGE") ) != 0 && g_UseMips, AlphaPercent != 255,
					AlphaPercent );
			}
			else 
			{
				x_fprintf( g_TextureNameFile, "%s\n",s_pMaterialSet->m_pTextures[i].m_Filename);
			}
        }

        // Export the textures with the GEOM file if requested.
        if( g_ExportTextures )
        {
		    x_bitmap	BMP;
		    xbool		ret;

		    if( g_Verbose )
			    x_printf( "Loading %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );

		    if( !x_stricmp( s_pMaterialSet->m_pTextures[i].m_Filename, "none" ) )
		    {
			    AUXBMP_SetupDefaultBitmap( BMP );
			    ret = TRUE;
		    }
		    else
		    {	ret = AUXBMP_LoadBitmap( BMP, s_pMaterialSet->m_pTextures[i].m_Filename );
		    }

		    if( !ret )
		    {
			    x_printf( "ERROR: Unable to load %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
                ASSERT( FALSE );
    //			PauseOnError();
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

			    //--- Create possible alpha texture filenames
			    x_sprintf( AlphaName, "%s_punch", Name );	x_makepath( PunchPath,  Drive, Dir, AlphaName, Ext );
			    x_sprintf( AlphaName, "%s_alpha", Name );	x_makepath( AlphaPath1, Drive, Dir, AlphaName, Ext );
			    x_sprintf( AlphaName, "%s_mask",  Name );	x_makepath( AlphaPath2, Drive, Dir, AlphaName, Ext );
			    x_sprintf( AlphaName, "%sAlpha",  Name );	x_makepath( AlphaPath3, Drive, Dir, AlphaName, Ext );

			    //--- Clear Alpha present
			    BMP.SetAllAlpha( 255 );

			    //--- Load alpha map if possible
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

			    ret = AUXBMP_ConvertToXBOX( BMP, g_UseMips, FALSE, TRUE );

			    if( !ret )
			    {
				    x_printf( "ERROR: Unable to convert %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
				    PauseOnError();
			    }
			    else
			    {
				    if( g_Verbose )
					    x_printf( "Saving %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );

				    ret = BMP.Save( outfile );
				    if( !ret )
				    {
					    x_printf( "ERROR: Unable to save %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
					    PauseOnError();
				    }
			    }
            }
        }
    }

    x_printf( "Exporting Materials\n" );
    ExportMaterials( rGeom, outfile );

    //  Clean Up all of that allocated memory now.
    for ( m = 0; m < hdr.NMeshes; m++ )
    {
        x_free( s_pMesh[m].pVerts );
        x_free( s_pMesh[m].pIndices );
    }
    delete []s_pMesh;
    delete []s_pSubMesh;

}
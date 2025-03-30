#include <stdlib.h>
#define POINTER_64 __ptr64

#include "d3d8.h"
#include "AUX_Bitmap.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "PC\PC_Geom.hpp"
#include "Q_Geom.hpp"
#include "RipGeom.h"
#include "Strip.h"
#include "PCRipper.h"


////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////
#define MAX_NUM_MESHES          64
#define MAX_NUM_SUBMESHES       1024
#define MAX_NUM_VERTS           65535 //D3DMAXNUMVERTICES
#define MAX_NUM_PRIMS           65535 //D3DMAXNUMPRIMITIVES
#define MAX_NUM_TRIS_IN_SUBMESH 10000


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

static mesh::material_set*  s_pMaterialSet = NULL;
static s32                  s_SubMeshTriData[MAX_NUM_TRIS_IN_SUBMESH];


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
        // Is this face a quad?
        if ( Chunk.m_pFaces[i].m_bQuad )
        {
            // Copy a quad, splitting it into two tris
            s_TriData[count].Index[0]   = Chunk.m_pFaces[i].m_Index[0];
            s_TriData[count].Index[1]   = Chunk.m_pFaces[i].m_Index[1];
            s_TriData[count].Index[2]   = Chunk.m_pFaces[i].m_Index[2];
            s_TriData[count].MaterialID = Chunk.m_pFaces[i].m_iMaterial;
            count++;

            s_TriData[count].Index[0]   = Chunk.m_pFaces[i].m_Index[0];
            s_TriData[count].Index[1]   = Chunk.m_pFaces[i].m_Index[2];
            s_TriData[count].Index[2]   = Chunk.m_pFaces[i].m_Index[3];
            s_TriData[count].MaterialID = Chunk.m_pFaces[i].m_iMaterial;
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

//-------------------------------------------------------------------------------------------------------------------------------------
s32 CompareMaterialIDs( const void* Element1, const void* Element2 )
{
    if( ((t_Triangle*)Element1)->MaterialID < ((t_Triangle*)Element2)->MaterialID )
        return -1;
    else 
    if( ((t_Triangle*)Element1)->MaterialID > ((t_Triangle*)Element2)->MaterialID )
        return 1;
    else
        return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 CountTrisWithSameMaterial( s32 StartIndex )
{
    s32 i;
    s32 count = 1;
    s32 CurMaterial = s_TriData[StartIndex].MaterialID;

    for( i = StartIndex + 1; i < s_NTris; i++ )
    {
        if ( s_TriData[i].MaterialID != CurMaterial )
            break;

        count++;
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
//  ExportPCGeometry()
//      Exports the PC geom to a file
//==========================================================================
void ExportPCGeometry( mesh::object& rGeom, X_FILE* outfile )
{
    SGeomFileHeader     hdr;
    s32                 i, m, t;
    s32                 vIndex;
    s32                 NIndices;
    s32                 MaxIndice;
    u32                 Flags;

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

    //  Allocate some space for the data we will be building
    s_pMesh      = new t_GeomMesh[MAX_NUM_MESHES];          ASSERT( s_pMesh );
    s_pSubMesh   = new t_GeomSubMesh[MAX_NUM_SUBMESHES];    ASSERT( s_pSubMesh );

    ////////////////////////////////////////////////////////////////////////
    //  BUILD MESHES
    //  Loop through each of the meshes adding them to the data
    ////////////////////////////////////////////////////////////////////////
    for( i = 0; i < rGeom.m_nChunks; i++ )
    {
        // Test for mesh overflow.
        ASSERT( hdr.NMeshes < MAX_NUM_MESHES );
        ASSERT( hdr.NSubMeshes < MAX_NUM_SUBMESHES );

        //  Fill in the basic mesh data
        FillName( s_pMesh[i].Name, rGeom.m_pChunks[i].m_Name, MAX_MESH_NAME );

        // Test for platform specific flags if alpha is used, and if Antialiasing is used.
        Flags  = 0;
        Flags |= g_EnableAlpha ? PRIVFLAG_MESH_ALPHA : 0;
        Flags |= g_EnableAA    ? PRIVFLAG_MESH_AA    : 0;
        s_pMesh[i].PrivateFlags  = Flags;

        // Determine the first (main) submesh, and it's size.
        s_pMesh[i].FirstSubMesh  = s_CurSubMesh;
        s_pMesh[i].Radius        = CalculateMeshRadius( rGeom, i );

        // Determine what the vertex format required to draw this mesh.
        s_pMesh[i].VertFormat     = g_DynamicLighting ? (VERTEX_FORMAT_XYZ_NORM_TEX1)    : (VERTEX_FORMAT_XYZ_ARGB_TEX1);
        s_pMesh[i].VertFormatSize = g_DynamicLighting ? sizeof(VertFormat_XYZ_NORM_TEX1) : sizeof(VertFormat_XYZ_ARGB_TEX1);
        s_pMesh[i].VertUsage      = g_BuildTriStrips  ? D3DUSAGE_DONOTCLIP               : 0;
        
        s_pMesh[i].NSubMeshes    = 0;
        s_pMesh[i].NVerts        = rGeom.m_pChunks[i].m_nVerts;
        s_pMesh[i].NIndices      = 0;
        s_pMesh[i].pVerts        = NULL;
        s_pMesh[i].pIndices      = NULL;

        // Fill the mesh specifics flags.
        Flags  = MESH_FLAG_VISIBLE;
        Flags |= g_DynamicLighting ? MESH_FLAG_DYNAMIC_LIGHTING : 0;
        Flags |= g_UseMips         ? MESH_FLAG_USE_MIPS         : 0;
        s_pMesh[i].Flags = Flags;

        // Create a vertex pool and fill in the vertecies from the chunk.
        switch( s_pMesh[i].VertFormat )
        {
            case VERTEX_FORMAT_XYZ_NORM_TEX1:
            {
                // Allocate a block of memory large enough to store the vertex list from the Import File.
                VertFormat_XYZ_NORM_TEX1* pVertexBuffer = (VertFormat_XYZ_NORM_TEX1*)x_malloc( sizeof(VertFormat_XYZ_NORM_TEX1) * s_pMesh[i].NVerts );
                ASSERT( pVertexBuffer );

                // For each vertex in the impore file, copy only the components that we need for this vertex format.
                for ( vIndex = 0; vIndex < s_pMesh[i].NVerts; vIndex++ )
                {
                    pVertexBuffer[vIndex].Pos       = rGeom.m_pChunks[i].m_pVerts[vIndex].m_vPos;
                    pVertexBuffer[vIndex].Normal    = rGeom.m_pChunks[i].m_pVerts[vIndex].m_vNormal;
                    pVertexBuffer[vIndex].UV        = rGeom.m_pChunks[i].m_pVerts[vIndex].m_UVs[0];
                }

                // Give this buffer over to the mesh object.
                s_pMesh[i].pVerts = (void*)pVertexBuffer;
                break;
            }

            case VERTEX_FORMAT_XYZ_ARGB_TEX1:
            {
                // Allocate a block of memory large enough to store the vertex list from the Import File.
                VertFormat_XYZ_ARGB_TEX1* pVertexBuffer = (VertFormat_XYZ_ARGB_TEX1*)x_malloc( sizeof(VertFormat_XYZ_ARGB_TEX1) * s_pMesh[i].NVerts );
                ASSERT( pVertexBuffer );

                // For each vertex in the import file, copy only the components that we need for this vertex format.
                for ( vIndex = 0; vIndex < s_pMesh[i].NVerts; vIndex++ )
                {
                    pVertexBuffer[vIndex].Pos   = rGeom.m_pChunks[i].m_pVerts[vIndex].m_vPos;
                    pVertexBuffer[vIndex].Color = D3DCOLOR_ARGB( (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.W * 255.0f), 
                                                                 (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.X * 255.0f),
                                                                 (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Y * 255.0f),
                                                                 (u8)(rGeom.m_pChunks[i].m_pVerts[vIndex].m_Color.Z * 255.0f) );
                    pVertexBuffer[vIndex].UV    = rGeom.m_pChunks[i].m_pVerts[vIndex].m_UVs[0];
                }

                // Give this buffer over to the mesh object.
                s_pMesh[i].pVerts = (void*)pVertexBuffer;
                break;
            }

            default:
                ASSERT( 0 );
                break;
           
        }
        
        //  Create Empty Indice pool for submeshes in mesh
        s_pMesh[i].pIndices = (u16*)x_malloc( sizeof(u16) * MAX_NUM_PRIMS );
        ASSERT( s_pMesh[i].pIndices );

        // Print the mesh name
        if ( g_Verbose )
            x_printf( "  Mesh Name: %s\n", s_pMesh[i].Name );

        //  Build the submeshes
        {
            s32 NTotalTris = 0;
            s32 TriCount;

            // Create a copy of the triangle data that came from the import file.
            BuildTriData( rGeom.m_pChunks[i] );

            //  Each set of faces with a unique material becomes a submesh
            while ( NTotalTris < s_NTris )
            {
                // Count the number of tris using the next material
                TriCount = CountTrisWithSameMaterial( NTotalTris );

                // Build SubMeshes out of the triangles that share the same material.
                if( g_BuildTriStrips )
                {
                    s32                 s;
                    s32                 NStrips     = 0;
                    t_TriangleStrip*    pStrips     = NULL;
                    s16*                pIndices    = (s16*)s_pMesh[i].pIndices;

                    // Build this submesh from triangle strips.
//                    BuildStrips( &s_TriData[NTotalTris], TriCount, &pStrips, NStrips, MAX_NUM_VERTS );

                    ///////////////////////////////////////////////////////////////
                    //  Fill in submesh struct
                    ///////////////////////////////////////////////////////////////
                    s_pSubMesh[s_CurSubMesh].Flags          = 0;
                    s_pSubMesh[s_CurSubMesh].IndexStart     = s_pMesh[i].NIndices;
                    s_pSubMesh[s_CurSubMesh].IndexMin       = MAX_NUM_PRIMS;
                    s_pSubMesh[s_CurSubMesh].IndexCount     = 0;
                    s_pSubMesh[s_CurSubMesh].NTris          = 0;
                    s_pSubMesh[s_CurSubMesh].TextureID[0]   = s_TriData[NTotalTris].MaterialID;
                    s_pSubMesh[s_CurSubMesh].TextureID[1]   = -1;
                    s_pSubMesh[s_CurSubMesh].TextureID[2]   = -1;

                    s_pSubMesh[s_CurSubMesh].AvgPixelSize   = 0;
                    s_pSubMesh[s_CurSubMesh].MaxPixelSize   = 0;
                    s_pSubMesh[s_CurSubMesh].MinPixelSize   = 0;
                    s_pSubMesh[s_CurSubMesh].NVertsTrans    = 0;

                    s_pSubMesh[s_CurSubMesh].PrimType       = D3DPT_TRIANGLESTRIP;

                    NIndices                                = 0;
                    MaxIndice                               = 0;

                    // Now go through each of the triangle stips created and add the indeces used
                    // to the indece list for the mesh.
                    for( s = 0; s < NStrips; s++ )
                    {
                        for( vIndex = 0; vIndex < pStrips[s].NVerts; vIndex++ )
                        {
                            //  Add indice to mesh indice pool
                            ASSERT(s_pMesh[i].NIndices < MAX_NUM_PRIMS);
                            pIndices[s_pMesh[i].NIndices] = pStrips[s].pVerts[vIndex];
                            
                            // Imcrement the number of indeces in the mesh as well as the local counter.
                            s_pMesh[i].NIndices++;
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
                            ASSERT(s_pMesh[i].NIndices + 2 < MAX_NUM_PRIMS);

                            pIndices[s_pMesh[i].NIndices] = pIndices[s_pMesh[i].NIndices-1];
                            s_pMesh[i].NIndices++;
                            
                            pIndices[s_pMesh[i].NIndices] = pStrips[s+1].pVerts[0];
                            s_pMesh[i].NIndices++;

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
                    s_pMesh[i].NSubMeshes++;
                }
                else
                {
                    s16* pIndices = (s16*)s_pMesh[i].pIndices;

                    //  Fill in submesh struct
                    s_pSubMesh[s_CurSubMesh].Flags          = 0;
                    s_pSubMesh[s_CurSubMesh].IndexStart     = s_pMesh[i].NIndices;
                    s_pSubMesh[s_CurSubMesh].IndexMin       = MAX_NUM_PRIMS;
                    s_pSubMesh[s_CurSubMesh].IndexCount     = 0;
                    s_pSubMesh[s_CurSubMesh].NTris          = TriCount;
                    s_pSubMesh[s_CurSubMesh].TextureID[0]   = s_TriData[NTotalTris].MaterialID;
                    s_pSubMesh[s_CurSubMesh].TextureID[1]   = -1;
                    s_pSubMesh[s_CurSubMesh].TextureID[2]   = -1;

                    s_pSubMesh[s_CurSubMesh].AvgPixelSize   = 0;
                    s_pSubMesh[s_CurSubMesh].MaxPixelSize   = 0;
                    s_pSubMesh[s_CurSubMesh].MinPixelSize   = 0;
                    s_pSubMesh[s_CurSubMesh].NVertsTrans    = 0;

                    s_pSubMesh[s_CurSubMesh].PrimType       = D3DPT_TRIANGLELIST;

                    NIndices                                = 0;
                    MaxIndice                               = 0;

                    // Go through each triangle that shares this material.
                    for ( t = 0; t < TriCount; t++ )
                    {
                        for ( vIndex = 0; vIndex < 3; vIndex++ )
                        {
                            ASSERT(s_pMesh[i].NIndices < MAX_NUM_PRIMS);

                            //  Add indice to mesh indice pool
                            pIndices[s_pMesh[i].NIndices] = s_TriData[NTotalTris+t].Index[vIndex];
                            s_pMesh[i].NIndices++;
                            NIndices++;

                            //  Find the minimum and maximum Vertex index used for this submesh.
                            s_pSubMesh[s_CurSubMesh].IndexMin = 
                                MIN(s_pSubMesh[s_CurSubMesh].IndexMin, s_TriData[NTotalTris+t].Index[vIndex] );

                            MaxIndice = 
                                MAX(MaxIndice, s_TriData[NTotalTris+t].Index[vIndex] );
                        }
                    }

                    //  Update the number of indeces used in this submesh.
                    s_pSubMesh[s_CurSubMesh].IndexCount = MaxIndice + 1 - s_pSubMesh[s_CurSubMesh].IndexMin;

                    //  Increment submesh count
                    s_CurSubMesh++;
                    s_pMesh[i].NSubMeshes++;
                }

                //  How many tris left to process
                NTotalTris += TriCount;
            }

            //  Free up the global tri data since it's been copied to the submeshes.
            delete[] s_TriData;
        }

        //  Display the number of Submeshes used.
        if ( g_Verbose )
            x_printf( "  NSubMeshes: %d\n", s_pMesh[i].NSubMeshes );

        //  Increase the mesh counts in the file header.
        hdr.NMeshes++;
        hdr.NSubMeshes += s_pMesh[i].NSubMeshes;
    }

    //-------------------------------------------------------------------------------------------------------------------------------------
    //  Write the output file.
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Write the file header.
    x_fwrite( &hdr, sizeof(t_GeomFileHeader), 1, outfile );

    // Write mesh struct with vert and indice pool
    for( m = 0; m < hdr.NMeshes; m++ )
    {
        x_fwrite( &s_pMesh[m], sizeof(t_GeomMesh), 1, outfile );    
        
        x_fwrite( s_pMesh[m].pVerts, s_pMesh[m].VertFormatSize, s_pMesh[m].NVerts, outfile );
        x_fwrite( s_pMesh[m].pIndices, sizeof(s16), s_pMesh[m].NIndices, outfile );

    }

    // Write out submeshes
    x_fwrite( s_pSubMesh, sizeof(t_GeomSubMesh), hdr.NSubMeshes, outfile );    

    // Write out the texture names
    for( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
    {
        char TextureName[TEXTURE_NAME_LENGTH];
        GetTextureName( TextureName, rGeom, i );
        PadStringWithZero( TextureName, TEXTURE_NAME_LENGTH );
        x_fwrite( TextureName, sizeof(char), TEXTURE_NAME_LENGTH, outfile );
    }

    // Write out the textures
    for( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
    {
        char NewDrive[5];
        char NewDir[255];
        char NewFileName[80];
        char NewExtension[10];
        u8   AlphaPercent = 255;;
        x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
        x_sprintf( NewFileName, "%s", x_strtoupper(NewFileName) );

        if( NewFileName[2] >= '0' && NewFileName[2] <= '9' )
        {
            AlphaPercent = (u8)(255 * ((NewFileName[2] - '0') * .1));
        }

        // now check to see if there is a 24-bit version of this texture so we can use that on the all mighty PC
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
            x_fprintf( g_TextureNameFile, "%s,GBL:0,CMP:%d,ALPHA:1,ALFNAME:%d,ALFGENC:%d,MIP:%d,CLRSCL:%d:255:255:255:%d\n",
                s_pMaterialSet->m_pTextures[i].m_Filename,
                !AlphaFlag && AlphaPercent == 255,
                !AlphaFlag,
                AlphaFlag,
                x_strncmp( "TEXTUREPAGE", NewFileName, x_strlen("TEXTUREPAGE") ) != 0 && g_UseMips, AlphaPercent != 255,
                AlphaPercent );
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

			    ret = AUXBMP_ConvertToPC( BMP, g_UseMips, FALSE, TRUE );

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

    //  Clean Up all of that allocated memory now.
    for ( m = 0; m < hdr.NMeshes; m++ )
    {
        x_free( s_pMesh[m].pVerts );
        x_free( s_pMesh[m].pIndices );
    }
    delete []s_pMesh;
    delete []s_pSubMesh;

}

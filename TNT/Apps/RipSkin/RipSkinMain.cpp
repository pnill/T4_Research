///////////////////////////////////////////////////////////////////////////
//  RipSkinMain.cpp
///////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <conio.h>
#include "x_files.hpp"
#include "x_stdio.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "x_math.hpp"

#include "mesh.h"
#include "RipMorph.h"
#include "ctextfile.h"
#include "skel.h"
#include "GeomMerge.h"

#include "PS2_RipSkin.h"
#include "GameCube_RipSkin.h"
#include "XBOX_RipSkin.h"
#include "PC_RipSkin.h"


////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////

#define MAX_INPUT_FILES     60

#define RIP_TARGET_UNKNOWN  0
#define RIP_TARGET_PS2      1
#define RIP_TARGET_GAMECUBE 2
#define RIP_TARGET_PC       3
#define RIP_TARGET_XBOX     4

#define EPSILON             0.05f

#define ENV_MAP_NAME        "EnvMap"


////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////

xbool                   g_Rotate180 = FALSE;
xbool                   g_Verbose = FALSE;
xbool                   g_ReportPathDetails = FALSE;
xbool                   g_ReportStripDetails = FALSE;
char                    g_StripFileName[255];
char                    g_TextureSeamFileName[255];
char                    g_BoneWeightingFileName[255];
static s32              s_NInputFiles = 0;
static char             s_InputFile[MAX_INPUT_FILES][255];
static s32              s_NTextureNames = 0;
static char             s_TextureName[MAX_INPUT_FILES][255];
static char             s_MeshName[MAX_INPUT_FILES][255];
static char             s_SourceFileName[255];
static char             s_SkinFileName[255];
static char             s_SkelFileName[255];
static char             s_BoneAssignmentFileName[255];
static s32              s_RipTarget = RIP_TARGET_UNKNOWN;

static QRipSkin*                    s_pRipSkin = NULL;
static mesh::object*                s_pGeom;
static QSkel                        s_Skel;
static QRipMorphTargetSet*          s_pMorphTargetSet = NULL;


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

static
void OutputUsageMessage( void )
{
    x_printf( "Usage: RipSkin [options] <inputs> <output.SKN>\n\n" );
    x_printf( "Version 1.3\n\n" );
    x_printf( "    The inputs are given in the form of a list of MTF files.\n\n" );
    x_printf( "    It is possible to specify a large number of MTF\n" );
    x_printf( "    files by passing @data.txt, where data.txt contains\n" );
    x_printf( "    a list of MTF files.\n\n" );
    x_printf( "Options:\n" );
    x_printf( "  -PS2 .............. Target PlayStation 2\n" );
    x_printf( "  -GC ............... Target GameCube\n" );
    x_printf( "  -PC ............... Target PC\n" );
    x_printf( "  -XBOX ............. Target XBOX\n" );
    x_printf( "  -ASF <file.asf> ... Load asf for bone re-assignment\n" );
    x_printf( "  -V ................ Verbose mode\n" );
    x_printf( "  -PD ............... Report path details for debugging\n" );
    x_printf( "  -SD ............... Report strip details for debugging\n" );
    x_printf( "  -BA <file.asc> .... Save ASC file showing bone assignments\n" );
    x_printf( "  -BW <file.asc> .... Save ASC file showing bone weighting\n" );
    x_printf( "  -SA <file.asc> .... Save ASC file showing strips\n" );
    x_printf( "  -TA <file.asc> .... Save ASC file showing texture seams\n" );
    x_printf( "  -R180 ............. Rotate mesh 180 degrees on Y before ripping\n" );
    x_printf( "  -TN <tex> <mesh> .. Specify that polys using the texture <tex>\n" );
    x_printf( "                      should get their own mesh. The mesh name\n" );
    x_printf( "                      will become <mesh>\n" );
}

//==========================================================================

void PauseOnError( void )
{
    x_printf( "Press any key to continue...\n" );

    while ( !getch() )
    {
        ;
    }
}

//==========================================================================

static
void StripFileExtension( char* filename )
{
    s32 length = x_strlen( filename );
    s32 i;

    ASSERT( filename );

    for ( i = length - 1; i >= 0; i-- )
    {
        if ( filename[i] == '\\' || filename[i] == '/' )
            break;
        if ( filename[i] == '.' )
        {
            filename[i] = '\0';
            return;
        }
    }
}

//==========================================================================

void ExportBoneAssignments( char* filename )
{
    //---   Save out .ASC file showing bone assignments from original .mtf geometry
    s32     C, F;
    s32     j,k;
    char    Mesh[32];
    f32     Weight[64];    
    X_FILE* fh;

    //---   print out some info
    if ( g_Verbose )
    {
        x_printf( "Exporting bone assignments...\n" );
    }

    //---   open the file
    fh = x_fopen( filename, "wt" );
    if ( !fh )
    {
        x_printf( "ERROR: Could not open %s for writing.\n", filename );
        PauseOnError();
        exit( 0 );
    }

    //---   loop through the chunks
    for( C = 0; C < s_pGeom->m_nChunks; C++ )
    {
        for( F = 0; F < s_pGeom->m_pChunk[C].m_nFaces; F++ )
        {
            //---   clear out the weight array
            for( j = 0; j < 64; j++ )
                Weight[j] = 0.0f;

            //---   loop through the verts, tallying up the weights as we go
            for( j = 0; j < 3; j++ )
            {   
                s32 NWeights;
                s32 VertID;
                VertID = s_pGeom->m_pChunk[C].m_pFace[F].m_Index[j];
                NWeights = s_pGeom->m_pChunk[C].m_pVertex[VertID].m_nWeights;
                
                //---   loop through each matrix that the vert uses
                for( k = 0; k < NWeights; k++ )
                {
                    s32 MatrixID = s_pGeom->m_pChunk[C].m_pVertex[VertID].m_Weight[k].m_iMatrix;
                    Weight[MatrixID] +=
                        s_pGeom->m_pChunk[C].m_pVertex[ VertID ].m_Weight[k].m_Weight;
                }
            }

            //---   now loop through the weights, and if this face uses enough of the weight,
            //      add it to a mesh
            for( j = 0; j < 64; j++ )
            {
                if( Weight[j] > EPSILON )
                {
                    mesh::vertex* pV;
                    s32 VertID;

                    //---   this face belongs to the j weight mesh
                    x_sprintf( Mesh, "%1c%1s%1c", '"', fs( "MESH%03d", j ), '"' ); 

                    //---   export each of the verts to the ASC file
                    VertID = s_pGeom->m_pChunk[C].m_pFace[F].m_Index[0];
                    pV = &s_pGeom->m_pChunk[C].m_pVertex[VertID];
                    x_fprintf( fh, "[%f, %f, %f, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, %1s,\n",
                        pV->m_vPos.X, pV->m_vPos.Y, pV->m_vPos.Z,
                        Mesh );

                    VertID = s_pGeom->m_pChunk[C].m_pFace[F].m_Index[1];
                    pV = &s_pGeom->m_pChunk[C].m_pVertex[VertID];
                    x_fprintf( fh, "%f, %f, %f, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, %1c%1s%1c,\n",
                        pV->m_vPos.X, pV->m_vPos.Y, pV->m_vPos.Z,
                        '"', "MAT0", '"' );

                    VertID = s_pGeom->m_pChunk[C].m_pFace[F].m_Index[2];
                    pV = &s_pGeom->m_pChunk[C].m_pVertex[VertID];
                    x_fprintf( fh, "%f, %f, %f, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0 ]\n",
                        pV->m_vPos.X, pV->m_vPos.Y, pV->m_vPos.Z
                        );
                }
            }
        }
    }
    //---   now, export a dummy material
    x_fprintf( fh, "(   64,   64,    0.50,     0.50,       0,       0,   0.50,     0.50,      0   ,  %1c%1s%1c,  %1c%1s%1c)\n", '"', "MAT0", '"', '"', "DUMMY.GIF", '"' );

    //---   close the file
    x_fclose( fh );
}

//==========================================================================

void RecurseBuildParentList( s32& rBone, s32* ParentList, s32 Parent )
{
    s32 i;
    s32 NewParent;

    if ( rBone >= s_pGeom->m_nBones )
        return;

    ParentList[rBone] = Parent;

    NewParent = rBone;
    for ( i = 0; i < s_pGeom->m_pBone[NewParent].m_nChildren; i++ )
    {
        rBone++;
        RecurseBuildParentList( rBone, ParentList, NewParent );
    }
}

//==========================================================================

s32 FindParentBone( s32 MtfBoneID )
{
    s32 CurrBone = 0;
    s32 Parents[100];

    RecurseBuildParentList( CurrBone, Parents, 0 );

    return Parents[MtfBoneID];
}

//==========================================================================

s32 FindASFBoneID( s32 MtfBoneID )
{
    s32     j;
    s32     ASFBoneID;

    //---   Given a bone name find the .ASF bone equivalent
    ASFBoneID = -1;

    while( ASFBoneID == -1 )
    {
        //---   Search for bone in asf bones
        for( j = 0; j < s_Skel.m_NBones; j++ )
        {
            if( x_stricmp( s_pGeom->m_pBone[MtfBoneID].m_Name, s_Skel.m_pBone[j].BoneName ) == 0 )
            {
                ASFBoneID = j;
                break;
            }
        }

        //---   if no bone found try the parent bone
        if( j == s_Skel.m_NBones )
        {
            //---   find the parent...the mtf bone layout isn't very nice, but we can
            //      still find the parent if we try...
            MtfBoneID = FindParentBone( MtfBoneID );
        }
    }

    return ASFBoneID;
}

//=========================================================================

void LoadSkel( char* FileName )
{
    //---   load the skeleton
    if ( !s_Skel.ImportASF( FileName ) )
    {
        x_printf( "ERROR: Could not load %s.\n", FileName );
        PauseOnError();
        exit(0);
    }
    s_Skel.DelDummyBones();
}

//=========================================================================

void ShrinkMatrixSet( t_RipVert& rRipVert )
{
    s32 i,j;
    s32 MatrixID[8];
    f32 MatrixWeight[8];
    s32 NMatrices;
    s32 MID;
    f32 MW;

    //---   Merge all matrices with same index into a single list
    //      AND sum up weights of duplicates
    NMatrices = 0;
    for( i = 0; i < rRipVert.NMatrices; i++ )
    {
        MID = rRipVert.MatrixID[i];
        MW  = rRipVert.MatrixWeight[i];

        //---   Search if already in list
        for( j = 0; j < NMatrices; j++ )
        {
            if( MatrixID[j] == MID )
                break;
        }

        //---   Add new matrix
        if( j == NMatrices )
        {
            MatrixID[NMatrices] = MID;
            MatrixWeight[NMatrices] = 0;
            NMatrices++;
        }

        MatrixWeight[j] += MW;
    }

    //---   Renormalize matrices
    MW = 0.0f;
    for( i = 0; i < NMatrices; i++ )
        MW += MatrixWeight[i];

    MW = 1.0f / MW;
    for( i = 0; i < NMatrices; i++ )
        MatrixWeight[i] *= MW;

    //---   Copy back into vert
    for( i = 0; i < 8; i++ )
    {
        if( i < NMatrices )
        {
            rRipVert.MatrixID[i] = MatrixID[i];
            rRipVert.MatrixWeight[i] = MatrixWeight[i];
        }
        else
        {
            rRipVert.MatrixID[i] = -1;
            rRipVert.MatrixWeight[i] = 0.0f;
        }
    }
    rRipVert.NMatrices = NMatrices;
}

//=========================================================================

void RemapVertWeights( t_RipVert& rRipVert )
{
    s32     i;
    s32     ExportBoneID;
    s32     ASFBoneID;

    //---   Remap matrix id's from .mtf skel to .asf skel
    for( i = 0; i < rRipVert.NMatrices; i++ )
    {
        ExportBoneID = rRipVert.MatrixID[i];
        ASFBoneID = FindASFBoneID( ExportBoneID );
        
        if( ASFBoneID == -1 ) 
            ASFBoneID = 0;
        
        rRipVert.MatrixID[i] = ASFBoneID;
    }

    ShrinkMatrixSet( rRipVert );
}

//==========================================================================

static
xbool IsEnvironmentMapped( char* MeshName )
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
xbool IsShadowMesh( char* MeshName )
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
xbool IsAlphaMesh( char* MeshName )
{
    s32 i;
    i = x_strlen( MeshName ) - x_strlen( "_alpha" );
    if ( i < 0 )
        return FALSE;

    if ( !x_stricmp( &MeshName[i], "_alpha" ) )
        return TRUE;

    return FALSE;
}

//=========================================================================

static
void AddVertsToRipSkin( s32 ChunkID )
{
    s32 i, j;

    for( i = 0; i < s_pGeom->m_pChunk[ChunkID].m_nVerts; i++ )
    {
        mesh::vertex*   pVert;
        t_RipVert       RipVert;

        //---   get a ptr to the vert we're working with
        pVert = &s_pGeom->m_pChunk[ChunkID].m_pVertex[i];
        
        //---   Copy over Pos
        RipVert.Pos = pVert->m_vPos;
        RipVert.Normal = pVert->m_vNormal;

        //---   Copy over color, note that at run-time we use
        //      only the normal, so this isn't so important.
        RipVert.Color = color(255,255,255,255);

        //---   by default, this vert doesn't morph
        RipVert.HasMorphDelta = FALSE;

        //---   Search for a duplicate position and change color
        for( j = 0; j < s_pGeom->m_pChunk[ChunkID].m_nVerts; j++ )
        {
            if( i != j )
            {
                mesh::vertex*   pV;
                pV = &s_pGeom->m_pChunk[ChunkID].m_pVertex[j];
    
                if( (x_abs( pV->m_vPos.X - RipVert.Pos.X ) < 0.01f) &&
                    (x_abs( pV->m_vPos.Y - RipVert.Pos.Y ) < 0.01f) &&
                    (x_abs( pV->m_vPos.Z - RipVert.Pos.Z ) < 0.01f) )
                {
                    RipVert.Color.Set( 0, 0, 0, 255 );
                    break;
                }
            }
        }

        //---   Copy over UV
        if ( s_pGeom->m_nTextures == 0 )
            RipVert.UV.Set( 0.0f, 0.0f );
        else
            RipVert.UV = pVert->m_UV[0];

        //---   Copy over matrix ids and weights
        RipVert.NMatrices = pVert->m_nWeights;
        RipVert.TransformID = -1;
        for( j = 0; j < 8; j++ )
        {
            if( j >= RipVert.NMatrices )
            {
                RipVert.MatrixID[j] = UNUSED_MATRIX_ID;
                RipVert.MatrixWeight[j] = 0.0f;
            }
            else
            {
                RipVert.MatrixID[j] = pVert->m_Weight[j].m_iMatrix;
                RipVert.MatrixWeight[j] = pVert->m_Weight[j].m_Weight;
            }
        }

        //---   If skeleton is present map weights
        if( s_SkelFileName[0] )
            RemapVertWeights( RipVert );

        //---   Remove duplicate matrices
        ShrinkMatrixSet( RipVert );

        //---   Add the vert
        s_pRipSkin->AddVert( RipVert );
    }
}

//==========================================================================

static
void AddTrisToRipSkin( s32 ChunkID, xbool UseEnvironmentMap )
{
    s32 i;

    for( i = 0; i < s_pGeom->m_pChunk[ChunkID].m_nFaces; i++ )
    {
        s32             TextureID;
        mesh::face*     pFace;

        pFace = &s_pGeom->m_pChunk[ChunkID].m_pFace[i];
        ASSERT( !pFace->m_bQuad );

        //---   figure out which texture this tri uses
        if ( pFace->m_iMaterial == -1 )
        {
            TextureID = 0;
        }
        else
        {
            s32 MaterialID;
            s32 SubMaterialID;

            // #### THIS SHOULD BE CHANGED TO SUPPORT MULTI-PASS RENDERING
            MaterialID = pFace->m_iMaterial;
            ASSERT( MaterialID < s_pGeom->m_nMaterials );
            SubMaterialID = s_pGeom->m_pMaterial[MaterialID].m_iSubMat[0];
            ASSERT( SubMaterialID < s_pGeom->m_nSubMaterials );
            TextureID = s_pGeom->m_pSubMaterial[SubMaterialID].m_iTexture;
            ASSERT( TextureID < s_pGeom->m_nTextures );
        }

        //---   are we using an environment map?
        if ( UseEnvironmentMap )
        {
            char    EnvMapName[32];

            x_sprintf( EnvMapName, "%s_%d", ENV_MAP_NAME, TextureID );
            //---   add the tri
            s_pRipSkin->AddTri( pFace->m_Index[0],
                              pFace->m_Index[1],
                              pFace->m_Index[2],
                              EnvMapName );
        }
        else
        {
            //---   add the tri
            s_pRipSkin->AddTri( pFace->m_Index[0],
                              pFace->m_Index[1],
                              pFace->m_Index[2],
                              s_pGeom->m_pTexture[TextureID].m_FileName );
        }
    }
}

//==========================================================================

static
void AddMorphTargetsToRipSkin( s32 ChunkID )
{
    s32 i, j;

    if ( !s_pMorphTargetSet )
        return;

    for ( i = 0; i < s_pMorphTargetSet->m_NTargets; i++ )
    {
        if ( !x_stricmp( s_pMorphTargetSet->m_pTargets[i].m_ChunkName, s_pGeom->m_pChunk[ChunkID].m_Name ) )
        {
            //---   this morph target belongs to this chunk
            s_pRipSkin->NewMorphTarget( s_pMorphTargetSet->m_pTargets[i].m_Name );

            for ( j = 0; j < s_pMorphTargetSet->m_pTargets[i].m_NDeltas; j++ )
            {
                s_pRipSkin->AddMorphDelta( s_pMorphTargetSet->m_pTargets[i].m_pDeltas[j].m_VertID,
                                           s_pMorphTargetSet->m_pTargets[i].m_pDeltas[j].m_Delta );
            }
        }
    }
}

//==========================================================================

/*
void ApplyMorphTarget( void )
{
    s32 MeshID;
    s32 DeltaID;

    for ( MeshID = 0; MeshID < s_pGeom->m_nChunks; MeshID++ )
    {
        if ( !x_stricmp( s_pGeom->m_pChunk[MeshID].m_Name, s_pMorphTargetSet->m_pTarget[0].m_ChunkName ) )
            break;
    }
    ASSERT( MeshID < s_pGeom->m_nChunks );

    for ( DeltaID = 0; DeltaID < s_pMorphTargetSet->m_pTarget[0].m_nDeltas; DeltaID++ )
    {
        s_pGeom->m_pChunk[MeshID].m_pVertex[s_pMorphTargetSet->m_pTarget[0].m_pDelta[DeltaID].m_iVert].m_vPos +=
            s_pMorphTargetSet->m_pTarget[0].m_pDelta[DeltaID].m_vDelta;
    }
}
*/

//==========================================================================

void RipGeometry( char* FileName )
{
    s32     i;

    //---   Create Rip Skin object
    switch (s_RipTarget)
    {
    case RIP_TARGET_PS2:
        s_pRipSkin = (QRipSkin*)new QPS2RipSkin;
        break;
    case RIP_TARGET_GAMECUBE:
        s_pRipSkin = (QRipSkin*)new QGameCubeRipSkin;
        break;
    case RIP_TARGET_PC:
        s_pRipSkin = (QRipSkin*)new QPCRipSkin;
        break;
    case RIP_TARGET_XBOX:
        s_pRipSkin = (QRipSkin*)new QXBOXRipSkin;
        break;
    default:
        ASSERTS(0, "Undefined Target");
    }

    ASSERT( s_pRipSkin != NULL );

    //---   Report name to QRipSkin
    s_pRipSkin->SetSkinName( s_pGeom->m_Name );

    //---   Add the bones to QRipSkin
    if ( s_SkelFileName[0] )
    {
        for ( i = 0; i < s_Skel.m_NBones; i++ )
        {
            s_pRipSkin->AddBone( s_Skel.m_pBone[i].BoneName );
        }
    }
    else
    {
        for ( i = 0; i < s_pGeom->m_nBones; i++ )
        {
            s_pRipSkin->AddBone( s_pGeom->m_pBone[i].m_Name );
        }
    }

    //---   Report textures to QRipSkin
    for( i = 0; i < s_pGeom->m_nTextures; i++ )
    {
        s_pRipSkin->NewTexture( s_pGeom->m_pTexture[i].m_FileName,
                              s_pGeom->m_pTexture[i].m_Width,
                              s_pGeom->m_pTexture[i].m_Height );
    }

    //---   If any of the chunks requires an environment map,
    //      add a texture to the list for that
    for ( i = 0; i < s_pGeom->m_nChunks; i++ )
    {
        if ( IsEnvironmentMapped( s_pGeom->m_pChunk[i].m_Name ) )
        {
            s_pRipSkin->NewTexture( ENV_MAP_NAME, 0, 0 );
            break;
        }
    }

    //ApplyMorphTarget();  // ####

    //---   Loop through chunks and report meshes to QRipSkin
    for ( i = 0; i < s_pGeom->m_nChunks; i++ )
    {
        //---   Create a new mesh
        s_pRipSkin->NewMesh( s_pGeom->m_pChunk[i].m_Name,
                             IsEnvironmentMapped( s_pGeom->m_pChunk[i].m_Name ),
                             IsShadowMesh( s_pGeom->m_pChunk[i].m_Name ),
                             IsAlphaMesh( s_pGeom->m_pChunk[i].m_Name ) );

        //---   Add verts to mesh
        AddVertsToRipSkin( i );

        //---   Add tris to mesh
        AddTrisToRipSkin( i, FALSE );

        //---   Add morph targets to mesh
        AddMorphTargetsToRipSkin( i );
    }

    //---   Decide how many matrices per vertex
//    s_pRipSkin->ForceSingleMatrix();
//    s_pRipSkin->ForceDoubleMatrix();
    s_pRipSkin->ForceTripleMatrix();

    //---   Build structures
    s_pRipSkin->PrepareStructures();

    //---   Write out some stats
    if ( g_Verbose )
    {
        s_pRipSkin->DisplayStats();
    }

    ////////////////////////////////////////////////////////////////////////
    //---   Save ripped geometry
    ////////////////////////////////////////////////////////////////////////

    s_pRipSkin->Save( s_SkinFileName );

    ////////////////////////////////////////////////////////////////////////
    //---   Delete rip skin class
    ////////////////////////////////////////////////////////////////////////

    delete s_pRipSkin;
}

//=========================================================================

static
xbool IsInputFile( char* filename )
{
    s32 i;

    i = x_strlen( filename );
    i -= x_strlen( ".MTF" );

    if ( !x_stricmp( &filename[i], ".MTF" ) )
        return TRUE;

    if ( !x_stricmp( &filename[i], ".ATF" ) )
        return TRUE;

    return FALSE;
}

//==========================================================================

static
xbool IsInputResponseFile( char* pStr )
{
    if ( pStr[0] == '@' )
        return TRUE;
    else
        return FALSE;
}

//==========================================================================

static
xbool GetLine( X_FILE* fh, char* Dest )
{
    s32     index = 0;
    char    C;

    while ( !x_feof( fh ) )
    {
        C = (char)x_fgetc( fh );
        if ( C == '\n' )
            break;
        Dest[index++] = C;
    }
    Dest[index] = '\0';

    if ( x_feof( fh ) )
        return FALSE;
    
    return TRUE;
}

//==========================================================================

void ProcessSwitches( int argc, char** argv )
{
    ////////////////////////////////////////////////////////////////////////
    //---   skip past the executable name
    ////////////////////////////////////////////////////////////////////////

    argc--;
    argv++;

    ////////////////////////////////////////////////////////////////////////
    //---   process all of the options
    ////////////////////////////////////////////////////////////////////////

    while ( argc )
    {
        char*   pArg;

        ////////////////////////////////////////////////////////////////////
        //---   get the argument
        ////////////////////////////////////////////////////////////////////

        pArg = argv[0];

        ////////////////////////////////////////////////////////////////////
        //---   check for a valid target
        ////////////////////////////////////////////////////////////////////

        if ( !x_stricmp( pArg, "-PS2" ) )
        {
            s_RipTarget = RIP_TARGET_PS2;
        }
        else
        if ( !x_stricmp( pArg, "-GC" ) )
        {
            s_RipTarget = RIP_TARGET_GAMECUBE;
        }
        else
        if ( !x_stricmp( pArg, "-PC" ) )
        {
            s_RipTarget = RIP_TARGET_PC;
        }
        else
        if ( !x_stricmp( pArg, "-XBOX" ) )
        {
            s_RipTarget = RIP_TARGET_XBOX;
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for a skeleton file
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-ASF" ) )
        {
            argc--;
            argv++;
            if ( !argc )
            {
                x_printf( "ERROR: Missing skeleton file name.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
            x_strcpy( s_SkelFileName, argv[0] );
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for a bone assigments file
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-BA" ) )
        {
            argc--;
            argv++;
            if ( !argc )
            {
                x_printf( "ERROR: Missing bone assignment file name.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
            x_strcpy( s_BoneAssignmentFileName, argv[0] );
            StripFileExtension( s_BoneAssignmentFileName );
            x_strcat( s_BoneAssignmentFileName, ".ASC" );
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for a bone weighting file
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-BW" ) )
        {
            argc--;
            argv++;
            if ( !argc )
            {
                x_printf( "ERROR: Missing bone weighting file name.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
            x_strcpy( g_BoneWeightingFileName, argv[0] );
            StripFileExtension( g_BoneWeightingFileName );
            x_strcat( g_BoneWeightingFileName, ".ASC" );
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for a strip .asc file
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-SA" ) )
        {
            argc--;
            argv++;
            if ( !argc )
            {
                x_printf( "ERROR: Missing strip ASC file name.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
            x_strcpy( g_StripFileName, argv[0] );
            StripFileExtension( g_StripFileName );
            x_strcat( g_StripFileName, ".ASC" );
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for texture seam .asc file
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-TA" ) )
        {
            argc--;
            argv++;
            if ( !argc )
            {
                x_printf( "ERROR: Missing texture seam ASC file name.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
            x_strcpy( g_TextureSeamFileName, argv[0] );
            StripFileExtension( g_TextureSeamFileName);
            x_strcat( g_TextureSeamFileName, ".ASC" );
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for creating a new mesh based on texture name
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-TN" ) )
        {
            argc--;
            argv++;
            if ( !argc )
            {
                x_printf( "ERROR: Missing texture file name after -TN switch.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
            x_strcpy( s_TextureName[s_NTextureNames], argv[0] );

            argc--;
            argv++;
            if ( !argc )
            {
                x_printf( "ERROR: Missing mesh name after -TN switch.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
            x_strcpy( s_MeshName[s_NTextureNames], argv[0] );
            
            s_NTextureNames++;
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for verbose mode
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-V" ) )
        {
            g_Verbose = TRUE;
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for reporting path details
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-PD" ) )
        {
            g_ReportPathDetails = TRUE;
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for reporting strip details
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-SD" ) )
        {
            g_ReportStripDetails = TRUE;
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for rotating 180 degrees on the y
        ////////////////////////////////////////////////////////////////////

        else
        if ( !x_stricmp( pArg, "-R180" ) )
        {
            g_Rotate180 = TRUE;
        }

        ////////////////////////////////////////////////////////////////////
        //---   check for an unknown option
        ////////////////////////////////////////////////////////////////////

        else
        if ( pArg[0] == '-' )
        {
            x_printf( "ERROR: Unkown option: %s\n", pArg );
            OutputUsageMessage();
            PauseOnError();
            exit( 0 );
        }
        else
        {
            //---   handle input/output names
            if ( IsInputFile( pArg ) )
            {
                //---   must be an input file
                x_strcpy( s_InputFile[s_NInputFiles], pArg );
                //StripFileExtension( s_InputFile[s_NInputFiles] );
                //x_strcat( s_InputFile[s_NInputFiles], ".MTF" );
                s_NInputFiles++;
            }
            else
            if ( IsInputResponseFile( pArg ) )
            {
                X_FILE* ResponseFile;
                char    temp[256];

                //---   must be a text file with a list of inputs
                x_strcpy( temp, &pArg[1] );
                ResponseFile = x_fopen( temp, "r" );
                if ( !ResponseFile )
                {
                    x_printf( "ERROR: Unable to open %s for reading.\n", temp );
                    OutputUsageMessage();
                    PauseOnError();
                    exit( 0 );
                }

                //---   add all of the inputs from the response file
                while ( GetLine( ResponseFile, temp ) )
                {
                    x_strcpy( s_InputFile[s_NInputFiles], temp );
                    StripFileExtension( s_InputFile[s_NInputFiles] );
                    x_strcat( s_InputFile[s_NInputFiles], ".MTF" );
                    s_NInputFiles++;
                }

                x_fclose( ResponseFile );
            }
            else
            if ( s_SkinFileName[0] == '\0' )
            {
                ////////////////////////////////////////////////////////////
                //---   must be the output file
                ////////////////////////////////////////////////////////////

                x_strcpy( s_SkinFileName, pArg );
                StripFileExtension( s_SkinFileName );
                x_strcat( s_SkinFileName, ".SKN" );
            }
            else
            {
                ////////////////////////////////////////////////////////////
                //---   we have an error
                ////////////////////////////////////////////////////////////

                x_printf( "ERROR: Too many options.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
        }

        ////////////////////////////////////////////////////////////////////
        //---   advance to the next argument
        ////////////////////////////////////////////////////////////////////

        argc--;
        argv++;
    }

    ////////////////////////////////////////////////////////////////////////
    //---   verify a proper target
    ////////////////////////////////////////////////////////////////////////

    if ( s_RipTarget == RIP_TARGET_UNKNOWN )
    {
        x_printf( "ERROR: Unknown target.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }

    ////////////////////////////////////////////////////////////////////////
    //---   verify a proper source file
    ////////////////////////////////////////////////////////////////////////

    if ( s_NInputFiles == 0 )
    {
        x_printf( "ERROR: Source file missing (mtf).\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }

    ////////////////////////////////////////////////////////////////////////
    //---   verify a proper destination file
    ////////////////////////////////////////////////////////////////////////

    if ( s_SkinFileName[0] == '\0' )
    {
        x_printf( "ERROR: Dest file name missing (skn).\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }
}

//==========================================================================

static
s32 FindTextureIndex( mesh::object* pObject, char* texture )
{
    s32 i;

    char Drive[X_MAX_DRIVE];
    char Dir[X_MAX_DIR];
    char FName[X_MAX_FNAME];
    char Ext[X_MAX_EXT];

    for ( i = 0; i < pObject->m_nTextures; i++ )
    {
        x_splitpath( pObject->m_pTexture[i].m_FileName, Drive, Dir, FName, Ext );
        if ( !x_stricmp( FName, texture ) )
        {
            return i;
        }
    }
    return -1;
}

//==========================================================================

static
s32 CountFacesUsingTexture( mesh::object* pObject, s32 TexID )
{
    s32 count;
    s32 ChunkID;
    s32 MatID;
    s32 SubMatID;
    s32 FaceID;
    s32 ChunkCount;

    count = 0;
    for ( ChunkID = 0; ChunkID < pObject->m_nChunks; ChunkID++ )
    {
		ChunkCount = 0;

        for ( FaceID = 0; FaceID < pObject->m_pChunk[ChunkID].m_nFaces; FaceID++ )
        {
            MatID = pObject->m_pChunk[ChunkID].m_pFace[FaceID].m_iMaterial;
            SubMatID = pObject->m_pMaterial[MatID].m_iSubMat[0];
            if ( TexID == pObject->m_pSubMaterial[SubMatID].m_iTexture )
            {   count++;
				ChunkCount++;
			}
        }
		
		// Check if all faces use texture, so that the entire mesh isn't stripped out
		if( ChunkCount == pObject->m_pChunk[ChunkID].m_nFaces )
			count -= ChunkCount;
    }
    return count;
}

//==========================================================================

static
s32 CountValidFaces( mesh::object* pGeom, s32 ChunkID )
{
    s32 i;
    s32 count = 0;

    for ( i = 0; i < pGeom->m_pChunk[ChunkID].m_nFaces; i++ )
    {
        if ( pGeom->m_pChunk[ChunkID].m_pFace[i].m_Index[0] != -1 )
            count++;
    }

    return count;
}

//==========================================================================

static
void PullOutTextureMeshes( void )
{
    s32 i;
    s32 TextureIndex;

    for ( i = 0; i < s_NTextureNames; i++ )
    {
        s32             NFaces;
        mesh::object*   pNewObject;
        QGeomMerge      GeomMerge;

        //---   find the texture index we are mucking around with
        TextureIndex = FindTextureIndex( s_pGeom, s_TextureName[i] );
        if ( TextureIndex < 0 )
            continue;
        
        //---   create a new mesh to hold the pulled-out tris
        pNewObject = new mesh::object();

        //===   copy the materials, sub-materials, and textures from the original geom
        //      NOTE: We can safely copy all materials, sub-materials, and textures
        //      because we know that QGeomMerge will consolidate them later!

        //---   copy the materials
        pNewObject->m_nMaterials = s_pGeom->m_nMaterials;
        if ( pNewObject->m_nMaterials )
        {
            s32 MatID;
            
            pNewObject->m_pMaterial = new mesh::material[pNewObject->m_nMaterials];
            
            for ( MatID = 0; MatID < pNewObject->m_nMaterials; MatID++ )
            {
                x_memcpy( &pNewObject->m_pMaterial[MatID],
                          &s_pGeom->m_pMaterial[MatID],
                          sizeof(mesh::material) );
            }
        }

        //---   copy the sub-materials
        pNewObject->m_nSubMaterials = s_pGeom->m_nSubMaterials;
        if ( pNewObject->m_nSubMaterials )
        {
            s32 SubMatID;

            pNewObject->m_pSubMaterial = new mesh::sub_material[pNewObject->m_nSubMaterials];

            for ( SubMatID = 0; SubMatID < pNewObject->m_nSubMaterials; SubMatID++ )
            {
                x_memcpy( &pNewObject->m_pSubMaterial[SubMatID],
                          &s_pGeom->m_pSubMaterial[SubMatID],
                          sizeof(mesh::sub_material) );
            }
        }

        //---   copy the textures
        pNewObject->m_nTextures = s_pGeom->m_nTextures;
        if ( pNewObject->m_nTextures )
        {
            s32 TexID;

            pNewObject->m_pTexture = new mesh::texture[pNewObject->m_nTextures];
            for ( TexID = 0; TexID < pNewObject->m_nTextures; TexID++ )
            {
                x_memcpy( &pNewObject->m_pTexture[TexID],
                          &s_pGeom->m_pTexture[TexID],
                          sizeof(mesh::texture) );
            }
        }

        //===   copy the name from the original geometry
        x_strcpy( pNewObject->m_Name, s_pGeom->m_Name );

        //===   copy the bones from the original geometry
        pNewObject->m_nBones = s_pGeom->m_nBones;
        if ( pNewObject->m_nBones )
        {
            s32 BoneID;

            pNewObject->m_pBone = new mesh::bone[pNewObject->m_nBones];
            for ( BoneID = 0; BoneID < pNewObject->m_nBones; BoneID++ )
            {
                x_memcpy( &pNewObject->m_pBone[BoneID],
                          &s_pGeom->m_pBone[BoneID],
                          sizeof(mesh::bone) );
            }
        }

        //===   now, build up the new mesh. pay careful attention here, its kinda' weird

        //---   allocate space for the new mesh
        pNewObject->m_nChunks = 1;
        pNewObject->m_pChunk = new mesh::chunk[1];
        x_strcpy( pNewObject->m_pChunk[0].m_Name, s_MeshName[i] );

        //---   count the number of faces that use this texture
        NFaces = CountFacesUsingTexture( s_pGeom, TextureIndex );
        
        //---   allocate space for the faces, and copy the verts into a temporary space,
        //      this is the tricky part...
        pNewObject->m_pChunk[0].m_nFaces = NFaces;
        if ( NFaces )
        {
            s32         NFacesAdded;
            s32         ChunkFaceCount;
            s32         ChunkID;
            s32         FaceID;
            s32         MatID;
            s32         SubMatID;
            s32         NTempVertIDs;
            s32*        TempVertIDs;
            s32*        TempVertChunkIDs;

            pNewObject->m_pChunk[0].m_pFace = new mesh::face[NFaces];
            ASSERT( pNewObject->m_pChunk[0].m_pFace );
            
            //---   allocate temporary storage to put the vert ids into
            NTempVertIDs = 0;
            TempVertIDs = (s32*)x_malloc( 10 * 1024 * sizeof(s32) );
            ASSERT( TempVertIDs );
            TempVertChunkIDs = (s32*)x_malloc( 10 * 1024 * sizeof(s32) );
            ASSERT( TempVertChunkIDs );

            //---   find faces that use this texture, and copy the vert indices,
            //      then mark the face as invalid by setting the indices to -1
            //      Invalid faces will later be removed
            NFacesAdded = 0;
            for ( ChunkID = 0; ChunkID < s_pGeom->m_nChunks; ChunkID++ )
            {
                // ADDITION: Check if all faces in chunk use same texture, so as not
                //  to rip out all the faces causing an empty mesh.
				ChunkFaceCount = 0;
				for( FaceID = 0; FaceID < s_pGeom->m_pChunk[ChunkID].m_nFaces; FaceID++ )
				{
					MatID = s_pGeom->m_pChunk[ChunkID].m_pFace[FaceID].m_iMaterial;
					SubMatID = s_pGeom->m_pMaterial[MatID].m_iSubMat[0];
					if( s_pGeom->m_pSubMaterial[SubMatID].m_iTexture == TextureIndex )
						ChunkFaceCount++;
				}
				if( ChunkFaceCount == s_pGeom->m_pChunk[ChunkID].m_nFaces )
					continue;
				// END ADDITION

                for ( FaceID = 0; FaceID < s_pGeom->m_pChunk[ChunkID].m_nFaces; FaceID++ )
                {
                    MatID = s_pGeom->m_pChunk[ChunkID].m_pFace[FaceID].m_iMaterial;
                    SubMatID = s_pGeom->m_pMaterial[MatID].m_iSubMat[0];
                    if ( s_pGeom->m_pSubMaterial[SubMatID].m_iTexture == TextureIndex )
                    {
                        s32 N, V;
                        s32 Index;
                        s32 NIndices;

                        //---   add this face to our new mesh
                        pNewObject->m_pChunk[0].m_pFace[NFacesAdded].m_bQuad =
                            s_pGeom->m_pChunk[ChunkID].m_pFace[FaceID].m_bQuad;
                        pNewObject->m_pChunk[0].m_pFace[NFacesAdded].m_iMaterial =
                            s_pGeom->m_pChunk[ChunkID].m_pFace[FaceID].m_iMaterial;

                        //---   how many indices should we copy over?
                        if ( s_pGeom->m_pChunk[ChunkID].m_pFace[FaceID].m_bQuad )
                            NIndices = 4;
                        else
                            NIndices = 3;

                        //---   copy each index, adding vert IDs into our list as appropriate
                        for ( N = 0; N < NIndices; N++ )
                        {
                            Index = s_pGeom->m_pChunk[ChunkID].m_pFace[FaceID].m_Index[N];
                            ASSERT( Index > 0 );

                            //---   search for the face indices in our list
                            for ( V = 0; V < NTempVertIDs; V++ )
                            {
                                if ( (TempVertIDs[V] == Index) && (TempVertChunkIDs[V] == ChunkID) )
                                    break;
                            }
                            if ( V == NTempVertIDs )
                            {
                                //---   the index is not in the list
                                TempVertIDs[V] = Index;
                                TempVertChunkIDs[V] = ChunkID;
                                NTempVertIDs++;
                            }

                            //---   'V' now contains where the new vert will be copied to
                            pNewObject->m_pChunk[0].m_pFace[NFacesAdded].m_Index[N] = V;

                            //---   mark the other face as invalid, so it can be removed later
                            s_pGeom->m_pChunk[ChunkID].m_pFace[FaceID].m_Index[N] = -1;
                        }

                        //---   we have just copied a face over...
                        NFacesAdded++;
                    }
                }
            }

            ASSERT( NFacesAdded == NFaces );

            //---   at this point all of the faces using this texture have been moved over,
            //      now copy the verts over to our new mesh...
            pNewObject->m_pChunk[0].m_nVerts = NTempVertIDs;
            if ( NTempVertIDs )
            {
                s32  TempVertID;
                
                pNewObject->m_pChunk[0].m_pVertex = new mesh::vertex[NTempVertIDs];
                ASSERT( pNewObject->m_pChunk[0].m_pVertex );

                for ( TempVertID = 0; TempVertID < NTempVertIDs; TempVertID++ )
                {
                    pNewObject->m_pChunk[0].m_pVertex[TempVertID] =
                        s_pGeom->m_pChunk[TempVertChunkIDs[TempVertID]].m_pVertex[TempVertIDs[TempVertID]];
                }
            }

            //---   we can free up our temporary storage now
            x_free( TempVertIDs );
            x_free( TempVertChunkIDs );

            //---   remove the invalid faces from s_pGeom
            for ( ChunkID = 0; ChunkID < s_pGeom->m_nChunks; ChunkID++ )
            {
                s32         FaceCount;
                s32         F;
                mesh::face* pNewFaces;

                FaceCount = CountValidFaces( s_pGeom, ChunkID );

                pNewFaces = NULL;
                if ( FaceCount )
                {

                    //---   copy the valid faces into a new array
                    pNewFaces = new mesh::face[FaceCount];
                    NFacesAdded = 0;
                    for ( F = 0; F < s_pGeom->m_pChunk[ChunkID].m_nFaces; F++ )
                    {
                        if ( s_pGeom->m_pChunk[ChunkID].m_pFace[F].m_Index[0] != -1 )
                        {
                            pNewFaces[NFacesAdded++] = s_pGeom->m_pChunk[ChunkID].m_pFace[F];
                        }
                    }

                    ASSERT( NFacesAdded == FaceCount );
                }

                //---   delete the original array, and replace it with our new one
                if ( s_pGeom->m_pChunk[ChunkID].m_nFaces )
                    delete []s_pGeom->m_pChunk[ChunkID].m_pFace;
                s_pGeom->m_pChunk[ChunkID].m_pFace = pNewFaces;
                s_pGeom->m_pChunk[ChunkID].m_nFaces = FaceCount;
            }
        }

        //---   Merge pNewObject and s_pGeom into a new s_pGeom
        GeomMerge.AddGeom( s_pGeom );
        GeomMerge.AddGeom( pNewObject );
        delete s_pGeom;
        s_pGeom = new mesh::object;
        GeomMerge.MergeObjects( s_pGeom );
        delete pNewObject;

        //---   #### TODO: Remove unused verts from s_pGeom
    }
}

//==========================================================================

#define ERR_NONE                0
#define ERR_NPASSES             1
#define ERR_SUBMAT_RANGE        2
#define ERR_UV_RANGE            3
#define ERR_INTENSITY_RANGE     4
#define ERR_TEXTURE_RANGE       5
#define ERR_OPERATION_RANGE     6
#define ERR_WRAP_RANGE          7
#define ERR_TEXTURE_HEIGHT      8
#define ERR_TEXTURE_WIDTH       9
#define ERR_NWEIGHTS            10
#define ERR_MATRIX_RANGE        11
#define ERR_WEIGHT_RANGE        12
#define ERR_TOTAL_WEIGHT        13
#define ERR_QUAD                14
#define ERR_MATERIAL_RANGE      15
#define ERR_VERT_RANGE          16

static
xbool SanityCheck( mesh::object* pGeom )
{
    s32 ChunkID;
    s32 TexID;
    s32 MatID;
    s32 SubMatID;
    s32 VertID;
    s32 TriID;
    s32 WeightID;

    mesh::material*     pMat;
    mesh::sub_material* pSubMat;
    mesh::texture*      pTexture;
    mesh::chunk*        pChunk;
    mesh::vertex*       pVert;
    mesh::weight*       pWeight;
    mesh::face*         pFace;

    //---   do a sanity check on the materials
    for ( MatID = 0; MatID < pGeom->m_nMaterials; MatID++ )
    {
        pMat = &pGeom->m_pMaterial[MatID];
        
        if ( pMat->m_nPasses != 1 )
            return ERR_NPASSES;

        if ( (pMat->m_iSubMat[0] < 0) ||
             (pMat->m_iSubMat[0] >= pGeom->m_nSubMaterials) )
            return ERR_SUBMAT_RANGE;

        if ( pMat->m_iUV[0] != 0 )
            return ERR_UV_RANGE;
    }

    //---   do a sanity check on the sub-materials
    for ( SubMatID = 0; SubMatID < pGeom->m_nSubMaterials; SubMatID++ )
    {
        pSubMat = &pGeom->m_pSubMaterial[SubMatID];

        if ( (pSubMat->m_Intensity < -0.001f) ||
             (pSubMat->m_Intensity > 1.001f) )
            return ERR_INTENSITY_RANGE;

        if ( (pSubMat->m_iTexture < -1) ||
             (pSubMat->m_iTexture >= pGeom->m_nTextures) )
            return ERR_TEXTURE_RANGE;

        if ( (pSubMat->m_Operation < mesh::OP_OPAQUE) ||
            (pSubMat->m_Operation > mesh::OP_SUB) )
            return ERR_OPERATION_RANGE;

        if ( (pSubMat->m_WrapU < mesh::WMODE_NONE) ||
             (pSubMat->m_WrapU > mesh::WMODE_MIRROR) ||
             (pSubMat->m_WrapV < mesh::WMODE_NONE) ||
             (pSubMat->m_WrapV > mesh::WMODE_MIRROR) )
            return ERR_WRAP_RANGE;
    }

    //---   do a sanity check on the textures
    for ( TexID = 0; TexID < pGeom->m_nTextures; TexID++ )
    {
        pTexture = &pGeom->m_pTexture[TexID];

        if ( pTexture->m_Height < 0 )
            return ERR_TEXTURE_HEIGHT;

        if ( pTexture->m_Width < 0 )
            return ERR_TEXTURE_WIDTH;
    }

    //---   do a sanity check on the chunks
    for ( ChunkID = 0; ChunkID < pGeom->m_nChunks; ChunkID++ )
    {
        pChunk = &pGeom->m_pChunk[ChunkID];

        //---   do a sanity check on the verts
        for ( VertID = 0; VertID < pChunk->m_nVerts; VertID++ )
        {
            f32 TotalWeight;

            pVert = &pChunk->m_pVertex[VertID];

            if ( (pVert->m_nPasses < 0) ||
                 (pVert->m_nPasses > 1) )
                return ERR_NPASSES;

            if ( pVert->m_nWeights == 0 )
                return ERR_NWEIGHTS;

            //---   do a sanity check on the weights
            TotalWeight = 0.0f;
            for ( WeightID = 0; WeightID < pVert->m_nWeights; WeightID++ )
            {
                pWeight = &pVert->m_Weight[WeightID];

                if ( (pWeight->m_iMatrix < 0) ||
                     (pWeight->m_iMatrix >= pGeom->m_nBones) )
                    return ERR_MATRIX_RANGE;

                if ( (pWeight->m_Weight < -0.001f) ||
                     (pWeight->m_Weight > 1.001f) )
                    return ERR_WEIGHT_RANGE;

                TotalWeight += pWeight->m_Weight;
            }

            if ( ((TotalWeight - EPSILON) > 1.0f) ||
                 ((TotalWeight + EPSILON) < 1.0f) )
                return ERR_TOTAL_WEIGHT;
        }

        //---   do a sanity check on the tris
        for ( TriID = 0; TriID < pChunk->m_nFaces; TriID++ )
        {
            pFace = &pChunk->m_pFace[TriID];

            if ( pFace->m_bQuad )
                return ERR_QUAD;

            if ( (pFace->m_iMaterial < 0) ||
                 (pFace->m_iMaterial >= pGeom->m_nMaterials) )
                return ERR_MATERIAL_RANGE;

            for ( VertID = 0; VertID < 3; VertID++ )
            {
                if ( (pFace->m_Index[VertID] < 0) ||
                     (pFace->m_Index[VertID] >= pChunk->m_nVerts) )
                    return ERR_VERT_RANGE;
            }
        }
    }

    return ERR_NONE;
}

//==========================================================================

static
void LoadMorphTargetSet( void )
{
    s_pMorphTargetSet = new QRipMorphTargetSet;
    if ( !s_pMorphTargetSet->Load( s_InputFile[0] ) )
    {
        x_printf( "UNABLE TO LOAD MORPH TARGET SET!\n" );
    }
}

//==========================================================================

static
s32 MorphVertCompare( const void* pV1, const void* pV2 )
{
    s32* pVert1 = (s32*)pV1;
    s32* pVert2 = (s32*)pV2;

    if ( *pVert1 < *pVert2 )
        return -1;
    else
    if ( *pVert2 > *pVert1 )
        return 1;
    else
        return 0;
}

//==========================================================================

static
void MergeMorphDataAndGeom( void )
{
    s32                         v;
    s32                         ChunkID;
    s32                         TargetID;
    s32                         DeltaID;
    s32                         NVerts;
    s32*                        pVertList;
    s32                         NTargetsTotal;
    s32                         NTargetsChunk;
    QRipMorphTargetSet*         pNewSet;

    if ( !s_pMorphTargetSet )
        return;

    //---   count the # of targets that will be in our new set
    NTargetsTotal = 0;
    for ( ChunkID = 0; ChunkID < s_pGeom->m_nChunks; ChunkID++ )
    {
        NTargetsChunk = 0;
        for ( TargetID = 0; TargetID < s_pMorphTargetSet->m_NTargets; TargetID++ )
        {
            if ( !x_stricmp( s_pGeom->m_pChunk[ChunkID].m_Name,
                             s_pMorphTargetSet->m_pTargets[TargetID].m_ChunkName ) )
            {
                NTargetsChunk++;
            }
        }
        if ( NTargetsChunk )
        {
            NTargetsTotal += NTargetsChunk;
            NTargetsTotal++;    // for creation of default target
        }
    }

    if ( NTargetsTotal == 0 )
        return;

    //---   initialize our new target set
    pNewSet = new QRipMorphTargetSet;
    ASSERT( pNewSet );
    x_strcpy( pNewSet->m_Name, s_pMorphTargetSet->m_Name );
    x_strcpy( pNewSet->m_MeshName, s_pMorphTargetSet->m_MeshName );
    pNewSet->m_NTargets = NTargetsTotal;
    pNewSet->m_pTargets = new QRipMorphTarget[NTargetsTotal];
    ASSERT( pNewSet->m_pTargets );

    //---   fill in our new morph target set
    NTargetsTotal = 0;
    for ( ChunkID = 0; ChunkID < s_pGeom->m_nChunks; ChunkID++ )
    {
        //---   create temp storage for a vert list
        NVerts = 0;
        pVertList = (s32*)x_malloc( s_pGeom->m_pChunk[ChunkID].m_nVerts * sizeof(s32) );
        
        //---   create a list of verts using morph targets
        for ( TargetID = 0; TargetID < s_pMorphTargetSet->m_NTargets; TargetID++ )
        {
            if ( !x_stricmp( s_pGeom->m_pChunk[ChunkID].m_Name,
                             s_pMorphTargetSet->m_pTargets[TargetID].m_ChunkName ) )
            {
                //---   this morph target goes with this mesh, add the vert
                //      indices to our list
                for ( DeltaID = 0; DeltaID < s_pMorphTargetSet->m_pTargets[TargetID].m_NDeltas; DeltaID++ )
                {
                    //---   is this vert ID already in the list
                    for ( v = 0; v < NVerts; v++ )
                    {
                        if ( s_pMorphTargetSet->m_pTargets[TargetID].m_pDeltas[DeltaID].m_VertID ==
                             pVertList[v] )
                        {
                            break;
                        }
                    }
                    if ( v == NVerts )
                    {
                        //---   this vert wasn't found, add it to our list
                        pVertList[NVerts++] =
                            s_pMorphTargetSet->m_pTargets[TargetID].m_pDeltas[DeltaID].m_VertID;
                    }
                }
            }
        }

        if ( NVerts )
        {
            QRipMorphTarget* pTarget;

            //---   this chunk has morph targets in it...

            //---   sort the vert list
            x_qsort( pVertList, NVerts, sizeof(s32), MorphVertCompare );

            //---   create a default target
            x_strcpy( pNewSet->m_pTargets[NTargetsTotal].m_Name, "DEFAULT" );
            x_strcpy( pNewSet->m_pTargets[NTargetsTotal].m_ChunkName, s_pGeom->m_pChunk[ChunkID].m_Name );
            pNewSet->m_pTargets[NTargetsTotal].m_NDeltas = NVerts;
            pNewSet->m_pTargets[NTargetsTotal].m_pDeltas = new QRipMorphDelta[NVerts];
            ASSERT( pNewSet->m_pTargets[NTargetsTotal].m_pDeltas );

            //---   set up the delta values for the default target
            for ( DeltaID = 0; DeltaID < NVerts; DeltaID++ )
            {
                pNewSet->m_pTargets[NTargetsTotal].m_pDeltas[DeltaID].m_VertID = pVertList[DeltaID];
                pNewSet->m_pTargets[NTargetsTotal].m_pDeltas[DeltaID].m_Delta.Set( 0.0f, 0.0f, 0.0f );
            }

            //---   up the morph target count
            NTargetsTotal++;
            ASSERT( NTargetsTotal <= pNewSet->m_NTargets );

            //---   loop through the all the targets, adding the deltas as appropriate
            for ( TargetID = 0; TargetID < s_pMorphTargetSet->m_NTargets; TargetID++ )
            {
                pTarget = &s_pMorphTargetSet->m_pTargets[TargetID];
                if ( !x_stricmp( pTarget->m_ChunkName, s_pGeom->m_pChunk[ChunkID].m_Name ) )
                {
                    //---   we found a morph target that belongs to this chunk, add the deltas
                    x_strcpy( pNewSet->m_pTargets[NTargetsTotal].m_Name, pTarget->m_Name );
                    x_strcpy( pNewSet->m_pTargets[NTargetsTotal].m_ChunkName, s_pGeom->m_pChunk[ChunkID].m_Name );
                    pNewSet->m_pTargets[NTargetsTotal].m_NDeltas = NVerts;
                    pNewSet->m_pTargets[NTargetsTotal].m_pDeltas = new QRipMorphDelta[NVerts];
                    ASSERT( pNewSet->m_pTargets[NTargetsTotal].m_pDeltas );

                    for ( DeltaID = 0; DeltaID < NVerts; DeltaID++ )
                    {
                        pNewSet->m_pTargets[NTargetsTotal].m_pDeltas[DeltaID].m_VertID = pVertList[DeltaID];

                        //---   find this delta in the source target
                        for ( v = 0; v < pTarget->m_NDeltas; v++ )
                        {
                            if ( pTarget->m_pDeltas[v].m_VertID == pVertList[DeltaID] )
                                break;
                        }

                        if ( v < pTarget->m_NDeltas )
                            pNewSet->m_pTargets[NTargetsTotal].m_pDeltas[DeltaID].m_Delta = pTarget->m_pDeltas[v].m_Delta;
                        else
                            pNewSet->m_pTargets[NTargetsTotal].m_pDeltas[DeltaID].m_Delta.Set( 0.0f, 0.0f, 0.0f );
                    }

                    //---   up the morph target count
                    NTargetsTotal++;
                    ASSERT( NTargetsTotal <= pNewSet->m_NTargets );
                }
            }
        }

        //---   delete temp storage for a vert list
        x_free( pVertList );
    }

    ASSERT( NTargetsTotal == pNewSet->m_NTargets );

    delete s_pMorphTargetSet;
    s_pMorphTargetSet = pNewSet;
}

//==========================================================================

static
void RotateSkin180( void )
{
    s32 i, j;

    //---   rotate the skin 180 degrees
    for ( i = 0; i < s_pGeom->m_nChunks; i++ )
    {
        for ( j = 0; j < s_pGeom->m_pChunk[i].m_nVerts; j++ )
        {
            s_pGeom->m_pChunk[i].m_pVertex[j].m_vPos.X = -s_pGeom->m_pChunk[i].m_pVertex[j].m_vPos.X;
            s_pGeom->m_pChunk[i].m_pVertex[j].m_vPos.Z = -s_pGeom->m_pChunk[i].m_pVertex[j].m_vPos.Z;
        }
    }

    //---   rotate the morph deltas 180 degrees
    for ( i = 0; i < s_pMorphTargetSet->m_NTargets; i++ )
    {
        for ( j = 0; j < s_pMorphTargetSet->m_pTargets[i].m_NDeltas; j++ )
        {
            s_pMorphTargetSet->m_pTargets[i].m_pDeltas[j].m_Delta.X = -s_pMorphTargetSet->m_pTargets[i].m_pDeltas[j].m_Delta.X;
            s_pMorphTargetSet->m_pTargets[i].m_pDeltas[j].m_Delta.Z = -s_pMorphTargetSet->m_pTargets[i].m_pDeltas[j].m_Delta.Z;
        }
    }
}

//==========================================================================

s32 main( int argc, char** argv )
{   
    s32 i;
    QGeomMerge  MergeObject;

    //---   Initialize system
    x_Init();
    s_SkinFileName[0] = '\0';
    s_SkelFileName[0] = '\0';
    s_BoneAssignmentFileName[0] = '\0';
    g_BoneWeightingFileName[0] = '\0';
    g_StripFileName[0] = '\0';
    g_TextureSeamFileName[0] = '\0';

    //---   Process switches and setup globals
    ProcessSwitches( argc, argv );

    //---   output some info
    if ( g_Verbose )
    {
        x_printf( "Dest file name:            %1s\n", s_SkinFileName );
        x_printf( "Skel file name:            %1s\n", s_SkelFileName );
    }

    //---   Load the morph target set
    LoadMorphTargetSet();

    //===   Load the geometry

    //---   load each of the input files, adding them into
    //      one mesh
    for ( i = 0; i < s_NInputFiles; i++ )
    {
        xbool           bStat;
        mesh::object*   TempGeom;
        CTextFile*      TempTf;

        TempTf = new CTextFile();

        if ( g_Verbose )
            x_printf( "INFILE(s): %s\n", s_InputFile[i] );

        //---   load the text file
        bStat = TempTf->LoadTextFile( s_InputFile[i] );
        if ( !bStat )
        {
            x_printf( "ERROR: Unable to load %s\n", s_InputFile[i] );
            PauseOnError();
            exit( 0 );
        }
        
        //---   put the text file into a temp geom object, and merge
        //      it in with our master geom object
        TempGeom = new mesh::object();
        TempGeom->Load( *TempTf );
        MergeObject.AddGeom( TempGeom );
        delete TempGeom;
        delete TempTf;
    }

    //---   merge the objects together
    if ( g_Verbose )
        x_printf( "Merging input files together...\n" );

    //---   load the geometry
    s_pGeom = new mesh::object();
    MergeObject.MergeObjects( s_pGeom );

    //---   get rid of unused morph targets, and make sure all morph targets
    //      for each mesh has the same vertex indices
    MergeMorphDataAndGeom();

    //---   save out ASC file showing bone assignments if necessary
    if ( s_BoneAssignmentFileName[0] != '\0' )
    {
        ExportBoneAssignments( s_BoneAssignmentFileName );
    }

    //---   Load ASF if needed
    if( s_SkelFileName[0] )
    {
        s32 i;
        
        //---   let the user know what's going on
        x_printf( "Loading ASF file...\n" );
        
        //---   load the skeleton file
        LoadSkel( s_SkelFileName );

        //---   Write out bone info for debugging
        if ( g_Verbose )
        {
            for( i = 0; i < s_pGeom->m_nBones; i++ )
            {
                s32 BoneID;

                ////////////////////////////////////////////////////////////
                //---   the bone as it is in the source file
                ////////////////////////////////////////////////////////////

                x_printf( "%2d]  %16s -> ", i, s_pGeom->m_pBone[i].m_Name );

                ////////////////////////////////////////////////////////////
                //---   the matching bone in the ASF
                ////////////////////////////////////////////////////////////

                BoneID = FindASFBoneID( i );
                if( BoneID != -1 )
                    x_printf( "%16s\n", s_Skel.m_pBone[BoneID].BoneName );
                else
                    x_printf( "%16s\n", "BONE_NOT_FOUND" );
            }
            x_printf( "\n" );
        }
    }

    //---   The tris that use texture specified with the -TN switch should
    //      be pulled out and put into a separate mesh
    if ( g_Verbose )
        x_printf( "Pulling out special textures as meshes (-TN).\n" );
    PullOutTextureMeshes();

    //---   rotate the skin if necessary
    if ( g_Rotate180 )
        RotateSkin180();

    //---   Process geometry
    x_printf( "Processing geometry (this may take a while)...\n" );
    RipGeometry( s_SkinFileName );

    //---   clean up
    //ASSERT( SanityCheck( s_pGeom ) == ERR_NONE );
    delete s_pGeom;
    if ( s_pMorphTargetSet )
        delete s_pMorphTargetSet;

    //---   Shut down.
    //while ( !getch() )
       //;

    //x_Kill();

    return 0;
}


//=====================================================================================================================================
// VertList
//=====================================================================================================================================
VertList::VertList( void )
{
    mpHead     = NULL;
    mpNextList = NULL;
    bDynamic   = FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
VertList::~VertList( void )
{
    KillList( );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void VertList::AttachList( s32 MeshID, s32 VertID )
{
    VertList* pListList;

    VertList* pNewList = new VertList;
    ASSERT( pNewList );

    pListList = this;

    // Walk the list of lists until you find the end of the road.
    while( pListList->mpNextList )
        pListList = pListList->mpNextList;

    pListList->mpNextList = pNewList;
    pNewList->mpNextList = NULL;

    pNewList->AddNode( MeshID, VertID );
    pNewList->bDynamic = TRUE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VertList::KillList( void )
{
    VertList* pCurrentList;
    VertList* pPrevList;

    // Destroy any connected lists.
    do
    {
        pCurrentList = this;
        pPrevList    = this;

        // While there are lists in this list loop until you get to the end.
        while( pCurrentList->mpNextList )
        {
            pPrevList    = pCurrentList;
            pCurrentList = pCurrentList->mpNextList;
        }

        // Well, we are down to the root list, so clean out it's nodes.
        VertNode* pNode;
        VertNode* pNextNode;

        pNode = pCurrentList->mpHead;
        while( pNode )
        {
            pNextNode = pNode->pNext;
            delete pNode;
            pNode = pNextNode;
        }
        pCurrentList->mpHead = NULL;

        if( pCurrentList != this && pCurrentList->bDynamic )
        {
            delete pPrevList->mpNextList;
            pPrevList->mpNextList = NULL;
        }

    }while( mpNextList );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VertList::AddNode( s32 MeshID, s32 VertID )
{
    VertNode* pNewNode;
    VertNode* pNextNode;

    pNewNode = new VertNode;
    ASSERT( pNewNode );

    pNewNode->pNext = NULL;
    pNewNode->MeshID = MeshID;
    pNewNode->VertID = VertID;

    // Test to see if it's the first node.
    if( mpHead == NULL )
    {
        mpHead = pNewNode;
    }
    else
    {
        // Find the end of the list.
        pNextNode = mpHead;
        while( pNextNode->pNext != NULL )
            pNextNode = pNextNode->pNext;

        pNextNode->pNext = pNewNode;
    }
}
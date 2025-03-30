#include <conio.h>
#include <stdlib.h>


#include "x_stdio.hpp"
#include "x_files.hpp"

#include "CTextFile.h"
#include "AsciiMesh.h"
#include "PS2Ripper.h"
#include "PCRipper.h"
#include "GCRipper.h"
#include "XBOXRipper.h"
#include "GeomMerge.h"


////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////

#define MAX_INPUT_FILES     60

#define RIP_TARGET_UNKNOWN  0x00
#define RIP_TARGET_PS2      0x01
#define RIP_TARGET_PC       0x02
#define RIP_TARGET_GAMECUBE 0x03
#define RIP_TARGET_XBOX     0x04


////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////

s32     g_Verbose = FALSE;
xbool   g_ExportTextures = TRUE;
xbool   g_ExportPosPtrs = FALSE;
xbool   g_ExportUVPtrs = FALSE;
xbool   g_ExportRGBAPtrs = FALSE;
xbool   g_ExportNormalPtrs = FALSE;
xbool   g_BuildTriStrips = FALSE;
xbool   g_EnableAlpha = FALSE;
xbool   g_EnableAA = FALSE;
xbool   g_UseMips = FALSE;
xbool   g_DynamicLighting = FALSE;
X_FILE* g_TextureNameFile = NULL;
char    g_InputFile[MAX_INPUT_FILES][255];
s32     g_NInputFiles = 0;
char    g_OutputFile[255];


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

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

static
void OutputUsageMessage( void )
{
    x_printf( "Usage: RipGeom [options] <Inputs> <output.GDF>\n\n" );
    x_printf( "Version 1.4\n\n" );
    x_printf( "    The inputs are given in the form of a list of MTF files.\n\n" );
    x_printf( "    It is possible to specify a large number of MTF\n" );
    x_printf( "    files by passing @data.txt, where data.txt contains\n" );
    x_printf( "    a list of MTF files.\n\n" );
    x_printf( "Options:\n" );
    x_printf( "  -PS2 ............ Target PlayStation 2\n" );
    x_printf( "  -PC ........... . Target PC\n" );
    x_printf( "  -GC ............. Target GameCube\n" );
    x_printf( "  -XBOX ........... Target XBOX\n" );
    x_printf( "  -M .............. Use mip-mapping\n" );
    x_printf( "  -S .............. Build triangle strips\n" );
    x_printf( "  -DL ............. Use dynamic lighting\n" );
    x_printf( "  -A .............. Enable alpha\n" );
    x_printf( "  -AA.............. Enable antialiasing\n" );
    x_printf( "  -NT ............. Don't export textures\n" );
    x_printf( "  -T <filename> ... Export texture names to <filename>\n" );
    x_printf( "  -EP ............. Export vertex position ptrs\n" );
    x_printf( "  -ET ............. Export vertex texture coord. ptrs\n" );
    x_printf( "  -EC ............. Export vertex color data ptrs\n" );
    x_printf( "  -EN ............. Export vertex normal ptrs\n" );
    x_printf( "  -V .............. Verbose output\n" );
    x_printf( "Special Notes:\n" );
    x_printf( "  -DL cannot be combined with -EC\n" );
    x_printf( "  -EN requires -DL be specified\n" );
}

//==========================================================================

static
xbool IsInputFile( char* filename )
{
    s32 i;

    i = x_strlen( filename );
    i -= x_strlen( ".MTF" );

    if ( !x_stricmp( &filename[i], ".MTF" ) )
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

void MESHUTIL_RemoveDegenerateTris( mesh::object* pMesh )
{
    s32             i;
    s32             ChunkID;
    mesh::chunk*    pChunk;
    s32             FaceID;
    mesh::face*     pFace;
    s32             NDegenFaces;
    s32*            pDegenFaces;
    vector3         temp1, temp2, temp3;

    for ( ChunkID = 0; ChunkID < pMesh->m_nChunks; ChunkID++ )
    {
        pChunk = &pMesh->m_pChunks[ChunkID];

        //---   allocate some temporary storage for the face indices
        NDegenFaces = 0;
        pDegenFaces = (s32*)x_malloc( sizeof(s32) * pChunk->m_nFaces );
        ASSERT( pDegenFaces );
        
        //---   create a list of degenerate faces
        for ( FaceID = 0; FaceID < pChunk->m_nFaces; FaceID++ )
        {
            pFace = &pChunk->m_pFaces[FaceID];

            if ( pFace->m_bQuad )
                continue;

            temp1 = pChunk->m_pVerts[pFace->m_Index[0]].m_vPos -
                    pChunk->m_pVerts[pFace->m_Index[1]].m_vPos;
            temp2 = pChunk->m_pVerts[pFace->m_Index[1]].m_vPos -
                    pChunk->m_pVerts[pFace->m_Index[2]].m_vPos;
            temp3 = pChunk->m_pVerts[pFace->m_Index[0]].m_vPos -
                    pChunk->m_pVerts[pFace->m_Index[2]].m_vPos;

            if ( (temp1.Length() < 0.001f) ||
                 (temp2.Length() < 0.001f) ||
                 (temp3.Length() < 0.001f) )
            {
                pDegenFaces[NDegenFaces++] = FaceID;
            }
        }

        //---   did we have any degenerate faces?
        if ( NDegenFaces )
        {
            s32         NNewFaces;
            mesh::face* pNewFaceList = NULL;

            NNewFaces = pChunk->m_nFaces - NDegenFaces;

            //---   create a new face list, sans the degenerate ones
            if ( NNewFaces )
            {
                pNewFaceList = new mesh::face[NNewFaces];
                ASSERT( pNewFaceList );

                NNewFaces = 0;
                for ( FaceID = 0; FaceID < pChunk->m_nFaces; FaceID++ )
                {
                    //---   is this a degenerate face?
                    for ( i = 0; i < NDegenFaces; i++ )
                    {
                        if ( pDegenFaces[i] == FaceID )
                            break;
                    }

                    if ( i == NDegenFaces )
                    {
                        //---   it is not a degenerate face, add it
                        pNewFaceList[NNewFaces++] = pChunk->m_pFaces[FaceID];
                    }
                }
            }

            //---   point the chunk to our new face list
            pChunk->m_nFaces = NNewFaces;
            if ( pChunk->m_pFaces )
                delete []pChunk->m_pFaces;
            pChunk->m_pFaces = pNewFaceList;
        }

        //---   clean up
        x_free( pDegenFaces );
    }
}

//==========================================================================

typedef struct SNormalVert
{
    vector3 Pos;
    vector3 Normal;
} t_NormalVert;

#define EPSILON 0.001f

void CalcVertNormals( mesh::object* pMesh )
{
    s32             i;
    s32             ChunkID;
    mesh::chunk*    pChunk;
    s32             VertID;
    s32             TempVertID;
    s32             NTempVerts;
    t_NormalVert*   pTempVerts;
    s32             TriID;
    f32             TriArea;
    vector3         TriNormal;
    vector3         DV1, DV2;
    f32             Length;

    //---   to avoid normals looking weird across textures seams, do them
    //      based on position (NOT INDEX), and by mesh
    for ( ChunkID = 0; ChunkID < pMesh->m_nChunks; ChunkID++ )
    {
        pChunk = &pMesh->m_pChunks[ChunkID];

        //---   create a list of temporary verts
        pTempVerts = (t_NormalVert*)x_malloc( sizeof(t_NormalVert) * pChunk->m_nVerts );
        ASSERT( pTempVerts );
        NTempVerts = 0;
        for ( VertID = 0; VertID < pChunk->m_nVerts; VertID++ )
        {
            //---   find this vert in the temporary list
            for ( TempVertID = 0; TempVertID < NTempVerts; TempVertID++ )
            {
                if ( (x_abs(pTempVerts[TempVertID].Pos.X - pChunk->m_pVerts[VertID].m_vPos.X) < EPSILON) &&
                     (x_abs(pTempVerts[TempVertID].Pos.Y - pChunk->m_pVerts[VertID].m_vPos.Y) < EPSILON) &&
                     (x_abs(pTempVerts[TempVertID].Pos.Z - pChunk->m_pVerts[VertID].m_vPos.Z) < EPSILON) )
                {
                    break;
                }
            }
            if ( TempVertID == NTempVerts )
            {
                //---   the current vert wasn't found, so add it
                pTempVerts[NTempVerts].Normal.Zero();
                pTempVerts[NTempVerts].Pos = pChunk->m_pVerts[VertID].m_vPos;
                NTempVerts++;
            }
        }

        //---   now, for each triangle this mesh uses, add the triangle's normal
        //      to each of its verts, weighted by the triangle area
        for ( TriID = 0; TriID < pChunk->m_nFaces; TriID++ )
        {
            //---   Compute tri normal and area
            DV1 = pChunk->m_pVerts[pChunk->m_pFaces[TriID].m_Index[1]].m_vPos -
                  pChunk->m_pVerts[pChunk->m_pFaces[TriID].m_Index[0]].m_vPos;
            DV2 = pChunk->m_pVerts[pChunk->m_pFaces[TriID].m_Index[2]].m_vPos -
                  pChunk->m_pVerts[pChunk->m_pFaces[TriID].m_Index[0]].m_vPos;
            TriNormal = Cross( DV1, DV2 );
            Length = TriNormal.Length();
            ASSERT( Length > 0.0f );
            TriArea = Length * 0.5f;
            TriNormal *= (1.0f / Length);

            for ( i = 0; i < 3; i++ )
            {
                //---   find each of the verts in the temporary list
                for ( TempVertID = 0; TempVertID < NTempVerts; TempVertID++ )
                {
                    if ( (x_abs(pTempVerts[TempVertID].Pos.X - pChunk->m_pVerts[pChunk->m_pFaces[TriID].m_Index[i]].m_vPos.X) < EPSILON) &&
                         (x_abs(pTempVerts[TempVertID].Pos.Y - pChunk->m_pVerts[pChunk->m_pFaces[TriID].m_Index[i]].m_vPos.Y) < EPSILON) &&
                         (x_abs(pTempVerts[TempVertID].Pos.Z - pChunk->m_pVerts[pChunk->m_pFaces[TriID].m_Index[i]].m_vPos.Z) < EPSILON) )
                    {
                        break;
                    }
                }
                ASSERT( TempVertID < NTempVerts );

                pTempVerts[TempVertID].Normal += TriNormal * TriArea;
            }
        }

        //---   renormalize the normals
        for ( TempVertID = 0; TempVertID < NTempVerts; TempVertID++ )
            pTempVerts[TempVertID].Normal.Normalize();

        //---   now, copy the temp verts normals back into the real normals
        for ( TempVertID = 0; TempVertID < NTempVerts; TempVertID++ )
        {
            //---   find every occurence of the temp vert in the mesh
            for ( VertID = 0; VertID < pChunk->m_nVerts; VertID++ )
            {
                if ( (x_abs(pTempVerts[TempVertID].Pos.X - pChunk->m_pVerts[VertID].m_vPos.X) < EPSILON) &&
                     (x_abs(pTempVerts[TempVertID].Pos.Y - pChunk->m_pVerts[VertID].m_vPos.Y) < EPSILON) &&
                     (x_abs(pTempVerts[TempVertID].Pos.Z - pChunk->m_pVerts[VertID].m_vPos.Z) < EPSILON) )
                {
                    pChunk->m_pVerts[VertID].m_vNormal = pTempVerts[TempVertID].Normal;
                }
            }
        }

        //---   clean up
        x_free( pTempVerts );
    }
}

//==========================================================================

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

int main( int argc, char* argv[] )
{
    s32             i;
    mesh::object    TheGeom;
    xbool           bStat;
    s32             RipTarget = RIP_TARGET_UNKNOWN;
    X_FILE*         fh;
    QGeomMerge      MergeObject;

    x_Init();
    g_InputFile[0][0] = '\0';
    g_OutputFile[0] = '\0';

    //---   skip past the executable name
    argc--;
    argv++;

    //---   process all of the options
    while ( argc )
    {
        char*   pArg;

        //---   get the argument
        pArg = argv[0];

        //---   check for a valid target
        if ( !x_strcmp( pArg, "-PS2" ) )
        {
            RipTarget = RIP_TARGET_PS2;
        }
        else
        if ( !x_strcmp( pArg, "-PC" ) )
        {
            RipTarget = RIP_TARGET_PC;
        }
        else
        if ( !x_strcmp( pArg, "-GC" ) )
        {
            RipTarget = RIP_TARGET_GAMECUBE;
        }
        //---   check the number of mip levels
        else
        if ( !x_strcmp( pArg, "-XBOX" ) )
        {
            RipTarget = RIP_TARGET_XBOX;
        }
        //---   check the number of mip levels
        else
        if ( !x_strcmp( pArg, "-M" ) )
        {
            g_UseMips = TRUE;
        }
        //---   check for verbose output
        else
        if ( !x_strcmp( pArg, "-V" ) )
        {
            g_Verbose = TRUE;
        }
        //---   check for vertex ptr output
        else
        if ( !x_strcmp( pArg, "-EP" ) )
        {
            g_ExportPosPtrs = TRUE;
        }
        //---   check for UV ptr output
        else
        if ( !x_strcmp( pArg, "-ET" ) )
        {
            g_ExportUVPtrs = TRUE;
        }
        //---   check for RGBA ptr output
        else
        if ( !x_strcmp( pArg, "-EC" ) )
        {
            g_ExportRGBAPtrs = TRUE;
        }
        //---   check for Normal ptr output
        else
        if ( !x_strcmp( pArg, "-EN" ) )
        {
            g_ExportNormalPtrs = TRUE;
        }
        //---   check for building triangle strips
        else
        if ( !x_strcmp( pArg, "-S" ) )
        {
            g_BuildTriStrips = TRUE;
        }
        //---   check for dynamic lighting
        else
        if ( !x_strcmp( pArg, "-DL" ) )
        {
            g_DynamicLighting = TRUE;
        }
        //---   do we allow alpha?
        else
        if ( !x_strcmp( pArg, "-A" ) )
        {
            g_EnableAlpha = TRUE;
        }
        //---   do we allow antialiasing?
        else
        if ( !x_strcmp( pArg, "-AA" ) )
        {
            g_EnableAA = TRUE;
        }
        //---   should we export textures?
        else
        if ( !x_strcmp( pArg, "-NT" ) )
        {
            g_ExportTextures = FALSE;
        }
        //---   should we export texture names?
        else
        if ( !x_strcmp( pArg, "-T" ) )
        {
            argc--;
            argv++;
            if ( !argc )
            {
                x_printf( "ERROR: Missing texture file name.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
            g_TextureNameFile = x_fopen( argv[0], "w" );
            if ( !g_TextureNameFile )
            {
                x_printf( "ERROR: Unable to open %s for writing.\n", argv[0] );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
        }
        //---   check for an unknown option
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
                x_strcpy( g_InputFile[g_NInputFiles], pArg );
                StripFileExtension( g_InputFile[g_NInputFiles] );
                x_strcat( g_InputFile[g_NInputFiles], ".MTF" );
                g_NInputFiles++;
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
                    x_strcpy( g_InputFile[g_NInputFiles], temp );
                    StripFileExtension( g_InputFile[g_NInputFiles] );
                    x_strcat( g_InputFile[g_NInputFiles], ".MTF" );
                    g_NInputFiles++;
                }

                x_fclose( ResponseFile );
            }
            else
            if ( g_OutputFile[0] == '\0' )
            {
                //---   must be the output file
                x_strcpy( g_OutputFile, pArg );
                StripFileExtension( g_OutputFile );
                x_strcat( g_OutputFile, ".GDF" );
            }
            else
            {
                //---   we have an error
                x_printf( "ERROR: Too many options.\n" );
                OutputUsageMessage();
                PauseOnError();
                exit( 0 );
            }
        }

        //---   advance to the next argument
        argc--;
        argv++;
    }

    //---   print out appropriate errors
    if ( RipTarget == RIP_TARGET_UNKNOWN )
    {
        x_printf( "ERROR: Unknown target.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }

    if ( g_InputFile[0][0] == '\0' )
    {
        x_printf( "ERROR: No input file specified.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }

    if ( g_OutputFile[0] == '\0' )
    {
        x_printf( "ERROR: No output file specified.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }

    if ( g_DynamicLighting && g_ExportRGBAPtrs )
    {
        x_printf( "ERROR: Cannot export vertex colors with dynamic lighting.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }

    if ( !g_DynamicLighting && g_ExportNormalPtrs )
    {
        x_printf( "ERROR: Cannot export vertex normals without dynamic lighting.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }

    //---   print out useful info
    if ( g_Verbose )
    {
        //---   display the target
        switch ( RipTarget )
        {
        case RIP_TARGET_PS2:
            x_printf( "TARGET: PlayStation 2\n" );
            break;
        case RIP_TARGET_PC:
            x_printf( "TARGET: PC\n" );
            break;
        case RIP_TARGET_GAMECUBE:
            x_printf( "TARGET: GameCube\n" );
            break;
        case RIP_TARGET_XBOX:
            x_printf( "TARGET: XBOX\n" );
            break;
        }
    }

    //---   load each of the input files, adding them into
    //      one mesh
    for ( i = 0; i < g_NInputFiles; i++ )
    {
        mesh::object*   TempGeom;
        CTextFile*      TempTf;

        TempTf = new CTextFile();

        if ( g_Verbose )
            x_printf( "INFILE(s): %s\n", g_InputFile[i] );

		x_printf("Loading text file..\n");
        //---   load the text file
        bStat = TempTf->LoadTextFile( g_InputFile[i] );
		x_printf("Completed loading text file...\n");
        if ( !bStat )
        {
            x_printf( "ERROR: Unable to load %s\n", g_InputFile[i] );
            PauseOnError();
            exit( 0 );
        }
        
        //---   put the text file into a temp geom object, and merge
        //      it in with our master geom object
		x_printf("Creating mesh::object()\n");
        TempGeom = new mesh::object();
		x_printf("TempGeom->Load()\n");
        TempGeom->Load( *TempTf );
		x_printf("MergeObject.AddGeom(TempGeom)\n");
        MergeObject.AddGeom( TempGeom );
        delete TempGeom;
        delete TempTf;
    }

    //---   merge the objects together
    if ( g_Verbose )
        x_printf( "Merging input files together...\n" );
    MergeObject.MergeObjects( &TheGeom );

    if ( g_Verbose )
        x_printf( "OUTFILE: %s\n", g_OutputFile );

    //---   open up the output file for writing
    fh = x_fopen( g_OutputFile, "wb" );
    if ( !fh )
    {
        x_printf( "ERROR: Unable to open %s for writing.\n", g_OutputFile );
        PauseOnError();
        exit( 0 );
    }

    //---   remove the degenerate polys
    MESHUTIL_RemoveDegenerateTris( &TheGeom );

    //---   build some normals
    if ( g_DynamicLighting )
        CalcVertNormals( &TheGeom );

    //---   do a sanity check...environment mapping must be stripped and use dynamic lighting
    for ( i = 0; i < TheGeom.m_nChunks; i++ )
    {
        if ( IsEnvMapped( TheGeom.m_pChunks[i].m_Name ) )
        {
            if ( (g_DynamicLighting == FALSE) || (g_BuildTriStrips == FALSE) )
            {
                x_printf( "ERROR: Environment mapping requires strips and dynamic lighting.\n" );
                PauseOnError();
                exit( 0 );
            }
        }
    }

    //---   Build the mesh into the appropriate format
    switch ( RipTarget )
    {
    case RIP_TARGET_PS2:
        ExportPS2Geometry( TheGeom, fh );
        break;
    case RIP_TARGET_PC:
        ExportPCGeometry( TheGeom, fh );
        break;
    case RIP_TARGET_GAMECUBE:
        ExportGCGeometry( TheGeom, fh );
        break;
    case RIP_TARGET_XBOX:
        ExportXBOXGeometry( TheGeom, fh );
        break;
    }

    x_fclose( fh );
    
    if ( g_TextureNameFile )
        x_fclose( g_TextureNameFile );

    x_printf( "%s was written out.\n", g_OutputFile );

    //while ( !getch() )
        //;

    return 0;
}
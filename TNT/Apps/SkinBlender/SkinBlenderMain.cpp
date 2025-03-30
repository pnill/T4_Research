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

#include "SkinBlenderMain.hpp"
#include "VertexNode.hpp"


//=====================================================================================================================================
// Defines
//=====================================================================================================================================
#define SKIN_FILENAME_MAX   255

#define TARGET_UNKNOWN      0
#define TARGET_XBOX         1
#define TARGET_GAMECUBE     2
#define TARGET_PS2          4

#define EPSILON             0.05f


//=====================================================================================================================================
// Globals
//=====================================================================================================================================
xbool       gTarget = TARGET_UNKNOWN;
char        gSkinFilename1[SKIN_FILENAME_MAX];
char        gSkinFilename2[SKIN_FILENAME_MAX];


//=====================================================================================================================================
// Function Prototypes
//=====================================================================================================================================
static void  BlendSkins( void );
static void  BlendSkinsGC( void );
static void  BlendSkinsPS2( void );
static void  BlendSkinsXBOX( void );

static xbool IsInputFile( char* filename );
static void  OutputUsageMessage( void );
static void  PauseOnError( void );
static void  ProcessSwitches( int argc, char** argv );
static void  StripFileExtension( char* filename );


//=====================================================================================================================================
// Implementation
//=====================================================================================================================================
s32 main( int argc, char** argv )
{   
    // Initialize the x_files.
    x_Init();

    gSkinFilename1[0] = '\0';
    gSkinFilename2[0] = '\0';

    // Process switches and setup globals
    ProcessSwitches( argc, argv );

    x_printf( "Skin 1 --> %s\n", gSkinFilename1 );
    x_printf( "Skin 2 --> %s\n", gSkinFilename2 );

    // Time to blend these mutha fuckas.
    BlendSkins( );

    x_Kill();

    return 0;
}


//=====================================================================================================================================
void BlendSkins( void )
{
    switch( gTarget )
    {
        case TARGET_XBOX:       BlendSkinsXBOX( );  break;
        case TARGET_GAMECUBE:   BlendSkinsXBOX( );  break;
        case TARGET_PS2:        BlendSkinsXBOX( );  break;
        
        default:
            x_printf( "ERROR...Invalid target" );
            PauseOnError();
            exit( 0 );
            break;
    }
}


//=====================================================================================================================================
void BlendSkinsXBOX( void )
{
    VertList        VList;
    VertList        NList;
    VertList*       pCurrentList;
    VertNode*       pTestNode;
    XBOXSkinInfo*   pArchivedSkin;

    vector3*        pvTestPosition;
    vector3*        pvArchivedPosition;
    vector3*        pvTestNormal;
    vector3*        pvArchivedNormal;
    f32             NormalDot;


    XBOXSkinInfo*   pCurrentSkin;
    char*           pCurrentFilename;
    XBOXSkinInfo    Skin1;
    XBOXSkinInfo    Skin2;

    s32             Offset;
    s32             MeshID;
    s32             VertID;

    s32             CommonVerts;
    vector3         AvgNormal;

    
    pCurrentSkin = &Skin1;
    pCurrentFilename = gSkinFilename1;

    while( pCurrentSkin )
    {
        // Open the exported skin file.
        //-------------------------------------------------------------------------------------------------------------------------------------
        x_printf( "Opening file %s\n", pCurrentFilename );
        pCurrentSkin->mpFile = x_fopen( pCurrentFilename, "rb" );
        if( pCurrentSkin->mpFile == NULL )
        {
            x_printf( "ERROR: Opening file %s.\n", pCurrentFilename );
            PauseOnError();
            exit( 0 );
        }

        // Determine the size of the file.
        //-------------------------------------------------------------------------------------------------------------------------------------
        x_fseek( pCurrentSkin->mpFile, 0, X_SEEK_END );
        pCurrentSkin->mSizeOfFile = x_ftell( pCurrentSkin->mpFile );
        
        x_fseek( pCurrentSkin->mpFile, 0, X_SEEK_SET );
        pCurrentSkin->mSizeOfFile -= x_ftell( pCurrentSkin->mpFile );

        // Allocate enough space to read the entire skin file.
        //-------------------------------------------------------------------------------------------------------------------------------------
        pCurrentSkin->mpRawData = new byte[pCurrentSkin->mSizeOfFile];
        ASSERT( pCurrentSkin->mpRawData );

    
        // Read the contents of the entire file.
        //-------------------------------------------------------------------------------------------------------------------------------------
        x_printf( "Reading skin from file %s\n", pCurrentFilename );
        if( x_fread( pCurrentSkin->mpRawData, pCurrentSkin->mSizeOfFile, 1, pCurrentSkin->mpFile ) != 1 )
        {
            x_printf( "ERROR: reading file %s.\n", pCurrentFilename );
            PauseOnError();
            exit( 0 );
        }

        // Close the file.
        //-------------------------------------------------------------------------------------------------------------------------------------
        x_printf( "Closing %s\n", pCurrentFilename );
        x_fclose( pCurrentSkin->mpFile );   
        pCurrentSkin->mpFile = NULL;


        // Now that we have the files in data.  Time to rip them apart.
        //-------------------------------------------------------------------------------------------------------------------------------------
        pCurrentSkin->mpFileHeader = (XBOXSkinFileHeader*)pCurrentSkin->mpRawData;

        pCurrentSkin->mpVerts      = new XBOXSkinD3DVert*[   pCurrentSkin->mpFileHeader->mNMeshes];
        pCurrentSkin->mpMeshHeader = new XBOXSkinMeshHeader*[pCurrentSkin->mpFileHeader->mNMeshes];
        ASSERT( pCurrentSkin->mpVerts );
        ASSERT( pCurrentSkin->mpMeshHeader );


        // Find the locations for where each vertex list for each mesh start in the file.
        //-------------------------------------------------------------------------------------------------------------------------------------
        Offset = sizeof( XBOXSkinFileHeader );
        for( MeshID = 0; MeshID < pCurrentSkin->mpFileHeader->mNMeshes; MeshID++ )
        {
            // Get the mesh header for this particular mesh.
            pCurrentSkin->mpMeshHeader[MeshID] = (XBOXSkinMeshHeader*)(pCurrentSkin->mpRawData + Offset);

            // Now jump past the mesh header.
            Offset += sizeof( XBOXSkinMeshHeader );

            // If there are verts stored within this mesh, grab the pointer as the address after the SkinFileHeader.
            if( pCurrentSkin->mpMeshHeader[MeshID]->mNVerts )
            {
                pCurrentSkin->mpVerts[MeshID] = (XBOXSkinD3DVert*)(pCurrentSkin->mpRawData + Offset);
            }
            else
            {
                pCurrentSkin->mpVerts[MeshID] = (XBOXSkinD3DVert*)-1;
            }

            // Now offset by the size of the vertex and index data 
            Offset += pCurrentSkin->mpMeshHeader[MeshID]->mVertexDataSize;
            Offset += pCurrentSkin->mpMeshHeader[MeshID]->mIndexDataSize;
        }

        

        // Go through the entire skin to find duplicate points.    
        x_printf( "Combining vertices in %s\n", pCurrentFilename );
        for( MeshID = 0; MeshID < pCurrentSkin->mpFileHeader->mNMeshes; MeshID++ )
        {
            x_printf( "Mesh: %d", MeshID );

            for( VertID = 0; VertID < pCurrentSkin->mpMeshHeader[MeshID]->mNVerts; VertID++ )
            {
                if( VertID % 25 == 0 )
                    x_printf( "." );

                // Start at the top of the list.
                pCurrentList = &VList;
    
                // Grab the vertex to test against.
                pvTestPosition = &pCurrentSkin->mpVerts[MeshID][VertID].mPos;
                pvTestNormal   = &pCurrentSkin->mpVerts[MeshID][VertID].mNormal;

                // Loop through the list of points until something is found.
                while( TRUE )
                {
                    pTestNode = pCurrentList->mpHead;
    
                    // If there is a node there, test the stored vertex with the current vert.
                    if( pTestNode )
                    {
                        pArchivedSkin = (XBOXSkinInfo*)pTestNode->pSkin;
                        pvArchivedPosition = &pArchivedSkin->mpVerts[pTestNode->MeshID][pTestNode->VertID].mPos;
                        pvArchivedNormal = &pArchivedSkin->mpVerts[pTestNode->MeshID][pTestNode->VertID].mNormal;

                        NormalDot = pvTestNormal->Dot( *pvArchivedNormal );

                        // Test to see if this vertex is a duplicate.
                        if( (x_abs( pvTestPosition->X - pvArchivedPosition->X) < EPSILON) &&
                            (x_abs( pvTestPosition->Y - pvArchivedPosition->Y) < EPSILON) &&
                            (x_abs( pvTestPosition->Z - pvArchivedPosition->Z) < EPSILON) && 
                            NormalDot > 0.0f )
                        {
                            // Add the duplicate node to the current list, then break from the while loop
                            pCurrentList->AddNode( pCurrentSkin, MeshID, VertID );
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
                                VList.AttachList( pCurrentSkin, MeshID, VertID );
                                break;
                            }
                        }
                    }
                    // There are no nodes in this list, so just add this one.
                    else
                    {
                        pCurrentList->AddNode( pCurrentSkin, MeshID, VertID );
                        break;
                    }
                }
            }        
            x_printf( "\n" );
        }

       
        // Go onto the next skin, or break the loop.
        if( pCurrentSkin == &Skin1 )
        {
            pCurrentSkin     = &Skin2;
            pCurrentFilename = gSkinFilename2;
        }
        else
        if( pCurrentSkin == &Skin2 )
        {
            pCurrentSkin = NULL;
            pCurrentFilename = NULL;
        }
        else
        {
            ASSERT( 0 );
        }
    }


    // Now we have a linked list of linked lists where each 2nd generation list has a chain of verts that share the same
    // space.  Time to average these normals and shove them back into the correct place.
    //-------------------------------------------------------------------------------------------------------------------------------------
    pCurrentList = &VList;
    NList.KillList();

    while( pCurrentList )
    {
        // Find how many verts share this point, and add average any normals that are different.
        CommonVerts = 0;
        AvgNormal.Zero( );

        pTestNode = pCurrentList->mpHead;
        while( pTestNode )
        {
            CommonVerts++;

            pCurrentSkin = (XBOXSkinInfo*)(pTestNode->pSkin);

            AvgNormal.X += pCurrentSkin->mpVerts[pTestNode->MeshID][pTestNode->VertID].mNormal.X;
            AvgNormal.Y += pCurrentSkin->mpVerts[pTestNode->MeshID][pTestNode->VertID].mNormal.Y;
            AvgNormal.Z += pCurrentSkin->mpVerts[pTestNode->MeshID][pTestNode->VertID].mNormal.Z;

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
            pCurrentSkin = (XBOXSkinInfo*)(pTestNode->pSkin);
            pCurrentSkin->mpVerts[pTestNode->MeshID][pTestNode->VertID].mNormal.X = AvgNormal.X;
            pCurrentSkin->mpVerts[pTestNode->MeshID][pTestNode->VertID].mNormal.Y = AvgNormal.Y;
            pCurrentSkin->mpVerts[pTestNode->MeshID][pTestNode->VertID].mNormal.Z = AvgNormal.Z;
            pTestNode = pTestNode->pNext;
        }

        pCurrentList = pCurrentList->mpNextList;
    }


    // So the normals have been averaged, time to re-write both files.
    //=====================================================================================================================================
    pCurrentSkin = &Skin1;
    pCurrentFilename = gSkinFilename1;

    while( pCurrentSkin )
    {
        pCurrentSkin->mpFile = x_fopen( pCurrentFilename, "wb" );
        ASSERT( pCurrentSkin->mpFile );

        if( x_fwrite( pCurrentSkin->mpRawData, pCurrentSkin->mSizeOfFile, 1, pCurrentSkin->mpFile ) != 1 )
        {
            ASSERT( 0 );
        }

        x_fclose( pCurrentSkin->mpFile );


        // Go onto the next skin, or break the loop.
        if( pCurrentSkin == &Skin1 )
        {
            pCurrentSkin     = &Skin2;
            pCurrentFilename = gSkinFilename2;
        }
        else
        if( pCurrentSkin == &Skin2 )
        {
            pCurrentSkin = NULL;
            pCurrentFilename = NULL;
        }
        else
        {
            ASSERT( 0 );
        }
    }

    // Delete the vertex list pointers.
    delete[] Skin1.mpVerts;
    delete[] Skin1.mpMeshHeader;

    delete[] Skin2.mpVerts;
    delete[] Skin2.mpMeshHeader;

    // Free the data from the file, and close the file.
    delete[] Skin1.mpRawData;
    delete[] Skin2.mpRawData;
}


//=====================================================================================================================================
void BlendSkinsGC( void )
{
}


//=====================================================================================================================================
void BlendSkinsPS2( void )
{
}


//=====================================================================================================================================
static xbool IsInputFile( char* filename )
{
    s32 i;

    i = x_strlen( filename );
    i -= x_strlen( ".SKN" );

    if ( !x_stricmp( &filename[i], ".SKN" ) )
        return TRUE;

    return FALSE;
}


//=====================================================================================================================================
void OutputUsageMessage( void )
{
    x_printf( "Usage: SkinBlender [options] <file1.skn> <file2.skn>\n\n" );
    x_printf( "Version 1.0\n\n" );

    x_printf( "Options:\n" );
    x_printf( "  -PS2 .............. Target PlayStation 2\n" );
    x_printf( "  -GC ............... Target GameCube\n" );
    x_printf( "  -XBOX ............. Target XBOX\n" );
}


//=====================================================================================================================================
void PauseOnError( void )
{
    x_printf( "Press any key to continue...\n" );

    while ( !getch() )
    {
        ;
    }
}


//=====================================================================================================================================
void ProcessSwitches( int argc, char** argv )
{
    xbool Filename1Found = FALSE;
    xbool Filename2Found = FALSE;
    char* pCurrentArg;

    //skip past the executable name
    argc--;
    argv++;

    if( argc != 3 )
    {
        x_printf( "ERROR: Incorrect argument counts.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }

    // As long as there are arugments, look for the switches.
    while( argc )
    {
        pCurrentArg = argv[0];

        // check for a valid target
        if ( !x_stricmp( pCurrentArg, "-XBOX" ) )
        {
            gTarget = TARGET_XBOX;
        }
        else
        if ( !x_stricmp( pCurrentArg, "-GC" ) )
        {
            gTarget = TARGET_GAMECUBE;
        }
        else
        if ( !x_stricmp( pCurrentArg, "-PS2" ) )
        {
            gTarget = TARGET_XBOX;
        }
        else
        {
            // Test to see if this is an input file.
            if( IsInputFile( pCurrentArg ) )
            {
                if( Filename1Found == FALSE )
                {
                    x_strncpy( gSkinFilename1, pCurrentArg, SKIN_FILENAME_MAX );
                    Filename1Found = TRUE;
                }
                else
                if( Filename2Found == FALSE )
                {
                    x_strncpy( gSkinFilename2, pCurrentArg, SKIN_FILENAME_MAX );
                    Filename2Found = TRUE;
                }
            }
        }

        // Advance to the next argument.
        argc--;
        argv++;
    }

    if( gTarget == TARGET_UNKNOWN )
    {
        x_printf( "ERROR: Unknown target.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }
    else
    if( Filename1Found == FALSE || Filename2Found == FALSE )
    {
        x_printf( "ERROR: Input files are invalid.\n" );
        OutputUsageMessage();
        PauseOnError();
        exit( 0 );
    }
}


//=====================================================================================================================================
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




// Mike Skinner
/*
void AdjustSharedVertexNormals( void )
{
    s32             MeshID;
    s32             VertID;
    VertList        VList;
    VertList*       pCurrentList;
    VertNode*       pTestNode;

    XBOXRipSkinMesh*  pCurrentMesh;

    XBOXSkinD3DVert*  pCurrentVerts;
    s32               nCurrentMeshVerts;
    vector3*          pvTestPosition;
    vector3           vArchivedPosition;

    // Vertex normals are previously calculated by averaging the face normals for all faces that share a particular vertex,
    // in a single mesh.  Models which have interchangable parts, such as different hands, or legs, may contain
    // duplicates of a single vertex so each submesh appears to attach itself correctly to the rest of the model.
    //
    // The problem that occurrs is that each copy of this same vertex, has a different vertex normal since the vertex normals
    // are calculated using the average of the  normals for each face that touches that vertex in a single mesh.
    //
    // This function will try to find copies of vertices shared between different SubMeshes and blend the vertex normals so 
    // no matter which interchangeable part is used, the lighting will look correct because the vertex normals have been blended.
    for( MeshID = 0; MeshID < mNMeshes; MeshID++ )
    {
        pCurrentMesh      = &mpMesh[MeshID];
        pCurrentVerts     = pCurrentMesh->mpD3DVerts;
        nCurrentMeshVerts = pCurrentMesh->mNVerts;

        // For each of the meshes, go through each vertex in the mesh.
        for( VertID = 0; VertID < nCurrentMeshVerts; VertID++ )
        {
            // Start at the first list.
            pCurrentList = &VList;

            // Get the current position of the test vertex;
            pvTestPosition = &pCurrentVerts[VertID].mPos;

            while( TRUE )
            {
                // Using the current list, and first node.
                pTestNode = pCurrentList->mpHead;

                // If there is a node present, then this list contains a vertex position.
                if( pTestNode != NULL )
                {
                    vArchivedPosition = mpMesh[pTestNode->MeshID].mpD3DVerts[pTestNode->VertID].mPos;

                    // Test to see if this vertex is a duplicate.
                    if( (x_abs( pvTestPosition->X - vArchivedPosition.X) < EPSILON) &&
                        (x_abs( pvTestPosition->Y - vArchivedPosition.Y) < EPSILON) &&
                        (x_abs( pvTestPosition->Z - vArchivedPosition.Z) < EPSILON) )
                    {
                        // Its a duplicate, so add it to the current list.
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
                // This is the first vertex to be tested, so just add the point to the current list.
                else
                {
                    pCurrentList->AddNode( MeshID, VertID );
                    break;
                }
            }
        }
    }
}*/
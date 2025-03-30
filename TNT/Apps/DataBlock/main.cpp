////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "x_files.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES AND TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef char TFilePath[300];

struct SDataHeader
{
	s32  NFiles;
};

struct SFileInfo
{
	char FName[32];
	u32  Offset;
    u32  Size;
};


////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////

static char s_InputFilename[256]  = {'\0'};
static char s_OutputFilename[256] = {'\0'};

static SFileInfo* s_pFileInfo  = NULL;
static TFilePath* s_pFilepaths = NULL;
static s32        s_NFiles     = 0;


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

static void PrintHelp( void )
{
	printf( "\n" );

	printf( "    DATA BLOCK HELP\n" );
	printf( "    Usage: DataBlock.exe <switches> <Input.txt> <OutputFilename>\n\n" );
	printf( "    Input.txt should be a simple list of file pathnames, with\n" );
	printf( "    one path per line.\n" );
	printf( "    Swtiches:\n" );

	printf( "\n" );
}

//==========================================================================

static void ProcessCmdLine( int argc, char** argv )
{
	char* pArg;

	s_InputFilename[0] = '\0';
	s_OutputFilename[0] = '\0';

	//skip past exe name
	argc--;
	argv++;

	while( argc )
	{
		pArg = argv[0];

		if( s_InputFilename[0] == '\0' )
		{
			strcpy( s_InputFilename, pArg );
		}
		else if( s_OutputFilename[0] == '\0' )
		{
			strcpy( s_OutputFilename, pArg );
			break;
		}
		else
		{
			break;
		}


		// move to next argument
		argc--;
		argv++;
	}


	if( s_InputFilename[0] == '\0' )
	{
		printf( "ERROR no input filename specified.\n" );
		PrintHelp();
		exit(-1);
	}
	if( s_OutputFilename[0] == '\0' )
	{
		printf( "ERROR no output file specified.\n" );
		PrintHelp();
		exit(-1);
	}

}

//==========================================================================

static void ParseInputFile( void )
{
	FILE*     pFile;
	TFilePath LineText;
	s32       FileCount;
	s32       FileCountAllocated;
	s32       LineCount;

	LineCount = 1;
	FileCount = 0;
	FileCountAllocated = 0;

	// Using non-x_files stdio so I can use fgets()
	pFile = fopen( s_InputFilename, "r" );

	if( pFile == NULL )
	{
		printf( "ERROR couldn't open input file %s.\n", s_InputFilename );
		PrintHelp();
		exit(-1);
	}

	while( !feof( pFile ) )
	{
		// Read a file path
		char* pFilename = fgets( LineText, 290, pFile );

		// Check for errors
		if( pFilename != LineText )
		{
			if( feof( pFile ) )
			{
				break;
			}
			else if( ferror( pFile ) )
			{
				printf( "### ERROR: Error occurred parsing file, line #%ld\n", LineCount );
				continue;
			}
			else
			{
				printf( "### ERROR: Unknown Error parsing file, line #%ld\n", LineCount );
				continue;
			}
		}

		LineCount++;

		// Remove whitespace at the end of the line(like newline characters)
		s32 Length = strlen( LineText ) - 1;

		while( iswspace( LineText[Length] ) )
		{
			LineText[Length] = '\0';
			Length--;
			if( Length < 0 )
				break;
		}

		if( Length < 0 )
		{
			printf( "### ERROR: Line %d only has white space\n", LineCount );
			continue;
		}

		// Test if file exists
		FILE* pTestOpen = fopen( LineText, "rb" );
		if( pTestOpen == NULL )
		{
			printf( "### ERROR: File path \"%s\" not found, cannot add to export\n", LineText );
			continue;
		}
		else
		{
			fclose( pTestOpen );
		}

		// Add file path to list
		{
			if( FileCountAllocated <= FileCount )
			{
				FileCountAllocated += 256;
				s_pFilepaths = (TFilePath*)realloc( s_pFilepaths, FileCountAllocated * sizeof(TFilePath) );
				if( s_pFilepaths == NULL )
				{
					printf( "### ERROR: Memory allocation failed, LINE %ld\n", (s32)__LINE__ );
					fclose( pFile );
					exit(-1);
				}
			}

			strcpy( s_pFilepaths[FileCount], LineText );
			FileCount++;
		}

		pFilename = NULL;
		LineText[0] = '\0';
	}

	s_NFiles = FileCount;

	fclose( pFile );

	if( s_NFiles == 0 )
	{
		if( s_pFilepaths != NULL )
			free( s_pFilepaths );

		printf( "### ERROR: No file paths available for use.\n" );
		PrintHelp();
		exit(-1);
	}


	// Generate file name and size info
	s_pFileInfo = (SFileInfo*)malloc( s_NFiles * sizeof(SFileInfo) );

	if( s_pFileInfo == NULL )
	{
		printf( "### ERROR: Memory allocation failed, LINE %ld\n", (s32)__LINE__ );
		exit(-1);
	}

	s32 i;

	for( i = 0; i < s_NFiles; i++ )
	{
		char pFilenameOnly[300];

		x_splitpath( s_pFilepaths[i], NULL, NULL, pFilenameOnly, NULL );

		memset( s_pFileInfo[i].FName, 0, 32 );
		strncpy( s_pFileInfo[i].FName, pFilenameOnly, 31 );

		s_pFileInfo[i].Offset = (u32)((char*)s_pFilepaths[i]);
        s_pFileInfo[i].Size   = 0;
	}
}

//==========================================================================

static void WriteOutputFile( void )
{
	s32 i;

	// Create file for export
	FILE* pOutFile;

	pOutFile = fopen( s_OutputFilename, "wb" );

	if( pOutFile == NULL )
	{
		printf( "ERROR couldn't create file for write: %s.\n", s_OutputFilename );
		free( s_pFileInfo );
		exit(-1);
	}

	// Write out header information
	SDataHeader Header;

	Header.NFiles = s_NFiles;

	fwrite( &Header, sizeof(SDataHeader), 1, pOutFile );

	// Write out filename and size information
	fwrite( s_pFileInfo, sizeof(SFileInfo), s_NFiles, pOutFile );


	// Write out file data
	for( i = 0; i < s_NFiles; i++ )
	{
		char* pTFilePath = (char*)(s_pFileInfo[i].Offset);

		FILE* pFile = fopen( pTFilePath, "rb" );
		if( pFile == NULL )
		{
			printf( "### ERROR: File path \"%s\" not found, couldn't add to export file\n", pTFilePath );
			continue;
		}

		s32 FileSize = 0;

		fseek( pFile, 0, SEEK_END );
		FileSize = ftell( pFile );
		fseek( pFile, 0, SEEK_SET );

		void* pFileData = malloc( FileSize );

		if( pFileData == NULL )
		{
			printf( "### ERROR: Memory allocation failed, LINE %ld\n", (s32)__LINE__ );
			fclose( pFile );
			fclose( pOutFile );
			free( s_pFileInfo );
			exit(-1);
		}

		memset( pFileData, 0, FileSize );
		fread( pFileData, 1, FileSize, pFile );

		s32 FileOffset = ftell( pOutFile );
		s_pFileInfo[i].Offset = FileOffset;
        s_pFileInfo[i].Size   = FileSize;
		fwrite( pFileData, 1, FileSize, pOutFile );

		free( pFileData );

		fclose( pFile );
	}

	// Re-write file info since the offsets are now generated
	fseek( pOutFile, sizeof(SDataHeader), SEEK_SET );
	fwrite( s_pFileInfo, sizeof(SFileInfo), s_NFiles, pOutFile );


	free( s_pFileInfo );

	fclose( pOutFile );
}

//==========================================================================

static int FileCompare( const void* pF1, const void* pF2 )
{
	return stricmp( ((SFileInfo*)pF1)->FName, ((SFileInfo*)pF2)->FName );
}

static void SortFileList( void )
{
	qsort( s_pFileInfo, s_NFiles, sizeof(SFileInfo), FileCompare );
}

//==========================================================================

int main( int argc, char* argv[] )
{
	ProcessCmdLine( argc, argv );

	printf( "\nParsing input file.......\n" );
	ParseInputFile();

	printf( "\nSorting file list alphabetically......\n" );
	SortFileList();

	printf( "Writing output file......\n" );
	WriteOutputFile();

	printf( "Finished.\n\n" );

	if( s_pFilepaths != NULL )
		free( s_pFilepaths );

	return 0;
}

//==========================================================================

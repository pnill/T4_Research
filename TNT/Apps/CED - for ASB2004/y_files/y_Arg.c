///////////////////////////////////////////////////////////////////////////
//
//  Y_ARG.C
//
///////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include "x_memory.h"
#include "x_debug.h"
#include "x_stdio.h"
#include "y_arg.h"

arg_entry* ArgEntry;
s32        NEntriesAllocated;
s32        NEntries;

///////////////////////////////////////////////////////////////////////////
// PROTOTYPES
///////////////////////////////////////////////////////////////////////////
void OpenArgSet     (void);
void AddArg         (s32 Type, f32 Float, char* Str);
void ArgifyFile     (char* FileName);
void ParseArg       (char* S);

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void OpenArgSet (void)
{
    NEntriesAllocated = 100;
    NEntries          = 0;
    ArgEntry = (arg_entry*)x_malloc(sizeof(arg_entry)*100);
    ASSERT(ArgEntry);
}

///////////////////////////////////////////////////////////////////////////

void AddArg (s32 Type, f32 Float, char* Str)
{
    /////////////////////////////////////////////////
    // RESIZE ARRAY OF ARGS
    /////////////////////////////////////////////////
    if (NEntries == NEntriesAllocated)
    {
        NEntriesAllocated += 100;
        ArgEntry = (arg_entry*)x_realloc(ArgEntry,sizeof(arg_entry)*NEntriesAllocated);
        ASSERT(ArgEntry);
    }

    /////////////////////////////////////////////////
    // ADD ARGUMENT
    /////////////////////////////////////////////////
    ArgEntry[NEntries].Type     = Type;
    ArgEntry[NEntries].Number   = Float;
    x_strcpy(ArgEntry[NEntries].Str,Str);
    NEntries++;
}

///////////////////////////////////////////////////////////////////////////

void ExpandEnvVar(char* S)
{
    char EnvVarBuff[X_MAX_PATH];
    char EnvVarName[X_MAX_PATH];
    char* EnvVarVal;
    s32 i,b,n;
    xbool EnvVarFound=FALSE;

    i=0;
    b=0;
    while (S[i] != 0)
    {
        if (S[i]=='%')
        {
            EnvVarFound = TRUE;

            // Load variable's name into EnvVarName
            n=0;
            i++;
            while ((S[i]!=0) && (S[i]!='%'))
            {
                EnvVarName[n] = S[i];
                n++;
                i++;
            }
            EnvVarName[n] = 0;
            if (S[i]=='%') i++;

            // Get ptr to variable's value
            EnvVarVal = getenv(EnvVarName);
            if (EnvVarVal!=NULL)
            {
                // Load value into buffer
                n=0;
                while (EnvVarVal[n] != 0)
                {
                    EnvVarBuff[b] = EnvVarVal[n];
                    b++;
                    n++;
                }
            }
        }
        else
        {
            EnvVarBuff[b] = S[i];
            b++;
            i++;
        }
    }
    EnvVarBuff[b] = 0;

    if (EnvVarFound)
    {
        x_strcpy(S,EnvVarBuff);
    }
}

///////////////////////////////////////////////////////////////////////////

void ExpandWildcards(char* S)
{
    s32 i;
    struct _finddata_t fileset;
    s32 hFile;
    char Drive[X_MAX_PATH];
    char Dir[X_MAX_PATH];
    char Path[X_MAX_PATH];

    // Pull out path for later
    x_splitpath( S, Drive, Dir, NULL, NULL );

    // Check if any wildcards are present
    i=0;
    while (S[i] != 0)
    {
        if ((S[i]=='*') || (S[i]=='?')) break;
        i++;
    }
    if (S[i]==0) return;

    
    // Expand wildcard and add as args
    if ((hFile = _findfirst( S, &fileset )) != -1)
    {
        x_makepath(Path,Drive,Dir,fileset.name,NULL);
        AddArg( ARG_STRING, 0.0f, Path );

        while( _findnext( hFile, &fileset ) == 0 )
        {
            x_makepath(Path,Drive,Dir,fileset.name,NULL);
            AddArg( ARG_STRING, 0.0f, Path );
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void ParseArg (char* S)
{
    s32     i;

    ExpandEnvVar(S);

    /////////////////////////////////////////////////
    // RECURSE TO ARGIFY A FILE
    /////////////////////////////////////////////////
    if ((S[0]=='@') || (S[0]=='#'))
    {
        ArgifyFile(&S[1]);
        return;
    }

    /////////////////////////////////////////////////
    // CHECK IF S IS A SWITCH
    /////////////////////////////////////////////////
    if ((S[0] == '-') && ((S[1]<'0') || (S[1]>'9')))
    {
        AddArg (ARG_SWITCH, 0.0f, &S[1]);
        return;
    }

    /////////////////////////////////////////////////
    // DETERMINE IF IT IS STRING OR NUMBER
    /////////////////////////////////////////////////
    if ((S[0]=='-') || (S[0]=='+') || ((S[0]>='0')&&(S[0]<='9')))
    {
        f32 F;
        F = (f32)atof(S);
        AddArg( ARG_NUMBER, F, S );
        return;
    }
    else
    {
        xbool HasWildcard = FALSE;

        /////////////////////////////////////////////
        // DETERMINE IF IT IS A WILDCARD 
        /////////////////////////////////////////////
        i=0;
        while (S[i] != 0)
        {
            if ((S[i]=='*') || (S[i]=='?')) break;
            i++;
        }

        // Process wildcard if necessary
        if (S[i]==0)
            AddArg( ARG_STRING, 0.0f, S );
        else
            ExpandWildcards(S);
    }
}

///////////////////////////////////////////////////////////////////////////

void ArgifyFile( char* FileName )
{
    char        S[X_MAX_PATH];
    X_FILE*     fp;
    s32         FileSize;
    char*       Buffer;
    s32         i,c;

    ///////////////////////////////////////////////
    // Open the file
    ///////////////////////////////////////////////
    fp = x_fopen(FileName,"rb");
    ASSERTS(fp,fs("Could not open <%s>",FileName));

    ///////////////////////////////////////////////
    // Find how large the file is
    ///////////////////////////////////////////////
    x_fseek (fp, 0, SEEK_END) ;
    FileSize = x_ftell (fp) ;
    x_fseek (fp, 0, SEEK_SET) ;

    ///////////////////////////////////////////////
    // Allocate a buffer to hold the file
    ///////////////////////////////////////////////
    Buffer = (char*)x_malloc(FileSize+1) ;
    ASSERT(Buffer);

    ///////////////////////////////////////////////
    // Load the entire file into the buffer
    ///////////////////////////////////////////////
    Buffer[FileSize] = 0;
    x_fread( Buffer, FileSize, 1, fp );

    ///////////////////////////////////////////////
    // Close the input file
    ///////////////////////////////////////////////
    x_fclose (fp) ;

    c = 0;
    while (c < FileSize)
    {
        // skip whitespace
        while ((c < FileSize) && (Buffer[c] <= 32)) c++;
        if (c >= FileSize) break;

        // check for a quote
        if (Buffer[c] == '"')
        {
            i=0; c++;
            while ((c < FileSize) && (i<X_MAX_PATH-1) && (Buffer[c] != '"'))
            {
                S[i] = Buffer[c];  c++;  i++;
            }
            c++;
            ASSERT(i<X_MAX_PATH-1);
        }
        else
        // read until whitespace
        {
            i=0;
            while ((c < FileSize) && (Buffer[c] > 32) && (i<X_MAX_PATH-1))
            {
                S[i] = Buffer[c];  c++;  i++;
            }
            ASSERT(i<X_MAX_PATH-1);
        }

        S[i] =0;
        ParseArg(S);
    }

    x_free(Buffer);
}

///////////////////////////////////////////////////////////////////////////

arg_entry* ARG_ExpandCommandLine( s32 argc, char** argv, s32* NArgs )
{
    s32 i;
    OpenArgSet();
    for (i=0; i<argc; i++)
        ParseArg(argv[i]);
    *NArgs = NEntries;
    return ArgEntry;
}

///////////////////////////////////////////////////////////////////////////

arg_entry* ARG_ExpandString( char* Str, s32* NArgs )
{
    char Buff[512];
    OpenArgSet();
    x_strcpy(Buff,Str);
    ParseArg(Buff);
    *NArgs = NEntries;
    return ArgEntry;
}

///////////////////////////////////////////////////////////////////////////

arg_entry* ARG_ExpandFile       ( char* FileName, s32* NArgs )
{
    OpenArgSet();
    ArgifyFile(FileName);
    *NArgs = NEntries;
    return ArgEntry;
}

///////////////////////////////////////////////////////////////////////////


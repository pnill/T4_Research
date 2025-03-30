///////////////////////////////////////////////////////////////////////////
//
//  Tokenizer.cpp
//
///////////////////////////////////////////////////////////////////////////

#include "Tokenizer.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"
#include "x_time.hpp"

static char CommentChar  = ';';
static char Delimiters[] = ",[]()";

///////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
///////////////////////////////////////////////////////////////////////////

s32  IsDelimiter( char c )
{
    s32 i=0;
    while (Delimiters[i] != 0)
    {
        if (c == Delimiters[i]) return c;
        i++;
    } 
    return 0;
}

///////////////////////////////////////////////////////////////////////////

void TKN_Clear( tokenizer* Tokenizer )
{
    ASSERT( Tokenizer );
    Tokenizer->FileBuffer       = NULL;
    Tokenizer->FileSize         = 0;
    Tokenizer->FilePos          = 0;
    Tokenizer->TokenType        = TOKEN_NONE;
    Tokenizer->String[0]        = 0;
    Tokenizer->Float            = (float)0.0;
    Tokenizer->Int              = 0;
    Tokenizer->NumberType       = NUMBER_FLOAT;
    Tokenizer->FileLineNumber   = 0;
}

///////////////////////////////////////////////////////////////////////////

xbool TKN_ReadFile( tokenizer* Tokenizer, const char* Path )
{
    X_FILE* fh;

    ASSERT( Tokenizer );

    // Clear any garbage info
	TKN_CloseFile( Tokenizer );
    TKN_Clear( Tokenizer );

    // Open requested file and handle open-error
    fh = x_fopen( Path, "rb" );
    if( fh == NULL )
    {
        return( FALSE );
    }

    // Find how large the file is
    x_fseek( fh, 0, X_SEEK_END );
    Tokenizer->FileSize = x_ftell(fh);
    x_fseek( fh, 0, X_SEEK_SET );

    // Allocate a buffer to hold the file
    Tokenizer->FileBuffer = (char*)x_malloc(Tokenizer->FileSize+1);
    ASSERTS( Tokenizer->FileBuffer, "Could not allocate memory" );

    // Load the entire file into the buffer
    Tokenizer->FileBuffer[Tokenizer->FileSize] = 0;
    if( !x_fread( Tokenizer->FileBuffer, 1, Tokenizer->FileSize, fh ) )
        ASSERTS( FALSE, "Could Not Read File" );
    
    // Close the input file
    x_fclose( fh );

    return( TRUE );
}

///////////////////////////////////////////////////////////////////////////

void TKN_CloseFile ( tokenizer* Tokenizer )
{
    ASSERT(Tokenizer);
    if (Tokenizer->FileBuffer) x_free(Tokenizer->FileBuffer);
    TKN_Clear(Tokenizer);
}

///////////////////////////////////////////////////////////////////////////

void TKN_DumpFile  ( tokenizer* Tokenizer )
{
    int i;
    ASSERT( Tokenizer );
    for( i=0; i<Tokenizer->FileSize; i++ )
    x_printf( "%1c", Tokenizer->FileBuffer[i] );
}

///////////////////////////////////////////////////////////////////////////

void TKN_ShowFile  ( tokenizer* Tokenizer )
{
    ASSERT(Tokenizer);
    TKN_Rewind(Tokenizer);
    TKN_ReadToken(Tokenizer);
    while( Tokenizer->TokenType != TOKEN_EOF )
    {
        if( Tokenizer->EOLNs > 0 )  
            x_printf("----------------------- LINE [%4d] ---------------------\n",
                    Tokenizer->LineNumber);

        switch( Tokenizer->TokenType )
        {
            case TOKEN_DELIMITER: x_printf("D:   %c \n",Tokenizer->Delimiter); break;
            case TOKEN_SYMBOL:    x_printf("S:   %s \n",Tokenizer->String); break;
            case TOKEN_STRING:    x_printf("Q:  '%s'\n",Tokenizer->String); break;
            case TOKEN_NUMBER:    if (Tokenizer->NumberType == NUMBER_FLOAT)
                                    x_printf("N:   %f\n",Tokenizer->Float); 
                                  else
                                    x_printf("N:   %1d\n",Tokenizer->Int); 
                                  break;
            default:    ASSERT(FALSE && "Bad token");
        }

        TKN_ReadToken(Tokenizer);
    }
}

///////////////////////////////////////////////////////////////////////////

void TKN_Rewind    ( tokenizer* Tokenizer )
{
    ASSERT(Tokenizer);
    Tokenizer->FilePos          = 0;
    Tokenizer->FileLineNumber   = 0;
    Tokenizer->TokenType        = TOKEN_NONE;
    Tokenizer->Delimiter        = ' ';
    Tokenizer->Float            = 0.0f;
    Tokenizer->Int              = 0;
    Tokenizer->NumberType       = NUMBER_FLOAT;
    Tokenizer->String[0]        = 0;
}

///////////////////////////////////////////////////////////////////////////

void TKN_ReadTokens ( tokenizer* Tokenizer, s32 NumTokens )
{
    s32 i;
    ASSERT(Tokenizer);
    for (i=0; i<NumTokens; i++)
        TKN_ReadToken(Tokenizer);
}

///////////////////////////////////////////////////////////////////////////

#define isspace(c)  ( ((c) == 0x20) || (((c) >= 0x09) && ((c) <= 0x0D)) )

s32  TKN_ReadIntToken   ( tokenizer* Tokenizer )
{
    ASSERT(Tokenizer);
    Tokenizer->EOLNs = 0;

    Tokenizer->TokenType    = TOKEN_NONE;
    Tokenizer->Delimiter    = ' ';
    Tokenizer->Float        = 0.0f;
    Tokenizer->Int          = 0;
    Tokenizer->NumberType   = NUMBER_INT;
    Tokenizer->String[0]    = 0;

    //////////////////////////////////////////////////////////////////////
    // Check if at end of file
    if (Tokenizer->FilePos >= Tokenizer->FileSize)
    {
        Tokenizer->TokenType = TOKEN_EOF;
        return Tokenizer->TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Skip whitespace and count EOLNs
    while( isspace(Tokenizer->FileBuffer[Tokenizer->FilePos]) && 
           (Tokenizer->FilePos < Tokenizer->FileSize) )
    {
        if (Tokenizer->FileBuffer[Tokenizer->FilePos]=='\n') 
        {
            Tokenizer->EOLNs++;
            Tokenizer->FileLineNumber++;
        }
        Tokenizer->FilePos++;
    }

    if( Tokenizer->FilePos==Tokenizer->FileSize )
    {
        Tokenizer->TokenType = TOKEN_EOF;
        Tokenizer->LineNumber = Tokenizer->FileLineNumber;
        return( Tokenizer->TokenType );
    }

    //////////////////////////////////////////////////////////////////
    // Copy token into user's buffer
    {
        s32 i=0;
        while (!isspace(Tokenizer->FileBuffer[Tokenizer->FilePos]))
        {
            Tokenizer->String[i] = (char)(Tokenizer->FileBuffer[Tokenizer->FilePos]);
            i++;
            Tokenizer->FilePos++;
        }
        Tokenizer->String[i]=0;
    }

    //////////////////////////////////////////////////////////////////
    // Let 'C' do the rough work for doubles
    Tokenizer->TokenType     = TOKEN_NUMBER;
    Tokenizer->NumberType    = NUMBER_FLOAT;
    Tokenizer->Int           = x_atoi(Tokenizer->String);
    Tokenizer->Float         = (f32)Tokenizer->Int;
    Tokenizer->LineNumber = Tokenizer->FileLineNumber;
    return Tokenizer->TokenType;
}

///////////////////////////////////////////////////////////////////////////

s32  TKN_ReadFloatToken ( tokenizer* Tokenizer )
{
    ASSERT(Tokenizer);
    Tokenizer->EOLNs = 0;

    Tokenizer->TokenType    = TOKEN_NONE;
    Tokenizer->Delimiter    = ' ';
    Tokenizer->Float        = 0.0f;
    Tokenizer->Int          = 0;
    Tokenizer->NumberType   = NUMBER_FLOAT;
    Tokenizer->String[0]    = 0;

    //////////////////////////////////////////////////////////////////////
    // Check if at end of file
    if (Tokenizer->FilePos >= Tokenizer->FileSize)
    {
        Tokenizer->TokenType = TOKEN_EOF;
        return Tokenizer->TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Skip whitespace and count EOLNs
    while( isspace(Tokenizer->FileBuffer[Tokenizer->FilePos]) && 
           (Tokenizer->FilePos < Tokenizer->FileSize) )
    {
        if (Tokenizer->FileBuffer[Tokenizer->FilePos]=='\n') 
        {
            Tokenizer->EOLNs++;
            Tokenizer->FileLineNumber++;
        }
        Tokenizer->FilePos++;
    }

    if( Tokenizer->FilePos==Tokenizer->FileSize )
    {
        Tokenizer->TokenType = TOKEN_EOF;
        Tokenizer->LineNumber = Tokenizer->FileLineNumber;
        return( Tokenizer->TokenType );
    }

    //////////////////////////////////////////////////////////////////
    // Copy token into user's buffer
    {
        s32 i=0;
        while (!isspace(Tokenizer->FileBuffer[Tokenizer->FilePos]))
        {
            Tokenizer->String[i] = (char)(Tokenizer->FileBuffer[Tokenizer->FilePos]);
            i++;
            Tokenizer->FilePos++;
        }
        Tokenizer->String[i]=0;
    }

    //////////////////////////////////////////////////////////////////
    // Let 'C' do the rough work for doubles
    Tokenizer->TokenType     = TOKEN_NUMBER;
    Tokenizer->NumberType    = NUMBER_FLOAT;
    Tokenizer->Float         = x_atof(Tokenizer->String);
    Tokenizer->Int           = (s32)Tokenizer->Float;
    Tokenizer->LineNumber = Tokenizer->FileLineNumber;
    return Tokenizer->TokenType;
}

///////////////////////////////////////////////////////////////////////////


s32  TKN_ReadToken ( tokenizer* Tokenizer )
{
    s32     i;
    char    ch;

    ASSERT(Tokenizer);
    Tokenizer->EOLNs = 0;

    Tokenizer->TokenType    = TOKEN_NONE;
    Tokenizer->Delimiter    = ' ';
    Tokenizer->Float        = 0.0f;
    Tokenizer->Int          = 0;
    Tokenizer->NumberType   = NUMBER_FLOAT;
    Tokenizer->StringOrig[0]= 0;
    Tokenizer->String[0]    = 0;

    //////////////////////////////////////////////////////////////////////
    // Check if at end of file
    //////////////////////////////////////////////////////////////////////
    if (Tokenizer->FilePos >= Tokenizer->FileSize)
    {
        Tokenizer->TokenType = TOKEN_EOF;
        return Tokenizer->TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Skip whitespace and count EOLNs
    //////////////////////////////////////////////////////////////////////
    while ( (Tokenizer->FileBuffer[Tokenizer->FilePos]<=32) &&
            (Tokenizer->FilePos < Tokenizer->FileSize))
    {
        if (Tokenizer->FileBuffer[Tokenizer->FilePos]=='\n') 
        {
            Tokenizer->EOLNs++;
            Tokenizer->FileLineNumber++;
        }
        Tokenizer->FilePos++;
    }

    if (Tokenizer->FilePos==Tokenizer->FileSize)
    {
        Tokenizer->TokenType = TOKEN_EOF;
        Tokenizer->LineNumber = Tokenizer->FileLineNumber;
        return Tokenizer->TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Check if it's a delimiter
    //////////////////////////////////////////////////////////////////////
    ch = Tokenizer->FileBuffer[Tokenizer->FilePos];
    if ((ch==',') || (ch=='[') || (ch==']') || (ch=='(') || (ch==')'))
    {
        Tokenizer->FilePos++;
        Tokenizer->TokenType      = TOKEN_DELIMITER;
        Tokenizer->LineNumber     = Tokenizer->FileLineNumber;
        Tokenizer->Delimiter      = ch;
        Tokenizer->StringOrig[0]  = ch;
        Tokenizer->String[0]      = ch;
        Tokenizer->StringOrig[1]  = 0;
        Tokenizer->String[1]      = 0;

        return Tokenizer->TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Decide if it is a number
    //////////////////////////////////////////////////////////////////////
    ch = Tokenizer->FileBuffer[Tokenizer->FilePos];
    if (((ch>='0') && (ch<='9')) || (ch=='-') || (ch=='+'))
    {
        
        // Copy number into temp buffer
        i=0;
        ch = Tokenizer->FileBuffer[Tokenizer->FilePos];
        do 
        {
            Tokenizer->String[i] = ch;
            Tokenizer->FilePos++;
            i++;
            ch = Tokenizer->FileBuffer[Tokenizer->FilePos];
            
        } while (((ch>='0') && (ch<='9'))|| 
                  (ch=='-') || (ch=='+') ||
                  (ch=='E') || (ch=='e') || (ch=='.') ||
                  (ch=='#') || (ch=='Q') || (ch=='N') ||
                  (ch=='B') || (ch=='I') || (ch=='F') ||
                  (ch=='A'));

        // Generate float version
        Tokenizer->String[i] = 0;
        Tokenizer->Float     = x_atof( Tokenizer->String );
        Tokenizer->Int       = (s32)Tokenizer->Float;

        // Decide on token type
        if (Tokenizer->Float == (f32)Tokenizer->Int)
        {
            Tokenizer->TokenType  = TOKEN_NUMBER;
            Tokenizer->NumberType = NUMBER_INT;
        }
        else
        {
            Tokenizer->TokenType     = TOKEN_NUMBER;
            Tokenizer->NumberType    = NUMBER_FLOAT;
        }

        // return values
        Tokenizer->LineNumber = Tokenizer->FileLineNumber;
        return Tokenizer->TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Check if this is a line comment
    //////////////////////////////////////////////////////////////////////
    if ((Tokenizer->FileBuffer[Tokenizer->FilePos] == CommentChar)||
        (Tokenizer->FileBuffer[Tokenizer->FilePos] == '#'))
//    if (Tokenizer->FileBuffer[Tokenizer->FilePos] == CommentChar)
    {
        while (Tokenizer->FileBuffer[Tokenizer->FilePos]!='\n' && 
               (Tokenizer->FilePos < Tokenizer->FileSize))
        {
            Tokenizer->FilePos++;
        }
        if (Tokenizer->FileBuffer[Tokenizer->FilePos]=='\n') 
        {
            Tokenizer->EOLNs++;
            Tokenizer->FileLineNumber++;
            Tokenizer->FilePos++;
        }
        return TKN_ReadToken(Tokenizer);
    }

    //////////////////////////////////////////////////////////////////////
    // Check if first character is a quote
    //////////////////////////////////////////////////////////////////////
    if (Tokenizer->FileBuffer[Tokenizer->FilePos] == '"')
    {
        Tokenizer->FilePos++;        
        i=0;
        while (Tokenizer->FileBuffer[Tokenizer->FilePos]!='"')
        {
            // Check for illegal ending of a string
            ASSERT((Tokenizer->FilePos < Tokenizer->FileSize) && "EOF in quote");
            ASSERT((i<TOKEN_STRING_SIZE-1) && "Quote too long");
            ASSERT((Tokenizer->FileBuffer[Tokenizer->FilePos]!='\n') && "EOLN in quote");
            
            Tokenizer->StringOrig[i] = Tokenizer->FileBuffer[Tokenizer->FilePos];
            Tokenizer->String[i]     = (char)x_toupper(Tokenizer->FileBuffer[Tokenizer->FilePos]);
            i++;
            Tokenizer->FilePos++;
        }

        Tokenizer->FilePos++;
        Tokenizer->StringOrig[i]  = 0;
        Tokenizer->String[i]  = 0;
        Tokenizer->TokenType  = TOKEN_STRING;
        Tokenizer->LineNumber = Tokenizer->FileLineNumber;

        return Tokenizer->TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // It's a raw symbol
    //////////////////////////////////////////////////////////////////////
    {
        i=0;
        while ((Tokenizer->FilePos < Tokenizer->FileSize) && 
               (i<TOKEN_STRING_SIZE-1) && 
               !isspace(Tokenizer->FileBuffer[Tokenizer->FilePos]) &&
               !IsDelimiter(Tokenizer->FileBuffer[Tokenizer->FilePos]))
        {
            Tokenizer->StringOrig[i] = (char)Tokenizer->FileBuffer[Tokenizer->FilePos];
            Tokenizer->String[i]     = (char)x_toupper(Tokenizer->FileBuffer[Tokenizer->FilePos]);
            i++;
            Tokenizer->FilePos++;
        }
        Tokenizer->StringOrig[i]=0;
        Tokenizer->String[i]=0;

        Tokenizer->TokenType  = TOKEN_SYMBOL;
        Tokenizer->LineNumber = Tokenizer->FileLineNumber;

        return Tokenizer->TokenType;
    }

    Tokenizer->LineNumber = Tokenizer->FileLineNumber;
    return Tokenizer->TokenType;
}

///////////////////////////////////////////////////////////////////////////

xbool  TKN_FindToken ( tokenizer* Tokenizer, char* Token )
{
    char Target[128];
    ASSERT(Tokenizer);
    x_strcpy(Target,Token);
    x_strtoupper(Target);

    TKN_ReadToken(Tokenizer);
    while (Tokenizer->TokenType != TOKEN_EOF)
    {
        if (x_strcmp(Tokenizer->String,Target)==0) return TRUE;
        TKN_ReadToken(Tokenizer);
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////



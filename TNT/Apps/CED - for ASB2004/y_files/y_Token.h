///////////////////////////////////////////////////////////////////////////
//
//  y_TOKEN.H
//
//  y_Token loads an entire text file into memory and then allows the user
//  to view the file as a list of tokens rather than characters.  This is
//  a very simple interface, just enough to make parsing ASCII files easier.
//
//
//  Recognizes ';' as a the beginning of a line comment.
//  Any characters inside of double quotes "hello world" is a string.
//  Numbers without a decimal point are integers.
//  Numbers with a decimal point are floats.
//  Characters ',[](){}<>' are delimiters.
//  Any other contiguous characters become a symbol .
//
//  Add Current line number
//
///////////////////////////////////////////////////////////////////////////

#ifndef Y_TOKEN_H
#define Y_TOKEN_H

///////////////////////////////////////////////////////////////////////////
// C++ READY
///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif


///////////////////////////////////////////////////////////////////////////
//  INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_types.h"

///////////////////////////////////////////////////////////////////////////
//  DEFINES
///////////////////////////////////////////////////////////////////////////

#define TOKEN_NONE              0
#define TOKEN_DELIMITER         1
#define TOKEN_SYMBOL            2
#define TOKEN_STRING            3
#define TOKEN_NUMBER            4
#define TOKEN_EOF               5
#define NUMBER_FLOAT            0
#define NUMBER_INT              1
#define TOKEN_STRING_SIZE       512

///////////////////////////////////////////////////////////////////////////
//  STRUCTURES
///////////////////////////////////////////////////////////////////////////

typedef struct
{
    // PRIVATE
    s32     FileSize;
    char*   FileBuffer;
    s32     FilePos;
    s32     FileLineNumber;

    // PUBLIC
    s32     TokenType;                      // Type of token found
    char    Delimiter;                      // Char if token is delimiter
    char    StringOrig[TOKEN_STRING_SIZE+1];// Raw string
    char    String[TOKEN_STRING_SIZE+1];    // String for symbol,string,number
                                            // always uppercase
    f64     Float;                          // Float version of number token
    s32     Int;                            // Int version of number token
    s32     NumberType;                     // Whether number was float or int
    s32     LineNumber;                     // Current line number
    s32     EOLNs;                          // Lines crossed when seeking token
   
} tokenizer;

///////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
///////////////////////////////////////////////////////////////////////////

void TKN_Clear          ( tokenizer* Text );    
err  TKN_ReadFile       ( tokenizer* Text, const char* Path );    
void TKN_CloseFile      ( tokenizer* Text );
void TKN_DumpFile       ( tokenizer* Text );    
void TKN_ShowFile       ( tokenizer* Text );
void TKN_Rewind         ( tokenizer* Text );    
s32  TKN_ReadToken      ( tokenizer* Text );    
s32  TKN_ReadFloatToken ( tokenizer* Text );
void TKN_ReadTokens     ( tokenizer* Text, s32 NumTokens );
xbool TKN_FindToken     ( tokenizer* Text, char* Token );    

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// C++ READY
///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif


#endif

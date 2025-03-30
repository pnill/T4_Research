#ifndef SOURCEPARSER_H
#define SOURCEPARSER_H


#include <stdio.h>

typedef enum _EParserToken
{
    TOKEN_EOF = 0,
    TOKEN_COMMENT_C_STYLE,
    TOKEN_COMMENT_CPP_STYLE,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_KEYWORD,
    TOKEN_OPEN_CURLY,
    TOKEN_CLOSED_CURLY,

    TOKEN_WILDCARD,
    TOKEN_MISMATCH,

    TOKEN_NUM_TOKENS
} EParserToken;

typedef union _UValue
{
    float fValue;
    int   iValue;
    char* sValue;
} UValue;

#ifdef __cplusplus
extern "C" {
#endif 

void            ParserPrepareForReading(FILE *file, const char *filename);
void            ParserFinishReading(FILE *file);
int             ParserFindKeywords( const char **keywordArray, int numKeywords, int SearchComments );
EParserToken    ParserGrabToken(EParserToken *t, UValue *); //Returns token found... if failed *t==TOKEN_MISMATCH

void            ParserResetInput();

int             ParserGetLineNumber();
const char*     ParserGetFileName();

#ifdef __cplusplus
}
#endif 

#endif
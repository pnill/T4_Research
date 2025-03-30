%{
    /* 
	
	   This lex script describes how to parse text out of a file.

	   Run flex passing in this script to generate the c file.      
	   A Custom Build step in the DevStudio project does this.

     */

	#include "SourceParser.h"
	#include <stdlib.h>
	#include <string.h>
	#include <assert.h>

	#ifdef yywrap
	#undef yywrap
	#endif 

	#define MAX_DEPTH 1
	#define MAX_FILENAME_LGTH 256

	static char*			stringVal;			
	static float			floatVal;
	static int				integerVal;
    static int				lineNumber[MAX_DEPTH];
	static YY_BUFFER_STATE	fileStack[MAX_DEPTH];
	static char				fileNameStack[MAX_DEPTH][MAX_FILENAME_LGTH];
	static int				fileDepth = -1;
	static int				withinComment[MAX_DEPTH] = {0};
	static int				retCode[2] = {0, TOKEN_COMMENT_C_STYLE};

%}

%%
  
\n                         { lineNumber[fileDepth]++; }

\/\/[^\n]*				   { stringVal = (char*)yytext;		return TOKEN_COMMENT_CPP_STYLE; }

\/\*[^\n]*\*\/			   { stringVal = (char*)yytext;     return TOKEN_COMMENT_C_STYLE; }

\/\*[^\n]*				   { withinComment[fileDepth] = 1; stringVal = (char*)yytext; return TOKEN_COMMENT_C_STYLE;}

.*\*\/					   { withinComment[fileDepth] = 0; stringVal = (char*)yytext; return TOKEN_COMMENT_C_STYLE;}

[\t ]+                     /* ignore whitespace */ ;

\"[^"]*\"                  {	stringVal = (char *)yytext;
								retCode[0] = TOKEN_STRING;
								return retCode[withinComment[fileDepth]];
						   }

-?(-*[0-9]*\.[0-9]+)       {	if (!withinComment[fileDepth]) 
								{ 
									floatVal = (float)atof(yytext);
									return TOKEN_FLOAT;
								}	
						   }

-?(-*[0-9]+)               {	if (!withinComment[fileDepth])
								{
									integerVal= (int)atoi(yytext);
									return TOKEN_INTEGER;
								}
						   }

[a-zA-Z0-9_]+              {	stringVal = (char *)yytext;
								retCode[0] = TOKEN_KEYWORD;
								return retCode[withinComment[fileDepth]];
						   }

\{                         { return TOKEN_OPEN_CURLY;   } 
\}                         { return TOKEN_CLOSED_CURLY; } 

.                          /* ignore everything else */ ;

%%

EParserToken ParserGrabToken(EParserToken* token, UValue* val)
{
	int result;

	result = yylex();

	if (result != *token)
	{
		if (*token == TOKEN_WILDCARD)
		{
		    *token = (EParserToken) result;
		}
		else
		{
			*token = TOKEN_MISMATCH;
			return (EParserToken) result;
		}
	}

	switch (result)
	{
		case TOKEN_COMMENT_C_STYLE:
		case TOKEN_COMMENT_CPP_STYLE:
		case TOKEN_KEYWORD:
		case TOKEN_STRING:
			val->sValue = stringVal;
			break;

		case TOKEN_INTEGER:
			val->iValue = integerVal;
			break;

		case TOKEN_FLOAT:
			val->fValue = floatVal;
			break;

		default:
			break;
	};

	return (EParserToken) result;
}

int ParserFindKeywords( const char **keyword, int numKeywords, int SearchComments )
{
	int result;
	int keywordIndex;
	
	if (numKeywords <= 0)
	    return -1;

	while (1)
	{
		result = yylex();

		switch (result)
		{
			case TOKEN_KEYWORD:
			    keywordIndex = 0;

				do
				{
					if (stringVal[0] == keyword[keywordIndex][0]) //Quick test to avoid function call overhead
					{
						if (strcmp(stringVal, keyword[keywordIndex]) == 0)
						{
						    return keywordIndex;
						}
					}
					++keywordIndex;
			    } while (keywordIndex<numKeywords);

				break;

			case TOKEN_EOF:
				return -1;

			case TOKEN_COMMENT_C_STYLE:
			case TOKEN_COMMENT_CPP_STYLE:
				if (SearchComments)
				{
				    keywordIndex = 0;
					do
					{
						if (strstr(stringVal, keyword[keywordIndex]) != NULL )
						{
						    return keywordIndex; //This could miss keywords -- if more than one is in this comment, it only reports the first one found
						}
						++keywordIndex;
					} while (keywordIndex<numKeywords);
				}
				break;

			default:
				break;
		};
	}
	
	return -1;
}

void ParserPrepareForReading(FILE *file, const char* fileName) 
{
	int length;

	++fileDepth;

	assert (fileDepth < MAX_DEPTH);

    yyin = 0;

	fileStack[fileDepth] = yy_create_buffer(file, YY_BUF_SIZE);
    lineNumber[fileDepth] = 1;
	withinComment[fileDepth] = 0;

	yy_switch_to_buffer(fileStack[fileDepth]);

	length = strlen(fileName);

	if (length+1 > MAX_FILENAME_LGTH)
	{
		fileName += (length - MAX_FILENAME_LGTH + 1);
		strcpy(fileNameStack[fileDepth], fileName);
		fileNameStack[fileDepth][0] = '.';
		fileNameStack[fileDepth][1] = '.';
		fileNameStack[fileDepth][2] = '.';
	}
	else
	{
		strcpy(fileNameStack[fileDepth], fileName);
	}
}

void ParserFinishReading(FILE *file) 
{
	assert( fileDepth >= 0 );

	yy_delete_buffer(fileStack[fileDepth]);

	--fileDepth;

	if (fileDepth >= 0)
	{
		yy_switch_to_buffer(fileStack[fileDepth]);
	}
}

void ParserResetInput()
{
	FILE *fin = fileStack[fileDepth]->yy_input_file;
	assert( fin );

	fseek(fin, 0, SEEK_SET); //Go back to beginning of file

	yyrestart( fin );
}

int  ParserGetLineNumber() {
	assert( fileDepth >= 0 );
    return lineNumber[fileDepth];
}

const char* ParserGetFileName()
{
	assert (fileDepth >= 0);
	return fileNameStack[fileDepth];
}

int yywrap(void) {
    return 1;
}

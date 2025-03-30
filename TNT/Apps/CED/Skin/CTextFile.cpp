/*
	textfile.cpp
	fast text file reading stuff. scans strings out of a textfile loaded as a binary file.
*/

#if defined(TARGET_PC)
// this is here to prevent 150+ warnings "unreferenced inline function has been removed"
#pragma warning(disable: 4514)
#endif

#include "CTextFile.h"

#include "x_stdio.hpp"
#include "x_types.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "x_debug.hpp"
#include "x_math.hpp"
#include "cstring.h"


static char CommentChar  = ';';
static char Delimiters[] = ",[](){}";

// Constructor can't return value, so won't load file here
CTextFile::CTextFile()
{
	m_FileLength = 0;
	m_CurrentPosition = 0;
	m_LineNumber = 0;
	m_pText = NULL;
	m_pFilename = NULL;
	m_fp = NULL;
	m_bPrevIndent	=	FALSE;
	m_TotalMarkedSections= 0;
	m_Indentation		 = 0;

	InitTokenizer();
	return;
}

CTextFile::~CTextFile()
{
	UnloadTextFile();
}



xbool CTextFile::StoreTimeStamp(void)
{
	ASSERT(m_fp != NULL);			// Make sure file is open.

#ifdef TARGET_PC_WINCON
	StoreText("%s",__TIMESTAMP__);
#endif
	return TRUE;
}



xbool CTextFile::LoadTextFile( const char * pFilename )
{
	X_FILE		*fp;
	u32	count;
	u32	Chunks,SmallChunk;

	ASSERT(m_pText == NULL);			// Can't open mulitple files

	fp=x_fopen(pFilename, "rb");
	if (fp==NULL)
	{
        // DON'T ADD AN ASSERT HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		return FALSE;
	}
	x_fseek(fp,0,X_SEEK_END);
	m_FileLength=x_ftell(fp);
	x_fseek(fp,0,X_SEEK_SET);
	m_CurrentPosition = 0;

	m_pText=new char[m_FileLength + 1];
	if (m_pText==NULL)
	{
		ASSERT(0 && "Cant get ram for file space!");
		x_fclose(fp);
		return FALSE;
	}

// because to 'sce' file lib is screwed, read in 16byte chunks..


	Chunks = m_FileLength/16;
	SmallChunk= m_FileLength%16;


#if	0
	if (x_fread(m_pText, m_FileLength, 1, fp)!=1)
	{
		ASSERT(0 && "Can't read text file!");
		delete m_pText;
		x_fclose(fp);
		return FALSE;
	}
#endif

	for ( count=0; count<Chunks; count++ )
	{
		if (x_fread(m_pText+(count*16), 16, 1, fp)!=1)
		{
			ASSERT(0 && "Can't read text file!");
			delete m_pText;
			x_fclose(fp);
			return FALSE;
		}
	}

	if ( SmallChunk )
	{
		if (x_fread(m_pText+(Chunks*16), SmallChunk, 1, fp)!=1)
		{
			ASSERT(0 && "Can't read text file!");
			delete m_pText;
			x_fclose(fp);
			return FALSE;
		}
	}


	m_pText[m_FileLength] = 0;	


	// Copy filename for debugging purposes
	m_pFilename = new char[x_strlen(pFilename) + 1];
	ASSERT(m_pFilename != NULL);
	x_strcpy(m_pFilename, pFilename);

	x_fclose(fp);

	return TRUE;
}



// unload the file
void CTextFile::UnloadTextFile(void)
{
	delete m_pFilename;
	m_pFilename = NULL;

	delete m_pText;
	m_pText = NULL;
}



xbool CTextFile::CreateTextFile(char *pFilename)
{

	ASSERT(m_pText == NULL);			// Can't open mulitple files
	ASSERT(m_fp == NULL);				// Can't create another file, if ones already open.
	m_fp=x_fopen(pFilename, "wt");
	if (m_fp==NULL)
	{
		ASSERT(0 && "Can't create text file!");
		return FALSE;
	}

	// Copy filename for debugging purposes
	m_pFilename = new char[x_strlen(pFilename) + 1];
	ASSERT(m_pFilename != NULL);
	x_strcpy(m_pFilename, pFilename);

	return TRUE;
}	

xbool CTextFile::OpenTextFile(const char *pFilename)
{
	if (m_fp==NULL)
	{
		m_fp=x_fopen(pFilename, "wt");
		if (m_fp==NULL)
		{
			ASSERT(0 && "Can't create text file!");
			return FALSE;
		}

		// Copy filename for debugging purposes
		char	*pNewFilename	=	new char[x_strlen(pFilename) + 1];			// Name of file loaded
		ASSERT(m_pFilename != NULL);
		x_strcpy(pNewFilename, pFilename);

		if (m_pFilename)	// If file was created then may have m_pFilename but no m_pText
		{
			delete m_pFilename;
		}

		m_pFilename = pNewFilename;

	}
	else
		return FALSE;

	return TRUE;
}	


xbool CTextFile::CloseTextFile(void)
{
//	ASSERT(m_fp != NULL);			// Can't close whats not open!

	if (m_fp != NULL)
	{
		x_fclose(m_fp);
		m_fp = NULL;
	}

	return TRUE;
}	


char *CTextFile::GetFilename(void)
{
//	ASSERT(m_pText != NULL);					// No file open

	return m_pFilename;
}
	
void CTextFile::SeekStart()
{
	ASSERT(m_pText != NULL);					// No file open

	m_CurrentPosition = 0;
	m_LineNumber = 0;

	return;
}

xbool CTextFile::Seek(u32 FilePos)
{
	ASSERT(m_pText != NULL);					// No file open

	if (FilePos >= m_FileLength)
	{
		ASSERT(0 && "Can't seek past end of file in CTextFile::Seek.");
		return FALSE;
	}
	m_CurrentPosition = FilePos;
	m_LineNumber = SeekCurrentLineNumber();

	return TRUE;
}

xbool CTextFile::SeekRelative(int FilePosDelta)
{
    const u32 OldPosition = m_CurrentPosition;
	ASSERT(m_pText != NULL);					// No file open

	if (((u32) (m_CurrentPosition + FilePosDelta)) >= m_FileLength)
	{
		ASSERT(0 && "Illegal seek delta for file %s in CTextFile::SeekRelative.");
		return FALSE;
	}
	m_CurrentPosition = (u32) (m_CurrentPosition + FilePosDelta);

    // SeekRelative() is often used for small changes in position.
    // We always need to update m_LineNumber if a line boundary 
    // was crossed, but very often no adjustment is needed (and calls 
    // to SeekCurrentLineNumber() are *very* expensive).
    // We will apply this simple optimization:
    //    If there is no new-line character between the new and old position,
    //    excluding the endpoints, then there is no need to update the line number.
    bool LineNumberNeedsUpdate;

    {
        const u32 NewPosition    = m_CurrentPosition;
        const u32 LowerPosition  = MIN( OldPosition , NewPosition );
        const u32 HigherPosition = MAX( OldPosition , NewPosition );
        bool NewLineFound  = false;
        // (Remember, don't include the endpoints in the loop)
        for( u32 CheckPosition = LowerPosition+1 ; !NewLineFound && CheckPosition < HigherPosition ; CheckPosition++ )
        {
            if( m_pText[CheckPosition] == '\n' ) 
            {
                NewLineFound = true;
            }
        }
        LineNumberNeedsUpdate = NewLineFound;
    }

    if( LineNumberNeedsUpdate )
    {
	    m_LineNumber = SeekCurrentLineNumber();
    }

	return TRUE;
}

u32 CTextFile::GetPosition(void)
{
	ASSERT(m_pText != NULL);					// No file open

	return m_CurrentPosition;
}

xbool CTextFile::IsEof(void)								
{
	ASSERT(m_pText != NULL);					// No file open
	ASSERT(m_CurrentPosition <= m_FileLength);		// Something is very wrong if we've got beyond end of file

	if (m_CurrentPosition >= m_FileLength)
	{
		return TRUE;
	}

	return FALSE;
}

xbool CTextFile::IsEofErrorCheck(void)
{
	ASSERT(m_pText != NULL);					// No file open
	ASSERT(m_CurrentPosition <= m_FileLength);		// Something is very wrong if we've got beyond end of file

	if (m_CurrentPosition >= m_FileLength)
	{
		if (m_CurrentPosition > m_FileLength)
		{
			ASSERT(0 && "Invalid eof read for file %s in CTextFile::GetNext*");
		}
		return TRUE;
	}

	return FALSE;
}

xbool CTextFile::IsNextCharWhiteSpace(void)
{
	ASSERT(m_pText != NULL);					// No file open

	if (IsEofErrorCheck())
	{
		return FALSE;
	}

	if ((m_pText[m_CurrentPosition] == ' ') || (m_pText[m_CurrentPosition] == ASCII_TAB) 
		|| (m_pText[m_CurrentPosition] == ASCII_NEWLINE) || (m_pText[m_CurrentPosition] == ASCII_CR))
	{
		return TRUE;
	}

	return FALSE;
}

xbool CTextFile::IsNextCharCarriageReturn(void)
{
	ASSERT(m_pText != NULL);					// No file open

	if (IsEofErrorCheck())
	{
		return FALSE;
	}

	if ((m_pText[m_CurrentPosition] == ASCII_NEWLINE) || (m_pText[m_CurrentPosition] == ASCII_CR))
	{
		return TRUE;
	}

	return FALSE;
}

char *CTextFile::GetCurrentTextPtr(void)
{
	ASSERT(m_pText != NULL);					// No file open

	if (IsEofErrorCheck())
	{
		return FALSE;
	}

	return &m_pText[m_CurrentPosition];
}

char CTextFile::GetNextChar(void)							
{
	ASSERT(m_pText != NULL);					// No file open

	if (IsEofErrorCheck())
	{
		return '\0';
	}

	if ((m_pText[m_CurrentPosition] == ASCII_NEWLINE) || (m_pText[m_CurrentPosition] == ASCII_CR))
		m_LineNumber++;

	return m_pText[m_CurrentPosition++];
}

xbool CTextFile::GetNextString(char *string)			
{
	ASSERT(m_pText != NULL);					// No file open

	while ((!IsEofErrorCheck()) && IsNextCharWhiteSpace())
	{
		GetNextChar();
	}
	if (IsEof())
	{
		*string = 0;
		return FALSE;
	}

	while ((!IsEof()) && (!IsNextCharWhiteSpace()))
	{
		*string = GetNextChar();
		string++;
	}
	*string = 0;

	return TRUE;
}

xbool CTextFile::GetNextLine(char *string, xbool SkipStartingWhiteSpace /* FALSE */)
{
	ASSERT(m_pText != NULL);					// No file open

	if (IsEofErrorCheck())
	{
		*string=  0;
		return FALSE;
	}

	if (SkipStartingWhiteSpace)
	{
		while ((!IsEof()) && IsNextCharWhiteSpace())
		{
			GetNextChar();
		}
	}

	while (!IsEof())
	{
		if (IsNextCharCarriageReturn())
		{
			break;
		}
		else
		{
			*string = GetNextChar();
			string++;
		}
	}

// skip any remaining cr/lf codes
	while ((!IsEof()) && (IsNextCharCarriageReturn()))
	{
		GetNextChar();
	}

	*string=0;

	return TRUE;
}

xbool CTextFile::ScanTillEndOfLine(void)
{
	ASSERT(m_pText != NULL);					// No file open

	if (IsEofErrorCheck())
	{
		return TRUE;
	}

	while (!IsEof())
	{
		GetNextChar();
		if (IsNextCharCarriageReturn())
		{
			break;
		}
	}

// skip any remaining cr/lf codes
	while ((!IsEof()) && (IsNextCharCarriageReturn()))
	{
		GetNextChar();
	}

	return TRUE;
}

u32 CTextFile::SeekCurrentLineNumber(void)
{
	u32	LineNumber = 1;
	u32	Loop;

	ASSERT(m_pText != NULL);					// No file open

	for (Loop = 0; Loop < m_CurrentPosition; Loop++)
	{
		if (m_pText[Loop] == ASCII_NEWLINE)
		{
			LineNumber++;
		}
	}

	return LineNumber;
}

xbool CTextFile::DeleteText(u32 DeleteFrom, u32 DeleteTo)
{
	ASSERT(m_pText != NULL);					// No file open
	if ((DeleteFrom >= m_FileLength) || (DeleteTo >= m_FileLength))
	{
		
		ASSERT(0 && "Invalid deletion position for file %s in CTextFile::DeleteText");
		return FALSE;
	}
	if (DeleteFrom >= DeleteTo)
	{
		ASSERT(0 && "Deletion positions are wrong way around for file %s in CTextFile::DeleteText");
		return FALSE;
	}
	x_memmove(&m_pText[DeleteFrom], &m_pText[DeleteTo], m_FileLength - DeleteTo + 1);
	m_FileLength -= (DeleteTo - DeleteFrom);

	return TRUE;
}

xbool CTextFile::InsertText(u32 InsertPosition, char *pString)
{
	u32	StringLength;
	char	*pTempText;

	ASSERT(m_pText != NULL);					// No file open
	if (InsertPosition >= m_FileLength)
	{
		ASSERT(0 && "Invalid insertion position for file %s in CTextFile::DeleteText");
		return FALSE;
	}
	StringLength = x_strlen(pString);
	pTempText = new char[m_FileLength + StringLength + 1];
	x_memcpy(pTempText, m_pText, InsertPosition);
	x_memcpy(&pTempText[InsertPosition], pString, StringLength);
	x_memcpy(&pTempText[InsertPosition + StringLength], &m_pText[InsertPosition], m_FileLength - InsertPosition + 1);

 	delete m_pText;
	m_pText = pTempText;
	m_FileLength += StringLength;

	return TRUE;
}


//=========================================================================
// StrToFloat
//-------------------------------------------------------------------------
// it will handle numbers such "123.4567e+34"
//=========================================================================
float CTextFile::StrToFloat( const char* Str )
{
    float     Total1 = 0;
    float     Total2 = 0;
    float     Mult   = 1;
    float     Sign1  = 1;

    ASSERT( Str );

    //----------------------------------------------------
    // skip whitespace
    //----------------------------------------------------
    for ( ; *Str == ' '; ++Str );

    //----------------------------------------------------
    // Get the sign
    //----------------------------------------------------
    if (*Str== '-') {Str++; Sign1 = -1; }
    if (*Str== '+') Str++;

    //----------------------------------------------------
    // Start computer integer portion
    //----------------------------------------------------
    Total1 = 0;

    while ( (*Str >= '0') && ( *Str <= '9' ) )
    {
        // accumulate digit
        Total1 = 10 * Total1 + (*Str - '0');

        // get next char
        Str++;
    }

    //----------------------------------------------------
    // Handle decimals
    //----------------------------------------------------
    if ( *Str == '.' )
    {
        //----------------------------------------------------
        // Skip the decimal point
        //----------------------------------------------------
        Str++;

        //----------------------------------------------------
        // Get all the decimals
        //----------------------------------------------------
        Total2 = 0;

        while ( (*Str >= '0') && ( *Str <= '9' ) )
        {
            // accumulate digit
            Total2 = 10 * Total2 + (*Str - '0');

            // get next char
            Str++;

            // Acumulate the mults
            Mult *= 10;
        }
    }

    //----------------------------------------------------
    // Handle the scientific notation
    //----------------------------------------------------
    if ((*Str == 'e') || (*Str == 'E'))
    {
        float     Sign2 = 1;
        float     Total3;
        float     e;

        //----------------------------------------------------
        // Skip the e
        //----------------------------------------------------
        Str++;

        //----------------------------------------------------
        // Get the sign2
        //----------------------------------------------------
        if (*Str== '-') {Str++; Sign2 = -1; }
        if (*Str== '+') Str++;

        //----------------------------------------------------
        // get exponent
        //----------------------------------------------------
        Total3 = 0;

        while ( (*Str >= '0') && ( *Str <= '9' ) )
        {
            // accumulate digit
            Total3 = 10 * Total3 + (*Str - '0');

            // get next char
            Str++;
        }

        //----------------------------------------------------
        // Compute exponent. We make a shortcut for small 
        // exponents.
        //----------------------------------------------------
        if ( Total3 < 30 )
        {
            e = 1;
            while(Total3) 
            {
                e *= 10;
                Total3--;
            }
        }
        else
        {
            e = (float)x_pow( 10, Total3 );
        }

        //----------------------------------------------------
        // Return final number
        //----------------------------------------------------
        if ( Sign2 < 0) return (Sign1 * ( Total1 + Total2 / Mult )) / e;

        return (Sign1 * ( Total1 + Total2 / Mult )) * e;
    }

    return Sign1 * ( Total1 + Total2 / Mult );
}

///////////////////////////////////////////////////////////////////////////

u32  CTextFile::IsDelimiter (char c)
{
    u32 i=0;

	if (c == TOKEN_IDENTIFIER) return TOKEN_IDENTIFIER;

    while (Delimiters[i] != 0)
    {
        if (c == Delimiters[i]) return c;
        i++;
    } 

    return 0;
}

///////////////////////////////////////////////////////////////////////////


void  CTextFile::InitTokenizer(void)
{
    m_Token.EOLNs		 = 0;
    m_Token.TokenType    = STRTOKEN_NONE;
    m_Token.Delimiter    = ' ';
    m_Token.Float        = 0.0f;
    m_Token.Int          = 0;
    m_Token.NumberType   = TF_NUMBER_FLOAT;
    m_Token.StringOrig[0]= 0;
    m_Token.String[0]    = 0;
//j	m_Indentation		 = 0;
//j	m_TotalMarkedSections= 0;
}


///////////////////////////////////////////////////////////////////////////


u32 CTextFile::ReadTokenElement(void)
{
    u32     i;
    char    ch;

	InitTokenizer();

    //////////////////////////////////////////////////////////////////////
    // Check if at end of file
    //////////////////////////////////////////////////////////////////////
    if (m_CurrentPosition >= m_FileLength)
    {
        m_Token.TokenType = STRTOKEN_EOF;
        return m_Token.TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Skip whitespace and count EOLNs
    //////////////////////////////////////////////////////////////////////
    while ( (m_pText[m_CurrentPosition]<=32) &&
            (m_CurrentPosition < m_FileLength))
    {
        if (m_pText[m_CurrentPosition]=='\n') 
        {
            m_Token.EOLNs++;
			m_LineNumber++;
//			m_Indentation=-1;
        }
        m_CurrentPosition++;
//		m_Indentation++;
    }

    if (m_CurrentPosition==m_FileLength)
    {
        m_Token.TokenType = STRTOKEN_EOF;
        return m_Token.TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Check if it's a delimiter
    //////////////////////////////////////////////////////////////////////
    ch = m_pText[m_CurrentPosition];
    if ((ch==',') || (ch=='[') || (ch==']') || (ch=='(') || (ch==')') || (ch=='{') || (ch=='}') || (ch==TOKEN_IDENTIFIER) )
    {
        m_CurrentPosition++;
        m_Token.TokenType      = STRTOKEN_DELIMITER;
        m_Token.Delimiter      = ch;
        m_Token.StringOrig[0]  = ch;
        m_Token.String[0]      = ch;
        m_Token.StringOrig[1]  = 0;
        m_Token.String[1]      = 0;

		if (ch=='{')	m_Indentation++;
		if (ch=='}')	m_Indentation--;

        return m_Token.TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Decide if it is a number
    //////////////////////////////////////////////////////////////////////
    ch = m_pText[m_CurrentPosition];
    if (((ch>='0') && (ch<='9')) || (ch=='-') || (ch=='+'))
    {
        
        // Copy number into temp buffer
        i=0;
        ch = m_pText[m_CurrentPosition];
        do 
        {
            m_Token.String[i] = ch;
            m_CurrentPosition++;
            i++;
            ch = m_pText[m_CurrentPosition];
            
        } while (((ch>='0') && (ch<='9'))|| 
                  (ch=='-') || (ch=='+') ||
                  (ch=='E') || (ch=='e') || (ch=='.') ||
                  (ch=='#') || (ch=='Q') || (ch=='N') ||
                  (ch=='B') || (ch=='I') || (ch=='F') ||
                  (ch=='A'));

        // Generate float version
        m_Token.String[i] = 0;
        m_Token.Float     = StrToFloat( m_Token.String );
        m_Token.Int       = (u32)m_Token.Float;

		//-----------------------------------------------------------------
        // Decide on token type
		// Changed method for deciding if value is float or int.
		// Ziggy.
		// if (m_Token.Float != (float)m_Token.Int)
		//-----------------------------------------------------------------
		if(x_strstr(m_Token.String,".") != 0)
        {
            m_Token.TokenType  = STRTOKEN_NUMBER;
            m_Token.NumberType = TF_NUMBER_FLOAT;
        }
        else
        {
            m_Token.TokenType  = STRTOKEN_NUMBER;
            m_Token.NumberType = TF_NUMBER_INT;
        }

        // return values
        return m_Token.TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // Check if this is a line comment
    //////////////////////////////////////////////////////////////////////
    if ((m_pText[m_CurrentPosition] == CommentChar)||
        (m_pText[m_CurrentPosition] == '#'))
//    if (m_pText[m_CurrentPosition] == CommentChar)
    {
        while (m_pText[m_CurrentPosition]!='\n' && 
               (m_CurrentPosition < m_FileLength))
        {
            m_CurrentPosition++;
        }
        if (m_pText[m_CurrentPosition]=='\n') 
        {
            m_Token.EOLNs++;
			m_LineNumber++;
            m_CurrentPosition++;
        }
        return ReadTokenElement();
    }

    //////////////////////////////////////////////////////////////////////
    // Check if first character is a quote
    //////////////////////////////////////////////////////////////////////
    if (m_pText[m_CurrentPosition] == '"')
    {
        m_CurrentPosition++;        
        i=0;
        while (m_pText[m_CurrentPosition]!='"')
        {
            // Check for illegal ending of a string
            ASSERT((m_CurrentPosition < m_FileLength) && "EOF in quote");
            ASSERT((i<STRTOKEN_STRING_SIZE-1) && "Quote too u32");
            ASSERT((m_pText[m_CurrentPosition]!='\n') && "EOLN in quote");
            
            m_Token.StringOrig[i] = m_pText[m_CurrentPosition];
            m_Token.String[i]     = (char)x_toupper(m_pText[m_CurrentPosition]);
            i++;
            m_CurrentPosition++;
        }

        m_CurrentPosition++;
        m_Token.StringOrig[i]  = 0;
        m_Token.String[i]  = 0;
        m_Token.TokenType  = STRTOKEN_STRING;

        return m_Token.TokenType;
    }

    //////////////////////////////////////////////////////////////////////
    // It's a raw symbol
    //////////////////////////////////////////////////////////////////////
    {
        i=0;
        while ((m_CurrentPosition < m_FileLength) && 
               (i<STRTOKEN_STRING_SIZE-1) && 
               !x_isspace(m_pText[m_CurrentPosition]) &&
               !IsDelimiter(m_pText[m_CurrentPosition]))
        {
            m_Token.StringOrig[i] = (char)m_pText[m_CurrentPosition];
            m_Token.String[i]     = (char)x_toupper(m_pText[m_CurrentPosition]);
            i++;
            m_CurrentPosition++;
        }
        m_Token.StringOrig[i]=0;
        m_Token.String[i]=0;

        m_Token.TokenType  = STRTOKEN_SYMBOL;

	//  return m_Token.TokenType;
    }

    return m_Token.TokenType;
}

///////////////////////////////////////////////////////////////////////////


	// Token storing routines
xbool	CTextFile::BeginToken(const char *TokenString, int ArraySize, const char *NameString)
{
	// Store Token
	StoreText("%c%s", TOKEN_IDENTIFIER, TokenString);

	// Store array size even if non-zero
	StoreText("[ %d ]", ArraySize);

	// Store assignment
	StoreText(" = ");

	// Store NameString if not NULL
	if (*NameString)
	{
		StoreText("\"%s\"", NameString);
	}

	// Open brace will be on a new line
	StoreText("\n");

	// Store opening brace
	StoreText("{\n");


	// Increase tab indentation for contents of object
	m_Indentation++ ;
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::NewLine(void)
{
	// Store new line character
	StoreText("\n");

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::BeginToken(const char *TokenString, int ArraySize)
{
	// Store Token
	StoreText("%c%s", TOKEN_IDENTIFIER, TokenString);

	// Store array size if non-zero
	if (ArraySize)
		StoreText("[ %d ]", ArraySize);

	// Store assignment
	StoreText(" = ");

	// Open brace will be on a new line
	StoreText("\n");

	// Store opening brace
	StoreText("{\n");


	// Increase tab indentation for contents of object
	m_Indentation++ ;
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::BeginToken(const char *TokenString)
{
	// Store Token
	StoreText("%c%s", TOKEN_IDENTIFIER, TokenString);

	// Store assignment
	StoreText(" = ");

	// Open brace will be on a new line
	StoreText("\n");

	// Store opening brace
	StoreText("{\n");


	// Increase tab indentation for contents of object
	m_Indentation++ ;
	
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::BeginToken(const char *TokenString, const char *NameString)
{
	// Store Token
	StoreText("%c%s", TOKEN_IDENTIFIER, TokenString);

	// Store assignment
	StoreText(" = ");

	// Store NameString if not NULL
	if (*NameString)
	{
		StoreText("\"%s\"", NameString);
	}

	// Open brace will be on a new line
	StoreText("\n");

	// Store opening brace
	StoreText("{\n");


	// Increase tab indentation for contents of object
	m_Indentation++ ;
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::BeginToken(const char *TokenString, float Value)
{
	// Store Token
	StoreText("%c%s", TOKEN_IDENTIFIER, TokenString);

	// Store assignment
	StoreText(" = ");

	// Store value
	StoreText("%f", Value);

	// Open brace will be on a new line
	StoreText("\n");

	// Store opening brace
	StoreText("{\n");

	// Increase tab indentation for contents of object
	m_Indentation++ ;
	
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::EndToken()
{
	// Decrease tab indentation at end of object
	if (m_Indentation)
		m_Indentation-- ;

	// Store closing brace
	StoreText("}\n\n");

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::Token(const char *TokenString, int Value)
{
	// Store Token assignment
	StoreText("%c%s = %d\n", TOKEN_IDENTIFIER, TokenString, Value);

	return TRUE;
}


xbool	CTextFile::Token(const char *TokenString, float Value)
{
	// Store Token assignment
	StoreText("%c%s = %f\n", TOKEN_IDENTIFIER, TokenString, Value);

	return TRUE;
}

xbool	CTextFile::Token(const char *TokenString, const char *NameString)
{
	// Store Token string assignment
	StoreText("%c%s = \"%s\"\n", TOKEN_IDENTIFIER, TokenString, NameString);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::Put(f64 FloatValue)
{
	// store float
	StoreText("%lf, ", FloatValue);

	return TRUE;
}

xbool	CTextFile::Put(f32 FloatValue)
{
	// store float
	StoreText("%f, ", FloatValue);

	return TRUE;
}

xbool	CTextFile::Put(int IntValue)
{
	// store integer
	StoreText("%d, ", IntValue);

	return TRUE;
}
xbool	CTextFile::Put(unsigned int UIntValue)
{
	// store unsigned integer
	StoreText("%u, ", UIntValue);

	return TRUE;
}



xbool	CTextFile::Put(const char *String)
{
	// store string
	StoreText("%s", String);

	return TRUE;
}
xbool	CTextFile::Puts(const char *String)
{
	// store quoted string
	StoreText("\"%s\"", String);

	return TRUE;
}



xbool CTextFile::Put(const vector2& vV)
{
	StoreText("%f, %f, ", vV.X, vV.Y);
	return(TRUE);
}


xbool CTextFile::Put(const vector3& vV)
{
	StoreText("%f, %f, %f, ", vV.X, vV.Y, vV.Z);
	return(TRUE);
}


xbool CTextFile::Put(const vector4& vV)
{
	StoreText("%f, %f, %f, %f, ", vV.X, vV.Y, vV.Z, vV.W);
	return(TRUE);
}


xbool CTextFile::Put(const quaternion& qQ)
{
	StoreText("%f, %f, %f, %f, ", qQ.X, qQ.Y, qQ.Z, qQ.W);
	return(TRUE);
}

xbool CTextFile::PutComment(const char *String)
{
	// store comment string includes new line at end
	StoreText(";%s\n", String);

	return TRUE;
}


xbool CTextFile::Put(const cstring *pString)
{
	return(Put(pString->c_str0()));
}


xbool CTextFile::Puts(const cstring *pString)
{
	return(Puts(pString->c_str0()));
}

///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::Get(f64 *FloatValue)
{
	ASSERT(FloatValue);

	if (fastsscanf( "%lf",
				FloatValue
				) != 1) return FALSE;

	return TRUE;
}

xbool	CTextFile::Get(f32 *FloatValue)
{
	ASSERT(FloatValue);

	if (fastsscanf( "%f",
				FloatValue
				) != 1) return FALSE;

	return TRUE;
}


xbool	CTextFile::Get(int *IntValue)
{
	ASSERT(IntValue);

	if (fastsscanf( "%d",
				IntValue
				) != 1) return FALSE;
	
	return TRUE;
}


xbool	CTextFile::Get(char *String)
{
	ASSERT(String);

	if (fastsscanf( "%s",
				String
				) != 1) return FALSE;

    int Length = x_strlen(String);
	ASSERT(Length > 0);

    if( String[0] == '"' ) // Remove leading quote?
    {
        x_memmove( &String[0] , &String[1] , Length ); // Remove first character, shifting string over and including trailing null.
        Length--; // Adjust length for removed character
    }

    if( String[Length - 1] == '"' ) // Remove trailing quote?
    {
        String[Length - 1] = 0;
        Length--; // Adjust length for removed character
    }
	return TRUE;
}


xbool CTextFile::Get(vector2 *pvV)
{
	ASSERT(pvV);

	if (fastsscanf("%f %f", &pvV->X, &pvV->Y) != 2)
		return(FALSE);

	return(TRUE);
}


xbool CTextFile::Get(vector3 *pvV)
{
	ASSERT(pvV);

	if (fastsscanf("%f %f %f", &pvV->X, &pvV->Y, &pvV->Z) != 3)
		return(FALSE);

	return(TRUE);
}


xbool CTextFile::Get(vector4 *pvV)
{
	ASSERT(pvV);

	if (fastsscanf("%f %f %f %f", &pvV->X, &pvV->Y, &pvV->Z, &pvV->W) != 4)
		return(FALSE);

	return(TRUE);
}


xbool CTextFile::Get(quaternion *pqQ)
{
	ASSERT(pqQ);

	if (fastsscanf("%f %f %f %f", &pqQ->X, &pqQ->Y, &pqQ->Z, &pqQ->W) != 4)
		return(FALSE);

	return(TRUE);
}


xbool CTextFile::Get(cstring *pString)
{
	char	Str[1024];

	//j: this is a little dodgy as I'm asuming all strings will be less than 1024 chars long
	if (Get(Str) == FALSE)
	{
		return(FALSE);
	}

	*pString = cstring(Str);
	return(TRUE);
}

///////////////////////////////////////////////////////////////////////////
xbool	CTextFile::SkipSection(void)
{
    bool AlwaysTrue = true; // Trick to avoid compiler warnings.
	int InitIndentDepth = m_Indentation;

	// Search for closing brace of TOKEN
	while(AlwaysTrue)
	{
	 	if(CheckForIndent() == TRUE)
		{
			if(m_Indentation < InitIndentDepth) return TRUE;
		}
		else
			return TRUE;
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::GetToken(CTextFile::tokentype *Token)
{
	// Clear Token values
	Token->Count		=	0;
	x_strcpy(&Token->Name[0], "");
	x_strcpy(&Token->String[0], "");
	Token->Int			=	0;
	Token->Float		=	0.0f;
	Token->NumberType	=	TF_NUMBER_UNDEFINED;
    Token->Indentation  =   0;


	while (!IsEof())
	{
        Token->Indentation = this->m_Indentation;
		ReadTokenElement();
		if ( (m_Token.TokenType==STRTOKEN_DELIMITER) && (m_Token.Delimiter==TOKEN_IDENTIFIER) )
			break;
	}

	Token->TokenType	=	m_Token.TokenType;

	if (IsEof())
		return FALSE;

	// Read Token
	ReadTokenElement();
	if (IsEof())
		return FALSE;

	// Store Name if valid token
	if (m_Token.TokenType==STRTOKEN_SYMBOL)
		x_strcpy(Token->Name, m_Token.String);
	else
		return FALSE;

	// Expecting '=' or array count
	ReadTokenElement();
	if (IsEof())
		return FALSE;

	// Check for array count
	if ((m_Token.TokenType==STRTOKEN_DELIMITER)&&(m_Token.Delimiter=='['))
	{
		// Process Array Header
		ReadTokenElement();
		if (IsEof())
			return FALSE;
		Token->Count = m_Token.Int;
		// Skip past ']'
		ReadTokenElement();
		// Skip past '='
		ReadTokenElement();
		if (IsEof())
			return FALSE;
	}

	// read opening parenthesis or possibly a numeric/string assignment
	ReadTokenElement();
	if (IsEof())
		return FALSE;

	// Check for numeric/string assignment
	if ( (m_Token.TokenType==STRTOKEN_STRING) || (m_Token.TokenType==STRTOKEN_NUMBER) )
	{
		if (m_Token.TokenType==STRTOKEN_STRING)
		{
			x_strcpy(Token->String, m_Token.StringOrig) ;
		} else if (m_Token.TokenType==STRTOKEN_NUMBER)
		{
			if (m_Token.NumberType==TF_NUMBER_FLOAT)
				Token->Float	= m_Token.Float;
			else
				Token->Int		= m_Token.Int;

			Token->NumberType = m_Token.NumberType;
		}
		// skip past any opening parenthesis {
		if (NextCharIsOpeningBrace())
			ReadTokenElement();
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::GetSectionToken(tokentype *Token)
{
	// Clear Token values
	Token->Count		=	0;
	x_strcpy(&Token->Name[0], "");
	Token->Int			=	0;
	Token->Float		=	0.0f;
	Token->NumberType	=	TF_NUMBER_UNDEFINED;

	while (!IsEof())
	{
		ReadTokenElement();
		if ( (m_Token.TokenType==STRTOKEN_DELIMITER) && (m_Token.Delimiter==TOKEN_IDENTIFIER) )
			break;

		if ( (m_Token.TokenType==STRTOKEN_DELIMITER) && (m_Token.Delimiter==CLOSING_BRACE) )
		{
			ASSERT(m_TotalMarkedSections);

			if (MarkedSections[m_TotalMarkedSections-1]	> m_Indentation)
			{
				UnmarkSection();
				return FALSE;
			}
		}
	}

	Token->TokenType	=	m_Token.TokenType;

	if (IsEof())
		return FALSE;

	// Read Token
	ReadTokenElement();
	if (IsEof())
		return FALSE;

	// Store Name if valid token
	if (m_Token.TokenType==STRTOKEN_SYMBOL)
		x_strcpy(Token->Name, m_Token.String);
	else
		return FALSE;

	// Expecting '=' or array count
	ReadTokenElement();
	if (IsEof())
		return FALSE;

	// Check for array count
	if ((m_Token.TokenType==STRTOKEN_DELIMITER)&&(m_Token.Delimiter=='['))
	{
		// Process Array Header
		ReadTokenElement();
		if (IsEof())
			return FALSE;
		Token->Count = m_Token.Int;
		// Skip past ']'
		ReadTokenElement();
		// Skip past '='
		ReadTokenElement();
		if (IsEof())
			return FALSE;
	}

	ReadTokenElement();
	if (IsEof())
		return FALSE;

	// Check for numeric/string assignment
	if ( (m_Token.TokenType==STRTOKEN_STRING) || (m_Token.TokenType==STRTOKEN_NUMBER) )
	{
		if (m_Token.TokenType==STRTOKEN_STRING)
		{
			x_strcpy(Token->String, m_Token.StringOrig) ;
		} else if (m_Token.TokenType==STRTOKEN_NUMBER)
		{
			if (m_Token.NumberType==TF_NUMBER_FLOAT)
				Token->Float	= m_Token.Float;
			else
				Token->Int		= m_Token.Int;
			Token->NumberType = m_Token.NumberType;
		}
		// skip past any opening parenthesis {
		if (NextCharIsOpeningBrace())
			ReadTokenElement();
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////

s32		CTextFile::GetIndex(void)
{
	return	m_Indentation;
}

///////////////////////////////////////////////////////////////////////////


xbool	CTextFile::CheckForIndent(void)
{
	char	c;
	xbool	ret = FALSE;
	while(!ret) 
	{
		// terminate if this token doesn't have a body {...}
		if (NextCharIsTokenIdentifier())
			return ret;

		c = GetNextChar();

		if(c == '{') 
		{
			m_Indentation++;
			ret = TRUE;
		}	
		if(c == '}') 
		{
			m_Indentation--;
			ret = TRUE;
		}	
		if(IsEof())
			return ret;

	}
	return ret;
}


///////////////////////////////////////////////////////////////////////////


//**********************************************************
//Quick and dirty sscanf replacement function.
//**********************************************************

//**********************************************************
// Defines.
//**********************************************************
enum
{
	ZS_STRING=0,
	ZS_SINT32,
	ZS_FLOAT,
	ZS_DOUBLE,
};

//**********************************************************
//Globals
//**********************************************************


//**********************************************************
// Replaces c lib sscanf (Note: Only handle %d %f %s types.)
//**********************************************************
s32 CTextFile::fastsscanf(char *FmtString, ...)
{

	s32		VType,i;
	u32	*IntPtr;
	float	*FloatPtr;
	char	*StrPtr;
	double	*DoublePtr;
	u32	NumParams = 0;
	
	va_list marker;
	va_start( marker, FmtString ); 
	
	ZS_FmtString = FmtString;

	do
	{
		VType = ZS_GetType();
		switch(VType)
		{
		case	ZS_STRING:
			if(ZS_GetString(TRUE) != ZS_STRING) 
				return NumParams;
			StrPtr = va_arg(marker, char *);
			i=0;
			while(ZS_String[i] != 0)
				*StrPtr++ = ZS_String[i++];
			*StrPtr = 0;
			break;
		case	ZS_SINT32:
			if(ZS_GetString() != ZS_SINT32) 
				return NumParams;
			IntPtr = va_arg(marker, u32 *);
			*IntPtr = x_atoi(ZS_String);
			break;
		case	ZS_FLOAT:
			if(ZS_GetString() == ZS_STRING) 
				return NumParams;
			FloatPtr = va_arg(marker, float *);
			*FloatPtr = (float) x_atof(ZS_String);
			break;
		case	ZS_DOUBLE:
			if(ZS_GetString() == ZS_STRING)
				return NumParams;
			DoublePtr = va_arg(marker, double *);
			*DoublePtr = (double) x_atof(ZS_String);
			break;

		}
		NumParams++;
	}while(VType != -1);


	va_end( marker );              /* Reset variable arguments.      */

	return NumParams-1;
}



//**********************************************************
// Returns the next type in the format string.
//**********************************************************
s32		CTextFile::ZS_GetType(void)
{
	while(*ZS_FmtString != 0)
	{
		if(*ZS_FmtString++ == '%')
		{
			//Found token.
			switch(*ZS_FmtString)
			{
			case	's':
				return ZS_STRING;
				break;
			case	'd':
				return ZS_SINT32;
				break;
			case	'f':
				return ZS_FLOAT;
				break;
			case	'l':	// long
				ZS_FmtString++;	// skip past l character
				if ((*ZS_FmtString)=='f')	// check if 'f' - then set as double float
					return ZS_DOUBLE;
				else
					return ZS_SINT32;		// else treat as integer
				break;
			}
		}
	}
	return -1;
}

//**********************************************************
// Gets the string for the next type and does some minimal error checking.
//**********************************************************
s32	CTextFile::ZS_GetString(xbool isString/*=FALSE*/)
{	
	u32	i=0;
	u32	StringType;
	xbool IsEnclosedString = FALSE;
	char	CurrChar;


	CurrChar	= GetNextChar();
	while( (CurrChar == CommentChar) || (CurrChar == ' ') || (CurrChar == ',') || (CurrChar == ASCII_NEWLINE) || (CurrChar == ASCII_CR) || (CurrChar == ASCII_TAB) )
	{
		if (CurrChar==CommentChar)	// If Comment then skip past end of line
		{
			while( (CurrChar != ASCII_NEWLINE) && (CurrChar != ASCII_CR) )
			{
				if(CurrChar == 0)
					return -1;
				CurrChar = GetNextChar();
			}
		}

		if(CurrChar == 0)
			return -1;
		CurrChar = GetNextChar();
	}

	if (CurrChar==CommentChar)
	{
		while( (CurrChar == ASCII_NEWLINE) || (CurrChar == ASCII_CR) || (CurrChar == ASCII_TAB) )
		{
			if(CurrChar == 0)
				return -1;
			CurrChar = GetNextChar();
		}
	}

	if(isString)
	{
		StringType = ZS_STRING;
	}else
	{
		StringType = ZS_SINT32;
	}
	
	if (CurrChar=='"')
	{
		IsEnclosedString = TRUE;
		CurrChar	= GetNextChar();
	}
	while( (CurrChar != 0) )
	{
		if( ((CurrChar == ' ')&&(!IsEnclosedString)) || (CurrChar == ASCII_NEWLINE) || (CurrChar == ASCII_CR) || (CurrChar == ASCII_TAB) )
			break;

		if((CurrChar < '0') || (CurrChar > '9') )
			if( (CurrChar != '.') && (CurrChar != '-') && (CurrChar != '+') && (CurrChar != ','))
				StringType = ZS_STRING;
		   
		if(CurrChar == '.')
			if(StringType != ZS_STRING)
				StringType = ZS_FLOAT;

		if (CurrChar != ',') 
			ZS_String[i++] = CurrChar;
		CurrChar	= GetNextChar();
		if ((IsEnclosedString)&&(CurrChar=='"'))
			break;
	}

	ZS_String[i] = 0;
	return StringType;
}

/*****************************************************************************/
xbool CTextFile::tokentype::operator ==(const char *pName)
{
	ASSERT(pName);
	return(x_stricmp(pName, Name) == 0);
}

/*****************************************************************************/

xbool CTextFile::NextCharIsTokenIdentifier(void)
{
    char    ch;
	u32	CurrPos = m_CurrentPosition;												


    //////////////////////////////////////////////////////////////////////
    // Check if at end of file
    //////////////////////////////////////////////////////////////////////
    if (CurrPos >= m_FileLength)
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // Skip whitespace
    //////////////////////////////////////////////////////////////////////
    while ( (m_pText[CurrPos]<=32) &&
            (CurrPos < m_FileLength))
        CurrPos++;

    if (CurrPos==m_FileLength)
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // Check if it's a delimiter
    //////////////////////////////////////////////////////////////////////
    ch = m_pText[CurrPos];
    if (ch==TOKEN_IDENTIFIER)
		return TRUE;

    return FALSE;
}

/*****************************************************************************/

xbool CTextFile::NextCharIsOpeningBrace(void)
{
    char    ch;
	u32	CurrPos = m_CurrentPosition;												


    //////////////////////////////////////////////////////////////////////
    // Check if at end of file
    //////////////////////////////////////////////////////////////////////
    if (CurrPos >= m_FileLength)
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // Skip whitespace and count EOLNs
    //////////////////////////////////////////////////////////////////////
    while ( (m_pText[CurrPos]<=32) &&
            (CurrPos < m_FileLength))
        CurrPos++;

    if (CurrPos==m_FileLength)
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // Check if it's a delimiter
    //////////////////////////////////////////////////////////////////////
    ch = m_pText[CurrPos];
    if (ch==OPENING_BRACE)
		return TRUE;

    return FALSE;
}


/*****************************************************************************/

xbool CTextFile::NextCharIsClosingBrace(void)
{
    char    ch;
	u32	CurrPos = m_CurrentPosition;												


    //////////////////////////////////////////////////////////////////////
    // Check if at end of file
    //////////////////////////////////////////////////////////////////////
    if (CurrPos >= m_FileLength)
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // Skip whitespace and count EOLNs
    //////////////////////////////////////////////////////////////////////
    while ( (m_pText[CurrPos]<=32) &&
            (CurrPos < m_FileLength))
        CurrPos++;

    if (CurrPos==m_FileLength)
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // Check if it's a delimiter
    //////////////////////////////////////////////////////////////////////
    ch = m_pText[CurrPos];
    if (ch==CLOSING_BRACE)
		return TRUE;

    return FALSE;
}



/*****************************************************************************/


xbool CTextFile::StoreText(char *fmt, ... )
{
	char	Str[1024];
	char	*pStr;
	x_va_list	Arg;
	int		Len;

	ASSERT(m_fp != NULL);			// Make sure file is open.

	x_va_start(Arg, fmt);
	x_vsprintf(Str, fmt, Arg);
	x_va_end(Arg);



	if (m_bPrevIndent)
	{
		Indent();
		m_bPrevIndent = FALSE;
	}


	// if the last char is a new-line dont indent this time but do it first thing next time
	Len = x_strlen(Str);
	if (Str[(Len - 1)] == '\n')
	{
		// remove the new-line so we dont have to check for it in the loop
		Str[(Len - 1)] = '\0';

		m_bPrevIndent = TRUE;
	}


	pStr = Str;
	while(*pStr)
	{
		x_fputc(*pStr, m_fp);

		// indent if we've got a new-line (will not indent any terminating new-line as this will have been removed above)
		if (*pStr == '\n')
		{
			Indent();
		}

		pStr++;
	}


	// output the new-line
	if (m_bPrevIndent)
	{
		x_fputc('\n', m_fp);
	}

	x_fflush(m_fp);

	return TRUE;
}



void	CTextFile::Indent(void)
{
	// Indent Token
	for (int i=0; i<m_Indentation; i++)
		x_fprintf(m_fp,"\t");
}


/*****************************************************************************/


void	CTextFile::MarkSection()
{
	ASSERT(m_TotalMarkedSections<MAX_MARKED_SECTIONS);
	MarkedSections[m_TotalMarkedSections++]	=	m_Indentation ;
}

void	CTextFile::UnmarkSection()
{
	ASSERT(m_TotalMarkedSections);
	m_TotalMarkedSections--;
}

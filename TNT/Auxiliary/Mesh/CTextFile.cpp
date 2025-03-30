//#include "first.h"
/*
	textfile.cpp
	fast text file reading stuff. scans strings out of a textfile loaded as a binary file.
*/

#if defined(TARGET_PC)
    #pragma warning(disable: 4514) // this is here to prevent 150+ warnings "unreferenced inline function has been removed"
    #pragma warning( disable: 4127 ) // conditional expression is constant
#endif

//#include "stdafx.h"

#include "x_stdio.hpp"
#include "x_types.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "x_debug.hpp"
#include "x_math.hpp"
#include "CTextFile.h"

static const char CommentChar  = ';';
static const char Delimiters[] = ",[](){}";

static const char TOKEN_IDENTIFIER  = '*';
static const char CLOSING_BRACE     = '}';
static const char OPENING_BRACE     = '{';


// Constructor can't return value, so won't load file here
CTextFile::CTextFile()
{
	m_FileLength                        = 0;
	m_CurrentPosition                   = 0;
	m_LineNumber                        = 0;
	m_pText                             = NULL;
	m_pFilename                         = NULL;
	m_fp                                = NULL;
	m_bPrevIndent	                    = false;
	m_TotalMarkedSections               = 0;
	m_Indentation		                = 0;
    m_WasLoadedFromString               = false ;
	m_GetPutBufferSize		            = 4096;
	m_pGetPutBuffer			            = new char[m_GetPutBufferSize];
	ZS_String                           = new char[TEMP_STRING_SIZE];
    m_bUsingEmergencyBuffer             = false;

    m_bRemoveTrailingZerosFromFloats    = true;
	InitTokenizer();
	return;
}

CTextFile::~CTextFile()
{
	CloseTextFile();
	UnloadTextFile();

	delete[] ZS_String;
	ZS_String       = 0;
	delete[] m_pGetPutBuffer;
	m_pGetPutBuffer = 0;
}

void CTextFile::SetGetPutBufferSize(s32 size)
{
	if(size > m_GetPutBufferSize)
	{
		delete[]				m_pGetPutBuffer;
		m_pGetPutBuffer			= 0;
		m_GetPutBufferSize		= size;
		m_pGetPutBuffer			= new char[m_GetPutBufferSize];
		ASSERT(m_pGetPutBuffer);
	}
	
}

xbool CTextFile::StoreTimeStamp(void)
{
	ASSERT(m_fp != NULL);			// Make sure file is open.

#ifdef TARGET_PC_WINCON
	StoreText("%s",__TIMESTAMP__);
#endif
	return TRUE;
}

xbool CTextFile::LoadTextFile( char * String , s32 Length )
{
    m_FileLength            =   Length  ;
    m_CurrentPosition       =   0       ;
    m_StartOfSectionTokenPosition = 0   ;
    m_pText                 =   String  ;
    m_WasLoadedFromString   =   true    ;
    return true ;
}

// Reallocate a string, preserving its contents. Always at least
// doubles the capacity.
static void ReallocateString
(
    char * &    String      ,   // Input: The string to reallocate (which will be deallocated). Output: The newly allocated string. 
    s32 &       Capacity    ,   // Input: The old capacity (how many bytes allocated to String). Output: The new capacity.
    s32         Length      ,   // Input: The length of string to be preserved during reallocation, not including the trailing null.
    s32         MinCapacity     // The minimum capacity of the new string.
)
{
    const s32 NewCapacity = MAX( MinCapacity , 2*Capacity ) ;
    char * NewString = new char[ NewCapacity ];
    ASSERT( Length < Capacity );
    if( Length > 0 ) // Copy old contents to new string?
    {
        x_memcpy( NewString , String , Length+1 ); // +1 to copy trailing null.
    }
    delete[] String ;
    String = NewString ;
    Capacity = NewCapacity ;
}

// Append a string to another, reallocating it as needed.
static void AppendWithReallocation
(
    char * &    String      ,   // Input: The string to which String2 is to be appended. Output: The new string, possibly reallocated.
    s32 &       Capacity    ,   // Input: The old capacity (how many bytes allocated to String). Output: The new capacity (unchanged if no reallocation was needed).
    s32 &       Length      ,   // Input: The length of String. Output: The new length of String.
    const char *String2     ,   // The string to append to String.
    s32         Length2         // The length of String2.
)
{
    const s32 NewLength = Length + Length2 ; 
    if( NewLength >= Capacity ) // Need to reallocate? (We need an extra byte to store the trailing null)
    {
        ReallocateString( String , Capacity , Length , NewLength+1 ); //+1 for trailing null.
    }  
    x_memmove( &String[Length] , String2 , Length2 );
    Length = NewLength ;
    String[Length] = 0 ; // Add trailing null
}

// Specialized read to handle "#include" commands.
xbool CTextFile::LoadTextFileWithIncludes
( 
    const char * FileName       ,   // The name of the file to load.
    const char * IncludeKey         // The key which identifies "include" lines. Should not contain whitespace.
)
{
    xbool Okay = false ;
    const s32 IncludeKeyLength = x_strlen(IncludeKey);
    if( LoadTextFile(FileName) )
    {
        const s32 GuessAtExpandedSize = 100000 ;
        s32     Capacity    =   MAX( GuessAtExpandedSize , static_cast<s32>(10*m_FileLength) )    ;   // The capacity of *NewText. Increases as we reallocate NewText.
        s32     Length      =   0   ; // The length of text copied into *NewText.
        char *  NewText     =   new char[ Capacity ]; // The new text. Reallocated as needed to store more text.

        const s32 MaxLineLength = 2000 ;
        char Line[MaxLineLength+1] ; // +1 for trailing null.

        while( GetNextLine( Line, true ) )
        {
            s32 LineLength = x_strlen(Line);

            char * IncludedFileName = 0 ; // The included file name.
            if( LineLength > IncludeKeyLength ) // Long enough to be an "include" line?
            {
                char & FollowingChar = Line[IncludeKeyLength] ; // If this is an include line, this is the character following IncludeKey.
                if( FollowingChar <= ' ' ) // Must be whitespace to be an include line.
                {
                    const char SavedChar = FollowingChar ;
                    FollowingChar = 0 ; // Add null termination to make comparison easier (since we don't have a strnicmp() function).
                    if( x_stricmp( Line , IncludeKey ) == 0 )
                    {
                        IncludedFileName = &Line[IncludeKeyLength+1]; // (Doesn't really point to the file name yet)
                    }
                    FollowingChar = SavedChar ; // Restore the character.
                }
            }

            if( IncludedFileName != 0 )
            {
                // We haven't actually found the start of the file name yet - look for the beginning quote:
                IncludedFileName = x_strstr( IncludedFileName , "\"" );
                if( IncludedFileName != 0 ) // Did we find a beginning quote?
                {
                    ++ IncludedFileName ; // File name starts after quote
                    char * EndOfFileName = x_strstr( IncludedFileName , "\"" ); // Find closing quote
                    if( EndOfFileName != 0 ) // Did we find ending quote?
                    {
                        EndOfFileName[0] = 0 ; // Add null termination at end of file name (replaces ending quote).
                    }
                    else
                    {
                        IncludedFileName = 0 ; // No file to include. (Format was invalid)
                    }
                }
            }

            if( IncludedFileName != 0 && IncludedFileName[0] != 0 ) // Include another file?
            {
                x_std::string FullIncludedFileName  ;
                const xbool  NameIsAbsolute         =   IncludedFileName[0] == '\\' ;
                const xbool  NameHasDriveSpecifier  =   IncludedFileName[1]!=0 && IncludedFileName[2]==':' ; // (Such as in "C:Fred.txt")
                if(  NameIsAbsolute || NameHasDriveSpecifier ) // Use the name as it is?
                {
                    FullIncludedFileName = IncludedFileName ;
                }
                else // Name is relative to the file which includes it.
                {
                    FullIncludedFileName = FileName ; // Start with the source file.
                    const x_std::string::size_type iLastSeparator = FullIncludedFileName.find_last_of("\\/"); // Index of last slash or backslash.
                    FullIncludedFileName.erase( iLastSeparator );
                    FullIncludedFileName += '\\' ;
                    FullIncludedFileName += IncludedFileName ;
                }
                CTextFile IncludedFile ;
                const xbool CanOpenIncludedFile = IncludedFile.LoadTextFileWithIncludes( FullIncludedFileName.c_str() , IncludeKey );
                ASSERT( CanOpenIncludedFile );
                if( CanOpenIncludedFile )
                {
                    AppendWithReallocation( NewText , Capacity , Length , IncludedFile.RawText() , IncludedFile.m_FileLength );
                    AppendWithReallocation( NewText , Capacity , Length , "\n" , 1 ); // Add a new-line.
                    IncludedFile.UnloadTextFile();
                }
            }
            else if( Line[0] == ';' || Line[0] == 0 ) // Comment or empty line?
            {
                // Ignore it
            }
            else // Add the line to NewText.
            {
                Line[LineLength] = ASCII_NEWLINE ; ++ LineLength ; // Add new-line
                AppendWithReallocation( NewText , Capacity , Length , Line , LineLength );
            }
        }
        UnloadTextFile();

        LoadTextFile( NewText , Length );
		m_WasLoadedFromString = false;		// forces deallocation of string
        m_pText = NewText ;
        m_pFilename = new char[x_strlen(FileName) + 1];
        x_strcpy( m_pFilename , FileName );
        Okay = true ;

    }
    return Okay ;
}


xbool CTextFile::LoadTextFile( const char * pFilename, char* pEmergencyBuffer )
{
	X_FILE		*fp;

	ASSERT(m_pText == NULL);			// Can't open mulitple files

	if (*pFilename==0)	// is filename empty?
	{
		return FALSE;
	}

    m_WasLoadedFromString = false ;
	fp=x_fopen(pFilename, "rb");
	if (fp==NULL)
	{
        // DON'T ADD AN ASSERT HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		return FALSE;
	}

//    m_FileLength = x_filelength(fp);
	x_fseek(fp,0,X_SEEK_END);
	m_FileLength=x_ftell(fp);
	x_fseek(fp,0,X_SEEK_SET);	
	m_CurrentPosition = 0;

	m_pText = new char[ m_FileLength + 1 ];

	if (m_pText==NULL)
	{
        if (pEmergencyBuffer==NULL)
        {
		    ASSERTS(0, "Cant get ram for file space!");
		    x_fclose(fp);
		    return FALSE;
        }
        else
        {
            m_bUsingEmergencyBuffer = true;
            m_pText = pEmergencyBuffer;
        }
	}

	if (m_FileLength != 0)
	{
		if (x_fread(m_pText, m_FileLength, 1, fp)!=1)
		{
			//ASSERTS(0, "Can't read text file!");

			delete []m_pText;

			m_pText=0;
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
	delete []m_pFilename;
	m_pFilename = NULL;

    if( !m_WasLoadedFromString )
    {
		delete []m_pText;
    }
	m_pText = NULL;
}



xbool CTextFile::CreateTextFile(const char *pFilename)
{

	ASSERT(m_pText == NULL);			// Can't open mulitple files
	ASSERT(m_fp == NULL);				// Can't create another file, if ones already open.
	m_fp=x_fopen(pFilename, "wt");
	if (m_fp==NULL)
	{
		//ASSERT(0 && "Can't create text file!");
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
		m_fp=x_fopen(pFilename, "at");
		if (m_fp==NULL)
		{
    		m_fp=x_fopen(pFilename, "wt");
    		if (m_fp==NULL)
            {
			    //ASSERT(0 && "Can't create text file!");
			    return FALSE;
            }
		}


        x_fseek( m_fp, 0, X_SEEK_END);
   

		// Copy filename for debugging purposes
		char	*pNewFilename	=	new char[x_strlen(pFilename) + 1];			// Name of file loaded
		ASSERT(pNewFilename!= NULL);
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


char *CTextFile::GetFilename(void) const
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

    // Allow a seek to the end of file (positioned at the file length), but no further.
	if (FilePos > m_FileLength) 
	{
		ASSERTS(0, "Can't seek past end of file in CTextFile::Seek.");
		return FALSE;
	}
	m_CurrentPosition = FilePos;
	m_LineNumber = SeekCurrentLineNumber();

	return TRUE;
}

xbool CTextFile::SeekRelative(s32 FilePosDelta)
{
    const u32 OldPosition = m_CurrentPosition;
	ASSERT(m_pText != NULL);					// No file open

	if (((u32) (m_CurrentPosition + FilePosDelta)) >= m_FileLength)
	{
		ASSERTS(0, fs("Illegal seek delta for file %s in CTextFile::SeekRelative.",m_pFilename));
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

    {
        const u32 NewPosition    = m_CurrentPosition;
        const u32 LowerPosition  = MIN( OldPosition , NewPosition );
        const u32 HigherPosition = MAX( OldPosition , NewPosition );
        s32 NewLinesFound  = 0 ;
        // (Remember, don't include the endpoints in the loop)
        for( u32 CheckPosition = LowerPosition+1 ; CheckPosition < HigherPosition ; CheckPosition++ )
        {
            if( m_pText[CheckPosition] == ASCII_NEWLINE ) 
            {
                ++ NewLinesFound ;
            }
        }
        if( NewLinesFound == 0 )
        {
        }
        else if( NewPosition <= OldPosition )
        {
	        m_LineNumber -= NewLinesFound ;
        }
        else 
        {
	        m_LineNumber += NewLinesFound ;
        }
    }


	return TRUE;
}

u32 CTextFile::GetPosition(void) const
{
	ASSERT(m_pText != NULL);					// No file open

	return m_CurrentPosition;
}

xbool CTextFile::IsEof(void) const
{
	ASSERT(m_pText != NULL);					// No file open
	ASSERT(m_CurrentPosition <= m_FileLength);		// Something is very wrong if we've got beyond end of file

	if (m_CurrentPosition >= m_FileLength)
	{
		return TRUE;
	}

	return FALSE;
}

xbool CTextFile::IsEofErrorCheck(void) const
{
	ASSERT(m_pText != NULL);					// No file open
	ASSERT(m_CurrentPosition <= m_FileLength);		// Something is very wrong if we've got beyond end of file

	if (m_CurrentPosition >= m_FileLength)
	{
		if (m_CurrentPosition > m_FileLength)
		{
			ASSERTS(0, fs("Invalid eof read for file %s in CTextFile::GetNext*",m_pFilename));
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

void CTextFile::SkipWhiteSpace()
{
    while
    (
            m_CurrentPosition < m_FileLength
        &&  ( m_pText[m_CurrentPosition] == CommentChar || m_pText[m_CurrentPosition]<=32 )
    )
    {
        if( m_pText[m_CurrentPosition] == CommentChar )
        {
            // Comment - skip to end of line
            while (m_pText[m_CurrentPosition]!=ASCII_NEWLINE && 
                   (m_CurrentPosition < m_FileLength))
            {
                m_CurrentPosition++;
            }
        }
        if( m_CurrentPosition >= m_FileLength )
        {
        }
        else if ( m_pText[m_CurrentPosition]==ASCII_NEWLINE) 
        {
            m_Token.EOLNs++;
			m_LineNumber++;
            m_CurrentPosition++;
        }
        else
        {
            m_CurrentPosition++;
        }
    }
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

	if (m_pText[m_CurrentPosition] == ASCII_NEWLINE)
	{
		m_LineNumber++;
	}

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
		if (IsNextCharCarriageReturn())
		{
			break;
		}
		GetNextChar();
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
		
		ASSERTS(0, fs("Invalid deletion position for file %s in CTextFile::DeleteText", m_pFilename));
		return FALSE;
	}
	if (DeleteFrom >= DeleteTo)
	{
		ASSERTS(0, fs("Deletion positions are wrong way around for file %s in CTextFile::DeleteText", m_pFilename));
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
    ASSERT(!m_bUsingEmergencyBuffer);           // Not using emergency buffer

	if (InsertPosition >= m_FileLength)
	{
		ASSERTS(0, fs ("Invalid insertion position for file %s in CTextFile::DeleteText", m_pFilename));
		return FALSE;
	}
	StringLength = x_strlen(pString);
	pTempText = new char[m_FileLength + StringLength + 1];
	x_memcpy(pTempText, m_pText, InsertPosition);
	x_memcpy(&pTempText[InsertPosition], pString, StringLength);
	x_memcpy(&pTempText[InsertPosition + StringLength], &m_pText[InsertPosition], m_FileLength - InsertPosition + 1);

	delete []m_pText;

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
    for (s32 trash = 0 ; *Str == ' '; ++Str ) trash++;  //trash added to get rid of warning

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


u32 CTextFile::ReadTokenElement(xbool bIgnoreDelimiters)
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
        if (m_pText[m_CurrentPosition]==ASCII_NEWLINE) 
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
    if (!bIgnoreDelimiters)
    {
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
        m_Token.Int       = x_atoi(m_Token.String);

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
        while (m_pText[m_CurrentPosition]!=ASCII_NEWLINE && 
               (m_CurrentPosition < m_FileLength))
        {
            m_CurrentPosition++;
        }
        if (m_pText[m_CurrentPosition]==ASCII_NEWLINE) 
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
            ASSERTS(m_CurrentPosition < m_FileLength, "EOF in quote");
            ASSERTS(m_CurrentPosition < m_FileLength, m_pFilename == 0 ? "File: none" : m_pFilename ); // It helps to see the file name.
            ASSERTS(i<STRTOKEN_STRING_SIZE-1, "Quote too long!");
            ASSERTS(m_pText[m_CurrentPosition]!=ASCII_NEWLINE, "EOLN in quote");
            
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
               ( (bIgnoreDelimiters && (i==0)) || (!IsDelimiter(m_pText[m_CurrentPosition])) ) )
        {
            m_Token.StringOrig[i] = (char)m_pText[m_CurrentPosition];
            m_Token.String[i]     = (char)x_toupper(m_pText[m_CurrentPosition]);
            i++;
            m_CurrentPosition++;
			if (bIgnoreDelimiters && (i==1) && (IsDelimiter(m_Token.String[0])))
			{	// if delimiter is 1st character and ignoring delimiters then just return delimiter character
				break;
			}
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
xbool	CTextFile::BeginToken(const char *TokenString, s32 ArraySize, const char *NameString)
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

xbool	CTextFile::BeginToken(const char *TokenString, s32 ArraySize)
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
    if (m_bRemoveTrailingZerosFromFloats)
    {
        char ValueStr[64];
        FloatToShortStr(Value, ValueStr);
	    StoreText("%s", ValueStr);
    }
    else
    {
    	StoreText("%f", Value);
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

xbool	CTextFile::Token(const char *TokenString, s32 Value)
{
	// Store Token assignment
	StoreText("%c%s = %d\n", TOKEN_IDENTIFIER, TokenString, Value);

	return TRUE;
}


xbool	CTextFile::Token(const char *TokenString, float Value)
{
	// Store Token assignment
	// Store value
    if (m_bRemoveTrailingZerosFromFloats)
    {
        char ValueStr[64];
        FloatToShortStr(Value, ValueStr);
	    StoreText("%c%s = %s\n", TOKEN_IDENTIFIER, TokenString, ValueStr);
    }
    else
    {
	    StoreText("%c%s = %f\n", TOKEN_IDENTIFIER, TokenString, Value);
    }

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
	// Store value
    if (m_bRemoveTrailingZerosFromFloats)
    {
        char ValueStr[64];
        FloatToShortStr(FloatValue, ValueStr);
	    StoreText("%s, ", ValueStr);
    }
    else
    {
	    StoreText("%f, ", FloatValue);
    }

	return TRUE;
}

xbool	CTextFile::Put(s32 IntValue)
{
	// store integer
	StoreText("%d, ", IntValue);

	return TRUE;
}
xbool	CTextFile::Put(u32 UIntValue)
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
	// Store value
    if (m_bRemoveTrailingZerosFromFloats)
    {
        char ValueStr[64];
        char ValueStr2[64];
        FloatToShortStr(vV.X, ValueStr);
        FloatToShortStr(vV.Y, ValueStr2);
	    StoreText("%s, %s, ", ValueStr, ValueStr2);
    }
    else
    {
    	StoreText("%f, %f, ", vV.X, vV.Y);
    }
	return(TRUE);
}


xbool CTextFile::Put(const vector3& vV)
{
	// Store value
    if (m_bRemoveTrailingZerosFromFloats)
    {
        char ValueStr[64];
        char ValueStr2[64];
        char ValueStr3[64];
        FloatToShortStr(vV.X, ValueStr);
        FloatToShortStr(vV.Y, ValueStr2);
        FloatToShortStr(vV.Z, ValueStr3);
	    StoreText("%s, %s, %s, ", ValueStr, ValueStr2, ValueStr3);
    }
    else
    {
	    StoreText("%f, %f, %f, ", vV.X, vV.Y, vV.Z);
    }
	return(TRUE);
}


xbool CTextFile::Put(const vector4& vV)
{
    if (m_bRemoveTrailingZerosFromFloats)
    {
        char ValueStr[64];
        char ValueStr2[64];
        char ValueStr3[64];
        char ValueStr4[64];
        FloatToShortStr(vV.X, ValueStr);
        FloatToShortStr(vV.Y, ValueStr2);
        FloatToShortStr(vV.Z, ValueStr3);
        FloatToShortStr(vV.W, ValueStr4);
	    StoreText("%s, %s, %s, %s, ", ValueStr, ValueStr2, ValueStr3, ValueStr4);
    }
    else
    {
    	StoreText("%f, %f, %f, %f, ", vV.X, vV.Y, vV.Z, vV.W);
    }
	return(TRUE);
}


xbool CTextFile::Put(const quaternion& qQ)
{
    if (m_bRemoveTrailingZerosFromFloats)
    {
        char ValueStr[64];
        char ValueStr2[64];
        char ValueStr3[64];
        char ValueStr4[64];
        FloatToShortStr(qQ.X, ValueStr);
        FloatToShortStr(qQ.Y, ValueStr2);
        FloatToShortStr(qQ.Z, ValueStr3);
        FloatToShortStr(qQ.W, ValueStr4);
	    StoreText("%s, %s, %s, %s, ", ValueStr, ValueStr2, ValueStr3, ValueStr4);
    }
    else
    {
    	StoreText("%f, %f, %f, %f, ", qQ.X, qQ.Y, qQ.Z, qQ.W);
    }
	return(TRUE);
}

xbool CTextFile::Put(const radian3& rR)
{
    if (m_bRemoveTrailingZerosFromFloats)
    {
        char ValueStr[64];
        char ValueStr2[64];
        char ValueStr3[64];
        FloatToShortStr(rR.Pitch, ValueStr);
        FloatToShortStr(rR.Roll, ValueStr2);
        FloatToShortStr(rR.Yaw, ValueStr3);
	    StoreText("%s, %s, %s, ", ValueStr, ValueStr2, ValueStr3);
    }
    else
    {
    	StoreText("%f, %f, %f, ", rR.Pitch, rR.Roll, rR.Yaw);
    }
	return(TRUE);
}


xbool CTextFile::PutComment(const char *String)
{
	// store comment string includes new line at end
	StoreText(";%s\n", String);

	return TRUE;
}


xbool CTextFile::Put(const x_std::string *pString)
{
    return(Put(pString->c_str()));
}


xbool CTextFile::Puts(const x_std::string *pString)
{
    return(Puts(pString->c_str()));
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


xbool	CTextFile::Get(s32 *IntValue)
{
	ASSERT(IntValue);

	if (fastsscanf( "%d",
				IntValue
				) != 1) return FALSE;
	
	return TRUE;
}


xbool	CTextFile::Get(u32 *IntValue)
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

    s32 Length = x_strlen(String);
	if (Length==0)
	{
		return FALSE;
	}

    if( String[0] == '"' ) // Remove leading quote?
    {
        x_memmove( &String[0] , &String[1] , Length ); // Remove first character, shifting string over and including trailing null.
        Length--; // Adjust length for removed character
    }

    if (Length > 0)
    {
        if( String[Length - 1] == '"' ) // Remove trailing quote?
        {
            String[Length - 1] = 0;
            Length--; // Adjust length for removed character
        }
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

xbool CTextFile::Get(radian3 *prR)
{
	ASSERT(prR);

	if (fastsscanf("%f %f %f", &prR->Pitch, &prR->Roll, &prR->Yaw) != 3)
		return(FALSE);

	return(TRUE);
}

xbool CTextFile::Get(x_std::string *pString)
{
	char* Str = m_pGetPutBuffer; // shortcut
	//char	Str[1024];

	// ** yup changed to have use defined length *** //j: this is a little dodgy as I'm asuming all strings will be less than 1024 chars long
	if (Get(Str) == FALSE)
	{
		return(FALSE);
	}

	*pString = Str;
	return(TRUE);
}

///////////////////////////////////////////////////////////////////////////
xbool	CTextFile::SkipSection(void)
{
	s32 InitIndentDepth = m_Indentation;

    bool Continue = true ; // false when we are done searching
	// Search for closing brace of TOKEN
	while(Continue)
	{
	 	if(CheckForIndent() == TRUE)
		{
			if(m_Indentation < InitIndentDepth) 
            {
                Continue = false ;
            }
		}
		else
        {
			Continue = false ;
        }
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// GetToken -
// if bDelimiterNotRequired is TRUE then will get any next token element
// regardless of whether it has a * before it.

xbool	CTextFile::GetToken(CTextFile::tokentype *Token, xbool bDelimiterNotRequired)
{
	// Clear Token values
	Token->Count		=	0;
	x_strcpy(&Token->Name[0], "");
	x_strcpy(&Token->String[0], "");
	Token->Int			=	0;
	Token->Float		=	0.0f;
	Token->NumberType	=	TF_NUMBER_UNDEFINED;
    Token->Indentation  =   0;
    m_CurrTokenHasBody  =   FALSE;


	while (!IsEof())
	{
        Token->Indentation = this->m_Indentation;
		ReadTokenElement(bDelimiterNotRequired);
        if (bDelimiterNotRequired)
        {
            Token->TokenType	=	m_Token.TokenType;
            m_CurrTokenHasBody  =   FALSE;
            switch (Token->TokenType)
            {
                case STRTOKEN_SYMBOL:
                {
		            x_strcpy(Token->Name, m_Token.String);
		            x_strcpy(Token->NameOrig, m_Token.StringOrig);
                }
                break;
                case STRTOKEN_STRING:
                {
                    x_strcpy(Token->String, m_Token.StringOrig) ;
                }
                break;
                case STRTOKEN_NUMBER:
                {
                    x_strcpy(Token->String, m_Token.String ) ;
			        if (m_Token.NumberType==TF_NUMBER_FLOAT)
				        Token->Float	= m_Token.Float;
			        else
				        Token->Int		= m_Token.Int;

			        Token->NumberType = m_Token.NumberType;
                }
//    else if (m_Token.TokenType==STRTOKEN_DELIMITER)
            }

            return (!IsEof());
        }

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
    {
		x_strcpy(Token->Name, m_Token.String);
		x_strcpy(Token->NameOrig, m_Token.StringOrig);
    }
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

	/// This causes failure if the token element just read is the
    // last thing in the file (and there is new-line after it).
    //? if (IsEof())
    //?		return FALSE;

	// Check for numeric/string assignment
	if ( (m_Token.TokenType==STRTOKEN_STRING) || (m_Token.TokenType==STRTOKEN_NUMBER) )
	{
		if (m_Token.TokenType==STRTOKEN_STRING)
		{
			x_strcpy(Token->String, m_Token.StringOrig) ;
		} else if (m_Token.TokenType==STRTOKEN_NUMBER)
		{
            x_strcpy(Token->String, m_Token.String ) ;
			if (m_Token.NumberType==TF_NUMBER_FLOAT)
				Token->Float	= m_Token.Float;
			else
				Token->Int		= m_Token.Int;

			Token->NumberType = m_Token.NumberType;
		}
		// skip past any opening parenthesis {
		if (NextCharIsOpeningBrace())
        {
            m_CurrTokenHasBody = TRUE;
			ReadTokenElement();
        }
	}
    else if (m_Token.TokenType==STRTOKEN_DELIMITER)
    {
        if (m_Token.Delimiter==OPENING_BRACE)
        {
            m_CurrTokenHasBody = TRUE;
        }
    }

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////

xbool	CTextFile::GetSectionToken(tokentype *Token)
{
	// Clear Token values
	Token->Count		=	0;
	x_strcpy(&Token->Name[0], "");
	x_strcpy(&Token->String[0], "");
	Token->Int			=	0;
	Token->Float		=	0.0f;
	Token->NumberType	=	TF_NUMBER_UNDEFINED;
    m_CurrTokenHasBody  =   FALSE;

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
//				UnmarkSection();
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
    {
		x_strcpy(Token->Name, m_Token.String);
		x_strcpy(Token->NameOrig, m_Token.StringOrig);
    }
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
        {
            m_CurrTokenHasBody = TRUE;
			ReadTokenElement();
        }
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
		// terminate if this token doesn't have a body {...} - handled by checking m_CurrTokenHasBody
        if (!m_CurrTokenHasBody) // only process if current token has no body
        {
		    if (NextCharIsTokenIdentifier())
		    	return ret;
        }

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
	s32	ReturnValue;
	
	x_va_list marker;
	x_va_start( marker, FmtString ); 
	
	ZS_FmtString = FmtString;

	do
	{
		VType = ZS_GetType();
		switch(VType)
		{
		case	ZS_STRING:
			if(ZS_GetString(TRUE) != ZS_STRING) 
				return NumParams;
			StrPtr = x_va_arg(marker, char *);
			i=0;
			while(ZS_String[i] != 0)
				*StrPtr++ = ZS_String[i++];
			*StrPtr = 0;
			break;
		case	ZS_SINT32:
			if(ZS_GetString() != ZS_SINT32) 
				return NumParams;
			IntPtr = x_va_arg(marker, u32 *);
			*IntPtr = x_atoi(ZS_String);
			break;
		case	ZS_FLOAT:
//			if(ZS_GetString() == ZS_STRING) 
//				return NumParams;
			ReturnValue = ZS_GetString();
			if(ReturnValue == ZS_STRING) 
				return NumParams;
			if (ReturnValue == -1)
				return 0;
			FloatPtr = x_va_arg(marker, float *);
			*FloatPtr = (float) x_atof(ZS_String);
			break;
		case	ZS_DOUBLE:
			if(ZS_GetString() == ZS_STRING)
				return NumParams;
			DoublePtr = x_va_arg(marker, double *);
			*DoublePtr = (double) x_atof(ZS_String);
			break;

		}
		NumParams++;
	}while(VType != -1);


	x_va_end( marker );              /* Reset variable arguments.      */

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
				CurrChar = GetNextChar();
				if(CurrChar == 0)
					return -1;
			}
		}

		CurrChar = GetNextChar();
		if(CurrChar == 0)
			return -1;
	}

	if (CurrChar==CommentChar)
	{
		while( (CurrChar == ASCII_NEWLINE) || (CurrChar == ASCII_CR) || (CurrChar == ASCII_TAB) )
		{
			CurrChar = GetNextChar();
			if(CurrChar == 0)
				return -1;
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

		// if character is a comma and not part of an enclosed string then skip character
		if ((CurrChar != ',') || (IsEnclosedString) )
			ZS_String[i++] = CurrChar;
		CurrChar	= GetNextChar();
		if ((IsEnclosedString)&&(CurrChar=='"'))
        {   // store closing string and exit
			ZS_String[i++] = CurrChar;
            break;
        }
	}

	ZS_String[i] = 0;
	return StringType;
}

/*****************************************************************************/
xbool CTextFile::tokentype::operator ==(const char *pName) const
{
	ASSERT(pName);
	return(x_stricmp(pName, Name) == 0);
}

/*****************************************************************************/
xbool CTextFile::tokentype::ReplaceEscapeSequences( char * String)
{
    xbool ReplacedSomething = false ;
    if( String != 0 )
    {
        // Since replacing escape sequences never increases the length of a string,
        // we can do the replacement in-place.
        char * Source = String ; // The place in String[] from which to get the next character.
        char * Target = String ; // The place in String[] to put the next character. Might be the same as Source.
        while( Source[0] != 0 )
        {
            if( Source[0] == '\\' ) // Escape character?
            {
                xbool DidChange = true ; // Assume there was a change until we find otherwise.
                switch( Source[1] )
                {
                    case '\'' : { Target[0] = Source[1] ; Source += 2 ; Target += 1 ; break; }
                    case '\"' : { Target[0] = Source[1] ; Source += 2 ; Target += 1 ; break; }
                    case '\?' : { Target[0] = Source[1] ; Source += 2 ; Target += 1 ; break; }
                    case '\\' : { Target[0] = Source[1] ; Source += 2 ; Target += 1 ; break; }
                    case 'a'  : { Target[0] = '\a'      ; Source += 2 ; Target += 1 ; break; }
                    case 'b'  : { Target[0] = '\b'      ; Source += 2 ; Target += 1 ; break; }
                    case 'f'  : { Target[0] = '\f'      ; Source += 2 ; Target += 1 ; break; }
                    case 'n'  : { Target[0] = '\n'      ; Source += 2 ; Target += 1 ; break; }
                    case 'r'  : { Target[0] = '\r'      ; Source += 2 ; Target += 1 ; break; }
                    case 't'  : { Target[0] = '\t'      ; Source += 2 ; Target += 1 ; break; }
                    case 'v'  : { Target[0] = '\v'      ; Source += 2 ; Target += 1 ; break; }

                    // Unrecognized escape sequences will be left unchanged.
                    default   : 
                    { 
                        Target[0] = Source[0] ; 
                        Target[1] = Source[1] ; 
                        Source += 2 ; 
                        Target += 2 ; 
                        DidChange = false ;
                        break; 
                    }
                }
                if( DidChange )
                {
                    ReplacedSomething = true ;
                }
            }
            else // Not an escape sequence - just copy the character.
            {
                Target[0] = Source[0] ; 
                Source += 1 ; 
                Target += 1 ; 
            }
        }
        Target[0] = 0 ; // Terminate the string
    }
    return ReplacedSomething ;
}

/*****************************************************************************/
xbool CTextFile::tokentype::ReplaceEscapeSequences()
{
    return ReplaceEscapeSequences(this->String);
}


/*****************************************************************************/

xbool CTextFile::NextCharIsTokenIdentifier(void)
{
    char    ch;

    SkipWhiteSpace();

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

    SkipWhiteSpace();

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

    SkipWhiteSpace();

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
	char*   Str = m_pGetPutBuffer; // shortcut
	//char	Str[1024];
	char	*pStr;
	x_va_list	Arg;
	s32		Len;

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
	if (Str[(Len - 1)] == ASCII_NEWLINE)
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
		if (*pStr == ASCII_NEWLINE)
		{
			Indent();
		}

		pStr++;
	}


	// output the new-line
	if (m_bPrevIndent)
	{
		x_fputc(ASCII_NEWLINE, m_fp);
	}

	x_fflush(m_fp);

	return TRUE;
}



void	CTextFile::Indent(void)
{
	// Indent Token
	for (s32 i=0; i<m_Indentation; i++)
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


/*****************************************************************************/
/* returns a ptr to start of current text section and length of section      */
/*****************************************************************************/

void    CTextFile::GetSectionRawText(char **ppText, s32* Len)
{
	tokentype	Token;
    s32 Start,End;

    // move back to start of token
    Start = m_CurrentPosition;
    while ((m_pText[Start]!=TOKEN_IDENTIFIER) && Start)
    {
        Start--;
    }

    // store ptr to start of section
    *ppText = &m_pText[Start];

    MarkSection();

    // skip through section
    while (GetSectionToken(&Token))
    {
        SkipSection();
    }

    // get ptr to end of section
    End = m_CurrentPosition;

    UnmarkSection();

    // store length of section
    *Len = (End - Start);

}

/*****************************************************************************/
/* adds raw text to current position in textfile                             */
/*****************************************************************************/

void    CTextFile::AddSectionRawText(char *pText)
{
	ASSERT(m_fp != NULL);			// Make sure file is open.

    // copy raw text string to current position in file
	while(*pText)
	{
		x_fputc(*pText, m_fp);
		pText++;
	}

	x_fflush(m_fp);

}


/*****************************************************************************/
/* Convert float to string removing excess 0's                               */
/*****************************************************************************/

void CTextFile::FloatToShortStr(f32 FloatValue, char *pStr)
{
    s32 pos = x_sprintf(pStr, "%f", FloatValue);

    // remove all excess 0's
    pos--;  // move before end of string
    while ( (pos>=2) && (pStr[pos-1]!='.') && (pStr[pos]=='0') )
    {
        pStr[pos--] = 0;
    }
}


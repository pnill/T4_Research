/*
	textfile.h
	fast text file reading stuff. scans strings out of a textfile loaded as a binary file.
*/

#ifndef	_INC_TEXTFILE
#define	_INC_TEXTFILE

#include "x_types.hpp"
#include "x_stdio.hpp"
#include "x_math.hpp"
#include "x_string.h"


///////////////////////////////////////////////////////////////////////////
//  DEFINES
///////////////////////////////////////////////////////////////////////////

    enum AsciiCharValues
    {
        ASCII_TAB       =   9,
        ASCII_NEWLINE   =   10,
        ASCII_CR        =   13,
    };

    enum TokenType
    {
        STRTOKEN_NONE       = 0,
        STRTOKEN_DELIMITER,
        STRTOKEN_SYMBOL,
        STRTOKEN_STRING,
        STRTOKEN_NUMBER,
        STRTOKEN_EOF,
    };

    enum NumberType
    {
        TF_NUMBER_UNDEFINED = 0 ,
        TF_NUMBER_FLOAT,
        TF_NUMBER_INT,
    };

static const s32 STRTOKEN_STRING_SIZE         =   1024;
static const s32 TEMP_STRING_SIZE		      =   2048;

///////////////////////////////////////////////////////////////////////////
//  FORWARD DECLARATIONS
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//  STRUCTURES
///////////////////////////////////////////////////////////////////////////
class CTextFile
{

private:

struct tokenizer
{
public:
    s32		TokenType                           ;   // Type of token found
    char	Delimiter                           ;   // Char if token is delimiter
    char	StringOrig[STRTOKEN_STRING_SIZE+1]  ;   // Raw string
    char	String[STRTOKEN_STRING_SIZE+1]      ;   // String for symbol,string,number
                                                    // always uppercase
    f32		Float                               ;   // Float version of number token
    s32     Int                                 ;   // Int version of number token
    s32     NumberType                          ;   // Whether number was f32 or s32
    s32     EOLNs                               ;   // Lines crossed when seeking token
} ;

public:

///////////////////////////////////////////////////////////////////////////

struct	tokentype
{
//	xbool Is(const char *pName);
	xbool operator == (const char *pName) const ;
    xbool operator != (const char *pName) const { return !((*this) == pName ); }

    // Replace any escape sequences in String with the characters they represent.
    // Return true if at least one escape sequence was found, false otherwise.
    // Handles all C++ simple escape sequences (\', \", and so on), but does
    // handle hexadecimal or octal escape sequences.
    static xbool ReplaceEscapeSequences( char * String) ; 

    // Replace any escape sequences in this->String with the characters they represent.
    // Return true if at least one escape sequence was found, false otherwise.
    // Handles all C++ simple escape sequences (\', \", and so on), but does
    // handle hexadecimal or octal escape sequences.
    xbool ReplaceEscapeSequences() ; 

public:
    s32     TokenType                       ;       // Type of token found
    char	String[STRTOKEN_STRING_SIZE+1]  ;       // String for symbol,string,number
							                            // always uppercase
							                            // always uppercase
    f32     Float                           ;       // Float version of number token
    s32     Int                             ;       // Int version of number token
    s32     NumberType                      ;       // Whether number was f32 or s32
    s32     Indentation                     ;       // The indentation of the token.

	s32     Count                           ;       // Array Count
    char    Name[STRTOKEN_STRING_SIZE+1]    ;       // Name string assigned to symbol - set to uppercase
    char    NameOrig[STRTOKEN_STRING_SIZE+1];       // Name string assigned to symbol not set to upper case
} ;

///////////////////////////////////////////////////////////////////////////

	CTextFile();
	~CTextFile();

	//  Read functions.
	xbool	LoadTextFile        ( const char *Filename, char* pEmergencyBuffer = NULL )  ;   // Load the file
    xbool   LoadTextFile        ( char * String , s32 Length )  ;   // Load the file from a string. *String must persist until file is unloaded.
	void	UnloadTextFile      (void)                          ;   // unload the file
	char	*GetFilename        (void) const                    ;   // Get filename
	xbool	IsEof               (void) const                    ;   // Is at end of file
	void	SetGetPutBufferSize (s32 size)                      ;   // Set the maximum Get/Put size, default is in constructor

    // Specialized read to handle "#include" commands.
    // Lines that start with IncludeKey (after any whitespace) are replaced with the contents
    // of the file name (in quotes) which follows, such as in:
    //    #include "MyFiles\Fred.txt"
    // The file can either be an absolute path name or relative to FileName.
    // Return true if successful, false if there was an error.
    xbool   LoadTextFileWithIncludes
    ( 
        const char * FileName                    ,          // The name of the file to load.
        const char * IncludeKey = "#include"                // The key which identifies "include" lines. Should not contain whitespace.
    );
															// Warning! Will cause realloc.
	// Save functions.
	xbool	CreateTextFile      (const char * pFilename)    ;   // Creates a text file.
	xbool	OpenTextFile        (const char *pFilename)     ;
	xbool	CloseTextFile       (void)                      ;   // Close the current text file.
	xbool	StoreText           (char *fmt, ...)            ;
	xbool	StoreTimeStamp      (void)                      ;

	// New Line storing routine
	xbool	NewLine             (void)                      ;

	// Token storing routines
	xbool	BeginToken(const char *TokenString, s32 ArraySize, const char *NameString);
	xbool	BeginToken(const char *TokenString, s32 ArraySize);
	xbool	BeginToken(const char *TokenString);
	xbool	BeginToken(const char *TokenString, const char *NameString);
	xbool	BeginToken(const char *TokenString, f32 Value);
	xbool	EndToken();
	xbool	Token(const char *TokenString, s32 Value);
	xbool	Token(const char *TokenString, f32 Value);
	xbool	Token(const char *TokenString, const char *NameString);

	// Get functions
	xbool	Get(f64 *FloatValue);
	xbool	Get(f32 *FloatValue);
	xbool	Get(s32 *IntValue);
	xbool	Get(u32 *IntValue);
	xbool	Get(char *String);
	xbool	Get(vector2 *pV);
	xbool	Get(vector3 *pV);
	xbool	Get(vector4 *pV);
	xbool	Get(radian3 *pR);
	xbool	Get(quaternion *pqQ);
	xbool	Get(x_std::string *pString);
	
	xbool	SkipSection(void);

	// Put functions
	xbool	Put(f64 FloatValue);
	xbool	Put(f32 FloatValue);
	xbool	Put(s32 IntValue);
	xbool	Put(u32 UIntValue);
	xbool	Put(const char *String);
	xbool	Puts(const char *String);
	xbool	Put(const vector2& vV);
	xbool	Put(const vector3& vV);
	xbool	Put(const vector4& vV);
	xbool	Put(const radian3& pR);
	xbool	Put(const quaternion& qQ);
	xbool	Put(const x_std::string *pString);
	xbool	Puts(const x_std::string *pString);

	xbool	PutComment              (const char *String);

	xbool	GetToken(tokentype *Token, xbool bDelimiterNotRequired=FALSE);  // set bDelimiterNotRequired to TRUE if you are going to parse elements individually (e.g. files not in CTextFile format)
	xbool	GetSectionToken         (tokentype *Token)  ;
	s32		GetIndex                (void)              ;

	s32		m_Indentation;

	void	SeekStart               (void)              ;           // Seek back to start of file
	xbool	Seek                    (u32 FilePos)       ;           // Seek to a position in file
	xbool	SeekRelative            (s32 FilePosDelta)  ;           // Seek relative to current position
	char	GetNextChar             (void)              ;           // Reads next char
	u32		CurrentLineNumber       (void) { return m_LineNumber; } // Get line number at CurrentPosition
	u32		GetPosition             (void) const        ;           // Get current position within file
	xbool	IsNextCharWhiteSpace    (void)              ;           // Is the next character white space
	xbool	GetNextLine             (char *pString, xbool bSkipStartingWhiteSpace = FALSE); // Reads till end of line
	xbool	ScanTillEndOfLine       (void)              ;           // Jumps to end of line
    xbool   NextLine                (void)              { return ScanTillEndOfLine();};     // Skip past rest of line
	xbool	IsNextCharCarriageReturn(void)              ;           // Is the next character a LR or CR

	//  Read modify functions.
	xbool	DeleteText              (u32 DeleteFrom, u32 DeleteTo);         // Deletes the text between these 2 positions
	xbool	InsertText              (u32 InsertPosition, char *pString);    // Inserts string at this position		
	void	MarkSection             (void)              ;			// Calling MarkSection will mean any subsequent calls to
																	// skipsection will skip past everything contained within
																	// the enclosing braces at this point
	void	UnmarkSection           (void)              ;			// this will be done automatically by the GetSectionToken
																	// upon encountering a closing brace that matches the opening brace of the section marked

    void    GetSectionRawText       (char **ppText, s32* Len)  ;    // returns a ptr to start of text section and length
    void    AddSectionRawText       (char *pText)              ;    // adds raw text to current position in textfile


	xbool	NextCharIsTokenIdentifier   (void)  ;       // Is the next character (after whitespace and comments) a token identifier?
	xbool	NextCharIsClosingBrace      (void)  ;       // Is the next character (after whitespace and comments) a closing brace?
	xbool	NextCharIsOpeningBrace      (void)  ;       // Is the next character (after whitespace and comments) an opening brace?

    const char * RawText() const { return m_pText ; }   // Use with caution - raw pointer to contents of text file. 0 if not ready.
    u32		Length() const { return m_FileLength ; }	// Use with caution - raw pointer to contents of text file. 0 if not ready.

    void    SetRemoveTrailingZerosFromFloats(xbool bVal)    { m_bRemoveTrailingZerosFromFloats = bVal; }

    void    SkipWhiteSpace          ()                      ;       // Skip over any whitespace and comments at the current position.
protected:																

    enum { MAX_MARKED_SECTIONS = 12 } ;

	char	*GetCurrentTextPtr      (void)                  ;       // Risky! For special cases only
	xbool	GetNextString           (char *pString)         ;       // Finds next string seperated by white space
	s32		fastsscanf              (char *FmtString, ...)  ;

    void    FloatToShortStr         (f32 FloatValue, char *pStr);


    // Very expensive: searches from beginning of text to find 
    // the line number of the current position. Can be used (if
    // function result is saved in m_LineNumber) to correct
    // m_LineNumber after changing the file position.
	u32     SeekCurrentLineNumber();                                        


	f32     StrToFloat          (const char* Str);
	u32		IsDelimiter         (char c)            ;

	void	Indent              (void)              ;

	tokenizer	m_Token;

	u32		ReadTokenElement    (xbool bIgnoreDelimiters = FALSE);
	void	InitTokenizer       (void)          ;
	xbool	IsEofErrorCheck     (void) const    ;                 // Same as IsEof() but with debug window message
	xbool	CheckForIndent      (void)          ;
	s32		ZS_GetType          (void)          ;
	s32		ZS_GetString        (xbool isString = FALSE);

	char	*ZS_FmtString                       ;
	char	*ZS_String                          ;                 // this used to be instantiated with [TEMP_STRING_SIZE], but it could end up blowing the stack

	X_FILE	*m_fp                               ;
	u32     m_FileLength                        ;
	u32     m_CurrentPosition                   ;
	u32     m_StartOfSectionTokenPosition       ;												
	char	*m_pText                            ;                   // When NULL file hasn't been loaded correctly
	char	*m_pFilename                        ;                   // Name of file loaded
    xbool   m_WasLoadedFromString               ;

    u32		m_LineNumber                        ;                   // Current line number
	xbool	m_bPrevIndent                       ;
	xbool	m_TotalMarkedSections               ;
	s32		MarkedSections[MAX_MARKED_SECTIONS] ;

    xbool   m_CurrTokenHasBody                  ;

	s32		m_GetPutBufferSize                  ;
	char*	m_pGetPutBuffer                     ;

    xbool   m_bRemoveTrailingZerosFromFloats    ;
    xbool   m_bUsingEmergencyBuffer             ;

};



#endif

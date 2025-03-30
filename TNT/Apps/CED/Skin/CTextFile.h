/*
	textfile.h
	fast text file reading stuff. scans strings out of a textfile loaded as a binary file.
*/

#ifndef	_INC_TEXTFILE
#define	_INC_TEXTFILE

#include "x_types.hpp"
#include "x_stdio.hpp"
#include "x_math.hpp"


///////////////////////////////////////////////////////////////////////////
//  DEFINES
///////////////////////////////////////////////////////////////////////////

#define ASCII_TAB		(9)
#define ASCII_NEWLINE	(10)
#define ASCII_CR		(13)


#define STRTOKEN_NONE              0
#define STRTOKEN_DELIMITER         1
#define STRTOKEN_SYMBOL            2
#define STRTOKEN_STRING            3
#define STRTOKEN_NUMBER            4
#define STRTOKEN_EOF               5
#define TF_NUMBER_UNDEFINED		0
#define TF_NUMBER_FLOAT            1
#define TF_NUMBER_INT              2
#define STRTOKEN_STRING_SIZE       512



///////////////////////////////////////////////////////////////////////////
//  FORWARD DECLARATIONS
///////////////////////////////////////////////////////////////////////////
class	cstring;



///////////////////////////////////////////////////////////////////////////
//  STRUCTURES
///////////////////////////////////////////////////////////////////////////
class CTextFile
{

private:

struct tokenizer
{
public:
    signed   long	TokenType;						// Type of token found
    char			Delimiter;                      // Char if token is delimiter
    char			StringOrig[STRTOKEN_STRING_SIZE+1];// Raw string
    char			String[STRTOKEN_STRING_SIZE+1];	// String for symbol,string,number
							                        // always uppercase
    float			Float;							// Float version of number token
    signed   long	Int;                            // Int version of number token
    signed   long	NumberType;                     // Whether number was float or int
    signed   long	EOLNs;                          // Lines crossed when seeking token
} ;

public:

///////////////////////////////////////////////////////////////////////////

#define	TOKEN_IDENTIFIER	'*'
#define	CLOSING_BRACE		'}'
#define	OPENING_BRACE		'{'

///////////////////////////////////////////////////////////////////////////

struct	tokentype
{
//	xbool Is(const char *pName);
	xbool operator == (const char *pName);
    xbool operator != (const char *pName) { return !((*this) == pName ); }

public:
    signed   long	TokenType;						// Type of token found
    char			String[STRTOKEN_STRING_SIZE+1];	// String for symbol,string,number
							                        // always uppercase
    float			Float;							// Float version of number token
    signed   long	Int;                            // Int version of number token
    signed   long	NumberType;                     // Whether number was float or int
    int             Indentation ;                   // The indentation of the token.

	int				Count;							// Array Count
    char			Name[STRTOKEN_STRING_SIZE+1];   // Name string assigned to symbol
} ;

///////////////////////////////////////////////////////////////////////////

	CTextFile();
	~CTextFile();

	//  Read functions.
	xbool	LoadTextFile( const char *Filename );							// Load the file
	void	UnloadTextFile(void);											// unload the file
	char	*GetFilename(void);												// Get filename
	xbool	IsEof(void);													// Is at end of file
																			// Warning! Will cause realloc.
	// Save functions.
	xbool	CreateTextFile(char *pText);									// Creates a text file.
	xbool	OpenTextFile(const char *pFilename);
	xbool	CloseTextFile(void);											// Close the current text file.
	xbool	StoreText(char *fmt, ...);
	xbool	StoreTimeStamp(void);

	// New Line storing routine
	xbool	NewLine(void);

	// Token storing routines
	xbool	BeginToken(const char *TokenString, int ArraySize, const char *NameString);
	xbool	BeginToken(const char *TokenString, int ArraySize);
	xbool	BeginToken(const char *TokenString);
	xbool	BeginToken(const char *TokenString, const char *NameString);
	xbool	BeginToken(const char *TokenString, float Value);
	xbool	EndToken();
	xbool	Token(const char *TokenString, int Value);
	xbool	Token(const char *TokenString, float Value);
	xbool	Token(const char *TokenString, const char *NameString);

	// Get functions
	xbool	Get(f64 *FloatValue);
	xbool	Get(f32 *FloatValue);
	xbool	Get(int *IntValue);
	xbool	Get(char *String);
	xbool	Get(vector2 *pV);
	xbool	Get(vector3 *pV);
	xbool	Get(vector4 *pV);
	xbool	Get(quaternion *pqQ);
	xbool	Get(cstring *pString);
	
	xbool	SkipSection(void);

	// Put functions
	xbool	Put(f64 FloatValue);
	xbool	Put(f32 FloatValue);
	xbool	Put(int IntValue);
	xbool	Put(unsigned int UIntValue);
	xbool	Put(const char *String);
	xbool	Puts(const char *String);
	xbool	Put(const vector2& vV);
	xbool	Put(const vector3& vV);
	xbool	Put(const vector4& vV);
	xbool	Put(const quaternion& qQ);
	xbool	Put(const cstring *pString);
	xbool	Puts(const cstring *pString);

	xbool	PutComment(const char *String);


	xbool	GetToken(tokentype *Token);
	xbool	GetSectionToken(tokentype *Token);
	s32		GetIndex(void);

	s32		m_Indentation;


	void	SeekStart(void);												// Seek back to start of file
	xbool	Seek(u32 FilePos);											// Seek to a position in file
	xbool	SeekRelative(int FilePosDelta);									// Seek relative to current position
	char	GetNextChar(void);												// Reads next char
	u32		CurrentLineNumber(void) { return m_LineNumber; }				// Get line number at CurrentPosition
	u32		GetPosition(void);												// Get current position within file
	xbool	IsNextCharWhiteSpace(void);										// Is the next character white space
	xbool	GetNextLine(char *pString, xbool bSkipStartingWhiteSpace = FALSE); // Reads till end of line
	xbool	ScanTillEndOfLine(void);										// Jumps to end of line
	xbool	IsNextCharCarriageReturn(void);									// Is the next character a LR or CR

	//  Read modify functions.
	xbool	DeleteText(u32 DeleteFrom, u32 DeleteTo);				// Deletes the text between these 2 positions
	xbool	InsertText(u32 InsertPosition, char *pString);			// Inserts string at this position		
	void	MarkSection(void);										// Calling MarkSection will mean any subsequent calls to
																	// skipsection will skip past everything contained within
																	// the enclosing braces at this point
	void	UnmarkSection(void);									// this will be done automatically by the GetSectionToken
																	// upon encountering a closing brace that matches the opening brace of the section marked

	xbool	NextCharIsTokenIdentifier(void);
	xbool	NextCharIsClosingBrace(void);
	xbool	NextCharIsOpeningBrace(void);

protected:																

#define	MAX_MARKED_SECTIONS	12

	char	*GetCurrentTextPtr(void);										// Risky! For special cases only
	xbool	GetNextString(char *pString);									// Finds next string seperated by white space
	s32		fastsscanf(char *FmtString, ...);

    // Very expensive: searches from beginning of text to find 
    // the line number of the current position. Can be used (if
    // function result is saved in m_LineNumber) to correct
    // m_LineNumber after changing the file position.
	u32     SeekCurrentLineNumber();                                        


	float	StrToFloat(const char* Str);
	u32		IsDelimiter(char c);

	void	Indent(void);

	tokenizer	m_Token;

	u32		ReadTokenElement(void);
	void	InitTokenizer(void);
	xbool	IsEofErrorCheck(void);											// Same as IsEof() but with debug window message
	xbool	CheckForIndent(void);
	s32		ZS_GetType(void);
	s32		ZS_GetString(xbool isString = FALSE);


	char	*ZS_FmtString;
	char	ZS_String[256];

	X_FILE	*m_fp;
	u32	m_FileLength;														
	u32	m_CurrentPosition;												
	char	*m_pText;															// When NULL file hasn't been loaded correctly
	char	*m_pFilename;														// Name of file loaded

    u32		m_LineNumber;								                     // Current line number
	xbool	m_bPrevIndent;
	xbool	m_TotalMarkedSections;
	s32		MarkedSections[MAX_MARKED_SECTIONS];


};



#endif

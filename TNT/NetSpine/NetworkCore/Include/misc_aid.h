////////////////////////////////////////////////////////////////////////////
//
//  NetSpine Brand Technology, Property of Acclaim Entertainment. 
//	Trademark and Patent Pending 2001.
//
////////////////////////////////////////////////////////////////////////////



//###################################################################
//	
//  Filename  : misc_aid.h
//
//  Purpose   : Provide useful miscellaneous functions.
//
//###################################################################

//-------------------------------------------------------------------
#ifndef	_MISC_AID_H_
#define _MISC_AID_H_
//-------------------------------------------------------------------
#ifdef __cplusplus	
    extern "C" {
#endif
//-------------------------------------------------------------------


//###################################################################
//- FILE I/O
//###################################################################

u32 IsValidPlatformFileName( char* pszDataFile );

//-------------------------------------------------------------------
//  Function  : A_GetParentPath
//  Purpose   : To get the full parent portion of a path.  Output buffer
//              should be at least the size of the input buffer.
//              Ex:"C:\fish\eat\tuna" will output "C:\fish\eat"
//
//  Returns   : (0) If success.
//  Inputs    : pszInFullyQualifiedPath, The full input path.
//  Outputs   : pszOutParentPath, Buffer stuffed with the parent path.
//...................................................................
void    A_GetParentPath(  char* pszInFullyQualifiedPath , char* pszOutParentPath );

//-------------------------------------------------------------------
//  Function  : A_StripFileExtension
//  Purpose   : To strip off the extension on a filename string.
//
//  Returns   : (0) If success.
//  In/Out    : pszFileName, The filename which may or may not include an extension.
//...................................................................
s32     A_StripFileExtension( char* pszFileName );

//Returns NULL or if found a file extension with period.
//Ex: "c:\logo.bmp" => ".bmp", "C:\" => NULL
char*   A_GetFileExtension( char* pInFullFileName );

void    A_StripPathName( char* pInFullFileName , char** ppOutFileNameOnly );

//Get the last part of a path
//ex: "abc\def\ghi"   Last part = "ghi"
//ex: "abc\def\ghi\"  Last part = "ghi"
//ex: "\abc\def\ghi"  Last part = "ghi"
//ex: "\abc\def\ghi\" Last part = "ghi"
//ex: "abc\"          Last part = "abc"
//ex: "abc"           Last part = "abc"
//ex: "\abc"          Last part = "abc"
//ex: "\"             Last part = NULL; success
//Returns 0 if success, otherwise error like double slash.
u32     A_PathGetLastPart( char* pszInFullPath , char* abyDestLastPart );

//Get a zero based index part of a path.
//Paths must begin with a slash or pathname, drives shouldn't be passed in.
//Double slashes are not accepted.
//returns 0 if success.
//ex: "\\part0\\part1"      , Index 0="part0", Index 1="part1"
//ex: "/part0/part1"        , Index 0="part0", Index 1="part1"
//ex: "/part0/part1/"       , Index 0="part0", Index 1="part1", Index 2 = Error/Null
//ex: "/part0//part1"       , Error two forward slashes.
//ex: "/part0/./part2"      , Index 0="part0", Index 1=".", Index 2 = "part2"
//ex: "/part0/../part2"     , Index 0="part0", Index 1="..", Index 2 = "part2"
//ex: "part0\\part1"        , Index 0="part0", Index 1="part1"
//ex: "part0"               , Index 0="part0", Index 1=Error/Null
//ex: "/"                   , Index 0=Error/Null

u32 A_GetNumPathParts( char* pszFullPath, u32* pdwGetNumPathParts );

u32 A_GetPathPart( char* pszFullPath, u32 dwWhichPartIndex, char* pabyDestPart );


//Get the path after the topmost parent part.
//The complete child path will start with a slash.
//Returns 0 if success, 
//ex: "abc\def"     output "\def"
//ex: "abc\def\"    output "\def\"
//ex: "\abc\def\"   output "\def\"
//ex: "abc\"        output ""
//ex: "\"           output ""
u32     A_GetPathAfterTopParent( char* pszTheKey, char* abyAfterParent );


//Verify that the last byte of the path is not a slash.
//Returns 0 if success.
//Error if two slashes in a row.
u32     A_InsureNoEndingSlash( char* pszInFullPath );

//Verify that there is no starting slash.
u32     A_InsureNoStartingSlash( char* pszInFullPath );


//Strip the filename AND extension leaving only the previous path ending with '\'
//"C:\" => "C:\", "C:\me\home.bmp" => "C:\me\", "C:\me\home" => "C:\me\"
s32     A_StripFileFromPath( char* pszFullPathAndFile );

char*   GetRelativePath( char* pszBasePath, char* pszBasePathAndMore );

//-------------------------------------------------------------------
//  Function  : INI_GetKeyStringSize
//  Purpose   : Retrieve the string size of the requested INI file key 
//
//  Returns   : (0) If success.
//  Inputs    : pszDataFile, Name of INI file.
//              pszSectionName, Name of section.
//              pszKeyName, Name of key within section.
//  Outputs   : pdwKeyDataSize, the size of the keydata.
//...................................................................
s32 INI_GetKeyStringSize( char *pszDataFile, char *pszSectionName, char *pszKeyName, u32 *pdwKeyDataSize );

//-------------------------------------------------------------------
//  Function  : INI_ReadString
//  Purpose   : To read a string from INI file.  32bit, Created since SDK
//              profile strings have become obsolete, forcing registry use.
//
//  Returns   : (0) If success.
//  Inputs    : pszDataFile, Name of INI file.
//              pszSectionName, Name of section.
//              pszKeyName, Name of key within section.
//				pszKeyDataSize, the size of the input buffer
//  Outputs   : pszKeyData, Storage area for key data.
//...................................................................
s32 INI_ReadString( char *pszDataFile, char *pszSectionName, char *pszKeyName , char *pszKeyData, u32 dwKeyDataSize );

s32  INI_ReadInt32( char *pszDataFile, char *pszSectionName, char *pszKeyName , s32 *piKeyData );

s32  INI_ReadU32( char *pszDataFile, char *pszSectionName, char *pszKeyName, u32 *piKeyData );

//-------------------------------------------------------------------
//  Function  : INI_WriteString
//  Purpose   : To write a string to an INI file.  32bit, Created since SDK
//              profile strings have become obsolete, forcing registry use.
//
//  Returns   : (0) If success.
//  Inputs    : pszDataFile, Name of INI file.
//              pszSectionName, Name of section.
//              pszKeyName, Name of key within section.
//              pszKeyData, String data to be associated with key.
//  Outputs   : None.
//...................................................................
u32 INI_WriteString( char *pszDataFile, char *pszSectionName, char *pszKeyName , const char *pszKeyData );

u32 INI_WriteInt32( char *pszDataFile, char *pszSectionName, char *pszKeyName , s32 iKeyData );

u32 INI_WriteU32( char *pszDataFile, char *pszSectionName, char *pszKeyName , u32 iKeyData );

//Returns 1 if section exists, 0 if not
u32 INI_DoesSectionExist
        ( char* pszFileName, char* pszSectionName
        , u32* pdwFoundSection 
        , u32* pdwOptGetFileOffset
        );




//###################################################################
//- TEXT I/O
//###################################################################


//-------------------------------------------------------------------
//	Function  : AC_StringSearchAndReplace
//	Purpose   : To replace a string with another string, sub strings need not be NULL terminated.
//
//  Returns   : None.
//	Inputs    : pSourceBuffer, a NULL terminated source buffer to search in.
//              pSourceSubString, the string to search for in the pSourceBuffer
//              pDestSubString, the string to replace each occurance of pSourceSubString.
//  Outputs   : pDestBuffer, the dest buffer to place the processed pSourceBuffer data.
//...................................................................
void AC_StringSearchAndReplace
	(char* pSourceBuffer, char* pDestBuffer, char* pSourceSubString, char* pDestSubString);

void AC_strcatbyte( char* pszDestString, char byAppendor );

//-------------------------------------------------------------------
//  Function  : AC_TextGetStartPrevLine
//  Purpose   : To locate the start of the previous line, using pTextBuffer & '\n' as a guide.
//
//  Returns   : (0) If success, (1) If no more lines to scroll back to.
//  Inputs    : pTextBuffer, the entire text buffer to scroll around in.
//              lCurrentOffset, current location within the text buffer.
//  Outputs   : plNewOffset, new location
//...................................................................
s32 AC_TextGetStartPrevLine( char* pTextBuffer , s32 lCurrentOffset, s32* plNewOffset );

//-------------------------------------------------------------------
//  Function  : AC_TextGetStartNextLine
//  Purpose   : To locate the start of the next line, using '\0' & '\n' as a guide.
//
//  Returns   : (0) If success, (1) If no more lines to scroll to.
//  Inputs    : pTextBuffer, the entire text buffer to scroll around in.
//              lCurrentOffset, current location within the text buffer.
//  Outputs   : plNewOffset, new location
//...................................................................
s32 AC_TextGetStartNextLine( char* pTextBuffer , s32 lCurrentOffset, s32* plNewOffset );

//-------------------------------------------------------------------
//  Function  : AC_TextGetWordsBetFit
//  Purpose   : To get an acceptable text word boundary length that fits within a character limit.
//
//  Returns   : (0) If success.
//  Inputs    : pTextBuffer, The text string to be scanned.
//              iMaxCharsToAccept, The maximum number of characters that should be returned.
//  Outputs   : piGetBestFitLength, The length of the text buffer that best fits max length.
//...................................................................
s32 AC_TextGetWordsBetFit( char* pTextBuffer , s32 iMaxCharsToAccept, s32* piGetBestFitLength );

//-------------------------------------------------------------------
//  Function  : A_TextAbsorbWhitespace
//  Purpose   : To count the number of whitespace characters until a non-whitespace.
//              Ex: ("" has len 0), ("  E" has len 2), ("   " has len 3 )
//
//  Returns   : (0) If success.
//  Inputs    : pTextBuffer, The source buffer for the text string.
//  Out       : piGetWhitespaceLen, Stuffed with the number of whitespace characters.
//				piGetNewlineCount, Stuffed with the number of embedded newlines, can be NULL.
//...................................................................
s32 A_TextAbsorbWhitespace( char* pTextBuffer, s32* piGetWhitespaceLen , s32* piGetNewlineCount );

//-------------------------------------------------------------------
// Fn    : A_TextGetLenToNextToken
// Desc  : To get whitespace and token information in a text stream.  Ex:("  abc" start=2,len=3)
//         ("" start=0,len=0), (" " start=1,len=0), ("  abc def", start=2,len=3)
//
// Ret   : (0) If success.
// In    : pTextBuffer, The source buffer for the text string.
// Out   : piGetLenToNextTokenStart, Stuffed with number of whitespace chars before token or end of buffer.
//         piGetNextTokenLen, Stuffed with the length of the token if exists.
//		   piGetNewlineCount, Stuffed with number of newlines prior to token, can be NULL.
//...................................................................
s32     A_TextGetLenToNextToken
	( char* pTextBuffer, s32* piGetLenToNextTokenStart, s32* piGetNextTokenLen, s32* piGetNewlineCount );


u32     A_strlenToNewline( char* pTextBuffer );

//Insures no ending newline, by filling the newline characters with NULL.
u32     A_InsureNoEndingNewLine( char* pString );

//-------------------------------------------------------------------
//  Function  : FileCopyPortion
//  Purpose   : To copy a portion of one file to a portion of another.
//
//  Returns   : (0) If success, 1 if bad source, 2 if can't open dest.
//  Inputs    : pszSourceFile, The file name of the source file.
//              lSourceOffset, The starting location to copy from in the source file.
//              pszDestFile, The file name of the destination file.
//              lDestOffset, The location within the destination file to start copying to.
//              lTotalBytes, The total number of bytes to copy from source to dest.
//  Outputs   : None.
//...................................................................
s32 FileCopyPortion( char *pszSourceFile, s32 lSourceOffset, char *pszDestFile, s32 lDestOffset , s32 lTotalBytes );

//###################################################################
//- TRULY MISCELLANEOUS
//###################################################################


/*-------------------------------------------------------------------
 * Function: CreateHexDumpLine
 * Desc    : Creates a string with hex bytes and optionally readable ASCII
 *           characters on right side.  Does not add a newline, processes
 *           up to one line of data.
 *
 * Returns : (0) If success.
 * In      : pSourceData, the data stream to be displayed as hex dump.
 *           dwSourceSize, the size of the data stream
 * Out     : pszOutputBuffer, buffer to write a null terminated hex dump string.
 *           pdwNumBytesEaten, number of bytes from the dwSourceSize processed.
 */
u32 CreateHexDumpLine
        ( void* pSourceData, u32 dwSourceSize
        , char* pszOutputBuffer
        , u32*  pdwNumBytesEaten
        );

//-------------------------------------------------------------------
#ifdef __cplusplus
    	}
#endif
//-------------------------------------------------------------------
#endif	//For file _MISC_AID_H_
//-------------------------------------------------------------------


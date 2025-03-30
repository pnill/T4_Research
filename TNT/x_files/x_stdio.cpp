////////////////////////////////////////////////////////////////////////////
//
//  X_STDIO.CPP
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif


#if defined( TARGET_PC ) || defined( TARGET_XBOX )
#include <stdio.h>
#include <string.h>
#endif


////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PS2_DEV
    #define USE_BUFFER  // Use fprintf output buffer
#endif

#if defined( TARGET_DOLPHIN )
    #if defined( __MWERKS__ ) || defined( __SN__ )
        //Under Metrowerks compiler there are linker problems with x_vsprintf.
        //Under SN compiler something appears to be wrong with x_vsprintf, but vsprintf
        //   works fine.

        //Please remove when problem(s) resolved.
        #define x_vsprintf  vsprintf
    #endif

    #undef x_sprintf
#endif

// This must be enabled in ALL game console release submissions(no TTY text output)
// Remove only for debugging purposes on release builds.
#if !defined( X_DEBUG) && !defined( TARGET_PC )
    #define DEBUG_TEXT_OFF
#endif


//--------------------------------------------------------------------------
// Static buffer for any internally formatted output string manipulations.
//--------------------------------------------------------------------------

#ifdef _CONSOLE
#define STATIC_BUF_SIZE         1024*4      // Give Console apps more buffer space
#else
#define STATIC_BUF_SIZE         512
#endif

#define MAGIC_SAFETY_NUMBER     0x12345678


////////////////////////////////////////////////////////////////////////////
//  FUNCTION DECLARATIONS
////////////////////////////////////////////////////////////////////////////

void DUMMY_Print  ( const char* pString );
void DUMMY_PrintAt( const char* pString, s32 X, s32 Y );


////////////////////////////////////////////////////////////////////////////
//  LOCAL STORAGE
////////////////////////////////////////////////////////////////////////////

static xbool s_Initialized = FALSE;             // Flag if x_stdio is initialized.
static xbool s_bAllowCDDriveAccess = TRUE;

static char  s_Buf1[ STATIC_BUF_SIZE ];         // Static internal buffer.
static u32   s_Safety1 = MAGIC_SAFETY_NUMBER;   // Magic value after s_Buf1.
static char  s_SafetyPadding[28];               // Padding between s_Buf1 and s_Buf2 in case of overflow.
static char  s_Buf2[ STATIC_BUF_SIZE ];         // Static internal buffer(for fs and vfs).
static u32   s_Safety2 = MAGIC_SAFETY_NUMBER;   // Magic value after s_Buf2.

// NOTE:  The s_Safety* variables are used to determine if operations have
//        overrun the static buffers.  s_Safety* must be placed immediately
//        after its corresponding s_Buf*.

//--------------------------------------------------------------------------
//  Pointers to functions supplied by the application.
//--------------------------------------------------------------------------

static print_fnptr      Print       = DUMMY_Print;
static print_at_fnptr   PrintAt     = DUMMY_PrintAt;

static open_fnptr       Open        = NULL;
static close_fnptr      Close       = NULL;
static read_fnptr       Read        = NULL;
static reada_fnptr      ReadA       = NULL;
static status_fnptr     ReadAStatus = NULL;
static cancel_fnptr     ReadACancel = NULL;
static write_fnptr      Write       = NULL;
static seek_fnptr       Seek        = NULL;
static tell_fnptr       Tell        = NULL;
static eof_fnptr        FnEOF       = NULL;


////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

// Function "fs" or "Format String".
char* fs( const char* FormatStr, ... )
{
    ASSERT( s_Initialized );

    x_va_list  Args;
    x_va_start( Args, FormatStr );

    s32 StrLen;

    StrLen = x_vsprintf( s_Buf2, FormatStr, Args );

    ASSERT( s_Safety2 == MAGIC_SAFETY_NUMBER );
    ASSERT( StrLen <= STATIC_BUF_SIZE );

    return( s_Buf2 );
}

//==========================================================================

char* vfs( const char* FormatStr, x_va_list Args )
{
    ASSERT( s_Initialized );

    s32 StrLen;

    StrLen = x_vsprintf( s_Buf2, FormatStr, Args );

    ASSERT( s_Safety2 == MAGIC_SAFETY_NUMBER );
    ASSERT( StrLen <= STATIC_BUF_SIZE );

    return( s_Buf2 );
}


//==========================================================================
// dummy print routines for RELEASE or just no text...
//==========================================================================
void DUMMY_Print( const char* /*pString*/ )
{

}

void DUMMY_PrintAt( const char* /*pString*/, s32 /*X*/, s32 /*Y*/ )
{

}

//==========================================================================

void x_InitSTDIO( void )
{
    if( s_Initialized )
        return;

    s_Initialized = TRUE;

    // This assignment is redundant, unless x_STDIO is killed
    // then re-initialized in the life of the client application.
    s_Safety1 = MAGIC_SAFETY_NUMBER;
    s_Safety2 = MAGIC_SAFETY_NUMBER;

    for( u32 i = 0; i < sizeof(s_SafetyPadding); i++ )
        s_SafetyPadding[i] = (char)i;

#if defined( _CONSOLE )
    x_SetPrintHandlers( (print_fnptr)printf,
                        DUMMY_PrintAt );
#endif
}

//==========================================================================

void x_KillSTDIO( void )
{
    ASSERT( s_Initialized );

    s_Initialized = FALSE;

    x_SetIOHandlers   ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
    x_SetPrintHandlers( DUMMY_Print, DUMMY_PrintAt );
}

//==========================================================================

s32 x_printf( const char* FormatStr, ... )
{
#ifdef DEBUG_TEXT_OFF
    return( 0 );
#else
    ASSERT( s_Initialized );

    x_va_list  Args;
    x_va_start( Args, FormatStr );

    s32 StrLen = x_vsprintf( s_Buf1, FormatStr, Args );

    ASSERT( s_Safety1 == MAGIC_SAFETY_NUMBER );
    ASSERT( Print );
    Print ( s_Buf1 );
    return( StrLen );
#endif
}

//==========================================================================

s32 x_printfxy( s32 x, s32 y, const char* pFormatStr, ... )
{
#ifdef DEBUG_TEXT_OFF
    return( 0 );
#else
    ASSERT( s_Initialized );

    x_va_list  Args;
    x_va_start( Args, pFormatStr );

    s32 StrLen = x_vsprintf( s_Buf1, pFormatStr, Args );

    ASSERT ( s_Safety1 == MAGIC_SAFETY_NUMBER );
    ASSERT ( PrintAt );
    PrintAt( s_Buf1, x, y );
    return ( StrLen );
#endif
}

//==========================================================================

s32 x_sprintf( char* pStr, const char* pFormatStr, ... )
{
    ASSERT( s_Initialized );

    x_va_list  Args;
    x_va_start( Args, pFormatStr );

    s32 StrLen = x_vsprintf( pStr, pFormatStr, Args );

    ASSERT( s_Safety1 == MAGIC_SAFETY_NUMBER );

    return( StrLen );
}

//==========================================================================

#ifdef USE_BUFFER

#define FPRINTF_BUFFER_SIZE (16*1024)

static X_FILE* s_CurrentBufferFP = NULL;
static s32     s_BufferIndex     = 0;
static char    s_Buffer[ FPRINTF_BUFFER_SIZE ];

//--------------------------------------------------------------------------

static void FlushBuffer( X_FILE* fp )
{
    if( (s_CurrentBufferFP==fp) && s_BufferIndex )
    {
        x_fwrite( s_Buffer, 1, s_BufferIndex, s_CurrentBufferFP );
        s_BufferIndex = 0;
    }
}

//--------------------------------------------------------------------------

static void AddToBuffer( X_FILE* fp, char* Str )
{
    // Switch owners if needed
    if( fp != s_CurrentBufferFP )
    {
        FlushBuffer( fp );
        s_CurrentBufferFP = fp;
    }

    // Add data to buffer
    while( (*Str) )
    {
        // Check if string doesn't fit
        if( s_BufferIndex == FPRINTF_BUFFER_SIZE )
            FlushBuffer( fp );

        // Add characters
        s_Buffer[ s_BufferIndex ] = (*Str);
        s_BufferIndex++;
        Str++;
    }
}

//--------------------------------------------------------------------------

static void CloseBuffer( X_FILE* fp )
{
    FlushBuffer( fp );
    s_CurrentBufferFP = NULL;
}

#endif //USE_BUFFER

//==========================================================================

s32 x_fprintf( X_FILE* pFile, const char* pFormatStr, ... )
{
    ASSERT( s_Initialized );

    x_va_list  Args;
    x_va_start( Args, pFormatStr );

    // Print to our internal buffer.
    s32 StrLen = x_vsprintf( s_Buf1, pFormatStr, Args );

    ASSERT( s_Safety1 == MAGIC_SAFETY_NUMBER );

#ifdef USE_BUFFER
    AddToBuffer( pFile, s_Buf1 );
    return( StrLen );
#else
    // Send the buffer out.
    return( x_fwrite( s_Buf1, 1, StrLen, pFile ) );
#endif
}

//==========================================================================

X_FILE* x_fopen( const char* pFilename, const char* pMode )
{
    ASSERT( s_Initialized );

    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return NULL;
    }

  #if defined( TARGET_PC )
    char NewMode[10];
    x_sprintf( NewMode, "%sb", pMode );  // force it into binary mode
    return( (X_FILE*)fopen( pFilename, NewMode ) );
  #else
    ASSERT( Open );
    return( Open( pFilename, pMode ) );
  #endif
}

//==========================================================================

void x_fclose( X_FILE* pFile )
{
    ASSERT( s_Initialized );
    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
    }

#ifdef USE_BUFFER
    CloseBuffer( pFile );
#endif

  #if defined( TARGET_PC )
    fclose( (FILE*)pFile );
  #else
    ASSERT( Close );
    Close( pFile );
  #endif
}

//==========================================================================

s32 x_fflush( X_FILE* pFile )
{
    ASSERT( s_Initialized );

#ifdef USE_BUFFER
    FlushBuffer( pFile );
#endif

  #if defined( TARGET_PC )
    return( fflush( (FILE*)pFile ) );
  #else
    pFile = NULL;   // This is here to surpress a warning.
    return( 0 );
  #endif
}

//==========================================================================

s32 x_fread( void* pBuffer, s32 Size, s32 Count, X_FILE* pFile )
{
    ASSERT( s_Initialized );

    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return 0;
    }

    if( (Size * Count) == 0 )
        return( 0 );

#if defined( TARGET_PC )
    {
        s32 ReturnCount;
        ReturnCount = (s32)fread( pBuffer, Size, Count, (FILE*)pFile );
        
        if( ReturnCount != Count )
        {
            char ErrorString[50];
            if( ferror((FILE*)pFile) )
                x_sprintf(ErrorString, "FILE ERROR: %s.\n", strerror(NULL));
            else if( feof((FILE*)pFile) )
                x_sprintf(ErrorString, "FILE ERROR: End of File Reached.\n");
            else
                x_sprintf(ErrorString, "FILE ERROR: Not sure what the hell happened.\n");
//            ASSERTS( ReturnCount == Count, ErrorString );
        }
        return ReturnCount;
    }
#else
    ASSERT( Read );
    s32 BytesRead = Read( pFile, (byte*)pBuffer, Size*Count );
    return( BytesRead / Size );
#endif
}

//==========================================================================

s32 x_fwrite( void* pBuffer, s32 Size, s32 Count, X_FILE* pFile )
{
    ASSERT( s_Initialized );
    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return 0;
    }

  #if defined( TARGET_PC )
    return( fwrite( pBuffer, Size, Count, (FILE*)pFile ) );
  #else
    ASSERT( Write );
    s32 Bytes = Write( pFile, (byte*)pBuffer, (Size * Count) );
    return( Bytes / Size );
  #endif
}

//==========================================================================

s32 x_fseek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    ASSERT( s_Initialized );
    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return 0;
    }

  #if defined( TARGET_PC )
    return( fseek( (FILE*)pFile, Offset, Origin ) );
  #else
    ASSERT( Seek );
    return( Seek( pFile, Offset, Origin ) );
  #endif
}

//==========================================================================

s32 x_ftell( X_FILE* pFile )
{
    ASSERT( s_Initialized );
    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return 0;
    }

  #if defined( TARGET_PC )
    return( ftell( (FILE*)pFile ) );
  #else
    ASSERT( Tell );
    return( Tell( pFile ) );
  #endif
}

//==========================================================================

s32 x_feof( X_FILE* pFile )
{
    ASSERT( s_Initialized );
    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return 0;
    }

  #if defined( TARGET_PC )
    return( feof( (FILE*)pFile ) );
  #else
    ASSERT( FnEOF );
    return( FnEOF( pFile ) );
  #endif
}

//==========================================================================

s32 x_fgetc( X_FILE* pFile )
{
    ASSERT( s_Initialized );
    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return 0;
    }

  #if defined( TARGET_PC )
    return( fgetc( (FILE*)pFile ) );
  #else
    char Char;
    s32  Bytes = x_fread( &Char, 1, 1, pFile );

    if( Bytes == 0 )
        return( (s32)X_EOF );
    else
        return( (s32)Char );
  #endif
}

//==========================================================================

s32 x_fputc( s32 C, X_FILE* pFile )
{
    ASSERT( s_Initialized );
    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return 0;
    }

  #if defined( TARGET_PC )
    return fputc( C, (FILE*)pFile );
  #else
    char Char = (char)C;
    s32 Bytes = x_fwrite( &Char, 1, 1, pFile );
    if( Bytes == 0 )
        return( (s32)X_EOF );
    else
        return( (s32)Char );
  #endif
}

//==========================================================================

s32 x_freada( void*   pBuffer,
              s32     Size,
              s32     Count,
              X_FILE* pFile,
              s32     FileOffset,
              s32     Priority,
              s32&    TaskID )
{
    ASSERT( s_Initialized );
    if( !s_bAllowCDDriveAccess )
    {
        ASSERT( 0 );
        return 0;
    }

    if( (Size * Count) == 0 )
        return( 0 );

  #if defined( TARGET_PC )
    fread( pBuffer, Size, Count, (FILE*)pFile );
    return( Count );
  #else
    ASSERT( ReadA );

    return( ReadA( pBuffer, Size*Count, pFile, FileOffset, Priority, TaskID ) );
  #endif
}

//==========================================================================

s32 x_freadastatus( s32 TaskID, s32& BytesRead )
{
    ASSERT( s_Initialized );

    // If drive access has been halted, please do not read anything,just identify that the
    // read is still in progress.
    if( !s_bAllowCDDriveAccess )
        return X_STATUS_INPROGRESS;

  #if defined( TARGET_PC )
    BytesRead = 0;
    return( X_STATUS_COMPLETE );
  #else
    ASSERT( ReadAStatus );
    return ReadAStatus( TaskID, BytesRead );
  #endif
}

//==========================================================================

s32 x_freadacancel( s32 TaskID )
{
    ASSERT( s_Initialized );

  #if defined( TARGET_PC )
    return( X_STATUS_COMPLETE );
  #else
    ASSERT( ReadACancel );
    return ReadACancel( TaskID );
  #endif
}

//=====================================================================================================================================
xbool x_SetDVDDriveAccess( xbool bEnable )
{
    xbool bPreviousState;

    bPreviousState = s_bAllowCDDriveAccess;
    s_bAllowCDDriveAccess = bEnable;    

    return bPreviousState;
}

//==========================================================================

void x_SetIOHandlers( open_fnptr    OpenFnPtr,
                      close_fnptr   CloseFnPtr,
                      read_fnptr    ReadFnPtr,
                      reada_fnptr   ReadAFnPtr,
                      status_fnptr  ReadAStatusFnPtr,
                      cancel_fnptr  ReadACancelFnPtr,
                      write_fnptr   WriteFnPtr,
                      seek_fnptr    SeekFnPtr,
                      tell_fnptr    TellFnPtr,
                      eof_fnptr     EOFFnPtr )
{
    Open        = OpenFnPtr;
    Close       = CloseFnPtr;
    Read        = ReadFnPtr; 
    ReadA       = ReadAFnPtr; 
    ReadAStatus = ReadAStatusFnPtr; 
    ReadACancel = ReadACancelFnPtr;
    Write       = WriteFnPtr;
    Seek        = SeekFnPtr; 
    Tell        = TellFnPtr; 
    FnEOF       = EOFFnPtr;
}

//==========================================================================

void x_GetIOHandlers( open_fnptr&    OpenFnPtr,
                      close_fnptr&   CloseFnPtr,
                      read_fnptr&    ReadFnPtr,
                      reada_fnptr&   ReadAFnPtr,
                      status_fnptr&  ReadAStatusFnPtr,
                      cancel_fnptr&  ReadACancelFnPtr,
                      write_fnptr&   WriteFnPtr,
                      seek_fnptr&    SeekFnPtr,
                      tell_fnptr&    TellFnPtr,
                      eof_fnptr&     EOFFnPtr )
{
    OpenFnPtr        = Open;
    CloseFnPtr       = Close;
    ReadFnPtr        = Read;
    ReadAFnPtr       = ReadA;
    ReadAStatusFnPtr = ReadAStatus;
    ReadACancelFnPtr = ReadACancel;
    WriteFnPtr       = Write;
    SeekFnPtr        = Seek;
    TellFnPtr        = Tell;
    EOFFnPtr         = FnEOF;
}

//==========================================================================

void x_SetPrintHandlers( print_fnptr    PrintFnPtr,
                         print_at_fnptr PrintAtFnPtr )
{
    if( PrintFnPtr != NULL )
        Print = PrintFnPtr;
    else
        Print = DUMMY_Print;

    if( PrintAtFnPtr != NULL )
        PrintAt = PrintAtFnPtr;
    else
        PrintAt = DUMMY_PrintAt;
}

//==========================================================================
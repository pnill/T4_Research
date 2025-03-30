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
#define CUSTOM_FILE_IO_HANDLER
#include "x_stdio.hpp"
#endif

#if defined(TARGET_PC) || defined(TARGET_XBOX)
#include <stdio.h>
#endif

#ifdef TARGET_PS2
#include <sifdev.h>
#include <stdio.h>
#endif

////////////////////////////////////////////////////////////////////////////
//  DEFINITIONS
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PS2
#define USE_BUFFER  // Use fprintf output buffer
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

static s32  Initialized = 0;
static char Buf[ STATIC_BUF_SIZE ];         // Static internal buffer.
static u32  Safety = MAGIC_SAFETY_NUMBER;   // Magic value after Buf.
static s32  StrLen = 0;                     // Length of string in Buf.
static s32	Locked = -1;					// Thread-management mutex

// NOTE:  The Safety variable is used to determine if operations have
//        overrun the static buffer.  Safety must be placed immediately
//        after the Buf.

////////////////////////////////////////////////////////////////////////////
//  LOCAL STORAGE
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
//  Storage for sizes of async reads. (Used by x_freadastatus)
//
static s32 AsyncReadSizes[50];


//--------------------------------------------------------------------------
//  Pointers to functions supplied by the application.
//--------------------------------------------------------------------------

static print_fnptr      Print       = NULL;
static print_at_fnptr   PrintAt     = NULL;

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
// THREAD MANAGEMENT ROUTINES
////////////////////////////////////////////////////////////////////////////

inline void x_LockStdIO( s32 LineNumber )
{
	while (Locked != -1);
	Locked = LineNumber;
}

inline void x_UnLockStdIO( void )
{
	Locked = -1;
}

#define LOCK_X_STDIO()		x_LockStdIO( __LINE__ )
#define UNLOCK_X_STDIO()	x_UnLockStdIO()


////////////////////////////////////////////////////////////////////////////
//  FUNCTION DECLARATIONS
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PS2_DEV

X_FILE* PS2_DEV_Open        ( const char* pFilename, const char* pMode );
void    PS2_DEV_Close       ( X_FILE* pFile );
s32     PS2_DEV_Read        ( X_FILE* pFile, byte* pBuffer, s32 Bytes );
s32     PS2_DEV_Write       ( X_FILE* pFile, byte* pBuffer, s32 Bytes );
s32     PS2_DEV_Seek        ( X_FILE* pFile, s32 Offset, s32 Origin );
s32     PS2_DEV_Tell        ( X_FILE* pFile );
xbool   PS2_DEV_EOF         ( X_FILE* pFile );
s32		PS2_DEV_ReadA		( void*   pBuffer,
							  s32     Bytes,
							  X_FILE* pFile,
							  s32     FileOffset,
							  s32     Priority,
							  s32&    TaskID );
s32     PS2_DEV_ReadAStatus ( s32 TaskID, s32& BytesRead );
void    PS2_DEV_ReadACancel ( s32 TaskID );

#endif

#ifdef TARGET_PS2

void    PS2_DEV_Print   ( const char* pString );
void    PS2_DEV_PrintAt ( const char* pString, s32 X, s32 Y );

#endif


////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

//==========================================================================
// Function "fs" or "Format String".

char* fs( const char* FormatStr, ... )
{
    x_va_list  Args;
    x_va_start( Args, FormatStr );
    StrLen = x_vsprintf( Buf, FormatStr, Args );
    ASSERT( Safety == MAGIC_SAFETY_NUMBER );
    ASSERT( StrLen <= STATIC_BUF_SIZE );
    return( Buf );
}

//==========================================================================

char* vfs( const char* FormatStr, x_va_list Args )
{
    StrLen = x_vsprintf( Buf, FormatStr, Args );
    ASSERT( Safety == MAGIC_SAFETY_NUMBER );
    ASSERT( StrLen <= STATIC_BUF_SIZE );
    return( Buf );
}


//==========================================================================
// dummy print routines for RELEASE or just no text...
//==========================================================================
void DUMMY_Print( const char* pString )
{
}

void DUMMY_PrintAt( const char* pString, s32 X, s32 Y )
{
//    X = Y   = 0;        // This is here to surpress a warning.
//    pString = NULL;     // This is here to surpress a warning.
}

//==========================================================================
//==========================================================================

void x_InitSTDIO( void )
{
    if( Initialized++ )  
        return;

	x_memset( AsyncReadSizes, -1, sizeof(AsyncReadSizes) );

    // This assignment is redundant; kills a warning in release builds.
    Safety = MAGIC_SAFETY_NUMBER;

	// setup some dummy handlers
    x_SetPrintHandlers( DUMMY_Print, DUMMY_PrintAt );
    
#ifdef TARGET_PS2_DEV
    x_SetIOHandlers   ( PS2_DEV_Open,
                        PS2_DEV_Close,
                        PS2_DEV_Read,
                        PS2_DEV_ReadA,
                        PS2_DEV_ReadAStatus,
						PS2_DEV_ReadACancel,
                        PS2_DEV_Write,
                        PS2_DEV_Seek,
                        PS2_DEV_Tell,
                        PS2_DEV_EOF );
#endif
#ifdef TARGET_PS2
    x_SetPrintHandlers( PS2_DEV_Print, 
                        PS2_DEV_PrintAt );
#endif
#ifdef _CONSOLE
    x_SetPrintHandlers( (print_fnptr)printf,
                        DUMMY_PrintAt );
#endif
}

//==========================================================================

void x_KillSTDIO( void )
{
    ASSERT( Initialized-- );

//#ifdef TARGET_PS2
    x_SetIOHandlers   ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
    x_SetPrintHandlers( NULL, NULL );
//#endif
}

//==========================================================================
//==========================================================================

s32 x_printf( const char* FormatStr, ... )
{
#ifndef DEBUG_TEXT_OFF

    x_va_list  Args;
    ASSERT( Initialized );
    x_va_start( Args, FormatStr );
    
//#if defined( TARGET_PC )
//        // The compiler generously gives us a simple route.
//        return( vprintf( FormatStr, Args ) );
//#else
    
        // Must use custom text handler.
        ASSERT( Print );
        StrLen = x_vsprintf( Buf, FormatStr, Args );
        ASSERT( Safety == MAGIC_SAFETY_NUMBER );
        Print ( Buf );
        return( StrLen );
        
//#endif

#else
    return( 0 );
#endif
}

//==========================================================================

s32 x_printfxy( s32 x, s32 y, const char* pFormatStr, ... )
{
#ifndef DEBUG_TEXT_OFF

    x_va_list  Args;
    ASSERT( Initialized );
    x_va_start( Args, pFormatStr );

//#if defined( TARGET_PC )
//
//        // TODO: Must insert code here to move the cursor in a console window.
//        return( 0 );
//
//#else

        // Must use custom text handler.
        ASSERT( PrintAt );
        StrLen = x_vsprintf( Buf, pFormatStr, Args );
        ASSERT( Safety == MAGIC_SAFETY_NUMBER );
        PrintAt( Buf, x, y );
        return( StrLen );

//#endif

#else
    return( 0 );
#endif
}

//==========================================================================

s32 x_sprintf( char* pStr, const char* pFormatStr, ... )
{
    x_va_list  Args;
    ASSERT( Initialized );
    x_va_start( Args, pFormatStr );
    StrLen = x_vsprintf( pStr, pFormatStr, Args );
    ASSERT( Safety == MAGIC_SAFETY_NUMBER );
    return( StrLen );
}

//==========================================================================

#ifdef USE_BUFFER

#define FPRINTF_BUFFER_SIZE (16*1024)

static X_FILE* s_CurrentBufferFP=NULL;
static char    s_Buffer[ FPRINTF_BUFFER_SIZE ];
static s32     s_BufferIndex=0;

static void FlushBuffer( X_FILE* fp )
{
    if( (s_CurrentBufferFP==fp) && s_BufferIndex )
    {
        x_fwrite( s_Buffer, 1, s_BufferIndex, s_CurrentBufferFP );
        s_BufferIndex = 0;
    }
}

static void AddToBuffer( X_FILE* fp, char* Str )
{
    // Switch owners if needed
    if( fp != s_CurrentBufferFP )
    {
        FlushBuffer(fp);
        s_CurrentBufferFP = fp;
    }

    // Add data to buffer
    while( (*Str) )
    {
        // Check if string doesn't fit
        if( s_BufferIndex == FPRINTF_BUFFER_SIZE )
            FlushBuffer(fp);

        // Add characters
        s_Buffer[s_BufferIndex++] = (*Str);
        Str++;
    }
}

static void CloseBuffer( X_FILE* fp )
{
    FlushBuffer( fp );
    s_CurrentBufferFP = NULL;
}

#endif

//==========================================================================

s32 x_fprintf( X_FILE* pFile, const char* pFormatStr, ... )
{
    x_va_list  Args;
    s32        StrLen;

    ASSERT( Initialized );
    x_va_start( Args, pFormatStr );

    // Print to our internal buffer.
    StrLen = x_vsprintf( Buf, pFormatStr, Args );    
    ASSERT( Safety == MAGIC_SAFETY_NUMBER );

#ifdef USE_BUFFER

    AddToBuffer( pFile, Buf );
    return StrLen;

#else

    // Send the buffer out.
    return( x_fwrite( Buf, 1, StrLen, pFile ) );

#endif
}


//==========================================================================

X_FILE* x_fopen( const char* pFilename, const char* pMode )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    return( (X_FILE*)fopen( pFilename, pMode ) );
#else

    ASSERT( Open );
	X_FILE* pFile = Open( pFilename, pMode );

    return( pFile );
#endif
}

//==========================================================================

void x_fclose( X_FILE* pFile )
{
    ASSERT( Initialized );

#ifdef USE_BUFFER
    CloseBuffer(pFile);
#endif



#if defined(TARGET_PC) || defined(TARGET_XBOX)
    fclose( (FILE*)pFile );
#else
    ASSERT( Close );

    Close( pFile );
#endif
}

//==========================================================================

s32 x_fflush( X_FILE* pFile )
{
    ASSERT( Initialized );

#ifdef USE_BUFFER
    FlushBuffer(pFile);
#endif

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    return( fflush( (FILE*)pFile ) );
#else
    pFile = NULL;   // This is here to surpress a warning.
    return( 0 );
#endif
}

//==========================================================================
//
// Setup function ptr table for different ways of implementing x_fread
//
//==========================================================================
typedef s32 (*x_fread_ptr)(void*, s32, s32, X_FILE*);

static s32 x_fread_default(void* pBuffer, s32 Size, s32 Count, X_FILE* pFile)
{
    return Read( pFile, (byte*)pBuffer, Size*Count );
}

static s32 x_fread_async(void* pBuffer, s32 Size, s32 Count, X_FILE* pFile)
{
    s32 TaskID;
    s32 FilePos = x_ftell(pFile);
    s32 BytesRead = ReadA( pBuffer,
        Size*Count,
        pFile,
        x_ftell(pFile),
        X_PRIORITY_HIGH,
        TaskID );
    s32 Status;
    do
    {
        Status = ReadAStatus( TaskID, BytesRead );
    } while ((Status != X_STATUS_COMPLETE) && (Status != X_STATUS_ERROR));
    
    x_fseek( pFile, FilePos + Size*Count, X_SEEK_SET );
    
    return( BytesRead / Size );
}

static x_fread_ptr x_fread_func[FREAD_NUM_OPTIONS] = {
    x_fread_default,
    x_fread_async
};

static s32 x_fread_option = FREAD_DEFAULT;

void x_fread_setup( FREAD_OPTION option)
{
    ASSERT( option >= 0 && option < FREAD_NUM_OPTIONS );

    x_fread_option = option;
}

//==========================================================================

s32 x_fread( void* pBuffer, s32 Size, s32 Count, X_FILE* pFile )
{
    ASSERT( Initialized );
	if( (Size * Count) == 0 )
        return( 0 );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    fread( pBuffer, Size, Count, (FILE*)pFile );
	return( Count );
#else
    return (*x_fread_func[x_fread_option])(pBuffer, Size, Count, pFile);
#endif
}

//==========================================================================

s32 x_fwrite( void* pBuffer, s32 Size, s32 Count, X_FILE* pFile )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    return( fwrite( pBuffer, Size, Count, (FILE*)pFile ) );
#else
    {
        ASSERT( Write );

        s32 Bytes = Write( pFile, (byte*)pBuffer, (Size * Count) );

        return( Bytes / Size );
    }
#endif
}

//==========================================================================

s32 x_fseek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    return( fseek( (FILE*)pFile, Offset, Origin ) );
#else
    ASSERT( Seek );

    s32 RPos = Seek( pFile, Offset, Origin );
	return( RPos );
#endif
}

//==========================================================================

s32 x_ftell( X_FILE* pFile )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    return( ftell( (FILE*)pFile ) );
#else
    ASSERT( Tell );
    return( Tell( pFile ) );
#endif
}

//==========================================================================

s32 x_feof( X_FILE* pFile )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    return( feof( (FILE*)pFile ) );
#else
    ASSERT( FnEOF );
    return( FnEOF( pFile ) );
#endif
}

//==========================================================================

s32 x_fgetc( X_FILE* pFile )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    return( fgetc( (FILE*)pFile ) );
#else
    {
        char Char;
        s32  Result;
        s32  Bytes;

        Bytes = x_fread( &Char, 1, 1, pFile );
        if( Bytes == 0 )
            Result = X_EOF;
        else
            Result = Char;
        return( Result );
    }
#endif
}

//==========================================================================

s32 x_fputc( s32 C, X_FILE* pFile )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    return fputc( C, (FILE*)pFile );
#else
    {
        char Char = (char)C;
        s32  Result;
        s32  Bytes;
        Bytes = x_fwrite( &Char, 1, 1, pFile );
        if( Bytes == 0 )
            Result = X_EOF;
        else
            Result = Char;
        return( Result );
    }
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
    ASSERT( Initialized );
	if( (Size * Count) == 0 )
        return( 0 );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    fread( pBuffer, Size, Count, (FILE*)pFile );
	return( Count );
#else
    {
		s32 RValue = ReadA( pBuffer,
					  Size*Count,
					  pFile,
					  FileOffset,
					  Priority,
					  TaskID );

		return( RValue );
    }
#endif
}

//==========================================================================

s32 x_freadastatus( s32 TaskID, s32& BytesRead )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
	BytesRead = 0;
    return( X_STATUS_COMPLETE );
#else
    {
		s32 RValue = ReadAStatus( TaskID, BytesRead );
		return( RValue );
    }
#endif
}

//==========================================================================

void x_freadacancel( s32 TaskID )
{
    ASSERT( Initialized );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
#else
	ReadACancel( TaskID );
#endif
}


//==========================================================================
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
    OpenFnPtr		 = Open;
    CloseFnPtr		 = Close;
    ReadFnPtr		 = Read;
    ReadAFnPtr	     = ReadA;
    ReadAStatusFnPtr = ReadAStatus;
    ReadACancelFnPtr = ReadACancel;
    WriteFnPtr		 = Write;
    SeekFnPtr		 = Seek;
    TellFnPtr		 = Tell;
    EOFFnPtr		 = FnEOF;
}

//==========================================================================

void  x_SetPrintHandlers( print_fnptr    PrintFnPtr,
                          print_at_fnptr PrintAtFnPtr )
{
    if( PrintFnPtr != NULL )
        Print   = PrintFnPtr;

    if( PrintAtFnPtr != NULL )
        PrintAt = PrintAtFnPtr;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  Functions for the PS2 under the CodeWarrior environment.
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PS2_DEV

//==========================================================================

X_FILE* PS2_DEV_Open( const char* pFilename, const char* pMode )
{
    s32   Handle;
    char  Buf[X_MAX_PATH];
    xbool Read    = FALSE;
    xbool Write   = FALSE;
    xbool Append  = FALSE;
    s32   OpenArg = 0;

    // We need to prefix the filename with "host:".
    Buf[0] = 'h';    
    Buf[1] = 'o';    
    Buf[2] = 's';    
    Buf[3] = 't';
    Buf[4] = ':';
    x_strcpy( &Buf[5], pFilename );

    // Pick through the Mode characters.
    while( *pMode )
    {
        if( (*pMode == 'r') || (*pMode == 'R') )  Read   = TRUE;
        if( (*pMode == 'w') || (*pMode == 'W') )  Write  = TRUE;
        if( (*pMode == 'a') || (*pMode == 'A') )  Append = TRUE;
        ++pMode;
    }
    
    ASSERT( Read || Write );

    // Build parameter to sceOpen based on values found in Mode.
    
    if( Read  )             OpenArg |= SCE_RDONLY;
    if( Write )             OpenArg |= SCE_WRONLY;
    if( Write )             OpenArg |= SCE_CREAT;
    if( Write && !Append )  OpenArg |= SCE_TRUNC;

    // For reference, here are the defines for the bit argument
    // to the sceOpen function:
    //          
    //  #define SCE_RDONLY      0x0001
    //  #define SCE_WRONLY      0x0002
    //  #define SCE_RDWR        0x0003
    //  #define SCE_NBLOCK      0x0010  // Non-Blocking I/O 
    //  #define SCE_APPEND      0x0100  // append (writes guaranteed at the end) 
    //  #define SCE_CREAT       0x0200  // open with file create 
    //  #define SCE_TRUNC       0x0400  // open with truncation 
    //  #define SCE_NOBUF       0x4000  // no device buffer and console interrupt 
    //  #define SCE_NOWAIT      0x8000  // asyncronous i/o 

	LOCK_X_STDIO();
    Handle = sceOpen( Buf, OpenArg );
	UNLOCK_X_STDIO();

    // If no valid handle, return NULL.
    if( Handle < 0 )
        return( NULL );

    // We may need to manually seek to the end of the file for append.
    // Check the behavior.    
        
    Handle++;
    return( (X_FILE*)Handle );
}

//==========================================================================

void PS2_DEV_Close( X_FILE* pFile )
{
	LOCK_X_STDIO();
    sceClose( ((s32)pFile)-1 );
	UNLOCK_X_STDIO();
}

//==========================================================================

s32 PS2_DEV_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
	LOCK_X_STDIO();
	s32 Size = sceRead( ((s32)pFile)-1, pBuffer, Bytes );
	UNLOCK_X_STDIO();
	return( Size );
}

//==========================================================================

s32 PS2_DEV_ReadA( void*   pBuffer,
				   s32     Bytes,
				   X_FILE* pFile,
				   s32     FileOffset,
				   s32     Priority,
				   s32&    TaskID )
{
	for (TaskID=0; TaskID < 50; TaskID++)
	{
		if (AsyncReadSizes[TaskID] == -1)
			break;
	}
	AsyncReadSizes[TaskID] = Bytes;

	LOCK_X_STDIO();
    s32 FilePos = sceLseek( ((s32)pFile)-1, 0, SCE_SEEK_CUR );

    sceLseek( ((s32)pFile)-1, FileOffset, SCE_SEEK_SET );
	sceRead( ((s32)pFile)-1, pBuffer, Bytes );

    sceLseek( ((s32)pFile)-1, FilePos, SCE_SEEK_SET );
	UNLOCK_X_STDIO();

    return( 1 );
}

//==========================================================================

s32 PS2_DEV_ReadAStatus( s32 TaskID, s32& BytesRead )
{
	if (AsyncReadSizes[TaskID] >= 0)
	{
		BytesRead = AsyncReadSizes[TaskID];
		AsyncReadSizes[TaskID] = -1;
		return X_STATUS_COMPLETE;
	}

	else
	{
		BytesRead = 0;
		return X_STATUS_NOTFOUND;
	}
}

//==========================================================================

void PS2_DEV_ReadACancel( s32 TaskID )
{
}

//==========================================================================

s32 PS2_DEV_Write( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
	LOCK_X_STDIO();
    s32 Size = sceWrite( ((s32)pFile)-1, pBuffer, Bytes );
	UNLOCK_X_STDIO();
	return( Size );
}

//==========================================================================

s32 PS2_DEV_Seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    s32 Result;
    s32 Where = 0;
    
    switch( Origin )
    {
        case X_SEEK_SET: Where = SCE_SEEK_SET; break;
        case X_SEEK_CUR: Where = SCE_SEEK_CUR; break;
        case X_SEEK_END: Where = SCE_SEEK_END; break;
    }

	LOCK_X_STDIO();
    Result = sceLseek( ((s32)pFile)-1, Offset, Where );
	UNLOCK_X_STDIO();

	if (Result == -1)   return(-1 );
	else                return( 0 );
}

//==========================================================================

s32 PS2_DEV_Tell( X_FILE* pFile )
{
	LOCK_X_STDIO();
    s32 Pos = sceLseek( ((s32)pFile)-1, 0, SCE_SEEK_CUR );
	UNLOCK_X_STDIO();
	return( Pos );
}

//==========================================================================

xbool PS2_DEV_EOF( X_FILE* pFile )
{
    // We have to do this the hard way.
    //  (1)  Save the current position.
    //  (2)  Seek to the end of the file.
    //  (3)  Save the 'end' position.
    //  (4)  Seek back to the saved position.
    //  (5)  Return comparison between original position and end position.
    //
    // NOTE:  Since this implementation is clearly inefficient, loops
    //        such as "while( !x_feof(f) )" are discouraged.

    s32 Pos;
    s32 End;

    Pos = x_ftell( pFile );     x_fseek( pFile,   0, X_SEEK_END );
    End = x_ftell( pFile );     x_fseek( pFile, Pos, X_SEEK_SET );

    return( Pos == End );
}

#endif		// TARGET_PS2_DEV

//==========================================================================
//==========================================================================

#ifdef	TARGET_PS2

void PS2_DEV_Print( const char* pString )
{
    #ifdef _MSC_VER
    pString = NULL;     // This is here to surpress a warning.
    #else
	LOCK_X_STDIO();
    printf( pString );
	UNLOCK_X_STDIO();
    #endif
}

//==========================================================================

void PS2_DEV_PrintAt( const char* pString, s32 X, s32 Y )
{
    X = Y   = 0;        // This is here to surpress a warning.
    pString = NULL;     // This is here to surpress a warning.
}

//==========================================================================

#endif	// TARGET_PS2


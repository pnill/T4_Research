///////////////////////////////////////////////////////////////////////////
//
//  X_STDIO.C
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_PLUS_H
#include "x_plus.h"
#endif

#include "x_debug.h"

#ifndef X_STDIO_H
#define CUSTOM_FILE_IO_HANDLER
#include "x_stdio.h"
#endif

///////////////////////////////////////////////////////////////////////////

#include <stdarg.h>

#ifdef TARGET_PC
#include <stdio.h>
#endif

#ifdef TARGET_N64  //##
#include <psyq.h>
#include "n64romfs.h"
#endif

#ifdef TARGET_PSX_DEV
#include <libsn.h>
#endif

/*  //##
#if defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM )
#include "ie_fsys.h"
#endif
*/

#ifdef TARGET_CLIENT
#include "SCSI_C.c"
#endif

///////////////////////////////////////////////////////////////////////////
// Static buffer for any internally formatted output string manipulations.

#define STATIC_BUF_SIZE         512
#define MAGIC_SAFETY_NUMBER     0x12345678

static s32  Initialized = 0;
static char Buf[ STATIC_BUF_SIZE ];         // Static internal buffer.
static u32  Safety = MAGIC_SAFETY_NUMBER;   // Magic value after Buf.
static s32  StrLen = 0;                     // Length of string in Buf.

// NOTE:  The Safety variable is used to determine if operations have
//        overrun the static buffer.  Safety must be placed immediately
//        after the Buf.

///////////////////////////////////////////////////////////////////////////
// CUSTOMER HANDLER FUNCTION POINTERS
///////////////////////////////////////////////////////////////////////////

static print_fnptr      Print   = NULL;
static print_at_fnptr   PrintAt = NULL;

static open_fnptr       Open    = NULL;
static close_fnptr      Close   = NULL;
static read_fnptr       Read    = NULL;
static write_fnptr      Write   = NULL;
static seek_fnptr       Seek    = NULL;
static tell_fnptr       Tell    = NULL;
static eof_fnptr        FnEOF   = NULL;

///////////////////////////////////////////////////////////////////////////
//## TEMPORARY SN I/O SUPPORT
#if defined( TARGET_PSX_DEV )
///////////////////////////////////////////////////////////////////////////

X_FILE* SNSYS_Open( const char* Filename, const char* Mode )
{
    s32     Arg;
    s32     Handle = 0;
    xbool   Read   = FALSE;
    xbool   Write  = FALSE;
    xbool   Append = FALSE;

    while( *Mode )
    {
        if( (*Mode == 'r') || (*Mode == 'R') )  Read   = TRUE;
        if( (*Mode == 'w') || (*Mode == 'W') )  Write  = TRUE;
        if( (*Mode == 'a') || (*Mode == 'A') )  Append = TRUE;
        ++Mode;
    }

    ASSERT( Read || Write );

    Arg = 2;    // Set default value.

    if( !Read &&  Write )    Arg = 1;
    if(  Read && !Write )    Arg = 0;

    Handle = PCopen( (char*)Filename, Arg, 0 );
    if( Handle == -1 )
        Handle = 0;

    if( !Handle && Write )
    {
        Handle = PCcreat( (char*)Filename, 0 );
        if( Handle == -1 )
            Handle = 0;
    }

    if( (Handle != 0) && Append )
        x_fseek( (X_FILE*)Handle, 0, X_SEEK_END );

    return( (X_FILE*)Handle );
}

//-------------------------------------------------------------------------

void SNSYS_Close( X_FILE* Stream )
{
    PCclose( (int)Stream );
}

//-------------------------------------------------------------------------

s32 SNSYS_Read( X_FILE* Stream, byte* Buffer, s32 Bytes )
{
    return( PCread( (int)Stream, (char*)Buffer, Bytes ) );
}

//-------------------------------------------------------------------------

s32 SNSYS_Write( X_FILE* Stream, byte* Buffer, s32 Bytes )
{
    return( PCwrite( (int)Stream, (char*)Buffer, Bytes ) );
}

//-------------------------------------------------------------------------

s32 SNSYS_Seek( X_FILE* Stream, s32 Offset, s32 Origin )
{
    // The PClseek function returns the absolute position after the seek.
    // The standard return for a seek function is 0=success, other=fail.
    PClseek( (int)Stream, Offset, Origin );
    return( 0 );
}

//-------------------------------------------------------------------------

s32 SNSYS_Tell( X_FILE* Stream )
{
    // There is no "ftell" function in the PsyQ library.  However, the
    // function PClseek() returns the absolute position of the result of
    // the seek.  So, if we seek to 0 bytes from the current position...
    return( PClseek( (int)Stream, 0, 1 ) );
}

//-------------------------------------------------------------------------

xbool SNSYS_EOF( X_FILE* Stream )
{
    // There is no "feof" function in the PsyQ library.
    // So, we have to do this the hard way.
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

    Pos = x_ftell( Stream );    x_fseek( Stream,   0, X_SEEK_END );
    End = x_ftell( Stream );    x_fseek( Stream, Pos, X_SEEK_SET );

    return( Pos == End );
}

///////////////////////////////////////////////////////////////////////////
//## TEMPORARY SN I/O SUPPORT
#endif
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////

char* vfs( const char* FormatStr, x_va_list Args )
{
    StrLen = x_vsprintf( Buf, FormatStr, Args );
    ASSERT( Safety == MAGIC_SAFETY_NUMBER );
    ASSERT( StrLen <= STATIC_BUF_SIZE );
    return( Buf );
}

///////////////////////////////////////////////////////////////////////////

void x_InitSTDIO( void )
{
    if( Initialized++ )  
        return;

    // This assignment is redundant; kills a warning in release builds.
    Safety = MAGIC_SAFETY_NUMBER;

#ifdef TARGET_N64

    #ifdef TARGET_N64_DEV
        PCinit();                           // PsyQ function
    #endif

    #ifdef TARGET_N64_CLIENT
        VERIFY( SCSI_Initialize() );
        SCSI_PollHost();

    #elif defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM )
		N64ROMFS_Init();
        //##
        x_SetIOHandlers( N64ROMFS_Open, 
                         N64ROMFS_Close,
                         N64ROMFS_Read, 
                         N64ROMFS_Write,
                         N64ROMFS_Seek, 
                         N64ROMFS_Tell, 
                         N64ROMFS_EOF );
        //##
    #endif

#endif

#ifdef TARGET_PSX

    #ifdef TARGET_PSX_DEV
        PCinit();                           // PsyQ function
        x_SetIOHandlers( SNSYS_Open, 
                         SNSYS_Close,
                         SNSYS_Read, 
                         SNSYS_Write,
                         SNSYS_Seek, 
                         SNSYS_Tell, 
                         SNSYS_EOF );
    #endif

#endif
}

///////////////////////////////////////////////////////////////////////////

void x_KillSTDIO ( void )
{
    ASSERT( Initialized-- );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

s32 x_printf( const char* FormatStr, ... )
{
#ifndef DEBUG_TEXT_OFF

    x_va_list  Args;
    ASSERT( Initialized );
    x_va_start( Args, FormatStr );

    #if defined( TARGET_PC_WINCON )
        // The compiler generously gives us a simple route.
        return( vprintf( FormatStr, Args ) );

    #elif defined( TARGET_CLIENT )
        // Send string back through CLIENT interface.
        StrLen = x_vsprintf( Buf, FormatStr, Args );
        ASSERT( Safety == MAGIC_SAFETY_NUMBER );
	    SCSI_PingServerWithString( Buf ) ;
        return( StrLen );

    #else
        // Must use custom text handler.
        StrLen = x_vsprintf( Buf, FormatStr, Args );
        ASSERT( Safety == MAGIC_SAFETY_NUMBER );
        if( Print )
            Print( Buf );
//      #if defined( X_DEBUG ) && (defined( TARGET_N64_DEV ) || defined( TARGET_PSX_DEV ))
//          PCwrite( -1, Buf, StrLen );
//      #endif
        return( StrLen );
    #endif

#else
    return( 0 );
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_printfxy( s32 x, s32 y, const char* FormatStr, ... )
{
#ifndef DEBUG_TEXT_OFF

    x_va_list  Args;
    ASSERT( Initialized );
    x_va_start( Args, FormatStr );

    #if defined( TARGET_PC_WINCON )
        // TODO: Must insert code here to move the cursor in a console window.
        return( 0 );

    /* TODO: Write a client which can place text anywhere on the screen.
    #elif defined( TARGET_CLIENT )
        // Send string back through CLIENT interface
        StrLen = x_vsprintf( Buf, FormatStr, Args );
        ASSERT( Safety == MAGIC_SAFETY_NUMBER );
        return( StrLen );
    */

    #else
        // Must use custom text handler.
        ASSERT( PrintAt );
        StrLen = x_vsprintf( Buf, FormatStr, Args );
        ASSERT( Safety == MAGIC_SAFETY_NUMBER );
        PrintAt( Buf, x, y );
        return( StrLen );
    #endif

#else
    return( 0 );
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_sprintf( char* Str, const char* FormatStr, ... )
{
    x_va_list  Args;
    ASSERT( Initialized );
    x_va_start( Args, FormatStr );
    StrLen = x_vsprintf( Str, FormatStr, Args );
    ASSERT( Safety == MAGIC_SAFETY_NUMBER );
    return( StrLen );
}

///////////////////////////////////////////////////////////////////////////

s32 x_fprintf( X_FILE* Stream, const char* FormatStr, ... )
{
    x_va_list  Args;
    s32        StrLen;

    ASSERT( Initialized );
    x_va_start( Args, FormatStr );

    // Print to our internal buffer.
    StrLen = x_vsprintf( Buf, FormatStr, Args );    
    ASSERT( Safety == MAGIC_SAFETY_NUMBER );

#if defined( TARGET_N64 ) || defined( TARGET_PSX )
    // This code section will expand '\n' as it writes out the buffer.
    {           
        byte*   p        = Buf;
        byte*   s        = Buf;
        s32     SubLen   = 0;
        s32     TotalLen = 0;

        // Ship the buffer out to the file.
        // Convert '\n' to '\n\r' while we're at it.

        // We will try to send the string over in as few write as possible.
        // So hold off writing until will hit a '\n' (which forces us to send
        // out a '\r') or the end.

        while( *p )
        {
            // Do we have a condition to cause us to write out the current 
            // sub-string?  That is, did we hit a '\n'?

            if( *p == '\n' )
            {
                // If the sub-string length is 0, just skip it.
                if( SubLen > 0 )
                {
                    // Write the current sub-string.
                    x_fwrite( s, 1, SubLen, Stream );
                    TotalLen += SubLen;
                    SubLen    = 0;
                    s         = p;
                }

                // Send a free '\r' now.  The '\n' will go out in the
                // next sub-string.
                x_fwrite( "\r", 1, 1, Stream );
                TotalLen += 1;
            }

            SubLen++;
            p++;            
        }

        // Write out any remaining sub-string.
        if( SubLen > 0 )
        {
            // Write the current sub-string.
            x_fwrite( s, 1, SubLen, Stream );
            TotalLen += SubLen;
        }

        return( TotalLen );
    }
#else
    // Just send the buffer out.
    return( x_fwrite( Buf, 1, StrLen, Stream ) );
#endif
}

///////////////////////////////////////////////////////////////////////////

X_FILE* x_fopen( const char* Filename, const char* Mode )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    return( (X_FILE*)fopen( Filename, Mode ) );

#elif defined( TARGET_CLIENT )
    return( (X_FILE*)SCSI_fopen( (char*)Filename, (char*)Mode ) );

#elif defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM ) || defined( TARGET_PSX ) || defined( TARGET_SDC )

    ASSERT(Open);
    return( Open( Filename, Mode ) );

#else
    ASSERT( FALSE );
    return( NULL );
#endif
}

///////////////////////////////////////////////////////////////////////////

void x_fclose( X_FILE* Stream )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    fclose( (FILE*)Stream );

#elif defined( TARGET_CLIENT )
    SCSI_fclose( Stream );

#elif defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM ) || defined( TARGET_PSX ) || defined( TARGET_SDC )
    ASSERT(Close);
    Close( Stream );

#else
    ASSERT( FALSE );
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_fflush( X_FILE* Stream )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    return( fflush( (FILE*)Stream ) );

#elif defined( TARGET_CLIENT )
    // TODO: Add this function the client server system.
    ASSERT( FALSE );
    return( 0 );

#elif defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM ) || defined( TARGET_PSX ) || defined( TARGET_SDC )
    return( 0 );

#else
    ASSERT( FALSE );
    return( 0 );
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_fread( void* Buffer, s32 Size, s32 Count, X_FILE* Stream )
{
    ASSERT( Initialized );
	if( (Size * Count) == 0 )
        return( 0 );

#if defined( TARGET_PC )
    return( fread( Buffer, Size, Count, (FILE*)Stream ) );

#elif defined( TARGET_CLIENT )
    return( SCSI_fread( Buffer, Size, Count, Stream ) );

#elif defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM ) || defined( TARGET_PSX ) || defined( TARGET_SDC )
    {
        s32 Bytes;
        ASSERT(Read);
        Bytes = Read( Stream, (byte*)Buffer, (Size * Count) );
        return( Bytes / Size );
    }

#else
    ASSERT( FALSE );
    return( 0 );
#endif

}

///////////////////////////////////////////////////////////////////////////

s32 x_fwrite( void* Buffer, s32 Size, s32 Count, X_FILE *Stream )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    return( fwrite( Buffer, Size, Count, (FILE*)Stream ) );

#elif defined( TARGET_CLIENT )
    return( SCSI_fwrite( Buffer, Size, Count, (FILE*)Stream ) );

#elif defined( TARGET_N64_DEV ) || defined( TARGET_PSX ) || defined( TARGET_SDC )
    {
        s32 Bytes;
        ASSERT(Write);
        Bytes = Write( Stream, (byte*)Buffer, (Size * Count) );
        return( Bytes / Size );
    }

#else
    ASSERT( FALSE );
    return( 0 );
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_fseek( X_FILE* Stream, s32 Offset, s32 Origin )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    return( fseek( (FILE*)Stream, Offset, Origin ) );

#elif defined( TARGET_CLIENT )
    return( SCSI_fseek( (FILE*)Stream, Offset, Origin ) );

#elif defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM ) || defined( TARGET_PSX ) || defined( TARGET_SDC )
    ASSERT(Seek);
    return( Seek( Stream, Offset, Origin ) );

#else
    ASSERT( FALSE );
    return( 0 );
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_ftell( X_FILE* Stream )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    return( ftell( (FILE*)Stream ) );

#elif defined( TARGET_CLIENT )
    return( SCSI_ftell( (FILE*)Stream ) );

#elif defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM ) || defined( TARGET_PSX ) || defined( TARGET_SDC )
    ASSERT(Tell);
    return( Tell( Stream ) );

#else
    ASSERT( FALSE );
    return( 0 );
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_feof( X_FILE* Stream )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    return( feof( (FILE*)Stream ) );

#elif defined( TARGET_CLIENT )
    {
        // There is no "feof" function in the client system.
        // So, we have to do this the hard way.
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

        Pos = x_ftell( Stream );    x_fseek( Stream,   0, X_SEEK_END );
        End = x_ftell( Stream );    x_fseek( Stream, Pos, X_SEEK_SET );

        return( Pos == End );
    }

#elif defined( TARGET_N64_DEV ) || defined( TARGET_N64_ROM ) || defined( TARGET_PSX ) || defined( TARGET_SDC )
    ASSERT(FnEOF);
    return( FnEOF( Stream ) );

#else
    ASSERT( FALSE );
    return( 0 );
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_fgetc( X_FILE* Stream )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    return( fgetc( (FILE*)Stream ) );

#else
    {
        char Char;
        s32  Result;
        s32  Bytes;

        Bytes = x_fread( &Char, 1, 1, Stream );
        if( Bytes == 0 )
            Result = X_EOF;
        else
            Result = Char;
        return( Result );
    }
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_fputc( s32 C, X_FILE* Stream )
{
    ASSERT( Initialized );

#if defined( TARGET_PC )
    return fputc( C, (FILE*)Stream );

#elif defined( TARGET_N64_ROM )
    ASSERT( FALSE );
    return( 0 );

#else
    {
        char Char = (char)C;
        s32  Result;
        s32  Bytes;
        Bytes = x_fwrite( &Char, 1, 1, Stream );
        if( Bytes == 0 )
            Result = X_EOF;
        else
            Result = Char;
        return( Result );
    }
#endif
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void  x_SetPrintHandlers( print_fnptr    PrintFnPtr,
                          print_at_fnptr PrintAtFnPtr )
{
    Print   = PrintFnPtr;
    PrintAt = PrintAtFnPtr;
}

///////////////////////////////////////////////////////////////////////////

void x_SetIOHandlers( open_fnptr    OpenFnPtr,
                      close_fnptr   CloseFnPtr,
                      read_fnptr    ReadFnPtr,
                      write_fnptr   WriteFnPtr,
                      seek_fnptr    SeekFnPtr,
                      tell_fnptr    TellFnPtr,
                      eof_fnptr     EOFFnPtr )
{
    Open    = OpenFnPtr;
    Close   = CloseFnPtr;
    Read    = ReadFnPtr; 
    Write   = WriteFnPtr;
    Seek    = SeekFnPtr; 
    Tell    = TellFnPtr; 
    FnEOF   = EOFFnPtr;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

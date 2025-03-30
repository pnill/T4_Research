////////////////////////////////////////////////////////////////////////////
//
//  X_STDIO.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_STDIO_HPP
#define X_STDIO_HPP

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////

#define X_SEEK_SET  0
#define X_SEEK_CUR  1
#define X_SEEK_END  2

#define X_EOF      -1

//--- Asynchronous Read Priorities
#define X_PRIORITY_LOW           1
#define X_PRIORITY_MEDIUM        2
#define X_PRIORITY_HIGH          3
#define X_PRIORITY_TIMECRITICAL  4

//--- Asynchronous Read Status
#define X_STATUS_PENDING         1
#define X_STATUS_INPROGRESS      2
#define X_STATUS_COMPLETE        3
#define X_STATUS_ERROR           4
#define X_STATUS_NOTFOUND        5


////////////////////////////////////////////////////////////////////////////
//  TYPES
////////////////////////////////////////////////////////////////////////////

typedef u32  X_FILE;

////////////////////////////////////////////////////////////////////////////
//  Variable argument support.
////////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER ) && defined( TARGET_PS2 )
    #define va_list         int
    #define va_start(a,b)   ((a)=0)
    #define va_arg(a,b)     (b)(a)
    void va_end  ( int );
#else
    #include <stdarg.h>
#endif

typedef va_list x_va_list;

#define x_va_start(ap,v)      va_start(ap,v)
#define x_va_end(list)        va_end(list)
#define x_va_arg(list,mode)   va_arg(list,mode)

////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
//  Init and Kill functions.  These functions will be called automatically 
//  by the "x_files management" (see x_files.hpp).  You should not need to 
//  invoke these functions directly.
//
////////////////////////////////////////////////////////////////////////////
//
//  x_InitSTDIO     - Init the stdio system in the current thread.
//  x_KillSTDIO     - Kill...
//
////////////////////////////////////////////////////////////////////////////

void    x_InitSTDIO     ( void );
void    x_KillSTDIO     ( void );

////////////////////////////////////////////////////////////////////////////
//
//  Basic file I/O functions.  
//
//  What could be easier?  These functions are specifically designed to 
//  mimic their standard C library counterparts.  For more information, look 
//  up these functions (without the "x_" prefix) in any C reference 
//  material.
//
////////////////////////////////////////////////////////////////////////////
//
//  x_fopen         - Open named file based on given Mode.  Return handle.
//  x_fclose        - Close given File.
//  x_fread         - Read from File into Buffer based on Size and Count.
//  x_fwrite        - Write from Buffer into File based on Size and Count.
//  x_fprintf       - Format print to File.
//  x_fflush        - Flush pending cached I/O for File.
//  x_fseek         - Reposition logical read/write cursor in File.
//  x_ftell         - Get position of logical read/write cursor in File.
//  x_feof          - Is logical read/write cursor at the end of the File?
//  x_fgetc         - Read one character from File.
//  x_fputc         - Write one character to File.
//
////////////////////////////////////////////////////////////////////////////

X_FILE* x_fopen        ( const char* pFilename, const char* pMode );
void    x_fclose       ( X_FILE* pFile );
s32     x_fread        ( void* pBuffer, s32 Size, s32 Count, X_FILE* pFile );
s32     x_fwrite       ( void* pBuffer, s32 Size, s32 Count, X_FILE* pFile );
s32     x_fprintf      ( X_FILE* pFile, const char* pFormatStr, ... );
s32     x_fflush       ( X_FILE* pFile );
s32     x_fseek        ( X_FILE* pFile, s32 Offset, s32 Origin );
s32     x_ftell        ( X_FILE* pFile );
s32     x_feof         ( X_FILE* pFile );
s32     x_fgetc        ( X_FILE* pFile );
s32     x_fputc        ( s32 C, X_FILE* pFile );

////////////////////////////////////////////////////////////////////////////
//
//  Advanced file I/O functions.
//
//  Functions to support advanced file I/O, not supported by the ANSI
//  standard. Even so, they are easy to use.
//
////////////////////////////////////////////////////////////////////////////
//
//  x_freada
//
//      Same parameters as x_fread with three added parameters. They are a
//      file offset to start the read, a priority, and storage for the task
//      ID. The Priority must be one of the X_PRIORITY_* defines.
//
//  x_freadastatus
//
//      It's very important to poll this function until the status is
//      X_STATUS_COMPLETE, or X_STATUS_ERROR. Don't just start an
//      asynchronous read, and forget about it. The BytesRead parameter
//      will be filled with the progress of the read. BytesRead is *not*
//      guaranteed to be correct until the return value is one of the
//      two above mentioned status's. The reason for this is that not
//      all hardware supportes this feature. After receiving one of the
//      above status's, the read task will be removed from the task list.
//      Meaning, that all subsequent functions calls referencing that
//      TaskID will return X_STATUS_NOTFOUND. BytesRead will be zero.
//
//      If you pass -1 as TaskID, it will return the BytesRead and status
//      of the current asynchronous read. When the status is COMPLETE or
//      ERROR; however, the task will not be removed from the task list.
//      It will only be removed from the task list, when it is polled
//      with it's actual Task Identifier.
//
//  x_freadacancel
//
//      Cancels an asynchronous read task.  Returns the status of the read
//      task at the time it was canceled.  If the task is currently reading in
//      progress, the read will complete but no data will be copied to the
//      user destination data buffer.
//      If -1 is passed as the TaskID, ALL asynchronous reads will be
//      canceled, returning X_STATUS_COMPLETE.
//
//  x_SetDVDDriveAccess - Disables or Re-Enables CD / DVD Drive functions.
//                        This was used to flush out areas of the game that
//                        are still trying to perform read/writes on the CD
//                        when we need them to shutdown for streaming, or 
//                        hardware specific functions which use the drive exclusively.
//                        The previous state drive access state is returned.
//
////////////////////////////////////////////////////////////////////////////

s32     x_freada( void*   pBuffer,
                  s32     Size,
                  s32     Count,
                  X_FILE* pFile,
                  s32     FileOffset,
                  s32     Priority,
                  s32&    TaskID );

s32     x_freadastatus ( s32 TaskID, s32& BytesRead );

s32     x_freadacancel ( s32 TaskID );

xbool   x_SetDVDDriveAccess( xbool bEnable );


////////////////////////////////////////////////////////////////////////////
//
//  Formatted string functions.
//
//  Again.  What could be easier.  These function mimic their standard
//  C library functions.  There is one new sheep in the flock though...
//
//  Each of these function returns the number of characters which were 
//  output (even if the characters were not visible).
//
////////////////////////////////////////////////////////////////////////////
//
//  x_printf
//
//      Formatted print to "standard text output".  This is straight forward
//      for text mode programs and is handled by the x_files.  (Graphical
//      programs must register a function to handle this operation.)
//
//  x_printfxy
//
//      Very much like x_printf except that the text will be printed at
//      the given character cell coordinates.  (0,0) represents the upper
//      left most character available.  The x_files will provide versions
//      of this function for each text mode target supported.  (Graphical
//      programs must register a function to handle this operation.)
//
//  x_sprintf   - Format print into given String.
//  x_vsprintf  - Format print into given String.  Use va_list for args.
//
////////////////////////////////////////////////////////////////////////////

s32     x_printf    (               const char* pFormatStr, ... );
s32     x_printfxy  ( s32 x, s32 y, const char* pFormatStr, ... );
s32     x_sprintf   ( char* pStr,   const char* pFormatStr, ... );
s32     x_vsprintf  ( char* pStr,   const char* pFormatStr, x_va_list Arg );

////////////////////////////////////////////////////////////////////////////
//
//  Temporary formatted string functions.
//
//  Within the x_stdio implementation, there is a global buffer which is
//  used as a scratch buffer for internal string manipulation.  And you can
//  use it too!  The following two functions provide an easy means for you
//  to format temporary strings without having to reserve your own buffer.
//  But beware!  Many functions use this secret buffer.  Strings you build
//  in this buffer with these function are not going to last long.
//
//  The fs function is real handy in cases like the following...
//      ASSERTS( x < MAX, fs( "Value x out of range: %d", x ) );
//      File = x_fopen( fs( "%s\\%s" ), Path, Filename, "r" );
//
//  Both functions return the address of the character buffer containing the
//  results.
//
////////////////////////////////////////////////////////////////////////////
//
//  fs   - "Format String" into internal scratch buffer.
//  vfs  - "VariableArgList Format String" version of fs().
//
////////////////////////////////////////////////////////////////////////////

char*   fs   ( const char* pFormatStr, ... );
char*   vfs  ( const char* pFormatStr, x_va_list Args );


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  EXPERTS ONLY FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  The x_files will personally implement all I/O stuff in "easy 
//  environments".  In "non-easy environments", like a game console, the
//  application (or some part of it such as an engine, like Quagmire) must
//  provide functions to carry out I/O operations.
//
//  Similarly, the x_files will take care of basic text output functions.
//  Graphics applications, such as a game, must provide functions to carry
//  out text output operations.
//
//  The function hooks for the I/O and text out operations use simplified
//  interfaces.
//
//  Function prototypes for "custom I/O":
//
//      X_FILE* Open        ( const char* pFilename, const char* pMode );
//      void    Close       ( X_FILE* pFile );
//      s32     Read        ( X_FILE* pFile, byte* pBuffer, s32 Bytes );
//      s32     ReadA       ( void*   pBuffer,
//                            s32     Bytes,
//                            X_FILE* pFile,
//                            s32     FileOffset,
//                            s32     Priority,
//                            s32&    TaskID );
//      s32     ReadAStatus ( s32 TaskID, s32& BytesRead );
//      s32     ReadACancel ( s32 TaskID );
//      s32     Write       ( X_FILE* pFile, byte* pBuffer, s32 Bytes );
//      s32     Seek        ( X_FILE* pFile, s32 Offset, s32 Origin );
//      s32     Tell        ( X_FILE* pFile );
//      xbool   EOF         ( X_FILE* pFile );
//
//  For targets which require custom I/O, the x_files will map the standard
//  functions to these functions.  Function x_putc( C, pFile ), for example, 
//  results in a call to *Write( pFile, &C, 1 ).  The function x_fprintf
//  will print into a temporary memory buffer and then call *Write.
//
//  Functions Read and Write return the number of characters processed.
//
//  Function prototypes for "custom text output":
//
//      void    Print   ( char* pString );
//      void    PrintAt ( char* pString, s32 X, s32 Y );
//
//  For targets which require custom text output, the x_files will map all 
//  of the text outout functions to these two functions.  Function 
//  x_printfxy will print into a temporary memory buffer and then call
//  *PrintAt.
//
////////////////////////////////////////////////////////////////////////////
//
//  x_SetIOHandlers       - Register "custom I/O" functions.
//  x_SetTextOutHandlers  - Register "custom text out" functions.
//
////////////////////////////////////////////////////////////////////////////

// Type definitions for custom I/O function pointers.

typedef X_FILE* (*  open_fnptr) ( const char* pFilename, const char* pMode );
typedef void    (* close_fnptr) ( X_FILE* pFile );
typedef s32     (*  read_fnptr) ( X_FILE* pFile, byte* pBuffer, s32 Bytes );
typedef s32     (* write_fnptr) ( X_FILE* pFile, byte* pBuffer, s32 Bytes );
typedef s32     (*  seek_fnptr) ( X_FILE* pFile, s32 Offset, s32 Origin );
typedef s32     (*  tell_fnptr) ( X_FILE* pFile );
typedef xbool   (*   eof_fnptr) ( X_FILE* pFile );

typedef s32     (* reada_fnptr) ( void*   pBuffer,
                                  s32     Bytes,
                                  X_FILE* pFile,
                                  s32     FileOffset,
                                  s32     Priority,
                                  s32&    TaskID );

typedef s32     (*status_fnptr) ( s32 TaskID, s32& BytesRead );
typedef s32     (*cancel_fnptr) ( s32 TaskID );

// Type definitions for custom text output function pointers.

typedef void (*   print_fnptr) ( const char* pString );
typedef void (*print_at_fnptr) ( const char* pString, s32 X, s32 Y );

// Functions to register the custom handlers.

void x_SetIOHandlers(   open_fnptr      OpenFnPtr,
                        close_fnptr     CloseFnPtr,
                        read_fnptr      ReadFnPtr,
                        reada_fnptr     ReadAFnPtr,
                        status_fnptr    ReadAStatFnPtr,
                        cancel_fnptr    ReadACancelFnPtr,
                        write_fnptr     WriteFnPtr,
                        seek_fnptr      SeekFnPtr,
                        tell_fnptr      TellFnPtr,
                        eof_fnptr       EOFFnPtr );

void x_GetIOHandlers(   open_fnptr&     OpenFnPtr,
                        close_fnptr&    CloseFnPtr,
                        read_fnptr&     ReadFnPtr,
                        reada_fnptr&    ReadAFnPtr,
                        status_fnptr&   ReadAStatFnPtr,
                        cancel_fnptr&   ReadACancelFnPtr,
                        write_fnptr&    WriteFnPtr,
                        seek_fnptr&     SeekFnPtr,
                        tell_fnptr&     TellFnPtr,
                        eof_fnptr&      EOFFnPtr );

void x_SetPrintHandlers( print_fnptr     PrintFnPtr,
                         print_at_fnptr  PrintAtFnPtr );

////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_DOLPHIN )

//### TEMPORARY: using stdlb functions instead of x_files on dolphin target.
//Please remove when problem(s) resolved.
#define x_sprintf   sprintf
#define x_vsprintf  vsprintf

#endif

////////////////////////////////////////////////////////////////////////////

#endif
///////////////////////////////////////////////////////////////////////////
//
//  X_STDIO.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_STDIO_H
#define X_STDIO_H

///////////////////////////////////////////////////////////////////////////
// INCLUDE
///////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_H
#include "x_types.h"
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
// ( stdarg.h )
///////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
typedef va_list x_va_list;

#define x_va_start(ap,v)      va_start(ap,v)
#define x_va_end(list)        va_end(list)
#define x_va_arg(list,mode)   va_arg(list,mode)

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define X_SEEK_SET  0
#define X_SEEK_CUR  1
#define X_SEEK_END  2

#define X_EOF      -1

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

typedef u32  X_FILE;

///////////////////////////////////////////////////////////////////////////
// PROTOTYPES
//
// Where applicable, the following functions are designed to mimic the
// behavior of their standard ANSI counterparts.
///////////////////////////////////////////////////////////////////////////

void    x_InitSTDIO ( void );
void    x_KillSTDIO ( void );                                               

s32     x_printf    (               const char* FormatStr, ... );
s32     x_printfxy  ( s32 x, s32 y, const char* FormatStr, ... );
s32     x_sprintf   ( char* Str,    const char* FormatStr, ... );
s32     x_vsprintf  ( char* Str,    const char* FormatStr, x_va_list Arg );

X_FILE* x_fopen     ( const char* Filename, const char* Mode );             
void    x_fclose    ( X_FILE* Stream );                                     
s32     x_fread     ( void* Buffer, s32 Size, s32 Count, X_FILE* Stream );  
s32     x_fwrite    ( void* Buffer, s32 Size, s32 Count, X_FILE* Stream );  
s32     x_fprintf   ( X_FILE* Stream, const char* FormatStr, ... );
s32     x_fflush    ( X_FILE* Stream );                                     
s32     x_fseek     ( X_FILE* Stream, s32 Offset, s32 Origin );             
s32     x_ftell     ( X_FILE* Stream );                                     
s32     x_feof      ( X_FILE* Stream );                                     
s32     x_fgetc     ( X_FILE* Stream );                                     
s32     x_fputc     ( s32 C, X_FILE* Stream );                              

///////////////////////////////////////////////////////////////////////////
// TEMPORARY FORMATTED STRING FUNCTIONS
///////////////////////////////////////////////////////////////////////////

char* fs ( const char* FormatStr, ... );
char* vfs( const char* FormatStr, x_va_list Args );

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/////////////////////// EXPERTS ONLY FROM HERE DOWN ///////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// CUSTOM HANDLERS - EXPERTS ONLY
// 
// As much functionality as possible has been implemented in a general
// form.  On some platforms, however, the implementation of the standard
// I/O functions requires a more intimate interaction than is typical for
// the x_files.  In these cases, the x_files client system must provide 
// simplified functions to take care of the platform specific details.  
// These custom hander functions are simplified since the x_files are able
// to take care of at least some of the functionality.  The custom handler
// functions come in two sets: text printing and file I/O.
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS REQUIRED FOR CUSTOM TEXT HANDLING
//
// s32  Print   ( char* String )
// s32  PrintAt ( char* String, s32 X, s32 Y )
//
// Both functions return the number of characters printed.
///////////////////////////////////////////////////////////////////////////

typedef void (*print_fnptr)   ( char* String );
typedef void (*print_at_fnptr)( char* String, s32 X, s32 Y );

#ifdef CUSTOM_TEXT_HANDLER

void    x_SetPrintHandlers   ( print_fnptr    PrintFnPtr,
                               print_at_fnptr PrintAtFnPtr );
#endif

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS REQUIRED FOR CUSTOM FILE I/O
// 
// These functions are similar to, but not exactly the same as, their ANSI
// counterparts.
// 
// X_FILE*  Open    ( const char* Filename, const char* Mode )
// void     Close   ( X_FILE* Stream )
// s32      Read    ( X_FILE* Stream, byte* Buffer, s32 Bytes )
// s32      Write   ( X_FILE* Stream, byte* Buffer, s32 Bytes )
// s32      Seek    ( X_FILE* Stream, s32 Offset, s32 Origin )
// s32      Tell    ( X_FILE* Stream )
// xbool    EOF     ( X_FILE* Stream )
//
// Both Read and Write return the number of bytes processed.
///////////////////////////////////////////////////////////////////////////

typedef X_FILE* (*open_fnptr)  ( const char* Filename, const char* Mode );
typedef void    (*close_fnptr) ( X_FILE* Stream );
typedef s32     (*read_fnptr)  ( X_FILE* Stream, byte* Buffer, s32 Bytes );
typedef s32     (*write_fnptr) ( X_FILE* Stream, byte* Buffer, s32 Bytes );
typedef s32     (*seek_fnptr)  ( X_FILE* Stream, s32 Offset, s32 Origin );
typedef s32     (*tell_fnptr)  ( X_FILE* Stream );
typedef xbool   (*eof_fnptr)   ( X_FILE* Stream );

#ifdef CUSTOM_FILE_IO_HANDLER

void    x_SetIOHandlers      ( open_fnptr    OpenFnPtr,
                               close_fnptr   CloseFnPtr,
                               read_fnptr    ReadFnPtr,
                               write_fnptr   WriteFnPtr,
                               seek_fnptr    SeekFnPtr,
                               tell_fnptr    TellFnPtr,
                               eof_fnptr     EOFFnPtr );
                               
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////

#endif

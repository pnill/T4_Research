///////////////////////////////////////////////////////////////////////////
//
//  E_TEXT.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef E_TEXT_HPP
#define E_TEXT_HPP

///////////////////////////////////////////////////////////////////////////
// 
// NOTE:  The text module is for internal Quagmire use only.
//        Public use is not allowed.  So stop reading this file!
//                                  
///////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"

///////////////////////////////////////////////////////////////////////////
// Type for "pointer to function to render a string".  Coordinates are in
// screen pixels.  The string is null terminated and contains no "cursor 
// control" character (tab, newline, etc) in the string.  The string is
// guaranteed to fit on the screen.  The large flag indicates if the 
// string should be rendered with "double size" characters.

typedef void (*render_string_fnptr)( s32 x, s32 y, char* s, xbool Large );

///////////////////////////////////////////////////////////////////////////

void TEXT_InitModule        ( s32 XRes,            s32 YRes, 
                              s32 FontSizeX,       s32 FontSizeY, 
                              s32 XPixelsIndented, s32 YPixelsIndented,
                              render_string_fnptr RenderStringFunction );

void TEXT_KillModule        ( void );
void TEXT_SwitchBuffers     ( void );
void TEXT_RenderAllText     ( void );
void TEXT_LargeMode         ( void );
void TEXT_SmallMode         ( void );

// Functions to be registered with x_stdio for x_printf and x_printfxy.
void TEXT_PrintString       ( const char* String );
void TEXT_PrintStringAt     ( const char* String, s32 X, s32 Y );

///////////////////////////////////////////////////////////////////////////

#endif
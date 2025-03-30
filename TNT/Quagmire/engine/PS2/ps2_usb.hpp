///////////////////////////////////////////////////////////////////////////
//
//  PS2 CD
//
//  NOTES:
//
//  When reading data, try to make sure that your buffer is 64-byte 
//  aligned.  This will get you the maximum speed from the hardware.
//  To do this, use the alignment token, like this:
//
//  byte    BlockReadBuffer[BLOCK_READ_BUFSIZE] __attribute__ ((aligned(64)));
//
//  
//
///////////////////////////////////////////////////////////////////////////

#ifndef __PS2_USB_HPP__
#define __PS2_USB_HPP__

///////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"


///////////////////////////////////////////////////////////////////////////
//
//  DEFINED SYSTEM CONSTANTS
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//      CALLS MADE INTERNALLY BY QUAGMIRE
//
void    PS2USB_Init          ( s32 argc,char *argv[]);
void    PS2USB_Kill          ( void );
void    PS2USB_Interface     ( void );
//
///////////////////////////////////////////////////////////////////////////

X_FILE* PS2USB_Open  ( const char* Filename, const char* Mode );
void    PS2USB_Close ( X_FILE* Stream                         );

s32     PS2USB_Read  ( X_FILE* Stream, const byte* Base, s32 Amount );
s32     PS2USB_Write ( X_FILE* Stream, byte* Base, s32 Amount );

s32     PS2USB_Seek  ( X_FILE* Stream, const s32 Offset, s32 Origin );
s32     PS2USB_Tell  ( X_FILE* Stream                         );
xbool   PS2USB_EOF   ( X_FILE* Stream                         );

s32     PS2USB_GetFileSize( X_FILE* Stream );

void	PS2USB_Printf ( const char* Text );

#endif // __PS2_INET_HPP__
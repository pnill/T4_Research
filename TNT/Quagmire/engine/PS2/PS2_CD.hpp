////////////////////////////////////////////////////////////////////////////
//
// PS2_CD.hpp
//
////////////////////////////////////////////////////////////////////////////

#ifndef PS2_CD_HPP_INCLUDED
#define PS2_CD_HPP_INCLUDED

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_stdio.hpp"


////////////////////////////////////////////////////////////////////////////
//
//  PS2CD_Boot :        Loads the system image from the CD system folder
//                      The image is defined at the top of PS2_CD.cpp 
//                      to minimize header modification.
//
//  PS2CD_Init :        Initializes CD system.  Called by engine
//  PS2CD_Kill :        Deactivates CD system.  Called by engine
//
////////////////////////////////////////////////////////////////////////////
//
// CALLS MADE INTERNALLY BY QUAGMIRE
//
void PS2CD_PrepImageFile ( char* pSystemImageFile );
void PS2CD_Boot          ( void );
void PS2CD_Init          ( void );
void PS2CD_Kill          ( void );
void PS2CD_Interface     ( void );

void PS2CD_UpdateAsyncReads( xbool bContinueTasks = TRUE );


////////////////////////////////////////////////////////////////////////////
//
// PS2CD_SetReadCallback :  Sets the callback function that is executed during
//                          synchronous reads, allows for updating display
//                          to let user know the application is loading.
//
////////////////////////////////////////////////////////////////////////////
//
// CALLS AVAILABLE TO USER
//

typedef void (*QSyncReadCallback)( void );

void PS2CD_SetReadCallback( QSyncReadCallback CallbackFunc );


////////////////////////////////////////////////////////////////////////////

#endif // PS2_CD_HPP_INCLUDED

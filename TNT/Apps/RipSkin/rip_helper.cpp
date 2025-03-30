///////////////////////////////////////////////////////////////////////////////
//
//  Rip Helper Implementation
//
///////////////////////////////////////////////////////////////////////////////

#include "rip_helper.h"

///////////////////////////////////////////////////////////////////////////////
//  String Helpers
///////////////////////////////////////////////////////////////////////////////

//==============================================================================
//  FillName()
//      Clears out string.  Crops if string is too large and copys string 
//==============================================================================

void FillName( char* Name, char* src, s32 NChars )
{
    s32 length = x_strlen( src );
    s32 start;

    x_memset( Name, 0, NChars );

    if ( length < NChars )
        start = 0;
    else
        start = length - NChars + 1;

    x_strcpy( Name, &src[start] );
}


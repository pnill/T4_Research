
///////////////////////////////////////////////////////////////////////////////
//
// x_std coding conventions:
//
//  1. Don't put function definitions inside class or template class declarations.
//     They tend to clutter the interface with implementation details, and they
//     can make debugging difficult.
//     Exception: If the function is a single statement.
//  
///////////////////////////////////////////////////////////////////////////////

#include "x_std.h"

#if X_STD_USES_X_FILES

    // x-files specific definitions

#else

    // Global operator new with placement
    void * operator new( size_t size , x_PlacementNewDummy * address  )
    {
        return address ;
    }


#endif


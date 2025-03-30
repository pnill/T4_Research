#ifndef X_STD_LIST2_H
#define X_STD_LIST2_H

///////////////////////////////////////////////////////////////////////////////
//
// Non-standard extensions based on STL <list>
//
// This declares numerous templates for additional list 
// classes and functions beyond those provided in x_list.h.
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_list.h"

namespace x_std 
{

    ///////////////////////////////////////////////////////////////////////////
    //  
    // class pooled_list< T , Allocator >
    //
    // A doubly-linked list of elements of type T using Allocator.
    // This list is similar to list<T,Allocator> except that some
    // (or all) of the freed list elements are kept for fast reuse
    // (reducing the number of memory allocations and deallocations).
    // 
    //todo: We don't really need this - just write an allocator to 
    // provide the functionality.
    //
    ///////////////////////////////////////////////////////////////////////////

    template
    <
        class   T                           ,   // The type for elements in the list
        class   Allocator   = allocator<T>      // The allocator to use for allocating and deleting elements. 
    >
    class x_pooled_list : public list<T,Allocator>
    {
      public :

        void reserve( size_type n ); // Make sure there are at least n elements in the reserve pool.
    };
    

};

#endif
    

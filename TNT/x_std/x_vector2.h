#ifndef X_STD_VECTOR2_H
#define X_STD_VECTOR2_H

///////////////////////////////////////////////////////////////////////////////
//
// Non-standard extensions based on STL <vector>
//
// This declares numerous templates for additional vector (variable-length array)
// classes and functions beyond those provided in x_vector.h.
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_vector.h"

namespace x_std 
{

    //todo:Find a better name
    ///////////////////////////////////////////////////////////////////////////
    //  
    // class vector_a< T , Allocator , CapacityFunction > 
    //
    // Similar to vector<T,Allocator>, but provides more control over the new capacity
    // chosen whenever the vector must be reallocated.
    //
    ///////////////////////////////////////////////////////////////////////////
    template
    <
        class T                                 ,   // The element type for the vector (array)
        class Allocator = x_std::allocator<T>   ,   // The allocator to use for reallocating the array when needed.
        class CapacityFunction                      // A binary function which takes the old size and capacity and returns the new capacity.
    >

    class vector_a
    
        : public vector<T,Allocator>
    {
        
      public:


    };


    

};

#endif
    

#ifndef X_STD_H
#define X_STD_H

///////////////////////////////////////////////////////////////////////////////
//
// Standard header for all x_std components
//
// x_std is a set of types, classes, functions, template classes, and template
// functions based mostly on STL (Standard Template Library). For example, x_std
// includes implementations for containers such as sets (template set<>) and 
// lists (template list<>), as well as general algorithms such as sort<>.
//
// All macros that x_std defines start with X_STD_...
// Other than such macros, the only names introduced into the global namespace are:
//
//     namespace x_std              For the public interfaces made available by x_std
//     namespace x_std_private      For private implementation details used by x_std
//     namespace x_std_statistics   For public but unsupported statistics
//
// x_std is organized by headers into groups of related classes or functions:
// 
//    x_algorithm.h   Useful algorithms for sorting, searching, reversing, randomizing, and so on.
//    x_bitset.h      Sets of bits.
//    x_complex.h     Arithmetic for complex numbers.
//    x_deque.h       A deque container.
//    x_functional.h  Predicates for, among other things, the templates defined in x_algorithm.h and x_numeric.h.
//    x_iterator.h    Iterators.
//    x_list.h        A list container.
//    x_map.h         An associative container.
//    x_memory.h      Allocators and other related memory functions.
//    x_numeric.h     Useful numeric functions.
//    x_queue.h       A queue container.
//    x_set.h         An associative container with unique elements.
//    x_stack.h       A stack container.
//    x_string.h      A string container.
//    x_utility.h     Some relational operators, the template class pair<>, and related definitions.
//    x_valarray.h    A varying-length list of elements like vector<> but with arithmetic behaviour.
//    x_vector.h      A varying-length array container.
//
// Non-standard member functions inside standard templates will be prefixed 
// with "x_", such as stack<T>::x_clear(). Additional non-standard template 
// classes and template functions will be added in files with a name containing 
// "2". For example, x_algorithm2.h contains additional algorithms not found in 
// x_algorithm.h, such as bubble_sort().
//
// For string manipulation          See x_string.h.
//
// For container classes            See x_deque.h, x_list.h, x_map.h, 
//   (set, list, vector)            x_queue.h, x_set.h, x_stack.h, x_vector.h
//
// For min, max, swap               See x_algorithm.h
//
// For searching and sorting        See x_algorithm.h
//
///////////////////////////////////////////////////////////////////////////////
//
// Compilation requirements:
//
//     If X_STD_USES_X_FILES dictates that the x-files should be used, then the 
//     "x-files" directory must be on the include path so that the x_std files
//     can include the needed x-files headers.
//
// Macro X_STD_USES_X_FILES (0 or 1)
//      
//     Where x_std was first developed, some local projects use a common set of 
//     declarations defined in something called the "x-files". If this macro is 
//     defined to be 0, then x_std will use the standard C++ types and library 
//     functions. If this macro is undefined or defined as anything except 0, 
//     then x_std will immediately redefine it to be 1 and will use some 
//     x-files types and some x-files library calls.
//
// Macro X_STD_ASSERT (function-style macro with one xbool argument)
//      
//     This macro is used by std to handle the assertion of boolean conditions:
//
//          X_STD_ASSERT(condition)
//
//     If this macro is already defined by the project, then x_std leaves the 
//     definition alone and uses it for assertions. Otherwise, it uses the 
//     standard assertion assert() or, when x-files are being used as described 
//     under X_STD_USES_X_FILES, the x-files ASSERT() macro.
//
// Macro X_STD_DEBUG (0 or 1)
//      
//     If this macro is defined as 0, then all x_std debugging information
//     and checks are removed. If this macro is defined as anything else,
//     then x_std debugging information and checks will be included and
//     the macro is immediately redefined to 1.
//     
//     If this macro is not defined at all, then it will be immediately 
//     redefined to 0 except in the following case: if the x-files are being 
//     used as described under X_STD_USES_X_FILES, then the setting of 
//     X_STD_DEBUG will be based on X_DEBUG. 
//
// Macro X_STD_KEEP_STATISTICS (0 or 1)
//      
//     Note: The statistics kept by x_std include such things as counts for the 
//     number of memory allocations, list allocations, vector reallocations, 
//     and so on. This information can be used (and changed) by applications 
//     with the understanding that this is not a formally supported interface 
//     and might change at any time. See x_private_stats for details.
//
//     If this macro is defined as 0, then all x_std statistics are removed. If 
//     this macro is defined as anything else, then statistics will be kept and 
//     the macro is immediately redefined to 1.
//     
//     If this macro is not defined at all, then it will be immediately 
//     redefined to 0 or 1 with the same value as X_STD_DEBUG (after it has
//     been redefined to 0 or 1).
//
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// Compiler-specific directives
//
///////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
    // Suppress certain Microsoft Visual C++ warnings
    #pragma warning( disable: 4514 ) // Warning:  unreferenced inline function removed
    #pragma warning( disable: 4710 ) // Warning: function 'x' not inlined
    #pragma warning( disable: 4097 ) // Warning: typedef-name 'x' used as synonym for class-name 'y'
    #pragma warning( disable: 4291 ) // 'function' : no matching operator delete found; memory will not be freed if initialization throws an exception
    #pragma warning( disable: 4786 ) // Warning: identifier truncated to 255 characters in the debug information
    #pragma warning( disable: 4284 ) // Return type for 'x' is 'y' (ie; not a UDT or reference to a UDT) [Note: generated for iterator operator ->]
	#pragma warning( disable: 4127 ) // Warning:  conditional expression is constant
#endif


// Macro X_STD_USES_MEMBER_TEMPLATES is used internally to define if
// the compiler supports template members.

#define X_STD_USES_MEMBER_TEMPLATES 0
#ifdef __GNUC__
    #if (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) || __GNUC__ > 2
        #undef  X_STD_USES_MEMBER_TEMPLATES 
        #define X_STD_USES_MEMBER_TEMPLATES 1
    #endif
#endif

// Macro X_STD_USES_TEMPLATE_REBIND is used internally to define if
// the compiler supports templates well enough that the allocator "rebind 
// mechanism" can be used for allocators.

#if X_STD_USES_MEMBER_TEMPLATES
    #define X_STD_USES_TEMPLATE_REBIND 1
#else
    #define X_STD_USES_TEMPLATE_REBIND 0
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Set up X_STD_... conditional macros and make sure each is either 0 or 1
//
///////////////////////////////////////////////////////////////////////////////
//#define X_STD_USES_X_FILES = 0
/*
#ifndef X_STD_USES_X_FILES
    #define X_STD_USES_X_FILES 1
#elif X_STD_USES_X_FILES == 0
    #undef  X_STD_USES_X_FILES
    #define X_STD_USES_X_FILES 0
#else
    #undef  X_STD_USES_X_FILES
    #define X_STD_USES_X_FILES 1
#endif    
*/
#ifndef X_STD_DEBUG
    #ifdef X_STD_USES_X_FILES
        #ifdef X_DEBUG
            #define X_STD_DEBUG 1
        #else
            #define X_STD_DEBUG 0
        #endif
    #else
        #define X_STD_DEBUG 0
    #endif
#elif X_STD_DEBUG == 0
    #undef  X_STD_DEBUG
    #define X_STD_DEBUG 0
#else
    #undef  X_STD_DEBUG
    #define X_STD_DEBUG 1
#endif    

#ifndef X_STD_KEEP_STATISTICS
    #if 0 //X_STD_DEBUG  ####Until I have time to figure out why I'm getting linker problems. RK
        #define X_STD_KEEP_STATISTICS 1
    #else
        #define X_STD_KEEP_STATISTICS 0
    #endif
#elif X_STD_KEEP_STATISTICS == 0
    #undef  X_STD_KEEP_STATISTICS
    #define X_STD_KEEP_STATISTICS 0
#else
    #undef  X_STD_KEEP_STATISTICS
    #define X_STD_KEEP_STATISTICS 1
#endif    

///////////////////////////////////////////////////////////////////////////////
//
// Include files and other dependencies
//
///////////////////////////////////////////////////////////////////////////////

#ifdef X_STD_USES_X_FILES

    // (Because the x-files uses "#define new ...", this creates some nasty syntax
    // errors if we define our own "new with placement" operator. Thus, we rely
    // on the definition of that operator being part of x-files.)

    #include    "x_types.hpp"
    #include    "x_debug.hpp"
    #include    "x_plus.hpp"

#else

    #include    <stddef.h>      // This gives us size_t
    #include    <string.h>      // This gives us memcpy() and related functions.
    #include    <assert.h>     

    class x_PlacementNewDummy ; // A dummy class to uniquely identify our "new with placement"

    // Global operator new with placement
    void * operator new( size_t size , x_PlacementNewDummy * address  );

#endif


///////////////////////////////////////////////////////////////////////////////
//
// namespace x_std
//
///////////////////////////////////////////////////////////////////////////////

namespace x_std 
{
    //-------------------------------------------------------------------------
    // The basic types used by x_std
    //-------------------------------------------------------------------------

    #if X_STD_USES_X_FILES

        typedef xbool       x_bool   ;
        typedef s32         x_int    ;
        typedef x_size_t    x_size_t ;

    #else

        typedef bool        x_bool   ;
        typedef int         x_int    ;
        typedef size_t      x_size_t ;

    #endif

    //-------------------------------------------------------------------------
    // The support functions used by x_std
    //-------------------------------------------------------------------------

    #if X_STD_USES_X_FILES

        // Because the x-files uses "#define new ..." we rely on it to provide the
        // template functions placement_new<T>(address) and placement_new<T>(address,value).

        inline void *  x_memcpy  ( void * target , const void * source , x_size_t size ) { return ::x_memcpy  (target,source,size); }
        inline void *  x_memmove ( void * target , const void * source , x_size_t size ) { return ::x_memmove (target,source,size); }
        inline void *  x_memset  ( void * target , x_int         value , x_size_t size ) { return ::x_memset  (target,value ,size); }
        inline x_int   x_memcmp  ( const void * lhs , const void * rhs , x_size_t size ) { return ::x_memcmp  (lhs   ,rhs   ,size); }
        inline x_int   x_strlen  ( const char * x )                                      { return ::x_strlen  ( x );                }
        inline x_int   x_strcmp  ( const char * lhs , const char * rhs                 ) { return ::x_strcmp  ( lhs , rhs );        }

        #if !defined(X_STD_ASSERT)
            // Sorry, this is an abuse of macros but it seems to be the only way.
            #define X_STD_ASSERT(condition) ASSERT(condition)
        #endif

    #else

        template <class T> inline void* placement_new( T * address ) 
            { return new (reinterpret_cast<x_PlacementNewDummy*>(address)) T; }
        template <class T> inline void* placement_new( void * address , const T & value ) 
            { return new (reinterpret_cast<x_PlacementNewDummy*>(address)) T(value); }

        inline void *   x_memcpy  ( void * target , const void * source , x_size_t size ) { return memcpy  (target,source,size); }
        inline void *   x_memmove ( void * target , const void * source , x_size_t size ) { return memmove (target,source,size); }
        inline void *   x_memset  ( void * target , x_int value         , x_size_t size ) { return memset  (target,value ,size); }
        inline x_int    x_memcmp  ( const void * lhs , const void * rhs , x_size_t size ) { return memcmp  (lhs   ,rhs   ,size); }
        inline x_size_t x_strlen  ( const char * x )                                      { return strlen  ( x );                }
        inline x_int    x_strcmp  ( const char * lhs , const char * rhs                 ) { return strcmp  ( lhs , rhs );        }

        #if !defined(X_STD_ASSERT)
            // Sorry, this is an abuse of macros but it seems to be the only way.
            #define X_STD_ASSERT(condition) assert(condition)
        #endif

    #endif

}


namespace x_std_private
{
    //-------------------------------------------------------------------------
    // Users should not rely on declarations inside of x_std_private
    //-------------------------------------------------------------------------
    typedef x_std::x_bool        x_bool   ;
    typedef x_std::x_int         x_int    ;
    typedef x_std::x_size_t      x_size_t ;

}

#include    "x_std_statistics.h"

#endif

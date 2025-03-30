#ifndef X_STD_UTILITY_H
#define X_STD_UTILITY_H

///////////////////////////////////////////////////////////////////////////////
//
// Templates based on STL <utility>
//
// This header defines several templates of general use throughout x_std.
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
// None
//
///////////////////////////////////////////////////////////////////////////////
//  
// Overview of basic functions
//
///////////////////////////////////////////////////////////////////////////////

#include "x_std.h"

namespace x_std 
{
    ///////////////////////////////////////////////////////////////////////////
    //
    // pair<T,U>
    //
    // This template class stores a pair of objects.
    // The first is of type T, the second is of type U.
    // The first (default) constructor initializes first to T() and second to U(). 
    // The second constructor initializes first to x and second to y. 
    // The third (template) constructor initializes first to pr.first and second to pr.second. 
    // T and U each need supply only a single-argument constructor and a destructor.
    //
    ///////////////////////////////////////////////////////////////////////////
    template < class T , class U >
    struct pair 
    {
        typedef T   first_type  ;
        typedef U   second_type ;

        T first     ;
        U second    ;

        pair() : first(T()) , second(U()) { }
        pair( const T & x , const U & y ) : first(x) , second(y) { }
        template< class V , class W > pair( const pair<V, W> & pr ) : first(pr.first) , second(pr.second) { }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // make_pair<T,U>(x,y)
    // 
    // This template function makes a pair out of x and y.
    //
    ///////////////////////////////////////////////////////////////////////////
    template < class T , class U >
    pair< T , U > make_pair( const T & x , const U & y )
    {
        return pair<T, U>(x, y);
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Relational template functions for pair<T,U>
    // 
    ///////////////////////////////////////////////////////////////////////////
    template<class T, class U> x_bool operator == (const pair<T,U> & x, const pair<T,U> & y) { return x.first == y.first && x.second == y.second ; }
    template<class T, class U> x_bool operator != (const pair<T,U> & x, const pair<T,U> & y) { return !(x == y); }
    template<class T, class U> x_bool operator <  (const pair<T,U> & x, const pair<T,U> & y) { return x.first < y.first || !(y.first < x.first && x.second < y.second); }
    template<class T, class U> x_bool operator >  (const pair<T,U> & x, const pair<T,U> & y) { return y < x ; }
    template<class T, class U> x_bool operator <= (const pair<T,U> & x, const pair<T,U> & y) { return !(y < x) ; }
    template<class T, class U> x_bool operator >= (const pair<T,U> & x, const pair<T,U> & y) { return !(x < y) ; }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Four template operators (!=, <=, >, >=) define a total ordering on pairs 
    // of operands of the same  type, given definitions of operator== and operator<.
    //
    ///////////////////////////////////////////////////////////////////////////
    namespace rel_ops 
    {
        template<class T> x_bool operator != (const T & x, const T & y) { return !(x == y); }
        template<class T> x_bool operator <= (const T & x, const T & y) { return !(y <  x); }
        template<class T> x_bool operator >  (const T & x, const T & y) { return  (y <  x); }
        template<class T> x_bool operator >= (const T & x, const T & y) { return !(x <  y); }
    }
}


#endif

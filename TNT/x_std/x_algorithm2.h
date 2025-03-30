#ifndef X_STD_ALGORITHM2_H
#define X_STD_ALGORITHM2_H

///////////////////////////////////////////////////////////////////////////////
//
// Useful (but non-STL) algorithms 
//
// This declares numerous template functions that perform useful algorithms. 
// These do not have corresponding declarations in the C++ STL 
// (Standard Template Library).
//
//
///////////////////////////////////////////////////////////////////////////////
#include "x_std.h"

namespace x_std 
{
    //todo: Delete this?
    // ///////////////////////////////////////////////////////////////////////////
    // //  
    // // find< Container , T >( x , value )
    // //
    // // Container must be a container class for T and have all the necessary
    // // container functions and types (such as const_iterator, begin() and end()).
    // //
    // // Return the first iterator i in the sequence [x.first(),x.last) for which
    // // the predicate *i == value is true. If no such iterator exists, return x.end().
    // //
    // ///////////////////////////////////////////////////////////////////////////
    // template< class Container , class T >
    // typename Container::const_iterator find( const Container & x , const T & value )
    // {
    //     typename Container::const_iterator i    = x.begin() ;
    //     typename Container::const_iterator end  = x.end()   ;
    //     for( ; i != end && !(*i==value) ; ++ i )
    //     {
    //     }
    //     return i ;
    // }

    // template< class Container , class T >
    // typename Container::iterator find( Container & x , const T & value )
    // {
    //     typename Container::iterator i    = x.begin() ;
    //     typename Container::iterator end  = x.end()   ;
    //     for( ; i != end && !(*i==value) ; ++ i )
    //     {
    //     }
    //     return i ;
    // }

    //todo: Delete this?
    // ///////////////////////////////////////////////////////////////////////////
    // //  
    // // find_if< Container , T , Predicate >( x , predicate )
    // //
    // // Container must be a container class for T and have all the necessary
    // // container functions and types (such as const_iterator, begin() and end()).
    // //
    // // Return the first iterator i in the sequence [x.first(),x.last) for which
    // // predicate(*i) is true. If no such iterator exists, return x.end().
    // //
    // ///////////////////////////////////////////////////////////////////////////
    // template< class Container , class T , class Predicate >
    // typename Container::const_iterator find_if( const Container & x , Predicate predicate )
    // {
    //     typename Container::const_iterator i    = x.begin() ;
    //     typename Container::const_iterator end  = x.end()   ;
    //     for( ; i != end && !predicate(*i) ; ++ i )
    //     {
    //     }
    //     return i ;
    // }

    // template< class Container , class T , class Predicate >
    // typename Container::iterator find_if( Container & x , Predicate predicate )
    // {
    //     typename Container::iterator i    = x.begin() ;
    //     typename Container::iterator end  = x.end()   ;
    //     for( ; i != end && !predicate(*i) ; ++ i )
    //     {
    //     }
    //     return i ;
    // }

    ///////////////////////////////////////////////////////////////////////////
    //  
    // bubble_sort<RandomAccessIterator, Compare> 
    //
    // Time Complexity: O(n*n)
    ///////////////////////////////////////////////////////////////////////////
	template<class RandomAccessIterator, class Compare>
	void bubble_sort(const RandomAccessIterator &first, 
	                 const RandomAccessIterator &last, 
					 Compare comp)
	{
		if( first != last )
		{
		    RandomAccessIterator curr, currPlusOne;
            RandomAccessIterator nextToLast = last;
		    x_bool Sorted;

            --nextToLast;

            do
			{
				Sorted = true ;
                currPlusOne = first;
                ++currPlusOne;

				for( curr = first; curr != nextToLast; ++curr, ++currPlusOne )
				{
					if( comp(*(currPlusOne), *curr) )
					{
						swap(*(currPlusOne), *curr);
						Sorted = false;
					}
				}
			}
			while( !Sorted );
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    //  
    // selection_sort<RandomAccessIterator> 
    //
    // Time Complexity: O(n*n)
    ///////////////////////////////////////////////////////////////////////////
	template<class RandomAccessIterator>
	void selection_sort(RandomAccessIterator first, 
	                    RandomAccessIterator last)
	{
        RandomAccessIterator least;
		while( first != last ) {
			RandomAccessIterator curr = least = first;
			while( (++curr) != last ) {
				if( *curr < *least ) { least = curr; }
			}
            swap(*first, *least);
			++first;
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    //  
    // selection_sort<RandomAccessIterator> 
    //
    // Time Complexity: O(n*n)
    ///////////////////////////////////////////////////////////////////////////
	template<class RandomAccessIterator, class Compare>
	void selection_sort(RandomAccessIterator first, 
	                    RandomAccessIterator last, 
					    Compare comp)
	{
        RandomAccessIterator least;
		while( first != last ) {
			RandomAccessIterator curr = least = first;
			while( (++curr) != last ) {
				if( comp(*curr, *least) ) { least = curr; }
			}
            swap(*first, *least);
			++first;
		}	
	}
}


#endif
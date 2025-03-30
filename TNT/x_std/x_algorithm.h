#ifndef X_STD_ALGORITHM_H
#define X_STD_ALGORITHM_H

///////////////////////////////////////////////////////////////////////////////
//
// Useful algorithms base on STL <algorithm>
//
// This declares numerous template functions that perform useful algorithms. 
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
// Under construction
//
///////////////////////////////////////////////////////////////////////////////
#include "x_std.h"

namespace x_std 
{

    #if defined(min)
        // Someone thought that #define min ... was a good idea. (Hard to believe, isn't it?)
        #undef min
    #endif

    #if defined(max)
        // Someone thought that #define max ... was a good idea. (Hard to believe, isn't it?)
        #undef max
    #endif

    ///////////////////////////////////////////////////////////////////////////
    //  
    // min<T> 
    //
    // Time Complexity: O(1)
    ///////////////////////////////////////////////////////////////////////////
    template<class T> 
    const T & min( const T & x , const T & y )
    {
        return y < x ? y : x ;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  
    // min<T> 
    //
    // Time Complexity: O(1)
    ///////////////////////////////////////////////////////////////////////////
    template<class T> 
    const T & max( const T & x , const T & y )
    {
        return x < y ? y : x ;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //  
    // swap<T> 
    //
    // Time Complexity: O(1)
    ///////////////////////////////////////////////////////////////////////////
    template<class T> 
    inline void swap( T & x , T & y )
    {
        T temp = x;
        x = y;
        y = temp;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  
    // equal< InputIterator1 , InputIterator2 >( first1 , last1 , first2 )
    //
    // Return true if and only if for each N in the range [0, last1 - first1), 
    // the predicate *(first1 + N) == *(first2 + N) is true. Return false otherwise.
    // Evaluate the predicate at most once for each N.
    //
    ///////////////////////////////////////////////////////////////////////////
    template< class InputIterator1 , class InputIterator2 >
    x_bool equal( InputIterator1 first1 , InputIterator1 last1 , InputIterator2 first2 )
    {
        // Reuse first1 and first2 to be the first in the range left to check.
        x_bool equal = true ;
        for( ; first1 != last1 && equal ; ++ first1 , ++ first2 )
        {
            equal =  *first1 == *first2  ;
        }
        return equal ;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  
    // equal< InputIterator1 ,InputIterator2 , Predicate >( first1 , last1 , first2 , predicate )
    //
    // Return true if and only if for each N in the range [0, last1 - first1), 
    // predicate( *(first1 + N) , *(first2 + N) ) is true. Return false otherwise.
    // Evaluate the predicate at most once for each N.
    //
    ///////////////////////////////////////////////////////////////////////////
    template< class InputIterator1 , class InputIterator2 , class Predicate >
    x_bool equal( InputIterator1 first1 , InputIterator1 last1 , InputIterator2 first2 , Predicate predicate )
    {
        // Reuse first1 and first2 to be the first in the range left to check.
        x_bool equal = true ;
        for( ; first1 != last1 && equal ; ++ first1 , ++ first2 )
        {
            equal = predicate( *first1 , *first2 );
        }
        return equal ;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  
    // find<iterator, T>( first , last , value )
    //
    // Return first+N, where N is the lowest value in the range [0, last - first) 
    // for which the predicate *(first + N) == value is true. If no such N exists,
    // return last. Evaluate the predicate once, at most, for each N.
    //
    // Time Complexity: O(last-first)
    ///////////////////////////////////////////////////////////////////////////
    template<class InputIterator, class T> 
    inline InputIterator find(const InputIterator &first , const InputIterator &last, const T &value)
    {
        InputIterator it;
        for (it=first; it!=last; ++it)
        {
            if ((*it) == value) break;
        }
        return it;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  
    // find_if<iterator, T>( first , last , predicate )
    //
    // Return first+N, where N is the lowest value in the range [0, last - first)
    // for which the predicate(*(first + N)) is true. If no such value exists, 
    // return last. Evaluate the predicate once, at most, for each N.
    //
    ///////////////////////////////////////////////////////////////////////////
    template < class InputIterator , class Predicate >
    InputIterator find_if( InputIterator first , InputIterator last , Predicate predicate )
    {
        // Reuse first each iteration as the first item in the remaining range.
        for( ; first != last && !predicate(*first) ; ++ first ) { }
        return first ;
    }


    ///////////////////////////////////////////////////////////////////////////
    //  
    // fill<ForwardIterator, T> 
    //
    // Time Complexity: O(n)
    ///////////////////////////////////////////////////////////////////////////
	template<class ForwardIterator, class T> 
	void fill(ForwardIterator first, ForwardIterator last, const T& value)
	{
		while( first != last )
		{
			*first = value;
			++first;
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    //  
    // copy<InputIterator, OutputIterator> 
    //
    // Time Complexity: O(n)
    ///////////////////////////////////////////////////////////////////////////
	template<class InputIterator, class OutputIterator>
	OutputIterator copy(InputIterator first, InputIterator last, OutputIterator out)
	{
		while( first != last )
		{
			*out = *first;
			++out; ++first;
		}
		return out;
	}

    ///////////////////////////////////////////////////////////////////////////
    //
    // lexicographical_compare< InputIterator1 , InputIterator2 )
    //      ( first1 , last1 , first2 , last2 )
    //
    // Returns true if the sequence [first1,last1) is less than the sequence [first2,last2).
    // 
    // Detailed explanation:
    // Compare K elements, where K is the smaller of last1 - first1 and last2 - first2. 
    // Determine the lowest value of N in the range [0, K) for which *(first1 + N) and *(first2 + N) 
    // do not have equivalent ordering. 
    // Return true if no such value exists and K < (last2-first2).
    // Otherwise, return true if *(first1 + N) < *(first2 + N). 
    // In all other cases, return false.
    // Evaluate the ordering predicate "<" at most 2 * K times.
    ///////////////////////////////////////////////////////////////////////////
    template< class InputIterator1 , class InputIterator2 >
    x_bool lexicographical_compare
    ( 
        InputIterator1 first1 , InputIterator1 last1 ,
        InputIterator2 first2 , InputIterator2 last2
    )
    {
        // Reuse first1 and first2 to be the first in the range left to check.
        for(  ; first1 != last1 && first2 != last2 ; ++ first1 , ++ first2 )
        {
            if( *first1 < *first2 )
            {
                return true ; // Unstructured return.
            }
            else if( *first2 < *first1 )
            {
                return false ; // Unstructured return.
            }
        }
        return first1 == last1 && first2 != last2 ;
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // lexicographical_compare< InputIterator1 , InputIterator2 , Predicate )
    //      ( first1 , last1 , first2 , last2 , predicate )
    //
    // Returns true if the sequence [first1,last1) is less than the sequence [first2,last2)
    // using the predicate instead of the usual "<" operator.
    //
    // Compare up to K elements, where K is the smaller of last1 - first1 and last2 - first2. 
    // Evaluate the ordering predicate at most 2 * K times.
    //
    ///////////////////////////////////////////////////////////////////////////
    template< class InputIterator1 , class InputIterator2 , class Predicate >
    x_bool lexicographical_compare
    ( 
        InputIterator1 first1    , InputIterator1 last1 ,
        InputIterator2 first2    , InputIterator2 last2 ,
        Predicate      predicate
    )
    {
        // Reuse first1 and first2 to be the first in the range left to check.
        for(  ; first1 != last1 && first2 != last2 ; ++ first1 , ++ first2 )
        {
            if( predicate( *first1 , *first2 ) )
            {
                return true ; // Unstructured return.
            }
            else if( predicate( *first2 , *first1 ) )
            {
                return false ; // Unstructured return.
            }
        }
        return first1 == last1 && first2 != last2 ;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  
    // sort<RandomAccessIterator> (Insertion Sort)
    //
    // Time Complexity: O(n*n)
    ///////////////////////////////////////////////////////////////////////////
	template<class RandomAccessIterator>
	void sort(RandomAccessIterator first, 
	                    RandomAccessIterator last)
	{
        if (first != last)
        {
            RandomAccessIterator ins;
            RandomAccessIterator key = first;
            ++key;
            while (key != last)
            {
                ins = first;
                while (ins != key)
                {
                    if (*key < *ins)
                    {
                        while (ins != key)
                        {
                            swap(*ins, *key);
                            ++ins;
                        }
                        break;
                    }
                    ++ins;
                }
                ++key;
            }
        }
	}

    ///////////////////////////////////////////////////////////////////////////
    //  
    // sort<RandomAccessIterator>  (Insertion Sort)
    //
    // Time Complexity: O(n*n)
    ///////////////////////////////////////////////////////////////////////////
	template<class RandomAccessIterator, class Compare>
	void sort(RandomAccessIterator first, 
	                    RandomAccessIterator last, 
					    Compare comp)
	{
        if (first != last)
        {
            RandomAccessIterator ins;
            RandomAccessIterator key = first;
            ++key;
            while (key != last)
            {
                ins = first;
                while (ins != key)
                {
                    if (comp(*key, *ins))
                    {
                        while (ins != key)
                        {
                            swap(*ins, *key);
                            ++ins;
                        }
                        break;
                    }
                    ++ins;
                }
                ++key;
            }
        }
	}
}


#endif
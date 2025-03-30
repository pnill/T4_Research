#if !defined( X_STD_VECTOR_H )
#define X_STD_VECTOR_H

///////////////////////////////////////////////////////////////////////////////
//
// Variable-length arrays - declarations based on STL <vector>
//
// Defines the template class vector<> and various supporting templates.
//
///////////////////////////////////////////////////////////////////////////////
//  
// Overview of basic functions for vector<T,Allocator>:
//
//   vector<T> X ;      // Declare X as an array of elements of type T.
//   X.push_back(E);    // Add element E to the end of the array
//   X.pop_back();      // Remove the last element from the array.
//   X[n]               // The nth element in the array, starting at 0.
//   X.front()          // The first element in the array. Same as X[0].
//   X.back()           // The last element in the array. Same as X[size()-1].
//   X.size()           // How many elements are in X?
//   X.empty()          // Is X empty? (Is its size 0?)
//   X.resize(n);       // Change the size of the array to n, adding or removing elements.
//   X.reserve(n);      // Make sure there is room for n elements without reallocating.
//                      // DOES NOT CHANGE the size of the array.
//   X.clear();         // Empty the array. If X is an array of pointers, the pointers are NOT deallocated.
//   X.insert( X.begin()+2 , E ); // Insert E before X[2].
//   X.erase( X.begin()+10 );       // Erase X[10] from the array. Does not delete X[10] if it is a pointer.
//
// Forward iteration:
//
//     x_std::vector<T>::const_iterator i ;
//     for( i = X.begin() ; i != X.end() ; ++i )
//     {
//         const T & Element = *i ; 
//     }
//
// Reverse iteration:
//
//     x_std::vector<T>::const_reverse_iterator i ;
//     for( i = X.rbegin() ; i != X.rend() ; ++i )
//     {
//         const T & Element = *i ; 
//     }
//
// To free an array X of pointers (where T is a pointer type):
//    vector<T> X ;
//        ...
//    while( !X.empty() )
//    {
//        delete X.back();
//        X.pop_back();
//    }
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
//   1. The following specialization is not provided: 
//      template<class Allocator> class vector<bool, Allocator>;
//   2. The following list<T,Allocator> members are not provided:
//         template<class Predicate> void sort
//   3. Assertion failures are used instead of throwing out_of_range.
//
///////////////////////////////////////////////////////////////////////////////
//
// Performance
//
// The time-complexity of each function is described near its declaration as 
// [O(f)], or [O(f1),O(f2)] where f, f1, and f2 are functions of the arguments 
// and *this. If two functions are given, f1 describes the case where vector 
// reallocation is not needed and f2 describes the case where reallocation is needed.
//
//    O(size())  The time depends on the size of the array.
//    O(n)       The time depends on the parameter n.
//    O(it2-it1) The time depends on the number of elements in the iterator range [it1,it2).
//
// If the complexity is omitted, it can be assumed to be O(1).
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_memory.h"
#include    "x_algorithm.h"

namespace x_std
{
    ///////////////////////////////////////////////////////////////////////////
    //  
    // class vector< T , Allocator >
    //
    // A variable-length array of elements of type T using Allocator.
    // When reallocation is required because there isn't enough room in the
    // vector, it is reallocated to have at least twice the old capacity.
    //
    ///////////////////////////////////////////////////////////////////////////
    template
    <
        class T                                 ,   // The element type for the vector (array)
        class Allocator = x_std::allocator<T>       // The allocator to use for reallocating the array when needed.
    >
    class vector 
    {
      public:

        //---------------------------------------------------------------------
        // Administrative types
        //---------------------------------------------------------------------

        typedef Allocator                            allocator_type     ;
        typedef typename Allocator::size_type        size_type          ;
        typedef typename Allocator::difference_type  difference_type    ;
        typedef T &                                  reference          ;
        typedef const T &                            const_reference    ;
        typedef T                                    value_type         ;

        class iterator               ;
        class const_iterator         ;
        class reverse_iterator       ;
        class const_reverse_iterator ;

        //---------------------------------------------------------------------
        // Construction and assignment
        //---------------------------------------------------------------------
        explicit vector // Construct an empty sequence. 
        ( 
            const Allocator & allocator = Allocator() 
        ); 
        explicit vector // Construct a sequence with n copies of value. [O(n)]
        ( 
            size_type           n                           ,    
            const T &           value       = T()           , 
            const Allocator &   a           = Allocator() 
        ); 
        vector  // Construct a sequence with a copy of value. [O(value.size())] 
        ( 
            const vector & value 
        ) ; 
        vector // Construct a sequence from [first,last).   [O(last-first)]
        ( 
            const_iterator      first                     ,    
            const_iterator      last                      ,    
            const Allocator &   allocator   = Allocator()
        );
        void assign // Replace *this with the sequence [first,last).   [O[size()+last-first)]
        ( 
            const_iterator  first   , 
            const_iterator  last    
        );
        void assign  // Replace *this with n copies of value. [O(size()+n)]
        ( 
            size_type   n           , 
            const T &   value = T() 
        );

        vector & operator = ( const vector & rhs ); // [O(size()+rhs.size())]

        ~vector(); // [O(size()]

        //---------------------------------------------------------------------
        // Iteration
        //---------------------------------------------------------------------
        // Each iterator type I (iterator, const_iterator, and so on) has these functions:
        // WARNING: Avoid using the inefficient post-increment i++ and post-decrement i--. 
        //
        //    I &       operator ++ ()                // Go to next item.
        //    I         operator ++ ( int )           // Go to next item. (Post-increment - avoid using this).
        //    I &       operator -- ()                // Go to previous item.
        //    I         operator -- ( int )           // Go to previous item (Post-decrement - avoid using this).
        //    I &       operator += (ptrdiff_t n)     // Go to n'th next item.
        //    I &       operator -= (ptrdiff_t n)     // Go to n'th previous item.
        //    I         operator +  (ptrdiff_t n)     // The n'th next item.
        //    I         operator -  (ptrdiff_t n)     // The n'th previous item.
        //    const T & operator *  ()                // The vector element referenced by the iterator
        //    const T * operator -> ()                // The list element referenced by the iterator.
        //    T &       operator [] (ptrdiff_t n)     // The vector element at position n from the iterator.
        //    x_bool    operator == ( const I & rhs ) // Do two iterators represent the same position in the vector?
        //    x_bool    operator != ( const I & rhs ) // Do two iterators represent different positions in the vector?
        //
        // For the non-const iterators, operator * and -> return non-const values.
        // Remember that for a reverse_iterator or const_reverse_iterator, increment means
        // iterate to the previous item in the list and decrement means iterate to the next
        // item in the list.

        iterator                begin   ()          ; // The start of forward iteration (the first item in the vector).
        const_iterator          begin   ()  const   ; // The start of forward iteration (the first item in the vector).
        iterator                end     ()          ; // The end of forward iteration (beyond the last item in the vector).
        const_iterator          end     ()  const   ; // The end of forward iteration (beyond the last item in the vector).
        reverse_iterator        rbegin  ()          ; // The start of reverse iteration (last item in the vector).
        const_reverse_iterator  rbegin  ()  const   ; // The start of reverse iteration (last item in the vector).
        reverse_iterator        rend    ()          ; // The end of reverse iteration (beyond the first item in the vector).
        const_reverse_iterator  rend    ()  const   ; // The end of reverse iteration (beyond the first item in the vector).

        //---------------------------------------------------------------------
        // Capacity and size
        //---------------------------------------------------------------------
        size_type   size     () const                   ; // How many elements are in *this?
        x_bool      empty    () const                   ; // Is *this empty? 
        size_type   capacity () const                   ; // How big can *this can get before reallocation occurs?
        void        reserve  ( size_type n )            ; // [O(max(size(),n))] Change the capacity to at least n. Does not change the current size.
        size_type   max_size ()                 const   ; // What is the largest size this class can support? 
        void        resize   ( size_type n , T value = T() ); // [O(max(size(),n))] Change the size of *this to n, removing elements or adding value as needed. 

        //---------------------------------------------------------------------
        // Indexing , first and last elements
        //---------------------------------------------------------------------
        // It is a fundamental property of vector<> that indexing is fast
        // (constant time, independent of the vector size).

              T & at         ( size_type pos )        ; // The element at index pos in the vector. Raise an error if pos is not in [0,size()).
        const T & at         ( size_type pos ) const  ; // The element at index pos in the vector. Raise an error if pos is not in [0,size()).
              T & operator[] ( size_type pos )        ; // The element at index pos in the vector. Raise an error if pos is not in [0,size()).
        const T & operator[] ( size_type pos ) const  ; // The element at index pos in the vector. Raise an error if pos is not in [0,size()).
              T & front      ()                       ; // The first element in the vector. Raise an error if empty(). 
        const T & front      ()                const  ; // The first element in the vector. Raise an error if empty().
              T & back       ()                       ; // The last element in the vector. Raise an error if empty().
        const T & back       ()                const  ; // The last element in the vector. Raise an error if empty().

        //---------------------------------------------------------------------
        // Insertion, deletion
        //---------------------------------------------------------------------

        void        push_back   ( const T & value ) ; // [O(1),O(size())] Insert x at the end of *this.
        void        pop_back    ( )                 ; // Remove the last element from *this.
        void        clear       ( )                 ; // [O(size())] Empty the array. Sets the size() to 0.

        // Insert a single value, n values, or a range of values in [first,last)
        // before the element *it of the vector.
        iterator    insert      ( iterator it , const T & value = T() ) ; // [O(end()-it), O(size())]
        void        insert      ( iterator it , size_type n , const T & value ) ;  // [O(n+end()-it), O(size()+n)]
        void        insert      ( iterator it , const_iterator first , const_iterator last ) ; // [O(last-first+end()-it), O(size()+last-first) ]

        // Erase the single element at *it or erase the range of values in [first,last).
        // Elements to the right of the erased range are moved over to fill the erased elements.
        iterator    erase       ( iterator it ) ; // [O(end()-it)]
        iterator    erase       ( iterator first , iterator last ) ; // [O(last-first+end()-last)]

        //---------------------------------------------------------------------
        // Miscellaneous
        //---------------------------------------------------------------------
        void swap ( vector & x ) ; 
        Allocator get_allocator ( ) const { return allocator ; } 

        //---------------------------------------------------------------------
        // Searching
        //---------------------------------------------------------------------
        // There are no searching functions, but you can use these functions:
        //    x_algorithm.h:  find( x.begin() , x.end() , value )

        //---------------------------------------------------------------------
        // const_iterator
        //---------------------------------------------------------------------
        class const_iterator
        {
          public:
            const_iterator()                                          { m_pointer = 0 ; }
            explicit const_iterator( iterator x )                     { m_pointer = x.m_pointer ; }
            const_iterator & operator ++ ()                           { m_pointer += 1 ; return *this ; }
            const_iterator & operator -- ()                           { m_pointer -= 1 ; return *this ; }
            const_iterator & operator += (size_type n)                { m_pointer += n ; return *this ; }
            const_iterator & operator -= (size_type n)                { m_pointer -= n ; return *this ; }
            const_iterator   operator +  (size_type n) const          { return m_pointer + n ; }
            const_iterator   operator -  (size_type n) const          { return m_pointer - n ; }
            const_iterator   operator ++ (int)                        { return m_pointer++; }
            const_iterator   operator -- (int)                        { return m_pointer--; }
            const T &        operator *  ()      const                { return *m_pointer ; }
            const T *        operator -> ()      const                { return m_pointer ; }
            const T &        operator [] (size_type n) const          { return m_pointer[n]; }

            x_bool           operator == (const const_iterator & rhs) const { return m_pointer == rhs.m_pointer ; }
            x_bool           operator != (const const_iterator & rhs) const { return m_pointer != rhs.m_pointer ; }
            x_bool           operator <  (const const_iterator & rhs) const { return m_pointer <  rhs.m_pointer ; }
            x_bool           operator <= (const const_iterator & rhs) const { return m_pointer <= rhs.m_pointer ; }
            x_bool           operator >  (const const_iterator & rhs) const { return m_pointer >  rhs.m_pointer ; }
            x_bool           operator >= (const const_iterator & rhs) const { return m_pointer >= rhs.m_pointer ; }

          protected :

            friend class vector<T,Allocator> ;
            const T * m_pointer ; // The pointer to the current element.
            const_iterator( const T * pointer ) : m_pointer(pointer) {}
        };

        //---------------------------------------------------------------------
        // iterator
        //---------------------------------------------------------------------
        class iterator
        {
          public:

            iterator()                                          { m_pointer = 0 ; }
            iterator & operator ++ ()                           { m_pointer += 1 ; return *this ; }
            iterator & operator -- ()                           { m_pointer -= 1 ; return *this ; }
            iterator & operator += (size_type n)                { m_pointer += n ; return *this ; }
            iterator & operator -= (size_type n)                { m_pointer -= n ; return *this ; }
            iterator   operator +  (size_type n) const          { return m_pointer + n ; }
            iterator   operator -  (size_type n) const          { return m_pointer - n ; }
            iterator   operator ++ (int)                        { return m_pointer++; }
            iterator   operator -- (int)                        { return m_pointer--; }
            T &        operator *  ()      const                { return *m_pointer ; }
            T *        operator -> ()      const                { return m_pointer ; }
            T &        operator [] (size_type n) const          { return m_pointer[n]; }

            x_bool           operator == (const iterator & rhs) const { return m_pointer == rhs.m_pointer ; }
            x_bool           operator != (const iterator & rhs) const { return m_pointer != rhs.m_pointer ; }
            x_bool           operator <  (const iterator & rhs) const { return m_pointer <  rhs.m_pointer ; }
            x_bool           operator <= (const iterator & rhs) const { return m_pointer <= rhs.m_pointer ; }
            x_bool           operator >  (const iterator & rhs) const { return m_pointer >  rhs.m_pointer ; }
            x_bool           operator >= (const iterator & rhs) const { return m_pointer >= rhs.m_pointer ; }

          protected :

            friend class vector<T,Allocator> ;
            T * m_pointer ; // The pointer to the current element.
            iterator( T * pointer ) : m_pointer(pointer) {}
        };

        //---------------------------------------------------------------------
        // const_reverse_iterator
        //---------------------------------------------------------------------
        class const_reverse_iterator
        {
          public:

            const_reverse_iterator()                                          { m_pointer = 0 ; }
            explicit const_reverse_iterator( reverse_iterator x )             { m_pointer = x.m_pointer ; }
            const_reverse_iterator & operator ++ ()                           { m_pointer -= 1 ; return *this ; }
            const_reverse_iterator & operator -- ()                           { m_pointer += 1 ; return *this ; }
            const_reverse_iterator & operator += (size_type n)                { m_pointer -= n ; return *this ; }
            const_reverse_iterator & operator -= (size_type n)                { m_pointer += n ; return *this ; }
            const_reverse_iterator   operator +  (size_type n) const          { return m_pointer - n ; }
            const_reverse_iterator   operator -  (size_type n) const          { return m_pointer + n ; }
            const_reverse_iterator   operator ++ (int)                        { return m_pointer--; }
            const_reverse_iterator   operator -- (int)                        { return m_pointer++; }
            const T &                operator *  ()      const                { return m_pointer[-1] ; }
            const T *                operator -> ()      const                { return m_pointer-1 ; }
            const T &                operator [] (size_type n) const          { return m_pointer[-1-n]; }

            x_bool           operator == (const const_reverse_iterator & rhs) const { return m_pointer == rhs.m_pointer ; }
            x_bool           operator != (const const_reverse_iterator & rhs) const { return m_pointer != rhs.m_pointer ; }
            x_bool           operator <  (const const_reverse_iterator & rhs) const { return m_pointer >  rhs.m_pointer ; }
            x_bool           operator <= (const const_reverse_iterator & rhs) const { return m_pointer >= rhs.m_pointer ; }
            x_bool           operator >  (const const_reverse_iterator & rhs) const { return m_pointer <  rhs.m_pointer ; }
            x_bool            operator >= (const const_reverse_iterator & rhs) const { return m_pointer <= rhs.m_pointer ; }

          protected :

            friend class vector<T,Allocator> ;
            const T * m_pointer ; // The pointer to ONE BEYOND the current element.
            const_reverse_iterator( const T * pointer ) : m_pointer(pointer) {}
        };

        //---------------------------------------------------------------------
        // reverse_iterator
        //---------------------------------------------------------------------
        class reverse_iterator
        {
          public:

            reverse_iterator()                                          { m_pointer = 0 ; }
            reverse_iterator & operator ++ ()                           { m_pointer -= 1 ; return *this ; }
            reverse_iterator & operator -- ()                           { m_pointer += 1 ; return *this ; }
            reverse_iterator & operator += (size_type n)                { m_pointer -= n ; return *this ; }
            reverse_iterator & operator -= (size_type n)                { m_pointer += n ; return *this ; }
            reverse_iterator   operator +  (size_type n) const          { return m_pointer - n ; }
            reverse_iterator   operator -  (size_type n) const          { return m_pointer + n ; }
            reverse_iterator   operator ++ (int)                        { return m_pointer--; }
            reverse_iterator   operator -- (int)                        { return m_pointer++; }
            const T &          operator *  ()      const                { return m_pointer[-1] ; }
            const T *          operator -> ()      const                { return m_pointer-1 ; }
            const T &          operator [] (size_type n) const          { return m_pointer[-1-n]; }

            x_bool             operator == (const reverse_iterator & rhs) const { return m_pointer == rhs.m_pointer ; }
            x_bool             operator != (const reverse_iterator & rhs) const { return m_pointer != rhs.m_pointer ; }
            x_bool             operator <  (const reverse_iterator & rhs) const { return m_pointer >  rhs.m_pointer ; }
            x_bool             operator <= (const reverse_iterator & rhs) const { return m_pointer >= rhs.m_pointer ; }
            x_bool             operator >  (const reverse_iterator & rhs) const { return m_pointer <  rhs.m_pointer ; }
            x_bool             operator >= (const reverse_iterator & rhs) const { return m_pointer <= rhs.m_pointer ; }

          protected :

            friend class vector<T,Allocator> ;
            const T * m_pointer ; // The pointer to ONE BEYOND the current element.
            reverse_iterator( const T * pointer ) : m_pointer(pointer) {}
        };

    
      protected:

        Allocator allocator ;

        //---------------------------------------------------------------------
        // Private details of vector<>
        //---------------------------------------------------------------------

      private:

        size_type   m_capacity      ; // m_array[] has room for this many elements.
        size_type   m_size          ; // The number of elements used (at the beginning) in m_array[]. 
        T *         m_array         ; // The elements in the array. Room for m_capacity, but only the first m_size are used.

        // Replace m_array with an (uninitialized, unconstructed) array with a capacity of at least
        // n. Also changes m_capacity, but does not change m_size. Return the old array, which must
        // be destructed/deallocated as appropriate.
        T * reallocate( size_type min_capacity );

        // Construct n values at *pointer use value.
        void construct( T * pointer , size_type n , const T & value ) ; 

        // Construct n values starting at *pointer with value.
        void construct_up( T * pointer , const T & value , size_type n ); 

        // Construct n values ending at *pointer with value.
        void construct_down( T * pointer , const T & value , size_type n ); 

        // Construct n values starting at *pointer with the n values starting at values[0].
        void construct_up( T * pointer , const T * values , size_type n ); 

        // Construct n values ending at *pointer with the n values ending at values[0].
        void construct_down( T * pointer , const T * values , size_type n ); 

        // Copy value into the n values starting at *pointer.
        void copy_up( T * pointer , const T & value , size_type n ); 

        // Copy value into the n values ending at *pointer.
        void copy_down( T * pointer , const T & value , size_type n ); 

        // Copy n values starting at *values into the n values starting at *pointer.
        void copy_up( T * pointer , const T * values , size_type n ); 

        // Copy n values ending at *values into the n values ending at *pointer.
        void copy_down( T * pointer , const T * values , size_type n ); 

        // Destroy the n elements starting at *pointer. Does nothing if pointer == 0.
        void destroy_up( T * pointer , size_type n ); 

        // Destroy the n elements ending at *pointer. Does nothing if pointer == 0.
        void destroy_down( T * pointer , size_type n ); 

        // Insert n elements at position pos, moving the elements over by n positions
        // starting with the element at pos. The new elements are all constructed or copied
        // from value.
        void insert( size_type pos , size_type n , const T & value ) ; 

        // Insert n elements at position pos, moving the elements over by n positions
        // starting with the element at pos. The new elements are all constructed or
        // copied from the elements starting at values.
        void insert( size_type pos , size_type n , const T * values ) ; 

        // Erase n elements starting with the pos'th one and moving all the following
        // elements to the left by n positions - destroying those left beyond the 
        // new end of the array.
        void erase( size_type pos , size_type n ) ; 
    };

    ///////////////////////////////////////////////////////////////////////////
    //  
    // Template functions and operators for vector<T,A>
    //
    ///////////////////////////////////////////////////////////////////////////
    // All of these have time complexity O( min(lhs.size(),rhs.size()) )
    // except swap(), which has O(1).

    template<class T, class A> x_bool operator == ( const vector<T,A> & lhs , const vector<T,A> & rhs );
    template<class T, class A> x_bool operator != ( const vector<T,A> & lhs , const vector<T,A> & rhs );
    template<class T, class A> x_bool operator <  ( const vector<T,A> & lhs , const vector<T,A> & rhs );
    template<class T, class A> x_bool operator >  ( const vector<T,A> & lhs , const vector<T,A> & rhs );
    template<class T, class A> x_bool operator <= ( const vector<T,A> & lhs , const vector<T,A> & rhs );
    template<class T, class A> x_bool operator >= ( const vector<T,A> & lhs , const vector<T,A> & rhs );
    template<class T, class A> void  swap        ( const vector<T, A>& lhs , const vector<T,A> & rhs );

    ///////////////////////////////////////////////////////////////////////////
    //  
    // Implementation details 
    //
    ///////////////////////////////////////////////////////////////////////////

    template<class T, class A> inline       T & vector<T,A>::at         ( size_type pos )        { X_STD_ASSERT( pos >= 0 ); X_STD_ASSERT( pos < m_size ); return m_array[pos] ; }
    template<class T, class A> inline const T & vector<T,A>::at         ( size_type pos ) const  { X_STD_ASSERT( pos >= 0 ); X_STD_ASSERT( pos < m_size ); return m_array[pos] ; }
    template<class T, class A> inline       T & vector<T,A>::operator[] ( size_type pos )        { X_STD_ASSERT( pos >= 0 ); X_STD_ASSERT( pos < m_size ); return m_array[pos] ; }
    template<class T, class A> inline const T & vector<T,A>::operator[] ( size_type pos ) const  { X_STD_ASSERT( pos >= 0 ); X_STD_ASSERT( pos < m_size ); return m_array[pos] ; }
    template<class T, class A> inline       T & vector<T,A>::front      ()                       { X_STD_ASSERT( m_size > 0 ); return m_array[0] ; }
    template<class T, class A> inline const T & vector<T,A>::front      ()                const  { X_STD_ASSERT( m_size > 0 ); return m_array[0] ; }
    template<class T, class A> inline       T & vector<T,A>::back       ()                       { X_STD_ASSERT( m_size > 0 ); return m_array[m_size-1] ; }
    template<class T, class A> inline const T & vector<T,A>::back       ()                const  { X_STD_ASSERT( m_size > 0 ); return m_array[m_size-1] ; }

    template<class T, class A> inline typename vector<T,A>::size_type    vector<T,A>::size     () const { return m_size ; }
    template<class T, class A> inline x_bool                             vector<T,A>::empty    () const { return m_size == 0 ; }
    template<class T, class A> inline typename vector<T,A>::size_type    vector<T,A>::capacity () const { return m_capacity ; }
    template<class T, class A> inline typename vector<T,A>::size_type    vector<T,A>::max_size () const { return 10000000 ; }

    template<class T, class A> inline typename vector<T,A>::iterator                vector<T,A>::begin   ()          { return iterator               ( &m_array[ 0      ] ); }
    template<class T, class A> inline typename vector<T,A>::const_iterator          vector<T,A>::begin   () const    { return const_iterator         ( &m_array[ 0      ] ); }
    template<class T, class A> inline typename vector<T,A>::iterator                vector<T,A>::end     ()          { return iterator               ( &m_array[ m_size ] ); }
    template<class T, class A> inline typename vector<T,A>::const_iterator          vector<T,A>::end     () const    { return const_iterator         ( &m_array[ m_size ] ); }
    template<class T, class A> inline typename vector<T,A>::reverse_iterator        vector<T,A>::rbegin  ()          { return reverse_iterator       ( &m_array[ m_size ] ); }
    template<class T, class A> inline typename vector<T,A>::const_reverse_iterator  vector<T,A>::rbegin  () const    { return const_reverse_iterator ( &m_array[ m_size ] ); }
    template<class T, class A> inline typename vector<T,A>::reverse_iterator        vector<T,A>::rend    ()          { return reverse_iterator       ( &m_array[ 0      ] ); }
    template<class T, class A> inline typename vector<T,A>::const_reverse_iterator  vector<T,A>::rend    () const    { return const_reverse_iterator ( &m_array[ 0      ] ); }
    
    //-------------------------------------------------------------------------
    template< class T , class A >
    inline T * vector<T,A>::reallocate( size_type min_capacity )
    {
        X_STD_ASSERT( min_capacity <= max_size() );
        // (We always at least double the current capacity.)
        T * old_array   =   m_array ;
        m_capacity      =   x_std::max( m_capacity * 2 , min_capacity );
        m_array         =   allocator.allocate( m_capacity );
        return old_array ;
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::construct( T * pointer , size_type n , const T & value )
    {
        for( ; n > 0 ; ++ pointer , -- n )
        {
            allocator.construct( pointer , value );
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::construct_up( T * pointer , const T & value , size_type n )
    {
        for( ; n > 0 ; ++ pointer , -- n )
        {
            allocator.construct( pointer , value );
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::construct_down( T * pointer , const T & value , size_type n )
    {
        for( ; n > 0 ; -- pointer , -- n )
        {
            allocator.construct( pointer , value );
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::construct_up( T * pointer , const T * values , size_type n )
    {
        for( ; n > 0 ; ++ pointer , ++ values , -- n )
        {
            allocator.construct( pointer , *values );

        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::construct_down( T * pointer , const T * values , size_type n )
    {
        for( ; n > 0 ; -- pointer , -- values , -- n )
        {
            allocator.construct( pointer , *values );
        }
    }


    //-------------------------------------------------------------------------
    template< class T , class A >
    inline void vector<T,A>::copy_up( T * pointer , const T & value , size_type n )
    {
        for( ; n > 0 ; ++ pointer , -- n )
        {
            *pointer = value ;
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    inline void vector<T,A>::copy_down( T * pointer , const T & value , size_type n )
    {
        for( ; n > 0 ; -- pointer , -- n )
        {
            *pointer = value ;
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    inline void vector<T,A>::copy_up( T * pointer , const T * values , size_type n )
    {
        for( ; n > 0 ; ++ pointer , ++ values , -- n )
        {
            *pointer = *values ;
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    inline void vector<T,A>::copy_down( T * pointer , const T * values , size_type n )
    {
        for( ; n > 0 ; -- pointer , -- values , -- n )
        {
            *pointer = *values ;
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::destroy_up( T * pointer , size_type n )
    {
        for( ; n > 0 ; ++ pointer , -- n )
        {
            allocator.destroy(pointer);
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::destroy_down( T * pointer , size_type n )
    {
        for( ; n > 0 ; -- pointer , -- n )
        {
            allocator.destroy(pointer);
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::reserve( size_type n )
    {
        X_STD_ASSERT( n <= max_size() );
        if( m_capacity >= n )
        {
            // No change needed - we already have enough room.
        }
        else 
        {
            const size_type old_capacity    =   m_capacity      ;
            T *             old_array       =   reallocate( n ) ;
            construct_up( m_array , old_array , m_size ); // Copy the old elements into the new array.
            destroy_up( old_array , m_size ); // Destruct the old elements (just the used ones).
            allocator.deallocate( old_array , old_capacity );
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    vector<T,A>::vector
    ( 
        const A & allocator_
    )
        :   allocator(allocator_)
    {
        m_capacity      =   0           ;
        m_size          =   0           ;
        m_array         =   0           ;
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    vector<T,A>::~vector()
    {
        clear();
        allocator.deallocate(m_array,m_capacity);
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    vector<T,A>::vector
    ( 
        size_type   n           ,    
        const T &   value       , 
        const A &   allocator_
    )
        :   allocator(allocator_)
    {
        m_capacity      =   0           ;
        m_size          =   0           ;
        m_array         =   0           ;
        assign( n , value );
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    vector<T,A>::vector
    ( 
        const vector & x 
    ) 
        :   allocator(x.allocator)
    {
        m_capacity  =   0   ;
        m_size      =   0   ;
        m_array     =   0   ;
        reserve( x.size() );
        assign( x.begin() , x.end() );
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    vector<T,A>::vector
    ( 
        const_iterator  first           , 
        const_iterator  last            , 
        const A &       allocator_
    )
        :   allocator(allocator_)
    {
        m_capacity      =   0           ;
        m_size          =   0           ;
        m_array         =   0           ;
        assign( first , last );
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::assign 
    ( 
        const_iterator  first   , 
        const_iterator  last    
    )
    {
        clear();
        insert( begin() , first , last );
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    vector<T,A> & vector<T,A>::operator = ( const vector & rhs ) 
    {
        if( this != &rhs ) // If not assigning to self
        {
            assign( rhs.begin() , rhs.end() );
        }
        return *this ;
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::assign  
    ( 
        size_type   n       , 
        const T &   value 
    )
    {
        clear();
        reserve( n );
        for( ; n > 0 ; n-- )
        {
            push_back(value);
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::resize( size_type n , T value )
    {
        X_STD_ASSERT( n <= max_size() );
        if( n <= 0 )
        {
            clear();
        }
        else if( n > m_size )
        {
            insert( m_size , n-m_size , value );
        }
        else if( n < m_size )
        {
            erase( n , m_size-n );
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::push_back( const T & value )
    {
        if( m_capacity <= m_size ) // Not enough room for one more?
        {
            reserve( m_size + 1 ); // Add room for one more
        }
        allocator.construct( &m_array[m_size] , value );
        ++m_size;
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::pop_back( )
    {
        X_STD_ASSERT( m_size > 0 );
        --m_size;
        allocator.destroy( &m_array[m_size] );
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    typename vector<T,A>::iterator vector<T,A>::insert( iterator it , const T & value  ) 
    {
        const size_type pos = it.m_pointer - m_array ;
        insert( pos , 1 , value );
        return &m_array[pos];
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::insert( size_type pos , size_type n , const T & value ) 
    {
        X_STD_ASSERT( pos <= max_size() );
        X_STD_ASSERT( pos <=  m_size  );
        X_STD_ASSERT( pos >=  0       );
        if( n > 0 )
        {
            const size_type new_size        =   m_size + n      ; 
            const size_type left_count      =   pos             ;   // The number of elements left of pos which don't need to be moved.
            const size_type right_count     =   m_size - pos    ;   // The number of elements right of pos which must be moved over.

            if( m_capacity < new_size ) // Not enough room for new elements without reallocating?
            {
                // We need to reallocate
                const size_type old_capacity    =   m_capacity  ;
                T *             old_array       =   reallocate( new_size );
                construct_up( m_array , old_array , left_count );
                construct_up( &m_array[pos] , value , n );
                construct_up( &m_array[pos+n] , &old_array[pos] , right_count );
                destroy_up( old_array , m_size ); // Destruct the old elements (just the used ones).
                allocator.deallocate( old_array , old_capacity );
            }
            else if( n >= right_count ) // The new elements won't all fit (or just exactly fit) in already initialized spaces
            {
                // Old array:
                // +---+---+---+---+---+---+---+---+---+---+---+---+---+
                // | 0 | 1 | 2 | 3 | 4 | 5 |///|///|///|///|///|///|///|
                // +---+---+---+---+---+---+---+---+---+---+---+---+---+
                //          pos 
                // New elements:
                //         +---+---+---+---+---+
                //         | A | B | C | D | E |
                //         +---+---+---+---+---+
                // Result:
                // +---+---+---+---+---+---+---+---+---+---+---+---+---+
                // | 0 | 1 | A | B | C | D | E | 2 | 3 | 4 | 5 |///|///|
                // +---+---+---+---+---+---+---+---+---+---+---+---+---+
                // All of the old elements from pos on (2,3,4,5) are copied to uninitialized entries.
                // Some of the new elements (such as A,B,C,D) are copied to initialized entries.
                // Some of the new entries (such as E) are copied into initialized entries.
                const size_type move_construct_count    =   right_count     ; // The number of old elements which must be moved by construction.
                const size_type new_construct_count     =   n - right_count ; // The number of new elements which must be created by construction.
                const size_type new_copy_count          =   right_count     ; // The number of new elements which must be created by copying.

                construct_down  ( &m_array[ new_size-1          ] , &m_array[ m_size-1       ] , move_construct_count        ); 
                construct_up    ( &m_array[ pos+new_copy_count  ] , &m_array[ new_copy_count ] , new_construct_count         );
                copy_up         ( &m_array[ pos                 ] , value                      , new_copy_count              );
            }

            else // The new elements all fit in spaces already initialized
            {
                // Old array:
                // +---+---+---+---+---+---+---+---+---+---+---+
                // | 0 | 1 | 2 | 3 | 4 | 5 |///|///|///|///|///|
                // +---+---+---+---+---+---+---+---+---+---+---+
                //          pos 
                // New elements:
                //         +---+---+---+
                //         | A | B | C |
                //         +---+---+---+
                // Result:
                // +---+---+---+---+---+---+---+---+---+---+---+
                // | 0 | 1 | A | B | C | 2 | 3 | 4 | 5 |///|///|
                // +---+---+---+---+---+---+---+---+---+---+---+
                // Some of the old elements (such as 3,4,5) are copied to uninitialized entries
                // and some of the old elements (such as 2) are copied to initialized entries.
                // All the new entries are copied into initialized entries.
                const size_type move_construct_count    =   n ; // The number of old elements which must be moved by construction.
                const size_type move_copy_count         =   right_count - n ; // The number of old elements which must be moved by copying.
                const size_type new_copy_count          =   n               ; // The number of new elements which must be created by copying.

                construct_down  ( &m_array[ new_size-1          ] , &m_array[ m_size-1  ] , move_construct_count        ); 
                copy_down       ( &m_array[ m_size-1            ] , &m_array[ m_size-1-n] , move_copy_count             );
                copy_up         ( &m_array[ pos                 ] , value                 , new_copy_count              );
            }
            m_size = new_size ;
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::insert( size_type pos , size_type n , const T * values ) 
    {
        X_STD_ASSERT( pos <=  max_size()  );
        X_STD_ASSERT( pos <=  m_size      );
        X_STD_ASSERT( pos >=  0           );
        if( n > 0 )
        {
            const size_type new_size    =   m_size + n      ; 
            const size_type left_count  =   pos             ;   // The number of elements left of pos which don't need to be moved.
            const size_type right_count =   m_size - pos    ;   // The number of elements right of pos which must be moved over.

            if( m_capacity < new_size ) // Not enough room for new elements without reallocating?
            {
                // We need to reallocate
                const size_type old_capacity    =   m_capacity  ;
                T *             old_array       =   reallocate( new_size );
                construct_up( m_array , old_array , left_count );
                construct_up( &m_array[pos] , values , n );
                construct_up( &m_array[pos+n] , &old_array[pos] , right_count );
                destroy_up( old_array , m_size ); // Destruct the old elements (just the used ones).
                allocator.deallocate( old_array , old_capacity );
            }
            else if( n >= right_count ) // The new elements won't all fit (or just exactly fit) in already initialized spaces
            {
                // Old array:
                // +---+---+---+---+---+---+---+---+---+---+---+---+---+
                // | 0 | 1 | 2 | 3 | 4 | 5 |///|///|///|///|///|///|///|
                // +---+---+---+---+---+---+---+---+---+---+---+---+---+
                //          pos 
                // New elements:
                //         +---+---+---+---+---+
                //         | A | B | C | D | E |
                //         +---+---+---+---+---+
                // Result:
                // +---+---+---+---+---+---+---+---+---+---+---+---+---+
                // | 0 | 1 | A | B | C | D | E | 2 | 3 | 4 | 5 |///|///|
                // +---+---+---+---+---+---+---+---+---+---+---+---+---+
                // All of the old elements from pos on (2,3,4,5) are copied to uninitialized entries.
                // Some of the new elements (such as A,B,C,D) are copied to initialized entries.
                // Some of the new entries (such as E) are copied into initialized entries.
                const size_type move_construct_count    =   right_count     ; // The number of old elements which must be moved by construction.
                const size_type new_construct_count     =   n - right_count ; // The number of new elements which must be created by construction.
                const size_type new_copy_count          =   right_count     ; // The number of new elements which must be created by copying.

                construct_down  ( &m_array[ new_size-1          ] , &m_array[ m_size-1       ] , move_construct_count        ); 
                construct_up    ( &m_array[ pos+new_copy_count  ] , &values [ new_copy_count ] , new_construct_count         );
                copy_up         ( &m_array[ pos                 ] , values                     , new_copy_count              );
            }
            else // The new elements all fit in spaces already initialized
            {
                // Old array:
                // +---+---+---+---+---+---+---+---+---+---+---+
                // | 0 | 1 | 2 | 3 | 4 | 5 |///|///|///|///|///|
                // +---+---+---+---+---+---+---+---+---+---+---+
                //          pos 
                // New elements:
                //         +---+---+---+
                //         | A | B | C |
                //         +---+---+---+
                // Result:
                // +---+---+---+---+---+---+---+---+---+---+---+
                // | 0 | 1 | A | B | C | 2 | 3 | 4 | 5 |///|///|
                // +---+---+---+---+---+---+---+---+---+---+---+
                // Some of the old elements (such as 3,4,5) are copied to uninitialized entries
                // and some of the old elements (such as 2) are copied to initialized entries.
                // All the new entries are copied into initialized entries.
                const size_type move_construct_count    =   n ; // The number of old elements which must be moved by construction.
                const size_type move_copy_count         =   right_count - n ; // The number of old elements which must be moved by copying.
                const size_type new_copy_count          =   n               ; // The number of new elements which must be created by copying.

                construct_down  ( &m_array[ new_size-1          ] , &m_array[ m_size-1  ] , move_construct_count        ); 
                copy_down       ( &m_array[ m_size-1            ] , &m_array[ m_size-1-n] , move_copy_count             );
                copy_up         ( &m_array[ pos                 ] , values                , new_copy_count              );
            }
            m_size = new_size ;

        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::insert( iterator it , size_type n , const T & value ) 
    {
        insert( it.m_pointer - m_array , n , value );
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::insert( iterator it , const_iterator first , const_iterator last )
    {
        if( last > first )
        {
            const size_type pos = it.m_pointer   - m_array          ;
            const size_type n   = last.m_pointer - first.m_pointer  ;
            insert( pos , n , &*first );
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    typename vector<T,A>::iterator vector<T,A>::erase( iterator it ) 
    {
        const size_t pos = it.m_pointer - m_array ;
        erase( pos , 1 );
        return it ;
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::erase( size_type pos , size_type n )
    {
        X_STD_ASSERT( pos <= max_size() );
        X_STD_ASSERT( pos >= 0          );
        X_STD_ASSERT( pos <= m_size     );
        if( pos < m_size )
        {
            if( pos + n > m_size ) // Deleting more than possible?
            {
                n = m_size - pos ;
            }
            // How many elements are left to the right of the erased ones?
            const size_type right_count = m_size - pos - n ; 

            copy_up( &m_array[pos] , &m_array[pos+n] , right_count );
            m_size -= n ;
            destroy_up( &m_array[m_size] , n );
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    typename vector<T,A>::iterator vector<T,A>::erase( iterator first , iterator last )
    {
        
        if( last > first )
        {
            const size_type pos = first.m_pointer - m_array         ;
            const size_type n   = last.m_pointer  - first.m_pointer ;
            erase( pos , n );
        }
        return first ;
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::clear( )
    {
        erase( 0 , m_size );
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    void vector<T,A>::swap( vector & x )
    {
        x_std::swap( this->m_array      , x.m_array     );
        x_std::swap( this->m_capacity   , x.m_capacity  );
        x_std::swap( this->m_size       , x.m_size      );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator == ( const vector<T,A> & lhs , const vector<T,A> & rhs )
    {
        return lhs.size() == rhs.size() && x_std::equal( lhs.begin() , lhs.end() , rhs.begin() );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator != ( const vector<T,A> & lhs , const vector<T,A> & rhs )
    {
        return ! ( lhs == rhs ) ; 
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator <  ( const vector<T,A> & lhs , const vector<T,A> & rhs )
    {
        return lexicographical_compare( lhs.begin() , lhs.end() , rhs.begin() , rhs.end() );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator >  ( const vector<T,A> & lhs , const vector<T,A> & rhs )
    {
        return rhs < lhs ;
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator <= ( const vector<T,A> & lhs , const vector<T,A> & rhs )
    {
        return ! ( rhs < lhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator >= ( const vector<T,A> & lhs , const vector<T,A> & rhs )
    {
        return ! ( lhs < rhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> void  swap        ( const vector<T, A>& lhs , const vector<T,A> & rhs )
    {
        lhs.swap(rhs);
    }

}

#endif
    

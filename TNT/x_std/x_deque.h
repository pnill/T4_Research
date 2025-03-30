#if !defined( X_STD_DEQUE_H )
#define X_STD_DEQUE_H

// ********* THIS FILE IS UNDER CONSTRUCTION **********

///////////////////////////////////////////////////////////////////////////////
//
// 
// Defines the template class deque<T,Allocator> and support template functions.
//
// A deque container allows amortized constant-time insertions and removals at
// either end of the queue.
//
///////////////////////////////////////////////////////////////////////////////
//
// Overview of basic functions for deque<T,Container>
//  
//    deque<T> X ;      // Declare X as a deque using the default container.
//    X.size()          // The number of elements in the deque.
//    X.empty()         // Is the deque empty?
//    X.resize(10,x);   // Resize the deque to size 10, adding copies of x if needed.
//    X[12]             // The element at position 12.
//    X.front()         // The first element in the deque.
//    X.back()          // The last element in the deque.
//    X.push_front(x);  // Add x to the front of the deque.
//    X.pop_front();    // Remove the first element of the deque.
//    X.push_back(x);   // Add x to the end of the deque.
//    X.pop_back();     // Remove the last element of the deque.
//    X.insert(it,x);   // Insert value x at iterator it in the deque.
//    X.insert(it,n,x); // Insert n copies of x in the deque at iterator it.
//    X.insert(it,first,last) // Insert the values in iterator range [first,last) at iterator it.
//    X.erase(it);          // Erase the element at iterator it.
//    X.erase(first,last);  // Erase the elements in iterator range [first,last).
//    X.clear();        // Empty the sequence.
//
// Forward iteration:
//
//     x_std::deque<T>::const_iterator i ;
//     for( i = X.begin() ; i != X.end() ; ++i )
//     {
//         const T & Element = *i ; 
//     }
//
// Reverse iteration:
//
//     x_std::deque<T>::const_reverse_iterator i ;
//     for( i = X.rbegin() ; i != X.rend() ; ++i )
//     {
//         const T & Element = *i ; 
//     }
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
//
///////////////////////////////////////////////////////////////////////////////
//
// Performance
//
// The time-complexity of each function is described near its declaration as 
// [O(f)], or [O(f1),O(f2)] where f, f1, and f2 are functions of the arguments 
// and *this. If two functions are given, f1 describes the case where deque
// reallocation is not needed and f2 describes the case where reallocation is needed.
//
//    O(size())  The time depends on the size of the deque.
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
    // deque< T , Allocator >
    //
    // This template class is a varying-length sequence of elements of type T
    // which uses a protected object of type allocator of class Allocator to
    // allocate and free storage for the sequence.
    //
    // The sequence permits amortized constant-time insertion or removal of an 
    // element at either end. Such operations in the middle of the sequence 
    // require element copies and assignments proportional to the number of 
    // elements in the sequence (linear time).
    //
    // The Allocator object must have the same external interface as that of 
    // template class allocator<>. Note that the allocator is not copied when 
    // the object is assigned.
    // 
    // Deque reallocation occurs when a member function must insert or erase 
    // elements of the controlled sequence. 
    //
    // Erasing an element at either end of the deque invalidates only iterators 
    // and references to the element erased. Erasing an element not at either 
    // end invalidates all iterators and references to elements in the deque.
    //
    // Inserting an element at either end of the deque invalidates all 
    // iterators but no references to elements in the deque. Inserting an 
    // element not at either end invalidates all iterators and references to 
    // elements in the deque.
    //
    ///////////////////////////////////////////////////////////////////////////

    template
    <
        class   T                         , // The type of elements in the deque.
        class   Allocator = allocator<T>    // The type of allocator for allocating and freeing elements of the deque.
    >
    class deque 
    {
      public:

        //---------------------------------------------------------------------
        // Administrative types
        //---------------------------------------------------------------------

        typedef Allocator                            allocator_type      ;
        typedef typename Allocator::size_type        size_type           ;
        typedef typename Allocator::difference_type  difference_type     ;
        typedef typename Allocator::reference        reference           ;
        typedef typename Allocator::const_reference  const_reference     ;
        typedef typename Allocator::value_type       value_type          ;

        //---------------------------------------------------------------------
        // Iterators
        //---------------------------------------------------------------------

        class   iterator                ;
        class   const_iterator          ;
        class   reverse_iterator        ;
        class   const_reverse_iterator  ;

        //---------------------------------------------------------------------
        // Construction and assignment
        //---------------------------------------------------------------------

        explicit deque( const Allocator & a = Allocator() );
        explicit deque
        (
            size_type           n                       ,
            const T &           value   =   T()         , 
            const Allocator &   a       =   Allocator()
        );
        deque( const deque & x );
        deque
        (
            const_iterator      first               , 
            const_iterator      last                ,
            const Allocator &   a       = Allocator()
        );
        void assign( const_iterator first , const_iterator last );
        void assign( size_type n , const T & x = T() );
        ~deque();

        //---------------------------------------------------------------------
        // 
        //---------------------------------------------------------------------

        iterator                begin   ()          ;
        const_iterator          begin   () const    ;
        iterator                end     ()          ;
        const_iterator          end     () const    ;
        reverse_iterator        rbegin  ()          ;
        const_reverse_iterator  rbegin  () const    ;
        reverse_iterator        rend    ()          ;
        const_reverse_iterator  rend    () const    ;

        //---------------------------------------------------------------------
        // Size 
        //---------------------------------------------------------------------

        void        resize      ( size_type n , T  x = T() );
        size_type   size        () const                    ;
        size_type   max_size    () const                    ;
        x_bool      empty       () const                    ;

        //---------------------------------------------------------------------
        // Indexing and element access
        //---------------------------------------------------------------------

        T &       at          ( size_type pos )       ;
        const T & at          ( size_type pos ) const ;
        T &       operator[]  ( size_type pos )       ;
        const T & operator[]  ( size_type pos ) const ;
        T &       front       ()                      ;
        const T & front       () const                ;
        T &       back        ()                      ;
        const T & back        () const                ;
        void      push_front  ( const T & x )         ;
        void      pop_front   ()                      ;
        void      push_back   ( const T & x )         ;
        void      pop_back    ()                      ;

        //---------------------------------------------------------------------
        // Insertion
        //---------------------------------------------------------------------

        iterator    insert  ( iterator it , const T & x = T()   );
        void        insert  ( iterator it , size_type n , const T & x );
        void        insert  ( iterator it , const_iterator first , const_iterator last );

        //---------------------------------------------------------------------
        // Removal
        //---------------------------------------------------------------------

        iterator    erase   ( iterator it )                     ;
        iterator    erase   ( iterator first , iterator last )  ;
        void        clear   ()                                  ;

        //---------------------------------------------------------------------
        // Miscellaneous
        //---------------------------------------------------------------------

        void        swap            ( deque & x)            ;
        Allocator   get_allocator   ()              const   ;

      private:

        friend class iterator ;
        friend class const_iterator ;

        // The deque container will be managed in chunks of contiguous elements of type T.
        enum { CHUNK_SIZE = sizeof(T) > 4096 ? 1 : 4096 / sizeof(T) };  // How many elements in each chunk?
        
        typedef T * * chunk_list_type ; // A list of pointers to chunks. Each chunk has CHUNK_SIZE elements in it (some may be unused).

      public :

        //---------------------------------------------------------------------
        // const_iterator
        //---------------------------------------------------------------------
        class const_iterator
        {
          public:
            const_iterator()                                          {}
            explicit const_iterator( iterator x )                     : m_list(x.m_list) , m_item(x.m_item) {}
            const_iterator & operator ++ ()                           { ++m_item; if( m_item >= end() ) { ++m_list; m_item = begin(); } return *this ; }
            const_iterator & operator -- ()                           ;
            const_iterator & operator += (size_type n)                ;
            const_iterator & operator -= (size_type n)                ;
            const_iterator   operator +  (size_type n) const          ;
            const_iterator   operator -  (size_type n) const          ;
            const_iterator   operator ++ (int)                        ;
            const_iterator   operator -- (int)                        ;
            const T &        operator *  ()      const                { return *m_item ; }
            const T *        operator -> ()      const                { return  m_item ; }
            const T &        operator [] (size_type n) const          ;

            x_bool           operator == (const const_iterator & rhs) const { return m_item == rhs.m_item ; }
            x_bool           operator != (const const_iterator & rhs) const { return m_item != rhs.m_item ; }

          protected :

            friend class deque<T,Allocator> ;

            explicit const_iterator
            (
                chunk_list_type list           , // A list of chunks starting with the one containing item.
                const T *       item             // The item at the iterator's current position.
            )
            {
                m_list  =   list    ;
                m_item  =   item    ;
            }

            chunk_list_type m_list           ;   // A list of chunks starting with the one containing item.
            const T *       m_item           ;   // The item at the iterator's current position.

            T * begin() const { return m_list[0] ; } // The first element in the chunk
            T * end  () const { return &m_list[0][CHUNK_SIZE]; } // One beyond the last element in the chunk.

        };

        //---------------------------------------------------------------------
        // const_iterator
        //---------------------------------------------------------------------
        class iterator 
        {
          public:
            iterator()                                          {}
            iterator & operator ++ ()                           { ++m_item; if( m_item >= end() ) { ++m_list; m_item = begin(); } return *this ; }
            iterator & operator -- ()                           ;
            iterator & operator += (size_type n)                ;
            iterator & operator -= (size_type n)                ;
            iterator   operator +  (size_type n) const          ;
            iterator   operator -  (size_type n) const          ;
            iterator   operator ++ (int)                        ;
            iterator   operator -- (int)                        ;
            T &        operator *  ()      const                { return *m_item ; }
            T *        operator -> ()      const                { return  m_item ; }
            T &        operator [] (size_type n) const          ;

            x_bool     operator == (const iterator & rhs) const { return m_item == rhs.m_item ; }
            x_bool     operator != (const iterator & rhs) const { return m_item != rhs.m_item ; }

          protected :


            friend class deque<T,Allocator> ;
            friend class const_iterator ;

            explicit iterator
            (
                chunk_list_type list           , // A list of chunks starting with the one containing item.
                T *             item             // The item at the iterator's current position.
            )
            {
                m_list  =   list    ;
                m_item  =   item    ;
            }

            chunk_list_type m_list           ;   // A list of chunks starting with the one containing item.
            T *             m_item           ;   // The item at the iterator's current position.
            T * begin() const { return m_list[0] ; } // The first element in the chunk
            T * end  () const { return &m_list[0][CHUNK_SIZE]; } // Ony beyond the last element in the chunk.
        };

      protected:

        Allocator allocator ;

      private:

        chunk_list_type m_chunks        ; // A list of m_chunk_count pointers. 0 if none.
        x_size_t        m_chunk_count   ; // The number of chunks in m_chunks. 0 if none.
        x_size_t        m_size          ; // How many elements are in the deque container?
        iterator        m_begin         ; // An iterator representing the first element.
        iterator        m_end           ; // An iterator representing beyond the last element.

        void initialize( const Allocator & a ); // Called by all constructors.

        
        void grow_front(); // Add another chunk to the front, adjusting m_begin and m_end for deque reallocation.
        T *  grow_back (); // Add another chunk to the back , adjusting m_begin and m_end for deque reallocation. Return the newly available (though uninitialized) element.
        void free_front(); // Free the first chunk. Does not adjust m_begin or m_end if needed - caller must do that.
        void free_back (); // Free the last chunk. Does not adjust m_begin or m_end if needed - caller must do that.
        x_bool front_is_full() const { return m_begin.m_list == 0 || m_begin.begin() == m_begin.m_item ; }
        x_bool back_is_full () const { return m_end.m_list   == 0 || m_end.end()     == m_end.m_item   ; }

    };

    ///////////////////////////////////////////////////////////////////////////
    // 
    // Template functions
    //
    ///////////////////////////////////////////////////////////////////////////

    template<class T, class A> x_bool operator == ( const deque<T,A> & lhs , const deque<T,A> & rhs );
    template<class T, class A> x_bool operator != ( const deque<T,A> & lhs , const deque<T,A> & rhs );
    template<class T, class A> x_bool operator <  ( const deque<T,A> & lhs , const deque<T,A> & rhs );
    template<class T, class A> x_bool operator >  ( const deque<T,A> & lhs , const deque<T,A> & rhs );
    template<class T, class A> x_bool operator <= ( const deque<T,A> & lhs , const deque<T,A> & rhs );
    template<class T, class A> x_bool operator >= ( const deque<T,A> & lhs , const deque<T,A> & rhs );
    template<class T, class A> void   swap        ( const deque<T,A> & lhs , const deque<T,A> & rhs );

    ///////////////////////////////////////////////////////////////////////////
    // 
    // Implementation
    //
    ///////////////////////////////////////////////////////////////////////////

    //-------------------------------------------------------------------------
    template<class T,class A> typename deque<T,A>::size_type      deque<T,A>::size          () const { return m_size ; }
    template<class T,class A> typename deque<T,A>::size_type      deque<T,A>::max_size      () const { return 1000000000 ; }
    template<class T,class A> x_bool                              deque<T,A>::empty         () const { return m_size == 0 ; }
    template<class T,class A> A                                   deque<T,A>::get_allocator () const { return allocator ; }
    template<class T,class A> typename deque<T,A>::iterator       deque<T,A>::begin         ()       { return m_begin ; }
    template<class T,class A> typename deque<T,A>::const_iterator deque<T,A>::begin         () const { return const_iterator(m_begin) ; }
    template<class T,class A> typename deque<T,A>::iterator       deque<T,A>::end           ()       { return m_end   ; }
    template<class T,class A> typename deque<T,A>::const_iterator deque<T,A>::end           () const { return const_iterator(m_end)   ; }

    //-------------------------------------------------------------------------
    template < class T , class A>
    void deque<T,A>::initialize( const A & a )
    {
        allocator       =   a   ;
        m_chunks        =   0   ;
        m_chunk_count   =   0   ;
        m_size          =   0   ;

        m_begin         =   iterator(0,0)   ;
        m_end           =   m_begin         ;
    }

    //-------------------------------------------------------------------------
    template < class T , class A>
    void deque<T,A>::grow_front()
    {
        const x_size_t old_chunk_count     =   m_chunk_count     ;
        const x_size_t new_chunk_count     =   m_chunk_count + 1 ;

        chunk_list_type old_list            =   m_chunks                        ;
        chunk_list_type new_list            =   new  T * [new_chunk_count]      ;
        if( old_chunk_count > 0 ) // Copy old list contents?
        {
            x_memmove( &new_list[chunk_count_to_add] , old_list , old_chunk_count * sizeof(old_list[0]) );
        }

        new_list[0] = allocator.allocate(CHUNK_SIZE);

        delete[] old_list ;
        m_chunk_count   =   new_chunk_count ;
        m_chunks        =   new_list        ;

        // Update the begin/end iterators
        {
            if( empty() )
            {
                X_STD_ASSERT( m_chunk_count == 1 );
                chunk_list_type first_chunk_list        =  &m_chunks[0]                     ;
                T *             beyond_last_in_chunk    =  &first_chunk_list[0][CHUNK_SIZE] ;
                m_begin = iterator( first_chunk_list , beyond_last_in_chunk );
                m_end   = m_begin ;
            }
            else
            {
                m_begin . m_list = &new_list[ m_begin.m_list - old_list ];
                m_end   . m_list = &new_list[ m_end  .m_list - old_list ];
            }
        }
        return m_begin.m_item ;
    }

    //-------------------------------------------------------------------------
    template < class T , class A>
    T * deque<T,A>::grow_back()
    {
        const x_size_t old_chunk_count     =   m_chunk_count         ;
        const x_size_t new_chunk_count     =   m_chunk_count + 1     ;

        chunk_list_type old_list            =   m_chunks                        ;
        chunk_list_type new_list            =   new  T * [new_chunk_count]      ;
        if( old_chunk_count > 0 ) // Copy old list contents?
        {
            x_memmove( &new_list[0] , old_list , old_chunk_count * sizeof(old_list[0]) );
        }

        new_list[ new_chunk_count-1 ] = allocator.allocate(CHUNK_SIZE);

        delete[] old_list ;
        m_chunk_count   =   new_chunk_count ;
        m_chunks        =   new_list        ;


        chunk_list_type last_chunk_list     =   &m_chunks[m_chunk_count-1];
        T * new_element = &last_chunk_list[0][0];
        // Update the begin/end iterators
        {
            if( empty() )
            {
                X_STD_ASSERT( m_chunk_count == 1 );
                m_end   = iterator( last_chunk_list , new_element );
                m_begin = m_end ;
            }
            else
            {
                m_begin. m_list = &new_list[ m_begin . m_list - old_list ];
                m_end  . m_list = &new_list[ m_end   . m_list - old_list ];
            }
        }
        return new_element ;
    }

    //-------------------------------------------------------------------------
    template < class T , class A>
    void deque<T,A>::free_front()
    {
        //todo
    }

    //-------------------------------------------------------------------------
    template < class T , class A>
    void deque<T,A>::free_back()
    {
        //todo:
    }


    //-------------------------------------------------------------------------
    template < class T , class A>
    deque<T,A>::deque( const A & a )
    {
        initialize(a);
    }

    //-------------------------------------------------------------------------
    template < class T , class A>
    deque<T,A>::~deque()
    {
        //todo: use free_back() directly for efficiency
    }

    //-------------------------------------------------------------------------
    template < class T , class A>
    deque<T,A>::deque
    (
        size_type   n       ,
        const T &   value   , 
        const A &   a       
    )
    {
        initialize(a);
        //todo
    }

    //-------------------------------------------------------------------------
    template < class T , class A>
    deque<T,A>::deque( const deque & x )
    {
        initialize( x.get_allocator() );
        const_iterator i = x.begin();
        while( i != x.end() )
        {
            push_back( *i );
            ++ i ;
        }
    }

    //-------------------------------------------------------------------------
    template < class T , class A>
    deque<T,A>::deque
    (
        const_iterator  first               , 
        const_iterator  last                ,
        const A &       a       
    )
    {
        initialize(a);
        //todo
    }

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // void assign( const_iterator first , const_iterator last );

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // void assign( size_type n , const T & x = T() );

    //-------------------------------------------------------------------------
    // reverse_iterator        rbegin  ()          ;
    // const_reverse_iterator  rbegin  () const    ;
    // reverse_iterator        rend    ()          ;
    // const_reverse_iterator  rend    () const    ;
    //
    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // void        resize      ( size_type n , T  x = T() );
    //
    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // T &       at          ( size_type pos )       ;
    // const T & at          ( size_type pos ) const ;

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // T &       operator[]  ( size_type pos )       ;
    // const T & operator[]  ( size_type pos ) const ;

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // T &       front       ()                      ;
    // const T & front       () const                ;

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // T &       back        ()                      ;
    // const T & back        () const                ;

    //-------------------------------------------------------------------------
    template < class T , class A>
    void deque<T,A>::push_front( const T & x )
    {
        if( front_is_full() ) 
        {
            grow_front();
        }
        allocator.construct( &*m_begin , x );
        -- m_begin ;
        ++ m_size  ;
    }


    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // void      pop_front   ()                      ;

    //-------------------------------------------------------------------------
    template < class T , class A>
    void deque<T,A>::push_back( const T & x )
    {
        T * new_element = back_is_full() ? grow_back() : m_end.m_item ;
        allocator.construct( new_element , x );
        ++ m_end  ;
        ++ m_size ;
    }


    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // void      pop_back    ()                      ;

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // iterator    insert  ( iterator it , const T & x = T()   );

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // void        insert  ( iterator it , size_type n , const T & x );

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // void        insert  ( iterator it , const_iterator first , const_iterator last );

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // iterator    erase   ( iterator it )                     ;

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // iterator    erase   ( iterator first , iterator last )  ;

    //-------------------------------------------------------------------------
    // template < class T , class A>
    // deque<T,A>::
    // void        clear   ()                                  ;

    //-------------------------------------------------------------------------
    template < class T , class A>
    void deque<T,A>::swap( deque & x)
    {
        x_std::swap( m_chunks       , x.m_chunks        );        
        x_std::swap( m_chunk_count  , x.m_chunk_count   );   
        x_std::swap( m_size         , x.m_size          );          
        x_std::swap( m_begin        , x.m_begin         );         
        x_std::swap( m_end          , x.m_end           );           
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator == ( const deque<T,A> & lhs , const deque<T,A> & rhs )
    {
        return lhs.size() == rhs.size() && x_std::equal( lhs.begin() , lhs.end() , rhs.begin() );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator != ( const deque<T,A> & lhs , const deque<T,A> & rhs )
    {
        return !( lhs == rhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator <  ( const deque<T,A> & lhs , const deque<T,A> & rhs )
    {
        return lexicographical_compare( lhs.begin() , lhs.end() , rhs.begin() , rhs.end() );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator >  ( const deque<T,A> & lhs , const deque<T,A> & rhs )
    {
        return rhs < lhs ;
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator <= ( const deque<T,A> & lhs , const deque<T,A> & rhs )
    {
        return !( rhs < lhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator >= ( const deque<T,A> & lhs , const deque<T,A> & rhs )
    {
        return !( lhs < rhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> void  swap( const deque<T,A> & lhs , const deque<T,A> & rhs )
    {
        lhs.swap(rhs);
    }
}

#endif

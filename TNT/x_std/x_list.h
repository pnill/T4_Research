#if !defined( X_STD_LIST_H )
#define X_STD_LIST_H

///////////////////////////////////////////////////////////////////////////////
//
// Doubly-linked-list-related declarations based on STL <list>
//
// Defines the template class list<> and various supporting templates.
//
///////////////////////////////////////////////////////////////////////////////
//  
// Overview of basic functions for list<T,Allocator>:
//
//     x_std::list<T> X ;       // Declare a list with elements of type T.
//     X.size()                 // How many elements are in X?
//     X.resize(10);            // Change the size of X to be 10, removing 
//                              // trailing elements or adding elements T() as needed.
//     X.empty()                // Is X empty?
//     X.clear();               // Empty X, removing all elements.
//
//     T & Front = X.front();   // The first element of X. Raises an error if X.empty().
//     X.push_front(Y);         // Add Y to the beginning of the list.
//     X.pop_front();           // Remove the first element from the list.
//    
//     T & Back = X.back();     // The last element of X. Raises an error if X.empty().
//     X.push_back(Y);          // Add Y to the end of the list.
//     X.pop_back();            // Remove the last element from the list.
//    
//     X.insert( it , Y );      // Insert a new element with value Y before iterator it.
//     X.erase(it);             // Erase the element at iterator it.
//     X.remove(Y);             // Remove all elements which compare equal to Y.
//
// Forward iteration:
//
//     x_std::list<T>::const_iterator i ;
//     for( i = X.begin() ; i != X.end() ; ++i )
//     {
//         const T & Element = *i ; 
//     }
//
// Reverse iteration:
//
//     x_std::list<T>::reverse_iterator i ;
//     for( i = X.rbegin() ; i != X.rend() ; ++i )
//     {
//         const T & Element = *i ; 
//     }
//
// Deallocating a list of pointers (where T is a pointer type):
// 
//     while( !X.empty() )
//     {
//         delete X.back();
//         X.pop_back();
//     }
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
// 1. If the compiler does not support member templates well enough to use the
//    rebind mechanism as described in x_memory.h, then operator new
//    is used for list node allocations.
// 2. If the compiler does not support member templates, then the following 
//    member templates are NOT provided:
//
//      template < class Predicate > void merge    ( list & x , Predicate predicate );
//      template < class Predicate > void remove_if( Predicate predicate );
//      template < class Predicate > void unique   ( Predicate predicate );
//      template < class Predicate > void sort     ( Predicate predicate );
//
///////////////////////////////////////////////////////////////////////////////
//
// Performance
//
// The time-complexity of each function is described near its declaration as 
// O(x), where x is some function of the arguments and *this. Examples:
//
//    O(size())  The time depends on the size of the list.
//    O(n)       The time depends on the parameter n.
//    O(it2-it1) The time depends on the number of elements in the iterator range [it1,it2).
//
// If the complexity is omitted, it can be assumed to be O(1).
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_memory.h"
#include    "x_algorithm.h"
#include    "x_functional.h"

namespace x_std 
{

    ///////////////////////////////////////////////////////////////////////////
    //  
    // class list< T , Allocator >
    //
    // A doubly-linked list of elements of type T using Allocator.
    //
    ///////////////////////////////////////////////////////////////////////////

    template
    <
        class   T                           ,   // The type for elements in the list
        class   Allocator   = allocator<T>      // The allocator to use for allocating and deleting elements. 
    >
    class list 
    {

      private : struct link_type ; // The next and previous links for the linked list.
      private : struct node_type ; // The type for each node in the list, containing T and link_type.

      public :
        
        //---------------------------------------------------------------------
        // Administrative types
        //---------------------------------------------------------------------

        typedef Allocator                           allocator_type      ;
        typedef typename Allocator::size_type       size_type           ;
        typedef typename Allocator::difference_type difference_type     ;
        typedef T &                                 reference           ;
        typedef const T &                           const_reference     ;
        typedef T &                                 value_type          ;

        //---------------------------------------------------------------------
        // Iterator types
        //---------------------------------------------------------------------
        // Each iterator type I (iterator, const_iterator, and so on) has these functions:
        // WARNING: Avoid using the inefficient post-increment i++ and post-decrement i--. 
        //
        //    I &       operator ++ ()                // Increment.
        //    I         operator ++ ( int )           // (Post-increment - avoid using this).
        //    I &       operator -- ()                // Decrement.
        //    I         operator -- ( int )           // (Post-decrement - avoid using this).
        //    const T & operator *  ()                // The list element referenced by the iterator.
        //    const T * operator -> ()                // The list element referenced by the iterator.
        //    x_bool    operator == ( const I & rhs ) // Do two iterators represent the same position in the list?
        //    x_bool    operator != ( const I & rhs ) // Do two iterators represent different positions in the list?
        //
        // For the non-const iterators, operator * and -> return non-const values.
        // Remember that for a reverse_iterator or const_reverse_iterator, increment means
        // iterate to the previous item in the list and decrement means iterate to the next
        // item in the list.

        class iterator                  ;
        class const_iterator            ;
        class reverse_iterator          ;
        class const_reverse_iterator    ;

        //---------------------------------------------------------------------
        // Construction and assignment
        //---------------------------------------------------------------------

        explicit list // Construct an empty list.
        ( 
            const Allocator & a = Allocator()  
        ) ;
        explicit list // Construct a list of n elements of value. [O(n)]
        ( 
            size_type           n                       ,    
            const T &           value   = T()           , 
            const Allocator &   a       = Allocator()
        );
        list // Construct a list with a copy of x. [O(x.size())]
        ( 
            const list & x 
        );
        list // Construct a list from [first,last).  [O(last-first)]
        ( 
            const_iterator      first               ,    
            const_iterator      last                , 
            const Allocator &   a   = Allocator()
        );
        void assign // Replace a list with [first,last). [O(size()+last-first)]
        ( 
            const_iterator  first , 
            const_iterator  last 
        );
        void assign // Replace a list with n elements of value.  [O(size()+n)]
        ( 
            size_type   n           , 
            const T &   value = T()  
        );

        list & operator = ( const list & rhs ); // [O(size()+rhs.size())]

        ~list(); // [O(size())]

        //---------------------------------------------------------------------
        // Iteration
        //---------------------------------------------------------------------
        // Remember that begin() and rbegin() reference a valid list item (unless
        // the list is empty) and the end() and rend() do not reference a valid
        // list item.

        iterator                begin   ()          ; // The start of forward iteration (the first item in the list).
        const_iterator          begin   ()  const   ; // The start of forward iteration (the first item in the list).
        iterator                end     ()          ; // The end of forward iteration (beyond the last item in the list).
        const_iterator          end     ()  const   ; // The end of forward iteration (beyond the last item in the list).
        reverse_iterator        rbegin  ()          ; // The start of reverse iteration (last item in the list).
        const_reverse_iterator  rbegin  ()  const   ; // The start of reverse iteration (last item in the list).
        reverse_iterator        rend    ()          ; // The end of reverse iteration (beyond the first item in the list).
        const_reverse_iterator  rend    ()  const   ; // The end of reverse iteration (beyond the first item in the list).

        //---------------------------------------------------------------------
        // Size, resize, empty, max_size
        //---------------------------------------------------------------------

        size_type   size        ()  const                       ; // How many elements are in the list?
        void        resize      ( size_type n , T value = T() ) ; // [O(n)] Change the number of elements to n, filling with value if needed.
        size_type   max_size    ()  const                       ; // The length of the largest list allowed.
        x_bool      empty       ()  const                       ; // Is the list empty?

        //---------------------------------------------------------------------
        // First and last elements
        //---------------------------------------------------------------------
              T & front ()          ; // The first element in the list. Raises an error if empty().
        const T & front () const    ; // The first element in the list. Raises an error if empty().
              T & back  ()          ; // The last element in the list. Raises an error if empty().
        const T & back  () const    ; // The last element in the list. Raises an error if empty().

        //---------------------------------------------------------------------
        // Insertion
        //---------------------------------------------------------------------
        // Note: It is a fundamental property of list<> that insertion or removal
        // anywhere is fast - O(1) when a single value is inserted or removed.

        void        push_front  ( const T & value ) ; // Insert value at the front of the list.
        void        pop_front   ()                  ; // Remove the first element. Raise an error if empty().
        void        push_back   ( const T & value ) ; // Insert value at the end of the list.
        void        pop_back    ()                  ; // Remove the last element. Raise an error if empty().

        // Insert before the element at iterator it either a single value, 
        // n values, or a sequence of values from [first,last).
        iterator    insert      ( iterator it , const T & value = T() )                         ;
        void        insert      ( iterator it , size_type n , const T & value )                 ; // [O(n)]
        void        insert      ( iterator it , const_iterator first , const_iterator last )    ; // [O(last-first)]
        void        insert      ( iterator it , const T * first , const T * last )              ; // [O(last-first)]

        //---------------------------------------------------------------------
        // Erasing
        //---------------------------------------------------------------------
        void        clear       ()                                  ; // [O(size())] Empty the list, deallocating its contents.
        iterator    erase       ( iterator it )                     ; // Erase the element at it.
        iterator    erase       ( iterator first , iterator last )  ; // [O(last-first)] Erase the elements in [first,last).
        void        remove      ( const T & value )                 ; // [O(size())] Remove all elements which are == value.
        void        unique      ()                                  ; // [O(size())] Remove all elements which are equal to their preceding element.
        
        #if X_STD_USES_MEMBER_TEMPLATES

            template < class Predicate > void remove_if( Predicate predicate ); // [O(size())]
            template < class Predicate > void unique   ( Predicate predicate ); // [O(size())]

        #endif


        //---------------------------------------------------------------------
        // Merging and splicing
        //---------------------------------------------------------------------
        // Splicing: 
        //     Splicing the contents from another list involves removing the 
        //     elements from the source list called "list" below and adding 
        //     them before the element "it" in *this, keeping the spliced 
        //     elements in the same order.
        // Merging: 
        //     When merging two lists, it is assumed the lists are already sorted
        //     by the predicate. The resulting list will also be so sorted.

        void    splice  ( iterator it , list & x )                                     ; // [O(x.size())]
        void    splice  ( iterator it , list & x , iterator first )                    ; 
        void    splice  ( iterator it , list & x , iterator first , iterator last )    ; // [O(last-first)]
        void    merge   ( list & x )                                                   ; // [O(max(size(),x.size()))]

        #if X_STD_USES_MEMBER_TEMPLATES

            template < class Predicate > void merge( list & x , Predicate predicate );

        #endif

        //---------------------------------------------------------------------
        // Miscellaneous (swap, sort, reverse, and so on)
        //---------------------------------------------------------------------
        void    reverse ()               ; // [O(size())] Reverse the order of the elements.
        void    swap    ( list & value ) ; // Swap the contents of *this and list.
        void    sort    ()            ; // [O(size()*size())] //todo: Make this more efficient

        #if X_STD_USES_MEMBER_TEMPLATES

            template < class Predicate > void sort( Predicate predicate ); // [O(size()*size())] //todo: Make this more efficient

        #endif

        Allocator get_allocator() const { return allocator ; }

        //---------------------------------------------------------------------
        // const_iterator
        //---------------------------------------------------------------------
        class const_iterator
        {

          public:
 
            const_iterator() { }
            const_iterator( const iterator & i )            { m_node = i.m_node ; }
            const_iterator & operator ++ ()                 { m_node = m_node->m_next; return *this;   }
            const_iterator   operator ++ ( int )            { const_iterator iOld = *this; m_node = m_node->m_next; return iOld;   }
            const_iterator & operator -- ()                 { m_node = m_node->m_previous; return *this;   }
            const_iterator   operator -- ( int )            { const_iterator iOld = *this; m_node = m_node->m_previous; return iOld;   }
            const T &        operator *  ()          const  { return m_node->m_value; }
            const T *        operator -> ()          const  { return m_node ; }

            x_bool           operator == ( const const_iterator & rhs ) const { return this->m_node == rhs.m_node ; }
            x_bool           operator != ( const const_iterator & rhs ) const { return this->m_node != rhs.m_node ; }
 
          protected :

            friend class list<T,Allocator> ;
            const node_type * m_node   ; // The current node, or anchor() if at end.
            const_iterator( const node_type * node ) : m_node(node) {}
        };

        //---------------------------------------------------------------------
        // iterator
        //---------------------------------------------------------------------
        class iterator
        {

          public:
 
            iterator() { }
            iterator & operator ++ ()                 { m_node = m_node->m_next; return *this;   }
            iterator   operator ++ ( int )            { iterator iOld = *this; m_node = m_node->m_next; return iOld;   }
            iterator & operator -- ()                 { m_node = m_node->m_previous; return *this;   }
            iterator   operator -- ( int )            { iterator iOld = *this; m_node = m_node->m_previous; return iOld;   }
            T &        operator *  ()          const  { return m_node->m_value; }
            T *        operator -> ()          const  { return m_node ; }

            x_bool     operator == ( const iterator & rhs ) const { return this->m_node == rhs.m_node ; }
            x_bool     operator != ( const iterator & rhs ) const { return this->m_node != rhs.m_node ; }
 
          protected :

            friend class const_iterator ;
            friend class list<T,Allocator> ;
            node_type * m_node   ; // The current node, or anchor() if at end.
            iterator( node_type * node ) : m_node(node) {}
        };

        //---------------------------------------------------------------------
        // const_reverse_iterator
        //---------------------------------------------------------------------
        class const_reverse_iterator
        {

          public:

            const_reverse_iterator() { }
            const_reverse_iterator(const reverse_iterator & i )     { m_node = i.m_node ; }
            const_reverse_iterator & operator ++ ()                 { m_node = m_node->m_previous; return *this;   }
            const_reverse_iterator   operator ++ ( int )            { const_reverse_iterator iOld = *this; m_node = m_node->m_previous; return iOld;   }
            const_reverse_iterator & operator -- ()                 { m_node = m_node->m_next; return *this;   }
            const_reverse_iterator   operator -- ( int )            { const_reverse_iterator iOld = *this; m_node = m_node->m_next; return iOld;   }
            const T &                operator *  ()                 { return m_node->m_value; }
            const T *                operator -> ()          const  { return m_node ; }

            x_bool                   operator == ( const const_reverse_iterator & rhs ) const { return this->m_node == rhs.m_node ; }
            x_bool                   operator != ( const const_reverse_iterator & rhs ) const { return this->m_node != rhs.m_node ; }

          protected :

            friend class list<T,Allocator> ;
            const node_type *   m_node   ; // The current node, or anchor() if at end of iteration.
            const_reverse_iterator( const node_type * node ) : m_node(node) { }
        };

        //---------------------------------------------------------------------
        // reverse_iterator
        //---------------------------------------------------------------------
        class reverse_iterator
        {

          public:

            reverse_iterator() { }
            reverse_iterator & operator ++ ()                 { m_node = m_node->m_previous; return *this;   }
            reverse_iterator   operator ++ ( int )            { reverse_iterator iOld = *this; m_node = m_node->m_previous; return iOld;   }
            reverse_iterator & operator -- ()                 { m_node = m_node->m_next; return *this;   }
            reverse_iterator   operator -- ( int )            { reverse_iterator iOld = *this; m_node = m_node->m_next; return iOld;   }
            T &                operator *  ()                 { return m_node->m_value; }
            T *                operator -> ()          const  { return m_node ; }

            x_bool             operator == ( const reverse_iterator & rhs ) const { return this->m_node == rhs.m_node ; }
            x_bool             operator != ( const reverse_iterator & rhs ) const { return this->m_node != rhs.m_node ; }

          protected :

            friend class const_reverse_iterator ;
            friend class list<T,Allocator> ;
            node_type *   m_node   ; // The current node, or anchor() if at end of iteration.
            reverse_iterator( node_type * node ) : m_node(node) { }
        };

      protected:

        #if 0 && X_STD_USES_TEMPLATE_REBIND // (The rebind mechanism for lists is not supported yet)

            typedef typename Allocator::rebind<node_type>::other node_allocator_type ;

        #else

            typedef Allocator node_allocator_type ; // The node allocator will use Allocator::x_allocate_char().

        #endif

        node_allocator_type allocator ;

        //---------------------------------------------------------------------
        // Private details 
        //---------------------------------------------------------------------

      private :

        struct link_type
        {
            node_type * m_next      ; // The next node in the list.
            node_type * m_previous  ; // The last node in the list.
            link_type() : m_next(0) , m_previous(0) { }
        };

        struct node_type : public link_type
        {
            T m_value ; // The node value.
            node_type( const T & value = T() ) : m_value(value) {}
        };

        // The nodes are all linked together into a circular doubly-linked
        // list starting and ending at m_anchor. 
        link_type   m_anchor    ; // The anchor list element, marking the beginning and end.
        size_type   m_size      ; // The size of the list.

        void insert_before( node_type * BeforeThis , node_type * NewNode ) ; // Insert a new node before BeforeThis, which can be the fake node m_anchor.
        void insert_after( node_type * AfterThis , node_type * NewNode ); // Insert a new node after AfterThis, which can be the fake node m_anchor.
        void Remove( node_type * Node ) ; // Remove a node, which cannot be the fake node m_anchor.

        inline const node_type * anchor() const { return reinterpret_cast<const node_type *>(&m_anchor) ; }
        inline       node_type * anchor()       { return reinterpret_cast<      node_type *>(&m_anchor) ; }

        inline void Construct()
        {
            m_anchor.m_next        = anchor() ;
            m_anchor.m_previous    = anchor() ;
            m_size  = 0 ;
        }

        inline node_type * allocate_node( const T & value )
        {
            node_type * new_node ;
            #if 0 && X_STD_USES_TEMPLATE_REBIND // (The rebind mechanism for lists is not supported yet)
            {
                new_node = allocator.allocate(1,0);
            }
            #else
            {
                new_node = (node_type *)allocator.x_allocate_char( sizeof(*new_node) );
            }
            #endif
            allocator.construct( &new_node->m_value , value );
            return new_node ;
        }
        inline void deallocate_node( node_type * node )
        {
            allocator.destroy( &node->m_value );

            #if 0 && X_STD_USES_TEMPLATE_REBIND // (The rebind mechanism for lists is not supported yet)
            {
                allocator.deallocate(node,1);
            }
            #else
            {
                allocator.x_deallocate_char( node , sizeof(*node) );
            }
            #endif

        }


    };

    ///////////////////////////////////////////////////////////////////////////////
    //  
    // Template functions and operators for list<T,Allocator>
    //
    ///////////////////////////////////////////////////////////////////////////////
    // All of these have time complexity O( min(lhs.size(),rhs.size()) )
    // except swap(), which has O(1).

    template<class T, class A> x_bool operator == ( const list<T,A> & lhs , const list<T,A> & rhs );
    template<class T, class A> x_bool operator != ( const list<T,A> & lhs , const list<T,A> & rhs );
    template<class T, class A> x_bool operator <  ( const list<T,A> & lhs , const list<T,A> & rhs );
    template<class T, class A> x_bool operator >  ( const list<T,A> & lhs , const list<T,A> & rhs );
    template<class T, class A> x_bool operator <= ( const list<T,A> & lhs , const list<T,A> & rhs );
    template<class T, class A> x_bool operator >= ( const list<T,A> & lhs , const list<T,A> & rhs );
    template<class T, class A> void  swap        ( const list<T,A> & lhs , const list<T,A> & rhs );

    ///////////////////////////////////////////////////////////////////////////
    //  
    // Implementation details 
    //
    ///////////////////////////////////////////////////////////////////////////

    //------------------------------------------------------------------------
    template<class T, class A> typename list<T,A>::iterator                list<T,A>::begin   ()          { return               iterator( m_anchor.m_next ); }
    template<class T, class A> typename list<T,A>::const_iterator          list<T,A>::begin   ()  const   { return         const_iterator( m_anchor.m_next ); }
    template<class T, class A> typename list<T,A>::iterator                list<T,A>::end     ()          { return               iterator( anchor() ); }
    template<class T, class A> typename list<T,A>::const_iterator          list<T,A>::end     ()  const   { return         const_iterator( anchor() ); }
    template<class T, class A> typename list<T,A>::reverse_iterator        list<T,A>::rbegin  ()          { return       reverse_iterator( m_anchor.m_previous   ); }
    template<class T, class A> typename list<T,A>::const_reverse_iterator  list<T,A>::rbegin  ()  const   { return const_reverse_iterator( m_anchor.m_previous   ); }
    template<class T, class A> typename list<T,A>::reverse_iterator        list<T,A>::rend    ()          { return       reverse_iterator( anchor() ); }
    template<class T, class A> typename list<T,A>::const_reverse_iterator  list<T,A>::rend    ()  const   { return const_reverse_iterator( anchor() ); }

    template<class T, class A> typename list<T,A>::size_type list<T,A>::size        ()  const               { return m_size      ;  }
    template<class T, class A> typename list<T,A>::size_type list<T,A>::max_size    ()  const               { return 100000000   ;  }
    template<class T, class A> x_bool                        list<T,A>::empty       ()  const               { return m_size == 0 ;  }

    template<class T, class A>       T & list<T,A>::front ()          { X_STD_ASSERT(m_size!=0); return m_anchor.m_next     -> m_value ; }
    template<class T, class A> const T & list<T,A>::front () const    { X_STD_ASSERT(m_size!=0); return m_anchor.m_next     -> m_value ; }
    template<class T, class A>       T & list<T,A>::back  ()          { X_STD_ASSERT(m_size!=0); return m_anchor.m_previous -> m_value ; }
    template<class T, class A> const T & list<T,A>::back  () const    { X_STD_ASSERT(m_size!=0); return m_anchor.m_previous -> m_value ; }

    //-------------------------------------------------------------------------
    template< class T , class A >
    inline void list<T,A>::insert_before( node_type * BeforeThis , node_type * NewNode )
    {
        node_type * Previous    = BeforeThis->m_previous ;
        Previous->m_next        = NewNode    ;
        NewNode->m_previous     = Previous   ;
        BeforeThis->m_previous  = NewNode    ;
        NewNode->m_next         = BeforeThis ;

        m_size++;
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    inline void list<T,A>::insert_after( node_type * AfterThis , node_type * NewNode )
    {
        node_type * Next        = AfterThis->m_next ;
        AfterThis->m_next       = NewNode    ;
        NewNode->m_previous     = AfterThis  ;
        Next->m_previous        = NewNode    ;
        NewNode->m_next         = Next       ;

        m_size++;
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    inline void list<T,A>::Remove( node_type * Node )
    {
        X_STD_ASSERT( Node != 0         );
        X_STD_ASSERT( Node != &m_anchor );
        node_type * Next        = Node->m_next       ;
        node_type * Previous    = Node->m_previous   ;
        Next->m_previous = Previous ;
        Previous->m_next = Next     ;

        m_size--;
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    list<T,A>::~list()
    {
        clear();
    }
            
    //---------------------------------------------------------------------
    template< class T , class A >
    list<T,A>::list( const A & a ) : allocator(a)
    {
        Construct();
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    list<T,A>::list ( size_type n , const T & value , const A & a )  : allocator(a)
    {
        Construct();
        for( ; n > 0 ; n-- )
        {
            push_front(value);
        }    
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    list<T,A>::list( const list & x )
    {
        Construct();
        allocator = x.get_allocator() ;
        insert( begin() , x.begin() , x.end() );    
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    list<T,A>::list( const_iterator first , const_iterator last , const A & a )
    {
        Construct();
        allocator = a ;
        insert( begin() , first , last );
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::assign ( const_iterator first , const_iterator last )
    {
        clear();
        for( ; first != last ; first++ )
        {
            push_back( *first );
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::assign( size_type n , const T & value )
    {
        clear();
        for( ; n > 0 ; n-- )
        {
            push_back(value);
        }
    }

    //-------------------------------------------------------------------------
    template< class T , class A >
    list<T,A> & list<T,A>::operator = ( const list & rhs ) 
    { 
        if( this != &rhs ) // Do nothing if assigning to self.
        {
            assign( rhs.begin() , rhs.end() ); 
        }
        return *this ;
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::push_front( const T & value )
    {
        node_type * Node = allocate_node(value);
        insert_after( anchor() , Node );
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::pop_front()
    {
        node_type * Node = m_anchor.m_next;
        X_STD_ASSERT( Node != anchor() );
        if( Node != anchor() )
        {
            Remove(Node);
            deallocate_node(Node);
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::push_back ( const T & value )
    {
        node_type * Node = allocate_node(value);
        insert_before( anchor() , Node );
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::pop_back()
    {
        node_type * Node = m_anchor.m_previous ;
        X_STD_ASSERT( Node != anchor() );
        if( Node != anchor() )
        {
            Remove(Node);
            deallocate_node(Node);
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    typename list<T,A>::iterator list<T,A>::insert( iterator it , const T & value)
    {
        node_type * NewNode = allocate_node(value);
        insert_before( it.m_node , NewNode );
        it.m_node = NewNode ;
        return it;
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::insert( iterator it , size_type n , const T & value )
    {
        for( ; n > 0 ; n-- )
        {
            insert( it , value );
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::insert( iterator it , const_iterator first , const_iterator last )
    {
        for( ; first != last ; first++ )
        {
            insert( it , *first );
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::insert( iterator it , const T * first , const T * last )
    {
        for( ; first < last ; first++ )
        {
            insert( it , *first );
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::clear()
    {
        while( !empty() )
        {
            pop_front();
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::resize( size_type n , T value )
    {
        x_int Change = static_cast<x_int>(n - size()) ; // Number of elements to add, or (if <0) negative of number of elements to delete
        if( n <= 0 )
        {
            clear();
        }
        else if( Change > 0 ) // Increasing the list?
        {
            insert( end() , Change , value );
        }
        else 
        {
            for( ; Change < 0 ; Change++ ) // While these is more to remove
            {
                pop_back();
            }
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    typename list<T,A>::iterator list<T,A>::erase( iterator it )
    {
        node_type * Node = it.m_node ;
        if( Node != anchor() )
        {
            it.m_node = Node->m_next ;
            Remove( Node );
            deallocate_node(Node);
        }
        return it;
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    typename list<T,A>::iterator list<T,A>::erase( iterator first , iterator last )
    {
        node_type * LastNode    =   last.m_node     ;   // Last node, which is not erased.
        node_type * Node        =   first.m_node    ;
        while( Node != anchor() && Node != LastNode )
        {
            node_type * NextNode = Node->m_next ;
            Remove(Node);
            deallocate_node(Node);
            Node = NextNode ;
        }
        return iterator( Node );
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::remove( const T & value )
    {
        for( iterator i = begin() ; i != end() ; )
        {
            if( *i == value )
            {
                i = erase(i);
            }
            else
            {
                ++ i ;
            }
        }
    }

    //---------------------------------------------------------------------
    #if X_STD_USES_MEMBER_TEMPLATES
        template< class T , class A >
        template < class Predicate > void list<T,A>::remove_if( Predicate predicate )
        {
            for( iterator i = begin() ; i != end() ; )
            {
                if( predicate( *i ) )
                {
                    i = erase(i);
                }
                else
                {
                    ++ i ;
                }
            }
        }
    #endif

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::unique()
    {
        if( m_size > 1 )
        {
            iterator i      = begin()   ;
            iterator iNext  = i         ;
            ++ iNext ;
            for( ; iNext != end() ; )
            {
                if( *i == *iNext )
                {
                    iNext = erase( iNext );
                }
                else
                {
                    i = iNext ;
                    ++ iNext ;
                }
            }
        }
    }


    //---------------------------------------------------------------------
    #if X_STD_USES_MEMBER_TEMPLATES
        template< class T , class A >
        template < class Predicate > void list<T,A>::unique( Predicate predicate )
        {
            if( m_size > 1 )
            {
                iterator i      = begin()   ;
                iterator iNext  = i         ;
                ++ iNext ;
                for( ; iNext != end() ; )
                {
                    if( predicate( *i , *iNext ) )
                    {
                        iNext = erase( iNext );
                    }
                    else
                    {
                        i = iNext ;
                    }
                }
            }
        }
    #endif


    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::splice( iterator it , list & x )
    {
        //todo: This can be made so much faster
        X_STD_ASSERT( this != &x );
        node_type * BeforeThis = it.m_node ;
        while( !x.empty() )
        {
            node_type * Node = x.m_anchor.m_next ;
            x.Remove(Node);
            insert_before( BeforeThis , Node );
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::splice( iterator it , list & x , iterator first ) 
    {
        node_type * Node = first.m_node ;
        if( Node != x.anchor() )
        {
            x.Remove(Node);
            insert_before( it.m_node , Node );
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::splice( iterator it , list & x , iterator first , iterator last )
    {
        //todo: This can be made so much faster
        iterator next = first ; // Next item to splice in.
        for( ; next != last ; first++ )
        {
            iterator remove = next ; // Splice in this node.
            next++; // Increment iterator before the splice/
            splice( it , x , remove );
        }    
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::merge( list & x )
    {
        if( this->empty() )
        {
            splice( begin() , x );
        }
        else
        {
            node_type * MergeBefore = m_anchor.m_next   ;
            while( !x.empty() )
            {
                node_type * MergeThis = x.m_anchor.m_next;
                x.Remove(MergeThis);
            
                // Advance MergeBefore until it reaches end of *this or a node which is not < MergeThis.
                while( MergeBefore != anchor() && (MergeBefore->m_value < MergeThis->m_value) )
                {
                    MergeBefore = MergeBefore->m_next ;
                }
                insert_before( MergeBefore , MergeThis );
            }
        }
    }

    //---------------------------------------------------------------------
    #if X_STD_USES_MEMBER_TEMPLATES
        template< class T , class A >
        template < class Predicate > void list<T,A>::merge( list & x , Predicate predicate )
        {
            if( this->empty() )
            {
                splice( begin() , x );
            }
            else
            {
                node_type * MergeBefore = m_anchor.m_next   ;
                while( !x.empty() )
                {
                    node_type * MergeThis = x.m_anchor.m_next ;
                    x.Remove(MergeThis);
            
                    // Advance MergeBefore until it reaches end of *this or until the predicate fails.
                    while( MergeBefore != anchor() && predicate(MergeBefore->m_value,MergeThis->m_value) )
                    {
                        MergeBefore = MergeBefore->m_next ;
                    }
                    insert_before( MergeBefore , MergeThis );
                }
            }
        }
   #endif

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::reverse()
    {
        node_type * insert_before = m_anchor.m_next ; // Move the nodes from the end to before insert_before.
        while( insert_before != m_anchor.m_previous )
        {
            node_type * Back = this->m_anchor.m_previous ;
            Remove( Back );
            this->insert_before( insert_before , Back );
        }
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::swap( list & x )
    {
        x_std::swap( this->m_anchor , x.m_anchor );
        x_std::swap( this->m_size   , x.m_size   );

        // We need to relink the first and last elements (if any),
        // since they point to the wrong anchor nodes (the anchor
        // node is saved directly in *this instead of as a true node).
        this->m_anchor.m_next       -> m_previous   =   this->anchor()  ;
        this->m_anchor.m_previous   -> m_next       =   this->anchor()  ;
        x.m_anchor.m_next           -> m_previous   =   x.anchor()      ;
        x.m_anchor.m_previous       -> m_next       =   x.anchor()      ;
    }

    //---------------------------------------------------------------------
    template< class T , class A >
    void list<T,A>::sort()
    {
        node_type * pNext = m_anchor.m_next ; // Next node to insert

        // Remove all the nodes from *this:
        m_anchor.m_next     =   anchor()    ;
        m_anchor.m_previous =   anchor()    ;
        m_size = 0;

        // Add the nodes back, one at a time, putting each one in the right order.
        while( pNext != anchor() )
        {
            node_type * pInsert = pNext ; // Insert this node.
            pNext = pNext->m_next ; // Remember next pointer now, since inserting it will change its m_next field.

            // Insert pInsert into *this at the right place:
            node_type * pCheck = m_anchor.m_next ;
            while( pCheck != anchor() && pCheck->m_value < pInsert->m_value )
            {
                pCheck = pCheck->m_next ;
            }
            insert_before( pCheck , pInsert );
        }
    }

    //---------------------------------------------------------------------
    #if X_STD_USES_MEMBER_TEMPLATES
        template< class T , class A >
        template < class Predicate > void list<T,A>::sort( Predicate predicate )
        {
            node_type * pNext = m_anchor.m_next ; // Next node to insert

            // Remove all the nodes from *this:
            m_anchor.m_next         =   anchor()    ;
            m_anchor.m_previous     =   anchor()    ;
            m_size                  =   0           ;

            // Add the nodes back, one at a time, putting each one in the right order.
            while( pNext != anchor() )
            {
                node_type * pInsert = pNext ; // Insert this node.
                pNext = pNext->m_next ; // Remember next pointer now, since inserting it will change its m_next field.

                // Insert pInsert into *this at the right place:
                node_type * pCheck = m_anchor.m_next ;
                while( pCheck != anchor() && predicate( pCheck->m_value , pInsert->m_value ) )
                {
                    pCheck = pCheck->m_next ;
                }
                insert_before( pCheck , pInsert );
            }
        }
    #endif

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator == ( const list<T,A> & lhs , const list<T,A> & rhs )
    {
        return lhs.size() == rhs.size() && x_std::equal( lhs.begin() , lhs.end() , rhs.begin() );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator != ( const list<T,A> & lhs , const list<T,A> & rhs )
    {
        return ! ( lhs == rhs ) ; 
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator <  ( const list<T,A> & lhs , const list<T,A> & rhs )
    {
        return lexicographical_compare( lhs.begin() , lhs.end() , rhs.begin() , rhs.end() );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator >  ( const list<T,A> & lhs , const list<T,A> & rhs )
    {
        return rhs < lhs ;
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator <= ( const list<T,A> & lhs , const list<T,A> & rhs )
    {
        return ! ( rhs < lhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> x_bool operator >= ( const list<T,A> & lhs , const list<T,A> & rhs )
    {
        return ! ( lhs < rhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class A> void  swap        ( const list<T, A>& lhs , const list<T,A> & rhs )
    {
        lhs.swap(rhs);
    }

}

#endif
    

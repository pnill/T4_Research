#if !defined( X_STD_QUEUE_H )
#define X_STD_QUEUE_H

///////////////////////////////////////////////////////////////////////////////
//
// Variable-length queues - declarations based on STL <queue>
// Defines the following template classes
//
//      queue          < T , Container > 
//      priority_queue < T , Container , Predicate >
//
// and some supporting template functions.
//
///////////////////////////////////////////////////////////////////////////////
//
// Overview of basic functions for queue<T,Container>
//  
//    queue<T> X ;      // Declare X as a queue using the default container.
//    X.empty()         // Is the queue empty?
//    X.size()          // The number of elements in the queue.
//    X.top()           // The first element of the sequence. 
//    X.push(x);        // Add an element to the end of the sequence.
//    X.pop();          // Remove an element from the front of the sequence.
//
// Declare a queue using vector<T> instead of the default container:
//
//    queue< T , x_std::vector<T> > X;
//
// Overview of basic functions for priority_queue<T,Container,Predicate>
//  
//    priority_queue<T> X ;      // Declare X as a priority_queue using the default container and predicate.
//    X.empty()         // Is the priority queue empty?
//    X.size()          // The number of elements in the priority queue.
//    X.top()           // The first (highest priority) element of the priority queue. 
//    X.push(x);        // Add an element to the end of the queue, then reorder by priority.
//    X.pop();          // Remove the first (highest priority) element from the queue.
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_deque.h"
#include    "x_vector.h"
#include    "x_functional.h"

namespace x_std
{

    ///////////////////////////////////////////////////////////////////////////
    // 
    // queue< T , Container >   [A first-in, first-out queue.]
    //
    // This template class controls a varying-length sequence of elements that 
    // allocates and frees storage for the sequence through a protected object 
    // named c, of class Container. The type T of elements in the controlled 
    // sequence must match Container::value_type.
    //
    // An object of class Container must supply several public members defined 
    // the same as for deque<> and list<> (both of which are suitable 
    // candidates for class Container). The required members are:
    //
    //     typedef T            value_type      ;
    //     typedef ...          size_type       ; 
    //     typedef ...          allocator_type  ;
    //     Container( const allocator_type & a );
    //     x_bool               empty           () const;
    //     size_type            size            () const;
    //     allocator_type       get_allocator   () const;
    //     value_type &         front           ();
    //     const value_type &   front           () const;
    //     value_type &         back            ();
    //     const value_type &   back            () const;
    //     void push_back       (const value_type & x);
    //     void pop_front       ();
    //
    ///////////////////////////////////////////////////////////////////////////

    template
    <   
        class T                         ,   // The type of elements in the queue.
        class Container = deque<T>          // The Container used to manage the queue.
    >
    class queue 
    {
      public:

        //---------------------------------------------------------------------
        // Administrative types
        //---------------------------------------------------------------------

        typedef typename Container::allocator_type   allocator_type  ;
        typedef typename T                           value_type      ;
        typedef typename Container::size_type        size_type       ;

        //---------------------------------------------------------------------
        // Construction
        //---------------------------------------------------------------------

        explicit queue( const allocator_type & a = allocator_type() );

        //---------------------------------------------------------------------
        // Size
        //---------------------------------------------------------------------

        x_bool              empty   () const    ; // Is the queue empty?
        size_type           size    () const    ; // How many elements are in the queue?

        //---------------------------------------------------------------------
        // Adding and removing elements
        //---------------------------------------------------------------------

        value_type &        top     ()          ; // What is the first element in the queue?
        const value_type &  top     () const    ; // What is the first element in the queue?
        void                push    ( const value_type& x ); // Add element x to the end of the queue.
        void                pop     ()          ; // Remove the first element from the queue.

        //---------------------------------------------------------------------
        // Miscellaneous
       //----------------------------------------------------------------------

        allocator_type      get_allocator() const;

    protected:

        Container c ;
    };


    ///////////////////////////////////////////////////////////////////////////
    // 
    // priority_queue< T , Container , Predicate >   
    //
    // This template class controls a varying-length sequence of elements of 
    // type T with allocation and freeing of elements managed through a 
    // protected object named c, of class Container. The type T of elements in 
    // the controlled sequence must match Container::value_type.
    //
    // The sequence is ordered using a protected bject named comp of type 
    // Predicate. After each insertion or removal of the top element (at 
    // position zero), for the iterators P0 and Pi designating elements at 
    // positions 0 and i, comp(*P0, *Pi) is false. (For the default template 
    // parameter less<Container::value_type>, the top element of the sequence 
    // compares largest, or highest priority.)
    //
    // An object of class Container must supply random-access iterators and 
    // several public members defined the same as for deque<> and vector<> 
    // (both of which are suitable candidates for class Container). The 
    // required members are:
    //
    //     typedef T            value_type      ;
    //     typedef ...          size_type       ; 
    //     typedef ...          allocator_type  ;
    //     Container( const allocator_type & a );
    //     Container( Init first , InIt last , const allocator_type & a );
    //     x_bool               empty           () const;
    //     size_type            size            () const;
    //     allocator_type       get_allocator   () const;
    //     value_type &         front           ();
    //     const value_type &   front           () const;
    //     value_type &         back            ();
    //     const value_type &   back            () const;
    //     void push_back       (const value_type & x);
    //     void pop_back        ();
    //
    ///////////////////////////////////////////////////////////////////////////

    template
    <
        class T                                         ,
        class Container = vector<T>                     ,
        class Predicate = less< typename Container::value_type > 
    >
    class priority_queue 
    {
      public:

        //---------------------------------------------------------------------
        // Administrative types
        //---------------------------------------------------------------------

        typedef typename Container::allocator_type       allocator_type  ;
        typedef typename Container::value_type           value_type      ;
        typedef typename Container::size_type            size_type       ;

        //---------------------------------------------------------------------
        // Construction
        //---------------------------------------------------------------------

        explicit priority_queue
        ( 
            const Predicate&        predicate   = Predicate()       ,
            const allocator_type &  a           = allocator_type()
        );

        priority_queue
        (
            const value_type *      first                       , 
            const value_type *      last                        ,
            const Predicate &       predicate   = Predicate()   , 
            const allocator_type &  a           = allocator_type()
        );

        #if X_STD_USES_MEMBER_TEMPLATES

            template< class InputIterator>
            priority_queue
            (
                InputIterator           first                   ,    
                InputIterator           last                    ,
                const Predicate &       predicate = Predicate() , 
                const allocator_type &  a    = allocator_type()
            );

        #endif

        //---------------------------------------------------------------------
        // Size
        //---------------------------------------------------------------------

        x_bool      empty   ()  const   ; // Is the queue empty?
        size_type   size    ()  const   ; // The number of elements in the queue.

        //---------------------------------------------------------------------
        // Adding and removing elements
        //---------------------------------------------------------------------

        value_type &        top     ()                      ; // The first (highest priority) element in the queue.
        const value_type &  top     () const                ; // The first (highest priority) element in the queue.
        void                push    ( const value_type& x)  ; // Add x to the end of the queue, then reorder by priority.
        void                pop     ()                      ; // Removes the first (highest priority) element in the queue.

        //---------------------------------------------------------------------
        // Miscellaneous
        //---------------------------------------------------------------------

        allocator_type get_allocator() const;

      protected:

        Container   c       ;
        Predicate   comp    ;

    };

    ///////////////////////////////////////////////////////////////////////////
    // 
    // Template functions
    //
    ///////////////////////////////////////////////////////////////////////////

    template<class T, class C> x_bool operator == ( const queue<T,C> & lhs , const queue<T,C> & rhs );
    template<class T, class C> x_bool operator != ( const queue<T,C> & lhs , const queue<T,C> & rhs );
    template<class T, class C> x_bool operator <  ( const queue<T,C> & lhs , const queue<T,C> & rhs );
    template<class T, class C> x_bool operator >  ( const queue<T,C> & lhs , const queue<T,C> & rhs );
    template<class T, class C> x_bool operator <= ( const queue<T,C> & lhs , const queue<T,C> & rhs );
    template<class T, class C> x_bool operator >= ( const queue<T,C> & lhs , const queue<T,C> & rhs );

    ///////////////////////////////////////////////////////////////////////////
    // 
    // Implementation
    //
    ///////////////////////////////////////////////////////////////////////////

    //-------------------------------------------------------------------------
    template<class T, class C> x_bool operator == ( const queue<T,C> & lhs , const queue<T,C> & rhs )
    {
        return lhs.c == rhs.c ;
    }

    //-------------------------------------------------------------------------
    template<class T, class C> x_bool operator != ( const queue<T,C> & lhs , const queue<T,C> & rhs )
    {
        return !( lhs == rhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class C> x_bool operator <  ( const queue<T,C> & lhs , const queue<T,C> & rhs )
    {
        return lhs.c < rhs.c ;
    }

    //-------------------------------------------------------------------------
    template<class T, class C> x_bool operator >  ( const queue<T,C> & lhs , const queue<T,C> & rhs )
    {
        return rhs < lhs ;
    }

    //-------------------------------------------------------------------------
    template<class T, class C> x_bool operator <= ( const queue<T,C> & lhs , const queue<T,C> & rhs )
    {
        return !( rhs < lhs );
    }

    //-------------------------------------------------------------------------
    template<class T, class C> x_bool operator >= ( const queue<T,C> & lhs , const queue<T,C> & rhs )
    {
        return !( lhs < rhs );
    }


    //---------------------------------------------------------------------
    // Function definitions for queue<>
    //---------------------------------------------------------------------

    //-------------------------------------------------------------------------
    template< class T , class Container >
    queue<T,Container>::queue( const allocator_type & a )
        :   c(a)
    {
    }

    //-------------------------------------------------------------------------
    template< class T , class Container >
    x_bool queue<T,Container>::empty() const
    {
        return c.empty();
    }

    //-------------------------------------------------------------------------
    template< class T , class Container >
    queue<T,Container>::size_type queue<T,Container>::size() const
    {
        return c.size();
    }

    //-------------------------------------------------------------------------
    template< class T , class Container >
    queue<T,Container>::value_type & queue<T,Container>::top()
    {
        return c.front();
    }

    //-------------------------------------------------------------------------
    template< class T , class Container >
    const queue<T,Container>::value_type & queue<T,Container>::top() const
    {
        return c.front();
    }

    //-------------------------------------------------------------------------
    template< class T , class Container >
    void queue<T,Container>::push( const value_type & x )
    {
        c.push_back(x);
    }

    //-------------------------------------------------------------------------
    template< class T , class Container >
    void queue<T,Container>::pop()
    {
        c.pop_front();
    }

    //-------------------------------------------------------------------------
    template< class T , class Container >
    queue<T,Container>::allocator_type queue<T,Container>::get_allocator() const
    {
        return c.get_allocator();
    }

    //---------------------------------------------------------------------
    // Function definitions for priority_queue<>
    //---------------------------------------------------------------------
    //todo

}


#endif



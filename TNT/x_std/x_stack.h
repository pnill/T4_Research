#if !defined( X_STD_STACK_H )
#define X_STD_STACK_H

///////////////////////////////////////////////////////////////////////////////
//
// Stack-related declarations based on STL <stack>
//
// Defines the template class stack<> and various supporting templates.
//
///////////////////////////////////////////////////////////////////////////////
//  
// Overview of basic functions for stack<T,C>:
//
//     x_std::stack<T> X ;      // Declare a stack of elements of type T using the default container.
//     X.size()                 // How many elements are in X?
//     X.empty()                // Is X empty?
//     T & top = X.top();       // The top element on the stack (the one last pushed).
//     X.push(Y);               // Push Y onto the stack, making it the top element.
//     X.pop();                 // Remove the top element from the stack.
//
// To use vector<> as a container instead of the default:
//
//     x_std::stack< T , x_std::vector<T> > X ; 
//
// Deallocating a stack of pointers (where T is a pointer type):
// 
//     while( !X.empty() )
//     {
//         delete X.top();
//         X.pop();
//     }
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
// 1. The default container class for stack is list<T>, instead of the standard
//    deque<T>. This will probably change once x_std implements deque<>.
// 2. Non-standard function stack<T,C>::x_clear is provided.
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_list.h"

namespace x_std
{

    ///////////////////////////////////////////////////////////////////////////
    //  
    // class stack< T , C >
    //
    // A varying-length stack of elements of type T using controlling container
    // type C. The stack allocates and frees storage for the elements through a 
    // protected container object called c, of class C. The type T of 
    // elements in that container must match value_type.
    //
    // Class C must supply several public members defined the same as for deque, 
    // list, and vector (all of which are suitable candidates for class C). 
    // The required members are:
    //
    // typedef    ...     value_type;
    // typedef    ...     size_type;
    // x_bool             empty () const;
    // size_type          size () const;
    // allocator_type     get_allocator() const;
    // value_type &       back();
    // const value_type & back() const;
    // void               push_back(const value_type& x);
    // void               pop_back();
    // C( const allocator_type & a );
    //
    ///////////////////////////////////////////////////////////////////////////

    template
    <
        class   T               ,   // The type for elements in the stack
        class   C   = list<T>       // The container class which implements the stack.
    >
    class stack 
    {
      public:

        typedef typename C::allocator_type   allocator_type  ;
        typedef typename C::value_type       value_type      ;
        typedef typename C::size_type        size_type       ;

        explicit stack( const allocator_type & a = allocator_type() );
        x_bool      empty   ()              const   ; // Is the stack empty?
        size_type   size    ()              const   ; // How many elements are in the stack?
        T &         top     ()                      ; // What is the top element in the stack?
        const T &   top     ()              const   ; // What is the top element in the stack?
        void        push    (const T & x)           ; // Push on element onto the top of the stack.
        void        pop     ()                      ; // Remove the top element from the stack.

        allocator_type get_allocator() const;

        //-------------------------------------------------------------------------
        // Non-standard extensions
        //-------------------------------------------------------------------------
        void x_clear();


      protected:

        C c ;

    };

    ///////////////////////////////////////////////////////////////////////////////
    //  
    // Template functions and operators for stack<T,C>
    //
    ///////////////////////////////////////////////////////////////////////////////

    template< class T, class C > x_bool operator == ( const stack<T,C> & lhs, const stack<T,C> & rhs );
    template< class T, class C > x_bool operator != ( const stack<T,C> & lhs, const stack<T,C> & rhs );
    template< class T, class C > x_bool operator <  ( const stack<T,C> & lhs, const stack<T,C> & rhs );
    template< class T, class C > x_bool operator >  ( const stack<T,C> & lhs, const stack<T,C> & rhs );
    template< class T, class C > x_bool operator <= ( const stack<T,C> & lhs, const stack<T,C> & rhs );
    template< class T, class C > x_bool operator >= ( const stack<T,C> & lhs, const stack<T,C> & rhs );


    ///////////////////////////////////////////////////////////////////////////
    //  
    // Implementation details 
    //
    ///////////////////////////////////////////////////////////////////////////

    //-------------------------------------------------------------------------
    template< class T, class C > x_bool                         stack<T,C>::empty   ()             const   { return c.empty(); } 
    template< class T, class C > typename stack<T,C>::size_type stack<T,C>::size    ()             const   { return c.size();  } 
    template< class T, class C > T &                            stack<T,C>::top     ()                     { return c.back();  } 
    template< class T, class C > const T &                      stack<T,C>::top     ()             const   { return c.back();  } 
    template< class T, class C > void                           stack<T,C>::push    (const T & x)          { c.push_back(x);   }
    template< class T, class C > void                           stack<T,C>::pop     ()                     { c.pop_back();     } 

    //-------------------------------------------------------------------------
    template< class T, class C >
    stack<T,C>::stack( const allocator_type & a )
        :   c(a)
    {
    }

    //-------------------------------------------------------------------------
    template< class T, class C >
    typename stack<T,C>::allocator_type stack<T,C>::get_allocator() const
    {
        return c.get_allocator();
    }

    //-------------------------------------------------------------------------
    template< class T, class C >
    void stack<T,C>::x_clear()
    {
        while( !empty() ) { pop(); }
    }

    //-------------------------------------------------------------------------
    template< class T, class C > x_bool operator == ( const stack<T,C> & lhs, const stack<T,C> & rhs )
    {
        return lhs.c == rhs.c ;
    }

    //-------------------------------------------------------------------------
    template< class T, class C > x_bool operator != ( const stack<T,C> & lhs, const stack<T,C> & rhs )
    {
        return !( lhs.c == rhs.c );
    }

    //-------------------------------------------------------------------------
    template< class T, class C > x_bool operator <  ( const stack<T,C> & lhs, const stack<T,C> & rhs )
    {
        return lhs.c < rhs.c ;
    }

    //-------------------------------------------------------------------------
    template< class T, class C > x_bool operator >  ( const stack<T,C> & lhs, const stack<T,C> & rhs )
    {
        return rhs.c < lhs.c ;
    }

    //-------------------------------------------------------------------------
    template< class T, class C > x_bool operator <= ( const stack<T,C> & lhs, const stack<T,C> & rhs )
    {
        return !( rhs.c < lhs.c );
    }

    //-------------------------------------------------------------------------
    template< class T, class C > x_bool operator >= ( const stack<T,C> & lhs, const stack<T,C> & rhs )
    {
        return !( lhs.c < rhs.c );
    }
}

#endif


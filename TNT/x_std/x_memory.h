#ifndef X_STD_MEMORY_H
#define X_STD_MEMORY_H

///////////////////////////////////////////////////////////////////////////////
//
// Declarations based on STL <memory>
//
// Defines a class, an operator, and several templates that help allocate and 
// free objects.
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
// 1. See the deviations listed under allocator<T>.
//
///////////////////////////////////////////////////////////////////////////////
//
// Overview 
//
// allocator<T>     A template class for allocating and freeing arrays of objects
//                  of type T.
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    <stddef.h>

namespace x_std 
{
    ///////////////////////////////////////////////////////////////////////////
    //
    // allocator<T>
    //
    // This template class describes an object that manages storage allocation 
    // and freeing for arrays of objects of type T. An object of class 
    // allocator is the default allocator object specified in the constructors 
    // for several container template classes in the Standard C++ library.
    //
    ///////////////////////////////////////////////////////////////////////////
    //
    // Deviation from standard STL behavior
    //
    // 1. Each allocator must have a template rebind<> as shown below.
    // 2. The hint parameter to allocate() defaults to 0.
    // 3. There is an additional x_construct(p) function which does not require 
    //    an initial value.
    // 4. There are functions x_allocate_char() and x_deallocate_char() to
    //    allocate arbitrary memory - used for systems that do not support the
    //    template rebind mechanism.
    //
    ///////////////////////////////////////////////////////////////////////////
    //
    // Overview of basic functions
    // 
    // allocate(n)      Allocate an array of n elements of type T. 
    //                  Return the pointer. Does not call T's constructor.
    // deallocate(p,n)  Deallocate an array of n elements of type T. p is a 
    //                  pointer returned previously by allocate. Does not
    //                  call T's destructor for the elements.
    // 
    ///////////////////////////////////////////////////////////////////////////
    template<class T>
    class allocator 
    {
      public :
        typedef x_size_t    size_type       ;
        typedef ptrdiff_t   difference_type ;
        typedef T *         pointer         ;
        typedef const T *   const_pointer   ;
        typedef T &         reference       ;
        typedef const T &   const_reference ;
        typedef T           value_type      ;

        pointer         address( reference       x ) const { return &x ; }
        const_pointer   address( const_reference x ) const { return &x ; }

        allocator() {}
        allocator<T> & operator=( const allocator<T> ) { return *this; }
        x_bool         operator==( const allocator<T>&){ return true; } //Used by container's swap() function

        pointer allocate    ( size_type n , const void * /*hint*/= 0 ) ;
        void    deallocate  ( pointer   p , size_type /* n */        ) ;
        void    x_construct ( pointer   p                            ) { placement_new<T>( p );                 }
        void    construct   ( pointer   p , const T & value          ) { placement_new<T>( p , value );         }
        void    destroy     ( pointer   p                            ) { pointer q; q = p; q->T::~T();          }

        size_type max_size() const { return 1000000000 ; }

        // The rebind template class, by which container classes (such as set<> or list<>)
        // can use a supplied allocator to allocate types other than those passed 
        // as template arguments.
        template < class U >
        struct rebind 
        {
            typedef allocator<U> other;
        };

        // Raw memory allocation and deallocation - used by container classes when the "template rebind"
        // mechanism is not supported by the compiler.
        void * x_allocate_char  ( size_type byte_count                        ) ;
        void   x_deallocate_char( void * memory        , size_type /*byte_count*/ );
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // auto_ptr<T>
    //
    // The class describes an object that stores a pointer to an allocated 
    // object of type T. The stored pointer must either be null or designate an 
    // object allocated by a new expression. The object also stores an 
    // ownership indicator. An object constructed with a non-null pointer owns 
    // the pointer. It transfers ownership if its stored value is assigned to 
    // another object. The destructor for auto_ptr<T> deletes the allocated 
    // object if it owns it.
    //
    ///////////////////////////////////////////////////////////////////////////
    //
    // Deviation from standard STL behavior
    //
    //    None.
    //
    ///////////////////////////////////////////////////////////////////////////
    //
    // Overview of basic functions
    // 
    // 
    ///////////////////////////////////////////////////////////////////////////

    // template<class T>
    // class auto_ptr 
    // {
    //   public:
    //     typedef T element_type;
    //     
    //     explicit auto_ptr( T * p = 0 ); // Construct *this from p, setting the ownership indicator true unless p==0.
    //     auto_ptr(const auto_ptr<T>& rhs) throw();
    //     auto_ptr<T>& operator=(auto_ptr<T>& rhs) throw();
    //     ~auto_ptr();
    //     T& operator*() const throw();
    //     T *operator->() const throw();
    //     T *get() const throw();
    //     T *release() const throw();
    // };



    //todo:     class allocator<void>;
    //todo: template<class FwdIt, class T>
    //todo:     class raw_storage_iterator;

    //todo: template<class T>
    //todo:     bool operator==(allocator<T>& lhs,
    //todo:         allocator<T>& rhs);
    //todo: template<class T>
    //todo:     bool operator!=(allocator<T>& lhs,
    //todo:         allocator<T>& rhs);
    //todo: template<class T>
    //todo:     void operator delete(void *p, size_t n, allocator& al);
    //todo: template<class T>
    //todo:     void *operator new(size_t n, allocator& al);
    //todo: template<class T>
    //todo:     pair<T *, ptrdiff_t> get_temporary_buffer(ptrdiff_t n, T *);
    //todo: template<class T>
    //todo:     void return_temporary_buffer(T *p);
    //todo: template<class InIt, class FwdIt>
    //todo:     FwdIt uninitialized_copy(InIt first, InIt last, FwdIt result);
    //todo: template<class FwdIt, class T>
    //todo:     void uninitialized_fill(FwdIt first, FwdIt last, const T& x);
    //todo: template<class FwdIt, class Size, class T>
    //todo:     void uninitialized_fill_n(FwdIt first, Size n, const T& x);
    

}

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------
template <class T> 
typename x_std::allocator<T>::pointer x_std::allocator<T>::allocate( size_type n , const void * /*hint*/  ) 
{ 
    #if X_STD_KEEP_STATISTICS
    {
        x_std_statistics :: calls_to_base_allocate  +=  1   ;
        x_std_statistics :: size_of_base_allocate   +=  n * sizeof(T) ;
        x_std_statistics :: calls_to_all_allocate   +=  1   ;
        x_std_statistics :: size_of_all_allocate    +=  n * sizeof(T) ;
    }
    #endif

    return reinterpret_cast<pointer>(new char[n*sizeof(T)]); 
}

template <class T> 
void x_std::allocator<T>::deallocate( pointer p , size_type n ) 
{ 
    #if X_STD_KEEP_STATISTICS
    if( p != 0 )
    {
        x_std_statistics :: calls_to_base_deallocate    +=  1 ;
        x_std_statistics :: size_of_base_deallocate     +=  n * sizeof(T) ;
        x_std_statistics :: calls_to_all_deallocate     +=  1 ;
        x_std_statistics :: size_of_all_deallocate      +=  n * sizeof(T) ;
    }
    #endif

    delete[] reinterpret_cast<char *>(p); 
    n = n ; // Trick to avoid compiler warnings about unreferenced formal parameter.
}

template <class T> 
void * x_std::allocator<T>::x_allocate_char ( size_type byte_count ) 
{ 
    #if X_STD_KEEP_STATISTICS
    {
        x_std_statistics :: calls_to_base_allocate_char +=  1   ;
        x_std_statistics :: size_of_base_allocate_char  +=  byte_count ;
        x_std_statistics :: calls_to_all_allocate_char  +=  1   ;
        x_std_statistics :: size_of_all_allocate_char   +=  byte_count ;
    }
    #endif

    return new char[byte_count]; 
}

template <class T> 
void x_std::allocator<T>::x_deallocate_char( void * memory , size_type byte_count ) 
{ 
    #if X_STD_KEEP_STATISTICS
    if( memory != 0 )
    {
        x_std_statistics :: calls_to_base_deallocate_char   +=  1   ;
        x_std_statistics :: size_of_base_deallocate_char    +=  byte_count ;
        x_std_statistics :: calls_to_all_deallocate_char    +=  1   ;
        x_std_statistics :: size_of_all_deallocate_char     +=  byte_count ;
    }
    #endif

    delete[] reinterpret_cast<char*>(memory) ; 
    byte_count = byte_count ; // Trick to avoid compiler warnings about unreferenced formal parameter.
}

#endif

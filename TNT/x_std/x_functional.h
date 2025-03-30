#ifndef X_STD_FUNCTIONAL_H
#define X_STD_FUNCTIONAL_H

///////////////////////////////////////////////////////////////////////////////
//
// Function objects based on STL <functional>
//
// This header defines several templates that help construct function objects, 
// objects of a class that defines operator(). Hence, function objects behave 
// much like function pointers, except that the object can store additional 
// information that can be used during a function call.
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
    // unary_function< argument_type , result_type >
    // 
    // This template class serves as a base for classes that define a member 
    // function of the form:
    //
    //      result_type operator()(argument_type)
    //
    ///////////////////////////////////////////////////////////////////////////
    template < class ArgumentType , class ResultType >
    struct unary_function 
    {
        typedef ArgumentType    argument_type   ;
        typedef ResultType      result_type     ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // binary_function< first_argument_type , second_argument_type , result_type >
    // 
    // This template class serves as a base for classes that define a member 
    // function of the form:
    //
    //      result_type operator()(first_argument_type, second_argument_type)
    //
    ///////////////////////////////////////////////////////////////////////////
    template < class FirstArgumentType , class SecondArgumentType , class ResultType >
    struct binary_function 
    {
        typedef FirstArgumentType   first_argument_type     ;
        typedef SecondArgumentType  second_argument_type    ;
        typedef ResultType          result_type             ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // Various templates for binary and unary functions using +, -, *, and so on.
    // 
    ///////////////////////////////////////////////////////////////////////////

    template < class T > struct plus : public binary_function<T, T, T> 
    {
        T operator()( const T & x , const T & y ) const { return x + y ; }
    };

    template<class T>
    struct minus : public binary_function<T, T, T> 
    {
        T operator()( const T & x , const T & y ) const { return x - y ; }
    };

    template<class T>
    struct multiplies : public binary_function<T, T, T> 
    {
        T operator()( const T & x , const T & y ) const { return x * y ; }
    };


    template<class T>
    struct divides : public binary_function<T, T, T>    
    {
        T operator()( const T & x , const T & y ) const { return x / y ; }
    };

    template<class T>
    struct modulus : public binary_function<T, T, T> 
    {
        T operator()( const T & x , const T & y ) const { return x % y ; }
    };


    template<class T>
    struct negate : public unary_function<T, T>     
    {
        T operator()( const T & x ) const { return -x ; }
    };

    template<class T>
    struct equal_to : public binary_function<T, T, x_bool> 
    {
        x_bool operator()( const T & x , const T & y ) const { return x == y ; }
    };

    template<class T>
    struct not_equal_to : public binary_function<T, T, x_bool> 
    {
        x_bool operator()( const T & x , const T & y ) const { return x != y ; }
    };

    template<class T>
    struct greater : public binary_function<T, T, x_bool> 
    {
        x_bool operator()( const T & x , const T & y ) const { return x > y ; }
    };


    template<class T>
    struct less : public binary_function<T, T, x_bool> 
    {
        x_bool operator()( const T & x , const T & y ) const { return x < y ; }
    };

    template<class T>
    struct greater_equal : public binary_function<T, T, x_bool> 
    {
        x_bool operator()( const T & x , const T & y ) const { return x >= y ; }
    };


    template<class T>
    struct less_equal : public binary_function<T, T, x_bool> 
    {
        x_bool operator()( const T & x , const T & y ) const { return x <= y ; }
    };

    template<class T>
    struct logical_and : public binary_function<T, T, x_bool> 
    {
        x_bool operator()( const T & x , const T & y ) const { return x && y ; }
    };

    template<class T>
    struct logical_or : public binary_function<T, T, x_bool> 
    {
        x_bool operator()( const T & x , const T & y ) const { return x || y ; }
    };

    template<class T>
    struct logical_not : public unary_function<T, x_bool> 
    {
        x_bool operator()( const T & x ) const { return !x ; }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // unary_negate < UnaryPredicate >      
    //    
    // UnaryPredicate must be a unary_function.
    // A template class to define operator() which negates a boolean unary function.
    //
    ///////////////////////////////////////////////////////////////////////////
    template< class UnaryPredicate >
    class unary_negate : public unary_function<typename UnaryPredicate::argument_type, x_bool> 
    {
      public :
        explicit unary_negate( const UnaryPredicate & pr ) : predicate(pr) {}
        x_bool operator()(const typename UnaryPredicate::argument_type& x) const { return !predicate(x); }
      private :
        UnaryPredicate predicate ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // binary_negate< BinaryPredicate >
    //
    // BinaryPredicate must be a binary_function.
    // A template class to define operator() which negates a boolean binary function.
    //
    ///////////////////////////////////////////////////////////////////////////
    template<class BinaryPredicate>
    class binary_negate 
        : public binary_function
            <
                typename BinaryPredicate::first_argument_type   , 
                typename BinaryPredicate::second_argument_type  , 
                x_bool
            > 
    {
      public:
        explicit binary_negate(const BinaryPredicate & pr) : predicate(pr) { }
        x_bool operator()
        (
            const typename BinaryPredicate::first_argument_type  & x , 
            const typename BinaryPredicate::second_argument_type & y
        ) 
        const { return !predicate(x,y); }
      private:
        BinaryPredicate predicate ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // binder1st< BinaryFunction >
    //
    // BinaryFunction must be a binary_function.
    // A template class to bind the first parameter of BinaryFunction to 
    // be value x and define operator(y) as op(x,y).
    //
    ///////////////////////////////////////////////////////////////////////////
    template<class BinaryFunction>
    class binder1st 
        : public unary_function
            <
                typename BinaryFunction::second_argument_type , 
                typename BinaryFunction::result_type
            > 
    {
      public:
        binder1st(const BinaryFunction & f, const typename BinaryFunction::first_argument_type x) : op(f) { value = x ; }
        typename BinaryFunction::result_type operator()(const typename BinaryFunction::second_argument_type & y) const { return op(value,y); }
      protected:
        BinaryFunction op ;
        typename BinaryFunction::first_argument_type value ;
      };

    ///////////////////////////////////////////////////////////////////////////
    //
    // binder2nd< BinaryFunction >
    //
    // BinaryFunction must be a binary_function.
    // A template class to bind the first parameter of BinaryFunction to 
    // be value x and define operator(y) as op(x,y).
    //
    ///////////////////////////////////////////////////////////////////////////

    template<class BinaryFunction>
    class binder2nd 
        : public unary_function
            <
                typename BinaryFunction::first_argument_type    , 
                typename BinaryFunction::result_type
            > 
    {
      public:
        binder2nd(const BinaryFunction & f, const typename BinaryFunction::second_argument_type y ) : op(f) , value(y) { value = y ; }
        typename BinaryFunction::result_type operator()(const typename BinaryFunction::first_argument_type& x) const;
      protected:
        BinaryFunction op;
        typename BinaryFunction::second_argument_type value;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // pointer_to_unary_function< ArgumentType , ResultType >
    //
    // A template class which defines operator(x) as (*f)(x).
    //
    ///////////////////////////////////////////////////////////////////////////

    template< class ArgumentType , class ResultType >
    class pointer_to_unary_function : public unary_function<ArgumentType, ResultType> 
    {
      public:
        explicit pointer_to_unary_function(ResultType (*f)(ArgumentType) ) : op(f) { }
        ResultType operator()(ArgumentType x) const { return (*op)(x); }
      private:
        ResultType (*op)(ArgumentType) ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // pointer_to_binary_function< ArgumentType , ResultType >
    //
    // A template class which defines operator(x,y) as (*f)(x,y).
    //
    ///////////////////////////////////////////////////////////////////////////

    template<class FirstArgumentType, class SecondArgumentType, class ResultType>
    class pointer_to_binary_function : public binary_function<FirstArgumentType, SecondArgumentType, ResultType> 
    {
      public:
        explicit pointer_to_binary_function(ResultType (*f)(FirstArgumentType, SecondArgumentType));
        ResultType operator()(const FirstArgumentType x, const SecondArgumentType y) const { return (*op)(x,y); }
      private:
        ResultType (*op)(FirstArgumentType,SecondArgumentType) ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // mem_fun_t< ResultType , ClassType >
    //
    // Pointer to member function with no parameters.
    // A template class which defines operator(p) as (p->*member_function)().
    //
    ///////////////////////////////////////////////////////////////////////////

    template<class ResultType, class ClassType>
    struct mem_fun_t : public unary_function<ClassType *, ResultType> 
    {
        explicit mem_fun_t(ResultType (ClassType::*pm)()) : member_function(pm) {}
        ResultType operator()(ClassType *p) { return (p->*member_function)(); }
        private : ResultType ( ClassType::*member_function )() ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // mem_fun1_t< ResultType , ClassType , ArgumentType >
    //
    // Template class for pointer to member function with one parameter.
    // Defines operator(p,argument) as (p->*member_function)(argument).
    //
    ///////////////////////////////////////////////////////////////////////////

    template< class ResultType , class ClassType , class ArgumentType >
    struct mem_fun1_t : public binary_function< ClassType * , ArgumentType, ResultType > 
    {
        explicit mem_fun1_t( ResultType (ClassType::*f)(ArgumentType) ) : member_function(f) {}
        ResultType operator()( ClassType * p , ArgumentType arg ) { return (p->*member_function)(arg) ; }
        private : ResultType (ClassType::*member_function)(ArgumentType)  ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // mem_fun_ref_t< ResultType , ClassType >
    //
    // Template class for pointer to member function with no parameters.
    // Defines operator(x) as (x.*member_function)().
    //
    ///////////////////////////////////////////////////////////////////////////

    template < class ResultType , class ClassType >
    struct mem_fun_ref_t : public unary_function<ClassType *, ResultType> 
    {
        explicit mem_fun_ref_t(ResultType (ClassType::*pm)()) : member_function(pm) {}
        ResultType operator()(ClassType & x) { return (x.*member_function)() ; }
        private : ResultType (ClassType::*member_function)() ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // mem_fun1_ref_t< ResultType , ClassType , ArgumentType >
    //
    // Template class for pointer to member function with one parameter.
    // Defines operator(x,argument) as (x.*member_function)(argument).
    //
    ///////////////////////////////////////////////////////////////////////////

    template < class ResultType , class ClassType , class ArgumentType >
    struct mem_fun1_ref_t : public binary_function< ClassType * , ArgumentType , ResultType >  
    {
        explicit mem_fun1_ref_t( ResultType (ClassType::*f)(ArgumentType) ) : member_function(f) { }
        ResultType operator()(ClassType& x, ArgumentType arg) { return (x.*member_function)(arg); }
        private : ResultType (ClassType::*member_function)(ArgumentType) ;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // ptr_fun< ArgumentType , ResultType >( ResultType (*f)(ArgumentType )
    //
    // Template function to make a pointer_to_unary_function out of function f.
    //
    ///////////////////////////////////////////////////////////////////////////
    template< class FirstArgumentType , class SecondArgumentType , class ResultType >
    pointer_to_binary_function< FirstArgumentType , SecondArgumentType , ResultType >
    ptr_fun( ResultType (*f)(FirstArgumentType,SecondArgumentType) )
    {
        return pointer_to_binary_function<FirstArgumentType,SecondArgumentType,ResultType>(f);
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // ptr_fun< ArgumentType , ResultType >( ResultType (*f)(ArgumentType )
    //
    // Template function to make a pointer_to_unary_function out of function f.
    //
    ///////////////////////////////////////////////////////////////////////////
    template< class ArgumentType , class ResultType >
    pointer_to_unary_function< ArgumentType , ResultType > ptr_fun( ResultType (*f)(ArgumentType) )
    {
        return pointer_to_unary_function<ArgumentType,ResultType>(f);
    }



//  TEMPLATE FUNCTIONS
template<class Predicate>
    unary_negate<Predicate> not1(const Predicate& pr);
template<class Predicate>
    binary_negate<Predicate> not2(const Predicate& pr);
template<class Predicate, class T>
    binder1st<Predicate> bind1st(const Predicate& pr, const T& x);
template<class Predicate, class T>
    binder2nd<Predicate> bind2nd(const Predicate& pr, const T& x);
template<class R, class T>
    mem_fun_t<R, T> mem_fun(R (T::*pm)());
template<class R, class T, class A>
    mem_fun1_t<R, T, A> mem_fun1(R (T::*pm)(A arg));
template<class R, class T>
    mem_fun_ref_t<R, T> mem_fun_ref(R (T::*pm)());
template<class R, class T, class A>
    mem_fun1_ref_t<R, T, A> mem_fun1_ref(R (T::*pm)(A arg));
}

#endif
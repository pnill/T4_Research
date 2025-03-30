#if !defined( Basics_H )
#define Basics_H

//------------------------------------------------------------------------
// Basic programming support
//------------------------------------------------------------------------
// Declare things that are simple, basic, and GENERALLY useful. Don't use this 
// file as an excuse to throw in anything that might possibly be used in a 
// project. There shouldn't be more than about 20 or 30 declarations here.

// Square(X), Cube(X): Small powers
template < class T > inline T Square ( T X ) { return X * X     ; }
template < class T > inline T Cube   ( T X ) { return X * X * X ; }

// Abs(X): Absolute value
template < class T > inline T Abs( T X )
{ 
    return X < 0 ? -X : X; 
}

// IsInRange(X,Min,Max): Range test
template< class T > inline bool IsInRange( T X , T Min , T Max )  
{ 
    return X >= Min && X <= Max ;
}

// FitToRange(X,Min,Max): Fit a value within a range at the nearest endpoint.
template < class T > inline T FitToRange( T X , T Min , T Max )
{
    return (X <= Min) ? Min : (X >= Max) ? Max : X ;
}

// WrapToRange(X,Min,Max): Fit a value within a range at the opposite endpoint.
template < class T > inline T WrapToRange( T X , T Min , T Max )
{
    return (X < Min) ? Max : (X > Max) ? Min : X ;
}

// Min(X,Y): Minimum of 2 values
template < class T > inline T Min( T X , T Y )
{
    return X <= Y ? X : Y ;
}

// Max(X,Y): Maximum of 2 values
template < class T > inline T Max( T X , T Y )
{
    return X >= Y ? X : Y ;
}

// Zero(X): Set X to zeros
template < class T > inline void Zero( T & X ) { x_memset( &X , 0 , sizeof(X) ); }

// ARRAY_LENGTH(X): How many elements in array X (X must be an array, not a pointer to one)
#define ARRAY_LENGTH(X) ((int)( sizeof(X) / sizeof((X)[0]) ))

// RoundUp(X,N): Round X (>0) up to the nearest multiple of N
template < class T > inline T RoundUp( T X , int N )
{
    return ( (int)X + N - 1) / N  * N;
}

// RoundDown(X,N): Round X (>0) down to the nearest multiple of N
template < class T > inline T RoundDown( T X , int N )
{
    return ( (int)X / N ) * N;
}

#endif

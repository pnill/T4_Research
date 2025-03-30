///////////////////////////////////////////////////////////////////
//
//  Axis Aligned Bounding Box
//
//
///////////////////////////////////////////////////////////////////

#ifndef __X_ABBOX_HPP__
#define __X_ABBOX_HPP__

#ifndef X_MATH_HPP
#include "x_math.h"
#endif


////////////////////////////////////////////////////////////////////////////
//  Axis Aligned Bounding Box Class
////////////////////////////////////////////////////////////////////////////
struct aligned_bbox
{

    ///////////////////////////////////////////////////////////////////////
    //  Members
    ///////////////////////////////////////////////////////////////////////
    
    vector3 Corner[2];
 
    ///////////////////////////////////////////////////////////////////////
    // Constructors/Destructors
    ///////////////////////////////////////////////////////////////////////

	aligned_bbox( void );
	aligned_bbox( const vector3& Corner1, const vector3& Corner2 ); 
	aligned_bbox( const vector3* Corners );
	aligned_bbox( const aligned_bbox& ABB );
   ~aligned_bbox( void );


    ////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////
	void	Normalize	( void );
	void	Set			( const vector3& Corner1, const vector3& Corner2 ); 
	void	Set			( const vector3* Corners );
	void	Set			( const aligned_bbox& ABB );

	void	Offset		( f32 X, f32 Y, f32 Z );
	void	Inflate		( f32 X, f32 Y, f32 Z );
	void	Deflate		( f32 X, f32 Y, f32 Z );
	void	Scale		( f32 X, f32 Y, f32 Z );
    void    AddPtToBox  ( const vector3& vPt );

	void	Offset		( const vector3& vAmount );
	void	Inflate		( const vector3& vAmount );
	void	Deflate		( const vector3& vAmount );
	void	Scale		( const vector3& vAmount );
														
	vector3	Dim			( void )						const;
	vector3	TopLeft		( void )						const;
	vector3	BottomLeft	( void )						const;
	vector3	CenterPoint	( void )						const;
														
	bool	IsEmpty		( void )						const;
	bool	IsNull		( void )						const;
	bool	IsNormalized( void )						const;
	bool	PtInABBox	( const vector3& V )			const;
	bool	ABBoxInABBox( const aligned_bbox& ABB  )	const;

    ///////////////////////////////////////////////////////////////////
    // Operators
    ///////////////////////////////////////////////////////////////////

	bool	operator ==( const aligned_bbox& ABB ) const;
	bool	operator !=( const aligned_bbox& ABB ) const;

	aligned_bbox&	operator = ( const aligned_bbox& ABB );
	aligned_bbox&	operator +=( const aligned_bbox& ABB );  //Union Assignment
	aligned_bbox&	operator -=( const aligned_bbox& ABB );  //Subtract Assignment
	aligned_bbox&	operator |=( const aligned_bbox& ABB );  //Intersect Assignment

    friend	aligned_bbox	operator + ( const aligned_bbox& ABB1, const aligned_bbox& ABB2 );
    friend	aligned_bbox	operator - ( const aligned_bbox& ABB1, const aligned_bbox& ABB2 );
    friend	aligned_bbox	operator | ( const aligned_bbox& ABB1, const aligned_bbox& ABB2 );

};

#include "x_abbox.inl"

#endif //__X_ABBOX_HPP__
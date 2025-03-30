

#ifndef __X_MATH_ABBOX_INL__
#define __X_MATH_ABBOX_INL__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"

////////////////////////////////////////////////////////////////////////////
//  Default Constructors
////////////////////////////////////////////////////////////////////////////

inline aligned_bbox:: aligned_bbox( void ) {   }
inline aligned_bbox::~aligned_bbox( void ) {   }


//////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////

inline aligned_bbox::aligned_bbox( const vector3& Corner1, const vector3& Corner2 )
{
    Corner[0] = Corner1;
    Corner[1] = Corner2;
}

inline aligned_bbox::aligned_bbox( const vector3* Corners )
{
    Corner[0] = Corners[0];
    Corner[1] = Corners[1];
}

inline aligned_bbox::aligned_bbox( const aligned_bbox& ABB )
{
    Corner[0] = ABB.Corner[0];
    Corner[1] = ABB.Corner[1];    
}


//////////////////////////////////////////////////////////////////////
// Assignment Operators
//////////////////////////////////////////////////////////////////////

inline aligned_bbox& aligned_bbox::operator=(const aligned_bbox& ABB)
{
	Corner[0] = ABB.Corner[0];
	Corner[1] = ABB.Corner[1];
	return *this;
}

inline aligned_bbox& aligned_bbox::operator+=(const aligned_bbox& ABB)
{
	return *this = *this + ABB;
}

inline aligned_bbox& aligned_bbox::operator-=(const aligned_bbox& ABB)
{
	return *this = *this - ABB;
}

inline aligned_bbox& aligned_bbox::operator|=(const aligned_bbox& ABB)
{
	return *this = *this | ABB;
}


//////////////////////////////////////////////////////////////////////
// Logic Operators
//////////////////////////////////////////////////////////////////////

inline bool aligned_bbox::operator==(const aligned_bbox& ABB) const
{
	return
	(
		Corner[0] == ABB.Corner[0] &&
		Corner[1] == ABB.Corner[1]
	) ? true : false;
}

inline bool aligned_bbox::operator!=(const aligned_bbox& ABB) const
{
	return !(*this == ABB); 
}

//////////////////////////////////////////////////////////////////////
// Manipulation Functions
//////////////////////////////////////////////////////////////////////

inline void aligned_bbox::Normalize(void)
{
	f32 t;
	if (Corner[0].X > Corner[1].X)
	{
		t = Corner[0].X;
		Corner[0].X = Corner[1].X;
		Corner[1].X = t;
	}
	if (Corner[0].Y > Corner[1].Y)
	{
		t = Corner[0].Y;
		Corner[0].Y = Corner[1].Y;
		Corner[1].Y = t;
	}
	if (Corner[0].Z > Corner[1].Z)
	{
		t = Corner[0].Z;
		Corner[0].Z = Corner[1].Z;
		Corner[1].Z = t;
	}
}

inline void aligned_bbox::Set( const vector3& Corner1, const vector3& Corner2 )
{
    Corner[0] = Corner1;
    Corner[1] = Corner2;
}


inline void aligned_bbox::Set( const vector3* Corners )
{
    Corner[0] = Corners[0];
    Corner[1] = Corners[1];
}

inline void aligned_bbox::Set( const aligned_bbox& ABB )
{
    Corner[0] = ABB.Corner[0];
    Corner[1] = ABB.Corner[1];    
}

inline void aligned_bbox::Offset( const vector3& vOffset )
{
	Corner[0] += vOffset;
	Corner[1] += vOffset;
}

inline void aligned_bbox::Offset( f32 X, f32 Y, f32 Z )
{
	Offset(vector3(X,Y,Z));
}

inline void aligned_bbox::Inflate( const vector3& vAmount )
{
	Corner[0] -= vAmount;
	Corner[1] += vAmount;
}

inline void aligned_bbox::Inflate( f32 X, f32 Y, f32 Z )
{
	Inflate(vector3(X,Y,Z));
}

inline void aligned_bbox::Deflate( const vector3& vAmount )
{
	Corner[0] += vAmount;
	Corner[1] -= vAmount;
}

inline void aligned_bbox::Deflate( f32 X, f32 Y, f32 Z )
{
	Deflate(vector3(X,Y,Z));
}

inline void aligned_bbox::Scale( const vector3& vAmount )
{
	Corner[0].X *= vAmount.X;
	Corner[1].X *= vAmount.X;
	Corner[0].Y *= vAmount.Y;
	Corner[1].Y *= vAmount.Y;
	Corner[0].Z *= vAmount.Z;
	Corner[1].Z *= vAmount.Z;
}

inline void aligned_bbox::AddPtToBox( const vector3& vPt )
{
	Corner[0].X = MIN( Corner[0].X, vPt.X );
    Corner[0].Y = MIN( Corner[0].Y, vPt.Y );
    Corner[0].Z = MIN( Corner[0].Z, vPt.Z );

	Corner[1].X = MAX( Corner[1].X, vPt.X );
    Corner[1].Y = MAX( Corner[1].Y, vPt.Y );
    Corner[1].Z = MAX( Corner[1].Z, vPt.Z );
}

inline void aligned_bbox::Scale( f32 X, f32 Y, f32 Z )
{
	Scale(vector3(X,Y,Z));
}

//////////////////////////////////////////////////////////////////////
// Info Functions
//////////////////////////////////////////////////////////////////////

inline vector3 aligned_bbox::Dim( void ) const
{
	return Corner[1] - Corner[0];
}

inline vector3 aligned_bbox::TopLeft( void ) const
{
	return Corner[0];
}

inline vector3 aligned_bbox::BottomLeft( void ) const
{
	return Corner[1];
}

inline vector3 aligned_bbox::CenterPoint( void ) const
{
    vector3 dim(Dim());
	return vector3(Corner[0].X + (dim.X * 0.5f), Corner[0].Y + (dim.Y * 0.5f), Corner[0].Z + (dim.Z * 0.5f));
}

//////////////////////////////////////////////////////////////////////
// Test Functions
//////////////////////////////////////////////////////////////////////

inline bool aligned_bbox::IsEmpty( void ) const
{
    vector3 dim(Dim());
	return ( dim.X <= 0 || dim.Y <= 0 || dim.Z <= 0 ) ? true : false;
}

inline bool aligned_bbox::IsNull( void ) const
{
    vector3 dim(Dim());
	return ( dim.X <= 0 && dim.Y <= 0 && dim.Z <= 0 ) ? true : false;
}

inline bool aligned_bbox::IsNormalized( void ) const
{
	return ((Corner[0].X > Corner[1].X) ||
            (Corner[0].Y > Corner[1].Y) ||
			(Corner[0].Z > Corner[1].Z)) 
			? false : true;
}

inline bool aligned_bbox::PtInABBox( const vector3& V ) const
{
	//Point lies in aligned_bbox if its x value is between the left and right value of the aligned_bbox
	//and if its y value is between the top and bottom value of the aligned_bbox.
	return ((V.X >= Corner[0].X && V.X <= Corner[1].X) && 
            (V.Y >= Corner[0].Y && V.Y <= Corner[1].Y) && 
            (V.Z >= Corner[0].Z && V.Z <= Corner[1].Z)) ? true : false;

}

inline bool aligned_bbox::ABBoxInABBox( const aligned_bbox& ABB ) const
{
	// To find if aligned_bbox is intersecting another rec:
	// 1.) Find largest top and left value.
	// 2.) Find smallest bottom and right value
	// 3.) DeteABBorner[0]e if these values create a normal rec
	// If it does then aligned_bboxs intersect and considered "In"
	//long t = (top > ABB1.top)		? top		: ABB1.top;
	//long l = (left > ABB1.left)		? left		: ABB1.left;
	//long b = (bottom < ABB1.bottom)	? bottom	: ABB1.bottom;
	//long r = (right < ABB1.right)	? right		: ABB1.right;
	//return ((t < b) && (l < r)) ? true : false;

	return (
		( MAX(ABB.Corner[0].X, Corner[0].X) < MIN(ABB.Corner[1].X, Corner[1].X) ) &&
        ( MAX(ABB.Corner[0].Y, Corner[0].Y) < MIN(ABB.Corner[1].Y, Corner[1].Y) ) &&
		( MAX(ABB.Corner[0].Z, Corner[0].Z) < MIN(ABB.Corner[1].Z, Corner[1].Z) )) 
		? true : false;
}

//////////////////////////////////////////////////////////////////////
// Union / Subtract / Intersection (FRIEND FUNCTIONS)
//////////////////////////////////////////////////////////////////////

inline aligned_bbox operator+( const aligned_bbox& ABB1, const aligned_bbox& ABB2 )
{
	//Union aligned_bbox = smallest aligned_bbox that contains both souABBe aligned_bboxs

	aligned_bbox t_ABB( 
        vector3(MIN(ABB2.Corner[0].X, ABB1.Corner[0].X), 
                MIN(ABB2.Corner[0].Y, ABB1.Corner[0].Y), 
                MIN(ABB2.Corner[0].Z, ABB1.Corner[0].Z)),
        vector3(MAX(ABB2.Corner[1].X, ABB1.Corner[1].X), 
                MAX(ABB2.Corner[1].Y, ABB1.Corner[1].Y), 
                MAX(ABB2.Corner[1].Z, ABB1.Corner[1].Z)));

	if (t_ABB.IsNormalized()) return t_ABB;
	else return aligned_bbox(vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, 0.0f));
}

inline aligned_bbox operator-( const aligned_bbox& ABB1, const aligned_bbox& ABB2 )
{
	aligned_bbox t_ABB(ABB1);
	if (ABB1.Corner[1].X > ABB2.Corner[0].X) t_ABB.Corner[1].X = ABB2.Corner[0].X;
	if (ABB1.Corner[1].Y > ABB2.Corner[0].Y) t_ABB.Corner[1].Y = ABB2.Corner[0].Y;
	if (ABB1.Corner[1].Z > ABB2.Corner[0].Z) t_ABB.Corner[1].Z = ABB2.Corner[0].Z;
	if (ABB1.Corner[0].X < ABB2.Corner[1].X) t_ABB.Corner[0].X = ABB2.Corner[1].X;
	if (ABB1.Corner[0].Y < ABB2.Corner[1].Y) t_ABB.Corner[0].Y = ABB2.Corner[1].Y;
	if (ABB1.Corner[0].Z < ABB2.Corner[1].Z) t_ABB.Corner[0].Z = ABB2.Corner[1].Z;

	if (t_ABB.IsNormalized()) return t_ABB;
	else return aligned_bbox(vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, 0.0f));
}

inline aligned_bbox operator|( const aligned_bbox& ABB1, const aligned_bbox& ABB2 )
{
	//Intersection = intersection of two aligned_bboxs (union)
	return ABB1 + ABB2;
}

#endif
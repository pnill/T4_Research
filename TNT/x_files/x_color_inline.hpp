////////////////////////////////////////////////////////////////////////////
//
//  X_COLOR_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_COLOR_INLINE_HPP
#define X_COLOR_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

//==========================================================================

inline color::color( void )
{

}

//==========================================================================

inline color::color( u8 aR, u8 aG, u8 aB, u8 aA )
{   
    A = aA;
    R = aR;
    G = aG;
    B = aB;
}
    
//==========================================================================

inline color::color( u32 RGBA )
{   
    A = (u8)((RGBA & 0xFF000000) >> 24);
    B = (u8)((RGBA & 0x00FF0000) >> 16);
    G = (u8)((RGBA & 0x0000FF00) >>  8);
    R = (u8)((RGBA & 0x000000FF) >>  0);
}
    
//==========================================================================

inline void color::Set( u8 aR, u8 aG, u8 aB, u8 aA )
{  
    A = aA;
    R = aR;
    G = aG;
    B = aB;
}

//==========================================================================

inline void color::Set( u32 RGBA )
{   
    A = (u8)((RGBA & 0xFF000000) >> 24);
    B = (u8)((RGBA & 0x00FF0000) >> 16);
    G = (u8)((RGBA & 0x0000FF00) >>  8);
    R = (u8)((RGBA & 0x000000FF) >>  0);
}

//==========================================================================

inline u32 color::Get( void )
{   
	u32	RGBA;

	RGBA = (((u32)A)<<24) | (((u32)R)<<16) | (((u32)G)<<8) | ((u32)B);
	return RGBA;
}

//==========================================================================

inline xbool color::operator == ( const color& C )
{
    return ( (R==C.R) && (G==C.G) && (B==C.B) && (A==C.A) );
}
    
//==========================================================================

inline color& color::operator = ( u32 RGBA )
{
    A = (u8)((RGBA & 0xFF000000) >> 24);
    B = (u8)((RGBA & 0x00FF0000) >> 16);
    G = (u8)((RGBA & 0x0000FF00) >>  8);
    R = (u8)((RGBA & 0x000000FF) >>  0);
    return *this;
}

//==========================================================================

inline color& color::operator = ( s32 C )
{
	*this = (u32)C;
//    color::color( (u32)C );    
    return *this;
}

//==========================================================================

inline color& color::operator += ( const color& C )
{
	R += C.R;
	G += C.G;
	B += C.B;
	A += C.A;

	return (*this);
}

//==========================================================================

inline color& color::operator -= ( const color& C )
{
	R -= C.R;
	G -= C.G;
	B -= C.B;
	A -= C.A;

	return (*this);
}

//==========================================================================

inline color& color::operator *= ( f32 Scalar )
{
	R = (u8)( (f32)R * Scalar );
	G = (u8)( (f32)G * Scalar );
	B = (u8)( (f32)B * Scalar );
	A = (u8)( (f32)A * Scalar );

	return (*this);
}

//==========================================================================

inline color& color::operator /= ( f32 Scalar )
{
	R = (u8)( (f32)R / Scalar );
	G = (u8)( (f32)G / Scalar );
	B = (u8)( (f32)B / Scalar );
	A = (u8)( (f32)A / Scalar );

	return (*this);
}

//==========================================================================

inline color operator * ( const color& C1, const color& C2 )
{
	return color( C1.R * C2.R,
				  C1.G * C2.G,
				  C1.B * C2.B,
				  C1.A * C2.A );
}

//==========================================================================

inline color& color::operator *= ( const color& C )
{
	R = (u8)( (f32)R * (f32)C.R );
	G = (u8)( (f32)G * (f32)C.G );
	B = (u8)( (f32)B * (f32)C.B );
	A = (u8)( (f32)A * (f32)C.A );

	return (*this);
}

//==========================================================================

inline color operator + ( const color& C1, const color& C2 )
{
	return color( C1.R + C2.R,
				  C1.G + C2.G,
				  C1.B + C2.B,
				  C1.A + C2.A );
}

//==========================================================================

inline color operator - ( const color& C1, const color& C2 )
{
	return color( C1.R - C2.R,
				  C1.G - C2.G,
				  C1.B - C2.B,
				  C1.A - C2.A );
}

//==========================================================================

inline color operator * ( f32 Scalar, const color& C )
{
	return color( (u8)(C.R * Scalar),
				  (u8)(C.G * Scalar),
				  (u8)(C.B * Scalar),
				  (u8)(C.A * Scalar) );
}

//==========================================================================

inline color operator * ( const color& C, f32 Scalar )
{
	return color( (u8)(C.R * Scalar),
				  (u8)(C.G * Scalar),
				  (u8)(C.B * Scalar),
				  (u8)(C.A * Scalar) );
}

//==========================================================================

inline color operator / ( const color& C, f32 Scalar )
{
	return color( (u8)(C.R / Scalar),
				  (u8)(C.G / Scalar),
				  (u8)(C.B / Scalar),
				  (u8)(C.A / Scalar) );
}

//==========================================================================

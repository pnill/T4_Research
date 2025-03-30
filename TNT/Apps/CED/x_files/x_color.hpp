////////////////////////////////////////////////////////////////////////////
//
//  X_COLOR.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_COLOR_HPP
#define X_COLOR_HPP

////////////////////////////////////////////////////////////////////////////
//
//  This file provides the following:
//    - generic type 'color' with all appropriate operations
//
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
//  Future features:
//    - enumeration of *every* possible way to represent a color
//    - macros to convert between the standard color and the other formats
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  TYPES
////////////////////////////////////////////////////////////////////////////

struct color
{
		//
		// Data fields.
		//

		u8   R, G, B, A;

		//
		// Operations.
		//

		color           ( void );
		color           ( u8  R, u8  G, u8  B, u8  A = 255  );  // Values   0 .. 255
		color           ( u32 RGBA );
    
		void Set        ( u8  R, u8  G, u8  B, u8  A = 255  );  // Values   0 .. 255
		void Set        ( u32 RGBA );
		u32	 Get		( void );			// returns a packed u32 (ABGR)
    
        xbool  operator == ( const color& C );

		color& operator =  ( u32 RGBA );
		color& operator =  ( s32 RGBA );    
		color& operator += ( const color& C );
		color& operator -= ( const color& C );
		color& operator *= ( f32 Scalar );
		color& operator /= ( f32 Scalar );
friend	color  operator +  ( const color& C1, const color& C2 );
friend	color  operator -  ( const color& C1, const color& C2 );
friend	color  operator *  ( f32 Scalar, const color& C );
friend	color  operator *  ( const color& C, f32 Scalar );
friend	color  operator /  ( const color& C, f32 Scalar );
  
		// How about these operations?  
		//  c *  c
		//  c *= c
		//  c *  s
		//  s *  c
		//  c *= s
		//  division, too
		//  HLS?
};

////////////////////////////////////////////////////////////////////////////
//  All of the inline function implementations have been relegated to a
//  separate header file...

#include "x_color_inline.hpp"
    
////////////////////////////////////////////////////////////////////////////

//
// This list is currently in progress!
//
enum color_form
{
    COLOR_FORM_START_OF_LIST = -1,
    
    COLOR_FORM_32_ARGB_8888,
    COLOR_FORM_32_URGB_8888,
    COLOR_FORM_32_RGBA_8888,
    COLOR_FORM_32_RGBU_8888,

    COLOR_FORM_24_ARGB_8565,
    COLOR_FORM_24_RGB_888,

    COLOR_FORM_16_ARGB_1555,
    COLOR_FORM_16_URGB_1555,
    COLOR_FORM_16_RGBA_5551,
    COLOR_FORM_16_RGBU_5551,
    COLOR_FORM_16_ARGB_4444,
    COLOR_FORM_16_URGB_4444,
    COLOR_FORM_16_RGBA_4444,
    COLOR_FORM_16_RGBU_4444,
    COLOR_FORM_16_RGB_565,
    
    COLOR_FORM_END_OF_LIST
};


#define COLOR_BLACK      0xFF000000u
#define COLOR_WHITE      0xFFFFFFFFu
#define COLOR_RED        0xFF0000FFu    
#define COLOR_GREEN      0xFF00FF00u    
#define COLOR_BLUE       0xFFFF0000u    
#define COLOR_YELLOW     0xFF00FFFFu    
#define COLOR_AQUA       0xFFFFFF00u    
#define COLOR_PURPLE     0xFFFF00FFu
#define COLOR_GREY       0xFF080808u


////////////////////////////////////////////////////////////////////////////

#endif

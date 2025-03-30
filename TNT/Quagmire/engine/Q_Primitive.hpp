///////////////////////////////////////////////////////////////////////////
//
//  Q_PRIMITIVE.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef __Q_PRIMITIVE_HPP__
#define __Q_PRIMITIVE_HPP__

//==============================================================================
// 
//  QUAGMIRE PRIMITIVE
//
//  This module is used to create, destroy, and render basic primitive types
//  in both 2D and 3D space with little setup.  
//
//  AVAILABLE PRIMITIVE TYPES:
//
//      NAME                        VERTS REQUIRED      PRIMS / N VERTS
//
//      Point List                  1                   N
//      Line List                   2                   N / 2
//      Line Strip                  2                   N - 1
//      Triangle List               3                   N / 3
//      Triangle Strip              3                   N - 2
//      Triangle Fan                3                   N - 2
//
//==============================================================================

#include "x_types.hpp"
#include "x_math.hpp"
#include "x_color.hpp"

//==============================================================================
//  Primitive Creation Params Struct and Types
//==============================================================================

typedef void* PRIMHANDLE;

struct SPrimParam
{
    //--Members

	u32			PrimType;           // Type of Primative
	vector3*	pVert;              // Pointer to Vertices
	vector3*    pNormal;            // Pointer to Normal (Lit if NULL)
	color*		pColor;             // Pointer to Colors (White if NULL)
	vector2*    pUV;                // Pointer to UV (Untextured if NULL)
    s16*        pIndex;             // Pointer to Indice List (Not Indexed if NULL)
	s32			NVerts;             // Number of Verts
    s32         NIndex;

    //--Constructors

    SPrimParam      ( void );                   // Default Constructor
    SPrimParam      ( f32 Size );               // Quick Tri Constructor
    SPrimParam      ( f32 Height, f32 Width );  // Quick Quad Constructor
};

//==============================================================================
//  Primitive Type Flags
//
//  Use these flags within the SPrimParam structure during the creation of the 
//  primitive.  Some flags are combinable and some are mutually exculisive.
//==============================================================================

enum PRIM_PrimType
{
    //////////////////////////////////////////////////
    //--Combinable Flags
    //////////////////////////////////////////////////

	PRIMTYPE_NOCLIP				= 0x01000000,       // Skip Clipping Phase
	PRIMTYPE_STATIC			    = 0x02000000,       // Primitive cannot be modified        

    //////////////////////////////////////////////////
    //--Mutually Exculsive Flags
    //////////////////////////////////////////////////

	PRIMTYPE_2D					= 0x10000000,       // 2D Primative
	PRIMTYPE_3D					= 0x20000000,       // 3D Primative

    PRIMTYPE_POINTLIST          = 0x00000001,       // Point List
    PRIMTYPE_LINELIST           = 0x00000002,       // Line List
    PRIMTYPE_LINESTRIP          = 0x00000003,       // Line Strip
    PRIMTYPE_TRIANGLELIST       = 0x00000004,       // Triangle List
    PRIMTYPE_TRIANGLESTRIP      = 0x00000005,       // Triangle Strip
    PRIMTYPE_TRIANGLEFAN        = 0x00000006,       // Triangle Fan

    //////////////////////////////////////////////////
    //--Private Flags
    //////////////////////////////////////////////////

	//PRIMTYPE_PRIV_TEXTURED	= 0x00100000,       // Primitive has texture 
    //PRIMTYPE_PRIV_INDEXED     = 0x00000010,       // Primitive has indexed verts
};


//==============================================================================
//  Primitive Interface
//
//  PRIM_CreatePrimitive()
//      Creates a new primitive from the SPrimParam structure passed into the
//      function.  All data passed into the function is considered volitile and
//      will not used past the scope of this function.
//
//  PRIM_DestroyPrimitive()
//      Destroys the primitive and all memory allocated to the primitive.  Sets
//      PRIMHANDLE to NULL.
//
//  PRIM_RenderPrimitive()
//      Renders the primitive to the current render target using the current
//      active texture.  ENG_BeginRenderMode() must be called before using this
//      function.
//
//  PRIM_SetL2W()
//      Set the local to world matrix for the primative to the specified matrix.
//
//==============================================================================


PRIMHANDLE	PRIM_CreatePrimitive	( const SPrimParam& PrimParam );
void		PRIM_DestroyPrimitive	( PRIMHANDLE& PrimHandle );
s32		    PRIM_RenderPrimitive	( const PRIMHANDLE PrimHandle );

void        PRIM_SetL2W             ( const PRIMHANDLE PrimHandle, const matrix4& L2W );


#endif //__Q_PRIMITIVE_HPP__
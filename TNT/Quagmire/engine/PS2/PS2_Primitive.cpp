#include "Q_Engine.hpp"
#include "Q_Primitive.hpp"

////////////////////////////////////////////////////////////////////////////
//  Constructors for the SPrimParam
////////////////////////////////////////////////////////////////////////////

SPrimParam::SPrimParam( void )
{
}

//==========================================================================

SPrimParam::SPrimParam( f32 Size )
{
}

//==========================================================================

SPrimParam::SPrimParam( f32 Height, f32 Width )
{
}

////////////////////////////////////////////////////////////////////////////
//  Implementation of the PRIM module for the PS2
////////////////////////////////////////////////////////////////////////////

PRIMHANDLE PRIM_CreatePrimitive( const SPrimParam& PrimParam )
{
    return NULL;
}

//==========================================================================

void PRIM_DestroyPrimitive( PRIMHANDLE& PrimHandle )
{
}

//==========================================================================

s32 PRIM_RenderPrimitive( const PRIMHANDLE PrimHandle )
{
    return 0;
}

//==========================================================================

void PRIM_SetL2W( const PRIMHANDLE PrimHandle, const matrix4& L2W )
{
}

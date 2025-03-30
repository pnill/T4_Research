/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*
*		Filename:	EndianHelper.c
*		Date:		12/1/99
*
*		Desc:		This is a collection of endian functions that will help network
*					program to convert local data format to network data format
*					(big-endian) or vice-versa..
*
*		Revisions History:
*
*
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#include "x_files.hpp"

#include "EndianHelper.h"

///////////////////////////////////////////////////////////////////////////

#ifdef BIG_ENDIAN
	#define CPC_BIG_ENDIAN
#elif defined( LITTLE_ENDIAN )
	#define CPC_LITTLE_ENDIAN
#endif


#define LSB_FIRST  0
#define MSB_FIRST  1

/*-----------------------------------------------------------------*/
//Get byte order for local machine.
Eendiantype ENDIAN_GetLocalEndian( void )
{
	s16 w = 0x0001;
	s8  *b = (s8*) &w;

if ( LSB_FIRST == ( b[0] ? LSB_FIRST : MSB_FIRST ) )
    {//little endian
	return ( eendian_little );
    }
else if ( MSB_FIRST == ( b[0] ? LSB_FIRST : MSB_FIRST ) )
    {//Big endina
    return ( eendian_big );
    }
else
    {
    return ( eendian_none );
    }
}

/*-----------------------------------------------------------------*/


//Returns nonzero if local machine is little endian
u32     ENDIAN_IsLittleEndian( void )
{
if ( eendian_little == ENDIAN_GetLocalEndian() )
    return ( 1 );
else
    return ( 0 );

}

/*-----------------------------------------------------------------*/


//Returns nonzero if local machine is big endian
u32     ENDIAN_IsBigEndian( void )
{
if ( eendian_big == ENDIAN_GetLocalEndian() )
    return ( 1 );
else
    return ( 0 );
}

/*-----------------------------------------------------------------*/

u16     ENDIAN_LocalToNet16( u16 wInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN
    return ( ENDIAN_LittleToBig16( wInValue ) );
#elif defined CPC_BIG_ENDIAN 
	return ( wInValue );
#else
    #error "ENDIAN_LocalToNet16(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

u16     ENDIAN_NetToLocal16( u16 wInValue  )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN
    return ( ENDIAN_BigToLittle16( wInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( wInValue );
#else
    #error "ENDIAN_NetToLocal16(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

u32     ENDIAN_LocalToNet32( u32 dwInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN
    return ( ENDIAN_LittleToBig32( dwInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( dwInValue );
#else
    #error "ENDIAN_LocalToNet32(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

u32     ENDIAN_NetToLocal32( u32 dwInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN   
    return ( ENDIAN_BigToLittle32( dwInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( dwInValue );
#else
    #error "ENDIAN_NetToLocal32(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

u64     ENDIAN_LocalToNet64( u64 qwInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN
    return ( ENDIAN_LittleToBig64( qwInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( qwInValue );
#else
    #error "ENDIAN_LocalToNet64(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

u64     ENDIAN_NetToLocal64( u64 qwInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN
    return ( ENDIAN_BigToLittle64( qwInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( qwInValue );
#else
    #error "ENDIAN_LocalToNet64(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

f32     ENDIAN_LocalToNet_f32( f32 fdInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN
    return ( ENDIAN_LittleToBig_f32( fdInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( fdInValue );
#else
    #error "ENDIAN_LocalToNet_f32(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

f32     ENDIAN_NetToLocal_f32( f32 fdInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN    
    return ( ENDIAN_BigToLittle_f32( fdInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( fdInValue );
#else
    #error "ENDIAN_NetToLocal_f32(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

f64     ENDIAN_LocalToNet_f64( f64 fqInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN
    return ( ENDIAN_LittleToBig_f64( fqInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( fqInValue );
#else
    #error "ENDIAN_LocalToNet_f64(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/

f64     ENDIAN_NetToLocal_f64( f64 fqInValue )
{
//take advantage of Endian Designation which is defined in x_target.h
#ifdef CPC_LITTLE_ENDIAN
    return ( ENDIAN_BigToLittle_f64( fqInValue ) );
#elif defined CPC_BIG_ENDIAN
	return ( fqInValue );
#else
    #error "ENDIAN_NetToLocal_f64(), Not implemented for this platform."
#endif //end of #ifdef

}

/*-----------------------------------------------------------------*/




//ENDIAN_BigToLittle16 and ENDIAN_LittleToBig16 will perform the same functionality
u16     ENDIAN_BigToLittle16( u16 wInValue )
{
	u8	abyTemp[2]; 
	u8  *pTemp02 = (u8*)&wInValue;

	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[1];	
	abyTemp[1] = pTemp02[0];	
	return *((u16*)abyTemp);
}

/*-----------------------------------------------------------------*/

u16     ENDIAN_LittleToBig16( u16 wInValue )
{
	u8	abyTemp[2]; 
	u8  *pTemp02 = (u8*)&wInValue;

	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[1];	
	abyTemp[1] = pTemp02[0];	
	return *((u16*)abyTemp);
}

/*-----------------------------------------------------------------*/
//ENDIAN_BigToLittle32 and ENDIAN_LittleToBig32 will perform the same functionality

u32     ENDIAN_BigToLittle32( u32 dwInValue )
{
	u8	abyTemp[4]; 
	u8  *pTemp02 = (u8*)&dwInValue;

	//Didnot use while() or for() for performance efficiency	
	abyTemp[0] = pTemp02[3];	
	abyTemp[1] = pTemp02[2];	
	abyTemp[2] = pTemp02[1];	
	abyTemp[3] = pTemp02[0];	
	return *((u32*)abyTemp);

}
/*-----------------------------------------------------------------*/
u32     ENDIAN_LittleToBig32( u32 dwInValue )
{	
	u8	abyTemp[4]; 
	u8  *pTemp02 = (u8*)&dwInValue;

	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[3];	
	abyTemp[1] = pTemp02[2];	
	abyTemp[2] = pTemp02[1];	
	abyTemp[3] = pTemp02[0];	
	return *((u32*)abyTemp);
	
}
/*-----------------------------------------------------------------*/
//ENDIAN_BigToLittle64 and ENDIAN_LittleToBig64 will perform the same functionality

u64     ENDIAN_BigToLittle64( u64  qwInValue )
{	
	u8	abyTemp[8]; 
	u8  *pTemp02 = (u8*)&qwInValue;

	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[7];	
	abyTemp[1] = pTemp02[6];	
	abyTemp[2] = pTemp02[5];	
	abyTemp[3] = pTemp02[4];
	abyTemp[4] = pTemp02[3];	
	abyTemp[5] = pTemp02[2];	
	abyTemp[6] = pTemp02[1];	
	abyTemp[7] = pTemp02[0];
	return *((u64*)abyTemp);

}
/*-----------------------------------------------------------------*/
u64     ENDIAN_LittleToBig64( u64  qwInValue )
{
	u8	abyTemp[8]; 
	u8  *pTemp02 = (u8*)&qwInValue;
	
	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[7];	
	abyTemp[1] = pTemp02[6];	
	abyTemp[2] = pTemp02[5];	
	abyTemp[3] = pTemp02[4];
	abyTemp[4] = pTemp02[3];	
	abyTemp[5] = pTemp02[2];	
	abyTemp[6] = pTemp02[1];	
	abyTemp[7] = pTemp02[0];
	return *((u64*)abyTemp);
}

/*-----------------------------------------------------------------*/
//ENDIAN_BigToLittle_f32 and ENDIAN_LittleToBig_f32 will perform the same functionality

f32     ENDIAN_BigToLittle_f32( f32  fdInValue )
{	
	u8	abyTemp[4]; 
	u8  *pTemp02 = (u8*)&fdInValue;
	
	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[3];	
	abyTemp[1] = pTemp02[2];	
	abyTemp[2] = pTemp02[1];	
	abyTemp[3] = pTemp02[0];
	return *((f32*)abyTemp);
}

/*-----------------------------------------------------------------*/

f32     ENDIAN_LittleToBig_f32( f32 fdInValue )
{
	u8	abyTemp[4]; 
	u8  *pTemp02 = (u8*)&fdInValue;
	
	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[3];	
	abyTemp[1] = pTemp02[2];	
	abyTemp[2] = pTemp02[1];	
	abyTemp[3] = pTemp02[0];
	return *((f32*)abyTemp);
}

/*-----------------------------------------------------------------*/
//ENDIAN_BigToLittle_f64 and ENDIAN_LittleToBig_f64 will perform the same functionality

f64     ENDIAN_BigToLittle_f64( f64  fqInValue )
{	
	u8	abyTemp[8]; 
	u8  *pTemp02 = (u8*)&fqInValue;
	
	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[7];	
	abyTemp[1] = pTemp02[6];	
	abyTemp[2] = pTemp02[5];	
	abyTemp[3] = pTemp02[4];
	abyTemp[4] = pTemp02[3];	
	abyTemp[5] = pTemp02[2];	
	abyTemp[6] = pTemp02[1];	
	abyTemp[7] = pTemp02[0];
	return *((f64*)abyTemp);
}
/*-----------------------------------------------------------------*/
f64     ENDIAN_LittleToBig_f64( f64  fqInValue )
{
	u8	abyTemp[8]; 
	u8  *pTemp02 = (u8*)&fqInValue;

	//Didnot use while() or for() for performance efficiency
	abyTemp[0] = pTemp02[7];	
	abyTemp[1] = pTemp02[6];	
	abyTemp[2] = pTemp02[5];	
	abyTemp[3] = pTemp02[4];
	abyTemp[4] = pTemp02[3];	
	abyTemp[5] = pTemp02[2];	
	abyTemp[6] = pTemp02[1];	
	abyTemp[7] = pTemp02[0];
	return *((f64*)abyTemp);
}


/**
* \file     CircularBuffer.c
*
* \brief    
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		5/29/02
*/


/**
++----+++++

*/

#include "x_files.hpp"
#include "CircularBuffer.h"

#define CBUFF_END( buff_data ) ((buff_data)->pBuffer + (buff_data)->dwBufferSize)

CBUFF_HANDLE CBUFF_Initialize( u32 dwSize, u32 dwAlign )
{
    CBUFF_DATA *pBuffData ;

    ASSERT( dwSize >= 0 ) ;
    
    pBuffData = (CBUFF_DATA*)x_malloc( sizeof( CBUFF_DATA ) ) ;
    pBuffData->pBuffer = (u8*)x_malloc( dwSize ) ;
	ASSERT_PTR(pBuffData->pBuffer);
    pBuffData->dwBufferSize = dwSize ;
    pBuffData->dwReadSize = 0 ;
    pBuffData->pReadPosition = pBuffData->pBuffer ;
    pBuffData->pWritePosition = pBuffData->pBuffer ;
    return pBuffData ;
}

void CBUFF_DeInitialize( CBUFF_HANDLE hBuff ) 
{
    CBUFF_DATA *pBuffData = (CBUFF_DATA*)hBuff ;
    x_free( pBuffData->pBuffer ) ;
    x_free( pBuffData ) ;
}

/**
* copies data from hbuff into pbuffer of size at most sdwSize
* returns the number of bytes actually copied
* updates the read pointer and the read size
*/
static u32 CBUFFINTERNAL_CircularRead( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) 
{
    u32 dwLeftSize, dwRightSize ;
    CBUFF_DATA *pBuffData = (CBUFF_DATA*)hBuff ;
    
    //-- make sure we are not trying to read too much data
    dwSize = (dwSize < pBuffData->dwReadSize ) ? dwSize : pBuffData->dwReadSize ;
    
    //-- check if the read size extends past the end of the buffer
    if( (pBuffData->pReadPosition + dwSize) > CBUFF_END( pBuffData ) )
    {
        //-- we need to read it in two parts
        dwRightSize = CBUFF_END(pBuffData) - pBuffData->pReadPosition ;
        dwLeftSize = dwSize - dwRightSize ;

        x_memcpy( pBuffer, pBuffData->pReadPosition, dwRightSize ) ;
        x_memcpy( (void*)(pBuffer+dwRightSize), pBuffData->pBuffer, dwLeftSize ) ;

        //-- update the read position 
        pBuffData->pReadPosition = pBuffData->pBuffer + dwLeftSize ;
    }
    else
    {
        //-- we can read it in one part
        x_memcpy( pBuffer, pBuffData->pReadPosition, dwSize ) ;

        //-- update the read position 
        pBuffData->pReadPosition += dwSize ;
    }

    //-- udpate the read size
    pBuffData->dwReadSize = pBuffData->dwReadSize - dwSize ;
    return dwSize ;    
}

u32 CBUFFINTERNAL_CircularWrite( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwBuffSizeIn, xbool overwrite ) 
{
    s32 sdwAvailableSize ;
    u32 dwRightSize ;
    u32 dwLeftSize ;

    CBUFF_DATA *pBuffData = (CBUFF_DATA*)hBuff ;

    sdwAvailableSize = pBuffData->dwBufferSize - pBuffData->dwReadSize ;

    if( 0 >= sdwAvailableSize && !overwrite )
	{
		//A_DError( "Circular Buffer::Write: Out of available memory.\n" ) ;
        return 0 ;
	}
    
	if( dwBuffSizeIn > (u32)sdwAvailableSize )
    {
		if( !overwrite )
			//A_DError( "Circular Buffer::Write: Out of available memory. Data will be lost\n" ) ;
			dwBuffSizeIn = sdwAvailableSize ;
		else
			//A_DError( "Circular Buffer::Write: User doesnt want to overwrite data\n" ) ;
			return 0;
    }
	

    if( (pBuffData->pWritePosition + dwBuffSizeIn) > CBUFF_END( pBuffData ) )
    {
        //we are going to need to write in two parts
        dwRightSize = CBUFF_END( pBuffData ) - pBuffData->pWritePosition ;
        dwLeftSize = dwBuffSizeIn - dwRightSize ;

        x_memcpy( pBuffData->pWritePosition, pBuffer, dwRightSize ) ;
        x_memcpy( pBuffData->pBuffer, (void*)(pBuffer + dwRightSize), dwLeftSize ) ;
        //-- update the 
        pBuffData->pWritePosition = pBuffData->pBuffer + dwLeftSize ;
        
    }
    else
    {
        //-- we can write it in one part
        x_memcpy( pBuffData->pWritePosition, pBuffer, dwBuffSizeIn ) ;
        pBuffData->pWritePosition += dwBuffSizeIn ;
    }
    
    //-- update the readable size
    pBuffData->dwReadSize += dwBuffSizeIn ;
    return dwBuffSizeIn ;
}

u32 CBUFF_Write( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) 
{
    return CBUFFINTERNAL_CircularWrite( hBuff, pBuffer, dwSize, FALSE ) ;
}

u32 CBUFF_SafeWrite( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) 
{
    return CBUFFINTERNAL_CircularWrite( hBuff, pBuffer, dwSize, TRUE ) ;
}

u32 CBUFF_Read( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) 
{
    return CBUFFINTERNAL_CircularRead( hBuff, pBuffer, dwSize ) ;
}

u32 CBUFF_GetUsedSize( CBUFF_HANDLE hBuff )
{
    CBUFF_DATA *pBuffData = (CBUFF_DATA*)hBuff ;
    return pBuffData->dwReadSize ;
}

u32 CBUFF_GetFreeSize( CBUFF_HANDLE hBuff )
{
    CBUFF_DATA *pBuffData = (CBUFF_DATA*)hBuff ;
    u32 dwAvailableSize = pBuffData->dwBufferSize - pBuffData->dwReadSize ;
    return dwAvailableSize ;
}

void CBUFF_Reset( CBUFF_HANDLE hBuff )
{
    CBUFF_DATA *pBuffData = (CBUFF_DATA*)hBuff ;
    pBuffData->dwReadSize = 0 ;
    pBuffData->pReadPosition = pBuffData->pBuffer ;
    pBuffData->pWritePosition = pBuffData->pBuffer ;
    
#ifdef DEBUG
    x_memset( pBuffData->pBuffer, 3, pBuffData->dwBufferSize ) ;
#endif
}

u32 CBUFF_MoveReadPointer( CBUFF_HANDLE hBuff, u32 dwSize )
{
    u32 dwLeftSize ;
    u32 dwRightSize ;

    CBUFF_DATA *pBuffData = (CBUFF_DATA*)hBuff ;
    if( dwSize > pBuffData->dwReadSize )
        dwSize = pBuffData->dwReadSize;
    
    if( (pBuffData->pReadPosition + dwSize) > CBUFF_END( pBuffData ) )
    {
        //-- we need to read it in two parts
        dwRightSize = CBUFF_END(pBuffData) - pBuffData->pReadPosition ;
        dwLeftSize = dwSize - dwRightSize ;

        //-- update the read position 
        pBuffData->pReadPosition = pBuffData->pBuffer + dwLeftSize ;
    }
    else
    {
        pBuffData->pReadPosition += dwSize ;
    }
    pBuffData->dwReadSize -= dwSize ;

    return dwSize ;
    
    

}
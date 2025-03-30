/**
* \file     CircularBuffer.h
*
* \brief    
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		5/29/02
*/


#ifndef __CIRCULARBUFFER_H__
#define __CIRCULARBUFFER_H__
/*-----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------*/

/**
* Internal structure
*/
typedef struct _tagCBUFF_DATA
{
    u32     dwReadSize ;
    u32     dwBufferSize ;
    u8      *pBuffer ;
    u8      *pReadPosition ;
    u8      *pWritePosition ;
    
} CBUFF_DATA ;
typedef CBUFF_DATA* CBUFF_HANDLE  ;

/**
*   \fn CBUFF_HANDLE CBUFF_Initialize( u32 dwSize, u32 dwAlign ) ; 
*   \brief  Initializes a circular buffer of size dwSize
*   @param  dwSize the size of the circular buffer
*   @param  dwAlign not implemented
*   @return CBUFF_HANDLE
*			@arg \c 0 The function was successfull
* 
*   \b No more details
*/
CBUFF_HANDLE CBUFF_Initialize( u32 dwSize, u32 dwAlign ) ;


/**
*   \fn void CBUFF_DeInitialize( CBUFF_HANDLE hBuff ) ; 
*   \brief  Free's the circular buffer
*   @param  hBuff
*   @return void
* 
*   \b No more details
*/
void CBUFF_DeInitialize( CBUFF_HANDLE hBuff ) ;


/**
*   \fn u32 CBUFF_Read( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) ; 
*   \brief  Reads data out of the circular buffer and into pBuffer
*   @param  hBuff
*   @param  *pBuffer
*   @param  dwSize
*   @return u32 The number of bytes actually copied
*			@arg \c 0 The function was successfull
* 
*   \b Reads up to dwSize bytes into pBuffer.  
*/
u32 CBUFF_Read( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) ;

/**
*   \fn u32 CBUFF_Write( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) ; 
*   \brief  Writes data from pBuffer into the circular buffer
*   @param  hBuff
*   @param  *pBuffer
*   @param  dwSize
*   @return u32
*			@arg \c 0 The function was successfull
* 
*   \b if dwSize is greater then the available memory in this buffer the behavior is indefined.
*/
u32 CBUFF_Write( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) ;


/**
*   \fn u32 CBUFF_SafeWrite( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) ; 
*   \brief  Writes data from source buffer to circ buffer
*	\remark Does not overwrite existing data
*   @param  hBuff
*   @param  *pBuffer
*   @param  dwSize
*   @return u32
*			@arg \c 0 The function was successfull
*/
u32 CBUFF_SafeWrite( CBUFF_HANDLE hBuff, u8 *pBuffer, u32 dwSize ) ;


/**
*   \fn u32 CBUFF_GetUsedSize( CBUFF_HANDLE hBuff ) ; 
*   \brief  Returns the number of bytes that are currently readable
*   @param  hBuff
*   @return u32
*			@arg \c 0 The function was successfull
* 
*   \b No more details
*/
u32 CBUFF_GetUsedSize( CBUFF_HANDLE hBuff ) ;

/**
*   \fn u32 CBUFF_GetFreeSize( CBUFF_HANDLE hBuff ) ; 
*   \brief  Returns the number of bytes available to write to
*   @param  hBuff
*   @return u32
*			@arg \c 0 The function was successfull
* 
*   \b No more details
*/
u32 CBUFF_GetFreeSize( CBUFF_HANDLE hBuff ) ;

/**
*   \fn void CBUFF_Reset( CBUFF_HANDLE hBuff ) ; 
*   \brief  Resets hBuff so there is no data and all pointers and sizes are reset.
*           Buff size is still what it was initialized with
*   @param  hBuff
*   @return void
* 
*   \b No more details
*/
void CBUFF_Reset( CBUFF_HANDLE hBuff ) ;

/**
*   \fn u32 CBUFF_MoveReadPointer( CBUFF_HANDLE hBuff, u32 dwSize ) 
*   \brief  Moves the read ponter forward by sdwSize bytes
*   @param  hBuff
*   @param  dwSize
*   @return u32
*			@arg \c 0 The function was successfull
* 
*   \b Will not move past the end of valid data
*/
u32 CBUFF_MoveReadPointer( CBUFF_HANDLE hBuff, u32 dwSize ) ;
/**
* Errors
*/



/*-----------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-----------------------------------------------------------------*/



#endif //__CIRCULARBUFFER_H__
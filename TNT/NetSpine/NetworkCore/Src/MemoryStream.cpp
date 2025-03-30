/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*
*		Filename:	MemoryStream.c
*		Date:		11/22/00
*
*		Desc:		Implementation of the memory stream functions
*
*
*		Revisions History:
*
*
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


#include "x_files.hpp"
#include "EndianHelper.h"
#include "MemoryStream.h"

//===================================================================
// NOTE TO MAINTAINER!!!!
// WARNING: PLEASE DO NOT RENAME ANY "pStream" ANYWHERE IN THIS FILE
// THE "pStream"'S IN THE FUNCTION IMPLEMENTATION MUST MATCH WITH THE pStream'S IN THE MACROS BELOW 
//===================================================================


//===================================================================
// Macro used for determining data size within the memory stream
#define XSTREAM_TAIL_MINUS_HEAD     (s32)( pStream->byIsBufferFull ? pStream->sdwMaxStreamSize : (pStream->pbyTail - pStream->pbyHead) )
#define XSTREAM_STREAMED_DATA_SIZE  (s32)( (0 > XSTREAM_TAIL_MINUS_HEAD) ? (pStream->sdwMaxStreamSize + XSTREAM_TAIL_MINUS_HEAD) : XSTREAM_TAIL_MINUS_HEAD )
#define XSTREAM_AVAILABLE_SIZE      (s32)( (0 > XSTREAM_TAIL_MINUS_HEAD) ? (0 - XSTREAM_TAIL_MINUS_HEAD) : (pStream->sdwMaxStreamSize - XSTREAM_TAIL_MINUS_HEAD) )
#define XSTREAM_SIZE_AT_END( p )    (s32)( pStream->pbyBufPtr + pStream->sdwMaxStreamSize - (p) )
#define XSTREAM_SIZE_AT_BEGIN( p )  (s32)( (p) - pStream->pbyBufPtr )

// Macro used for determining whether appending will overwrite the memory stream 
#define WILL_XSTREAM_STREAM_OVERFLOW( x )   ( (x) > XSTREAM_AVAILABLE_SIZE )

// Macro used for determining whether extracting will remove beyond the capacity of the memory stream 
#define WILL_XSTREAM_STREAM_UNDERFLOW( x )  ( (x) > XSTREAM_STREAMED_DATA_SIZE )

// Macro used for determining whether moving a pointer will exceed the end of the memory stream buffer
#define WILL_XSTREAM_PTR_WRAPAROUND_AT_END( p, x )    ( (x) > XSTREAM_SIZE_AT_END(p) )

// Macro used for determining whether moving a pointer will exceed the beginning of the memory stream buffer
#define WILL_XSTREAM_PTR_WRAPAROUND_AT_BEGIN( p, x )  ( (x) > XSTREAM_SIZE_AT_BEGIN(p) )
//===================================================================


#ifdef X_DEBUG
#define XSTREAM_BYTE_STREAM_INITCODE 6893642
#endif

#define XSTREAM_NONE_OPCODE             (u8)0x00
#define XSTREAM_APPEND_OPCODE           (u8)0x01
#define XSTREAM_PREFIX_OPCODE           (u8)0x02
#define XSTREAM_EXTRACT_OPCODE          (u8)0x04
#define XSTREAM_TRUNCATE_OPCODE         (u8)0x08
#define XSTREAM_SEEK_HEAD_OPCODE        (u8)0x10
#define XSTREAM_SEEK_TAIL_OPCODE        (u8)0x11

//===================================================================
// Function : XSTREAM_Init
// Desc     : Assigns a buffer to be used to stream data into. 
//			  NOTE: In Debug mode, all calls to stream buffer access functions will fail
//			  if stream buffer has not been initialized
// In		: pStream:	A pointer to an allocated x_mem_stream structure
//			  pbyBuffer: The buffer to be used as the stream buffer
//			  dwMaxStreamSize:  the maximum size 
// Returns	: 0 for success, 1 if size of buffer is invalid
//===================================================================
u32 XSTREAM_Init	( x_mem_stream *pStream, u8 *pbyBuffer, s32 sdwMaxStreamSize, s32 sdwSizeOfValidData )
{
    // pStream must be allocated before calling XSTREAM_Init
    ASSERTS_PTR( pStream, "XSTREAM_Init" );
    ASSERTS_PTR( pbyBuffer, "XSTREAM_Init" );
    ASSERTS( sdwMaxStreamSize > 0, "XSTREAM_Init" );
	ASSERTS( sdwSizeOfValidData <= sdwMaxStreamSize, "XSTREAM_Init" );
// by a fluke this could be equal to the init code ~ DM //  ASSERTS( pStream->dwInitCode != XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Init"  );
    
	// Must initialize buffer with some max size
	// and valid data must be at least 0
	if ( ( sdwMaxStreamSize <= 0 ) || ( sdwSizeOfValidData < 0 ) )
		return 1;
    
	// Valid data cannot be greater than max size
	if ( sdwSizeOfValidData > sdwMaxStreamSize )
		sdwSizeOfValidData = sdwMaxStreamSize;

    // Assign the stream buffer
    pStream->pbyBufPtr = pbyBuffer;
    
#ifdef X_DEBUG
    // Activate the x_mem_stream struct for use in debug mode. Stream functions will
    // fail if x_mem_stream does not have a valid initialization code
    pStream->dwInitCode = XSTREAM_BYTE_STREAM_INITCODE;
#endif
    
    // Initialize x_mem_stream members
    pStream->pbyHead = pStream->pbyBufPtr;

    // Check for full buffer
    if ( sdwSizeOfValidData == sdwMaxStreamSize )
    {
        pStream->pbyTail = pStream->pbyBufPtr;
        pStream->byIsBufferFull = 1;
    }
    else
    {
        pStream->pbyTail = pStream->pbyBufPtr + sdwSizeOfValidData;
        pStream->byIsBufferFull = 0;
    }


    pStream->sdwMaxStreamSize = sdwMaxStreamSize;
    x_memset( &pStream->PrevOp, 0, sizeof(LAST_XSTREAM_STATE) );
    
    return 0;
}


//===================================================================
// Function : XSTREAM_DeInit
// Desc     : Clears the x_mem_stream structure for reinitialization
// In		: pStream: A pointer to an initialized x_mem_stream structure
//===================================================================
void XSTREAM_DeInit		( x_mem_stream *pStream )
{
    ASSERTS_PTR( pStream, "XSTREAM_DeInit" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_DeInit"  );
    
    // zero out the members of the x_mem_stream struct
    x_memset( pStream, 0, sizeof(x_mem_stream) );
}

//===================================================================
// Function : XSTREAM_ReuseStream
// Desc     : Resets the stream buffer for reuse. Maintains its internal buffer
// In		: pStream: A pointer to an initialized x_mem_stream structure
//===================================================================
void XSTREAM_ReuseStream	( x_mem_stream *pStream )
{
    ASSERTS_PTR( pStream, "XSTREAM_ReuseStream" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_ReuseStream"  );
    
    // resets the read/write pointers to the beginning of the stream
    pStream->pbyHead = pStream->pbyBufPtr;
    pStream->pbyTail = pStream->pbyBufPtr;
    pStream->byIsBufferFull = 0;
    x_memset( &pStream->PrevOp, 0, sizeof(LAST_XSTREAM_STATE) );
}

//===================================================================
// Function : XSTREAM_GetMaxStreamDataSize
// Desc     : Used to inquire the maximum size of the stream buffer
// In		: pStream: A pointer to an initialized x_mem_stream structure
// Returns	: The size of the streamed data within the stream buffer
//===================================================================
s32	XSTREAM_GetMaxStreamSize	( x_mem_stream *pStream )
{
    ASSERTS_PTR( pStream, "XSTREAM_GetMaxStream" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_GetMaxStream" );
    
    return ( pStream->sdwMaxStreamSize );
}

//===================================================================
// Function : XSTREAM_GetStreamDataSize
// Desc     : Used to inquire the size of the streamed data within the stream buffer
// In		: pStream: A pointer to an initialized x_mem_stream structure
// Returns	: The size of the streamed data within the stream buffer
//===================================================================
s32 XSTREAM_GetStreamDataSize ( x_mem_stream *pStream )
{
    ASSERTS_PTR( pStream, "XSTREAM_GetStreamSize" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_GetStreamSize" );
    
    return XSTREAM_STREAMED_DATA_SIZE;
}

//===================================================================
// Function : DefragmentByteStream
// Static function to defragment the streamed data either within the streamed buffer
// or to an optional stream buffer
//===================================================================
static void DefragmentByteStream ( x_mem_stream *pStream, x_mem_stream *pOptStream )
{
    s32 sdwTotalDataSize = XSTREAM_STREAMED_DATA_SIZE
        , sdwDataSizeAtBegin = XSTREAM_SIZE_AT_BEGIN( pStream->pbyTail )
        , sdwDataSizeAtEnd = XSTREAM_SIZE_AT_END( pStream->pbyHead );

    // Because we are moving data, if any data was previously extracted or truncated
    // it will no longer be accessible. (Prefixed or appended data is considered part of
    // the valid data and can be undone) So if the last operation was a truncation or 
    // extraction, it cannot be undone
    if (    ( XSTREAM_TRUNCATE_OPCODE == pStream->PrevOp.byOpID )
        ||  ( XSTREAM_EXTRACT_OPCODE == pStream->PrevOp.byOpID )
        )
        x_memset( &pStream->PrevOp, 0, sizeof(LAST_XSTREAM_STATE) );

    // if streamed data does not wraparound, move data to the beginning
    // NOTE: the XSTREAM_TAIL_MINUS_HEAD macro may return a positive value even if tail equals head
    // (if the buffer is full), so we have to check for a full buffer!
    if ( ((XSTREAM_TAIL_MINUS_HEAD > 0) && (!pStream->byIsBufferFull))
        || (sdwDataSizeAtBegin == 0)
        )
    {
        // if we are defragmenting within this stream buffer...
        if ( !pOptStream )
        {
            x_memmove( pStream->pbyBufPtr, pStream->pbyHead, sdwTotalDataSize );
        
            // adjust read/write pointers
            pStream->pbyHead = pStream->pbyBufPtr;
            pStream->pbyTail = pStream->pbyBufPtr + sdwTotalDataSize;
        }
        else
            x_memcpy( pOptStream->pbyBufPtr, pStream->pbyHead, sdwTotalDataSize );

        return;
    }

    // if streamed data does wraparound, 2 memory moves are required
    if ( !pOptStream )
    {
        u8  *pTempBuf;               

        // try to shift the defragmented data within the stream buffer without the 
        // aid of a secondary buffer but only if there is room to do so...
        if ( pStream->pbyBufPtr + sdwTotalDataSize <= pStream->pbyHead )
        {
            // first, shift the data at the beginning of the stream buffer
            x_memmove( pStream->pbyBufPtr + sdwDataSizeAtEnd
                     , pStream->pbyBufPtr
                     , sdwDataSizeAtBegin
                     );
            
            // next, shift the data at the end of the stream buffer to the beginning
            x_memmove( pStream->pbyBufPtr
                     , pStream->pbyHead
                     , sdwDataSizeAtEnd
                     );

            // adjust read/write pointers
            pStream->pbyHead = pStream->pbyBufPtr;
            pStream->pbyTail = pStream->pbyBufPtr + sdwTotalDataSize;
            return;
       }

        // copy the smaller of the two fragments into the temp buffer,
        // shift the larger block within the stream buffer,
        // then copy the temp buffer into the appropriate place in the stream buffer,
        // and set the head/tail pointers accordingly
        if ( sdwDataSizeAtEnd < sdwDataSizeAtBegin )
        {
            pTempBuf = (u8 *)x_malloc( (sdwDataSizeAtEnd + 1) * sizeof(u8) );
            x_memcpy( pTempBuf, pStream->pbyHead, sdwDataSizeAtEnd );

            // first move the wraparound data from the beginning to make room for data at the end
            x_memmove( pStream->pbyBufPtr + sdwDataSizeAtEnd
                     , pStream->pbyBufPtr
                     , sdwDataSizeAtBegin
                     );

            // now move the data from the tempbuf into the beginning
            x_memcpy( pStream->pbyBufPtr
                     , pTempBuf
                     , sdwDataSizeAtEnd 
                     );
        }
        else
        {
            pTempBuf = (u8 *)x_malloc( (sdwDataSizeAtBegin + 1) * sizeof(u8) );
            x_memcpy( pTempBuf, pStream->pbyBufPtr, sdwDataSizeAtBegin );

            // first move the end data into the beginning
            x_memmove( pStream->pbyBufPtr
                     , pStream->pbyHead
                     , sdwDataSizeAtEnd 
                     );

            // now move the data from the tempbuf into the stream buffer
            x_memcpy( pStream->pbyBufPtr + sdwDataSizeAtEnd
                     , pTempBuf
                     , sdwDataSizeAtBegin
                     );
        }

        x_free( pTempBuf );

        // adjust read/write pointers
        pStream->pbyHead = pStream->pbyBufPtr;
        pStream->pbyTail = pStream->pbyBufPtr + sdwTotalDataSize;
    }
    else
    {
        // first move the end data into the beginning
        x_memcpy( pOptStream->pbyBufPtr
                 , pStream->pbyHead
                 , sdwDataSizeAtEnd 
                 );

        // now move the rest of the data into the stream buffer
        x_memcpy( pOptStream->pbyBufPtr + sdwDataSizeAtEnd
                 , pStream->pbyBufPtr
                 , sdwDataSizeAtBegin
                 );

    }

}

//===================================================================
// Function : XSTREAM_GetStreamPtr
// Desc     : Used to acquire a pointer to the beginning of the streamed data
// In		: pStream: A pointer to an initialized x_mem_stream structure
// Returns	: A pointer to the beginning of the streamed data buffer
// NOTE		: This function has the added cost of first defragmenting the streamed data
//			  Use judiciously!!
//===================================================================
u8 *XSTREAM_GetStreamPtr ( x_mem_stream *pStream )
{
    ASSERTS_PTR( pStream, "XSTREAM_GetStreamPtr" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_GetStreamPtr" );
    
    // if data is fragmented, defragment
    if ( pStream->pbyHead != pStream->pbyBufPtr )
        DefragmentByteStream( pStream, NULL );

    return ( pStream->pbyBufPtr );
}

//===================================================================
// Function : SetValidDataPointer 
//===================================================================
#define     SET_HEAD_POINTER    x_true
#define     SET_TAIL_POINTER    x_false
static u32 SetValidDataPointer ( x_mem_stream *pStream, xbool bSetHeadPtr, s32 sdwOffset, u32 dwWhatPosition )
{
    u8 *pbyNewLocation
        , *pbyStart = pStream->pbyBufPtr
        , *pbyEnd = pStream->pbyBufPtr + pStream->sdwMaxStreamSize - sizeof(u8)
        , *pbyPtr = ( (SET_HEAD_POINTER == bSetHeadPtr) ? pStream->pbyHead : pStream->pbyTail );

    s32 sdwDirection; 

    // find out what location is requested
    switch ( dwWhatPosition )
    {
    case XSTREAM_SEEK_SET:  
        {
            pbyNewLocation =  pbyStart + sdwOffset; 
            sdwDirection =  pbyNewLocation - pbyPtr; 
            break;
        }
    case XSTREAM_SEEK_END:  
        {
            pbyNewLocation =  pbyEnd + sdwOffset;   
            sdwDirection =  pbyNewLocation - pbyPtr;
            break;
        }
    case XSTREAM_SEEK_CUR:  
        {
            // allow wraparound when using XSTREAM_SEEK_CUR
            pbyNewLocation =  pbyPtr + sdwOffset;
            
            // we don't care if the location wraps around at this point
            // only the direction is important
            sdwDirection =  pbyNewLocation - pbyPtr;

            // check to see if the location is valid
            if ( (pbyNewLocation >= pbyStart) && (pbyNewLocation <= pbyEnd) )
                break;

            // otherwise, calculate the wraparound location
            if ( pbyNewLocation > pbyEnd )
                pbyNewLocation = pbyStart + ((pbyNewLocation - pbyStart) % pStream->sdwMaxStreamSize);
            else
                pbyNewLocation = pbyEnd - ((pbyEnd - pbyNewLocation) % pStream->sdwMaxStreamSize);

            break;
        }
    default:                return 1;
    }
    
    // the new requested location is exactly the same as the current one! we're done
    if ( 0 == sdwDirection )
        return 0;

    // check to see if the location is valid
    if ( (pbyNewLocation < pbyStart) || (pbyNewLocation > pbyEnd) )
        return 1;

    // we are now ready to commit our changes    
    if ( SET_HEAD_POINTER == bSetHeadPtr )
    {
        // first, set the pointer
        pStream->pbyHead        = pbyNewLocation; 

        // set the opcode to undo this operation
        pStream->PrevOp.byOpID  = XSTREAM_SEEK_HEAD_OPCODE;
    }
    else
    {
        // first, set the pointer
        pStream->pbyTail = pbyNewLocation; 

        // set the opcode to undo this operation
        pStream->PrevOp.byOpID  = XSTREAM_SEEK_TAIL_OPCODE;
    }

    // if the head and tail pointers overlap, we need to resolve whether the 
    // buffer is considered full or empty. 
    if ( pStream->pbyHead == pStream->pbyTail )
    {
        // Use the pre-calculated direction to resolve buffer status
        // if begin position was moved in the negative direction, then buffer data MUST have been "added"
        // if begin position was moved in the positive direction, then buffer data MUST have been "lost"
        // if end position was moved in the negative direction, then buffer data MUST have been "lost"
        // if end position was moved in the positive direction, then buffer data MUST have been "added"
        if ( sdwDirection < 0 )
            pStream->byIsBufferFull = ( (SET_HEAD_POINTER == bSetHeadPtr) ? 1 : 0 );
        else
            pStream->byIsBufferFull = ( (SET_HEAD_POINTER == bSetHeadPtr) ? 0 : 1 );
    }
    else
        pStream->byIsBufferFull = 0;

    pStream->PrevOp.sdwOpSize = sdwDirection;

    return 0;
}

//===================================================================
// Function : XSTREAM_SetValidDataBegin
// Desc     : Used to set the given position as the beginning of valid data
// In		: pStream:	A pointer to an allocated x_mem_stream structure
//			  sdwOffset:  The desired offset from dwWhatPosition
//            dwWhatPosition: 
//                  XSTREAM_SEEK_SET - Represents the beginning of the buffer
//                      sdwOffset   MUST be a positive value
//                                  MUST strictly be in the range btwn 0 - (buffer max size-1)
//                  XSTREAM_SEEK_CUR - Represents the current beginning of valid data
//                      sdwOffset   MAY be a positive OR negative value
//                      NOTE: any value is accepted and pointer will be overlapped to 
//                      the appropriate position within the buffer
//                  XSTREAM_SEEK_END - Represents the end of the buffer
//                      sdwOffset   MUST be a negative value
//                                  MUST strictly be in the range btwn 0 - (buffer max size-1)
// Returns	: 1 indicates that the beginning of valid data could NOT be set
//			  0 indicates success
//===================================================================
u32 XSTREAM_SetValidDataBegin   ( x_mem_stream *pStream, s32 sdwOffset, u32 dwWhatPosition )
{
    ASSERTS_PTR( pStream, "XSTREAM_SetValidDataBegin" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_SetValidDataBegin" );
 
    return ( SetValidDataPointer( pStream, SET_HEAD_POINTER, sdwOffset, dwWhatPosition ) );
}

//===================================================================
// Function : XSTREAM_SetValidDataEnd
// Desc     : Used to set the given position as the end of valid data
// In		: pStream:	A pointer to an allocated x_mem_stream structure
//            dwWhatPosition: 
//                  XSTREAM_SEEK_SET - Represents the beginning of the buffer
//                      sdwOffset   MUST be a positive value
//                                  MUST strictly be in the range btwn 0 - (buffer max size-1)
//                  XSTREAM_SEEK_CUR - Represents the current beginning of valid data
//                      sdwOffset   MAY be a positive OR negative value
//                      NOTE: any value is accepted and pointer will be overlapped to 
//                      the appropriate position within the buffer
//                  XSTREAM_SEEK_END - Represents the end of the buffer
//                      sdwOffset   MUST be a negative value
//                                  MUST strictly be in the range btwn 0 - (buffer max size-1)
// Returns	: 1 indicates that the end of valid data could NOT be set
//			  0 indicates success
//===================================================================
u32 XSTREAM_SetValidDataEnd   ( x_mem_stream *pStream, s32 sdwOffset, u32 dwWhatPosition )
{
    ASSERTS_PTR( pStream, "XSTREAM_SetValidDataEnd" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_SetValidDataEnd" );

    return ( SetValidDataPointer( pStream, SET_TAIL_POINTER, sdwOffset, dwWhatPosition ) );
}

//===================================================================
// Function : XSTREAM_CanExpandSize
// Desc     : Used to determine if size to be added will overflow the stream buffer 
// In		: pStream:	A pointer to an allocated x_mem_stream structure
//			  sdwSizeToExpand:  The desired size to be appended in bytes
// Returns	: 1 indicates that it is safe to append the desired size, 0 indicates that a
//			  subsequent call to any append function with that size or greater will fail 
//===================================================================
u32 XSTREAM_CanExpandSize ( x_mem_stream *pStream, s32 sdwSizeToExpand )
{
    ASSERTS_PTR( pStream, "XSTREAM_CanExpandSize" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_CanExpandSize" );
    ASSERTS( sdwSizeToExpand >= 0, "XSTREAM_CanExpandSize" );
    
    return !WILL_XSTREAM_STREAM_OVERFLOW( sdwSizeToExpand );
}

//===================================================================
// Function : XSTREAM_CanShrinkSize
// Desc     : Used to determine if a size can be removed from the stream buffer 
// In		: pStream:	A pointer to an allocated x_mem_stream structure
//			  sdwSizeToShrink:  The desired size to be extracted in bytes
// Returns	: 1 indicates that it is safe to extract the desired size, 0 indicates that a
//			  subsequent call to any extract function with that size or greater will fail 
//===================================================================
u32 XSTREAM_CanShrinkSize ( x_mem_stream *pStream, s32 sdwSizeToShrink )
{
    ASSERTS_PTR( pStream, "XSTREAM_CanShrinkSize" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_CanShrinkSize" );
    ASSERTS( sdwSizeToShrink >= 0, "XSTREAM_CanShrinkSize" );
    
    return !WILL_XSTREAM_STREAM_UNDERFLOW( sdwSizeToShrink );
}

//===================================================================
// Static function used by all append functions to write into the stream
//===================================================================
static u32 AppendXToByteStream ( x_mem_stream *pStream, void *pvData, s32 sdwSizeOfData, u8 byCopyData )
{
    // if there is no room to append...
    if ( WILL_XSTREAM_STREAM_OVERFLOW(sdwSizeOfData) )
        return 1;

    pStream->PrevOp.byOpID = XSTREAM_APPEND_OPCODE;
    pStream->PrevOp.sdwOpSize = sdwSizeOfData;

    // if appending will completely fill the buffer, set the "isfull" flag
    if ( sdwSizeOfData == XSTREAM_AVAILABLE_SIZE )
        pStream->byIsBufferFull = 1;

    // if there is no longer any data in the stream buffer, reset read/write pointers
    if ( !XSTREAM_STREAMED_DATA_SIZE )
    {
        ASSERTS( pStream->pbyHead == pStream->pbyTail, "AppendXFromByteStream" );
        pStream->pbyHead = pStream->pbyBufPtr;
        pStream->pbyTail = pStream->pbyBufPtr;
        goto APPENDX_COPY_DATA_DIRECTLY;
    }

    // if appending causes the memory stream buffer to wraparound...
    if ( WILL_XSTREAM_PTR_WRAPAROUND_AT_END( pStream->pbyTail, sdwSizeOfData) )
    {
        s32 sdwCopiedBytes = XSTREAM_SIZE_AT_END( pStream->pbyTail );

        if ( byCopyData )
        {
            // copy as much of the data as possible up to the end of the stream buffer
            if ( pvData != pStream->pbyTail )
                x_memcpy( pStream->pbyTail, pvData, sdwCopiedBytes );

            // copy the rest of the data into the beginning of the stream buffer
            x_memcpy( pStream->pbyBufPtr, (u8 *)pvData + sdwCopiedBytes, sdwSizeOfData - sdwCopiedBytes );
        }

        pStream->pbyTail = pStream->pbyBufPtr + sdwSizeOfData - sdwCopiedBytes;
        return 0;
    }
    
APPENDX_COPY_DATA_DIRECTLY:
    // otherwise copy data into the stream
    if ( (byCopyData) && (pvData != pStream->pbyTail) )
        x_memcpy( pStream->pbyTail, pvData, sdwSizeOfData );
        
    // advance the next available byte pointer
    // if write pointer will be advanced to the end of the stream buffer,
    // set write pointer to the beginning of the buffer
    pStream->pbyTail = 
        ( !XSTREAM_SIZE_AT_END( pStream->pbyTail + sdwSizeOfData ) ? pStream->pbyBufPtr : pStream->pbyTail + sdwSizeOfData );

    return 0;
}

//===================================================================
// Static function used by all extract functions to write out of the stream
//===================================================================
static u32 ExtractXFromByteStream ( x_mem_stream *pStream, void *pvData, s32 sdwSizeOfData, u8 byCopyData )
{
    // if there are insufficient bytes to extract...
    if ( WILL_XSTREAM_STREAM_UNDERFLOW(sdwSizeOfData) )
        return 1;
    
    pStream->PrevOp.byOpID = XSTREAM_EXTRACT_OPCODE;
    pStream->PrevOp.sdwOpSize = sdwSizeOfData;

    // we will extract data, so this is always true
    pStream->byIsBufferFull = 0;

    // if extracting causes the memory stream buffer to wraparound...
    if ( WILL_XSTREAM_PTR_WRAPAROUND_AT_END( pStream->pbyHead, sdwSizeOfData) )
    {
        s32 sdwCopiedBytes = XSTREAM_SIZE_AT_END( pStream->pbyHead );

        if ( byCopyData )
        {
            // copy as much of the data as possible up to the end of the stream buffer
            x_memcpy( pvData, pStream->pbyHead, sdwCopiedBytes );

            // copy the rest of the data from the beginning of the stream buffer
            x_memcpy( (u8 *)pvData + sdwCopiedBytes, pStream->pbyBufPtr, sdwSizeOfData - sdwCopiedBytes );
        }

        pStream->pbyHead = pStream->pbyBufPtr + sdwSizeOfData - sdwCopiedBytes;
        return 0;
    }
    
    // otherwise retract the data end pointer and copy data into the requested storage 
    if ( byCopyData )
        x_memcpy( pvData, pStream->pbyHead, sdwSizeOfData );

    // advance the next available byte pointer
    // if read pointer will be advanced to the end of the stream buffer,
    // set read pointer to the beginning of the buffer
    pStream->pbyHead = 
        ( !XSTREAM_SIZE_AT_END( pStream->pbyHead + sdwSizeOfData ) ? pStream->pbyBufPtr : pStream->pbyHead + sdwSizeOfData );

    return 0;
}

//===================================================================
// Static function used by all truncate functions to write out of the stream
//===================================================================
static u32 TruncateXFromByteStream ( x_mem_stream *pStream, void *pvData, s32 sdwSizeOfData, u8 byCopyData )
{
	// if there are insufficient bytes to extract...
	if ( WILL_XSTREAM_STREAM_UNDERFLOW(sdwSizeOfData) )
		return 1;

    pStream->PrevOp.byOpID = XSTREAM_TRUNCATE_OPCODE;
    pStream->PrevOp.sdwOpSize = sdwSizeOfData;

    // we will extract data, so this is always true
    pStream->byIsBufferFull = 0;

    // if extracting causes the memory stream buffer to wraparound...
    if ( WILL_XSTREAM_PTR_WRAPAROUND_AT_BEGIN( pStream->pbyTail, sdwSizeOfData) )
    {
        s32 sdwCopiedBytes = XSTREAM_SIZE_AT_BEGIN( pStream->pbyTail );

        if ( byCopyData )
        {
            // copy the partial data at end of stream buffer to the beginning of the return buffer
            x_memcpy( pvData
                    , pStream->pbyBufPtr + pStream->sdwMaxStreamSize - (sdwSizeOfData - sdwCopiedBytes)
                    , (sdwSizeOfData - sdwCopiedBytes) );

            // copy the rest of the data into the appropriate place in the return buffer
            x_memcpy( (u8 *)pvData + (sdwSizeOfData - sdwCopiedBytes), pStream->pbyBufPtr, sdwCopiedBytes );
        }

        pStream->pbyTail = pStream->pbyBufPtr + pStream->sdwMaxStreamSize - (sdwSizeOfData - sdwCopiedBytes);

	    return 0;
    }

	// otherwise retract the data end pointer and copy data into the requested storage 
	pStream->pbyTail -= sdwSizeOfData;

    if ( byCopyData )
	    x_memcpy( pvData, pStream->pbyTail, sdwSizeOfData );

	return 0;
}

//===================================================================
// Static function used by all prefix functions to write into the stream
//===================================================================
static u32 PrefixXToByteStream ( x_mem_stream *pStream, void *pvData, s32 sdwSizeOfData, u8 byCopyData )
{
    // if there is no room to prefix...
    if ( WILL_XSTREAM_STREAM_OVERFLOW(sdwSizeOfData) )
        return 1;

    pStream->PrevOp.byOpID = XSTREAM_PREFIX_OPCODE;
    pStream->PrevOp.sdwOpSize = sdwSizeOfData;

    // if prefixing will completely fill the buffer, set the "isfull" flag
    if ( sdwSizeOfData == XSTREAM_AVAILABLE_SIZE )
        pStream->byIsBufferFull = 1;

    // if there is no longer any data in the stream buffer, reset read/write pointers
    if ( !XSTREAM_STREAMED_DATA_SIZE )
    {
        ASSERTS( pStream->pbyHead == pStream->pbyTail, "PrefixXFromByteStream" );
        pStream->pbyHead = pStream->pbyBufPtr + sdwSizeOfData;
        pStream->pbyTail = pStream->pbyBufPtr + sdwSizeOfData;
        goto PREFIXX_COPY_DATA_DIRECTLY;
    }

    // if prefixing causes the memory stream buffer to wraparound...
    if ( WILL_XSTREAM_PTR_WRAPAROUND_AT_BEGIN( pStream->pbyHead, sdwSizeOfData) )
    {
        s32 sdwCopiedBytes = XSTREAM_SIZE_AT_BEGIN( pStream->pbyHead );

        if ( byCopyData )
        {
            // copy the partial data at the beginning of input buffer to the appropiate place in the stream buffer
            if ( pvData != pStream->pbyBufPtr + pStream->sdwMaxStreamSize - (sdwSizeOfData - sdwCopiedBytes) )
                x_memcpy( pStream->pbyBufPtr + pStream->sdwMaxStreamSize - (sdwSizeOfData - sdwCopiedBytes)
                        , pvData
                        , (sdwSizeOfData - sdwCopiedBytes) );

            // copy the rest of the data into the stream buffer
            x_memcpy( pStream->pbyBufPtr, (u8 *)pvData + (sdwSizeOfData - sdwCopiedBytes), sdwCopiedBytes );
        }

        pStream->pbyHead = pStream->pbyBufPtr + pStream->sdwMaxStreamSize - (sdwSizeOfData - sdwCopiedBytes);
        
        return 0;
    }

PREFIXX_COPY_DATA_DIRECTLY:
	// otherwise retract the data begin pointer and copy requested data
	pStream->pbyHead -= sdwSizeOfData;

    if ( (byCopyData) && (pvData != pStream->pbyHead) )
    	x_memcpy( pStream->pbyHead, pvData, sdwSizeOfData );

	return 0;
}

//===================================================================
// Function : XSTREAM_Memcpy
// Desc     : Copies data from a buffer into the memory stream. NOTE: Any previous data
//			  in the destination stream buffer will be lost				
// In		: pStream: A pointer to an initialized x_mem_stream structure
//			  pSrcData: A pointer to the buffer that will be copied into the 
//						stream buffer
//			  dwSizeOfSrcData: The size of the source buffer
// Returns  : 0 for success, 1 if copying will cause the memory stream to
//			  overflow
//===================================================================
u32	XSTREAM_Memcpy ( x_mem_stream *pStream, void *pSrcData, s32 sdwSizeOfSrcData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Memcpy" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Memcpy" );
    ASSERTS_PTR( pSrcData, "XSTREAM_Memcpy" );
    ASSERTS( sdwSizeOfSrcData >= 0, "XSTREAM_Memcpy" );
    
    // lose any previous data
    XSTREAM_ReuseStream( pStream );
    return ( AppendXToByteStream(pStream, pSrcData, sdwSizeOfSrcData, 1) );
}

//===================================================================
// Function : XSTREAM_XSTREAMMemcpy
// Desc     : Makes an exact copy of the source stream. NOTE: Any previous data
//			  in the destination stream buffer will be lost. 				
// In		: pStream: A pointer to the initialized destination x_mem_stream structure
//			  pSrcStream: A pointer to the initialized source x_mem_stream structure
// Returns  : 0 for success, 1 if there is no room in the destination stream buffer to copy data
//===================================================================
u32	XSTREAM_XSTREAMMemcpy	( x_mem_stream *pDestStream, x_mem_stream *pStream )
{
    ASSERTS_PTR( pStream, "XSTREAM_XSTREAMMemcpy" );
    ASSERTS_PTR( pDestStream, "XSTREAM_XSTREAMMemcpy" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_XSTREAMMemcpy" );
    ASSERTS( pDestStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_XSTREAMMemcpy" );
    
    // lose any previous data
    XSTREAM_ReuseStream( pDestStream );

    // if there is no room to append...
    if ( !XSTREAM_CanExpandSize( pDestStream, XSTREAM_STREAMED_DATA_SIZE ) )
        return 1;

    // if data is fragmented, copy the defragmented data to the destination stream
    if ( pStream->pbyHead != pStream->pbyBufPtr )
        DefragmentByteStream( pStream, pDestStream );
    else
        x_memcpy( pDestStream->pbyBufPtr, pStream->pbyHead, XSTREAM_STREAMED_DATA_SIZE );

    // set the internal pointers
    return ( AppendXToByteStream(pDestStream, NULL, XSTREAM_STREAMED_DATA_SIZE, 0) );	
}

//===================================================================
// Function : XSTREAM_UndoLast
// Desc     : Undoes the last streaming operation. Calling XSTREAM_UndoLast twice
//            consecutively, re-does the last streaming operation
// In		: pStream:	A pointer to an allocated x_mem_stream structure
// Returns	: 1, if the last operation cannot be undone
//===================================================================
u32	XSTREAM_UndoLast		    ( x_mem_stream *pStream )
{
    ASSERTS_PTR( pStream, "XSTREAM_UndoLast" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_UndoLast" );

    switch ( pStream->PrevOp.byOpID )
    {
        u32 dwReturnCode;

    case XSTREAM_APPEND_OPCODE:
        dwReturnCode = TruncateXFromByteStream( pStream, NULL, pStream->PrevOp.sdwOpSize, 0 );
        ASSERTS( !dwReturnCode, "XSTREAM_UndoLast" );
        return 0;
 
    case XSTREAM_TRUNCATE_OPCODE:
        dwReturnCode = AppendXToByteStream( pStream, NULL, pStream->PrevOp.sdwOpSize, 0 );
        ASSERTS( !dwReturnCode, "XSTREAM_UndoLast" );
        return 0;

    case XSTREAM_PREFIX_OPCODE:
        dwReturnCode = ExtractXFromByteStream( pStream, NULL, pStream->PrevOp.sdwOpSize, 0 );
        ASSERTS( !dwReturnCode, "XSTREAM_UndoLast" );
        return 0;

    case XSTREAM_EXTRACT_OPCODE:
        dwReturnCode = PrefixXToByteStream( pStream, NULL, pStream->PrevOp.sdwOpSize, 0 );
        ASSERTS( !dwReturnCode, "XSTREAM_UndoLast" );
        return 0;

    case XSTREAM_SEEK_HEAD_OPCODE:
        dwReturnCode = SetValidDataPointer( pStream, SET_HEAD_POINTER, (0 - pStream->PrevOp.sdwOpSize), XSTREAM_SEEK_CUR );
        ASSERTS( !dwReturnCode, "XSTREAM_UndoLast" );
        return 0;

    case XSTREAM_SEEK_TAIL_OPCODE:
        dwReturnCode = SetValidDataPointer( pStream, SET_TAIL_POINTER, (0 - pStream->PrevOp.sdwOpSize), XSTREAM_SEEK_CUR );
        ASSERTS( !dwReturnCode, "XSTREAM_UndoLast" );
        return 0;

    default:
        break;
    }

    return 1;
}

//===================================================================
//===================================================================
// NOTE FOR ALL APPEND FUNCTIONS: 
// Desc    : Appends data into the END OF THE memory stream
// In	   : The address of an initialized x_mem_stream structure, and
//			 the data of the corresponding type to be appended to stream
// Returns : 0 for success, 1 if appending will cause the memory stream to
//			 overflow
//===================================================================

//===================================================================
// NOTE FOR ALL PREFIX FUNCTIONS: 
// Desc    : Prefixes data into the BEGINNING OF THE memory stream
// In	   : The address of an initialized x_mem_stream structure, and
//			 the data of the corresponding type to be appended to stream
// Returns : 0 for success, 1 if prefixing will cause the memory stream to
//			 overflow
//===================================================================

//===================================================================
// NOTE FOR ALL EXTRACT FUNCTIONS: 
// Desc    : Extracts data from the BEGINNING OF THE memory stream into the given storage location
// In	   : The address of an allocated x_mem_stream structure
// In\Out  : The storage location (address) of the corresponding type to be 
//			 written into with the data extracted from the memory stream
// Returns : 0 for success, 1 if extraction cannot occur because stream 
//			 does not contain the requested size 
//===================================================================

//===================================================================
// NOTE FOR ALL TRUNCATE FUNCTIONS: 
// Desc    : Truncatess data from the END OF THE memory stream into the given storage location
// In	   : The address of an allocated x_mem_stream structure
// In\Out  : The storage location (address) of the corresponding type to be 
//			 written into with the data extracted from the memory stream
// Returns : 0 for success, 1 if extraction cannot occur because stream 
//			 does not contain the requested size 
//===================================================================
//===================================================================
u32	XSTREAM_Append_mem	( x_mem_stream *pStream, void *pSrcData, s32 sdwSizeOfSrcData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_mem" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_mem" );
    ASSERTS_PTR( pSrcData, "XSTREAM_Append_mem" );
    ASSERTS( sdwSizeOfSrcData >= 0, "XSTREAM_Append_mem" );
    
    return( AppendXToByteStream(pStream, pSrcData, sdwSizeOfSrcData, 1) );
}

//===================================================================
u32	XSTREAM_Extract_mem ( x_mem_stream *pStream, void *pDestData, s32 sdwSizeOfDestData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_mem" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_mem" );
    ASSERTS_PTR( pDestData, "XSTREAM_Extract_mem" );
    ASSERTS( sdwSizeOfDestData >= 0, "XSTREAM_Extract_mem" );
    
    return( ExtractXFromByteStream(pStream, pDestData, sdwSizeOfDestData, 1) );
}

//===================================================================
u32	XSTREAM_Prefix_mem	( x_mem_stream *pStream, void *pSrcData, s32 sdwSizeOfSrcData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_mem" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_mem" );
    ASSERTS_PTR( pSrcData, "XSTREAM_Prefix_mem" );
    ASSERTS( sdwSizeOfSrcData >= 0, "XSTREAM_Prefix_mem" );
    
    return( PrefixXToByteStream(pStream, pSrcData, sdwSizeOfSrcData, 1) );
}

//===================================================================
u32	XSTREAM_Truncate_mem ( x_mem_stream *pStream, void *pDestData, s32 sdwSizeOfDestData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_mem" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_mem" );
    ASSERTS_PTR( pDestData, "XSTREAM_Truncate_mem" );
    ASSERTS( sdwSizeOfDestData >= 0, "XSTREAM_Truncate_mem" );
    
    return( TruncateXFromByteStream(pStream, pDestData, sdwSizeOfDestData, 1) );
}

//===================================================================
u32	XSTREAM_Extract_memptr( x_mem_stream *pStream, void **pDestData, s32 sdwSizeOfDestData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_mem" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_mem" );
    ASSERTS_PTR( pDestData, "XSTREAM_Extract_mem" );
    ASSERTS( sdwSizeOfDestData >= 0, "XSTREAM_Extract_mem" );
    
    //	/ if there are insufficient bytes to extract...
	if ( WILL_XSTREAM_STREAM_UNDERFLOW(sdwSizeOfDestData) )
        return 1;
    
    // if data is fragmented, defragment
    if ( pStream->pbyHead != pStream->pbyBufPtr )
        DefragmentByteStream( pStream, NULL );

    // it is safe to do this now...
    pStream->pbyHead += sdwSizeOfDestData;
    
    *pDestData = pStream->pbyHead;

    pStream->PrevOp.byOpID = XSTREAM_EXTRACT_OPCODE;
    pStream->PrevOp.sdwOpSize = sdwSizeOfDestData;

    return 0;
}

//===================================================================
u32	XSTREAM_Truncate_memptr( x_mem_stream *pStream, void **pDestData, s32 sdwSizeOfDestData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_memptr" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_memptr" );
    ASSERTS_PTR( pDestData, "XSTREAM_Truncate_memptr" );
    ASSERTS( sdwSizeOfDestData >= 0, "XSTREAM_Truncate_memptr" );
    
    // if there are insufficient bytes to extract...
	if ( WILL_XSTREAM_STREAM_UNDERFLOW(sdwSizeOfDestData) )
        return 1;
    
    // if data is fragmented, defragment
    if ( pStream->pbyHead != pStream->pbyBufPtr )
        DefragmentByteStream( pStream, NULL );

    // it is safe to do this now...
    pStream->pbyTail -= sdwSizeOfDestData;
    
    *pDestData = pStream->pbyTail;

    pStream->PrevOp.byOpID = XSTREAM_TRUNCATE_OPCODE;
    pStream->PrevOp.sdwOpSize = sdwSizeOfDestData;

    return 0;
}

//===================================================================
u32 XSTREAM_Append_u8	( x_mem_stream *pStream, u8 byData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_u8" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_u8" );
    
    return ( AppendXToByteStream(pStream, &byData, XSTREAM_SIZE_U8, 1) );
}

//===================================================================
u32 XSTREAM_Extract_u8	( x_mem_stream *pStream, u8 *pbyData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_u8" );
    ASSERTS_PTR( pbyData, "XSTREAM_Extract_u8" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_u8" );
    
    return ( ExtractXFromByteStream(pStream, pbyData, XSTREAM_SIZE_U8, 1) );
}

//===================================================================
u32 XSTREAM_Prefix_u8	( x_mem_stream *pStream, u8 byData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_u8" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_u8" );
    
    return ( PrefixXToByteStream(pStream, &byData, XSTREAM_SIZE_U8, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_u8	( x_mem_stream *pStream, u8 *pbyData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_u8" );
    ASSERTS_PTR( pbyData, "XSTREAM_Truncate_u8" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_u8" );
    
    return ( TruncateXFromByteStream(pStream, pbyData, XSTREAM_SIZE_U8, 1) );
}

//===================================================================
u32 XSTREAM_Append_s8	( x_mem_stream *pStream, s8 sbyData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_s8" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_s8" );
    
    return ( AppendXToByteStream( pStream, &sbyData, XSTREAM_SIZE_S8, 1) );
}

//===================================================================
u32 XSTREAM_Extract_s8	( x_mem_stream *pStream, s8 *psbyData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_s8" );
    ASSERTS_PTR( psbyData, "XSTREAM_Extract_s8" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_s8" );
    
    return ( ExtractXFromByteStream(pStream, psbyData, XSTREAM_SIZE_S8, 1) );
}

//===================================================================
u32 XSTREAM_Prefix_s8	( x_mem_stream *pStream, s8 sbyData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_s8" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_s8" );
    
    return ( PrefixXToByteStream( pStream, &sbyData, XSTREAM_SIZE_S8, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_s8	( x_mem_stream *pStream, s8 *psbyData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_s8" );
    ASSERTS_PTR( psbyData, "XSTREAM_Truncate_s8" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_s8" );
    
    return ( TruncateXFromByteStream(pStream, psbyData, XSTREAM_SIZE_S8, 1) );
}

//===================================================================
u32 XSTREAM_Append_u16_noendian	( x_mem_stream *pStream, u16 wData)
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_u16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_u16" );
    
    //wData = ENDIAN_LocalToNet16( wData );
    return ( AppendXToByteStream( pStream, &wData, XSTREAM_SIZE_U16, 1) );
}

//===================================================================
u32 XSTREAM_Append_u16	( x_mem_stream *pStream, u16 wData)
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_u16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_u16" );
    
    wData = ENDIAN_LocalToNet16( wData );
    return ( AppendXToByteStream( pStream, &wData, XSTREAM_SIZE_U16, 1) );
}

//===================================================================
u32 XSTREAM_Extract_u16	( x_mem_stream *pStream, u16 *pwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_u16" );
    ASSERTS_PTR( pwData, "XSTREAM_Extract_u16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_u16" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == ExtractXFromByteStream(pStream, pwData, XSTREAM_SIZE_U16, 1) )
    {
        *pwData = ENDIAN_NetToLocal16( *pwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Prefix_u16	( x_mem_stream *pStream, u16 wData)
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_u16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_u16" );
    
    wData = ENDIAN_LocalToNet16( wData );
    return ( PrefixXToByteStream( pStream, &wData, XSTREAM_SIZE_U16, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_u16	( x_mem_stream *pStream, u16 *pwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_u16" );
    ASSERTS_PTR( pwData, "XSTREAM_Truncate_u16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_u16" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == TruncateXFromByteStream(pStream, pwData, XSTREAM_SIZE_U16, 1) )
    {
        *pwData = ENDIAN_NetToLocal16( *pwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Append_s16	( x_mem_stream *pStream, s16 swData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_s16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_s16" );
    
    swData = ENDIAN_LocalToNet16( swData );
    return ( AppendXToByteStream( pStream, &swData, XSTREAM_SIZE_S16, 1) );
}

//===================================================================
u32 XSTREAM_Extract_s16	( x_mem_stream *pStream, s16 *pswData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_s16" );
    ASSERTS_PTR( pswData, "XSTREAM_Extract_s16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_s16" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == ExtractXFromByteStream(pStream, pswData, XSTREAM_SIZE_S16, 1) )
    {
        *pswData = ENDIAN_NetToLocal16( *pswData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Prefix_s16	( x_mem_stream *pStream, s16 swData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_s16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_s16" );
    
    swData = ENDIAN_LocalToNet16( swData );
    return ( PrefixXToByteStream( pStream, &swData, XSTREAM_SIZE_S16, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_s16	( x_mem_stream *pStream, s16 *pswData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_s16" );
    ASSERTS_PTR( pswData, "XSTREAM_Truncate_s16" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_s16" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == TruncateXFromByteStream(pStream, pswData, XSTREAM_SIZE_S16, 1) )
    {
        *pswData = ENDIAN_NetToLocal16( *pswData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Append_u32	( x_mem_stream *pStream, u32 dwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_u32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_u32" );
    
    dwData = ENDIAN_LocalToNet32( dwData );
    return ( AppendXToByteStream( pStream, &dwData, XSTREAM_SIZE_U32, 1) );
}
      
//===================================================================
u32 XSTREAM_Extract_u32	( x_mem_stream *pStream, u32 *pdwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_u32" );
    ASSERTS_PTR( pdwData, "XSTREAM_Extract_u32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_u32" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == ExtractXFromByteStream(pStream, pdwData, XSTREAM_SIZE_U32, 1) )
    {
        *pdwData = ENDIAN_NetToLocal32( *pdwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Prefix_u32	( x_mem_stream *pStream, u32 dwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_u32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_u32" );
    
    dwData = ENDIAN_LocalToNet32( dwData );
    return ( PrefixXToByteStream( pStream, &dwData, XSTREAM_SIZE_U32, 1) );
}
      
//===================================================================
u32 XSTREAM_Truncate_u32	( x_mem_stream *pStream, u32 *pdwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_u32" );
    ASSERTS_PTR( pdwData, "XSTREAM_Truncate_u32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_u32" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == TruncateXFromByteStream(pStream, pdwData, XSTREAM_SIZE_U32, 1) )
    {
        *pdwData = ENDIAN_NetToLocal32( *pdwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Append_s32	( x_mem_stream *pStream, s32 sdwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_s32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_s32" );
    
    sdwData = ENDIAN_LocalToNet32( sdwData );
    return ( AppendXToByteStream( pStream, &sdwData, XSTREAM_SIZE_S32, 1) );
}

//===================================================================
u32 XSTREAM_Extract_s32	( x_mem_stream *pStream, s32 *psdwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_s32" );
    ASSERTS_PTR( psdwData, "XSTREAM_Extract_s32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_s32" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == ExtractXFromByteStream(pStream, psdwData, XSTREAM_SIZE_S32, 1) )
    {
        *psdwData = ENDIAN_NetToLocal32( *psdwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Prefix_s32	( x_mem_stream *pStream, s32 sdwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_s32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_s32" );
    
    sdwData = ENDIAN_LocalToNet32( sdwData );
    return ( PrefixXToByteStream( pStream, &sdwData, XSTREAM_SIZE_S32, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_s32	( x_mem_stream *pStream, s32 *psdwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_s32" );
    ASSERTS_PTR( psdwData, "XSTREAM_Truncate_s32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_s32" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == TruncateXFromByteStream(pStream, psdwData, XSTREAM_SIZE_S32, 1) )
    {
        *psdwData = ENDIAN_NetToLocal32( *psdwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Append_u64	( x_mem_stream *pStream, u64 qwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_u64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_u64" );
    
    qwData = ENDIAN_LocalToNet64( qwData );
    return ( AppendXToByteStream( pStream, &qwData, XSTREAM_SIZE_U64, 1) );
}

//===================================================================
u32 XSTREAM_Extract_u64	( x_mem_stream *pStream, u64 *pqwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_u64" );
    ASSERTS_PTR( pqwData, "XSTREAM_Extract_u64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_u64" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == ExtractXFromByteStream(pStream, pqwData, XSTREAM_SIZE_U64, 1) )
    {
        *pqwData = ENDIAN_NetToLocal64( *pqwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Prefix_u64	( x_mem_stream *pStream, u64 qwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_u64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_u64" );
    
    qwData = ENDIAN_LocalToNet64( qwData );
    return ( PrefixXToByteStream( pStream, &qwData, XSTREAM_SIZE_U64, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_u64	( x_mem_stream *pStream, u64 *pqwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_u64" );
    ASSERTS_PTR( pqwData, "XSTREAM_Truncate_u64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_u64" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == TruncateXFromByteStream(pStream, pqwData, XSTREAM_SIZE_U64, 1) )
    {
        *pqwData = ENDIAN_NetToLocal64( *pqwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Append_s64	( x_mem_stream *pStream, s64 sqwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_s64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_s64" );
    
    sqwData = ENDIAN_LocalToNet64( sqwData );
    return ( AppendXToByteStream( pStream, &sqwData, XSTREAM_SIZE_S64, 1) );
}

//===================================================================
u32 XSTREAM_Extract_s64	( x_mem_stream *pStream, s64 *psqwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_s64" );
    ASSERTS_PTR( psqwData, "XSTREAM_Extract_s64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_s64" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == ExtractXFromByteStream(pStream, psqwData, XSTREAM_SIZE_S64, 1) )
    {
        *psqwData = ENDIAN_NetToLocal64( *psqwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Prefix_s64	( x_mem_stream *pStream, s64 sqwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_s64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_s64" );
    
    sqwData = ENDIAN_LocalToNet64( sqwData );
    return ( PrefixXToByteStream( pStream, &sqwData, XSTREAM_SIZE_S64, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_s64	( x_mem_stream *pStream, s64 *psqwData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_s64" );
    ASSERTS_PTR( psqwData, "XSTREAM_Extract_s64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_s64" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == TruncateXFromByteStream(pStream, psqwData, XSTREAM_SIZE_S64, 1) )
    {
        *psqwData = ENDIAN_NetToLocal64( *psqwData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Append_f32	( x_mem_stream *pStream, f32 fData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_f32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_f32" );
    
    fData = ENDIAN_LocalToNet_f32( fData );
    return ( AppendXToByteStream( pStream, &fData, XSTREAM_SIZE_F32, 1) );
}

//===================================================================
u32 XSTREAM_Extract_f32	( x_mem_stream *pStream, f32 *pfData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_f32" );
    ASSERTS_PTR( pfData, "XSTREAM_Extract_f32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_f32" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == ExtractXFromByteStream(pStream, pfData, XSTREAM_SIZE_F32, 1) )
    {
        *pfData = ENDIAN_NetToLocal_f32( *pfData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Prefix_f32	( x_mem_stream *pStream, f32 fData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_f32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_f32" );
    
    fData = ENDIAN_LocalToNet_f32( fData );
    return ( PrefixXToByteStream( pStream, &fData, XSTREAM_SIZE_F32, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_f32	( x_mem_stream *pStream, f32 *pfData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_f32" );
    ASSERTS_PTR( pfData, "XSTREAM_Truncate_f32" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_f32" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == TruncateXFromByteStream(pStream, pfData, XSTREAM_SIZE_F32, 1) )
    {
        *pfData = ENDIAN_NetToLocal_f32( *pfData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Append_f64	( x_mem_stream *pStream, f64 dData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Append_f64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Append_f64" );
    
    dData = ENDIAN_LocalToNet_f64( dData );
    return ( AppendXToByteStream( pStream, &dData, XSTREAM_SIZE_F64, 1) );
}

//===================================================================
u32 XSTREAM_Extract_f64	( x_mem_stream *pStream, f64 *pdData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Extract_f64" );
    ASSERTS_PTR( pdData, "XSTREAM_Extract_f64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Extract_f64" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == ExtractXFromByteStream(pStream, pdData, XSTREAM_SIZE_F64, 1) )
    {
        *pdData = ENDIAN_NetToLocal_f64( *pdData );
        return 0;
    }
    return 1;
}

//===================================================================
u32 XSTREAM_Prefix_f64	( x_mem_stream *pStream, f64 dData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Prefix_f64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Prefix_f64" );
    
    dData = ENDIAN_LocalToNet_f64( dData );
    return ( PrefixXToByteStream( pStream, &dData, XSTREAM_SIZE_F64, 1) );
}

//===================================================================
u32 XSTREAM_Truncate_f64	( x_mem_stream *pStream, f64 *pdData )
{
    ASSERTS_PTR( pStream, "XSTREAM_Truncate_f64" );
    ASSERTS_PTR( pdData, "XSTREAM_Truncate_f64" );
    ASSERTS( pStream->dwInitCode == XSTREAM_BYTE_STREAM_INITCODE, "XSTREAM_Truncate_f64" );
    
    // if extract is successful (zero is returned), undo network byte order
    if ( 0 == TruncateXFromByteStream(pStream, pdData, XSTREAM_SIZE_F64, 1) )
    {
        *pdData = ENDIAN_NetToLocal_f64( *pdData );
        return 0;
    }
    return 1;
}

////////////////////////////////////////////////////////////////////////////
//
//  NetSpine Brand Technology, Property of Acclaim Entertainment. 
//	Trademark and Patent Pending 2001.
//
////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//
//		Filename:	MemoryStream.h
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef __X_MEMSTREAM_H__
#define __X_MEMSTREAM_H__

///////////////////////////////////////////////////////////////////////////////////
//
//		Desc:	The x_mem_stream data type provides a way to stream data of primitive 
//				types only. The x_mem_stream was primarily created to marshall and unmarshall
//				data that is to be sent over a network. Basically, the x_mem_stream data type 
//				uses standard buffers and can be initialized with a maximum size (internal 
//				buffer is dynamically allocated). As data is appended, the end of the streamed 
//				data is stored. Therefore, subsequent calls to the append functions appends data
//				automatically into the stream buffer without the need of the user to supply a location.
//				The byte stream can be reused via the XSTREAM_ReuseStream function.
//				However, if a new maximum size is needed, the stream buffer must be deinitialized and
//				reinitialized with a new maximum size. Finally, the byte stream access functions were 
//				implemented to ignore runtime checks for NULL x_mem_stream pointers in release mode 
//				to optimize runtime performance. 
//				WARNING!!!! IT IS THE RESPONSIBILITY OF THE CALLING FUNCTION TO PREVENT THE INPUTTING
//				OF NULL BYTE STREAM POINTERS INTO THE BYTE STREAM FUNCTIONS!  
//				   
//				NOTE: You must also include files EndianHelper.c/h in projects using the bytestream!!!
//
///////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//  C++ READY, In case this header is accessed by a C++ file.
////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    
////////////////////////////////////////////////////////////////////////////
//  HELPFUL DEFINITIONS
////////////////////////////////////////////////////////////////////////////
#define XSTREAM_SEEK_SET    0
#define XSTREAM_SEEK_CUR    1
#define XSTREAM_SEEK_END    2
    
#define XSTREAM_SIZE_U8		(s32) sizeof(u8)
#define XSTREAM_SIZE_S8		(s32) sizeof(s8)
#define XSTREAM_SIZE_U16	(s32) sizeof(u16)
#define XSTREAM_SIZE_S16	(s32) sizeof(s16)
#define XSTREAM_SIZE_U32	(s32) sizeof(u32)
#define XSTREAM_SIZE_S32	(s32) sizeof(s32)
#define XSTREAM_SIZE_U64	(s32) sizeof(u64)
#define XSTREAM_SIZE_S64	(s32) sizeof(s64)
#define XSTREAM_SIZE_F32	(s32) sizeof(f32)
#define XSTREAM_SIZE_F64	(s32) sizeof(f64)
    
////////////////////////////////////////////////////////////////////////////
// STRUCTURE DEFINITION OF THE BYTE STREAM
////////////////////////////////////////////////////////////////////////////

typedef struct _tagX_MEM_STREAM x_mem_stream;

////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

//===================================================================
// Function : XSTREAM_Init
// Desc     : Assigns a buffer to be used to stream data into. 
//			  NOTE: In Debug mode, all calls to stream buffer access functions will fail
//			  if stream buffer has not been initialized
// In		: pStream:	A pointer to an allocated x_mem_stream structure
//			  pbyBuffer: The buffer to be used as the stream buffer
//			  sdwMaxStreamSize:  the maximum size 
// Returns	: 0 for success, 1 if size of buffer is invalid
//===================================================================
u32 XSTREAM_Init		( x_mem_stream *pStream, u8 *pbyBuffer, s32 dwMaxStreamSize, s32 sdwSizeOfValidData );

//===================================================================
// Function : XSTREAM_DeInit
// Desc     : Clears the x_mem_stream structure for reinitialization
// In		: pStream: A pointer to an initialized x_mem_stream structure
//===================================================================
void XSTREAM_DeInit				( x_mem_stream *pStream );

//===================================================================
// Function : XSTREAM_ReuseStream
// Desc     : Resets the stream buffer for reuse. Its internal buffer is maintained.
//			  Has the effect of "erasing" any streamed data. Any streamed data will
//			  no longer be recoverable
// In		: pStream: A pointer to an initialized x_mem_stream structure
//===================================================================
void XSTREAM_ReuseStream		( x_mem_stream *pStream );

//===================================================================
// Function : XSTREAM_GetMaxStreamDataSize
// Desc     : Used to inquire the maximum size of the stream buffer
// In		: pStream: A pointer to an initialized x_mem_stream structure
// Returns	: The size of the streamed data within the stream buffer
//===================================================================
s32	XSTREAM_GetMaxStreamSize	( x_mem_stream *pStream );

//===================================================================
// Function : XSTREAM_GetStreamDataSize
// Desc     : Used to inquire the size of the streamed data within the stream buffer
// In		: pStream: A pointer to an initialized x_mem_stream structure
// Returns	: The size of the streamed data within the stream buffer
//===================================================================
s32	XSTREAM_GetStreamDataSize	( x_mem_stream *pStream );

//===================================================================
// Function : XSTREAM_GetStreamPtr
// Desc     : Used to acquire a pointer to the beginning of the streamed data
// In		: pStream: A pointer to an initialized x_mem_stream structure
// Returns	: A pointer to the beginning of the streamed data buffer
// NOTE		: This function has the added cost of first defragmenting the streamed data
//			  Use judiciously!!
//===================================================================
u8	*XSTREAM_GetStreamPtr		( x_mem_stream *pStream );

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
//            bTreatPtrOverlapAsFull: 
//                  x_true: Treat buffer as FULL when begin and end data pointers
//                          point to the same address
//                  x_false: Treat buffer as EMPTY when begin and end data pointers
//                          point to the same address
// Returns	: 1 indicates that the beginning of valid data could NOT be set
//			  0 indicates success
//===================================================================
u32 XSTREAM_SetValidDataBegin   ( x_mem_stream *pStream
                                 , s32 sdwOffset
                                 , u32 dwWhatPosition );

//===================================================================
// Function : XSTREAM_SetValidDataEnd
// Desc     : Used to set the given position as the end of valid data
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
// Returns	: 1 indicates that the end of valid data could NOT be set
//			  0 indicates success
//===================================================================
u32 XSTREAM_SetValidDataEnd   ( x_mem_stream *pStream
                                 , s32 sdwOffset
                                 , u32 dwWhatPosition );

//===================================================================
// Function : XSTREAM_CanExpandSize
// Desc     : Used to determine if size to be added will overflow the stream buffer 
// In		: pStream:	A pointer to an allocated x_mem_stream structure
//			  sdwSizeToExpand:  The desired size to be appended in bytes
// Returns	: 1 indicates that it is safe to append the desired size, 0 indicates that a
//			  subsequent call to any append function with that size or greater will fail 
//===================================================================
u32 XSTREAM_CanExpandSize ( x_mem_stream *pStream, s32 sdwSizeToExpand );

//===================================================================
// Function : XSTREAM_CanShrinkSize
// Desc     : Used to determine if a size can be removed from the stream buffer 
// In		: pStream:	A pointer to an allocated x_mem_stream structure
//			  sdwSizeToShrink:  The desired size to be extracted in bytes
// Returns	: 1 indicates that it is safe to extract the desired size, 0 indicates that a
//			  subsequent call to any extract function with that size or greater will fail 
//===================================================================
u32 XSTREAM_CanShrinkSize ( x_mem_stream *pStream, s32 sdwSizeToShrink );

//===================================================================
// Function : XSTREAM_Memcpy
// Desc     : Copies data from a buffer into the byte stream. NOTE: Any previous data
//			  in the destination stream buffer will be lost				
// In		: pStream: A pointer to an initialized x_mem_stream structure
//			  pSrcData: A pointer to the buffer that will be copied into the 
//						stream buffer
//			  sdwSizeOfSrcData: The size of the source buffer
// Returns  : 0 for success, 1 if copying will cause the byte stream to
//			  overflow
//===================================================================
u32	XSTREAM_Memcpy				( x_mem_stream *pStream, void *pSrcData, s32 sdwSizeOfSrcData );

//===================================================================
// Function : XSTREAM_XSTREAMMemcpy
// Desc     : Makes an exact copy of the source stream. NOTE: Any previous data
//			  in the destination stream buffer will be lost. 				
// In		: pStream: A pointer to the initialized destination x_mem_stream structure
//			  pSrcStream: A pointer to the initialized source x_mem_stream structure
// Returns  : 0 for success, 1 if there is no room in the destination stream buffer to copy data
//===================================================================
u32	XSTREAM_XSTREAMMemcpy		( x_mem_stream *pDestStream, x_mem_stream *pSrcStream );


//===================================================================
// Function : XSTREAM_UndoLast
// Desc     : Undoes the last streaming operation. Calling XSTREAM_UndoLast twice
//            consecutively, re-does the last streaming operation
// In		: pStream:	A pointer to an allocated x_mem_stream structure
// Returns	: 1, if the last operation cannot be undone
//===================================================================
u32	XSTREAM_UndoLast		    ( x_mem_stream *pStream );

//===================================================================
//===================================================================
// NOTE FOR ALL APPEND FUNCTIONS: 
// Desc    : Appends data into the END OF THE valid data within the stream buffer
// In	   : The address of an initialized x_mem_stream structure, and
//			 the data of the corresponding type to be appended to stream
// Returns : 0 for success, 1 if appending will cause the byte stream to
//			 overflow
//===================================================================

//===================================================================
// NOTE FOR ALL PREFIX FUNCTIONS: 
// Desc    : Prefixes data to the BEGINNING OF THE valid data within the stream buffer
// In	   : The address of an initialized x_mem_stream structure, and
//			 the data of the corresponding type to be appended to stream
// Returns : 0 for success, 1 if prefixing will cause the byte stream to
//			 overflow
//===================================================================

//===================================================================
// NOTE FOR ALL EXTRACT FUNCTIONS: 
// Desc    : Extracts data from the BEGINNING OF THE valid data within the stream buffer
//           into the given storage location
// In	   : The address of an allocated x_mem_stream structure
// In\Out  : The storage location (address) of the corresponding type to be 
//			 written into with the data extracted from the byte stream
// Returns : 0 for success, 1 if extraction cannot occur because stream 
//			 does not contain the requested size 
//===================================================================

//===================================================================
// NOTE FOR ALL TRUNCATE FUNCTIONS: 
// Desc    : Truncates data from the END OF THE valid data within the stream buffer
//           into the given storage location
// In	   : The address of an allocated x_mem_stream structure
// In\Out  : The storage location (address) of the corresponding type to be 
//			 written into with the data extracted from the byte stream
// Returns : 0 for success, 1 if extraction cannot occur because stream 
//			 does not contain the requested size 
//===================================================================
//===================================================================


//===================================================================

u32	XSTREAM_Append_mem	    ( x_mem_stream *pStream, void *pSrcData, s32 sdwSizeOfSrcData );
u32	XSTREAM_Extract_mem	    ( x_mem_stream *pStream, void *pDestData, s32 sdwSizeOfDestData );
u32	XSTREAM_Prefix_mem	    ( x_mem_stream *pStream, void *pSrcData, s32 sdwSizeOfSrcData );
u32	XSTREAM_Truncate_mem    ( x_mem_stream *pStream, void *pDestData, s32 sdwSizeOfDestData );

// doesn't actually copy the data but returns a pointer to it within the buffer!
// WARNING!!!! IT IS POSSIBLE TO OVERWRITE THIS DATA DEPENDING ON HOW YOU ARE USING THE BYTESTREAM
// NOTE: CALLING THESE FUNCTIONS MAY INCUR THE COST OF DEFRAGMENTING THE STREAMED DATA
u32	XSTREAM_Extract_memptr  ( x_mem_stream *pStream, void **pDestData, s32 sdwSizeOfDestData );
u32	XSTREAM_Truncate_memptr ( x_mem_stream *pStream, void **pDestData, s32 sdwSizeOfDestData );


u32 XSTREAM_Append_u8	    ( x_mem_stream *pStream, u8 byData );
u32 XSTREAM_Extract_u8	    ( x_mem_stream *pStream, u8 *pbyData );
u32 XSTREAM_Prefix_u8	    ( x_mem_stream *pStream, u8 byData );
u32 XSTREAM_Truncate_u8	    ( x_mem_stream *pStream, u8 *pbyData );

u32 XSTREAM_Append_s8	    ( x_mem_stream *pStream, s8 sbyData );
u32 XSTREAM_Extract_s8	    ( x_mem_stream *pStream, s8 *psbyData );
u32 XSTREAM_Prefix_s8	    ( x_mem_stream *pStream, s8 sbyData );
u32 XSTREAM_Truncate_s8	    ( x_mem_stream *pStream, s8 *psbyData );

u32 XSTREAM_Append_u16	    ( x_mem_stream *pStream, u16 wData);
u32 XSTREAM_Append_u16_noendian	    ( x_mem_stream *pStream, u16 wData);
u32 XSTREAM_Extract_u16	    ( x_mem_stream *pStream, u16 *pwData );
u32 XSTREAM_Prefix_u16	    ( x_mem_stream *pStream, u16 wData);
u32 XSTREAM_Truncate_u16	( x_mem_stream *pStream, u16 *pwData );

u32 XSTREAM_Append_s16	    ( x_mem_stream *pStream, s16 swData );
u32 XSTREAM_Extract_s16	    ( x_mem_stream *pStream, s16 *pswData );
u32 XSTREAM_Prefix_s16	    ( x_mem_stream *pStream, s16 swData );
u32 XSTREAM_Truncate_s16	( x_mem_stream *pStream, s16 *pswData );

u32 XSTREAM_Append_u32	    ( x_mem_stream *pStream, u32 dwData );
u32 XSTREAM_Extract_u32	    ( x_mem_stream *pStream, u32 *pdwData );
u32 XSTREAM_Prefix_u32	    ( x_mem_stream *pStream, u32 dwData );
u32 XSTREAM_Truncate_u32	( x_mem_stream *pStream, u32 *pdwData );

u32 XSTREAM_Append_s32	    ( x_mem_stream *pStream, s32 sdwData );
u32 XSTREAM_Extract_s32	    ( x_mem_stream *pStream, s32 *psdwData );
u32 XSTREAM_Prefix_s32	    ( x_mem_stream *pStream, s32 sdwData );
u32 XSTREAM_Truncate_s32	( x_mem_stream *pStream, s32 *psdwData );

u32 XSTREAM_Append_u64	    ( x_mem_stream *pStream, u64 qwData );
u32 XSTREAM_Extract_u64	    ( x_mem_stream *pStream, u64 *pqwData );
u32 XSTREAM_Prefix_u64	    ( x_mem_stream *pStream, u64 qwData );
u32 XSTREAM_Truncate_u64    ( x_mem_stream *pStream, u64 *pqwData );

u32 XSTREAM_Append_s64      ( x_mem_stream *pStream, s64 sqwData );
u32 XSTREAM_Extract_s64     ( x_mem_stream *pStream, s64 *psqwData );
u32 XSTREAM_Prefix_s64      ( x_mem_stream *pStream, s64 sqwData );
u32 XSTREAM_Truncate_s64    ( x_mem_stream *pStream, s64 *psqwData );

u32 XSTREAM_Append_f32      ( x_mem_stream *pStream, f32 fData );
u32 XSTREAM_Extract_f32     ( x_mem_stream *pStream, f32 *pfData );
u32 XSTREAM_Prefix_f32      ( x_mem_stream *pStream, f32 fData );
u32 XSTREAM_Truncate_f32    ( x_mem_stream *pStream, f32 *pfData );

u32 XSTREAM_Append_f64	    ( x_mem_stream *pStream, f64 dData );
u32 XSTREAM_Extract_f64	    ( x_mem_stream *pStream, f64 *pdData );
u32 XSTREAM_Prefix_f64	    ( x_mem_stream *pStream, f64 dData );
u32 XSTREAM_Truncate_f64    ( x_mem_stream *pStream, f64 *pdData );

////////////////////////////////////////////////////////////////////////////
//  WARNING: FOR INTERNAL USE ONLY!!!!
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  WARNING: FOR INTERNAL USE ONLY!!!!
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  WARNING: FOR INTERNAL USE ONLY!!!!
////////////////////////////////////////////////////////////////////////////
typedef struct _tagX_MEM_STREAM_STATE
{
    u8  byOpID;
    s32 sdwOpSize;
}   LAST_XSTREAM_STATE;

struct _tagX_MEM_STREAM
{
    u8		            *pbyBufPtr;
    u8		            *pbyHead;
    u8		            *pbyTail;
    s32		            sdwMaxStreamSize;
    u8                  byIsBufferFull;
    LAST_XSTREAM_STATE  PrevOp;
#ifdef X_DEBUG										
    u32		dwInitCode;							
#endif												
};

////////////////////////////////////////////////////////////////////////////
//  C++ READY
////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////
#endif		//__X_MEMSTREAM_H__
////////////////////////////////////////////////////////////////////////////
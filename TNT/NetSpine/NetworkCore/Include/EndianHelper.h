////////////////////////////////////////////////////////////////////////////
//
//  NetSpine Brand Technology, Property of Acclaim Entertainment. 
//	Trademark and Patent Pending 2001.
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
//
//	EndianHelper.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef ENDIANHELPER_H
#define ENDIANHELPER_H

////////////////////////////////////////////////////////////////////////////
/*
General:
	******retyped from Encyclopedia of Graphics File Formats******
	We generally think of information in memory or on disk as being organized
 into a series of individual bytes of data. The data is read sequentially 
 in the order in which the bytes are stored. This type of data is called 
 byte-oriented data and is typitcally used to store character strings and
 data treated by 8-bit CPU. 
 
   Few computers look at the universe through an
 8-bit window, however. For reasons of efficiency, 16-, 32-, and 64-bit CPUs
 prefer to work with bytes organized into 16, 32, and 64-bit cells, which 
 are called words, doublewords, and quadwords, respectively. The ORDER OF 
 THE BYTES within word-, doubleword-, and quadword-oriented data is not 
 always the same; it varies depending upon the CPU that created it. 
 
   Byte-oriented data has no particular order and is therefore read the 
 same on all systems. Word-oriented data does present a potential 
 problem-probably the most common portablitity problem you will encounter 
 when moving files between platforms. The problem arises when binary data
 is written to a file on a machine with one byte order and is then read on
 a machine assuming a different byte order. Obviously, the data will be 
 read incorrectly. 
 
   It is the order of the bytes within each word and  doubleword of data 
 that determine the "endianness" of the data. The two main categories of 
 byte-ordering schemes are called big-endian and little-endian. 
 Big-endian machines store the most significant byte (MSB) at the lowest 
 address in a word, usually referred to a byte 0. Big-endian machines 
 include those based on the Motorola MC68000A series of CPU, including 
 the Commodore Amiga,and the Apple Macintosh. 
 
   Little-endian machines store the least significant byte (LSB) at 
 the lowest address in a word. The two-byte word value, 1234h, written 
 to a file in little-endian format, would be read as 3412h on a 
 big-endian system. This occurs because the big-endian system assumes 
 that the MSB, in this case the value 12h, is that the lowest address 
 within the byte.The little-endian system, however, places the MSB at 
 the highest address in the byte. When read, the position of the bytes 
 in the word are effectively flipped in the file-reading process by the
 big-endian machine. Little-endian machine include those based onthe Intel 
 iAPX86 series of CPU, including the IBM PC and clones. 
 
   Other endian formats are also been use, such as middle-endian. These 
 endian format will not discuss here....

If you have this portial of source code.
	.
	.
	.
	u16 w = 0x0001;
    s8  *b = (XCHAR*) &w;
	.
	.
	.
This is how BIG-ENDIAN machine treat a u16 data:

	b[0] will equal to 0x00
	b[1] will equal to 0x01

This is how LITTLE-ENDIAN machine treat a u16 data:

	b[0] will equal to 0x01
	b[1] will equal to 0x00

Explain portable file problems.




Explain network problems.

  Imagine that you have two (big-endian and little-endian) computers 
 communicating with each other on a computer network. The big-endian 
 machine send some data - let's say sending a u16 (0x1268) to the 
 other little-endian machine. This is the actual problem occurs. 
 The problem is, how does the little-endian machine interprate that 
 u16? 

  Since the way the little-endian CPU is designed(read General article) 
 to process data, the little-endian machine will interprete the u16 
 (0x1268) as 0x6812 which is totally different from what the big-endian 
 machine try to tell.
*/



////////////////////////////////////////////////////////////////////////////
//  C++ READY, In case this header is accessed by a C++ file.
////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////
//  TYPE DEFINITIONS
////////////////////////////////////////////////////////////////////////////

typedef enum
    { eendian_none
    , eendian_little        //LITTLE ENDIAN
    , eendian_big           //BIG ENDIAN
    , eendian_sentry_last
    } Eendiantype;


////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

//Get byte order for local machine.
Eendiantype ENDIAN_GetLocalEndian( void );

//Returns nonzero if local machine is little endian
u32     ENDIAN_IsLittleEndian( void );

//Returns nonzero if local machine is big endian
u32     ENDIAN_IsBigEndian( void );


//network functions.
//Network byte order is always big endian.
//Have Knowledge of local hardware...

//===================================================================
// Function:	ENDIAN_LocalToNet16()
// Desc    :	Converts local input data to network byte order (big-endian).
//				If the local machine is big-endian, the function will
//				just return the input value.
// Returns :	Input data in network byte order

u16     ENDIAN_LocalToNet16( u16 wInValue );

//===================================================================
// Function:	ENDIAN_NetToLocal16()
// Desc    :	Converts network byte order (big-endian) to local data
//				format. If the local machine is big-endian, the 
//				function will just return the input value.
// Returns :	Input data in network byte order

u16     ENDIAN_NetToLocal16( u16 wInValue  );

//===================================================================
// Function:	ENDIAN_LocalToNet32()
// Desc    :	Converts local input data to network byte order (big-endian).
//				If the local machine is big-endian, the function will
//				just return the input value.
// Returns :	Input data in network byte order

u32     ENDIAN_LocalToNet32( u32 dwInValue );

//===================================================================
// Function:	ENDIAN_NetToLocal32()
// Desc    :	Converts network byte order (big-endian) to local data
//				format. If the local machine is big-endian, the 
//				function will just return the input value.
// Returns :	Input data in network byte order

u32     ENDIAN_NetToLocal32( u32 dwInValue );

//===================================================================
// Function:	ENDIAN_LocalToNet64()
// Desc    :	Converts local input data to network byte order (big-endian).
//				If the local machine is big-endian, the function will
//				just return the input value.
// Returns :	Input data in network byte order

u64     ENDIAN_LocalToNet64( u64 qwInValue );

//===================================================================
// Function:	ENDIAN_NetToLocal64()
// Desc    :	Converts network byte order (big-endian) to local data
//				format. If the local machine is big-endian, the 
//				function will just return the input value.
// Returns :	Input data in network byte order
u64     ENDIAN_NetToLocal64( u64 qwInValue );

//===================================================================
// Function:	ENDIAN_LocalToNet_f32()
// Desc    :	Converts local input data to network byte order (big-endian).
//				If the local machine is big-endian, the function will
//				just return the input value.
// Returns :	Input data in network byte order

f32     ENDIAN_LocalToNet_f32( f32 fdInValue );

//===================================================================
// Function:	ENDIAN_NetToLocal_f32()
// Desc    :	Converts network byte order (big-endian) to local data
//				format. If the local machine is big-endian, the 
//				function will just return the input value.
// Returns :	Input data in network byte order

f32     ENDIAN_NetToLocal_f32( f32 fdInValue );

//===================================================================
// Function:	ENDIAN_LocalToNet_f64()
// Desc    :	Converts local input data to network byte order (big-endian).
//				If the local machine is big-endian, the function will
//				just return the input value.
// Returns :	Input data in network byte order

f64     ENDIAN_LocalToNet_f64( f64 fqInValue );
//===================================================================
// Function:	ENDIAN_NetToLocal_f64()
// Desc    :	Converts network byte order (big-endian) to local data
//				format. If the local machine is big-endian, the 
//				function will just return the input value.
// Returns :	Input data in network byte order

f64     ENDIAN_NetToLocal_f64( f64 fqInValue );



/****************** USED BY ENDIAN FUNCTIONS ***********************/

//Endian helper functions
//These function should be only called by the network functions

u16     ENDIAN_BigToLittle16( u16 wInValue );
u16     ENDIAN_LittleToBig16( u16 wInValue );

u32     ENDIAN_BigToLittle32( u32 dwInValue );
u32     ENDIAN_LittleToBig32( u32 dwInValue );

u64     ENDIAN_BigToLittle64( u64  qwInValue );
u64     ENDIAN_LittleToBig64( u64  qwInValue );

f32     ENDIAN_BigToLittle_f32( f32  fdInValue );
f32     ENDIAN_LittleToBig_f32( f32  fdInValue );

f64     ENDIAN_BigToLittle_f64( f64  fqInValue );
f64     ENDIAN_LittleToBig_f64( f64  fqInValue );



////////////////////////////////////////////////////////////////////////////
//  C++ READY
////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////
#endif      //For Current file
////////////////////////////////////////////////////////////////////////////

//###################################################################
//	
//  Filename  : CRC.H
//
//  Purpose   : Cyclic Redundancy Check functions.
//
//###################################################################
//-------------------------------------------------------------------
#ifndef	_CRC_H_	    
#define _CRC_H_	    
//-------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
	{
#endif
//Uncomment and test, came from Texas team... 
//Should work.  Just too lazy/busy too change types.
//u32     x_chksum    ( const void* Buf, s32 Count, u32 CRC );

u16     A_CalcCRC16( const u8* pabBuf, s32 lSizeBuf );
u16     A_CalcCRC16Byte( u8 b, u32 crc );
u32     A_CalcCRC32( const u8* pabBuf, u32 crc, s32 lSizeBuf );
u32     A_CalcCRC32Byte( u8 b, u32 crc );
u32		A_CalcCRC32_lword(u32 data, u32 crc);
u32		A_CalcCRC32_word(u16 data, u32 crc);
u8      A_CalcCRC8( const u8* pabBuf, s32 lSizeBuf );

//Returns 0 if success, otherwise error.
//s32     A_CalcFileCRC32( s8* pszFileName, u32* pdwCrcValue );

#ifdef __cplusplus

	}
#endif



//-------------------------------------------------------------------
#endif  //For the file _CRC_H_	    
//-------------------------------------------------------------------



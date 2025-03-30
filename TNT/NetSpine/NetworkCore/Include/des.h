////////////////////////////////////////////////////////////////////////////
//
//  NetSpine Brand Technology, Property of Acclaim Entertainment. 
//	Trademark and Patent Pending 2001.
//
////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//
//  DES.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef DES_H
#define DES_H

////////////////////////////////////////////////////////////////////////////
//
//  Description: Basic DES cipher

////RETYPED FROM APPLIED CRYPTOGRAPHY - BRUCH SCHNEIER 2nd Edition page 270////

/*  
DES - Data Encryption Standard, known as the Data Encryption Algorithm(DEA)
      by ANSI and the DEA-1 by the ISO.

DES is a block cipher; it encrypts data in 64-bit blocks. A 64-bit block 
of plaintext goes in one end of the algorithm and a 64-bit block of 
ciphertext comes out the oher end. DES is a symmetric algoithm: The same 
algorithm and key are used for both encryption and decryption(exception 
for minor differences in the key schedule).

The key length is 56 bits. (The key is usually exressed as a 64-bit number, 
but every eighth bit is used for parity checking and is ignored.These 
parity bits are the least-significant bits of the key bytes.) The key can 
be any 56-bit number and can be changed at any time. A handful of numbers 
are considered weak keys, but they can easily be avoided,. All security 
rests within the key.
  
At its simplest level, the algorithem is nothing more than a combination 
of the two basic techniques of encryption: confusion and diffusion. The 
fundamental building block of DES is a single combiantion of these 
techniques (a substitution followed by a permutation)on the text, based 
on the key. This is known as a round. DES has 16 rounds; it applies the 
same combination of techniques on the plaintext block 16 times.    
*/

/*
Note of the DES API's usage:

  Despite of DES being a block cipher, this API allows the application to 
use this DES encryption for any data size. However, the returning 
encrypted data will be in multiple of DES block size (8-byte or 64-bit). 
The size of the encryption data is slightly higher than the application data,
this is due to the header information and padding if necessary.

  In addition, this algorithem is cross platform, that means encrypted 
  application data on a little endian machine can be decrypted on a big 
  endian machine. 
  However, if the DES key structure is going to be used in a network environment
  then converting the key to endian format is the application's 
  responsibility.

*/

/*
DES performance Data:
On a Pentium II 400MHz w/ 64Mbyte RAM in Debug mode.

Testing [1] bytes
Took [0] ms to encrypt
Took [0] ms to decrypt
Testing [3334] bytes
Took [0] ms to encrypt
Took [0] ms to decrypt
Testing [6667] bytes
Took [0] ms to encrypt
Took [14] ms to decrypt
Testing [10000] bytes
Took [13] ms to encrypt
Took [0] ms to decrypt
Testing [13333] bytes
Took [10] ms to encrypt
Took [4] ms to decrypt
Testing [16666] bytes
Took [0] ms to encrypt
Took [13] ms to decrypt
Testing [19999] bytes
Took [14] ms to encrypt
Took [0] ms to decrypt
Testing [23332] bytes
Took [0] ms to encrypt
Took [14] ms to decrypt
*/
////////////////////////////////////////////////////////////////////////////

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
	{ eDES_Success               = 0    //successful operation of a function
    , eDES_DestBufferTooSmall           //Destination buffer size too small
    , eDES_InCorrectVersion             //cipher ID or cipher version is incorrect
    , eDES_InvalidParam                 //invalid parameter
    , eDES_KeyNotInitialized            //the key is not initialized
    , eDES_GenericError                 //generic error
    
    , eDES_eLastError			        // For version and error checking.
	}   EDESErr;


typedef struct
{
  u32   key_schedule[32];
  u8    byInitFlag; 
} DESKey;


/////////////////////////////////////////////////////////////////////
//////////////////DES MAIN FUNCTIONS/////////////////////////////////
/////////////////////////////////////////////////////////////////////

//===================================================================
// Function: DES_Make_Key
// Desc    : Makes an Encrypt/Decrypt key from a 8-bytes charactor string.
//           The least significant bit of each byte of the charactor string
//           is ignored, or can be used as parity checking.
//           If the pString is a NULL, a random DES key will be generated 
//           
//           WARNING: When generating more than one key, if ANY two 8-character 
//                    strings differ only in the least significant bit then 
//                    the Generated-Keys will be the same.
//                    example: {0x12,0x23,0x45,0x67,0x89,0xAB,0xCD,0x42}
//                             and
//                             {0x12,0x23,0x45,0x67,0x89,0xAB,0xCD,0x43}
//                             Both charactor string will generate the same key.
//
// Returns : eDES_Success   if success
// In      : pString        8-bytes charactor string OR NULL for random key
// Out     : pstuDESKey     Encryption/Decryption Key structure.
//
EDESErr DES_Make_Key(DESKey *pstuDESKey, u8 *pString);


//===================================================================
// Function: DES_Encrypt
// Desc    : Encrypt an data buffer with a key.
//
//           Note: Encrypted data(output buffer) size will be greater than input buffer size.
//           
//                 If you need to find out how much memory is require to store your encrypted data,
//                 just pass a (0) to the dwDestBufferSize. The number(in bytes) will be specified by 
//                 pdwTotalEncryptedBytes.
//
// Returns : eDES_Success               if success
//           eDES_KeyNotInitialized     if the Key structure is not initialized
//           eDES_InvalidParam          if the input parameter(s) is invalid
//
// In      : pstuDESKey,            Encryption Key structure - MUST use the same key for decryption.
// In      : dwDestBufferSize,      specified the size of pDestBuffer in byte 
//                                  OR
//                                  (0) to obtain the required buffer size
// In      : pSrcBuffer,            Source buffer 
// In      : dwSrcBufferSize,       specified the dwSrcBufferSize in byte
// InOut   : pDestBuffer,           Destination buffer 
// Out     : pdwTotalEcryptedBytes, the number of encrypted data bytes
//                                  OR
//                                  the number of bytes needed to store the encrypted data
//
EDESErr DES_Encrypt( DESKey     *pstuDESKey
                    , u8        *pbyDestBuffer
                    , u32       dwDestBufferSize                    
                    , u8        *pbySrcBuffer
                    , u32       dwSrcBufferSize
                    , u32       *pdwTotalEncryptedBytes
                    );

//===================================================================
// Function: DES_Decrypt
// Desc    : Decrypt an encrypted buffer back to original data. 
//          
//           Note: If you need to find out how much memory is require to store your decrypted data,
//                 just pass a (0) to the dwDestBufferSize. The number(in bytes) will be specified by 
//                 pdwTotalDataBytes.
//
// Returns : eDES_Success               if success
//           eDES_KeyNotInitialized     if the Key structure is not initialized
//           eDES_InvalidParam          if the input parameter(s) is invalid
//           eDES_DestBufferTooSmall    if the destination buffer size is too small to store the 
//                                      original data.
//           eDES_InCorrectVersion      if the CipherID
//                                      OR
//                                      CipherVer of the encryption header don't match the current
//                                      cipher.
// In      : pstuDESKey,        Decryption Key structure - MUST be the same key from encryption.
// In      : dwDestBufferSize,  specified the pDestBuffer in byte 
//                              OR
//                              (0) to obtain the required buffer size
// In      : pSrcBuffer,        Source buffer 
// In      : dwSrcBufferSize,   specified the size of encrypted data bytes(not size of source buffer)
// InOut   : pDestBuffer,       Destination buffer 
// Out     : pdwTotalDataBytes, the number of original data bytes
//                              or
//                              the number of bytes needed to store the original data
//

EDESErr DES_Decrypt(  DESKey    *pstuDESKey
                    , u8        *pbyDestBuffer
                    , u32       dwDestBufferSize
                    , u8        *pbySrcBuffer
                    , u32       dwSrcBufferSize
                    , u32       *pdwTotalDataBytes
                    );

//This is the actual Algorithm that does DEScipher
//if dwEncryptFlag is (1) for Encryption
//if dwEncryptFlag is (0) for Decryption
#define DES_CIPHER_ENCRYPT 1 
#define DES_CIPHER_DECRYPT 0
void DES_Cipher(DESKey *ks, u8 *pDestBuffer, u8 *pSrcBuffer, u32 dwEncryptFlag);


////////////////////////////////////////////////////////////////////////////
//  C++ READY
////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////
#endif      //DES_H
////////////////////////////////////////////////////////////////////////////

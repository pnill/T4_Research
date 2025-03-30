/*
DES implementation; 1995 Tatu Ylonen <ylo@cs.hut.fi>

The original source code is available on Phil Karn's website:

http://people.qualcomm.com/karn/code/des/

and Mr Karn can be reached at
Phil Karn [karn@ka9q.ampr.org]
or
karn@qualcomm.com

This implementation is derived from libdes-3.06, which is copyright
(c) 1993 Eric Young, and distributed under the GNU GPL or the ARTISTIC licence
(at the user's option).  The original distribution can be found e.g. from
ftp://ftp.dsi.unimi.it/pub/security/crypt/libdes/libdes-3.06.tar.gz.

This implementation is distributed under the same terms.  See
libdes-README, libdes-ARTISTIC, and libdes-COPYING for more
information.

A description of the DES algorithm can be found in every modern book on
cryptography and data security, including the following:

  Bruce Schneier: Applied Cryptography.  John Wiley & Sons, 1994.

  Jennifer Seberry and Josed Pieprzyk: Cryptography: An Introduction to 
    Computer Security.  Prentice-Hall, 1989.

  Man Young Rhee: Cryptography and Secure Data Communications.  McGraw-Hill, 
    1994.

*/

/*
 * $Id: des.c,v 1.1.1.1 1996/02/18 21:38:11 ylo Exp $
 * $Log: des.c,v $
 * Revision 1.1.1.1  1996/02/18  21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:22:25  ylo
 * 	Added cvs log.
 *
 * $Endlog$
 */

/*
Application data is store in the following format.
_______________________________________________________________
|           |           |               |          |          |
|CipherID   |CipherVer  |DataFieldSize  | AppData  |Padding   |
|4byte      |2byte      |4byte          | any size |0 - 7byte |
---------------------------------------------------------------
*/


//for Core functions
#include "x_files.hpp"

#include "endianhelper.h"
#include "ciphertype.h" 
#include "des.h"

////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////

#define DES_BLOCK_SIZE                  ((u32)(8))     //64 bits 
#define DES_INIT_ENCRYPT_BLOCK_SIZE     (DES_BLOCK_SIZE << 1) // Multiply by 2

#define DES_CIPHER_ID_FIELD_SIZE        (sizeof(u32))
#define DES_CIPHER_VER_FIELD_SIZE       (sizeof(u16))
#define DES_DATA_SIZE_FIELD_SIZE        (sizeof(u32))
#define DES_CIPHER_HEADER_SIZE          (DES_CIPHER_ID_FIELD_SIZE + \
								        DES_CIPHER_VER_FIELD_SIZE + \
								        DES_DATA_SIZE_FIELD_SIZE)

#define DES_PAD_BYTE        ('X')   //charactor for padding.
#define DES_KEY_INIT_FLAG   (0x96)  //10010110

/////////////////////////////////////////////////////////////////////
//////////////////HELPER FUNCTIONS//////////////////////////////////
/////////////////////////////////////////////////////////////////////



/* Table for key generation.  This used to be in sk.h. */
/* Copyright (C) 1993 Eric Young - see README for more details */
static const u32 des_skb[8][64]={
/* for C bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
{ 0x00000000,0x00000010,0x20000000,0x20000010,
0x00010000,0x00010010,0x20010000,0x20010010,
0x00000800,0x00000810,0x20000800,0x20000810,
0x00010800,0x00010810,0x20010800,0x20010810,
0x00000020,0x00000030,0x20000020,0x20000030,
0x00010020,0x00010030,0x20010020,0x20010030,
0x00000820,0x00000830,0x20000820,0x20000830,
0x00010820,0x00010830,0x20010820,0x20010830,
0x00080000,0x00080010,0x20080000,0x20080010,
0x00090000,0x00090010,0x20090000,0x20090010,
0x00080800,0x00080810,0x20080800,0x20080810,
0x00090800,0x00090810,0x20090800,0x20090810,
0x00080020,0x00080030,0x20080020,0x20080030,
0x00090020,0x00090030,0x20090020,0x20090030,
0x00080820,0x00080830,0x20080820,0x20080830,
0x00090820,0x00090830,0x20090820,0x20090830 },
/* for C bits (numbered as per FIPS 46) 7 8 10 11 12 13 */
{ 0x00000000,0x02000000,0x00002000,0x02002000,
0x00200000,0x02200000,0x00202000,0x02202000,
0x00000004,0x02000004,0x00002004,0x02002004,
0x00200004,0x02200004,0x00202004,0x02202004,
0x00000400,0x02000400,0x00002400,0x02002400,
0x00200400,0x02200400,0x00202400,0x02202400,
0x00000404,0x02000404,0x00002404,0x02002404,
0x00200404,0x02200404,0x00202404,0x02202404,
0x10000000,0x12000000,0x10002000,0x12002000,
0x10200000,0x12200000,0x10202000,0x12202000,
0x10000004,0x12000004,0x10002004,0x12002004,
0x10200004,0x12200004,0x10202004,0x12202004,
0x10000400,0x12000400,0x10002400,0x12002400,
0x10200400,0x12200400,0x10202400,0x12202400,
0x10000404,0x12000404,0x10002404,0x12002404,
0x10200404,0x12200404,0x10202404,0x12202404 },
/* for C bits (numbered as per FIPS 46) 14 15 16 17 19 20 */
{ 0x00000000,0x00000001,0x00040000,0x00040001,
0x01000000,0x01000001,0x01040000,0x01040001,
0x00000002,0x00000003,0x00040002,0x00040003,
0x01000002,0x01000003,0x01040002,0x01040003,
0x00000200,0x00000201,0x00040200,0x00040201,
0x01000200,0x01000201,0x01040200,0x01040201,
0x00000202,0x00000203,0x00040202,0x00040203,
0x01000202,0x01000203,0x01040202,0x01040203,
0x08000000,0x08000001,0x08040000,0x08040001,
0x09000000,0x09000001,0x09040000,0x09040001,
0x08000002,0x08000003,0x08040002,0x08040003,
0x09000002,0x09000003,0x09040002,0x09040003,
0x08000200,0x08000201,0x08040200,0x08040201,
0x09000200,0x09000201,0x09040200,0x09040201,
0x08000202,0x08000203,0x08040202,0x08040203,
0x09000202,0x09000203,0x09040202,0x09040203 },
/* for C bits (numbered as per FIPS 46) 21 23 24 26 27 28 */
{ 0x00000000,0x00100000,0x00000100,0x00100100,
0x00000008,0x00100008,0x00000108,0x00100108,
0x00001000,0x00101000,0x00001100,0x00101100,
0x00001008,0x00101008,0x00001108,0x00101108,
0x04000000,0x04100000,0x04000100,0x04100100,
0x04000008,0x04100008,0x04000108,0x04100108,
0x04001000,0x04101000,0x04001100,0x04101100,
0x04001008,0x04101008,0x04001108,0x04101108,
0x00020000,0x00120000,0x00020100,0x00120100,
0x00020008,0x00120008,0x00020108,0x00120108,
0x00021000,0x00121000,0x00021100,0x00121100,
0x00021008,0x00121008,0x00021108,0x00121108,
0x04020000,0x04120000,0x04020100,0x04120100,
0x04020008,0x04120008,0x04020108,0x04120108,
0x04021000,0x04121000,0x04021100,0x04121100,
0x04021008,0x04121008,0x04021108,0x04121108 },
/* for D bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
{ 0x00000000,0x10000000,0x00010000,0x10010000,
0x00000004,0x10000004,0x00010004,0x10010004,
0x20000000,0x30000000,0x20010000,0x30010000,
0x20000004,0x30000004,0x20010004,0x30010004,
0x00100000,0x10100000,0x00110000,0x10110000,
0x00100004,0x10100004,0x00110004,0x10110004,
0x20100000,0x30100000,0x20110000,0x30110000,
0x20100004,0x30100004,0x20110004,0x30110004,
0x00001000,0x10001000,0x00011000,0x10011000,
0x00001004,0x10001004,0x00011004,0x10011004,
0x20001000,0x30001000,0x20011000,0x30011000,
0x20001004,0x30001004,0x20011004,0x30011004,
0x00101000,0x10101000,0x00111000,0x10111000,
0x00101004,0x10101004,0x00111004,0x10111004,
0x20101000,0x30101000,0x20111000,0x30111000,
0x20101004,0x30101004,0x20111004,0x30111004 },
/* for D bits (numbered as per FIPS 46) 8 9 11 12 13 14 */
{ 0x00000000,0x08000000,0x00000008,0x08000008,
0x00000400,0x08000400,0x00000408,0x08000408,
0x00020000,0x08020000,0x00020008,0x08020008,
0x00020400,0x08020400,0x00020408,0x08020408,
0x00000001,0x08000001,0x00000009,0x08000009,
0x00000401,0x08000401,0x00000409,0x08000409,
0x00020001,0x08020001,0x00020009,0x08020009,
0x00020401,0x08020401,0x00020409,0x08020409,
0x02000000,0x0A000000,0x02000008,0x0A000008,
0x02000400,0x0A000400,0x02000408,0x0A000408,
0x02020000,0x0A020000,0x02020008,0x0A020008,
0x02020400,0x0A020400,0x02020408,0x0A020408,
0x02000001,0x0A000001,0x02000009,0x0A000009,
0x02000401,0x0A000401,0x02000409,0x0A000409,
0x02020001,0x0A020001,0x02020009,0x0A020009,
0x02020401,0x0A020401,0x02020409,0x0A020409 },
/* for D bits (numbered as per FIPS 46) 16 17 18 19 20 21 */
{ 0x00000000,0x00000100,0x00080000,0x00080100,
0x01000000,0x01000100,0x01080000,0x01080100,
0x00000010,0x00000110,0x00080010,0x00080110,
0x01000010,0x01000110,0x01080010,0x01080110,
0x00200000,0x00200100,0x00280000,0x00280100,
0x01200000,0x01200100,0x01280000,0x01280100,
0x00200010,0x00200110,0x00280010,0x00280110,
0x01200010,0x01200110,0x01280010,0x01280110,
0x00000200,0x00000300,0x00080200,0x00080300,
0x01000200,0x01000300,0x01080200,0x01080300,
0x00000210,0x00000310,0x00080210,0x00080310,
0x01000210,0x01000310,0x01080210,0x01080310,
0x00200200,0x00200300,0x00280200,0x00280300,
0x01200200,0x01200300,0x01280200,0x01280300,
0x00200210,0x00200310,0x00280210,0x00280310,
0x01200210,0x01200310,0x01280210,0x01280310 },
/* for D bits (numbered as per FIPS 46) 22 23 24 25 27 28 */
{ 0x00000000,0x04000000,0x00040000,0x04040000,
0x00000002,0x04000002,0x00040002,0x04040002,
0x00002000,0x04002000,0x00042000,0x04042000,
0x00002002,0x04002002,0x00042002,0x04042002,
0x00000020,0x04000020,0x00040020,0x04040020,
0x00000022,0x04000022,0x00040022,0x04040022,
0x00002020,0x04002020,0x00042020,0x04042020,
0x00002022,0x04002022,0x00042022,0x04042022,
0x00000800,0x04000800,0x00040800,0x04040800,
0x00000802,0x04000802,0x00040802,0x04040802,
0x00002800,0x04002800,0x00042800,0x04042800,
0x00002802,0x04002802,0x00042802,0x04042802,
0x00000820,0x04000820,0x00040820,0x04040820,
0x00000822,0x04000822,0x00040822,0x04040822,
0x00002820,0x04002820,0x00042820,0x04042820,
0x00002822,0x04002822,0x00042822,0x04042822 }
};

/* Tables used for executing des.  This used to be in spr.h. */
/* Copyright (C) 1993 Eric Young - see README for more details */
static const u32 des_SPtrans[8][64]={
/* nibble 0 */
{ 0x00820200, 0x00020000, 0x80800000, 0x80820200,
0x00800000, 0x80020200, 0x80020000, 0x80800000,
0x80020200, 0x00820200, 0x00820000, 0x80000200,
0x80800200, 0x00800000, 0x00000000, 0x80020000,
0x00020000, 0x80000000, 0x00800200, 0x00020200,
0x80820200, 0x00820000, 0x80000200, 0x00800200,
0x80000000, 0x00000200, 0x00020200, 0x80820000,
0x00000200, 0x80800200, 0x80820000, 0x00000000,
0x00000000, 0x80820200, 0x00800200, 0x80020000,
0x00820200, 0x00020000, 0x80000200, 0x00800200,
0x80820000, 0x00000200, 0x00020200, 0x80800000,
0x80020200, 0x80000000, 0x80800000, 0x00820000,
0x80820200, 0x00020200, 0x00820000, 0x80800200,
0x00800000, 0x80000200, 0x80020000, 0x00000000,
0x00020000, 0x00800000, 0x80800200, 0x00820200,
0x80000000, 0x80820000, 0x00000200, 0x80020200 },

/* nibble 1 */
{ 0x10042004, 0x00000000, 0x00042000, 0x10040000,
0x10000004, 0x00002004, 0x10002000, 0x00042000,
0x00002000, 0x10040004, 0x00000004, 0x10002000,
0x00040004, 0x10042000, 0x10040000, 0x00000004,
0x00040000, 0x10002004, 0x10040004, 0x00002000,
0x00042004, 0x10000000, 0x00000000, 0x00040004,
0x10002004, 0x00042004, 0x10042000, 0x10000004,
0x10000000, 0x00040000, 0x00002004, 0x10042004,
0x00040004, 0x10042000, 0x10002000, 0x00042004,
0x10042004, 0x00040004, 0x10000004, 0x00000000,
0x10000000, 0x00002004, 0x00040000, 0x10040004,
0x00002000, 0x10000000, 0x00042004, 0x10002004,
0x10042000, 0x00002000, 0x00000000, 0x10000004,
0x00000004, 0x10042004, 0x00042000, 0x10040000,
0x10040004, 0x00040000, 0x00002004, 0x10002000,
0x10002004, 0x00000004, 0x10040000, 0x00042000 },

/* nibble 2 */
{ 0x41000000, 0x01010040, 0x00000040, 0x41000040,
0x40010000, 0x01000000, 0x41000040, 0x00010040,
0x01000040, 0x00010000, 0x01010000, 0x40000000,
0x41010040, 0x40000040, 0x40000000, 0x41010000,
0x00000000, 0x40010000, 0x01010040, 0x00000040,
0x40000040, 0x41010040, 0x00010000, 0x41000000,
0x41010000, 0x01000040, 0x40010040, 0x01010000,
0x00010040, 0x00000000, 0x01000000, 0x40010040,
0x01010040, 0x00000040, 0x40000000, 0x00010000,
0x40000040, 0x40010000, 0x01010000, 0x41000040,
0x00000000, 0x01010040, 0x00010040, 0x41010000,
0x40010000, 0x01000000, 0x41010040, 0x40000000,
0x40010040, 0x41000000, 0x01000000, 0x41010040,
0x00010000, 0x01000040, 0x41000040, 0x00010040,
0x01000040, 0x00000000, 0x41010000, 0x40000040,
0x41000000, 0x40010040, 0x00000040, 0x01010000 },

/* nibble 3 */
{ 0x00100402, 0x04000400, 0x00000002, 0x04100402,
0x00000000, 0x04100000, 0x04000402, 0x00100002,
0x04100400, 0x04000002, 0x04000000, 0x00000402,
0x04000002, 0x00100402, 0x00100000, 0x04000000,
0x04100002, 0x00100400, 0x00000400, 0x00000002,
0x00100400, 0x04000402, 0x04100000, 0x00000400,
0x00000402, 0x00000000, 0x00100002, 0x04100400,
0x04000400, 0x04100002, 0x04100402, 0x00100000,
0x04100002, 0x00000402, 0x00100000, 0x04000002,
0x00100400, 0x04000400, 0x00000002, 0x04100000,
0x04000402, 0x00000000, 0x00000400, 0x00100002,
0x00000000, 0x04100002, 0x04100400, 0x00000400,
0x04000000, 0x04100402, 0x00100402, 0x00100000,
0x04100402, 0x00000002, 0x04000400, 0x00100402,
0x00100002, 0x00100400, 0x04100000, 0x04000402,
0x00000402, 0x04000000, 0x04000002, 0x04100400 },

/* nibble 4 */
{ 0x02000000, 0x00004000, 0x00000100, 0x02004108,
0x02004008, 0x02000100, 0x00004108, 0x02004000,
0x00004000, 0x00000008, 0x02000008, 0x00004100,
0x02000108, 0x02004008, 0x02004100, 0x00000000,
0x00004100, 0x02000000, 0x00004008, 0x00000108,
0x02000100, 0x00004108, 0x00000000, 0x02000008,
0x00000008, 0x02000108, 0x02004108, 0x00004008,
0x02004000, 0x00000100, 0x00000108, 0x02004100,
0x02004100, 0x02000108, 0x00004008, 0x02004000,
0x00004000, 0x00000008, 0x02000008, 0x02000100,
0x02000000, 0x00004100, 0x02004108, 0x00000000,
0x00004108, 0x02000000, 0x00000100, 0x00004008,
0x02000108, 0x00000100, 0x00000000, 0x02004108,
0x02004008, 0x02004100, 0x00000108, 0x00004000,
0x00004100, 0x02004008, 0x02000100, 0x00000108,
0x00000008, 0x00004108, 0x02004000, 0x02000008 },

/* nibble 5 */
{ 0x20000010, 0x00080010, 0x00000000, 0x20080800,
0x00080010, 0x00000800, 0x20000810, 0x00080000,
0x00000810, 0x20080810, 0x00080800, 0x20000000,
0x20000800, 0x20000010, 0x20080000, 0x00080810,
0x00080000, 0x20000810, 0x20080010, 0x00000000,
0x00000800, 0x00000010, 0x20080800, 0x20080010,
0x20080810, 0x20080000, 0x20000000, 0x00000810,
0x00000010, 0x00080800, 0x00080810, 0x20000800,
0x00000810, 0x20000000, 0x20000800, 0x00080810,
0x20080800, 0x00080010, 0x00000000, 0x20000800,
0x20000000, 0x00000800, 0x20080010, 0x00080000,
0x00080010, 0x20080810, 0x00080800, 0x00000010,
0x20080810, 0x00080800, 0x00080000, 0x20000810,
0x20000010, 0x20080000, 0x00080810, 0x00000000,
0x00000800, 0x20000010, 0x20000810, 0x20080800,
0x20080000, 0x00000810, 0x00000010, 0x20080010 },

/* nibble 6 */
{ 0x00001000, 0x00000080, 0x00400080, 0x00400001,
0x00401081, 0x00001001, 0x00001080, 0x00000000,
0x00400000, 0x00400081, 0x00000081, 0x00401000,
0x00000001, 0x00401080, 0x00401000, 0x00000081,
0x00400081, 0x00001000, 0x00001001, 0x00401081,
0x00000000, 0x00400080, 0x00400001, 0x00001080,
0x00401001, 0x00001081, 0x00401080, 0x00000001,
0x00001081, 0x00401001, 0x00000080, 0x00400000,
0x00001081, 0x00401000, 0x00401001, 0x00000081,
0x00001000, 0x00000080, 0x00400000, 0x00401001,
0x00400081, 0x00001081, 0x00001080, 0x00000000,
0x00000080, 0x00400001, 0x00000001, 0x00400080,
0x00000000, 0x00400081, 0x00400080, 0x00001080,
0x00000081, 0x00001000, 0x00401081, 0x00400000,
0x00401080, 0x00000001, 0x00001001, 0x00401081,
0x00400001, 0x00401080, 0x00401000, 0x00001001 },

/* nibble 7 */
{ 0x08200020, 0x08208000, 0x00008020, 0x00000000,
0x08008000, 0x00200020, 0x08200000, 0x08208020,
0x00000020, 0x08000000, 0x00208000, 0x00008020,
0x00208020, 0x08008020, 0x08000020, 0x08200000,
0x00008000, 0x00208020, 0x00200020, 0x08008000,
0x08208020, 0x08000020, 0x00000000, 0x00208000,
0x08000000, 0x00200000, 0x08008020, 0x08200020,
0x00200000, 0x00008000, 0x08208000, 0x00000020,
0x00200000, 0x00008000, 0x08000020, 0x08208020,
0x00008020, 0x08000000, 0x00000000, 0x00208000,
0x08200020, 0x08008020, 0x08008000, 0x00200020,
0x08208000, 0x00000020, 0x00200020, 0x08008000,
0x08208020, 0x00200000, 0x08200000, 0x08000020,
0x00208000, 0x00008020, 0x08008020, 0x08200000,
0x00000020, 0x08208000, 0x00208020, 0x00000000,
0x08000000, 0x08200020, 0x00008000, 0x00208020 }};


#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
	(b)^=(t),\
	(a)^=((t)<<(n)))

#define IP(l,r,t) \
	PERM_OP(r,l,t, 4,0x0f0f0f0f); \
	PERM_OP(l,r,t,16,0x0000ffff); \
	PERM_OP(r,l,t, 2,0x33333333); \
	PERM_OP(l,r,t, 8,0x00ff00ff); \
	PERM_OP(r,l,t, 1,0x55555555);

#define FP(l,r,t) \
	PERM_OP(l,r,t, 1,0x55555555); \
	PERM_OP(r,l,t, 8,0x00ff00ff); \
	PERM_OP(l,r,t, 2,0x33333333); \
	PERM_OP(r,l,t,16,0x0000ffff); \
	PERM_OP(l,r,t, 4,0x0f0f0f0f);

#define D_ENCRYPT(L,R,S)	\
	u=(R^s[S  ]); \
	t=R^s[S+1]; \
	t=((t>>4)+(t<<28)); \
	L^=	des_SPtrans[1][(t    )&0x3f]| \
		des_SPtrans[3][(t>> 8)&0x3f]| \
		des_SPtrans[5][(t>>16)&0x3f]| \
		des_SPtrans[7][(t>>24)&0x3f]| \
		des_SPtrans[0][(u    )&0x3f]| \
		des_SPtrans[2][(u>> 8)&0x3f]| \
		des_SPtrans[4][(u>>16)&0x3f]| \
		des_SPtrans[6][(u>>24)&0x3f];

#define GET_32BIT_LSB_FIRST(cp) \
  (((u32)(u8)(cp)[0]) | \
  ((u32)(u8)(cp)[1] << 8) | \
  ((u32)(u8)(cp)[2] << 16) | \
  ((u32)(u8)(cp)[3] << 24))


#define PUT_32BIT_LSB_FIRST(cp, value)  \
  (cp)[0] = (u8)(value); \
  (cp)[1] = (u8)((value) >> 8); \
  (cp)[2] = (u8)((value) >> 16); \
  (cp)[3] = (u8)((value) >> 24); 

/* Code based on set_key.c. */
/* Copyright (C) 1993 Eric Young - see README for more details */

#define HPERM_OP(a,t,n,m) ((t)=((((a)<<(16-(n)))^(a))&(m)),\
	(a)=(a)^(t)^(t>>(16-(n))))


void DES_Cipher(DESKey *ks, u8 *pDestBuffer, u8 *pSrcBuffer, u32 dwEncryptFlag)
{
    u32 t,u;
    s32 i; 
    u32 *s;
    u32 dwL, dwR;

    s = ks->key_schedule;

    dwL  = GET_32BIT_LSB_FIRST(pSrcBuffer);
    dwR = GET_32BIT_LSB_FIRST(pSrcBuffer + 4);

    IP(dwL,dwR,t);
    /* Things have been modified so that the initial rotate is
    * done outside the loop.  This required the
    * des_SPtrans values in sp.h to be rotated 1 bit to the right.
    * One perl script later and things have a 5% speed up on a sparc2.
    * Thanks to Richard Outerbridge <71755.204@CompuServe.COM>
    * for pointing this out. */
    t=(dwR<<1)|(dwR>>31);
    dwR=(dwL<<1)|(dwL>>31);
    dwL=t;

    /* I don't know if it is worth the effort of loop unrolling the
    * inner loop */
    if (dwEncryptFlag)
    {
        for (i=0; i<32; i+=4)
        {
            D_ENCRYPT(dwL,dwR,i+0); /*  1 */
            D_ENCRYPT(dwR,dwL,i+2); /*  2 */
        }
    }
    else
    {
        for (i=30; i>0; i-=4) 
        {
            D_ENCRYPT(dwL,dwR,i-0); /* 16 */
            D_ENCRYPT(dwR,dwL,i-2); /* 15 */  
        }
    }
    dwL=(dwL>>1)|(dwL<<31);
    dwR=(dwR>>1)|(dwR<<31);

    FP(dwR,dwL,t);

    PUT_32BIT_LSB_FIRST(pDestBuffer, dwL);
    PUT_32BIT_LSB_FIRST(pDestBuffer + 4, dwR);
}


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
EDESErr DES_Make_Key(DESKey *pstuDESKey, u8 *pString)
{
    u32 c, d, t, s, shifts;
    s32 i;
    u32 *schedule;
    u8  abyRandomKey[8];
    u32 dwRandomInit;
    u32 dwRandomKeyCount;

    schedule = pstuDESKey->key_schedule;
    //if the pString is a NULL, then generate a randon key.
    if(pString == NULL)
    {
        dwRandomInit = x_GetSystemUpTimeMs();
        //set an initial random number 
	    x_srand ( dwRandomInit );
        //since the eigth bit will be ignored, two consecutive key will be identical.
        for(dwRandomKeyCount = 0; dwRandomKeyCount < 8; dwRandomKeyCount++)
        {
            abyRandomKey[dwRandomKeyCount] = (u8)(x_rand() % 0xFF);
        }
        pString = abyRandomKey;
    }

    c = GET_32BIT_LSB_FIRST(pString);
    d = GET_32BIT_LSB_FIRST(pString + 4);

    /* I now do it in 47 simple operations :-)
    * Thanks to John Fletcher (john_fletcher@lccmail.ocf.llnl.gov)
    * for the inspiration. :-) */
    PERM_OP(d,c,t,4,0x0f0f0f0f);
    HPERM_OP(c,t,-2,0xcccc0000);
    HPERM_OP(d,t,-2,0xcccc0000);
    PERM_OP(d,c,t,1,0x55555555);
    PERM_OP(c,d,t,8,0x00ff00ff);
    PERM_OP(d,c,t,1,0x55555555);
    d = ((d & 0xff) << 16) | (d & 0xff00) |
        ((d >> 16) & 0xff) | ((c >> 4) & 0xf000000);
    c&=0x0fffffff;

    shifts = 0x7efc;
    for (i=0; i < 16; i++)
    {
        if (shifts & 1)
        {
            c=((c>>2)|(c<<26));
            d=((d>>2)|(d<<26)); 
        }
        else
        {
            c=((c>>1)|(c<<27)); 
            d=((d>>1)|(d<<27)); 
        }
        shifts >>= 1;
        c&=0x0fffffff;
        d&=0x0fffffff;

        /* could be a few less shifts but I am to lazy at this
        * point in time to investigate */

        s = des_skb[0][ (c    )&0x3f                ] |
            des_skb[1][((c>> 6)&0x03)|((c>> 7)&0x3c)] |
            des_skb[2][((c>>13)&0x0f)|((c>>14)&0x30)] |
            des_skb[3][((c>>20)&0x01)|((c>>21)&0x06)|((c>>22)&0x38)];

        t = des_skb[4][ (d    )&0x3f                ] |
            des_skb[5][((d>> 7)&0x03)|((d>> 8)&0x3c)] |
            des_skb[6][ (d>>15)&0x3f                ] |
            des_skb[7][((d>>21)&0x0f)|((d>>22)&0x30)];

        /* table contained 0213 4657 */
        *(schedule++) = ((t << 16) | (s & 0xffff)); 
        s = ((s >> 16) | (t & 0xffff0000));
        *(schedule++) = ((s << 4) | (s >> 28)); 
        //ENDIAN_LocalToNet32
    }

    pstuDESKey->byInitFlag = DES_KEY_INIT_FLAG;  //set the initialize flag;
    return eDES_Success;
}

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
                    )
{
    u32     dwPaddingSize;
    u32     dwExtraByte;
    u32     dwTotalByte;
    u32     dwCount;
    u8      abyTemp[DES_INIT_ENCRYPT_BLOCK_SIZE];
    u8      abyLastBlock[8];
    u8      *pbyCurrentDestBuffer = pbyDestBuffer;
    u8      *pbyCurrentSrcBuffer = pbySrcBuffer;

	u32		dw32Temp;
	u16		w16Temp;

    //ensure key struct is initialized
    if(DES_KEY_INIT_FLAG != pstuDESKey->byInitFlag)
    {
        *pdwTotalEncryptedBytes = 0;
        return eDES_KeyNotInitialized;
    }
    //return error if no source buffer 
    if(!dwSrcBufferSize)
    {
        *pdwTotalEncryptedBytes = 0;
        return eDES_InvalidParam;
    }

	//caculate the number of byte for padding.
    dwExtraByte = (DES_CIPHER_HEADER_SIZE + dwSrcBufferSize) % DES_BLOCK_SIZE ;
	if(dwExtraByte)
	{
		dwPaddingSize = DES_BLOCK_SIZE - dwExtraByte;
	}
	else
	{
		dwPaddingSize = 0;
	}
    //total size of the output buffer
    dwTotalByte = (DES_CIPHER_HEADER_SIZE + dwSrcBufferSize + dwPaddingSize);

    *pdwTotalEncryptedBytes = dwTotalByte;
    
    //check for dest buffer if big enough
    if(dwTotalByte > dwDestBufferSize )
    {
        //if the dest buffer size is not enough, then set number to byte required for the dest buffer        
        return eDES_DestBufferTooSmall;        
    }
    //set the cipher type
//    *(u32*)abyTemp = ENDIAN_LocalToNet32(CIPHER_TYPE_BASIC_DES); 
	dw32Temp = ENDIAN_LocalToNet32(CIPHER_TYPE_BASIC_DES);
	x_memcpy(abyTemp, &dw32Temp, sizeof(u32));
    //set the cipher version
//    *((u16*)(abyTemp + DES_CIPHER_ID_FIELD_SIZE)) = ENDIAN_LocalToNet16(CIPHER_VER_BASIC_DES); 
	w16Temp = ENDIAN_LocalToNet16(CIPHER_VER_BASIC_DES);
	x_memcpy(abyTemp + DES_CIPHER_ID_FIELD_SIZE, &w16Temp, sizeof(u16));
    //set the data size
//    *((u32*)(abyTemp + (DES_CIPHER_ID_FIELD_SIZE + DES_CIPHER_VER_FIELD_SIZE))) = ENDIAN_LocalToNet32(dwSrcBufferSize);
	dw32Temp = ENDIAN_LocalToNet32(dwSrcBufferSize);
	x_memcpy(abyTemp + (DES_CIPHER_ID_FIELD_SIZE + DES_CIPHER_VER_FIELD_SIZE), &dw32Temp, sizeof(u32));

    //if the source buffer size fits into the DES_INIT_ENCRYPT_BLOCK_SIZE
    if(dwSrcBufferSize <= (DES_INIT_ENCRYPT_BLOCK_SIZE - DES_CIPHER_HEADER_SIZE))
    {
        //put the app data into the rest of header block 
        x_memcpy((abyTemp + DES_CIPHER_HEADER_SIZE), pbySrcBuffer, dwSrcBufferSize);
        //stuff DES_PAD_BYTE character into the padding byte(s)
        x_memset((abyTemp + (dwTotalByte - dwPaddingSize)), DES_PAD_BYTE, dwPaddingSize);
		//encrypt the DES_INIT_ENCRYPT_BLOCK_SIZE first
		for(dwCount = 0; dwCount < DES_INIT_ENCRYPT_BLOCK_SIZE ; dwCount += DES_BLOCK_SIZE)
		{    
			DES_Cipher( pstuDESKey
						, (pbyCurrentDestBuffer += dwCount)
						, (&abyTemp[dwCount])
						, 1 //encryption flag
						);                
		}
	}
    else
    {
        //put the app data into the rest of header block 
        x_memcpy((abyTemp + DES_CIPHER_HEADER_SIZE), pbySrcBuffer, DES_INIT_ENCRYPT_BLOCK_SIZE - DES_CIPHER_HEADER_SIZE);
        //stuff last block with source buffer and  DES_PAD_BYTE character
        x_memcpy(abyLastBlock, (pbySrcBuffer + dwSrcBufferSize - (DES_BLOCK_SIZE - dwPaddingSize)), (DES_BLOCK_SIZE - dwPaddingSize));
		x_memset((abyLastBlock + (DES_BLOCK_SIZE - dwPaddingSize)), DES_PAD_BYTE, dwPaddingSize);
		
		//encrypting the initial block
		for(dwCount = 0; dwCount < DES_INIT_ENCRYPT_BLOCK_SIZE ; dwCount += DES_BLOCK_SIZE)
		{    
			DES_Cipher( pstuDESKey
						, (pbyCurrentDestBuffer + dwCount)
						, (&abyTemp[dwCount])
						, 1 //encryption flag
						);                
		}
		if(dwSrcBufferSize > (DES_INIT_ENCRYPT_BLOCK_SIZE - DES_CIPHER_HEADER_SIZE + DES_BLOCK_SIZE))
		{
			//resetting the current source buffer to the offset position
			pbyCurrentSrcBuffer = (pbySrcBuffer + (DES_INIT_ENCRYPT_BLOCK_SIZE - DES_CIPHER_HEADER_SIZE));    
	
			pbyCurrentDestBuffer += dwCount;  
			//start encrypting the application data
			//don't encrypt the last block
			for(dwCount = 0; dwCount < (dwTotalByte - DES_INIT_ENCRYPT_BLOCK_SIZE - DES_BLOCK_SIZE); dwCount += DES_BLOCK_SIZE)
			{			
				DES_Cipher( pstuDESKey
							, (pbyCurrentDestBuffer + dwCount)
							, (pbyCurrentSrcBuffer + dwCount)
							, 1 //encryption flag
							);
			}
		}
		//encrypt the last block of data
		DES_Cipher( pstuDESKey
					, (pbyCurrentDestBuffer + dwCount)
					, abyLastBlock
					, 1 //encryption flag
					);
    } 
    return eDES_Success;
}

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
                    )
{
    u32     dwCount, dwTemp32;
    u32     dwTotalByteLeft;

    u32     dwCipherID;
    u16     dwCipherVer, wTemp16;
    u32     dwDataFieldSize;
    u8      *pbyCurrentDestBuffer = pbyDestBuffer;
    u8      *pbyCurrentSrcBuffer = pbySrcBuffer;
    u8      abyTempLastBlock[8];
    u8      abyTemp[DES_BLOCK_SIZE * 2];

    
    //ensure key struct is initialized
    if(DES_KEY_INIT_FLAG != pstuDESKey->byInitFlag)
    {
        *pdwTotalDataBytes = 0;
        return eDES_KeyNotInitialized;
    }
    //return error if no source buffer 
    if(!dwSrcBufferSize)
    {
        *pdwTotalDataBytes = 0;
        return eDES_InvalidParam;
    }

    //check for multiple of DES_BLOCK_SIZE
    if((dwSrcBufferSize % DES_BLOCK_SIZE) || (dwSrcBufferSize < DES_INIT_ENCRYPT_BLOCK_SIZE))
    {
        *pdwTotalDataBytes = 0;
        return eDES_InvalidParam;  
    }
    
    for(dwCount = 0 ; dwCount < DES_INIT_ENCRYPT_BLOCK_SIZE ; dwCount += DES_BLOCK_SIZE)
    {
        DES_Cipher( pstuDESKey
                    , (abyTemp + dwCount)
                    , (pbySrcBuffer + dwCount)
                    , 0 //Decryption flag
                    );        
    }
    
 //   pdwTemp32 = (u32*)abyTemp;
	x_memcpy( &dwTemp32, abyTemp, sizeof( u32 ) );
	dwCipherID = ENDIAN_NetToLocal32(dwTemp32);

//    pwTemp16 = (u16*)(abyTemp + DES_CIPHER_ID_FIELD_SIZE) ;
	x_memcpy( &wTemp16, abyTemp + DES_CIPHER_ID_FIELD_SIZE, sizeof( u16 ) );
	dwCipherVer = ENDIAN_NetToLocal16(wTemp16);

//    pdwTemp32 = (u32*)(abyTemp + DES_CIPHER_ID_FIELD_SIZE + DES_CIPHER_VER_FIELD_SIZE);
	x_memcpy( &dwTemp32, abyTemp + DES_CIPHER_ID_FIELD_SIZE + DES_CIPHER_VER_FIELD_SIZE, sizeof( u32 ) );
	dwDataFieldSize = ENDIAN_NetToLocal32(dwTemp32);

    //making sure that data field size can not be greater than...
    if(dwDataFieldSize > (dwSrcBufferSize - DES_CIPHER_HEADER_SIZE))
    {
        *pdwTotalDataBytes = 0;
        return eDES_GenericError; 
    }
    //check for destination buffer size
    if((dwDestBufferSize < dwDataFieldSize ))
    {        
        *pdwTotalDataBytes = dwDataFieldSize;
        return eDES_DestBufferTooSmall;  
    }

    if((CIPHER_TYPE_BASIC_DES != dwCipherID) || (CIPHER_VER_BASIC_DES != dwCipherVer))
    {
        *pdwTotalDataBytes = 0;
        return eDES_InCorrectVersion;  
    }
    
    if(dwDataFieldSize > (DES_INIT_ENCRYPT_BLOCK_SIZE - DES_CIPHER_HEADER_SIZE))
    {
        x_memcpy(pbyDestBuffer
                , (abyTemp + DES_CIPHER_HEADER_SIZE)
                , (DES_INIT_ENCRYPT_BLOCK_SIZE - DES_CIPHER_HEADER_SIZE)
                );
    }
    else
        x_memcpy(pbyDestBuffer
                , (abyTemp + DES_CIPHER_HEADER_SIZE)
                , dwDataFieldSize 
                );
 
    
    //reset the pDestBuffer to the beginning of the next block of data
    pbyCurrentDestBuffer = (pbyDestBuffer + (DES_INIT_ENCRYPT_BLOCK_SIZE - DES_CIPHER_HEADER_SIZE));

    
    dwTotalByteLeft = dwSrcBufferSize - DES_INIT_ENCRYPT_BLOCK_SIZE;
	if(dwTotalByteLeft)
	{
		//copy DES_INIT_ENCRYPT_BLOCK_SIZE - dwHeaderSize to the dest buffer
		pbyCurrentSrcBuffer = (pbySrcBuffer + DES_INIT_ENCRYPT_BLOCK_SIZE);
        //reusing dwCount
		for(dwCount = 0; dwCount < (dwTotalByteLeft - DES_BLOCK_SIZE) ; dwCount += DES_BLOCK_SIZE)
		{
			DES_Cipher( pstuDESKey
						, (pbyCurrentDestBuffer + dwCount)
						, (pbyCurrentSrcBuffer + dwCount)
						, 0 //Decryption flag
						);
		}
        DES_Cipher( pstuDESKey
					, abyTempLastBlock
					, (pbyCurrentSrcBuffer + dwCount)
					, 0 //Decryption flag
					);
        x_memcpy((pbyCurrentDestBuffer + dwCount)
                , abyTempLastBlock
                , DES_BLOCK_SIZE - (dwSrcBufferSize - dwDataFieldSize - DES_CIPHER_HEADER_SIZE)
                );

	}
    //set the number of byte of application data
    *pdwTotalDataBytes = dwDataFieldSize;
    return eDES_Success;
   
}





///////////////////////////////////////////////////////////////////////////
//
//  X_PLUS.C
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// INCLUDE
///////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_H
#include "x_debug.h"
#endif

#ifndef X_PLUS_H
#include "x_plus.h"
#endif

#ifndef X_MATH_H
#include "x_math.h"
#endif

///////////////////////////////////////////////////////////////////////////

static void x_qsort_qst( char *base, char *max );

///////////////////////////////////////////////////////////////////////////
// RANDOM
///////////////////////////////////////////////////////////////////////////

static long LastRand = 1L;

void x_srand( s32 Seed )
{
    LastRand = Seed;
}

s32  x_rand( void )
{
    LastRand = LastRand * 214013L + 2531011L;
    return( (LastRand >> 16) & X_RAND_MAX );
}

s32  x_rand_int( s32 min, s32 max )
{
    ASSERT(max>min);
    return( (x_rand() % (max-min+1)) + min );
}

f32  x_rand_float( f32 min, f32 max )
{
    ASSERT(max>min);
    return( (((f32)x_rand() / (f32)X_RAND_MAX) * (max-min)) + min );
}

///////////////////////////////////////////////////////////////////////////
// STRING
///////////////////////////////////////////////////////////////////////////

char* x_strncpy ( char* dest, const char* source, s32 count )
{
    char* start = dest;

    ASSERT( dest );
    ASSERT( source );
    ASSERT( (count < 1024) && (count>=0) );

    while (count && (*dest++ = *source++))
        count--;

    if (count)
        while (--count)
            *dest++ = '\0';

    return(start);
}

///////////////////////////////////////////////////////////////////////////

char* x_strstr( const char* MainStr, const char* SubStr )
{
    char* str;
    char* substr;

    ASSERT( MainStr );
    ASSERT( SubStr  );

    str = (char*)MainStr;

    while( *str )
    {
		char* t;

        while( *str && (*str != *SubStr) ) str++;

        for (t = str, substr=(char*)SubStr; *substr && (*str == *substr); str++, substr++ );

        if ( *substr == 0 ) return t;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////

s32 x_strlen ( const char* str )
{
    const char *eos = str;

    ASSERT(str);

    while( *eos++ );

    return( (s32)(eos - str - 1) );
}

///////////////////////////////////////////////////////////////////////////

char* x_strcat ( char* dst, const char* src )
{
    char* cp = dst;

    ASSERT(dst);
    ASSERT(src);

    while( *cp ) cp++;
    while( (*cp++ = *src++) )
        ;
    return( dst );
}

///////////////////////////////////////////////////////////////////////////

char* x_strcpy ( char* dst, const char* src )
{
    char* cp = dst;

    ASSERT(dst);
    ASSERT(src);

    while( (*cp++ = *src++) )
        ;
    return( dst );
}

///////////////////////////////////////////////////////////////////////////

char* x_strncat ( char* front, const char* back, s32 count )
{
    char *start = front;

    ASSERT(front);
    ASSERT(back);
    ASSERT((count>0) && (count<1024));

    while (*front++)
        ;
    front--;
    while (count--)
        if (!(*front++ = *back++))
            return(start);
    *front = '\0';
    return(start);
}

///////////////////////////////////////////////////////////////////////////

s32 x_strncmp ( const char* src0, const char* src1, s32 count )
{

    ASSERT(src0);
    ASSERT(src1);
    ASSERT((count>0) && (count<1024));

    if (!count) return(0);

    while (--count && *src0 && *src0 == *src1)
    {
        src0++;
        src1++;
    }
    return(s32)( *(unsigned char *)src0 - *(unsigned char *)src1 );
}

///////////////////////////////////////////////////////////////////////////

s32 x_strcmp ( const char* src0, const char* src1 )
{
    s32 ret = 0 ;

    ASSERT(src0);
    ASSERT(src1);

    while( ! (ret = *(unsigned char *)src0 - *(unsigned char *)src1) && *src1 )
        ++src0, ++src1;

    return( ret );
}

///////////////////////////////////////////////////////////////////////////

s32 x_stricmp ( const char* src0, const char* src1 )
{
    s32 f,l;

    ASSERT(src0);
    ASSERT(src1);

    do
    {
        if ( ((f = (unsigned char)(*(src1++))) >= 'A') && (f <= 'Z') )
            f -= ('A' - 'a');

        if ( ((l = (unsigned char)(*(src0++))) >= 'A') && (l <= 'Z') )
            l -= ('A' - 'a');

    } while ( f && (f == l) );

    return(l - f);
}

///////////////////////////////////////////////////////////////////////////

char* x_strrchr( const char* str, s32 c )
{
    char* start = (char*)str;

    // Find end of string.
    while( *str++ )
        ;

    // Search from back towards front.
    while( (--str != start) && (*str != (char)c) )
        ;

    // Did we find it?
    if( *str == (char)c )
        return( (char*)str );
    else
        return( NULL );
}

///////////////////////////////////////////////////////////////////////////

char* x_strtoupper ( char* src )
{
    char* s = src;

    if (s==NULL) return NULL;
    ASSERT(src);

    while (*s != 0)
    {
        if ( (*s >= 'a') && (*s <= 'z') )
            *s -= 32;
        s++;
    }

    return src;
}

///////////////////////////////////////////////////////////////////////////

char* x_strtolower ( char* src )
{
    char* s = src;

    if (s==NULL) return NULL;
    ASSERT(src);

    while (*s != 0)
    {
        if ( (*s >= 'A') && (*s <= 'Z') )
            *s += 32;
        s++;
    }

    return src;
}

///////////////////////////////////////////////////////////////////////////
// ASCII
///////////////////////////////////////////////////////////////////////////

s32 x_toupper( s32 c )
{
    if( (c >= 'a') && (c <= 'z') )
        c -= 32;
    return( c );
}

///////////////////////////////////////////////////////////////////////////

s32 x_tolower( s32 c )
{
    if( (c >= 'A') && (c <= 'Z') )
        c += 32;
    return( c );
}

///////////////////////////////////////////////////////////////////////////
// BLOCK MEMORY OPERATIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================
// x_memmove
//=========================================================================
void* x_memmove( void* dest, const void* src, s32 count )
{
	register byte* To;
	register byte* From;
	s32   t;

    ASSERT(dest);
    ASSERT(src);
    ASSERT(count >= 0);

#ifdef TARGET_N64
    ASSERT( (((u32)dest) & 0x00FFFFFF) < 8*1024*1024 );
    ASSERT( count < 2*1024*1024 );
    ASSERT( (((u32)dest) & 0x00FFFFFF)+count-1 < 8*1024*1024 );
#endif

    To   = (byte*)dest;
    From = (byte*)src;

    // check if we have to do something
	if (count == 0 || To == From) return (dest);

	if ((u32)To < (u32)From)
    {
        //------------------
        // Copy forward.
        //------------------

		t = (s32)From;	// only need low bits

		if ( (t | (s32)To) & (sizeof(s32)-1) )
        {
            // Try to align operands.  This cannot be done
            // unless the low bits match.

			if ((t ^ (s32)To) & (sizeof(s32)-1) || count < (s32)sizeof(s32))
            {
                t = count;
            }
			else
            {
                t = sizeof(s32) - ( t &  (sizeof(s32)-1) );
            }

			count -= t;

            do
            {
                *To++ = *From++;
            } while (--t);
		}

        //--------------------------------------------------
		// Copy whole words, then mop up any trailing bytes.
        //--------------------------------------------------

		t = count / sizeof(s32);

        // Try to copy s32
        if (t)
        {
            do
            {
                *(s32*)To = *(s32*)From;
                From += sizeof(s32);
                To += sizeof(s32);
            } while (--t);

        }

		t = count & (sizeof(s32)-1);

        // Copy the bytes
        if (t)
        {
            do
            {
                *To++ = *From++;
            } while (--t);

        }
	}
    else
    {
		//--------------------------------------------------
		//  Copy backwards.  Otherwise essentially the same.
		//  Alignment works as before, except that it takes
		//  (t&wmask) bytes to align, not sizeof-(t&wmask).
		//--------------------------------------------------
		From += count;
		To += count;

		t = (s32)From;

		if ((t | (s32)To) & (sizeof(s32)-1) )
        {
			if ((t ^ (int)To) & (sizeof(s32)-1) || count <= (s32)sizeof(s32))
            {
				t = count;
            }
			else
            {
				t &= (sizeof(s32)-1);
            }

			count -= t;

            do
            {
                *--To = *--From;
            } while (--t);
		}

		t = count / sizeof(s32);

        // Try to copy s32
        if (t)
        {
            do
            {
                From -= sizeof(s32);
                To -= sizeof(s32);
                *(s32*)To = *(s32*)From;

            } while (--t);
        }

		t = count & (sizeof(s32)-1);

        // copy bytes
        if (t)
        {
            do
            {
                *--To = *--From;
            } while (--t);
        }
	}

	return( dest );
}

//=========================================================================
// x_chksum
//-------------------------------------------------------------------------
// This is a ZModem's 32-bit CRC.  Note that the input and the output are 
// actually the inverse of the typical Zmodem's CRC.  That is because most 
// CRC initial values are 0 instead of the Zmodem which is ~0. This way it 
// is more compliant.  CRC generating polynomial is x^32 + x^26 + x^23 + 
// x^22 + x^16 + x^12 + x^11 + x^11 + x^10 + x^8 + x^7 + x^5 + x^5 + x^4 + 
// x^2 + x^1 + x^0.  With binary representation 0x104c11db7, ZModem 
// computes the 32bit CRC 'backwards', so it actually uses a binary 
// representation of 0xEDB88320 (the x32 bit is implied).  This should miss 
// one error out of 2^32. 
// This code comes from "Serial Protocols" by Tim Kientzle. Page 276, 277.
//=========================================================================

u32 x_chksum( const void* Buf, s32 Count, u32 CRC )
{
    s32 Table[256];
    u32 i, j, crc;

    //----------------------------------------------------------
    // Create table
    //----------------------------------------------------------

    for ( i = 0; i < 256; i++ )
    {
        crc = i;

        for ( j = 0; j < 8; j++ )
        {
            crc = (crc>>1) ^ ((crc&1) ? (u32)0xEDB88320 : 0 );
        }

        Table[i] = crc & 0xFFFFFFFF;
    }

    //----------------------------------------------------------
    // Do CRC
    //----------------------------------------------------------
    {
        byte* p;
        byte* pend;

        // Invert it so we can be complient with other types of CRCs
        CRC  = ~CRC;

        // Set up variables
        p    = (byte*)Buf;
        pend = &p[Count];

        // compute the new CRC
        while ( p < pend )
        {
            CRC = Table[ ( CRC ^ *p++) & 0xFF ] ^ ((CRC>>8) & 0xffffff );
        }

        // Invert it Back so next time we will be okay
        CRC = ~CRC;
    }

    return CRC;
}


///////////////////////////////////////////////////////////////////////////

void*   x_memcpy    ( void* dest, const void* src, s32 count )
{
    ASSERT(dest);
    if (count==0) return dest;
    ASSERT(src);
    ASSERT( count >= 0 );

    ASSERTS( ((dest < src) && ((&((byte*)dest)[count]) <=   ((byte*)src)        )) ||
             ((dest > src) && (  ((byte*)dest)         >= (&((byte*)src)[count]))),
             "x_memcpy: Does not support overlapping regions. ANSI LAW." );

    return x_memmove( dest, src, count );
}

///////////////////////////////////////////////////////////////////////////

void*   x_memset    ( void* dest, byte c, s32 count )
{
    register byte* pdst;
    register byte* pdstend;
    register u32   c4;

    pdst        = (byte*)dest;
    pdstend     = pdst + count;
    c4          = (((s32)c)<<24) |
                  (((s32)c)<<16) |
                  (((s32)c)<< 8) |
                  (((s32)c)<< 0);

    // write starting bytes
    while ((pdst < pdstend) && (((u32)pdst) & 0x03))
        *pdst++ = c;

    // write 4bytes at a time
    while ((pdst+4 < pdstend))
        *((u32*)pdst)++ = c4;

    // write ending bytes
    while (pdst < pdstend)
        *pdst++ = c;

    return dest;
}

///////////////////////////////////////////////////////////////////////////
//
// UNCLEAN - Needed for GCC when it sets some large localized variables.
//
///////////////////////////////////////////////////////////////////////////
#ifndef TARGET_PSX
#ifndef _MSC_VER

void* memset( void* dest, int c, unsigned int count )
{
	return x_memset(dest,c,count);
}

#endif
#endif
///////////////////////////////////////////////////////////////////////////

void* x_memchr( void* buf, s32 chr, s32 cnt )
{
    while ( cnt && (*((u8*)buf) != (u8)chr) )
    {
        buf = ((u8*)buf) + 1;
        cnt--;
    }

    return(cnt ? (void*)buf : NULL);
}

///////////////////////////////////////////////////////////////////////////

//=========================================================================
// x_atoi
//=========================================================================
s32 x_atoi( const char* Str )
{
    char c;                          // current char
    char sign;                       // if '-', then negative, otherwise positive
    s32  total;                      // current total

    ASSERT( Str );

    // skip whitespace
    for ( ; *Str == ' '; ++Str );

     // save sign indication
    c = *Str++;
    sign = c;

    // skip sign
    if ( c == '-' || c == '+' )
    {
        c = *Str++;
    }

    total = 0;

    while ( (c >= '0') && ( c <= '9' ) )
    {
        // accumulate digit
        total = 10 * total + (c - '0');

        // get next char
        c = *Str++;
    }

    // negate the total if negative
    if ( sign == '-' ) return -total;

    return total;
}

//=========================================================================
// x_atof
//-------------------------------------------------------------------------
// it will handle numbers such "123.4567e+34"
//=========================================================================
f64 x_atof( const char* Str )
{
    f64     Total1 = 0;
    f64     Total2 = 0;
    f64     Mult   = 1;
    f64     Sign1  = 1;

    ASSERT( Str );

    //----------------------------------------------------
    // skip whitespace
    //----------------------------------------------------
    for ( ; *Str == ' '; ++Str );

    //----------------------------------------------------
    // Get the sign
    //----------------------------------------------------
    if (*Str== '-') {Str++; Sign1 = -1; }
    if (*Str== '+') Str++;

    //----------------------------------------------------
    // Start computer integer portion
    //----------------------------------------------------
    Total1 = 0;

    while ( (*Str >= '0') && ( *Str <= '9' ) )
    {
        // accumulate digit
        Total1 = 10 * Total1 + (*Str - '0');

        // get next char
        Str++;
    }

    //----------------------------------------------------
    // Handle decimals
    //----------------------------------------------------
    if ( *Str == '.' )
    {
        //----------------------------------------------------
        // Skip the decimal point
        //----------------------------------------------------
        Str++;

        //----------------------------------------------------
        // Get all the decimals
        //----------------------------------------------------
        Total2 = 0;

        while ( (*Str >= '0') && ( *Str <= '9' ) )
        {
            // accumulate digit
            Total2 = 10 * Total2 + (*Str - '0');

            // get next char
            Str++;

            // Acumulate the mults
            Mult *= 10;
        }
    }

    //----------------------------------------------------
    // Handle the scientific notation
    //----------------------------------------------------
    if ((*Str == 'e') || (*Str == 'E'))
    {
        f64     Sign2 = 1;
        f64     Total3;
        f64     e;

        //----------------------------------------------------
        // Skip the e
        //----------------------------------------------------
        Str++;

        //----------------------------------------------------
        // Get the sign2
        //----------------------------------------------------
        if (*Str== '-') {Str++; Sign2 = -1; }
        if (*Str== '+') Str++;

        //----------------------------------------------------
        // get exponent
        //----------------------------------------------------
        Total3 = 0;

        while ( (*Str >= '0') && ( *Str <= '9' ) )
        {
            // accumulate digit
            Total3 = 10 * Total3 + (*Str - '0');

            // get next char
            Str++;
        }

        //----------------------------------------------------
        // Compute exponent. We make a shortcut for small 
        // exponents.
        //----------------------------------------------------
        if ( Total3 < 30 )
        {
            e = 1;
            while(Total3) 
            {
                e *= 10;
                Total3--;
            }
        }
        else
        {
            e = x_pow( 10, Total3 );
        }

        //----------------------------------------------------
        // Return final number
        //----------------------------------------------------
        if ( Sign2 < 0) return (Sign1 * ( Total1 + Total2 / Mult )) / e;

        return (Sign1 * ( Total1 + Total2 / Mult )) * e;
    }

    return Sign1 * ( Total1 + Total2 / Mult );
}

//=========================================================================
// x_memcmp
//=========================================================================
s32 x_memcmp( const void* buf1, const void* buf2, s32 count )
{
    byte*   pBuf1;
    byte*   pBuf2;
    u32     pdstend;

    //----------------------------------------------------
    // ASSERTS
    //----------------------------------------------------
    ASSERT( buf1 );
    ASSERT( buf2 );
    ASSERT( count > 0 );

    //----------------------------------------------------
    // Setup
    //----------------------------------------------------
    pBuf1       = (byte*)buf1;
    pBuf2       = (byte*)buf2;
    pdstend     = (u32)pBuf1 + count;

    //----------------------------------------------------
    // comapare align buffers
    //----------------------------------------------------
    if ( ( count >= 4 ) && !((u32)pBuf1 & 3) && !((u32)pBuf2 & 3) )
    {
        pdstend -= 4;
        while ( (u32)pBuf1 <= pdstend )
        {
            if ( *((u32*)pBuf1) != *((u32*)pBuf2) )
            {
                pBuf1 -= 4;
                pBuf2 -= 4;
                break;
            }

            pBuf1 += 4;
            pBuf2 += 4;
        }
        pdstend += 4;
    }

    //----------------------------------------------------
    // compare bytes
    //----------------------------------------------------
    while ( (u32)pBuf1 < pdstend )
    {
        if ( *pBuf1 != *pBuf2 )
        {
            // return the value expected
            if ( *pBuf1 < *pBuf2 ) return -1;
            return 1;
        }

        pBuf1 ++;
        pBuf2 ++;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////
// PATH MANIPULATION
///////////////////////////////////////////////////////////////////////////

//
// Split path implementation based on Microsoft's implementation.
//

void x_splitpath( const char* path, char* drive,
                                    char* dir,
                                    char* fname,
                                    char* ext )
{
    char* p;
    char* last_slash    = NULL;
    char* dot           = NULL;
    s32   len;

    // Extract drive letter and ':', if any.

    if( (x_strlen(path) >= X_MAX_DRIVE-2) &&
        (path[X_MAX_DRIVE-2] == ':') )
    {
        if( drive != NULL )
        {
            x_strncpy( drive, path, X_MAX_DRIVE-1 );
            drive[X_MAX_DRIVE-1] = '\0';
        }
        path += X_MAX_DRIVE-1;
    }
    else
    if ( drive )
    {
        // No drive.
        drive[0] = '\0';
    }

    // Extract path string, if any.  Path now points to first character
    // of the path, if present, or the filename or even the extension.
    // Look ahead for the last path separator ('\' or '/').  If none is
    // found, there is no path.  Also note the location of the last '.',
    // if any, to assist in isolating the extension later.

    for( p = (char*)path; *p; ++p )
    {
        if( (*p == '/') || (*p == '\\') )
            last_slash = p+1;
        else
        if( *p == '.' )
            dot = p;
    }

    if ( last_slash != NULL )
    {
        // There is a path.  Copy up to last slash or X_MAX_DIR chars.
        if ( dir != NULL )
        {
            len = MIN( (last_slash - path), X_MAX_DIR-1 );
            x_strncpy( dir, path, len );
            dir[len] = '\0';
        }
        path = last_slash;
    }
    else
    if ( dir != NULL )
    {
        // No path.
        dir[0] = '\0';
    }

    // Extract file name and extension, if any.  Path now points to the first
    // character of the file name, if any, or to the extension.  Dot points
    // to the beginning of the extension, if any.

    if ( (dot != NULL) && (dot >= path) )
    {
        // Found the marker for the extension.  Copy the file name up to
        // the '.' marker.

        if ( fname != NULL )
        {
            len = MIN( (dot - path), X_MAX_FNAME-1 );
            x_strncpy( fname, path, len );
            fname[len] = '\0';
        }

        // Now get the extension.  Note that p still points to the null which
        // terminates the path.

        if( ext != NULL )
        {
            len = MIN( (p - dot), X_MAX_EXT-1 );
            x_strncpy( ext, dot, len );
            ext[len] = '\0';
        }
    }
    else
    {
        if ( fname != NULL )
        {
            len = MIN( (p - path), X_MAX_FNAME );
            x_strncpy( fname, path, len );
            fname[len] = '\0';
        }
        if ( ext != NULL )
        {
            ext[0] = '\0';
        }
    }
}

///////////////////////////////////////////////////////////////////////////

//
// Make path implementation based on Microsoft's implementation.
//

void x_makepath( char* path,  const char* drive,
                              const char* dir,
                              const char* fname,
                              const char* ext )
{
    const char* p;

    // Copy the drive specification, if given.

    if ( drive && *drive )
    {
        *path++ = *drive;
        *path++ = ':';
    }

    // Copy the directory specification, if given.

    if ( dir && *dir )
    {
        p = dir;
        do
        {
            *path++ = *p++;
        }
        while( *p );

        p--;
        if ( (*p != '/') && (*p != '\\') )
        {
            *path++ = '\\';
        }
    }

    // Copy the fname.

    if ( fname )
    {
        p = fname;
        while ( *p )
        {
            *path++ = *p++;
        }
    }

    // Copy the extension.
    // Add the '.' marker if not given.

    if ( ext )
    {
        p = ext;
        if( *p && (*p != '.') )
        {
            *path++ = '.';
        }
        while ( *p )
        {
            *path++ = *p++;
        }
    }

    // Terminate the string.
    *path = '\0';
}

///////////////////////////////////////////////////////////////////////////
// MUTEX
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64

void InitMutexInfo      ( mutex_info* MI )
{
    osCreateMesgQueue(&(MI->MsgQ), &(MI->Msg[0]), 1);
    osSendMesg(&(MI->MsgQ), (OSMesg)1, OS_MESG_BLOCK);
}

void EnterMutexSection  ( mutex_info* MI )
{
    osRecvMesg(&(MI->MsgQ), &(MI->Msg[0]), OS_MESG_BLOCK);
}

void ExitMutexSection   ( mutex_info* MI )
{
    osSendMesg(&(MI->MsgQ), (OSMesg)1, OS_MESG_BLOCK);
}

#endif

///////////////////////////////////////////////////////////////////////////

xbool x_bsearch ( s32* Index, const void* FirstItem, s32 NItems,
                 const void* DesiredItem, s32 ItemSize, compare_fnptr Compare )
{
    s32   To;
    s32   From;
    s32   Pos;
    s32   Res;
    byte* Src;

    ASSERT( ItemSize > 0 );
    ASSERT( FirstItem    );
    ASSERT( DesiredItem  );
    ASSERT( Compare      );

    //-----------------------------------------------------------
    // Initialize Variables
    //-----------------------------------------------------------
    From = 0;
    Pos  = 0;
    To   = NItems - 1;

    if (Index)
        *Index = 0;

    if (NItems == 0)
        return FALSE;

    //-----------------------------------------------------------
    // Search for the Node
    //-----------------------------------------------------------
    while ( From <= To )
    {
        Pos = ( To - From ) >> 1;
        Src = ((byte*)FirstItem ) + ( From + Pos ) * ItemSize;

        Res = Compare( DesiredItem, Src );

        if ( Res >= 0 )
        {
            if ( Res == 0 )
            {
                //-----------------------------------------------------
                // Make Sure to Find the First Item of the equivanlency
                // so we will search linearly to find the first one.
                //-----------------------------------------------------
                for ( Src -= ItemSize; Src > (byte*)FirstItem ; Src -= ItemSize )
                {
                    // Is a diferent node?
                    if ( Compare( DesiredItem, Src ) )
                        break;

                    // Set new Index
                    Pos--;
                }

                //-----------------------------------------------------
                // Set Index and return
                //-----------------------------------------------------
                if ( Index )
                    *Index = (From + Pos);

                return TRUE;
            }

            From += (Pos+1);
        }
        else
            To -= (Pos+1);
    };

    //-----------------------------------------------------
    // Set Index and return
    //-----------------------------------------------------
    if ( Index )
        *Index = (From + Pos);

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  QSORT 
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Our version of the system qsort routine which faster by an average of 
// 25%, with lows and highs of 10% and 50%.  The THRESHold below is the 
// insertion sort threshold, and has been adjusted for records of size 48
// bytes.  The MTHREShold is where we stop finding a better median.

#define THRESH  4		                // threshold for insertion
#define MTHRESH 6		                // threshold for median

static int qsz;			                // size of each record
static int thresh;		                // THRESHold in chars
static int mthresh;		                // MTHRESHold in chars
static compare_fnptr qcmp;              // the comparison routine

//==========================================================================
// x_qsort_qst
//--------------------------------------------------------------------------
// Do a quicksort.  First, find the median element, and put that one in
// the first place as the discriminator.  (This "median" is just the median
// of the first, last and middle elements).  (Using this median instead of
// the first element is a big win). Then, the usual partitioning/swapping,
// followed by moving the discriminator into the right place.  Then, figure
// out the sizes of the two partions, do the smaller one recursively and the
// larger one via a repeat of this code.  Stopping when there are less than
// THRESH elements in a partition and cleaning up with an insertion sort (in
// our caller) is a huge win. All data swaps are done in-line, which is
// space-losing but time-saving. (And there are only three places where this
// is done).
//==========================================================================

static
void x_qsort_qst( char *base, char *max )
{
    register char* i;
    register char* j;
    register char* jj;
    register char* mid;
    register int   ii;
    register char  c;
    char*   tmp;
    int     lo;
    int     hi;

    lo = (int)(max - base);		// number of elements as chars

    do
    {
        //------------------------------------------------------------------
        // At the top here, lo is the number of characters of elements in the
        // current partition.  (Which should be max - base). Find the median
        // of the first, last, and middle element and make that the middle
        // element.  Set j to largest of first and middle.  If max is larger
        // than that guy, then it's that guy, else compare max with loser of
        // first and take larger.  Things are set up to prefer the middle,
        // then the first in case of ties.
        //------------------------------------------------------------------

        mid = i = base + qsz * ((unsigned) (lo / qsz) >> 1);

	    if (lo >= mthresh)
        {
	        j = ((*qcmp) ((jj = base), i) > 0 ? jj : i);

	        if ( (*qcmp) (j, (tmp = max - qsz)) > 0 )
            {
		        // switch to first loser //
		        j = (j == jj ? i : jj);
		        if ((*qcmp) (j, tmp) < 0) j = tmp;
	        }

	        if (j != i)
            {
		        ii = qsz;
		        do
                {
		            c = *i;
		            *i++ = *j;
		            *j++ = c;

		        } while (--ii);
	        }
	    }

	    // Semi-standard quicksort partitioning/swapping
	    for ( i = base, j = max - qsz; ; )
        {
	        while (i < mid && (*qcmp) (i, mid) <= 0) i += qsz;

	        while (j > mid)
            {
		        if ((*qcmp) (mid, j) <= 0)
                {
		            j -= qsz;
		            continue;
		        }

		        tmp = i + qsz;	       // value of i after swap

		        if (i == mid)
                {	                    // j <-> mid, new mid is j
		            mid = jj = j;
		        }
                else
                {	       // i <-> j
		            jj = j;
		            j -= qsz;
		        }

		        goto swap;
	        }

	        if (i == mid)
            {
		        break;
            }
            else
            {		       // i <-> mid, new mid is i
		        jj = mid;
		        tmp = mid = i;	       // value of i after swap
		        j -= qsz;
	        }
swap:
    	    ii = qsz;
	        do
            {
		        c = *i;
		        *i++ = *jj;
		        *jj++ = c;

	        } while (--ii);

	        i = tmp;
	    }

        //------------------------------------------------------------------
        // Look at sizes of the two partitions, do the smaller one first by
        // recursion, then do the larger one by making sure lo is its size,
        // base and max are update correctly, and branching back. But only
        // repeat (recursively or by branching) if the partition is of at
        // least size THRESH.
        //------------------------------------------------------------------

    	i = (j = mid) + qsz;

	    if ((lo = (int)(j - base)) <= (hi = (int)(max - i)))
        {
	        if (lo >= thresh) x_qsort_qst(base, j);

	        base = i;
	        lo = hi;
	    }
        else
        {
	        if (hi >= thresh) x_qsort_qst(i, max);
	        max = j;
	    }

    } while (lo >= thresh);
}

//==========================================================================
// x_qsort
//--------------------------------------------------------------------------
// First, set up some global parameters for x_qsort_qst to share.  Then, 
// quicksort with x_qsort_qst(), and then a cleanup insertion sort 
// ourselves.  Sound simple?  It's not...
//==========================================================================

void x_qsort( void* Base, s32 n, s32 size, compare_fnptr compar )
{
    register char* i;
    register char* j;
    register char* lo;
    register char* hi;
    register char* min;
    register char c;
    char*   max;
    char*   base = Base;

    if (n <= 1)	return;

    qsz     = size;
    qcmp    = compar;
    thresh  = qsz * THRESH;
    mthresh = qsz * MTHRESH;
    max     = base + n * qsz;

    if (n >= THRESH)
    {
	    x_qsort_qst(base, max);
	    hi = base + thresh;
    }
    else
    {
	    hi = max;
    }

    //------------------------------------------------------------------
    // First put smallest element, which must be in the first THRESH, in the
    // first position as a sentinel.  This is done just by searching the
    // first THRESH elements (or the first n if n < THRESH), finding the min,
    // and swapping it into the first position.
    //------------------------------------------------------------------

    for (j = lo = base; (lo += qsz) < hi; )
    {
	    if ( (*qcmp) (j, lo) > 0 ) j = lo;
    }

    // swap j into place
    if (j != base)
    {
	    for ( i = base, hi = base + qsz; i < hi; )
        {
	        c    = *j;
	        *j++ = *i;
	        *i++ = c;
    	}
    }

    //------------------------------------------------------------------
    // With our sentinel in place, we now run the following hyper-fast
    // insertion sort. For each remaining element, min, from [1] to [n-1],
    // set hi to the index of the element AFTER which this one goes. Then, do
    // the standard insertion sort shift on a character at a time basis for
    // each element in the frob.
    //------------------------------------------------------------------
    for ( min = base; (hi = min += qsz) < max; )
    {
	    while ( (*qcmp) (hi -= qsz, min) > 0 );

	    if ((hi += qsz) != min)
        {
	        for ( lo = min + qsz; --lo >= min; )
            {
		        c = *lo;
		        for (i = j = lo; (j -= qsz) >= hi; i = j)
                {
		            *i = *j;
                }
		        *i = c;
    	    }
	    }
    }
}

///////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
//  X_PLUS.CPP
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "x_memory.hpp"
#endif

#ifndef X_MATH_HPP
#include "x_math.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

//==========================================================================
//  
//  Quick Sort
//  
//  This implementation of the quick sort algorithm is, on average, 25% 
//  faster than most "system" versions.  The improvement ranges from a low
//  of 10% to a high of 50%.
//  
//  Two functions are used to accomplish the sort: x_qsort and 
//  PseudoQuickSort.  
//  
//  The PseudoQuickSort function (abbreviated PQS) will only "mostly" sort
//  a given array of record.  It is recursive, but it is guaranteed that
//  the function will not recurse more than log base 2 of n times.
//  
//  The x_qsort function will first perform some set up.  It then optionally
//  invokes the PQS function.  Finally, it will perform an optimized 
//  insertion sort to finish of the sort process.
//  
//==========================================================================

// Two thresholds are used to optimize the algorithm.  These values are 
// currently tuned for records having an average size of 48 bytes.

#define THRESH   4      //  Records needed to invoke PQS.
#define MTHRESH  6      //  Records needed to bother seeking partition key.

// A few "globals" are used to communicate between x_qsrt and PQS.

static s32           QRecSize;      // Size of one record.
static s32           QThresh;	    // THRESH  * QRecSize
static s32           QMThresh;	    // MTHRESH * QRecSize
static compare_fnptr QCompare;      // Comparison function.

// Anounce the PQS function.

static void PseudoQuickSort( byte* pBase, byte* pMax );

//--------------------------------------------------------------------------

void x_qsort( const void*     apBase,     
              s32             NItems,    
              s32             ItemSize,  
              compare_fnptr   Compare )  
{
    byte  c;
    byte* i;
    byte* j;
    byte* lo;
    byte* hi;
    byte* min;
    byte* pMax;
    byte* pBase = (byte*)apBase;

    // Easy out?
    if( NItems <= 1 )
        return;

    // Set up some "global" values.
    QRecSize = ItemSize;
    QCompare = Compare;
    QThresh  = QRecSize * THRESH;
    QMThresh = QRecSize * MTHRESH;
    pMax     = pBase + (NItems * QRecSize);

    //
    // Set the 'hi' value.
    // Also, if there are enough values, call the PseudoQuickSort function.
    //
    if( NItems >= THRESH )
    {
        PseudoQuickSort( pBase, pMax );
        hi = pBase + QThresh;
    }
    else
    {
        hi = pMax;
    }

    //
    // First, find the smallest element in the first "MIN(THRESH,NItems)"
    // items.  The smallest element in the entire list is guaranteed to be
    // present in this sublist.
    //
    for( j = lo = pBase; (lo += QRecSize) < hi;  )
    {
        if( QCompare( j, lo ) > 0 )
            j = lo;
    }

    // 
    // Now put the smallest item in the first position to prime the next 
    // loop.
    //
    if( j != pBase )
    {
        for( i = pBase, hi = pBase + QRecSize; i < hi;  )
        {
            c    = *j;
            *j++ = *i;
            *i++ = c;
        }
    }

    //
    // Smallest item is in place.  Now we run the following hyper-fast
    // insertion sort.  For each remaining element, min, from [1] to [n-1],
    // set hi to the index of the element AFTER which this on goes.  Then,
    // do the standard insertion sort shift on a byte at a time basis.
    //
    for( min = pBase; (hi = min += QRecSize) < pMax;  )
    {
        while( QCompare( hi -= QRecSize, min ) > 0 )
        {
            // No body in this loop.
        }        

        if( (hi += QRecSize) != min )
        {
            for( lo = min + QRecSize; --lo >= min;  )
            {
                c = *lo;
                for( i = j = lo; (j -= QRecSize) >= hi; i = j )
                {
                    *i = *j;
                }
                *i = c;
            }
        }
    }
}

//--------------------------------------------------------------------------
//
//  PseudoQuickSort?  What is that?  Its "sorta quick sort".
//
//  First, find a partition element and put it in the first position of the
//  list.  The partition value is the median value of the first, middle, and
//  last records.  (Using the median value of these three records rather 
//  than just using the first record is a big win.)
//
//  Then, the usual partitioning and swapping, followed by swapping the 
//  partition element into place.
//
//  Then, of the two portions of the list which remain (one on either side
//  of the partition), sort the smaller portion recursively, and sort the
//  larger portion via another iteration of the same code.
//
//  Do not bother "quick sorting" lists (or sub lists) which have fewer than
//  THRESH records.  All final sorting is handled with an insertion sort
//  which is executed by the caller (x_qsort).  This is another huge win.
//  (This means that this function does not actually completely sort the
//  input list.  It mostly sorts it.  That is, each record will be within
//  THRESH positions of its correct position.  Thus, an insert sort will
//  be able to finish off the sort process without a serious performance
//  hit.)
//
//  All data swaps are done in-line, which trades some code space for better
//  performance.  And there are only three swap points, anyway.
//
//--------------------------------------------------------------------------

static 
void PseudoQuickSort( byte* pBase, byte* pMax )
{
    byte* i;
    byte* j;
    byte* jj;
    byte* mid;
    byte* tmp;
    byte  c;
    s32   ii;
    s32   lo;
    s32   hi;

    lo = (s32)(pMax - pBase);   // Total data to sort in bytes.

    // Deep breath now...
    do
    {
        //
        // At this point, lo is the number of bytes in the elements in the
        // current partition.  We want to find the median value record of
        // the first, middle, and last elements.  This median record will 
        // become the middle record.  Set j to the lastest of first and 
        // middle.  If last is larger than j, then j is the median.  
        // Othewise, compare the last record to "the lesser of the first and
        // middle" and take the larger.  Things are set up to prefer the 
        // middle over the first in the event of a tie.
        //

        mid = i = pBase + QRecSize * ((u32)(lo / QRecSize) >> 1);

        if( lo >= QMThresh )
        {
            j = (QCompare( (jj = pBase), i ) > 0)  ?  jj : i;

            if( QCompare( j, (tmp = pMax - QRecSize) ) > 0 )
            {
                // Use lesser of first and middle.  (First loser.)
                j = (j == jj ? i : jj);
                if( QCompare( j, tmp ) < 0 )
                    j = tmp;
            }

            if( j != i )
            {
                // Swap!
                ii = QRecSize;
                do
                {
                    c    = *i;
                    *i++ = *j;
                    *j++ = c;
                } while( --ii );
            }
        }
                
        //
        // Semi-standard quicksort partitioning / swapping...
        //

        for( i = pBase, j = pMax - QRecSize;  ;  )
        {
            while( (i < mid) &&
                   (QCompare( i, mid ) <= 0) )
                i += QRecSize;

            while( j > mid )
            {
                if( QCompare( mid, j ) <= 0 )
                {
                    j -= QRecSize;
                    continue;
                }

                tmp = i + QRecSize;     // Value of i after swap.

                if( i == mid )
                {
                    // j <-> mid, new mid is j.
                    mid = jj = j;       
                }
                else
                {
                    // i <-> j
                    jj = j;
                    j -= QRecSize;
                }

                goto SWAP;
            }

            if( i == mid )
            {
                break;
            }
            else
            {
                jj  = mid;
                tmp = mid = i;
                j  -= QRecSize;
            }
SWAP:
            ii = QRecSize;
            do
            {
                c     = *i;
                *i++  = *jj;
                *jj++ = c;
            } while( --ii );

            i = tmp;
        }

        //
        // Consider the sizes of the two partitions.  Process the smaller
        // partition first via recursion.  Then process the larger partition
        // by iterating through the above code again.  (Update variables as
        // needed to make it work.)
        //
        // NOTE:  Do not bother sorting a given partition, either 
        // recursively or by another iteration, if the size of the partition
        // is less than THRESH records.
        //

        j = mid;
        i = mid + QRecSize;

        if( (lo = (j-pBase)) <= (hi = (pMax-i)) )
        {
            if( lo >= QThresh )
                PseudoQuickSort( pBase, j );

            pBase = i;
            lo    = hi;
        }
        else
        {
            if( hi >= QThresh )
                PseudoQuickSort( i, pMax );
            pMax = j;
        }

    } while( lo >= QThresh );
}

//==========================================================================
// This implementation does NOT have special processing added for duplicate
// items in the list.

void* x_bsearch( const void*    pKey,
                 const void*    pBase,       
                 s32            NItems,      
                 s32            ItemSize,    
                 compare_fnptr  Compare )
{
    byte* pLo;
    byte* pHi;
    byte* pMid;
    s32   Half1;
    s32   Half2;
    s32   Result;
    
    ASSERT( pKey         );
    ASSERT( pBase        );
    ASSERT( NItems   > 0 );
    ASSERT( ItemSize > 0 );
    ASSERT( Compare      );

    pLo = (byte*)pBase;
    pHi = pLo + ((NItems-1) * ItemSize);

    while( pLo <= pHi )
    {
        //
        // If there are at least two items in the list, then we want to do
        // the binary search stuff.
        //
        // But if there is only one item, then just do a single compare and
        // be done with it.  (The "at least two items" code would work, but
        // it is far less efficient.)
        //
        // A "no items in the list" case is impossible based on the nature
        // of the surrounding while loop.
        //

        if( NItems >= 2 )
        {
            Half1  = NItems / 2;
            Half2  = NItems - Half1 - 1;
            pMid   = pLo + (Half1 * ItemSize);
            Result = Compare( pKey, pMid );

            if( Result == 0 )
                return( pMid );

            if( Result < 0 )
            {
                pHi    = pMid - ItemSize;
                NItems = Half1;
            }
            else // Result > 0
            {
                pLo    = pMid + ItemSize;
                NItems = Half2;
            }
        }
        else
        {
            Result = Compare( pKey, pLo );
            if( Result == 0 )
                return( pLo );
            else
                return( NULL );
        }
    }

    return( NULL );
}

//==========================================================================
//  Pseudo random number generation functions.
//==========================================================================

static s32 s_LastRand = 1;

//==========================================================================

void x_srand( s32 Seed )
{
    s_LastRand = Seed;
}

//==========================================================================

s32 x_rand( void )
{
    s_LastRand = s_LastRand * 214013 + 2531011;
    return( (s32)((s_LastRand >> 16) & X_RAND_MAX) );
}

//==========================================================================

s32 x_irand( s32 Min, s32 Max )
{
    ASSERT( Max >= Min );
    return( (x_rand() % (Max-Min+1)) + Min );
}

//==========================================================================

f32 x_frand( f32 Min, f32 Max )
{
    ASSERT( Max >= Min );
    return( (((f32)x_rand() / (f32)X_RAND_MAX) * (Max-Min)) + Min );
}

//==========================================================================
//  Standard NULL terminated string manipulation functions.
//==========================================================================

s32 x_strlen( const char* pStr )
{
    const char *pEnd = pStr;

    ASSERT( pStr );

    while( *pEnd++ )
        ; // empty body

    return( (s32)(pEnd - pStr - 1) );
}

//==========================================================================

char* x_strcpy( char* pDest, const char* pSrc )
{
    char* p = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );

    while( (*p++ = *pSrc++) )
        ; // empty body

    return( pDest );
}

//==========================================================================

char* x_strncpy( char* pDest, const char* pSrc, s32 Count )
{
    char* pStart = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );
    ASSERT( Count >= 0 );

    while( Count && (*pDest++ = *pSrc++) )
        Count--;

    if( Count )
        while( --Count )
            *pDest++ = '\0';

    return( pStart );
}

//==========================================================================

char* x_strcat( char* pFront, const char* pBack )
{
    char* p = pFront;

    ASSERT( pFront );
    ASSERT( pBack  );

    while( *p ) 
        p++;

    while( (*p++ = *pBack++) )
        ; // empty body

    return( pFront );
}

//==========================================================================

char* x_strncat( char* pFront, const char* pBack, s32 Count )
{
    char *pStart = pFront;

    ASSERT( pFront );
    ASSERT( pBack  );
    ASSERT( Count >= 0 );

    while( *pFront++ )
        ; // empty body

    pFront--;

    while( Count-- )
        if( !(*pFront++ = *pBack++) )
            return( pStart );

    *pFront = '\0';

    return( pStart );
}

//==========================================================================

s32 x_strcmp( const char* pStr1, const char* pStr2 )
{
    s32 Result = 0;

    ASSERT( pStr1 );
    ASSERT( pStr2 );

    while( !(Result = ((s32)*pStr1) - ((s32)*pStr2)) && *pStr1 )
        ++pStr1, ++pStr2;

    return( Result );
}                    

//==========================================================================

s32 x_strncmp( const char* pStr1, const char* pStr2, s32 Count )
{

    ASSERT( pStr1 );
    ASSERT( pStr2 );
    ASSERT( Count >= 0 );

    if( !Count )  return( 0 );

    while( --Count && *pStr1 && (*pStr1 == *pStr2) )
    {
        pStr1++;
        pStr2++;
    }

    return( ((s32)*pStr1) - ((s32)*pStr2) );
}

//==========================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode.  (Not so for EBCDIC!)

s32 x_stricmp( const char* pStr1,  const char* pStr2 )
{
    s32 f, l;

    ASSERT( pStr1 );
    ASSERT( pStr2 );

    do
    {
        if( ((l = (s32)(*(pStr1++))) >= 'A') && (l <= 'Z') )
            l -= ('A' - 'a');

        if( ((f = (s32)(*(pStr2++))) >= 'A') && (f <= 'Z') )
            f -= ('A' - 'a');

    } while( f && (f == l) );

    return( l - f );
}

//==========================================================================

char* x_strstr( const char* pMainStr, const char* pSubStr )
{
    char* pM = (char*)pMainStr;
    char* pS1; 
    char* pS2;

    if( !*pSubStr )
        return( (char*)pMainStr );

    while( *pM )
    {
        pS1 = pM;
        pS2 = (char*)pSubStr;

        while( *pS1 && *pS2 && !(*pS1 - *pS2) )
        {
            pS1++;
            pS2++;
        }

        if( !*pS2 )
            return( pM );

        pM++;
    }

    return( NULL );
}

//==========================================================================

char* x_strrchr( const char* pStr, s32 c )
{
    char* pStart = (char*)pStr;

    // Find end of string.
    while( *pStr++ )
        ; // empty body

    // Search from back towards front.
    while( (--pStr != pStart) && (*pStr != (char)c) )
        ; // empty body

    // Did we find it?
    if( *pStr == (char)c )
        return( (char*)pStr );
    else
        return( NULL );
}

//==========================================================================

char* x_strtok( char* pString, const char* pDelimit )
{
	static char* pSavedString = NULL;

	// --- New string to parse
	if (pString)
		pSavedString = pString;

	s32   DelimCount;
	char* pStart     = pSavedString;
	xbool DelimFound = FALSE;

	if (pSavedString)
	{
		// --- Loop until delimiter found
		while ( (!DelimFound) && (*pSavedString != NULL) )
		{
			for (DelimCount=0; DelimCount < x_strlen(pDelimit); DelimCount++)
			{
				// --- Found delimiter! Set it to NULL, and return token
				if (*pSavedString == pDelimit[DelimCount])
				{
					DelimFound    = TRUE;
					*pSavedString = NULL;
				}
			}

			pSavedString++;
		}
	}

	return pStart;
}

//==========================================================================

s32 x_sscanf( const char *in_string, const char *format, ... )
{
	x_va_list ArgList;
	char*     ArgAddr;
	char*     TextString;
	char*     StringPtr;
	s32       NArgs=0;

	// --- No format arguments
	if (!format)
		return 0;

	// --- We will be modifying in_string, so make a copy
	TextString = (char*)x_malloc( x_strlen(in_string)+1 );
	x_strcpy( TextString, in_string );
	StringPtr = x_strtok( TextString, " " );

	// --- Start the variable argument list
	x_va_start( ArgList, format );

	while (*format)
	{
		// --- Found a format specifier!
		if (*format == '%')
		{
			// --- Retrieve the address of the destination variable
			NArgs++;
			ArgAddr = x_va_arg( ArgList, char* );

			// --- Parse the format type
			format++;
			switch (*format)
			{
				case 'c':			// Character
					*ArgAddr = *StringPtr;
					break;

				case 's':			// String
					x_strcpy( ArgAddr, StringPtr );
					break;

				case 'i':			// Signed Integer
				case 'd':			// Signed Integer
				case 'u':			// Unsigned Integer
					*((s32*)ArgAddr) = x_atoi( StringPtr );
					break;

				case 'e':			// Scientific Notation
				case 'g':			// Scientific Notation
				case 'f':			// Single Precision Float
					*((f32*)ArgAddr) = (f32)x_atof( StringPtr );
					break;
			}

			// --- Find the next value from the copy of in_string
			StringPtr = x_strtok( NULL, " " );
			if (!StringPtr)
				return X_EOF;
		}

		format++;
	}

	// --- Minor Cleanup
	x_va_end( ArgList );

	x_free( TextString );

	return NArgs;
}


//==========================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode.  (Not so for EBCDIC!)

char* x_strtoupper( char* pStr )
{
    char* p = pStr;

    ASSERT( pStr );

    while( *p != '\0' )
    {
        if( (*p >= 'a') && (*p <= 'z') )
            *p += ('A' - 'a');
        p++;
    }

    return( pStr );
}

//==========================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode.  (Not so for EBCDIC!)

char* x_strtolower( char* pStr )
{
    char* p = pStr;

    ASSERT( pStr );

    while( *p != '\0' )
    {
        if( (*p >= 'A') && (*p <= 'Z') )
            *p += ('a' - 'A');
        p++;
    }

    return( pStr );
}

//==========================================================================
//  Standard block memory functions.
//==========================================================================
inline void* x_memcpy64( void *pDst, const void* pSrc, u32 Count)
{
    ASSERT( !((reinterpret_cast<u64>(pDst) | reinterpret_cast<u64>(pSrc)) & 0x7) );

    u32 NumDWords( Count >> 3 );
    u32 NumBytes ( Count & 0x7);

    u64 *pD64 = reinterpret_cast<u64*>(pDst);
    const u64 *pS64 = reinterpret_cast<const u64*>(pSrc);

    if (NumDWords)
    {
        u32 NumQWords = NumDWords >> 1;

        if (NumQWords)
        {
            do
            {
                *pD64++ = *pS64++;
                *pD64++ = *pS64++;
            } while (--NumQWords);
        }

        if (NumDWords & 0x1)
        {
            *pD64++ = *pS64++;
        }
    }

    if (NumBytes)
    {
        u8* pD8 = reinterpret_cast<u8*>(pD64);
        const u8* pS8 = reinterpret_cast<const u8*>(pS64);

        do
        {
            *pD8++ = *pS8++;
        } while (--NumBytes);
    }

    return pDst;
}

inline void* x_memcpy32( void *pDst, const void* pSrc, u32 Count)
{
    ASSERT( !((reinterpret_cast<u32>(pDst) | reinterpret_cast<u32>(pSrc)) & 0x3) );

    u32 NumWords( Count >> 2 );
    u32 NumBytes ( Count & 0x3);

    u32 *pD32 = reinterpret_cast<u32*>(pDst);
    const u32 *pS32 = reinterpret_cast<const u32*>(pSrc);

    if (NumWords)
    {
        u32 NumDWords = NumWords >> 1;

        if (NumDWords)
        {
            do
            {
                *pD32++ = *pS32++;
                *pD32++ = *pS32++;
            } while (--NumDWords);
        }

        if (NumWords & 0x1)
        {
            *pD32++ = *pS32++;
        }
    }

    if (NumBytes)
    {
        u8* pD8 = reinterpret_cast<u8*>(pD32);
        const u8* pS8 = reinterpret_cast<const u8*>(pS32);

        do
        {
            *pD8++ = *pS8++;
        } while (--NumBytes);
    }

    return pDst;
}

inline void* x_memcpy8 ( void *pDst, const void* pSrc, u32 Count)
{
    u32 NumWords ( Count >> 2 );
    u32 NumBytes ( Count & 0x3);

    u8 *pD8 = reinterpret_cast<u8*>(pDst);
    const u8 *pS8 = reinterpret_cast<const u8*>(pSrc);

    if (NumWords)
    {
        do
        {
            *pD8++ = *pS8++;
            *pD8++ = *pS8++;
            *pD8++ = *pS8++;
            *pD8++ = *pS8++;
        } while (--NumWords);
    }

    if (NumBytes)
    {
        do
        {
            *pD8++ = *pS8++;
        } while (--NumBytes);
    }

    return pDst;
}

void* x_memcpy( void* pDest, const void* pSorc, s32 Count )
{
    ASSERT( pDest );
    ASSERT( pSorc );
    ASSERT( Count >= 0 );

    ASSERTS( ((pDest < pSorc) && ((&((byte*)pDest)[Count]) <=   ((byte*)pSorc)        )) ||
             ((pDest > pSorc) && (  ((byte*)pDest)         >= (&((byte*)pSorc)[Count]))),
             "x_memcpy does not support overlapping blocks." );

    if (Count == 0)
        return pDest;

    u32 AddressOr ( reinterpret_cast<u32>(pDest) | reinterpret_cast<u32>(pSorc) );
    u32 AddressXor( reinterpret_cast<u32>(pDest) ^ reinterpret_cast<u32>(pSorc) );
    u32 UCount    ( static_cast<u32>(Count) );
    
    u8 *pDst = reinterpret_cast<u8*>(pDest);
    const u8 *pSrc = reinterpret_cast<const u8*>(pSorc);

    u32 NumBytesToAlign;

    //Check alignment of buffers and choose optimum copy function
    if (AddressOr & 0x7)
    {
        if (AddressOr & 0x3)
        {
            if (AddressXor & 0x3)
            {
                //Worst case, must byte copy the entire buffer
                return x_memcpy8(pDst, pSrc, UCount);
            }

            //Buffers can be aligned to 32bit boundaries, so align them
            NumBytesToAlign = 4 - reinterpret_cast<u32>(pSrc) & 0x3;
            ASSERT( NumBytesToAlign );

            if (NumBytesToAlign > UCount)
            {
                ASSERT( UCount );
                NumBytesToAlign = UCount;

                do
                {
                    *pDst++ = *pSrc++;
                } while (--NumBytesToAlign);

                return pDest;
            }

            UCount -= NumBytesToAlign;

            do
            {
                *pDst++ = *pSrc++;
            } while (--NumBytesToAlign);
            //pDst and pSrc are guaranteed to be 32bit aligned now
        }

        if (AddressXor & 0x7)
        {
            //Buffers are word aligned, but can't get dword aligned
            return x_memcpy32(pDst, pSrc, UCount);
        }
        
        //Buffers can be aligned to 64 bit boundaries, so align then copy
        NumBytesToAlign = 8 - reinterpret_cast<u32>(pSrc) & 0x7;

        if (NumBytesToAlign)
        {
            
            if (NumBytesToAlign > UCount)
            {
                ASSERT( UCount );

                NumBytesToAlign = UCount;
                
                do
                {
                    *pDst++ = *pSrc++;
                } while (--NumBytesToAlign);

                return pDest;
            }
            
            UCount -= NumBytesToAlign;

            do
            {
                *pDst++ = *pSrc++;
            } while (--NumBytesToAlign);
            //pDst and pSrc are guaranteed to be 64bit aligned now
        }
    }
    
    return x_memcpy64( pDst, pSrc, UCount );
}

//==========================================================================
// This version of memmove will attempt to move as much memory as possible
// as 32-bit data.

void* x_memmove( void* pDest, const void* pSrc, s32 Count )
{
	byte* pFrom;
	byte* pTo;
	s32   t;
    xbool CopyForward = FALSE;

    ASSERT( pDest );
    ASSERT( pSrc );
    ASSERT( Count >= 0 );

    // See if we have anything to do.
	if( (Count == 0) || (pSrc == pDest) )
        return( pDest );

    pFrom = (byte*)pSrc;
    pTo   = (byte*)pDest;

    // If the data is not in overlapping regions, always copy forward to avoid
    // thrashing the data cache
    if ( ((pDest < pSrc) && ((&((byte*)pDest)[Count]) <=   ((byte*)pSrc)        )) ||
         ((pDest > pSrc) && (  ((byte*)pDest)         >= (&((byte*)pSrc)[Count]))) )
    {
        CopyForward = TRUE;
    }

	if( (pTo < pFrom) || CopyForward )
    {
        //
        // Copy forward.
        //

		t = (s32)pFrom;	// Only need low bits.

		if( (t | (s32)pTo) & (sizeof(s32)-1) )
        {
            // Try to align operands.  This cannot be done unless the low 
            // bits match.

			if( (t ^ (s32)pTo) & (sizeof(s32)-1) || Count < (s32)sizeof(s32) )
            {
                t = Count;
            }
			else
            {
                t = (s32)sizeof(s32) - ( t & ((s32)sizeof(s32)-1) );
            }

			Count -= t;

            do
            {
                *pTo++ = *pFrom++;
            } while( --t );
		}

        //
		// Copy whole words, then mop up any trailing bytes.
        //

		t = Count / (s32)sizeof(s32);

        // Try to copy s32's.
        if( t )
        {
            do
            {
                *(s32*)pTo = *(s32*)pFrom;
                pFrom += sizeof(s32);
                pTo   += sizeof(s32);
            } while( --t );
        }

		t = Count & ((s32)sizeof(s32)-1);

        // Copy bytes.
        if( t )
        {
            do
            {
                *pTo++ = *pFrom++;
            } while( --t );
        }
	}
    else
    {
		//
		// Copy backwards.  Otherwise essentially the same.  Alignment 
        // works as before, except that it takes (t&wmask) bytes to align,
        // not (sizeof-(t&wmask)).
		//

		pFrom += Count;
		pTo   += Count;

		t = (s32)pFrom;

		if( (t | (s32)pTo) & (sizeof(s32)-1) )
        {
			if( ((t ^ (s32)pTo) & (sizeof(s32)-1)) || 
                (Count <= (s32)sizeof(s32)) )
            {
				t = Count;
            }
			else
            {
				t &= (sizeof(s32)-1);
            }

			Count -= t;

            do
            {
                *--pTo = *--pFrom;
            } while( --t );
		}

		t = Count / (s32)sizeof(s32);

        // Try to copy s32's.
        if( t )
        {
            do
            {
                pFrom -= sizeof(s32);
                pTo   -= sizeof(s32);
                *(s32*)pTo = *(s32*)pFrom;

            } while( --t );
        }

		t = Count & ((s32)sizeof(s32)-1);

        // Copy bytes.
        if( t )
        {
            do
            {
                *--pTo = *--pFrom;
            } while( --t );
        }
	}

	return( pDest );
}

//==========================================================================

void* x_memset( void* pBuf, s32 C, s32 Count )
{
    byte* p;
    byte* pEnd;
    u32*  p4;
    u32   C4;

    ASSERT( pBuf );
    ASSERT( Count >= 0 );

    p    = (byte*)pBuf;
    pEnd = p + Count;
    C4   = ( ((u32)C) << 24 ) |
           ( ((u32)C) << 16 ) |
           ( ((u32)C) <<  8 ) |
           ( ((u32)C) <<  0 );

    // Write starting bytes.
    while( (p < pEnd) && (((u32)p) & 0x03) )
        *p++ = (byte)C;

    // Write 4 bytes at a time.
    p4 = (u32*)p;
    while( (p4+1) < (u32*)pEnd )
    {
        *p4++ = C4;
    }
    p = (byte*)p4;

    // Write ending bytes.
    while( p < pEnd )
        *p++ = (byte)C;

    return( pBuf );
}

//==========================================================================

void* x_memchr( void* pBuf, s32 C, s32 Count )
{
    byte* p;
    byte  c;

    ASSERT( pBuf );
    ASSERT( Count >= 0 );

    p = (byte*)pBuf;
    c = (byte)C;

    while( Count && (*p != c) )
    {
        p++;
        Count--;
    }

    return( Count ? (void*)p : NULL );
}

//==========================================================================

s32 x_memcmp( const void* pBuf1, const void* pBuf2, s32 Count )
{
    byte* p1;
    byte* p2;
    byte* pEnd;

    ASSERT( pBuf1 );
    ASSERT( pBuf2 );
    ASSERT( Count >= 0 );

    p1   = (byte*)pBuf1;
    p2   = (byte*)pBuf2;
    pEnd = (byte*)pBuf1 + Count;

    //
    // Compare aligned data 32 bits at a time.
    //
    if( ( Count >= 4 ) && !((u32)p1 & 3) && !((u32)p2 & 3) )
    {
        pEnd -= 4;
        while( p1 <= pEnd )
        {
            if( *((u32*)p1) != *((u32*)p2) )
            {
                p1 -= 4;
                p2 -= 4;
                break;
            }

            p1 += 4;
            p2 += 4;
        }
        pEnd += 4;
    }

    //
    // Compare remaining data one byte at a time.
    //
    while( p1 < pEnd )
    {
        if( *p1 != *p2 )
        {
            if ( *p1 < *p2 ) 
                return( -1 );
            else
                return(  1 );
        }

        p1++;
        p2++;
    }

    return( 0 );
}

//==========================================================================
//
// This is a ZModem's 32-bit CRC.  Note that the input and the output are
// actually the inverse of the typical ZModem CRC.  This is because most
// CRC initial values are 0 instead of ~0 which is what ZModem uses.  This
// way it is more compliant.
//
// The CRC generating polynomial is:                                                       
//   x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + 
//   x^ 8 + x^ 7 + x^ 5 + x^ 5 + x^ 4 + x^ 2 + x^ 1 + x^ 0
//
// The binary representation is: 0x1,04C1,1DB7
//
// ZModem computes the 32-bit CRC backwards, so it actually uses a binary 
// representation of 0xEDB8,8320 (the original x^32 bit is implied).  This 
// should miss one error out of 2^32.
//
// This code is derived from "Serial Protocols" by Tim Kientzle, pp 276-7.
//

u32 x_chksum( const void* pBuf, s32 Count, u32 CRC )
{
    u32 Table[256];
    u32 i, j, crc;

    //
    // Create table.
    //
    for( i = 0; i < 256; i++ )
    {
        crc = i;

        for( j = 0; j < 8; j++ )
        {
            crc = (crc>>1) ^ ((crc&1) ? (u32)0xEDB88320 : 0 );
        }

        Table[i] = crc & 0xFFFFFFFF;
    }

    //
    // Do the CRC.
    //
    {
        byte* p;
        byte* pEnd;

        // Invert it so we can be compliant with other types of CRCs.
        CRC  = ~CRC;

        // Set up variables.
        p    = (byte*)pBuf;
        pEnd = &p[Count];

        // Compute the new CRC.
        while( p < pEnd )
        {
            CRC = Table[ (CRC ^ *p++) & 0xFF ] ^ ((CRC>>8) & 0x00FFFFFF );
        }

        // Invert it back so next time we will be okay.
        CRC = ~CRC;
    }

    return( CRC );
}

//==========================================================================
//  Standard ASCII conversion functions.
//==========================================================================

//==========================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode.  (Not so for EBCDIC!)

s32 x_toupper( s32 C )
{
    if( (C >= 'a') && (C <= 'z') )
        C += ('A' - 'a');
    return( C );
}

//==========================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode.  (Not so for EBCDIC!)

s32 x_tolower( s32 C )
{
    if( (C >= 'A') && (C <= 'Z') )
        C += ('a' - 'A');
    return( C );
}

//==========================================================================

s32 x_atoi( const char* pStr )
{
    char C;         // Current character.
    char Sign;      // If '-', then negative, otherwise positive.
    s32  Total;     // Current total.

    ASSERT( pStr );

    // Skip whitespace.
    for( ; *pStr == ' '; ++pStr )
        ; // empty body

     // Save sign indication.
    C = *pStr++;
    Sign = C;

    // Skip sign.
    if( (C == '-') || (C == '+') )
    {
        C = *pStr++;
    }

    Total = 0;

    while( (C >= '0') && (C <= '9') )
    {
        // Accumulate digit.
        Total = (Total<<3) + (Total<<1) + (C - '0');
        //Total = (10 * Total) + (C - '0');

        // Get next char.
        C = *pStr++;
    }

    // Negate the total if negative.
    if( Sign == '-' ) 
        Total = -Total;

    return( Total );
}

//==========================================================================
// This implementation will handle scientific notation such as "1.23e+7".

f64 x_atof( const char* pStr )
{
    s32  FastIValue = 0;    // Integer portion value.
    s32  FastDValue = 0;    // Decimal portion value (numerator).
    s32  FastDenom  = 1;    // Decimal portion value (denominator).
    s32  FastISign  = 1;    // Integer portion sign.
    char ch;

    ASSERT( pStr );

    //
    // Skip whitespace.
    //
    for( ; *pStr == ' '; ++pStr )
        ; // empty body

    //
    // Get the sign.
    //
    if( *pStr == '-' )  { pStr++; FastISign = -1; }
    if( *pStr == '+' )    pStr++;

    //
    // Handle integer portion.
    // Accumulate integer digits.
    //      
    ch = *pStr;
    while( (ch >= '0') && ( ch <= '9' ) )
    {
        FastIValue = (FastIValue<<3) + (FastIValue<<1) + (ch - '0');
        pStr++;
        ch = *pStr;
    }

    //
    // Handle decimals.
    //
    if( ch == '.' )
    {
        // Skip the decimal point.
        pStr++;

        // Accumulate decimal digits.
        ch = *pStr;
        while( (ch >= '0') && ( ch <= '9' ) )
        {
            FastDValue = (FastDValue<<3) + (FastDValue<<1) + (ch - '0');
            FastDenom = (FastDenom<<3) + (FastDenom<<1);

			//---	our 32 bit integer cannot handle decimal numbers of greater than 9 places.
			if( FastDenom == 1000000000 )
			{
				//---	skip the rest of the decimal number
		        while( (ch >= '0') && ( ch <= '9' ) )
				{
					pStr++;
					ch = *pStr;
				}

				break;
			}

            pStr++;
            ch = *pStr;
        }
    }

    //
    // Check if we are finished or have an E??? on the end
    //
    if( (ch != 'e') && (ch != 'E') )
    {
        if( FastISign > 0 )
            return(  ((f32)FastIValue + ((f32)FastDValue / (f32)FastDenom)) );
        else
            return( -((f32)FastIValue + ((f32)FastDValue / (f32)FastDenom)) );
    }

    //
    // Handle SLOW DOUBLE scientific notation.
    //
    {
        f64  IValue;        // Integer portion value.
        f64  DValue;        // Decimal portion value (numerator).
        f64  Denom ;        // Decimal portion value (denominator).
        f64  ISign ;        // Integer portion sign.
        f64  EValue = 0;    // Exponent portion value,
        f64  ESign  = 1;    // Exponent portion sign.
        f64  e;             // Exponent order.

        // Copy float versions to double
        IValue = (f64)FastIValue;
        DValue = (f64)FastDValue;
        Denom  = (f64)FastDenom;
        ISign  = (f64)FastISign;


        // Skip the 'e'.
        pStr++;

        // Get the sign for the exponent.
        if( *pStr == '-' )  { pStr++; ESign = -1; }
        if( *pStr == '+' )    pStr++;

        // Accumulate exponent digits.
        while( (*pStr >= '0') && ( *pStr <= '9' ) )
        {
            EValue = (10.0 * EValue) + (*pStr - '0');
            pStr++;
        }

        //
        // Compute exponent. 
        // We make a shortcut for small exponents.
        //
        if( EValue < 30 )
        {
            e = 1;
            while( EValue )
            {
                e *= 10.0;
                EValue--;
            }
        }
        else
        {
            e = x_pow( 10.0, EValue );
        }

        //
        // Return final number for exponent case.
        //
        {
            f64 Temp = ISign * (IValue + (DValue / Denom));

            if( ESign < 0 )  return( Temp / e );
            else             return( Temp * e );
        }

        //
        // Return final number for non-exponent case.
        //
        return( ISign * (IValue + (DValue / Denom)) );
    }

}

//==========================================================================
//  Path manipulation functions.
//==========================================================================

//==========================================================================
// Split path implementation based on Microsoft's implementation.

void x_splitpath( const char* pPath, char* pDrive,
                                     char* pDir,
                                     char* pFName,
                                     char* pExt )
{
    char* p;
    char* pLastSlash = NULL;
    char* pLastDot   = NULL;
    s32   Len;

    //
    // Extract drive letter and ':', if any.
    //

    if( (x_strlen(pPath) >= X_MAX_DRIVE-2) &&
        (pPath[X_MAX_DRIVE-2] == ':') )
    {
        if( pDrive != NULL )
        {
            x_strncpy( pDrive, pPath, X_MAX_DRIVE-1 );
            pDrive[X_MAX_DRIVE-1] = '\0';
        }
        pPath += X_MAX_DRIVE-1;
    }
    else
    if( pDrive )
    {
        // No drive.
        pDrive[0] = '\0';
    }

    //
    // Extract path string, if any.  pPath now points to first character
    // of the path, if present, or the filename or even the pxtension.
    // Look ahead for the last path separator ('\' or '/').  If none is
    // found, there is no path.  Also note the location of the last '.',
    // if any, to assist in isolating the extension later.
    //

    for( p = (char*)pPath; *p; ++p )
    {
        if( (*p == '/') || (*p == '\\') )
            pLastSlash = p+1;
        else
        if( *p == '.' )
            pLastDot = p;
    }

    if( pLastSlash != NULL )
    {
        // There is a path.  Copy up to last slash or X_MAX_DIR chars.
        if( pDir != NULL )
        {
            Len = MIN( (pLastSlash - pPath), X_MAX_DIR-1 );
            x_strncpy( pDir, pPath, Len );
            pDir[Len] = '\0';
        }
        pPath = pLastSlash;
    }
    else
    if ( pDir != NULL )
    {
        // No path.
        pDir[0] = '\0';
    }

    //
    // Extract file name and extension, if any.  pPath now points to the 
    // first character of the file name, if any, or to the extension.  
    // pLastDot points to the beginning of the extension, if any.
    //

    if( (pLastDot != NULL) && (pLastDot >= pPath) )
    {
        // Found the marker for the extension.  Copy the file name up to
        // the '.' marker.

        if( pFName != NULL )
        {
            Len = MIN( (pLastDot - pPath), X_MAX_FNAME-1 );
            x_strncpy( pFName, pPath, Len );
            pFName[Len] = '\0';
        }

        // Now get the extension.  Note that p still points to the null 
        // which terminates the path.

        if( pExt != NULL )
        {
            Len = MIN( (p - pLastDot), X_MAX_EXT-1 );
            x_strncpy( pExt, pLastDot, Len );
            pExt[Len] = '\0';
        }
    }
    else
    {
        if( pFName != NULL )
        {
            Len = MIN( (p - pPath), X_MAX_FNAME );
            x_strncpy( pFName, pPath, Len );
            pFName[Len] = '\0';
        }
        if ( pExt != NULL )
        {
            pExt[0] = '\0';
        }
    }
}

//==========================================================================
// Make path implementation based on Microsoft's implementation.

void x_makepath( char* pPath, const char* pDrive,
                              const char* pDir,
                              const char* pFName,
                              const char* pExt )
{
    const char* p;

    // Copy the drive specification, if given.

    if( pDrive && *pDrive )
    {
        *pPath++ = *pDrive;
        *pPath++ = ':';
    }

    // Copy the directory specification, if given.

    if( pDir && *pDir )
    {
        p = pDir;
        do
        {
            *pPath++ = *p++;
        } while( *p );

        p--;
        if( (*p != '/') && (*p != '\\') )
        {
            *pPath++ = '\\';
        }
    }

    // Copy the file name, if given.

    if( pFName )
    {
        p = pFName;
        while( *p )
        {
            *pPath++ = *p++;
        }
    }

    // Copy the extension, if given.
    // Add the '.' marker if not given.

    if( pExt )
    {
        p = pExt;
        if( *p && (*p != '.') )
        {
            *pPath++ = '.';
        }
        while( *p )
        {
            *pPath++ = *p++;
        }
    }

    // Terminate the string.
    *pPath = '\0';
}

//==========================================================================
//  Mutex stuff: type, macros, and functions.
//==========================================================================

//==========================================================================
#ifdef TARGET_N64
//--------------------------------------------------------------------------

void InitMutexInfo( x_mutex& M )
{
    osCreateMesgQueue( &(M.MsgQ), &(M.Msg[0]), 1 );
    osSendMesg( &(M.MsgQ), (OSMesg)1, OS_MESG_BLOCK );
}

//--------------------------------------------------------------------------

void EnterMutexSection( x_mutex& M )
{
    osRecvMesg( &(M.MsgQ), &(M.Msg[0]), OS_MESG_BLOCK );
}

//--------------------------------------------------------------------------

void ExitMutexSection( x_mutex& M )
{
    osSendMesg( &(M.MsgQ), (OSMesg)1, OS_MESG_BLOCK );
}

//--------------------------------------------------------------------------
#endif
//==========================================================================

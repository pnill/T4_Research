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

#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "x_memory.hpp"
#endif

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
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

void x_srand_real( s32 seed )
{
    s_LastRand = seed;
}

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

char* x_strtok( char* pString, const char* pDelimit )
{
    static char* pSavedString = NULL;

    // --- New string to parse
    if (pString)
        pSavedString = pString;

    s32   DelimCount;
    char* pStart = pSavedString;
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

//=====================================================================================================
//  UNICODE/WIDE Versions of Standard NULL terminated string manipulation functions.
//=====================================================================================================

s32 x_wstrlen( const u16* pStr )
{
    const u16 *pEnd = pStr;

    ASSERT( pStr );

    while( *pEnd++ )
        ; // empty body

    return( (s32)(pEnd - pStr - 1) );
}

//==========================================================================

u16* x_wstrcpy( u16* pDest, const u16* pSrc )
{
    u16* p = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );

    while( (*p++ = *pSrc++) )
        ; // empty body

    return( pDest );
}

//==========================================================================

u16* x_wstrncpy( u16* pDest, const u16* pSrc, s32 Count )
{
    u16* pStart = pDest;

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

u16* x_wstrcat( u16* pFront, const u16* pBack )
{
    u16* p = pFront;

    ASSERT( pFront );
    ASSERT( pBack  );

    while( *p ) 
        p++;

    while( (*p++ = *pBack++) )
        ; // empty body

    return( pFront );
}

//==========================================================================

u16* x_wstrncat( u16* pFront, const u16* pBack, s32 Count )
{
    u16 *pStart = pFront;

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

s32 x_wstrcmp( const u16* pStr1, const u16* pStr2 )
{
    s32 Result = 0;

    ASSERT( pStr1 );
    ASSERT( pStr2 );

    while( !(Result = ((s32)*pStr1) - ((s32)*pStr2)) && *pStr1 )
        ++pStr1, ++pStr2;

    return( Result );
}                    

//==========================================================================

s32 x_wstrncmp( const u16* pStr1, const u16* pStr2, s32 Count )
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

s32 x_wstricmp( const u16* pStr1,  const u16* pStr2 )
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

u16* x_wstrstr( const u16* pMainStr, const u16* pSubStr )
{
    u16* pM = (u16*)pMainStr;
    u16* pS1; 
    u16* pS2;

    if( !*pSubStr )
        return( (u16*)pMainStr );

    while( *pM )
    {
        pS1 = pM;
        pS2 = (u16*)pSubStr;

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

u16* x_wstrrchr( const u16* pStr, s32 c )
{
    u16* pStart = (u16*)pStr;

    // Find end of string.
    while( *pStr++ )
        ; // empty body

    // Search from back towards front.
    while( (--pStr != pStart) && (*pStr != (u16)c) )
        ; // empty body

    // Did we find it?
    if( *pStr == (u16)c )
        return( (u16*)pStr );
    else
        return( NULL );
}

//==========================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode.  (Not so for EBCDIC!)

u16* x_wstrtoupper( u16* pStr )
{
    u16* p = pStr;

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

u16* x_wstrtolower( u16* pStr )
{
    u16* p = pStr;

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

u16* x_wstrtok( u16* pString, const u16* pDelimit )
{
    static u16* pSavedString = NULL;

    // --- New string to parse
    if (pString)
        pSavedString = pString;

    s32   DelimCount;
    u16* pStart = pSavedString;
    xbool DelimFound = FALSE;

    if (pSavedString)
    {
        // --- Loop until delimiter found
        while ( (!DelimFound) && (*pSavedString != NULL) )
        {
            for (DelimCount=0; DelimCount < x_wstrlen(pDelimit); DelimCount++)
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

s32 x_wsscanf( const u16 *in_string, const char *format, ... )
{
	x_va_list ArgList;
	u16*      ArgAddr;
	u16*      TextString;
	u16*      StringPtr;
    char*     TempCString;
    u16       TempWString[2];
	s32       NArgs=0;

	// --- No format arguments
	if (!format)
		return 0;

	// --- We will be modifying in_string, so make a copy
	TextString = (u16*)x_malloc( sizeof(u16) * (x_wstrlen(in_string)+1) );
	TempCString = (char*)x_malloc( sizeof(char) * (x_wstrlen(in_string)+1) );

	x_wstrcpy( TextString, in_string );
    x_cstrtowstr( " ", TempWString );
	StringPtr = x_wstrtok( TextString, TempWString );

	// --- Start the variable argument list
	x_va_start( ArgList, format );

	while (*format)
	{
		// --- Found a format specifier!
		if (*format == '%')
		{
			// --- Retrieve the address of the destination variable
			NArgs++;
			ArgAddr = x_va_arg( ArgList, u16* );

			// --- Parse the format type
			format++;
			switch (*format)
			{
				case 'c':			// Character
					*ArgAddr = *StringPtr;
					break;

				case 's':			// String
					x_wstrcpy( ArgAddr, StringPtr );
					break;

				case 'i':			// Signed Integer
				case 'd':			// Signed Integer
				case 'u':			// Unsigned Integer
                    x_wstrtocstr( StringPtr, TempCString );
					*((s32*)ArgAddr) = x_atoi( TempCString );
					break;

				case 'e':			// Scientific Notation
				case 'g':			// Scientific Notation
				case 'f':			// Single Precision Float
                    x_wstrtocstr( StringPtr, TempCString );
					*((f32*)ArgAddr) = (f32)x_atof( TempCString );
					break;
			}

			// --- Find the next value from the copy of in_string
			StringPtr = x_wstrtok( NULL, TempWString );
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

// this function will not convert correctly if the string contains characters above 255.
char*   x_wstrtocstr( u16* pSrc, char* pDest )
{
    char* p = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );

    while( (*p++ = (char)*pSrc++) )
        ; // empty body

    return( pDest );
}

u16*    x_cstrtowstr( char* pSrc, u16* pDest )
{
    u16* p = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );

    while( (*p++ = *pSrc++) )
        ; // empty body

    return( pDest );
}

//==========================================================================
//  Standard block memory functions.
//==========================================================================

void* x_memcpy( void* pDest, const void* pSrc, s32 Count )
{
    ASSERT( pDest );
    ASSERT( pSrc );
    ASSERT( Count >= 0 );

    ASSERTS( ((pDest < pSrc) && ((&((byte*)pDest)[Count]) <=   ((byte*)pSrc)        )) ||
             ((pDest > pSrc) && (  ((byte*)pDest)         >= (&((byte*)pSrc)[Count]))),
             "x_memcpy does not support overlapping blocks." );

    return( x_memmove( pDest, pSrc, Count ) );
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

    C = (u8)C;

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
    ASSERT( pBuf1 );
    ASSERT( pBuf2 );
    ASSERT( Count >= 0 );

	const byte * p1   = (const byte*)pBuf1          ; // Place in pBuf1 where we are comparing.
	const byte * p2   = (const byte*)pBuf2          ; // Place in pBuf2 where we are comparing.
	const byte * pEnd = (const byte*)pBuf1 + Count  ; // One byte beyond the last byte to compare in pBuf1.

    //
    // Compare aligned data 32 bits at a time.
    //
    if( ( Count >= 4 ) && !((u32)p1 & 3) && !((u32)p2 & 3) )
    {
        while( p1 <= pEnd-4 ) // While at least 4 bytes left to compare ...
        {
            if( *((u32*)p1) != *((u32*)p2) )
            {
                break;
            }

            p1 += 4;
            p2 += 4;
        }
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

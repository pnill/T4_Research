///////////////////////////////////////////////////////////////////////////
//
//  ACCCMP_C.C
//
//  ACCELERATION COMPRESSION ROUTINES
//
///////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "x_plus.hpp"
#include "x_debug.hpp"

#include "acccmp.h"



///////////////////////////////////////////////////////////////////////////
//  DEFINES
///////////////////////////////////////////////////////////////////////////
#define MAX_SAMPLES             1024
//#define COUNTERS_ON

///////////////////////////////////////////////////////////////////////////
//  GLOBALS
///////////////////////////////////////////////////////////////////////////

// Bitstream Manipulation Variables
static u8       wr_BitBuffer;                          
static s32      wr_BitsInBuffer;
static byte     *wr_BitPtr;

// Compression buffers
static s16      _InSample;
static s16      _ErrorValues[MAX_SAMPLES];
static s16      _MinBits[MAX_SAMPLES];
static s16      _NBits[MAX_SAMPLES];
static xbool    _StoreRaw;

// Counters
#ifdef COUNTERS_ON
static s32      _ErrorCount[65536];
static s32      _ErrorComboCount[256];
#endif

///////////////////////////////////////////////////////////////////////////

static void ACCCMP_WriteBitsInit (byte *Buffer)
{
    wr_BitBuffer    = 0 ;
    wr_BitsInBuffer = 0 ;
    wr_BitPtr       = Buffer ;
}

///////////////////////////////////////////////////////////////////////////

static void ACCCMP_WriteBits (u32 NBits, s32 Value)
{
    Value &= ~(-1<<NBits) ;
    wr_BitBuffer |= Value << wr_BitsInBuffer ;
    wr_BitsInBuffer += NBits ;
    while (wr_BitsInBuffer >= 8)
    {
        *wr_BitPtr++ = wr_BitBuffer ;
        wr_BitsInBuffer -= 8 ;
        wr_BitBuffer = (u8)(Value >> (NBits - wr_BitsInBuffer)) ;
    }
    ASSERT( wr_BitsInBuffer < 24 ) ;
}

///////////////////////////////////////////////////////////////////////////

static void ACCCMP_WriteBitsFlush ()
{
    if (wr_BitsInBuffer > 0)
    {
        *wr_BitPtr++ = wr_BitBuffer ;
    }
}

///////////////////////////////////////////////////////////////////////////

static void DecideErrorBits( s32 NSamples )
{
    s32 i,j,t;
    
    s32 ON[MAX_SAMPLES];
    s32 Left[MAX_SAMPLES];
    s32 Right[MAX_SAMPLES];
    s32 NBits[MAX_SAMPLES];
    s32 Waste[MAX_SAMPLES];
    s32 L,R,B;
        
    for (i=0; i<NSamples; i++)
    {
        ON[i]       = 1;
        Left[i]     = i;
        Right[i]    = i;
        NBits[i]    = _MinBits[i];
        Waste[i]    = 4+_MinBits[i];
    }
	//#### NOTE: Could it improve compression by changing the *3 to a *4, *5, *? - Kevin
    for (t=0; t<NSamples * 3; t++)
    {
        i = t%NSamples;

        if (ON[i])
        {
            // Find blocks on left and right, if none == -1
            L = i-1; 
            while ((L>=0) && (!ON[L])) L--;     
        
            R = i+1; 
            while ((R<NSamples) && (!ON[R])) R++;

            if (R==NSamples) R=-1;
            if ( (L!=-1) && (NBits[L]<NBits[i]) ) L=-1;
            if ( (R!=-1) && (NBits[R]<NBits[i]) ) R=-1;
            
            // Decide who is closer to current error bits
            if ( L==-1 && R==-1 ) B = -1;
            else
            if ( L==-1 && R!=-1 ) B = R;
            else
            if ( L!=-1 && R==-1 ) B = L;
            else
            if ( (NBits[L]-NBits[i]) < (NBits[R]-NBits[i]) ) B = L;
            else B = R;

            // If we found a candidate see if it's worth merging
            if (B!=-1)
            {
                s32 BlockSize   = (Right[i] - Left[i] + 1);
                s32 NBitsError  = BlockSize * (NBits[B] - NBits[i]);
            
                if ( NBitsError < (4 + NBits[i]) )
                {
                    // do merge
                    if (B < i)
                    {
                        Right[B] = Right[i];
                        Waste[B] += (NBitsError + (Waste[i] - (4+NBits[i])));
                    }
                    else
                    {
                        Left[B] = Left[i];
                        Waste[B] += (NBitsError + (Waste[i] - (4+NBits[i])));
                    }

                    ON[i] = 0;
                }
            }
        }
    }

    for (i=0; i<NSamples; i++) _NBits[i] = 0;
    
    for (i=0; i<NSamples; i++)
    if (ON[i])
    {
        for (j=Left[i]; j<=Right[i]; j++)
            _NBits[j] = (s16)NBits[i];
    }
}

///////////////////////////////////////////////////////////////////////////

static void ComputeErrorValues ( s16* SamplePtr, s32 NSamples )
{
    s32     i;                          // Loop Counter
    s32     Sample;                     // Decompressed sample
    s32     Delta;
    s32     EstDelta;
    s32     PrevDelta;
    s32     DeltaError;
    s32     RawSample;

    //---   Assert in input
    ASSERT(SamplePtr);
    ASSERT(NSamples>0 && NSamples<MAX_SAMPLES);

    //---   Set previous samples 
    _StoreRaw   = FALSE;
    _InSample   = SamplePtr[0];
    Sample      = _InSample;
    PrevDelta   = 0;

    //---   Clear values
    x_memset(_ErrorValues,0,sizeof(s16)*MAX_SAMPLES);
    x_memset(_MinBits,0,sizeof(s16)*MAX_SAMPLES);
    x_memset(_NBits,0,sizeof(s16)*MAX_SAMPLES);

    //---   Write the packet into the ACCCMP memory
    for (i = 0 ; i < NSamples ; i++)
    {
        RawSample  = SamplePtr[i];

        ///////////////////////////////////////////////////////////////////
        // Compute delta between previous sample and current sample
        Delta = (RawSample - Sample);

        ///////////////////////////////////////////////////////////////////
        // Estimate delta using previous delta
        EstDelta = PrevDelta;

        ///////////////////////////////////////////////////////////////////
        // Find error in estimation
        DeltaError = (Delta - EstDelta);

        ///////////////////////////////////////////////////////////////////
        // Don't peg error deltas...they will be handled later

        ///////////////////////////////////////////////////////////////////
        // Compute new delta and decompressed sample
        Delta       = PrevDelta + DeltaError;
        Sample      = Sample + Delta;
        PrevDelta   = Delta;

        ///////////////////////////////////////////////////////////////////
        //  Store the prediction error to be used at decompression
        _ErrorValues[i]  = (s16)( DeltaError );

        ///////////////////////////////////////////////////////////////////
        //  Increment counters for this error
        #ifdef COUNTERS_ON
        _ErrorCount[ ((u32)(DeltaError)) & 0xFFFF ]++;
        #endif
    }

    ///////////////////////////////////////////////////////////////////////
    //  Combination counters
    #ifdef COUNTERS_ON
    for (i=0; (i+3)<NSamples; i++)
    {
        s32 j;

        if ( ((_ErrorValues[i+0]>=-1) && (_ErrorValues[i+0]<=1)) &&
             ((_ErrorValues[i+1]>=-1) && (_ErrorValues[i+1]<=1)) &&
             ((_ErrorValues[i+2]>=-1) && (_ErrorValues[i+2]<=1)) &&
             ((_ErrorValues[i+3]>=-1) && (_ErrorValues[i+3]<=1)))
        {
            j = ((_ErrorValues[i+0]+1) *  3*3*3) +
                ((_ErrorValues[i+1]+1) *  3*3) +
                ((_ErrorValues[i+2]+1) *  3) +
                ((_ErrorValues[i+3]+1) *  1);
            _ErrorComboCount[j]++;
            i += 3;
        }
    }
    #endif

    //---   Decide on the minimum bits for each error sample
    for (i=0; i<NSamples; i++)
    {
        s32 E = ABS(_ErrorValues[i]);
        _MinBits[i] = 1;
        while (E>0) {E>>=1; _MinBits[i]++;}
        if (_MinBits[i]>15) _StoreRaw = TRUE;
    }

}    

///////////////////////////////////////////////////////////////////////////

s32 ACCCMP_Encode (byte *ACCCMPBuffer, s16 *RawBuffer, s32 NSignals, s32 NSamples)
{
    s32 i,s,NB,Size,TotalBits=0;

    ///////////////////////////////////////////////////////////////////////
    //  Assert on input
    ASSERT(ACCCMPBuffer);
    ASSERT(RawBuffer);
    ASSERT(NSignals>0);
    ASSERT(NSamples>0);

    ///////////////////////////////////////////////////////////////////////
    //  Inititalize bit packer
    ACCCMP_WriteBitsInit( ACCCMPBuffer ) ;

    ///////////////////////////////////////////////////////////////////////
    //  Loop through the signals
    for (s=0; s<NSignals; s++)
    {
        ///////////////////////////////////////////////////////////////////
        // Compute error values and decide on error bits
        ComputeErrorValues( &(RawBuffer[(s*NSamples)]), NSamples );
        DecideErrorBits( NSamples );

        ///////////////////////////////////////////////////////////////////
        // Pack initial sample into stream
        ACCCMP_WriteBits( 16, _InSample );
        TotalBits += 16;

        ///////////////////////////////////////////////////////////////////
        // Check if we need to pack it raw
        if (_StoreRaw)
        {
            // Write 0 bits...meaning raw
            ACCCMP_WriteBits( 4, 0 );
            TotalBits += 4;
            for (i=0; i<NSamples; i++)
            {
                ACCCMP_WriteBits(16,_ErrorValues[i]);
                TotalBits += 16;
            }
        }
        else
        {
            ///////////////////////////////////////////////////////////////
            // Pack error bits into stream
            i=0;
            while ( i < NSamples )
            {
                // Get bits for the new samples
                NB = _NBits[i];
                ASSERT(NB > 0);

                // Encode error bits
                {
                    u16 BitMask;
                    s16 Value;
                    u16 AbsValue;
                    u16 SignBit;

                    // Write num bits for samples
                    ACCCMP_WriteBits( 4, (NB & 0x000F) );
                    TotalBits += 4;

                    // Compute masks
                    BitMask = ~((0xFFFF) << (NB-1));
                    SignBit = (1 << (NB-1));

                    //---   Write all error bits of this size
                    while ((i<NSamples) && (_NBits[i]==NB))
                    {
                        //---   Write this sample
                        Value    = _ErrorValues[i];
                        AbsValue = ABS(Value) & BitMask;
                        if (Value < 0) AbsValue |= SignBit;
                        ACCCMP_WriteBits(NB,AbsValue);
                        TotalBits += NB;
                        i++;
                    }

                    //--- Add -0 terminator
                    ACCCMP_WriteBits( NB, SignBit );
                    TotalBits += NB;
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////
    //  Make sure it is a multiple of 8 bits
    if (TotalBits%8!=0)
        ACCCMP_WriteBits( 8-(TotalBits%8) , 0);

    ///////////////////////////////////////////////////////////////////////
    //  Flush bit packer
    ACCCMP_WriteBitsFlush();

    ///////////////////////////////////////////////////////////////////////
    //  Return size of compressed block
    Size = (s32)(wr_BitPtr - ACCCMPBuffer);
    while ((Size & 0x0F) != 0)
    {
        *wr_BitPtr = 0;
        wr_BitPtr++;
        Size = (s32)(wr_BitPtr - ACCCMPBuffer);
    }

    return (s32)(Size);
}

///////////////////////////////////////////////////////////////////////////

void ACCCMP_ClearCounters (void)
{
#ifdef COUNTERS_ON

    s32 i;
    for (i=0; i<65536; i++) _ErrorCount[i] =0;
    for (i=0; i<256; i++) _ErrorComboCount[i] =0;

#endif
}

///////////////////////////////////////////////////////////////////////////

void ACCCMP_DisplayCounters (void)
{
#ifdef COUNTERS_ON

    s32 i;
    s32 c,c0,c1,c2,c3;
    printf("==========================================\n");
    for (i=0; i<81; i++)
    {
        c  = i;
        c0 = (c % 3);   c /= 3;
        c1 = (c % 3);   c /= 3;
        c2 = (c % 3);   c /= 3;
        c3 = (c % 3);   c /= 3;
        printf("|%2d|%2d|%2d|%2d|   %6d\n",c0-1,c1-1,c2-1,c3-1,_ErrorComboCount[i]);
    }
    printf("==========================================\n");
    for (i=-256; i<256; i++) 
        printf("%8d %8d\n",i,_ErrorCount[((u32)(i)) & 0xFFFF]);
    printf("==========================================\n");

#endif
}

///////////////////////////////////////////////////////////////////////////

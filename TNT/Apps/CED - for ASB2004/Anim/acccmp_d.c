///////////////////////////////////////////////////////////////////////////
//
//  ACCCMP_D.C
//
//  ACCELERATION DECOMPRESSION ROUTINE
//
///////////////////////////////////////////////////////////////////////////

#include "acccmp.h"                  
#include "x_debug.h"

///////////////////////////////////////////////////////////////////////////
//  GLOBALS
///////////////////////////////////////////////////////////////////////////

xbool ACCCMP_Decode (byte *ACCCMPBuffer, s16 *RawBuffer, s32 NSignals, s16 NSamples)
{
    register u32     BitBuffer;                         
    register s32     BitsInBuffer;
    register byte*   BitPtr;
    register s32     i,s;
    register s32     NB;
    register s16     Sample;
    register s16     Delta;
    register s16     ErrorValue;
    register u16     BitMask;
    register u16     SignBit;
    register s16*    DestBuffer;
    
    /////////////////////////////////////////////////////////
    //  Assert on input
    /////////////////////////////////////////////////////////
    ASSERT(ACCCMPBuffer);
    ASSERT(RawBuffer);
    ASSERT(NSignals>0);
    ASSERT(NSamples>0);

    /////////////////////////////////////////////////////////
    //  Inititalize bit unpacker
    /////////////////////////////////////////////////////////
    {
        byte* Data = ACCCMPBuffer;
        BitBuffer = *Data++ ;
        BitBuffer |= *Data++ << 8 ;
        BitBuffer |= *Data++ << 16 ;
        BitBuffer |= *Data++ << 24 ;
        BitsInBuffer = 32;
        BitPtr = Data ;
    }

#ifndef MERGE_STREAMS_IN_DECOMP
	//---	assign the destination buffer to the beginning of the RawBuffer to be filled
	DestBuffer = RawBuffer;
#endif

    /////////////////////////////////////////////////////////
    //  Loop through the signals
    /////////////////////////////////////////////////////////
    for (s=0; s<NSignals; s++)
    {
        /////////////////////////////////////////////////////
        //  Set DestBuffer Ptr
        /////////////////////////////////////////////////////
#ifdef MERGE_STREAMS_IN_DECOMP
		//---	since we are merging the streams as we decompress them,
		//		Set the dest buffer back to the start of this stream
		//		data and increment the RawBuffer pointer to the beginning
		//		of the next stream
        DestBuffer = RawBuffer;
        RawBuffer++;
#endif

        /////////////////////////////////////////////////////
        //  Read initial sample
        /////////////////////////////////////////////////////
        {
            Sample = (s16)(BitBuffer & ~(-1<<16)) ;
            BitBuffer >>= 16 ;
            BitsInBuffer -= 16 ;
            while (BitsInBuffer <= 24)
            {
                BitBuffer |= (u32)(*BitPtr++) << BitsInBuffer ;
                BitsInBuffer += 8 ;
            }
            //if (BitsInBuffer < 16) return FALSE;
            //ASSERT( BitsInBuffer >= 16 ) ;
        }
        Delta    = 0;

        /////////////////////////////////////////////////////
        //  Decompress error bits
        /////////////////////////////////////////////////////
        i=0;
        while (i < NSamples)
        {
            /////////////////////////////////////////////////
            // Read 4 bit header
            /////////////////////////////////////////////////
            {
                NB = (s16)(BitBuffer & ~(-1<<4)) ;
                BitBuffer >>= 4 ;
                BitsInBuffer -= 4 ;
                while (BitsInBuffer <= 24)
                {
                    BitBuffer |= (u32)(*BitPtr++) << BitsInBuffer ;
                    BitsInBuffer += 8 ;
                }
                //if (BitsInBuffer < 16) return FALSE;
                //ASSERT( BitsInBuffer >= 16 ) ;
            }

            /////////////////////////////////////////////////
            // Check for raw packing
            /////////////////////////////////////////////////
            if (NB==0)
            {
                while (i < NSamples)
                {
                    ErrorValue = (s16)(BitBuffer & ~(-1<<16)) ;
                    BitBuffer >>= 16 ;
                    BitsInBuffer -= 16 ;
                    while (BitsInBuffer <= 24)
                    {
                        BitBuffer |= (u32)(*BitPtr++) << BitsInBuffer ;
                        BitsInBuffer += 8 ;
                    }

                    Delta       += ErrorValue;
                    Sample      += Delta;
                    *DestBuffer  =  Sample;
                    DestBuffer += NSignals;
                    i++;
                }
                break;
            }

            if (NB<=0) return FALSE;
            //ASSERT(NB>0);

            /////////////////////////////////////////////////
            // Build masks
            /////////////////////////////////////////////////
            BitMask  = ~((0xFFFF) << (NB-1));
            SignBit  = (1 << (NB-1));

            /////////////////////////////////////////////////
            // Read samples until terminator
            /////////////////////////////////////////////////
            while (1)
            {
                /////////////////////////////////////////////
                // Read error value from stream
                /////////////////////////////////////////////
                {
                    ErrorValue = (s16)(BitBuffer & ~(-1<<NB)) ;
                    BitBuffer >>= NB ;
                    BitsInBuffer -= NB ;
                    while (BitsInBuffer <= 24)
                    {
                        BitBuffer |= (u32)(*BitPtr++) << BitsInBuffer ;
                        BitsInBuffer += 8 ;
                    }
                    //if (BitsInBuffer < 16) return FALSE;
                    //ASSERT( BitsInBuffer >= 16 ) ;
                }

                /////////////////////////////////////////////
                // Save sample and increment i
                /////////////////////////////////////////////
                if (ErrorValue & SignBit) 
                {
                    if (ErrorValue==SignBit) break;
                    ErrorValue = -(ErrorValue & BitMask);
                }
                if (i>=NSamples) return FALSE;
                //ASSERT(i<NSamples);

                /////////////////////////////////////////////
                // Increment sample to new value
                /////////////////////////////////////////////
                Delta       += ErrorValue;
                Sample      += Delta;
                *DestBuffer  =  Sample;

                /////////////////////////////////////////////
                // Update counters
                /////////////////////////////////////////////
#ifdef MERGE_STREAMS_IN_DECOMP
				//---	since we are merging streams, increment to the next
				//		location for this stream data.
                DestBuffer += NSignals;
#else
				//---	we are not merging streams, so simply increment the DestBuffer to the next
				//		location.
				DestBuffer++;
#endif
                i++;
            }
        }
    }

    return TRUE;
}


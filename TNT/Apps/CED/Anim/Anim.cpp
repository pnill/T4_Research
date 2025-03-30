///////////////////////////////////////////////////////////////////////////
//
//  ANIM.C
//
///////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"

#include "anim.h"
#include "y_token.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                      BASIC MANIPULATIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void ANIM_InitInstance( anim* Anim )
{
    ASSERT(Anim);
    Anim->Flags     = 0;
    Anim->NFrames   = 0;
    Anim->NBones    = 0;
    Anim->NStreams  = 0;
    Anim->Stream    = NULL;
    Anim->NEvents   = 0;
    Anim->Event     = NULL;
}


///////////////////////////////////////////////////////////////////////////

void ANIM_CopyAnim( anim* AnimDst, anim* AnimSrc )
{
	ANIM_CopyAnimHeader( AnimDst, AnimSrc );
	ANIM_CopyAnimStreams( AnimDst, AnimSrc );
}

///////////////////////////////////////////////////////////////////////////

void ANIM_CopyAnimHeader( anim* AnimDst, anim* AnimSrc )
{
    ASSERT(AnimDst);
    ASSERT(AnimSrc);

    AnimDst->Flags     = AnimSrc->Flags;
    AnimDst->StartFrame= AnimSrc->StartFrame;
    AnimDst->NFrames   = AnimSrc->NFrames;
    AnimDst->NBones    = AnimSrc->NBones;
    AnimDst->NStreams  = AnimSrc->NStreams;
    AnimDst->NEvents   = AnimSrc->NEvents;

	for( s32 i=0; i<STREAM_TYPE_INDEX(MAX_STREAMS-1); i++ )
		AnimDst->StreamType[i] = AnimSrc->StreamType[i];
}

///////////////////////////////////////////////////////////////////////////

void ANIM_CopyAnimStreams( anim* AnimDst, anim* AnimSrc )
{
    ASSERT(AnimDst);
    ASSERT(AnimSrc);

    // Allocate animation space
    AnimDst->Stream = (s16*)x_malloc(sizeof(s16)*AnimDst->NFrames*AnimDst->NStreams);
    ASSERT(AnimDst->Stream);

    x_memcpy(AnimDst->Stream, AnimSrc->Stream,
            (sizeof(s16)*AnimDst->NFrames*AnimDst->NStreams));

    // Allocate animation events and duplicate
    if (AnimDst->NEvents > 0)
    {
        AnimDst->Event = (SAnimEvent*)x_malloc(sizeof(SAnimEvent)*AnimDst->NEvents);
        ASSERT(AnimDst->Event);
        x_memcpy(AnimDst->Event, AnimSrc->Event, sizeof(SAnimEvent)*AnimDst->NEvents);
    }
    else
    {
        AnimDst->Event = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////

void ANIM_KillInstance          (anim* Anim)
{
    ASSERT(Anim);
    if( Anim->Stream ) x_free(Anim->Stream);
    if( Anim->Event ) x_free(Anim->Event);
    ANIM_InitInstance(Anim);
}

///////////////////////////////////////////////////////////////////////////

void ANIM_AddEvent              (anim* Anim, s32 Frame, const char* EventName, vector3  P)
{
    s32 i,j;
    SAnimEvent* EV;

    ASSERT(Anim);
    ASSERT((Frame>0) && (Frame<Anim->NFrames));
    ASSERT( (P.X<2000) && (P.X>-2000) );
    ASSERT( (P.Y<2000) && (P.Y>-2000) );
    ASSERT( (P.Z<2000) && (P.Z>-2000) );

    // Alloc new set of events
    Anim->NEvents++;
    EV = (SAnimEvent*)x_malloc(sizeof(SAnimEvent)*Anim->NEvents);
    ASSERT(EV);

    // Copy old events into new list
    if (Anim->NEvents>1)
    {
        x_memcpy(EV,Anim->Event,sizeof(SAnimEvent)*(Anim->NEvents-1));
        x_free(Anim->Event);
        Anim->Event = EV;
    }

    // Add new event info
    EV = &EV[Anim->NEvents-1];
    EV->Frame   = (s16)Frame;
    x_strcpy(EV->Name,EventName);
    EV->PosX    = (s16)(P.X*16.0f);
    EV->PosY    = (s16)(P.Y*16.0f);
    EV->PosZ    = (s16)(P.Z*16.0f);

    // Simple sort new events from lowest frame to highest
    for (i=0; i<Anim->NEvents; i++)
    {
        for (j=i+1; j<Anim->NEvents; j++)
        {
            if (Anim->Event[j].Frame < Anim->Event[i].Frame)
            {
                SAnimEvent TempEV;
                TempEV = Anim->Event[i];
                Anim->Event[i] = Anim->Event[j];
                Anim->Event[j] = TempEV;
            }
        }
    }

    ANIM_SetDirtyBit(Anim);
}

///////////////////////////////////////////////////////////////////////////

void ANIM_GetEvent              (anim* Anim,  const char* EventName, SAnimEvent* EventDst)
{
    s32 i;

    ASSERT(Anim);
     
    // Clear event
    EventDst->Frame   = -1;
    x_strcpy(EventDst->Name,"NO NAME");
    EventDst->PosX    =  0;
    EventDst->PosY    =  0;
    EventDst->PosZ    =  0;

    // Search for matching event in anim
    for (i=0; i<Anim->NEvents; i++)
    if (x_strcmp(Anim->Event[i].Name,EventName)==0)
    {
        *EventDst = Anim->Event[i];
        return;
    }
}

///////////////////////////////////////////////////////////////////////////

void ANIM_DelEvent              (anim* Anim, const char* EventName)
{
    s32 i,j;
    SAnimEvent* EV;

    // Allocate new EV list
    if (Anim->NEvents > 1)
    {
        EV = (SAnimEvent*)x_malloc(sizeof(SAnimEvent)*(Anim->NEvents-1));
    }
    else
    {
        EV = NULL;
    }


    // Copy over necessary events
    j = 0;
    for (i=0; i<Anim->NEvents++; i++)
    {
        if (x_strcmp(Anim->Event[i].Name,EventName)==0)
        {
            EV[j] = Anim->Event[i];
            j++;
        }
        else
        {
            // Be sure only one event is deleted
            ASSERT(j == i);
        }
    }

    // Free previous events
    x_free(Anim->Event);
    Anim->Event   = EV;
    Anim->NEvents = Anim->NEvents-1;
    ANIM_SetDirtyBit(Anim);
}

///////////////////////////////////////////////////////////////////////////

void ANIM_SetStreamValue     (anim* Anim, s32 Frame, s32 Stream, s16 DataValue)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    ASSERT(Stream>=0 && Stream<Anim->NStreams);
    Anim->Stream[ (Stream*Anim->NFrames) + Frame ] = DataValue;
    ANIM_SetDirtyBit(Anim);
}

///////////////////////////////////////////////////////////////////////////

void ANIM_GetStreamValue        (anim* Anim, s32 Frame, s32 Stream, s16 *DataValue)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    ASSERT(Stream>=0 && Stream<Anim->NStreams);
	*DataValue = Anim->Stream[ (Stream*Anim->NFrames) + Frame ];
}

///////////////////////////////////////////////////////////////////////////

void ANIM_SetQuatValue     (anim* Anim, s32 Frame, s32 Stream, f32 DataValue)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    ASSERT(Stream>=0 && Stream<Anim->NStreams);
	ASSERT((DataValue>=-1.0f)&&(DataValue<=1.0f));
	s16 NewValue = (s16)(DataValue*(f32)((1L<<16)/2-1));
    Anim->Stream[ (Stream*Anim->NFrames) + Frame ] = NewValue;
    ANIM_SetDirtyBit(Anim);
}

///////////////////////////////////////////////////////////////////////////

f32 ANIM_GetQuatValue        (anim* Anim, s32 Frame, s32 Stream)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    ASSERT(Stream>=0 && Stream<Anim->NStreams);
	s32 StreamValue = (s32)Anim->Stream[ (Stream*Anim->NFrames) + Frame ];
    f32 ret = StreamValue / (f32)((1L<<16)/2-1);
	return ret;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_SetRotation          (anim* Anim, s32 Frame, s32 Stream, f32 RotValue)
{
	s32 Rot;
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    ASSERT(Stream>=0 && Stream<Anim->NStreams);
	ASSERT(Anim->StreamType[STREAM_TYPE_INDEX(Stream)]&STREAM_TYPE_MASK(Stream)); // assert that it is a rotation stream
	Rot = (s32)(RotValue*(f32)ANGLE_360/R_360);
	while( Rot >= ANGLE_360 )	Rot -= ANGLE_360;
	while( Rot < 0 )			Rot += ANGLE_360;
    Anim->Stream[ (Stream*Anim->NFrames) + Frame ] = (u16)Rot;
}

///////////////////////////////////////////////////////////////////////////

f32 ANIM_GetRotation          (anim* Anim, s32 Frame, s32 Stream)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    ASSERT(Stream>=0 && Stream<Anim->NStreams);
	ASSERT(Anim->StreamType[STREAM_TYPE_INDEX(Stream)]&STREAM_TYPE_MASK(Stream)); // assert that it is a rotation stream
    return (f32)Anim->Stream[ (Stream*Anim->NFrames) + Frame ]*R_360/(f32)ANGLE_360;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_SetRotValue           (anim* Anim, s32 Frame, s32 Stream, s32 RotValue)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    ASSERT(Stream>=0 && Stream<Anim->NBones*3);
    Anim->Stream[ (Stream*Anim->NFrames) + Frame ] = (s16)RotValue;
}

///////////////////////////////////////////////////////////////////////////

s32 ANIM_GetRotValue           (anim* Anim, s32 Frame, s32 Stream)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    ASSERT(Stream>=0 && Stream<Anim->NBones*3);
    return (s32) Anim->Stream[ (Stream*Anim->NFrames) + Frame ];
}

///////////////////////////////////////////////////////////////////////////

void ANIM_SetTransValue         (anim* Anim, s32 Frame, vector3 * Trans)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    Anim->Stream[ (Anim->NFrames*Anim->NBones*3) + (Anim->NFrames*0) + Frame ] = (s16)(Trans->X * 16.0f);
    Anim->Stream[ (Anim->NFrames*Anim->NBones*3) + (Anim->NFrames*1) + Frame ] = (s16)(Trans->Y * 16.0f);
    Anim->Stream[ (Anim->NFrames*Anim->NBones*3) + (Anim->NFrames*2) + Frame ] = (s16)(Trans->Z * 16.0f);
}

///////////////////////////////////////////////////////////////////////////

void ANIM_GetTransValue         (anim* Anim, s32 Frame, vector3 * Trans)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
    Trans->X = Anim->Stream[ (Anim->NFrames*Anim->NBones*3) + (Anim->NFrames*0) + Frame ] * (1/16.0f);
    Trans->Y = Anim->Stream[ (Anim->NFrames*Anim->NBones*3) + (Anim->NFrames*1) + Frame ] * (1/16.0f);
    Trans->Z = Anim->Stream[ (Anim->NFrames*Anim->NBones*3) + (Anim->NFrames*2) + Frame ] * (1/16.0f);
}

///////////////////////////////////////////////////////////////////////////

void ANIM_SetMiscValue          (anim* Anim, s32 Frame, s32 Stream, s32 MiscDataValue)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
//    Stream += (Anim->NBones+1)*3;
    ASSERT(Stream>=0 && Stream<Anim->NStreams);
    Anim->Stream[ (Stream*Anim->NFrames) + Frame ] = (s16)MiscDataValue;
}

///////////////////////////////////////////////////////////////////////////

s32 ANIM_GetMiscValue          (anim* Anim, s32 Frame, s32 Stream)
{
    ASSERT(Anim);
    ASSERT(Frame>=0 && Frame<Anim->NFrames);
//    Stream += (Anim->NBones+1)*3;
    ASSERT(Stream>=0 && Stream<Anim->NStreams);
    return (s32) Anim->Stream[ (Stream*Anim->NFrames) + Frame ];
}

///////////////////////////////////////////////////////////////////////////

void ANIM_DuplicateLastFrame    (anim* Anim)
{
    s32 i,k;
    s16* StreamData;

    ASSERT(Anim);

    // Allocate a new block for the streams
    StreamData = (s16*)x_malloc(sizeof(s16)*(Anim->NFrames+1)*Anim->NStreams);
    x_memset(StreamData,0,sizeof(s16)*(Anim->NFrames+1)*Anim->NStreams);

    // Copy old streams into new data
    for (i=0; i<Anim->NStreams; i++)
    {
        // Copy normal frames into new data
        for (k=0; k<Anim->NFrames; k++)
        {
            StreamData[ (i*(Anim->NFrames+1)) + k ] =
                Anim->Stream[ (i*Anim->NFrames) + k ];
        }

        // Duplicate last frame
        StreamData[ (i*(Anim->NFrames+1)) + (Anim->NFrames) ] =
            Anim->Stream[ (i*Anim->NFrames) + (Anim->NFrames-1) ];
    }

    // Swap streams
    x_free(Anim->Stream);
    Anim->Stream = StreamData;
    Anim->NFrames++;
}

///////////////////////////////////////////////////////////////////////////

void ShiftStreamType( anim* Anim, s32 Index, s32 Shift )
{
	u8	TempBuffer[MAX_STREAMS];
	s32	i, Len;
	u8*	pTo;
	u8* pFrom;

	if( Shift == 0 )
		return;

	//---	pull the boolean values out of the bit set to work with them easier.
	for( i=0; i<Anim->NStreams; i++ )
		TempBuffer[i] = (Anim->StreamType[STREAM_TYPE_INDEX(i)]&STREAM_TYPE_MASK(i)) ? TRUE : FALSE;

	//---	shift the stream types
	if( Shift > 0 )
	{
		pTo		= &TempBuffer[Index+Shift];
		pFrom	= &TempBuffer[Index];
	}
	else
	{
		Shift	= -Shift;
		pTo		= &TempBuffer[Index];
		pFrom	= &TempBuffer[Index-Shift];
	}
	Len = (MAX_STREAMS-Index-Shift)*sizeof(u8);
	x_memmove( pTo, pFrom, Len );

	//---	store the boolean values back into the bit set
	for( i=0; i<Anim->NStreams+Shift; i++ )
	{
		if( TempBuffer[i] )
			Anim->StreamType[STREAM_TYPE_INDEX(i)] |= STREAM_TYPE_MASK(i);
		else
			Anim->StreamType[STREAM_TYPE_INDEX(i)] &= ~STREAM_TYPE_MASK(i);
	}

}

///////////////////////////////////////////////////////////////////////////
/*
s32 ANIM_AddStream( anim* Anim, xbool bRotationStream )
{
	s32 Index = Anim->NStreams - (Anim->NBones+1)*3;

	ANIM_AddMiscStream( Anim, Index, bRotationStream );

	return Index;
}
*/
///////////////////////////////////////////////////////////////////////////

s32 ANIM_AddStream( anim* Anim, xbool bRotationStream )
{
    s32 i,j,k;
    s16* StreamData;
	s32		NewStreamIndex = Anim->NStreams; //(Anim->NBones+1)*3;
    
    ASSERT(Anim);
    ASSERT(NewStreamIndex<(Anim->NStreams+1));

    // Alloc a new block for the streams
    StreamData = (s16*)x_malloc(sizeof(s16)*Anim->NFrames*(Anim->NStreams+1));
    x_memset(StreamData,0,sizeof(s16)*Anim->NFrames*(Anim->NStreams+1));

    // Copy old stream into new data
    for (i=0; i<Anim->NStreams; i++)
    {
        if (i>=NewStreamIndex) j = i+1;
        else                   j = i;

        for (k=0; k<Anim->NFrames; k++)
        {
            StreamData[ (j*Anim->NFrames) + k ] = Anim->Stream[ (i*Anim->NFrames) + k ];
        }
    }

	//---	shift the stream types
	ShiftStreamType( Anim, NewStreamIndex, 1 );

    // Swap Streams, and x_free old one + Reset number of streams in anim
	x_free( Anim->Stream ) ;
	Anim->Stream = StreamData ;
	if( bRotationStream )
		Anim->StreamType[STREAM_TYPE_INDEX(NewStreamIndex)] |= STREAM_TYPE_MASK(NewStreamIndex);
	else
		Anim->StreamType[STREAM_TYPE_INDEX(NewStreamIndex)] &= ~STREAM_TYPE_MASK(NewStreamIndex);
    Anim->NStreams++;

	return NewStreamIndex;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_DelStream( anim* Anim, s32 StreamIndex )
{
    s32 i,j,k;
    s16* StreamData;

    ASSERT(Anim);
    ASSERT(StreamIndex>=((Anim->NBones+1)*3) && (StreamIndex<Anim->NStreams));

    // Alloc a new block for the streams
    StreamData = (s16*)x_malloc(sizeof(s16)*Anim->NFrames*(Anim->NStreams-1));
    x_memset(StreamData,0,sizeof(s16)*Anim->NFrames*(Anim->NStreams-1));

    // Copy old stream into new data
    for (i=0; i<Anim->NStreams-1; i++)
    {
        if (i>=StreamIndex) j = i+1;
        else                j = i;

        for (k=0; k<Anim->NFrames; k++)
        {
            StreamData[ (i*Anim->NFrames) + k ] = Anim->Stream[ (j*Anim->NFrames) + k ];
        }
    }

	//---	shift the stream types
	ShiftStreamType( Anim, StreamIndex, -1);

    // Swap Streams, and x_free old one + Reset number of streams in anim
	x_free( Anim->Stream ) ;
	Anim->Stream = StreamData ;
    Anim->NStreams--;
}

///////////////////////////////////////////////////////////////////////////
/*
s32 ANIM_AddStreams( anim* Anim, s32 NumMiscStreams, xbool bRotationStream )
{
	s32 Index = Anim->NStreams - (Anim->NBones+1)*3;

	ANIM_AddMiscStreams( Anim, Index, NumMiscStreams, bRotationStream );

	return Index;
}
*/
s32 ANIM_AddStreams( anim* Anim, s32 NumStreams, xbool bRotationStreams )
{
	s32		NewStreamIndex = Anim->NStreams;

	ANIM_AddStreams( Anim, NewStreamIndex, NumStreams, bRotationStreams );

	return NewStreamIndex;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_AddStreams( anim* Anim, s32 NewStreamIndex, s32 NumMiscStreams, xbool bRotationStreams )
{
    s32		i;
	s32		NewStreamsSize;
    s16*	StreamData;
	s16*	pNewStreams;
	s32		StreamLength = sizeof(s16)*Anim->NFrames;
    
    ASSERT(Anim);
    ASSERT(NewStreamIndex<(Anim->NStreams+1));

    // Alloc a new block for the streams
	StreamData = Anim->Stream;
    StreamData = (s16*)x_realloc( StreamData, StreamLength*(Anim->NStreams+NumMiscStreams)) ;

	pNewStreams = (s16*)(((u32)StreamData) + (NewStreamIndex*StreamLength));
	NewStreamsSize = NumMiscStreams*StreamLength;

	//---	if the new stream(s) are added inside the other streams, shift them down to make room
	if( NewStreamIndex < (Anim->NStreams-1) )
	{
		s32 SizeToMove = (Anim->NStreams - NewStreamIndex)*StreamLength;
		x_memmove( (s8*)(((u32)pNewStreams)+NewStreamsSize), pNewStreams, SizeToMove );
	}
	
	//---	shift the stream types
	ShiftStreamType( Anim, NewStreamIndex, NumMiscStreams );

    // Swap Streams, and x_free old one + Reset number of streams in anim
	for( i=0; i<NumMiscStreams; i++ )
		if( bRotationStreams )
			Anim->StreamType[STREAM_TYPE_INDEX(NewStreamIndex+i)] |= STREAM_TYPE_MASK(NewStreamIndex+i);
		else
			Anim->StreamType[STREAM_TYPE_INDEX(NewStreamIndex+i)] &= ~STREAM_TYPE_MASK(NewStreamIndex+i);

	//---	clear the new streams
	x_memset( pNewStreams, 0, NewStreamsSize );

    // Swap Streams, and x_free old one + Reset number of streams in anim
	Anim->Stream = StreamData ;
    Anim->NStreams+=NumMiscStreams;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_DelStreams( anim* Anim, s32 StreamIndex, s32 NumMiscStreams )
{
    s16*	StreamData;
	s32		StreamLength = sizeof(s16)*Anim->NFrames;
 
    ASSERT(Anim);
    ASSERT(StreamIndex>=((Anim->NBones+1)*3) && (StreamIndex<Anim->NStreams));

	StreamData = Anim->Stream;

	//---	if the deleted streams are at the end, simply reallocate the buffer and be done
	if( (Anim->NStreams - StreamIndex) <= NumMiscStreams )
	{
	    StreamData = (s16*)x_realloc( &StreamData, StreamLength*(Anim->NStreams-NumMiscStreams)) ;
	}
	else
	{
		s32 SizeStreams2 = (Anim->NStreams - (StreamIndex+NumMiscStreams))*StreamLength;
		s8* pStreams2 = (s8*)(((u32)StreamData) + (StreamIndex*StreamLength));
		s8* pOldStreams2 = (s8*)(((u32)Anim->Stream) + ((StreamIndex+NumMiscStreams)*StreamLength));

		// Alloc a new block for the streams
		StreamData = (s16*)x_malloc( StreamLength*(Anim->NStreams - NumMiscStreams) );

		//---	copy the first streams over
		x_memcpy( StreamData, Anim->Stream, StreamIndex*StreamLength );
		x_memcpy( pStreams2, pOldStreams2, SizeStreams2 );

		x_free( Anim->Stream ) ;
	}

	//---	shift the stream types
	ShiftStreamType( Anim, StreamIndex, -NumMiscStreams );

    // Swap Streams, and x_free old one + Reset number of streams in anim
	Anim->Stream = StreamData ;
    Anim->NStreams-= NumMiscStreams;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_ChangeNumFrames(anim* Anim, s32 NewNumFrames )
{
    s32 i,k;
    s16* StreamData;
    
    ASSERT(Anim);

    // Alloc a new block for the streams
    StreamData = (s16*)x_malloc(sizeof(s16)*NewNumFrames*Anim->NStreams );
	x_memset( StreamData, 0, sizeof(s16)*NewNumFrames*Anim->NStreams );

    // Copy old stream into new data
    for( i=0; i<Anim->NStreams; i++ )
    {
        for( k=0; k<Anim->NFrames; k++ )
            StreamData[ (i*NewNumFrames) + k ] = Anim->Stream[ (i*Anim->NFrames) + k ];
    }

	x_free( Anim->Stream );
	Anim->Stream = StreamData;
	Anim->NFrames = NewNumFrames;
}

///////////////////////////////////////////////////////////////////////////
//
//	anim*	Anim			- the animation
//	s32		AngleStream		- stream of the animation
//	s32		Angle360		- values used for 360 degrees in the stream.
//							  (may not be ANGLE_360 if it is being processed for export)
//
void ANIM_MinimizeAngleDiff( anim* Anim, s32 AngleStream, s32 Angle360 )
{
    s32  i;
    s16* StreamData;
    s32  R0,R1,DR;
	s32 Angle180 = Angle360/2;

    // Get Ptr to stream
    ASSERT(Anim);
    StreamData = &Anim->Stream[ AngleStream * Anim->NFrames ];

    // Put angle 0 into 0-ANGLE_360 range
    R0 = StreamData[0];
    while (R0 <			 0)	R0 += Angle360;
    while (R0 >= Angle360)	R0 -= Angle360;
    StreamData[0] = (s16)R0;

    // Minimize ABS diff between the two angles
    for (i=0; i<Anim->NFrames-1; i++)
    {
        R0 = StreamData[i+0];
        R1 = StreamData[i+1];

        DR = R1 - R0;
        while (DR <= -Angle180)	DR += Angle360;
        while (DR >  Angle180)	DR -= Angle360;
        R1 = R0 + DR;

        StreamData[i+1] = (s16)R1;
    }
}

///////////////////////////////////////////////////////////////////////////

xbool ANIM_IsDirty               (anim* Anim)
{
    ASSERT(Anim);
    return ((Anim->Flags & ANIM_FLAG_DIRTY) ? (TRUE):(FALSE));
}

///////////////////////////////////////////////////////////////////////////

void ANIM_ClearDirtyBit         (anim* Anim)
{
    ASSERT(Anim);
    Anim->Flags &= ~ANIM_FLAG_DIRTY;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_SetDirtyBit           (anim* Anim)
{
    ASSERT(Anim);
    Anim->Flags |= ANIM_FLAG_DIRTY;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_SetFlag               (anim* Anim, u32 FlagType, xbool FlagOn)
{
    ASSERT(Anim);
    if (FlagOn)
    {
        Anim->Flags |= FlagType;        // Turn flags on
    }
    else
    {
        Anim->Flags &= ~FlagType;       // Turn flags off
    }
}

///////////////////////////////////////////////////////////////////////////

xbool ANIM_GetFlag               (anim* Anim, u32 FlagType)
{
    ASSERT(Anim);
    if (Anim->Flags & FlagType) return TRUE;
    return FALSE;
}


///////////////////////////////////////////////////////////////////////////
void ANIM_ConvertToQuat(anim* Anim)
{
	if( Anim->Flags & ANIM_FLAG_QUATERNION )
		return;

	//---	calculate the 
	s32			NFrames					= Anim->NFrames;
	s32			NBones					= Anim->NBones;
	radian3		Rot;
	quaternion	Quat;
	s32			i, j;

	//---	add enough streams to hold the new
	ANIM_AddStreams( Anim, 3*NBones, NBones, FALSE );

	for (i=0; i<NFrames; ++i)
	{
		//---	Create Quaternions for each euler rotation set.  Work backward from the end of the frame so that
		//		data isn't overwritten before it is referenced.
		for (j=NBones-1; j>=0; --j)
		{
			Rot.Pitch	= ANIM_GetRotation( Anim, i, 3*j + 0 );
			Rot.Yaw		= ANIM_GetRotation( Anim, i, 3*j + 1 );
			Rot.Roll	= ANIM_GetRotation( Anim, i, 3*j + 2 );

			//---	convert into a quaternion. make sure the quaternion is normalized
			//		so that each value is between -1.0f..1.0f.
			Quat.SetupRotateXYZ( Rot );
			Quat.Normalize();

			//---	Place quaternion data over the old rotation data.  Since quats are from -1 to 1, they conversion
			//		is a bit different from the angle conversion.
			ANIM_SetQuatValue( Anim, i, 4*j + 0, Quat.X );
			ANIM_SetQuatValue( Anim, i, 4*j + 1, Quat.Y );
			ANIM_SetQuatValue( Anim, i, 4*j + 2, Quat.Z );
			ANIM_SetQuatValue( Anim, i, 4*j + 3, Quat.W );
		}
	}

	//---	the rest of the streams are no longer rotations
	for( i=0; i<3*NBones; i++ )
		ANIM_SetStreamType( Anim, i, FALSE );

	Anim->Flags |= ANIM_FLAG_QUATERNION;
}


///////////////////////////////////////////////////////////////////////////
void ANIM_ConvertToEuler(anim* Anim)
{
	if( Anim->Flags & ANIM_FLAG_QUATERNION )
		return;

	//---	calculate the 
	s32			NFrames					= Anim->NFrames;
	s32			NBones					= Anim->NBones;
	radian3		Rot;
	quaternion	Quat;
	matrix4		Mat;
	s32			i, j;

	//---	add enough streams to hold the new
	ANIM_AddStreams( Anim, 3*NBones, NBones, FALSE );

	//---	the rest of the streams are no longer rotations
	for( i=0; i<3*NBones; i++ )
		ANIM_SetStreamType( Anim, i, TRUE );

	for (i=0; i<NFrames; ++i)
	{
		//---	Create Quaternions for each euler rotation set.  Work backward from the end of the frame so that
		//		data isn't overwritten before it is referenced.
		for (j=0; j>=NBones; j++)
		{
			Quat.X = ANIM_GetQuatValue( Anim, i, 4*j + 0 );
			Quat.Y = ANIM_GetQuatValue( Anim, i, 4*j + 1 );
			Quat.Z = ANIM_GetQuatValue( Anim, i, 4*j + 2 );
			Quat.W = ANIM_GetQuatValue( Anim, i, 4*j + 3 );

			//---	convert into a quaternion. make sure the quaternion is normalized
			//		so that each value is between -1.0f..1.0f.
			Quat.BuildMatrix( Mat );
			Rot = Mat.GetRotationXYZ();

			//---	Place quaternion data over the old rotation data.  Since quats are from -1 to 1, they conversion
			//		is a bit different from the angle conversion.
			ANIM_SetRotation( Anim, i, 3*j + 0, Rot.Pitch );
			ANIM_SetRotation( Anim, i, 3*j + 1, Rot.Yaw );
			ANIM_SetRotation( Anim, i, 3*j + 2, Rot.Roll );
		}
	}

	//---	add enough streams to hold the new
	ANIM_DelStreams( Anim, 3*NBones, NBones );

	Anim->Flags &= ~ANIM_FLAG_QUATERNION;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  IMPORT AMC
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

err ANIM_ImportAMC (anim* Anim, const char* FileName, CSkel* Skel, s16 Version )
{
    s16         BoneMatchIndex[100];
    tokenizer   TOK;
    s32         Tx,Ty,Tz;
    s32         Rx,Ry,Rz;                                                                           
    s32         BIndex, AMCBIndex;
    s32         i,j;
    xbool       SkipReadingBoneName;
    s16*        FrameData;
    s32         NFramesAllocated;
    s32         FrameNumber;
	s32			LineNumber;

	#define	CONVERSION_ANGLE		(1<<16)

    ASSERT(Anim);
    ASSERT(FileName);
    ASSERT(Skel);

    /////////////////////////////////////////////////////////////
    // Init anim to valid state, open AMC file,
    // and clear bonematching array
    /////////////////////////////////////////////////////////////
    ANIM_KillInstance(Anim);
    Anim->Version = Version;
    x_strcpy(Anim->Name,"AMC Animation");
    TKN_Clear(&TOK);
    if (TKN_ReadFile(&TOK,FileName) != ERR_SUCCESS)
    {
        return ERR_FAILURE;
    }
    for (i=0; i<100; i++)
        BoneMatchIndex[i] = -1;

    /////////////////////////////////////////////////////////////
    // Reset dynamic sized frame buffer
    /////////////////////////////////////////////////////////////
    Anim->NBones     = Skel->m_NBones;
    Anim->NStreams   = ((Anim->NBones*3)+3);
    Anim->StartFrame = 9999999;
    Anim->NFrames    = 0;
    Anim->Stream     = NULL;
	for( i=0; i<Anim->NStreams; i++ )
	{
		//---	if it is a bone stream, set the type to TRUE (rotation), otherwise set it to FALSE (translation)
		if( i < (Anim->NBones*3) )
			Anim->StreamType[STREAM_TYPE_INDEX(i)] |= STREAM_TYPE_MASK(i);
		else
			Anim->StreamType[STREAM_TYPE_INDEX(i)] &= ~STREAM_TYPE_MASK(i);
	}


    NFramesAllocated = 200;
    FrameData        = (s16*)x_malloc(sizeof(s16)*Anim->NStreams*NFramesAllocated);
    ASSERT(FrameData);
    x_memset(FrameData,0,(sizeof(s16)*Anim->NStreams*NFramesAllocated));

    /////////////////////////////////////////////////////////////
    // Skip any comments and determine first frame number
    /////////////////////////////////////////////////////////////
    TKN_Rewind(&TOK);
    while (TOK.TokenType != TOKEN_NUMBER)
	{
		//---	skip to the end of this line
		LineNumber = TOK.LineNumber;
		do
		{
			TKN_ReadToken(&TOK);
		}
		while(( TOK.LineNumber == LineNumber )&&( TOK.FilePos < TOK.FileSize ));

		//---	had trouble loading this file.
		if( TOK.LineNumber == LineNumber )
		{
			x_free( FrameData );
	        return ERR_FAILURE;
		}

	}
    Anim->StartFrame = TOK.Int-1;

    /////////////////////////////////////////////////////////////
    // Loop until we hit the end of the file
    /////////////////////////////////////////////////////////////
    SkipReadingBoneName = TRUE;
    while (1)
    {
        ASSERT(TOK.TokenType == TOKEN_NUMBER);
        FrameNumber = TOK.Int;
        FrameNumber--;
        FrameNumber-= Anim->StartFrame;
        if ((FrameNumber+1) > Anim->NFrames) Anim->NFrames = FrameNumber+1;

        // Handle dynamically sized frame data
        if (FrameNumber >= NFramesAllocated)
        {
            s32  NewNFramesAllocated;
            s16* NewFrameData;

            // Allocate new data block
            NewNFramesAllocated = NFramesAllocated;
            while (NewNFramesAllocated <= FrameNumber) NewNFramesAllocated += 100;
            NewFrameData = (s16*)x_malloc(sizeof(s16)*Anim->NStreams*NewNFramesAllocated);
            ASSERT(NewFrameData);
    
            // Copy old data into new block and clear unused section
            for (i=0; i<Anim->NStreams; i++)
            {
                x_memcpy( &NewFrameData[ i * NewNFramesAllocated ],
                          &FrameData   [ i * NFramesAllocated],
                          sizeof(s16) * NFramesAllocated);
                x_memset( &NewFrameData[ (i*NewNFramesAllocated) + NFramesAllocated ],
                          0,
                          sizeof(s16) * (NewNFramesAllocated - NFramesAllocated));
            }

            // Setup new values
            NFramesAllocated = NewNFramesAllocated;
            x_free(FrameData);
            FrameData = NewFrameData;
        }

        // Read this frame of data
        AMCBIndex = 0;
        TKN_ReadToken(&TOK);
        while (TOK.TokenType == TOKEN_SYMBOL)
        {
            /////////////////////////////////////////////////////
            // Determine bone index to use
            // -1 = never seen before
            // -2 = no match in asf
            // 0+ = index of bone in asf
            /////////////////////////////////////////////////////
            if (BoneMatchIndex[AMCBIndex] == -1)
            {
                // Search for bone in skeleton
                for (j=0; j<Skel->m_NBones; j++)
                if (x_strcmp(Skel->m_pBone[j].BoneName,TOK.String)==0)
                {
                    BoneMatchIndex[AMCBIndex] = (s16)j;
                    break;
                }

                if (j==Skel->m_NBones)
                    BoneMatchIndex[AMCBIndex] = -2;
            }
            BIndex = BoneMatchIndex[AMCBIndex];

            /////////////////////////////////////////////////////
            // Load values
            /////////////////////////////////////////////////////
            if (BIndex>=0)
            {
                matrix4 M;
                SSkelBone* B = &Skel->m_pBone[BIndex];
                radian RotX=0,RotY=0,RotZ=0;
                
                // Clear values
                Tx = Ty = Tz = Rx = Ry = Rz = 0;
				M.Identity();

                for (j=0; j<6; j++)
                {
                    u32 DOF = Skel->m_pBone[BIndex].DOFOrder[j];

                    // Load individual anim info
                    if ((DOF == BONE_DOF_TX) && (B->Flags & BONE_DOF_TX))
                    {
                        TKN_ReadFloatToken(&TOK);
                        Tx = (s32)(TOK.Float*16.0f*Skel->m_TransScale);
                    }
                    else
                    if ((DOF == BONE_DOF_TY) && (B->Flags & BONE_DOF_TY))
                    {
                        TKN_ReadFloatToken(&TOK);
                        Ty = (s32)(TOK.Float*16.0f*Skel->m_TransScale);
                    }
                    else
                    if ((DOF == BONE_DOF_TZ) && (B->Flags & BONE_DOF_TZ))
                    {
                        TKN_ReadFloatToken(&TOK);
                        Tz = (s32)(TOK.Float*16.0f*Skel->m_TransScale);
                    }
                    else
                    if ((DOF == BONE_DOF_RX) && (B->Flags & BONE_DOF_RX))
                    {
                        TKN_ReadFloatToken(&TOK);
						M.RotateX( (f32)(TOK.Float * R_1) );
                    }
                    else
                    if ((DOF == BONE_DOF_RY) && (B->Flags & BONE_DOF_RY))
                    {
                        TKN_ReadFloatToken(&TOK);
						M.RotateY( (f32)(TOK.Float * R_1) );
                    }
                    else
                    if ((DOF == BONE_DOF_RZ) && (B->Flags & BONE_DOF_RZ))
                    {
                        TKN_ReadFloatToken(&TOK);
						M.RotateZ( (f32)(TOK.Float * R_1) );
                    }
                }

                // Pull rotations from matrix
                {
                    radian3 R;
					R = M.GetRotationXYZ();
//                  M4_GetRotationsXYZ(&M,&R);
                    Rx = (s32)((R.X * (f32)CONVERSION_ANGLE) / R_360);
                    while (Rx >= CONVERSION_ANGLE) Rx -= CONVERSION_ANGLE;
                    while (Rx <                 0) Rx += CONVERSION_ANGLE;
                    Ry = (s32)((R.Y * (f32)CONVERSION_ANGLE) / R_360);
                    while (Ry >= CONVERSION_ANGLE) Ry -= CONVERSION_ANGLE;
                    while (Ry <                 0) Ry += CONVERSION_ANGLE;
                    Rz = (s32)((R.Z * (f32)CONVERSION_ANGLE) / R_360);
                    while (Rz >= CONVERSION_ANGLE) Rz -= CONVERSION_ANGLE;
                    while (Rz <                 0) Rz += CONVERSION_ANGLE;
                }

                TKN_ReadToken(&TOK);
            }
            else
            {
                ASSERT(BIndex==-2);
                TKN_ReadToken(&TOK);
                while (TOK.EOLNs==0) TKN_ReadToken(&TOK); 
            }

            if (BIndex >= 0)
            {
                /////////////////////////////////////////////////////
                // Place translation values if it was the root bone
                /////////////////////////////////////////////////////
                if (BIndex == 0)
                {
                    s16* TransPtr;
                    TransPtr = &FrameData[Anim->NBones*3*NFramesAllocated];
                    TransPtr[(NFramesAllocated*0) + FrameNumber] = (s16)(Tx);
                    TransPtr[(NFramesAllocated*1) + FrameNumber] = (s16)(Ty);
                    TransPtr[(NFramesAllocated*2) + FrameNumber] = (s16)(Tz);
                }

                /////////////////////////////////////////////////////
                // Place the rotation values in the correct stream
                /////////////////////////////////////////////////////
                {
                    u16* RotPtr;
                    RotPtr = (u16*)&FrameData[BIndex*3*NFramesAllocated];
                    RotPtr[(NFramesAllocated*0) + FrameNumber] = (u16)(Rx);
                    RotPtr[(NFramesAllocated*1) + FrameNumber] = (u16)(Ry);
                    RotPtr[(NFramesAllocated*2) + FrameNumber] = (u16)(Rz);
                }
            }

            AMCBIndex++;
        }

        // Read next frame number
        if (TOK.TokenType == TOKEN_EOF) break;
        ASSERT(TOK.TokenType == TOKEN_NUMBER);
		if( TOK.TokenType != TOKEN_NUMBER ) break ;
    }

    TKN_CloseFile(&TOK);

    /////////////////////////////////////////////////////////////
    // Shrink data size
    /////////////////////////////////////////////////////////////
    ASSERT(Anim->NFrames <= NFramesAllocated);
    if (Anim->NFrames != NFramesAllocated)
    {
        s32  NewNFramesAllocated;
        s16* NewFrameData;

        // Allocate new data block
        NewNFramesAllocated = Anim->NFrames;
        NewFrameData = (s16*)x_malloc(sizeof(s16)*Anim->NStreams*NewNFramesAllocated);
        ASSERT(NewFrameData);

        // Copy old data into new block and clear unused section
        for (i=0; i<Anim->NStreams; i++)
        {
            x_memcpy( &NewFrameData[ i * NewNFramesAllocated ],
                      &FrameData   [ i * NFramesAllocated],
                      sizeof(s16) * NewNFramesAllocated);
        }

        // Setup new values
        NFramesAllocated = NewNFramesAllocated;
        x_free(FrameData);
        FrameData = NewFrameData;
    }
    Anim->Stream = FrameData;

    /////////////////////////////////////////////////////////////
    // Remove axis corrections
    /////////////////////////////////////////////////////////////
    for (i=0; i<Anim->NBones; i++)
    {
        u16*    RotPtr;
        radian3  R;
        matrix4 M,InvM,RM;

        // Build axis alignment matrices for this bone
		M.Identity();
		M.SetRotationXYZ( Skel->m_pBone[i].Axis );
		InvM = M;
		InvM.InvertSRT();

        // Setup ptr to rotation streams
        RotPtr = (u16*)&Anim->Stream[i*3*Anim->NFrames];

        // Transform rotations
        for (j=0; j<Anim->NFrames; j++)
        {
            R.X = (*((u16*)&RotPtr[(Anim->NFrames*0) + j]) * R_360) / (f32)CONVERSION_ANGLE;
            R.Y = (*((u16*)&RotPtr[(Anim->NFrames*1) + j]) * R_360) / (f32)CONVERSION_ANGLE;
            R.Z = (*((u16*)&RotPtr[(Anim->NFrames*2) + j]) * R_360) / (f32)CONVERSION_ANGLE;

            // RM = InvM * R * M
			RM.Identity();
			RM.SetRotationXYZ( R );
			RM.Transform( M );
			RM.PreTransform( InvM );

            // Pull rotations back out of matrix
			R = RM.GetRotationXYZ();

            // Build ANGLE_360 version of rotations
            Rx = (s32)((R.X * (f32)ANGLE_360) / R_360);
            while (Rx >= ANGLE_360)	Rx -= ANGLE_360;
            while (Rx < 0)			Rx += ANGLE_360;
            Ry = (s32)((R.Y * (f32)ANGLE_360) / R_360);
            while (Ry >= ANGLE_360) Ry -= ANGLE_360;
            while (Ry < 0)			Ry += ANGLE_360;
            Rz = (s32)((R.Z * (f32)ANGLE_360) / R_360);
            while (Rz >= ANGLE_360) Rz -= ANGLE_360;
            while (Rz < 0)			Rz += ANGLE_360;

            // Place new rotations back in stream
            RotPtr[(Anim->NFrames*0) + j] = (s16)Rx;
            RotPtr[(Anim->NFrames*1) + j] = (s16)Ry;
            RotPtr[(Anim->NFrames*2) + j] = (s16)Rz;
		}
    }

    /////////////////////////////////////////////////////////////
    // Minimize Angle differences
    /////////////////////////////////////////////////////////////
    for (i=0; i<Anim->NBones*3; i++)
        ANIM_MinimizeAngleDiff( Anim, i, ANGLE_360 );

    /////////////////////////////////////////////////////////////
    // Return success
    /////////////////////////////////////////////////////////////
    return ERR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  EXPORT AMC
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

err ANIM_ExportAMC( anim* Anim, CSkel* Skel, const char* FileName )
{
    s32 RX,RY,RZ;
    X_FILE* fp;
    s32 i,j;

    fp = x_fopen(FileName,"wb");
    if (!fp) return ERR_FAILURE;

    x_fprintf(fp,":FULLY-SPECIFIED\n");
    x_fprintf(fp,":DEGREES\n");

    for (i=0; i<Anim->NFrames; i++)
    {
        x_fprintf(fp,"%1d\n",i+1);

        for (j=0; j<Skel->m_NBones; j++)
        {
            x_fprintf(fp,"%s ",Skel->m_pBone[j].BoneName);
            
            if (j==0)
            {
                vector3  T;
                ANIM_GetTransValue(Anim,i,&T);
                x_fprintf(fp,"%f %f %f ",T.X,T.Y,T.Z);
            }

            RX = ANIM_GetRotValue( Anim,i,(j*3)+0 );
            RY = ANIM_GetRotValue( Anim,i,(j*3)+1 );
            RZ = ANIM_GetRotValue( Anim,i,(j*3)+2 );
            x_fprintf(fp,"%f %f %f\n",RX*(360/(f32)ANGLE_360),RY*(360/(f32)ANGLE_360),RZ*(360/(f32)ANGLE_360));
        }
    }

    x_fclose(fp);

    return ERR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////


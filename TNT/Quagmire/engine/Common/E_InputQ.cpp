///////////////////////////////////////////////////////////////////////////
//
//  E_INPUTQ.CPP
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//  INCLUDES
///////////////////////////////////////////////////////////////////////////
#include "x_plus.hpp"
#include "x_debug.hpp"
#include "E_InputQ.hpp"

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define QUEUE_SIZE      256

///////////////////////////////////////////////////////////////////////////
// LOCALS
///////////////////////////////////////////////////////////////////////////

// Circular queue for holding pending events.
static  input_event   s_Queue[ QUEUE_SIZE ];

static  xbool   s_Initialized = FALSE;
static  s32     s_Head;                   // Next event to leave.
static  s32     s_Tail;                   // Place for next arrival.
static  s32     s_Count;                  // Number of events in queue.

DECLARE_MUTEX( InputQueueMutex );


///////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void INPUTQ_InitModule( void )
{
    ASSERT( !s_Initialized );
    s_Initialized = TRUE;

    s_Head  = 0;
    s_Tail  = 0;
    s_Count = 0;

    INIT_MUTEX( InputQueueMutex );
}
 
///////////////////////////////////////////////////////////////////////////

void INPUTQ_KillModule( void )
{
    ASSERT( s_Initialized );
    s_Initialized = FALSE;

    KILL_MUTEX( InputQueueMutex );
}
 
///////////////////////////////////////////////////////////////////////////

void INPUTQ_EnQueue( input_event& Event )
{
    s32 Dest;
    s32 Prev;

    ASSERT( s_Initialized );
    ASSERT( Event.DeviceNum  < 100 );
    ASSERT( Event.DeviceNum  >= 0  );
    ASSERT( Event.DeviceType >= 0  );
    ASSERT( Event.DeviceType < DEVICE_END_OF_DEVICES );
    ASSERT( Event.GadgetType < GADGET_TYPE_END_OF_LIST );
    ASSERT( Event.GadgetType >= 0 );

    ENTER_MUTEX( InputQueueMutex );


    // Is there room in the queue?

    if( s_Count >= QUEUE_SIZE )
    {
        // We have an overflow of pending events.
        // What to do?

        // Ignore it for now.
        EXIT_MUTEX( InputQueueMutex );
        return;
    }

    // There is space in the queue to add the event.
     
    // There is a chance that events will be submitted to this queue
    // slightly out of order.  So when adding to the queue, the new
    // event should be insert sorted into the list.

    Dest = s_Head;                            // Consider insert at Head.
    Prev = Dest - 1;                        // Get previous event's index.
    if( Prev < 0 )  Prev = QUEUE_SIZE-1;    

    // Search for a safe place to put the new event.

    while( (Dest != s_Tail) && (Event.Timestamp < s_Queue[Prev].Timestamp) )
    {
        // We cannot put the new event at "dest" because there will
        // be an order problem.  So move the event at "prev" to "dest"
        // and loop again.
        s_Queue[Dest] = s_Queue[Prev];
        Dest = Prev;
        Prev = Dest - 1;
        if( Prev < 0 )  Prev = QUEUE_SIZE-1;
    }

    // Copy the new event into the queue at "dest".
    s_Queue[Dest] = Event;

    // Update the Head index.
    s_Head++;
    if( s_Head == QUEUE_SIZE )
        s_Head = 0;

    // Update the queue usage count.
    s_Count++;

    EXIT_MUTEX( InputQueueMutex );
}
 
///////////////////////////////////////////////////////////////////////////

xbool INPUTQ_DeQueue( input_event& Event )
{
    xbool Result = FALSE;

    ASSERT( s_Initialized );

    ENTER_MUTEX( InputQueueMutex );

    // Got anything in the queue?
    if( s_Count )
    {
        // Copy the event.
        Event = s_Queue[s_Tail];

        // Update the Tail index.
        s_Tail++;
        if( s_Tail >= QUEUE_SIZE )
            s_Tail -= QUEUE_SIZE;

        // Update the queue usage count.
        s_Count--;

        // And flag that an event is being provided.
        Result = TRUE;
    }

    EXIT_MUTEX( InputQueueMutex );

    return( Result );
}
 
///////////////////////////////////////////////////////////////////////////

s32 INPUTQ_GetNEvents( void )
{
    return( s_Count );
}

///////////////////////////////////////////////////////////////////////////

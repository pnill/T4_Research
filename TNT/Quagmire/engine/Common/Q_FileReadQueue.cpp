//=====================================================================================================================================
// Includes
//=====================================================================================================================================
#include "Q_FileReadQueue.h"

//=====================================================================================================================================
// Defines
//=====================================================================================================================================
enum FileQueueStates
{
    FILE_QUEUE_IDLE,
    FILE_QUEUE_READ_IN_PROGRESS,
};

//=====================================================================================================================================
// Declare once global sucker.
//=====================================================================================================================================
Q_FileReadQueue gGlobalFileReadQueue;


//=====================================================================================================================================
// User interface functions.
//=====================================================================================================================================
xbool ENG_FileQueueRequestRead( ReadRequest* NewReadRequest )
{
    // If there isn't a callback function, than this read request is useless, since there is no way to notify
    // when the read is complete.
    if( NewReadRequest->mpCallbackFunction == NULL )
    {
        ASSERT( 0 );
        return FALSE;
    }
    else
    {
        return gGlobalFileReadQueue.RequestRead( NewReadRequest );
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ENG_UpdateFileReadQueue( void )
{
    // Update any reads in progress.
    gGlobalFileReadQueue.Update( );
}


//=====================================================================================================================================
// Q_FileReadQueue
//=====================================================================================================================================
Q_FileReadQueue::Q_FileReadQueue( void )
{
    s32 index;

    for( index = 0; index < MAX_FILE_READ_REQUESTS; index++ )
        mReadRequests[index].Reset( );

    mReadIndex   = 0;
    mInsertIndex = 0;
    mStatus      = FILE_QUEUE_IDLE;

}


//-------------------------------------------------------------------------------------------------------------------------------------
Q_FileReadQueue::~Q_FileReadQueue( void )
{
    ClearAllNonActiveRequests( );
}



//-------------------------------------------------------------------------------------------------------------------------------------
xbool Q_FileReadQueue::RequestRead( ReadRequest* NewRequest )
{
    ReadRequest* pNextAvailableRequest;

    // Test first to see if the Queue is full.
    if( IsQueueFull( ) == FALSE )
    {
        pNextAvailableRequest = &mReadRequests[mInsertIndex];

        // Copy the New Request into that slot.
        *pNextAvailableRequest = *NewRequest;

        // Increment the next Insert Request Slot.
        IncrementInsertIndex( );
        return TRUE;
    }
    else
    {
        // The Queue was full, can't add the request.
#ifdef X_DEBUG
        x_printf( "The Q_FileReadQueue was full, ReadRequest failed.\n" );
#endif
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 Q_FileReadQueue::ReturnWaitingRequestCount( void )
{
    s32 ReadsToGo;

    // Determine how many ReadRequests are waiting to be done.
    if( mInsertIndex != mReadIndex )
    {
        if( mInsertIndex > mReadIndex )
            ReadsToGo = mInsertIndex - mReadIndex;
        else
            ReadsToGo = MAX_FILE_READ_REQUESTS - mReadIndex + mInsertIndex;
    }
    else
    {
        ReadsToGo = 0;
    }

    return ReadsToGo;
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool Q_FileReadQueue::Update( void )
{
    ReadRequest* pCurrentRequest;
    s32          AsyncReturnValue;
    s32          BytesRead;

    // Start a read if there is one in the queue.
    switch( mStatus )
    {
        case FILE_QUEUE_IDLE:
        {
            // If there are reads in waiting.
            if( ReturnWaitingRequestCount( ) )
            {
                // Get the first ReadRequest from the Array.
                pCurrentRequest = &mReadRequests[mReadIndex];

                // If there is a file to be read, start reading it.
                x_freada( pCurrentRequest->mpUserBuffer,
                          1,
                          pCurrentRequest->mNBytesToRead,
                          (X_FILE*)pCurrentRequest->mpFile,
                          pCurrentRequest->mReadOffset, 
                          X_PRIORITY_MEDIUM,
                          pCurrentRequest->mAsyncReadTaskID );

                // Update the status of the read.
                mStatus = FILE_QUEUE_READ_IN_PROGRESS;
            }
            else
            {
                return FALSE;
            }

            break;
        }

        case FILE_QUEUE_READ_IN_PROGRESS:
        {
            // Get the first ReadRequest from the Array.
            pCurrentRequest = &mReadRequests[mReadIndex];

            // Get the status on the read.
            AsyncReturnValue = x_freadastatus( pCurrentRequest->mAsyncReadTaskID, BytesRead );

            // Test to see if this is complete.
            switch( AsyncReturnValue )
            {
                // Still waiting to see what's happening with it.
                case X_STATUS_PENDING:
                case X_STATUS_INPROGRESS:
                    break;

                case X_STATUS_COMPLETE:
                case X_STATUS_ERROR:
                case X_STATUS_NOTFOUND:
                {
                    // If the read is complete, call the users callback function.
                    pCurrentRequest->mpCallbackFunction( pCurrentRequest, AsyncReturnValue, BytesRead );
                    
                    // Now reset the Read Request that just completed, and move the read index up one.
                    pCurrentRequest->Reset( );
                    IncrementReadIndex( );

                    // Return the status of the reads to IDLE to allow for more reads.
                    mStatus = FILE_QUEUE_IDLE;
                    break;
                }

                default:
                    ASSERT( 0 );
                    break;
            }

            break;
        }

        default:
        {
            ASSERT( 0 );
            break;
        }
    }

    return TRUE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void Q_FileReadQueue::ClearAllNonActiveRequests( void )
{
    s32 index;
    ReadRequest* pCurrentReadRequest;

    // If there are reads in waiting.
    if( ReturnWaitingRequestCount( ) )
    {
        // Go through all of the ReadRequests that are queued and just remove them.
        for( index = 0; index < MAX_FILE_READ_REQUESTS; index++ )
        {
            pCurrentReadRequest = &mReadRequests[index];

            // If this isn't the current read index, and there is a file associated to this read request, then kill it off.
            if( (index != mReadIndex) && (pCurrentReadRequest->mpFile != NULL) )
            {
                // Call the callback function to indicate that the read has been cancelled.
                pCurrentReadRequest->mpCallbackFunction( pCurrentReadRequest, X_STATUS_ERROR, 0 );
                
                // Now reset the now wasted read request.
                pCurrentReadRequest->Reset( );
            }
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void Q_FileReadQueue::IncrementReadIndex( void )
{ 
    mReadIndex++;
    mReadIndex = mReadIndex % MAX_FILE_READ_REQUESTS;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void Q_FileReadQueue::IncrementInsertIndex( void )
{ 
    mInsertIndex++;
    mInsertIndex = mInsertIndex % MAX_FILE_READ_REQUESTS;

    // If the insert index has caught up to the read index, there is trouble.
    if( mInsertIndex == mReadIndex )
    {
        ASSERT( 0 );
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool Q_FileReadQueue::IsQueueFull( void )
{
    xbool IsItFull = FALSE;

    // If the mInsertindex is knocking on the read index, that means the queue would fill up if a read was authorized.
    // Basically, you can never let the Insert Index equal the ReadIndex.
    if( mReadIndex > mInsertIndex )
    {
        if( (mReadIndex - mInsertIndex) > 1 )
            IsItFull = FALSE;
        else
            IsItFull = TRUE;
    }
    else
    {
        if( (mReadIndex == 0) && (mInsertIndex == (MAX_FILE_READ_REQUESTS - 1)) )
        {
            IsItFull = TRUE;
        }
        else
        {
            IsItFull = FALSE;
        }
    }

    return IsItFull;
}

#ifndef Q_FILE_READ_QUEUE_H
#define Q_FILE_READ_QUEUE_H

//=====================================================================================================================================
// Includes
//=====================================================================================================================================
#include "x_files.hpp"


//=====================================================================================================================================
// Forward Declarations
//=====================================================================================================================================
struct ReadRequest;


//=====================================================================================================================================
// Definitions
//=====================================================================================================================================
#define MAX_FILE_READ_REQUESTS      20
typedef void (*FileReadCallBack)(ReadRequest* pReadRequest, s32 FinalReadStatus, s32 BytesRead );


//=====================================================================================================================================
// User interface functions.
//=====================================================================================================================================
xbool ENG_FileQueueRequestRead( ReadRequest* NewReadRequest );
void  ENG_UpdateFileReadQueue( void );


//=====================================================================================================================================
// Structures and Classes.
//=====================================================================================================================================
struct ReadRequest
{
    X_FILE*             mpFile;             // File (must be opened already) to read from.
    void*               mpUserBuffer;       // Buffer to store the information from the file.
    s32                 mNBytesToRead;      // Number of bytes to read from the file.
    s32                 mReadOffset;        // Offset into the file to start reading from.
    s32                 mUserSpecial;       // User defined variable that you can do what you want with.
    s32                 mAsyncReadTaskID;   // Task ID provided by the Asnychronous engine code to identify this read.
    FileReadCallBack    mpCallbackFunction; // Callback function that is called when the read is completed by either an
                                            // error condition or it's actually finished.

    void Reset( void )
    {
        mpFile = NULL;
        mpUserBuffer = NULL;
        mNBytesToRead = 0;
        mReadOffset = 0;
        mUserSpecial = 0;
        mAsyncReadTaskID = -1;
        mpCallbackFunction = NULL;
    }
};

//=====================================================================================================================================
// 
// Q_FileReadQueue: This class will help better organize asychronous reads accross all platforms.
//                  To use it, must
//
class Q_FileReadQueue
{
    public:
        Q_FileReadQueue( void );
       ~Q_FileReadQueue( void );

        xbool RequestRead( ReadRequest* NewRequest );           // Insert a new read into the queue.
        s32   ReturnWaitingRequestCount( void );                // Return how many reads are in the queue that have not started.

        xbool Update( void );
        void  ClearAllNonActiveRequests( void );                // Void all reads that haven't started yet.


    private:
        void  IncrementReadIndex( void );
        void  IncrementInsertIndex( void );
        xbool IsQueueFull( void );

    private:
        ReadRequest     mReadRequests[MAX_FILE_READ_REQUESTS];  // An array of requests.
        s32             mReadIndex;                             // Index of the next Request to use.
        s32             mInsertIndex;                           // Index of where to store the next read request.
        s32             mStatus;                                // Status of the file queue.
};


#endif // FILE_READ_QUEUE_H
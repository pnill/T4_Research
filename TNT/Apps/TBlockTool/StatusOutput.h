////////////////////////////////////////////////////////////////////////////
//
// StatusOutput.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef STATUSOUTPUT_H_INCLUDED
#define STATUSOUTPUT_H_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"


////////////////////////////////////////////////////////////////////////////
// STATUS OUTPUT INTERFACE
////////////////////////////////////////////////////////////////////////////

namespace STATUS
{

////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

enum STATUS_MSG_TYPE
{
    MSG_NORMAL,         // plain message
    MSG_IMPORTANT,      // message is marked as important information
    MSG_WARNING,        // warning is generated, user should take note of it
    MSG_ERROR,          // serious error occured, message indicates problem

    MSG_TYPE_COUNT
};


////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////

typedef void (*HANDLER_Print        )( u32 cbParam, s32 MsgType, const char* pMsg );
typedef void (*HANDLER_ProgressStart)( u32 cbParam, s32 EndValue );
typedef void (*HANDLER_ProgressStep )( u32 cbParam, s32 NSteps );


////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// InitHandlers  - Sets application-specific handlers for status output
//                 operations. The cbParam argument is passed to the handlers.
//                 A handler may be NULL if no processing will be done for it.
//
// Print         - Prints a message to application specific output device,
//                 works like printf() and takes a MsgType parameter(STATUS_MSG_TYPE)
//                 to flag what kind of message it is.
//
// ProgressStart - Starts application specific progress report, resetting
//                 progress point to zero and setting the new range to EndValue.
//
// ProgressStep  - Increments app. specific progress report by NSteps.
//
////////////////////////////////////////////////////////////////////////////

void InitHandlers( u32                   cbParam,
                   HANDLER_Print         PrintHandler,
                   HANDLER_ProgressStart ProgressStartHandler,
                   HANDLER_ProgressStep  ProgressStepHandler );

void GetHandlers ( u32&                   cbParam,
                   HANDLER_Print&         PrintHandler,
                   HANDLER_ProgressStart& ProgressStartHandler,
                   HANDLER_ProgressStep&  ProgressStepHandler );


void Print        ( s32 MsgType, const char* pMsg, ... );
void ProgressStart( s32 EndValue );
void ProgressStep ( s32 NSteps = 1 );


////////////////////////////////////////////////////////////////////////////

} // namespace STATUS

////////////////////////////////////////////////////////////////////////////

#endif // STATUSOUTPUT_H_INCLUDED
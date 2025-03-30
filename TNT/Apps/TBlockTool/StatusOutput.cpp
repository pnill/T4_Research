////////////////////////////////////////////////////////////////////////////
//
// StatusOutput.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"

#include "StatusOutput.h"


////////////////////////////////////////////////////////////////////////////
// DEFAULT HANDLERS
////////////////////////////////////////////////////////////////////////////

static void DefaultPrint( u32 cbParam, s32 MsgType, const char* pMsg )
{
}

static void DefaultProgStart( u32 cbParam, s32 EndValue )
{
}

static void DefaultProgStep( u32 cbParam, s32 NSteps )
{
}

////////////////////////////////////////////////////////////////////////////
// STORAGE
////////////////////////////////////////////////////////////////////////////

namespace STATUS
{

static u32                   s_cbParam = 0;
static HANDLER_Print         s_PrintHandler = DefaultPrint;
static HANDLER_ProgressStart s_ProgressStartHandler = DefaultProgStart;
static HANDLER_ProgressStep  s_ProgressStepHandler = DefaultProgStep;

static char s_PrintBuffer[300];


////////////////////////////////////////////////////////////////////////////
// STATUS OUTPUT IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

void InitHandlers( u32                   cbParam,
                   HANDLER_Print         PrintHandler,
                   HANDLER_ProgressStart ProgressStartHandler,
                   HANDLER_ProgressStep  ProgressStepHandler )
{
    s_cbParam = cbParam;

    if( PrintHandler != NULL )
        s_PrintHandler = PrintHandler;
    else
        s_PrintHandler = DefaultPrint;

    if( ProgressStartHandler != NULL )
        s_ProgressStartHandler = ProgressStartHandler;
    else
        s_ProgressStartHandler = DefaultProgStart;

    if( ProgressStepHandler != NULL )
        s_ProgressStepHandler = ProgressStepHandler;
    else
        s_ProgressStepHandler = DefaultProgStep;
}

//==========================================================================

void GetHandlers( u32&                   cbParam,
                  HANDLER_Print&         PrintHandler,
                  HANDLER_ProgressStart& ProgressStartHandler,
                  HANDLER_ProgressStep&  ProgressStepHandler )
{
    cbParam = s_cbParam;
    PrintHandler = s_PrintHandler;
    ProgressStartHandler = s_ProgressStartHandler;
    ProgressStepHandler = s_ProgressStepHandler;
}

//==========================================================================

void Print( s32 MsgType, const char* pMsg, ... )
{
    x_va_list ArgList;

    x_va_start( ArgList, pMsg );

    x_vsprintf( s_PrintBuffer, pMsg, ArgList );

    x_va_end( ArgList );

    s_PrintHandler( s_cbParam, MsgType, s_PrintBuffer );
}

//==========================================================================

void ProgressStart( s32 EndValue )
{
    s_ProgressStartHandler( s_cbParam, EndValue );
}

//==========================================================================

void ProgressStep( s32 NSteps )
{
    s_ProgressStepHandler( s_cbParam, NSteps );
}

//==========================================================================

} // namespace STATUS
///////////////////////////////////////////////////////////////////////////
//
//  DMAMAN.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef DMAMAN_HPP
#define DMAMAN_HPP

///////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"

///////////////////////////////////////////////////////////////////////////

#define DMAMAN_PATH_VIF1    0
#define DMAMAN_PATH_GIF     1

///////////////////////////////////////////////////////////////////////////

void    DMAMAN_Init             ( void );
void    DMAMAN_Kill             ( void );

void    DMAMAN_NewTask          ( s32 Path, void* DMAChain );
void    DMAMAN_HandlersOn       ( void );
void    DMAMAN_HandlersOff      ( void );
void    DMAMAN_ExecuteTasks     ( void );
xbool   DMAMAN_AreTasksFinished ( void );

void    DMAMAN_PrintTaskStats   ( void );

void    DMAMAN_EndDList         ( void );

f32     DMAMAN_GetTaskTime      ( s32 Path, s32 TaskNum );

void    DMAMAN_DumpRunChainTasks( s32 ChainID, s32 TaskCount );


///////////////////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////////////////
//
//  DMAMAN.CPP
//
///////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <math.h>
#include <libdma.h>
#include <libgraph.h>
#include <sifdev.h>
#include <libvu0.h>
#include <stdio.h>

#include "x_debug.hpp"
#include "x_stdio.hpp"             
#include "x_plus.hpp"
#include "x_time.hpp"

#include "dvtrace.hpp"
#include "dmahelp.hpp"

#include "dmaman.hpp"
#include "Q_PS2.hpp"
//=========================================================================

#define MAX_TASKS   15

//=========================================================================

typedef struct
{
    s64 StartTime;
    s64 EndTime;
    u32 DMAAddr;
} dma_task;

typedef struct
{
    s64         StartTime;
    s64         EndTime;
    dma_task    Task[ MAX_TASKS ];
    s32         NTasks;
    s32         PrevNTasks;
    s32         CurrentTaskID;
    xbool       IsFinished;
} dma_task_chain;

//=========================================================================

static dma_task_chain   s_Chain0Array[2];
static dma_task_chain   s_Chain1Array[2];
static dma_task_chain*  s_RunChain[2];
static dma_task_chain*  s_BuildChain[2];
static sceDmaChan*      s_DMA[2];
static s32              s_InterruptCount[2];
static s32              s_HandlerCount[2];
static xbool            s_HandlersOn;

static s32              dma_handler_id[2];

//#define DEF_PRIORITY        0
//#define DEF_STACK_SIZE      2048
//static u64 DMAThreadStack[DEF_STACK_SIZE/8];
//static s32 DMAThreadID;
//static volatile s32 DMAThreadCount=0;

void SaveVIF1PerfFrame( void );

//=========================================================================
//=========================================================================
//=========================================================================
// INTERRUPT HANDLER
//=========================================================================
//=========================================================================
//=========================================================================

static 
void LaunchTask( dma_task* pTask, sceDmaChan* pDMA, s32 Path, s64 StartTime )
{
    u32 C;

    // Store starting time of task
    pTask->StartTime = StartTime;
    
    // Let path3 be intermittent    
    {
        // continuous = 0
        // intermittent = 4
        (*GIF_MODE) |= 0x04;
    }
    
    // Setup DMA
	pDMA->chcr.TTE = 0;
	pDMA->chcr.TIE = 1;
    C = *D_STAT;
    if( ((C>>(16+Path+1))&0x01) == 0 )
        *D_STAT = (u32)((1<<(16+Path+1)) | (1<<(Path+1)));
    
    // Kick off DMA
    sceDmaSend( pDMA, (void*)pTask->DMAAddr );
    
}

//=========================================================================

static 
int DMA0InterruptHandler( int ca )
{
    dma_task* pTask;    
    s64 Ticks;

    if( s_HandlersOn == FALSE )
        return 1;
    
    s_HandlerCount[0]++;
     
    ca = 0;   
    Ticks = x_GetTimerTicks();
    
    // Collect timing info for completed task
    pTask = &s_RunChain[0]->Task[ s_RunChain[0]->CurrentTaskID ];
    pTask->EndTime = Ticks;
    
    // Increment task index
    s_RunChain[0]->CurrentTaskID++;
    
    // Check if task was last one
    if( s_RunChain[0]->CurrentTaskID == s_RunChain[0]->NTasks )
    {
        s_RunChain[0]->IsFinished = TRUE;
        s_RunChain[0]->EndTime    = Ticks;
        SaveVIF1PerfFrame();
    }
    else
    {
        pTask = &s_RunChain[0]->Task[ s_RunChain[0]->CurrentTaskID ];
        LaunchTask( pTask, s_DMA[0], 0, Ticks );
    }
    
    // Allow additional service to run, -1 prevents chaining interrupt
    return 1;
}

//=========================================================================

static 
int DMA1InterruptHandler( int ca )
{
    dma_task* pTask;    
    s64 Ticks;
    s32 P = 1;

    if( s_HandlersOn == FALSE )
        return 1;

    s_HandlerCount[P]++;
 
    ca = 0;   
    Ticks = x_GetTimerTicks();
    
    // Collect timing info for completed task
    pTask = &s_RunChain[P]->Task[ s_RunChain[P]->CurrentTaskID ];
    pTask->EndTime = Ticks;
    
    // Increment task index
    s_RunChain[P]->CurrentTaskID++;
    
    // Check if task was last one
    if( s_RunChain[P]->CurrentTaskID == s_RunChain[P]->NTasks )
    {
        s_RunChain[P]->IsFinished = TRUE;
        s_RunChain[P]->EndTime    = Ticks;
    }
    else
    {
        pTask = &s_RunChain[P]->Task[ s_RunChain[P]->CurrentTaskID ];
        LaunchTask( pTask, s_DMA[P], P, Ticks );
    }
    
    
    // Allow additional service to run, -1 prevents chaining interrupt
    return 1;
}

//=========================================================================
/*
static void DMAThread( void )
{
    while(1)
    {
        DMAThreadCount++;
        SleepThread();
    }
}
*/
//=========================================================================

static
void InitDMAHandlers( void )
{
    // Install Interrupt Handler for end of dma transfer
    dma_handler_id[0] = AddDmacHandler( DMAC_VIF1, DMA0InterruptHandler, 0 );
    dma_handler_id[1] = AddDmacHandler( DMAC_GIF, DMA1InterruptHandler, 0 );
    EnableDmac( DMAC_VIF1 );
    EnableDmac( DMAC_GIF );
}

//=========================================================================
static
void KillDMAHandlers( void )
{
    RemoveDmacHandler( DMAC_VIF1, dma_handler_id[0] );
    RemoveDmacHandler( DMAC_GIF,  dma_handler_id[1] );
    DisableDmac( DMAC_VIF1 );
    DisableDmac( DMAC_GIF );
}

//=========================================================================

void DMAMAN_Kill( void )
{
    KillDMAHandlers();
}

//=========================================================================

void DMAMAN_Init( void )
{
    // Get dma channel
	s_DMA[0] = sceDmaGetChan(SCE_DMA_VIF1);
	s_DMA[1] = sceDmaGetChan(SCE_DMA_GIF);
	
	// Install interrupts
	InitDMAHandlers();
	s_HandlersOn = FALSE;
	
	// Setup initial chains
    x_memset( s_Chain0Array, 0, sizeof(s_Chain0Array) );
    s_BuildChain[0] = &s_Chain0Array[0];
    s_RunChain[0] = &s_Chain0Array[1];
    s_RunChain[0]->IsFinished = TRUE;
    
    x_memset( s_Chain1Array, 0, sizeof(s_Chain1Array) );
    s_BuildChain[1] = &s_Chain1Array[0];
    s_RunChain[1] = &s_Chain1Array[1];
    s_RunChain[1]->IsFinished = TRUE;


    x_printf( "s_Chain0Array[ 0 ].Task[ 0 ].DMAAddr = 0x%08X\n", (u32)&s_Chain0Array[ 0 ].Task[ 0 ].DMAAddr );
    x_printf( "s_Chain0Array[ 0 ].Task[ 1 ].DMAAddr = 0x%08X\n", (u32)&s_Chain0Array[ 0 ].Task[ 1 ].DMAAddr );
    x_printf( "s_Chain0Array[ 1 ].Task[ 0 ].DMAAddr = 0x%08X\n", (u32)&s_Chain0Array[ 1 ].Task[ 0 ].DMAAddr );
    x_printf( "s_Chain0Array[ 1 ].Task[ 1 ].DMAAddr = 0x%08X\n", (u32)&s_Chain0Array[ 1 ].Task[ 1 ].DMAAddr );
#if !defined( TARGET_PS2_DVD )
    printf  ( "s_Chain0Array[ 0 ].Task[ 0 ].DMAAddr = 0x%08X\n", (u32)&s_Chain0Array[ 0 ].Task[ 0 ].DMAAddr );
    printf  ( "s_Chain0Array[ 0 ].Task[ 1 ].DMAAddr = 0x%08X\n", (u32)&s_Chain0Array[ 0 ].Task[ 1 ].DMAAddr );
    printf  ( "s_Chain0Array[ 1 ].Task[ 0 ].DMAAddr = 0x%08X\n", (u32)&s_Chain0Array[ 1 ].Task[ 0 ].DMAAddr );
    printf  ( "s_Chain0Array[ 1 ].Task[ 1 ].DMAAddr = 0x%08X\n", (u32)&s_Chain0Array[ 1 ].Task[ 1 ].DMAAddr );
#endif
}

//=========================================================================

void DMAMAN_NewTask( s32 Path, void* DMAChain )
{
    dma_task* pTask;

    ASSERT( (Path>=0) && (Path<=1) );
    
    // Add task to BuildChain1
    ASSERT( s_BuildChain[Path]->NTasks < MAX_TASKS );
    pTask = &s_BuildChain[Path]->Task[ s_BuildChain[Path]->NTasks ];
    s_BuildChain[Path]->NTasks++;
    
    // Build task info
    pTask->DMAAddr   = (u32)DMAChain;
    pTask->StartTime = 0;
    pTask->EndTime   = 0;
}

//=========================================================================

void DMAMAN_ExecuteTasks( void )
{
    dma_task_chain* pChain;
    s64 StartTime;
    s32 i;
    
    // Get Current time
    StartTime = x_GetTimerTicks();

    for( i=0; i<2; i++ )
    {
        // Confirm previous chain is finished
        ASSERT( s_RunChain[i]->IsFinished );
        
        // Clear counters
        s_InterruptCount[i] = s_HandlerCount[i];
        s_HandlerCount[i] = 0;
    
        // Finish building chain
        s_BuildChain[i]->CurrentTaskID = 0;
        s_BuildChain[i]->IsFinished    = FALSE;    
        
        // Flip Run and Build
        pChain = s_RunChain[i];
        s_RunChain[i] = s_BuildChain[i];
        s_BuildChain[i] = pChain;
        
        // Kick off first tasks
        if( s_RunChain[i]->NTasks > 0 )
        {
            s_RunChain[i]->StartTime  = StartTime;
            s_RunChain[i]->EndTime    = 0;
            LaunchTask( s_RunChain[i]->Task, s_DMA[i], i, StartTime );
        }
        else
        {
            s_RunChain[i]->StartTime  = StartTime;
            s_RunChain[i]->EndTime    = StartTime;
            s_RunChain[i]->IsFinished = TRUE;
        }
        
        // Prep next build chain
        s_BuildChain[i]->PrevNTasks = s_BuildChain[i]->NTasks;
        s_BuildChain[i]->NTasks = 0;
    }
}

//=========================================================================

void DMAMAN_EndDList( void )
{
    sceDmaTag* pTag = (sceDmaTag*)pDList;
    
    // Make sure size is 16 byte multiple
    ASSERT( (((u32)pTag) & 0xF) == 0 );
    pTag->qwc  = 0;
    pTag->id   = 0x70;
    pTag->next = 0;
    pTag->mark = 17;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
    
    // Set IRQ bit
    ((u32*)pTag)[0] |= 0x80000000;
    
    pDList += sizeof(sceDmaTag);
}

//=========================================================================

xbool   DMAMAN_AreTasksFinished ( void )
{
    if( s_RunChain[0]->IsFinished && s_RunChain[1]->IsFinished )
        return TRUE;
    
    return FALSE;
}

//=========================================================================

void    OLDDMAMAN_PrintTaskStats ( void )
{
    s32 i,j;
    s64 TicksPerSec;
    s64 BaseTime;
    
    TicksPerSec = x_GetTimerTicksPerSec();
    x_printf("=====================================\n");
    x_printf("DMAMANAGER - Task Times\n");

    BaseTime = MIN( s_BuildChain[0]->StartTime,s_BuildChain[1]->StartTime);


    for( j=0; j<2; j++ )
    {
        s64 Total=0;
        
        x_printf("=====================================\n");
        x_printf("CALLS: %1d\n",s_InterruptCount[j]);
        x_printf("=====================================\n");
        
        
        for( i=0; i<s_BuildChain[j]->PrevNTasks; i++ )
        {
            dma_task* pTask = &s_BuildChain[j]->Task[i];
            s64 Diff;
            f32 ITime;
            f32 TTime;
            s32 StartTime,EndTime;
            
            StartTime = (s32)((1000000*(pTask->StartTime - BaseTime))/TicksPerSec);
            EndTime   = (s32)((1000000*(pTask->EndTime - BaseTime))/TicksPerSec);
            
            
            Diff = pTask->EndTime-pTask->StartTime;
            Total += Diff;
            ITime = 0.001f*(f32)((1000000 * Diff)/TicksPerSec);
            TTime = 0.001f*(f32)((1000000 * Total)/TicksPerSec);
            
            x_printf("%1d %2d] %6.3f %6.3f %08d %08d\n",j,i,ITime,TTime,StartTime,EndTime);
        }
    }
    
    x_printf("=====================================\n");

    
    
}

//=========================================================================

typedef struct
{
    s32  Time;
    char Msg[128];
} task_stat;

void DMAMAN_PrintTaskStats ( void )
{
    s64 TicksPerSec;
    s64 BaseTime;
    task_stat Stat0[4];
    task_stat Stat1[4];
    s32       NStats0;
    s32       NStats1;
    s32       TaskID0;
    s32       TaskID1;
    s32       Time0,Time1;
    
    char*     StartFormat0 = "=== START [%03d] <%06d>          ===    =====================================";
    char*     StartFormat1 = "=====================================    === START [%03d] <%06d>          ===";
    char*     EndFormat0   = "=== END   [%03d] <%06d> (%06d) ===    =====================================";
    char*     EndFormat1   = "=====================================    === END   [%03d] <%06d> (%06d) ===";
    
    OLDDMAMAN_PrintTaskStats();
    
    TicksPerSec = x_GetTimerTicksPerSec();
    BaseTime = MIN( s_BuildChain[0]->StartTime,s_BuildChain[1]->StartTime);
    
    x_printf("=====================================\n");
    x_printf("DMAMANAGER - Task Interlacing\n");
    x_printf("=====================================    =====================================\n");

    TaskID0 = TaskID1 = 0;
    NStats0 = NStats1 = 0;
    while(1)
    {
        // Check if we've scanned all tasks
        if( (TaskID0 == s_BuildChain[0]->PrevNTasks) &&
            (TaskID1 == s_BuildChain[1]->PrevNTasks) &&
            (NStats0 == 0) && (NStats1 == 0) )
            break;
            
        // Check if there are entries in the stat queues
        if( NStats0 == 0 )
        {
            if( TaskID0 < s_BuildChain[0]->PrevNTasks )
            {
                dma_task* pTask = &s_BuildChain[0]->Task[TaskID0];
                Stat0[0].Time = (s32)((1000000*(pTask->StartTime - BaseTime))/TicksPerSec);
                Stat0[1].Time = (s32)((1000000*(pTask->EndTime - BaseTime))/TicksPerSec);
                x_sprintf(Stat0[0].Msg,StartFormat0,TaskID0,Stat0[0].Time);
                x_sprintf(Stat0[1].Msg,EndFormat0,TaskID0,Stat0[1].Time,(Stat0[1].Time-Stat0[0].Time));
                NStats0 = 2;
                TaskID0++;
            }
        }
        
        // Check if there are entries in the stat queues
        if( NStats1 == 0 )
        {
            if( TaskID1 < s_BuildChain[1]->PrevNTasks )
            {
                dma_task* pTask = &s_BuildChain[1]->Task[TaskID1];
                Stat1[0].Time = (s32)((1000000*(pTask->StartTime - BaseTime))/TicksPerSec);
                Stat1[1].Time = (s32)((1000000*(pTask->EndTime - BaseTime))/TicksPerSec);
                x_sprintf(Stat1[0].Msg,StartFormat1,TaskID1,Stat1[0].Time);
                x_sprintf(Stat1[1].Msg,EndFormat1,TaskID1,Stat1[1].Time,(Stat1[1].Time-Stat1[0].Time));
                NStats1 = 2;
                TaskID1++;
            }
        }
        
        // Pull stat from queue and display
        if( NStats0==0 ) Time0 = 0x7FFFFFFF;
        else             Time0 = Stat0[0].Time;
        
        if( NStats1==0 ) Time1 = 0x7FFFFFFF;
        else             Time1 = Stat1[0].Time;
        
        if( Time0 < Time1 )
        {
            x_printf("%1s\n",Stat0[0].Msg);
            Stat0[0] = Stat0[1];
            NStats0--;
        }
        else
        {
            x_printf("%1s\n",Stat1[0].Msg);
            Stat1[0] = Stat1[1];
            NStats1--;
        }
    
    }

    x_printf("=====================================    =====================================\n");

    
}

//=========================================================================

void    DMAMAN_HandlersOn       ( void )
{
    s_HandlersOn = TRUE;
}

//=========================================================================

void    DMAMAN_HandlersOff      ( void )
{
    s_HandlersOn = FALSE;
}

//=========================================================================

//==========================================================================
//==========================================================================
//  PERFORMANCE TUNING FUNCTIONS
//==========================================================================
//==========================================================================

static s32      s_NPerfDescriptions = 0;
static char**   s_pPerfDescriptions = NULL;
static s32      s_NFrames = 0;
static byte*    s_pCurrFrameData = NULL;
static xbool    s_PerfTimingActive = FALSE;

#define MAX_NUM_PERFTASKS   10

typedef struct
{
    s32 NTasks;
    s64 TaskTimes[MAX_NUM_PERFTASKS]; // in Ticks
} SPerfFrame;

//==========================================================================

void PS2_SetRenderModeDescriptions( char** Descriptions, s32 NDescriptions )
{
    s_pPerfDescriptions = Descriptions;
    s_NPerfDescriptions = NDescriptions;
}

//==========================================================================

void PS2_BeginTiming( void )
{
    s_NFrames = 0;
    s_pCurrFrameData = (byte*)0x04000000;   // The 64 MB mark. x_memory should never get up that high
    s_PerfTimingActive = TRUE;
}

//==========================================================================

void PS2_EndTiming( char* FileName )
{
    s32         i, j;
    byte*       pStart = (byte*)0x04000000;
    X_FILE*     pFH;
    SPerfFrame* pFrame;
    s64         TotalTime;
    s64         Time;
    f32         AvgTime;

    //---   verify that the number of Begin and End RenderModes were the same every frame
    ASSERT( s_NFrames );
    ASSERT( ((SPerfFrame*)pStart)->NTasks );
    s_pCurrFrameData = pStart;
    for ( i = 0; i < s_NFrames; i++ )
    {
        ASSERT( ((SPerfFrame*)pStart)->NTasks == ((SPerfFrame*)s_pCurrFrameData)->NTasks );
        s_pCurrFrameData += sizeof(SPerfFrame);
    }

    //---   open up the file for writing
    pFH = x_fopen( FileName, "wb" );
    ASSERT( pFH );

    //---   write out some header info
    x_fprintf( pFH, "DMA TIMINGS\n\n" );

    //---   tally up some averages, and output those
    for ( i = 0; i < ((SPerfFrame*)pStart)->NTasks; i++ )
    {
        if ( i < s_NPerfDescriptions )
            x_fprintf( pFH, "%10s:  ", s_pPerfDescriptions[i] );
        else
            x_fprintf( pFH, "%10s:  ", "UNKNOWN" );

        AvgTime = 0.0f;
        pFrame = (SPerfFrame*)pStart;
        for ( j = 0; j < s_NFrames; j++ )
        {
            AvgTime += x_TicksToMs( pFrame->TaskTimes[i] );

            pFrame++;
        }

        AvgTime /= (f32)s_NFrames;

        x_fprintf( pFH, "%2.3f\n", AvgTime );
    }
    x_fprintf( pFH, "\n" );
    
    //---   write out comma-delimited text describing the timings
    for ( i = 0; i < ((SPerfFrame*)pStart)->NTasks; i++ )
    {
        if ( i < s_NPerfDescriptions )
            x_fprintf( pFH, "%s, ", s_pPerfDescriptions[i] );
        else
            x_fprintf( pFH, "UNKNOWN TASK, " );
    }

    x_fprintf( pFH, "TOTAL\n" );

    //---   write out comma-delimited text for all of the timings
    pFrame = (SPerfFrame*)pStart;
    for ( i = 0; i < s_NFrames; i++ )
    {
        TotalTime = 0;

        for ( j = 0; j < pFrame->NTasks; j++ )
        {
            Time = pFrame->TaskTimes[j];
            TotalTime += Time;
            x_fprintf( pFH, "%f, ", x_TicksToMs( Time ) );
        }
        x_fprintf( pFH, "%f\n", x_TicksToMs( TotalTime ) );

        pFrame++;
    }

    //---   close the file
    x_fclose( pFH );

    s_PerfTimingActive = FALSE;
}

//==========================================================================

void SaveVIF1PerfFrame( void )
{
    s32         i;
    SPerfFrame* pFrame;

    if ( s_PerfTimingActive == FALSE )
        return;

    pFrame = (SPerfFrame*)s_pCurrFrameData;
    pFrame->NTasks = s_RunChain[0]->NTasks;
    for ( i = 0; i < s_RunChain[0]->NTasks; i++ )
    {
        pFrame->TaskTimes[i] = s_RunChain[0]->Task[i].EndTime - s_RunChain[0]->Task[i].StartTime;
    }

    s_pCurrFrameData += sizeof(SPerfFrame);
    s_NFrames++;
}

//==========================================================================

f32 DMAMAN_GetTaskTime( s32 Path, s32 TaskNum )
{
    ASSERT( DMAMAN_AreTasksFinished() );

    return x_TicksToMs(s_RunChain[Path]->Task[TaskNum].EndTime - s_RunChain[Path]->Task[TaskNum].StartTime);
}

//==========================================================================

void DMAMAN_DumpRunChainTasks( s32 ChainID, s32 TaskCount )
{
    if (ChainID < 0 || ChainID > 1)
        return;

    if (TaskCount < 0 || TaskCount > MAX_TASKS)
        return;

    s32 i;

    for (i=0; i<TaskCount; ++i)
    {
#if !defined( TARGET_PS2_DVD )
        printf( "s_RunChain[ %d ]->Task[ %d ].DMAAddr:  0x%08X\n", ChainID, i, s_RunChain[ ChainID ]->Task[ i ].DMAAddr );
#endif
        x_printf( "s_RunChain[ %d ]->Task[ %d ].DMAAddr:  0x%08X\n", ChainID, i, s_RunChain[ ChainID ]->Task[ i ].DMAAddr );
    }

#if !defined( TARGET_PS2_DVD )
    printf( "s_DMA[0]->chcr.STR:  0x%08X\n", s_DMA[0]->chcr.STR );
    printf( "s_DMA[0]->madr:  0x%08X\n", s_DMA[0]->madr );
    printf( "s_DMA[0]->sadr:  0x%08X\n", s_DMA[0]->sadr );
    printf( "s_DMA[0]->tadr:  0x%08X\n", s_DMA[0]->tadr );
#endif
    x_printf( "s_DMA[0]->chcr.STR:  0x%08X\n", s_DMA[0]->chcr.STR );
    x_printf( "s_DMA[0]->madr:  0x%08X\n", s_DMA[0]->madr );
    x_printf( "s_DMA[0]->sadr:  0x%08X\n", s_DMA[0]->sadr );
    x_printf( "s_DMA[0]->tadr:  0x%08X\n", s_DMA[0]->tadr );

}

//==========================================================================

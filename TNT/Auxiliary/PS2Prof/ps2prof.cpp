////////////////////////////////////////////////////////////////////////////
//
//  PS2PROF.CPP
//
////////////////////////////////////////////////////////////////////////////

#include <eekernel.h>
#include <eeregs.h>
#include "x_types.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "x_debug.hpp"
#include "ps2prof.hpp"

#define TIME_PS2_CLOCK      150000000 // 150MHZ

static xbool    CPUProfilingOn = FALSE;
static xbool    GraphicsProfilingOn = FALSE;
static u32*     pBuffer;
static s32      BufferIndex;
static s32      BufferSize;
static X_FILE*  BufferFP;
static s32      Counter;


//=========================================================================

static void FlushBuffer( void )
{
    xbool OldCPUOn;
    xbool OldGraphicsOn;

    // Turn profiling off if needed
    OldCPUOn = CPUProfilingOn;
    OldGraphicsOn = GraphicsProfilingOn;
    if( CPUProfilingOn )
        PS2PROF_CPUOff();
    if( GraphicsProfilingOn )
        PS2PROF_GraphicsOff();

    // Write buffer to file
    if( BufferIndex > 0 )
    {
        x_fwrite( pBuffer, BufferIndex*4, 1, BufferFP );
        BufferIndex = 0;
    }

    // Turn profiling back on
    if( OldCPUOn )
        PS2PROF_CPUOn();
    if( OldGraphicsOn )
        PS2PROF_GraphicsOn();
}

//=========================================================================

static int ProfInterruptHandler( int ca )
{
    ca = 0;

    // Shut off counter
    *T1_MODE = *T1_MODE & (~T_MODE_CUE_M);

    // Record that interrupt kicked off
    Counter++;

    if( CPUProfilingOn )
    {
        u32 Addr;

        // Read address of location when interrupt occurred
        asm ("		sw	$8,%0			" : : "m" (Addr) );

        // Place address in buffer
        if( BufferIndex < BufferSize )
        {
            pBuffer[ BufferIndex ] = (Addr & 0x0FFFFFFF);
            BufferIndex++;
        }
    }

    if( GraphicsProfilingOn )
    {
        u32 VIF1;

        VIF1 = *VIF1_STAT;

        // Place address in buffer
        if( BufferIndex < BufferSize )
        {
            pBuffer[ BufferIndex ] = (VIF1 & 0x0FFFFFFF) | 0x10000000;
            BufferIndex++;
        }
    }

    // Clear equal bit and let counter continue
    *T1_MODE = *T1_MODE | T_MODE_EQUF_M | T_MODE_CUE_M;

    // Allow additional service to run, -1 prevents chaining interrupt
    return 1;
}

//=========================================================================

void PS2PROF_Init( s32 SamplesPerSec, s32 SamplesInBuffer )
{
    // Allocate buffer
    BufferSize = SamplesInBuffer;
    pBuffer = (u32*)x_malloc( sizeof(u32) * SamplesInBuffer );
    ASSERT( pBuffer );
    BufferIndex = 0;

    // Open output file
    BufferFP = x_fopen("profbuff.bin","wb");
    ASSERT(BufferFP);
                                                            
    // Turn off profiling
    CPUProfilingOn = FALSE;
    GraphicsProfilingOn = FALSE;
    Counter = 0;

    // Install Interrupt Handler for Timer1 Overflow
    AddIntcHandler( INTC_TIM1, ProfInterruptHandler, 0 );

    // Count 1/16th of Bus Clock, Enable Overflow Interrupt and Start
    *T1_COUNT = 0;                                                  
    *T1_COMP  = (TIME_PS2_CLOCK/SamplesPerSec);
    *T1_MODE  = 0x00 |          // BUSCLK speed (150MHZ)
                T_MODE_ZRET_M | // Clear counter when it reaches COMP
//                T_MODE_CUE_M |  // Starts/Restarts counting
                T_MODE_CMPE_M;  // Do compare interrupt

    // Enable Interrupt VBlankON
    EnableIntc( INTC_TIM1 );

    PS2PROF_CPUOff();
    PS2PROF_GraphicsOff();
}

//=========================================================================

void PS2PROF_Kill( void )
{
    // Turn off profiling
    CPUProfilingOn = FALSE;
    GraphicsProfilingOn = FALSE;

    // Remove Timer0 Interrupt Handler and Disable Interrupt
    RemoveIntcHandler( INTC_TIM1, ProfInterruptHandler(0) );
    DisableIntc( INTC_TIM1 );

    // Free buffer
    FlushBuffer();
    x_fclose(BufferFP);
    x_free( pBuffer );
}

//=========================================================================

void PS2PROF_GraphicsOn( void )
{
    GraphicsProfilingOn = TRUE;
    if( GraphicsProfilingOn || CPUProfilingOn )
        *T1_MODE = *T1_MODE | (T_MODE_CUE_M);
}

//=========================================================================

void PS2PROF_GraphicsOff( void )
{
    GraphicsProfilingOn = FALSE;
    if( !GraphicsProfilingOn && !CPUProfilingOn )
        *T1_MODE = *T1_MODE & (~T_MODE_CUE_M);
}

//=========================================================================

void PS2PROF_CPUOn( void )
{
    CPUProfilingOn = TRUE;
    if( GraphicsProfilingOn || CPUProfilingOn )
        *T1_MODE = *T1_MODE | (T_MODE_CUE_M);
}

//=========================================================================

void PS2PROF_CPUOff( void )
{
    CPUProfilingOn = FALSE;
    if( !GraphicsProfilingOn && !CPUProfilingOn )
        *T1_MODE = *T1_MODE & (~T_MODE_CUE_M);
}

//=========================================================================

xbool PS2PROF_IsCPUOn( void )
{
    return CPUProfilingOn;
}

//=========================================================================

xbool PS2PROF_IsGraphicsOn( void )
{
    return GraphicsProfilingOn;
}

//=========================================================================

s32 PS2PROF_GetCount( void )
{
    return Counter;
}

//=========================================================================

void PS2PROF_FlushBuffer( xbool Force = FALSE )
{
    if( Force || (BufferIndex == BufferSize) )
    {
        x_printf("******************* Flushing buffer.\n");
        FlushBuffer();
    }
}

//=========================================================================


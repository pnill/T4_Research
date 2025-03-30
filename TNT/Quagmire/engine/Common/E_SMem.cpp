////////////////////////////////////////////////////////////////////////////
//
// E_SMEM.CPP
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_plus.hpp"
#include "x_stats.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"

#include "E_SMem.hpp"
#include "Q_SMem.hpp"
#include "Q_Engine.hpp"


////////////////////////////////////////////////////////////////////////////
//
//  SCRATCH MEMORY LAYOUT:
//
//    +---Storage
//    |
//    V
//
//    +-------------------------------------------------------------+
//    |   STACK -->                                    <-- BUFFER   |
//    +-------------------------------------------------------------+
//
//    ^                                                             ^
//    |                                                             |
//    |\                                                            |
//    | +-----StackTop                                              |
//    |                                                             |
//   [+][-][-][-]... Marker[]                         BufferPoint---+
//    0  1  2  3                                               
//
//    TopMarker = 0
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define MAX_MARKERS 16

//#define SMEM_ADD_LINING       //Define to add SMEM lining


////////////////////////////////////////////////////////////////////////////
// LOCAL STORAGE
////////////////////////////////////////////////////////////////////////////

static xbool s_Initialized     = FALSE;
static s32   s_ActiveSection;

static byte* s_Storage[2]      = { NULL, NULL };
static s32   s_StorageSize;
static s32   s_DesiredStorageSize;

static byte* s_Marker[ MAX_MARKERS ];       // Array of markers into stack.
static s32   s_TopMarker;                   // Index to topmost marker.

static byte* s_BufferPoint;                 // "Left" edge of the buffer.
static byte* s_StackTop;                    // "Right" edge of the stack.

PSTAT( StatSMemAvailable  );                // Current free scratch memory.
PSTAT( StatSMemStackUsed  );                // Current scratch stack used.
PSTAT( StatSMemBufferUsed );                // Current scratch buffer used.


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Internal private functions.
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void SMEM_ResetBuffer( void )
{
    s_StackTop    = s_Storage[s_ActiveSection];
    s_BufferPoint = s_Storage[s_ActiveSection] + s_StorageSize;

    // Marker[0] always points to the bottom of the stack.
    // This marker is reserved and cannot be popped.
    s_TopMarker   = 0;
    s_Marker[ 0 ] = s_StackTop;

    // Keep our stats up to date.
    PSTAT_SetValue( StatSMemAvailable,  s_StorageSize );
    PSTAT_SetValue( StatSMemStackUsed,  0 );
    PSTAT_SetValue( StatSMemBufferUsed, 0 );
}

////////////////////////////////////////////////////////////////////////////

void SMEM_InitModule( s32 NBytes )
{
    ASSERT( !s_Initialized );
    ASSERT( NBytes >= 0 );

    NBytes = ALIGN_16( NBytes );

    s_Storage[0]  = (byte*)x_malloc( NBytes );
    s_Storage[1]  = (byte*)x_malloc( NBytes );
    ASSERT( s_Storage[0] && s_Storage[1] );

    s_StorageSize = NBytes;
    s_DesiredStorageSize = s_StorageSize;

    s_ActiveSection = 0;
    SMEM_ResetBuffer();

    s_Initialized = TRUE;
}

////////////////////////////////////////////////////////////////////////////

void SMEM_KillModule( void )
{
    if( s_Initialized == 0 )
        return;

    x_free( s_Storage[0] );
    x_free( s_Storage[1] );

    s_Storage[0] = NULL;
    s_Storage[1] = NULL;

    s_Initialized = FALSE;
}

////////////////////////////////////////////////////////////////////////////

void SMEM_Toggle( void )
{
    ASSERT( s_Initialized );

    ASSERTS( s_TopMarker == 0, "Client app did not pop all scratch stack markers." );

    // Check if user has requested change in size
    if (s_DesiredStorageSize != s_StorageSize)
    {
        // Be sure the other buffer is no longer in use.
        ENG_WaitForGraphicsSynch();
        ENG_WaitForGraphicsSynch();

        // Free current buffers
        x_free( s_Storage[0] );
        x_free( s_Storage[1] );

        // Allocate new buffers
        s_Storage[0]  = (byte*)x_malloc( s_DesiredStorageSize );
        s_Storage[1]  = (byte*)x_malloc( s_DesiredStorageSize );
        ASSERT( s_Storage[0] && s_Storage[1] );
        s_StorageSize = s_DesiredStorageSize;
    }

    // Flip to next buffer as usual
    s_ActiveSection = 1 - s_ActiveSection;
    SMEM_ResetBuffer();
}

////////////////////////////////////////////////////////////////////////////

byte* SMEM_StackAlloc( s32 NBytes )
{
    byte* Result = s_StackTop;

    ASSERT( s_Initialized );
    ASSERT( NBytes > 0 );

    if( NBytes <= 0 )
        return NULL;

#ifdef SMEM_ADD_LINING
    NBytes += 16;
#endif

    NBytes = ALIGN_8( NBytes );

    s_StackTop += NBytes;
    ASSERTS( s_StackTop < s_BufferPoint, "Out of scratch memory" );

    if( s_StackTop >= s_BufferPoint )
    {
        s_StackTop -= NBytes;
        return NULL;
    }

    PSTAT_AddValue( StatSMemStackUsed,  NBytes );
    PSTAT_AddValue( StatSMemAvailable, -NBytes );

#ifdef SMEM_ADD_LINING
    u32* pLining = (u32*)(s_StackTop-16);
    pLining[0] = 0xDEADBEEF;
    pLining[1] = 0xDEADBEEF;
    pLining[2] = 0xDEADBEEF;
    pLining[3] = 0xDEADBEEF;
#endif

    return( Result );
}

////////////////////////////////////////////////////////////////////////////

void SMEM_StackPushMarker( void )
{
    ASSERT( s_Initialized );
    s_TopMarker++;
    ASSERTS( s_TopMarker < MAX_MARKERS, "Too many scratch stack markers" );

    if( s_TopMarker >= MAX_MARKERS )
        return;

    s_Marker[ s_TopMarker ] = s_StackTop;
}

////////////////////////////////////////////////////////////////////////////

void SMEM_StackPopToMarker( void )
{
    s32 NBytes;

    ASSERT( s_Initialized );
    ASSERTS( s_TopMarker > 0, "No marker to pop" );

    if( s_TopMarker <= 0 )
        return;

#if defined(SMEM_ADD_LINING)
    u32* pLining = (u32*)(s_StackTop-16);
    if( pLining[0] != 0xDEADBEEF ||
        pLining[1] != 0xDEADBEEF ||
        pLining[2] != 0xDEADBEEF ||
        pLining[3] != 0xDEADBEEF )
    {
        x_printf( "SMEM Lining Broke!\n" );
    }
#endif

    NBytes = s_Marker[ s_TopMarker ] - s_StackTop;

    s_StackTop = s_Marker[ s_TopMarker ];
    s_TopMarker--;

    PSTAT_AddValue( StatSMemStackUsed, -NBytes );
    PSTAT_AddValue( StatSMemAvailable,  NBytes );
}

////////////////////////////////////////////////////////////////////////////

byte* SMEM_BufferAlloc( s32 NBytes )
{
    ASSERT( s_Initialized );
    ASSERT( NBytes > 0 );

    if( NBytes <= 0 )
        return NULL;

    NBytes = ALIGN_8( NBytes );

    s_BufferPoint -= NBytes;
    ASSERTS( s_BufferPoint > s_StackTop, "Out of scratch memory" );

    if( s_BufferPoint <= s_StackTop )
    {
        s_BufferPoint += NBytes;
        return NULL;
    }

    PSTAT_AddValue( StatSMemBufferUsed, NBytes );
    PSTAT_AddValue( StatSMemAvailable, -NBytes );

    return( s_BufferPoint );
}

////////////////////////////////////////////////////////////////////////////

void SMEM_ClearStats( void )
{
    PSTAT_ClearMinMax( StatSMemAvailable  );
    PSTAT_ClearMinMax( StatSMemStackUsed  );
    PSTAT_ClearMinMax( StatSMemBufferUsed );
}

////////////////////////////////////////////////////////////////////////////

void SMEM_SetSize( s32 NBytes )
{
    ASSERT( s_Initialized );
    ASSERT( NBytes >= 0 );

    NBytes = ALIGN_8( NBytes );
    s_DesiredStorageSize = NBytes;
}

////////////////////////////////////////////////////////////////////////////

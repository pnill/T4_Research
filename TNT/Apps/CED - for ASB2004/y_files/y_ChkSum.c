
#include "x_debug.h"
#include "x_plus.h"

#include "y_ChkSum.h"


///////////////////////////////////////////////////////////////
// MODULE OPTIONS
///////////////////////////////////////////////////////////////

#if 0

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(x) if(!(x)) __asm__("break 0x402");

#ifdef ASSERTS
#undef ASSERTS
#endif
#define ASSERTS(A,B) ASSERT( (A) && (B) )

#endif

///////////////////////////////////////////////////////////////

#define M_ASSERT(A) ASSERT(A)


///////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// MGLOBALS
///////////////////////////////////////////////////////////////

xbool g_CheckSumActive = TRUE;

///////////////////////////////////////////////////////////////
// LOCAL TYPES
///////////////////////////////////////////////////////////////

typedef struct
{
    u32 StartAdress;
    s32 Length;
    u32 Sum;

} chksumnode;


///////////////////////////////////////////////////////////////
// LOCAL VARIABLES
///////////////////////////////////////////////////////////////

chksumnode m_ChkNode[ CHECK_SUM_MAX_NODES ];
static s32        m_ModuleInit = 0;

///////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES
///////////////////////////////////////////////////////////////

static u32 FindSum( u32 Adress, s32 Length );
static chksumnode* FindNode( void* pAdress );

//=============================================================
// CHKSUM_InitModule
//=============================================================
void CHKSUM_InitModule( void )
{
    //--------------------------------------------
    // Init the module
    //--------------------------------------------
    ASSERT( m_ModuleInit == 0 );
    m_ModuleInit++;

    //--------------------------------------------
    // Init variables
    //--------------------------------------------
    x_memset( m_ChkNode, 0, sizeof(chksumnode) * CHECK_SUM_MAX_NODES );
}


//=============================================================
// CHKSUM_KillModule
//=============================================================
void CHKSUM_KillModule( void )
{
    ASSERT( m_ModuleInit );
    m_ModuleInit--;
}

//=============================================================
// FindSum
//=============================================================
static
u32 FindSum( u32 Adress, s32 Length )
{
    u32     Sum;
    u32     EndAddress;

    Sum         = 0;
    EndAddress  = Adress + Length;

    //-----------------------------------------------
    // Handle unaling bytes and short arrays
    // NOP | SUM3 | SUM2 | SUM1
    //-----------------------------------------------
    while ( Adress & 3 )
    {
        Sum ^= *((byte*)Adress);
        Sum <<= 8;
        Adress++;

        //-------------------------------------
        // Shortcut for Reguions of less than 4 bytes
        //-------------------------------------
        if ( Adress == EndAddress )
            return Sum;
    }

    //-----------------------------------------------
    // Deals with blocks of 4 byte aling adresses
    // SUM1 | SUM1 | SUM1 | SUM1
    //-----------------------------------------------
    EndAddress -= 4;
    while ( Adress <= EndAddress )
    {
        Sum ^= *((u32*)Adress);
        Adress += 4;
    }
    EndAddress += 4;

    //-----------------------------------------------
    // Finish the missaling part of the block if any
    // SUM1 | SUM2 | SUM3 | NOP
    //-----------------------------------------------
    M_ASSERT( (EndAddress - Adress) < 4 );
    while ( Adress < EndAddress )
    {
        Sum ^= ((u32)(*((byte*)Adress)) << ((EndAddress - Adress ) * 8));
        Adress++;
    }

    //Make sure that all happen acourding to plan
    M_ASSERT( Adress == EndAddress );

    return Sum;
}

//=============================================================
// FindNode
//=============================================================
static
chksumnode* FindNode( void* pAdress )
{
    s32 i;
    u32 Adress = (u32)pAdress;

    for ( i = 0 ; i < CHECK_SUM_MAX_NODES; i++ )
    {
        if ( m_ChkNode[i].Length )
        {
            if ( m_ChkNode[i].StartAdress == Adress )
            {
                return &m_ChkNode[i];
            }
        }
    }

    return NULL;
}

//=============================================================
// CHKSUM_AddRegion
//=============================================================
err CHKSUM_AddRegion( void* pAdress, s32 Length )
{
    s32 i;
    u32 Adress = (u32)pAdress;

    ASSERT( m_ModuleInit );
    ASSERT( Length > 0 );

    //--------------------------------------------
    // Make sure that there is not a duplicate
    //--------------------------------------------
    if ( FindNode( pAdress ) )
    {
        ASSERTS( 0, "CHKSUM_SetRegion: We already have that address in");
        return ERR_FAILURE;
    }

    //--------------------------------------------
    // Find an empty node
    //--------------------------------------------
    for ( i = 0 ; i < CHECK_SUM_MAX_NODES; i++ )
    {
        if ( m_ChkNode[i].Length == 0 )
            break;
    }

    // Did we found an empty node?
    if ( i == CHECK_SUM_MAX_NODES )
    {
        ASSERTS( 0, "CHKSUM_SetRegion: We don't have anymore empty nodes" );
        return ERR_FAILURE;
    }

    //--------------------------------------------
    // Fill the variables
    //--------------------------------------------
    m_ChkNode[i].StartAdress   = Adress;
    m_ChkNode[i].Length        = Length;
    m_ChkNode[i].Sum           = FindSum( Adress, Length );

    return ERR_SUCCESS;
}

//=============================================================
// CHKSUM_DelRegion
//=============================================================
err CHKSUM_DelRegion( void* pAdress )
{
    chksumnode* Node;

    ASSERT( m_ModuleInit );

    //-----------------------------------------
    // Find the node
    //-----------------------------------------
    Node = FindNode( pAdress );

    ASSERTS( Node, "CHKSUM_DelRegion: Unable to find the region to delete" );

    //-----------------------------------------
    // Delete the node
    //-----------------------------------------
    if ( Node )
    {
        Node->Length = 0;
        return ERR_SUCCESS;
    }

    return ERR_FAILURE;
}


//=============================================================
// CHKSUM_CheckAllRegions
//=============================================================
err CHKSUM_CheckAllRegions( void )
{
    s32 i;

    //--------------------------------------------
    // This allows the check sum to be deactivated from the N64 debugger
    //--------------------------------------------
    if ( !g_CheckSumActive )
        return ERR_SUCCESS;

    ASSERT( m_ModuleInit );

    //--------------------------------------------
    // Check all our nodes
    //--------------------------------------------
    for ( i = 0 ; i < CHECK_SUM_MAX_NODES; i++ )
    {
        if ( m_ChkNode[i].Length != 0 )
        {
            if ( FindSum( m_ChkNode[i].StartAdress, m_ChkNode[i].Length ) != m_ChkNode[i].Sum )
            {
                ASSERT( 0 );
                return ERR_FAILURE;
            }
        }
    }

    return ERR_SUCCESS;
}

//=============================================================
// CHKSUM_UpdateRegion
//=============================================================
err CHKSUM_UpdateRegion( void* pAdress )
{
    chksumnode* Node;

    ASSERT( m_ModuleInit );

    //-----------------------------------------
    // Find the node
    //-----------------------------------------
    Node = FindNode( pAdress );
    ASSERTS( Node, "CHKSUM_UpdateRegion: Unable to find the node");

    //--------------------------------------------
    // Update Node
    //--------------------------------------------
    if ( Node )
    {
        Node->Sum = FindSum( Node->StartAdress, Node->Length );
        return ERR_SUCCESS;
    }

    return ERR_FAILURE;
}


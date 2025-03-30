////////////////////////////////////////////////////////////////////////////
//
// GC_TriStrip.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"

#include "GC_TriStrip.h"


////////////////////////////////////////////////////////////////////////////
// Triangle stripping algorithm:
//
// 0) Initially connect all the given polys into one big list
//
// 1) While there's polys left in start list:
//     a) Get a list of matrix IDs the list uses(sorted from low to high)
//     b) Grab maximum # of IDs(unless there isn't that many) and make a group
//     c) loop through polys and add them if all the verts' mtxIDs are in group
//
// 2) For each group:
//     a) For each remaining triangle in group list:
//         1) Attempt to create a strip with each edge
//         2) remember which strip was the longest
//     b) Choose the triangle with the longest strip
//     c) Create a triangle strip starting with that triangle, and remove the
//        "stripped" triangles from the group's list
//     d) If triangles are left in groups list, goto step 2a
//
// 3) Copy groups and each group's triangle strips into format user will need
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// DEFINES AND CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define MAX_MTX_IN_GROUP            16
#define MAX_MTX_COUNT_IN_TRI_LIST   1024

#define MAX_VERTS_IN_STRIP          150
#define MAX_TRIS_IN_STRIP           (MAX_VERTS_IN_STRIP - 2)

#define POINTS_FOR_TRIS             5
#define POINTS_FOR_NO_EDGE          10


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct GCTempStrip
{
    u16          NVerts;
    u16          Verts[MAX_VERTS_IN_STRIP];
    GC_StripTri* Tris[MAX_TRIS_IN_STRIP];
    s32          Score;
    GCTempStrip* pNextStrip;
};


struct GCTempStripGroup
{
    s32     NMatrices;
    s32     MatrixIDs[MAX_MTX_IN_GROUP];

    GC_StripTri* pTriList;
    GCTempStrip* pStripList;

    GCTempStripGroup* pNextGroup;
};


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

static s32 GetUnusedTriCount( GC_StripTri* pTriList )
{
    s32 Count = 0;

    while( pTriList != NULL )
    {
        if( pTriList->bAddedToStrip == FALSE )
            Count++;

        pTriList = pTriList->pNextTri;
    }

    return Count;
}

//==========================================================================

static xbool FindSharedEdge( GC_StripTri* pTriList, u16 Vert0, u16 Vert1, GC_StripTri** pSharedTri = NULL )
{
    GC_StripTri* pCurTri = pTriList;

    while( pCurTri != NULL )
    {
        if( (pCurTri->bAddedToStrip == FALSE) &&
            (pCurTri->bInCurStrip   == FALSE) )
        {
            if( (pCurTri->Verts[0] == Vert0) ||
                (pCurTri->Verts[1] == Vert0) ||
                (pCurTri->Verts[2] == Vert0) )
            {
                if( (pCurTri->Verts[0] == Vert1) ||
                    (pCurTri->Verts[1] == Vert1) ||
                    (pCurTri->Verts[2] == Vert1) )
                {
                    if( pSharedTri != NULL )
                        *pSharedTri = pCurTri;

                    return TRUE;
                }
            }
        }

        pCurTri = pCurTri->pNextTri;
    }

    if( pSharedTri != NULL )
        *pSharedTri = NULL;

    return FALSE;
}

//==========================================================================

static void CompleteStrip( GC_StripTri* pTriList, GCTempStrip& rStrip, s32 MaxVertsInStrip )
{
    GC_StripTri* pCurTri;
    GC_StripTri* pSharedTri;
    s32 Vert0, Vert1;
    s32 NextVert;

    //--- Mark all tris as not being in this strip
    for( pCurTri = pTriList; pCurTri != NULL; pCurTri = pCurTri->pNextTri )
        pCurTri->bInCurStrip = FALSE;

    //--- Set the first tri (which has already been filled) in this strip
    rStrip.Tris[0]->bInCurStrip = TRUE;

    //--- Now go through the remaining tris, adding them to the strip
    while( TRUE )
    {
        //--- We know the first two verts, find a triangle that uses them
        Vert0 = rStrip.Verts[ rStrip.NVerts - 2 ];
        Vert1 = rStrip.Verts[ rStrip.NVerts - 1 ];

        if( FindSharedEdge( pTriList, Vert0, Vert1, &pSharedTri ) )
        {
            NextVert = -1;
            
            //--- find the next vert to add
            if( pSharedTri->Verts[0] != Vert0 &&
                pSharedTri->Verts[0] != Vert1 )
            {
                NextVert = pSharedTri->Verts[0];
            }
            if( pSharedTri->Verts[1] != Vert0 &&
                pSharedTri->Verts[1] != Vert1 )
            {
                NextVert = pSharedTri->Verts[1];
            }
            if( pSharedTri->Verts[2] != Vert0 &&
                pSharedTri->Verts[2] != Vert1 )
            {
                NextVert = pSharedTri->Verts[2];
            }

            //--- this will assert if there are degenerate or duplicate tris
            ASSERT( NextVert != -1 );

            //--- add the vert to the strip
            rStrip.Verts[ rStrip.NVerts ]  = NextVert;
            rStrip.Tris[ rStrip.NVerts-2 ] = pSharedTri;
            rStrip.NVerts++;

            //--- mark this triangle as used
            pSharedTri->bInCurStrip = TRUE;

            //--- also add to its score
            rStrip.Score += POINTS_FOR_TRIS;
            if( !FindSharedEdge( pTriList, rStrip.Verts[rStrip.NVerts - 1], rStrip.Verts[rStrip.NVerts - 3] ) )
                rStrip.Score += POINTS_FOR_NO_EDGE;
        }

        //--- could we find another triangle?
        if( pSharedTri == NULL )
            break;

        //--- have we maxed out our list?
        if( rStrip.NVerts >= MaxVertsInStrip )
            break;
        if( rStrip.NVerts >= MAX_VERTS_IN_STRIP )
            break;
    }
}

//==========================================================================

static void FindBestStrip( GC_StripTri* pTriList, GCTempStrip& rStrip, s32 MaxVertsInStrip )
{
    s32          i;
    GCTempStrip  Strip1, Strip2, Strip3;
    GCTempStrip* pBest;
    GC_StripTri* pCurTri;

    rStrip.NVerts = 0;
    rStrip.Score  = -1;

    pCurTri = pTriList;

    //--- go through all possibilities of strips, finding the longest one
    while( pCurTri != NULL )
    {
        if( pCurTri->bAddedToStrip == FALSE )
        {
            //=== Build three strips for each triangle, starting with different verts

            //--- strip one, starts with first vert
            Strip1.NVerts   = 3;
            Strip1.Verts[0] = pCurTri->Verts[0];
            Strip1.Verts[1] = pCurTri->Verts[1];
            Strip1.Verts[2] = pCurTri->Verts[2];
            Strip1.Tris[0]  = pCurTri;
            Strip1.Score    = 0;
            CompleteStrip( pTriList, Strip1, MaxVertsInStrip );

            //--- strip two, starts with second vert
            Strip2.NVerts   = 3;
            Strip2.Verts[0] = pCurTri->Verts[1];
            Strip2.Verts[1] = pCurTri->Verts[2];
            Strip2.Verts[2] = pCurTri->Verts[0];
            Strip2.Tris[0]  = pCurTri;
            Strip2.Score    = 0;
            CompleteStrip( pTriList, Strip2, MaxVertsInStrip );

            //--- strip three, starts with third vert
            Strip3.NVerts   = 3;
            Strip3.Verts[0] = pCurTri->Verts[2];
            Strip3.Verts[1] = pCurTri->Verts[0];
            Strip3.Verts[2] = pCurTri->Verts[1];
            Strip3.Tris[0]  = pCurTri;
            Strip3.Score    = 0;
            CompleteStrip( pTriList, Strip3, MaxVertsInStrip );

            //--- which of the 3 strips is the longest?
            pBest = &Strip1;
            if( Strip2.Score > pBest->Score )
                pBest = &Strip2;
            if( Strip3.Score > pBest->Score )
                pBest = &Strip3;

            //--- is it better than our current strip?
            if( pBest->Score > rStrip.Score )
            {
                //--- this is now the best strip
                rStrip.Score  = pBest->Score;
                rStrip.NVerts = pBest->NVerts;

                for( i = 0; i < rStrip.NVerts; i++ )
                    rStrip.Verts[i] = pBest->Verts[i];

                for( i = 0; i < rStrip.NVerts - 2; i++ )
                    rStrip.Tris[i] = pBest->Tris[i];
            }
        }

        pCurTri = pCurTri->pNextTri;
    }

    ASSERT( rStrip.NVerts > 0 );
}

//==========================================================================

s32 CountGroupsInLinkedList( GCTempStripGroup* pGroupList )
{
    s32               Count = 0;
    GCTempStripGroup* pCurr = pGroupList;

    while( pCurr != NULL )
    {
        Count++;
        pCurr = pCurr->pNextGroup;
    }

    return Count;
}

//==========================================================================

s32 CountStripsInLinkedList( GCTempStrip* pList )
{
    s32          Count = 0;
    GCTempStrip* pCurr = pList;

    while( pCurr != NULL )
    {
        Count++;
        pCurr = pCurr->pNextStrip;
    }

    return Count;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static void ConnectTris( GC_StripTri* pTris, s32 NTris )
{
    s32 i;

    GC_StripTri* pLastTri = NULL;

    for( i = NTris-1; i >= 0; i-- )
    {
        pTris[i].bAddedToStrip = FALSE;
        pTris[i].pNextTri      = pLastTri;

        pLastTri = &pTris[i];
    }
}

//==========================================================================

static void GetMatrixIDs( GC_StripTri* pTriList, s32& rNMatrices, u16* pMatrixIDs )
{
    s32   i;
    s32   TriMtxIdx;
    xbool MtxIDFound;

    rNMatrices = 0;

    while( pTriList != NULL )
    {
        //--- test each matrix ID of the tri against all current matrix IDs
        for( TriMtxIdx = 0; TriMtxIdx < 3; TriMtxIdx++ )
        {
            MtxIDFound = FALSE;

            for( i = 0; i < rNMatrices; i++ )
            {
                if( pMatrixIDs[i] == pTriList->MtxID[TriMtxIdx] )
                {
                    MtxIDFound = TRUE;
                    break;
                }
            }

            //--- current tri matrix ID wasn't in list, add it and update count
            if( !MtxIDFound )
            {
                pMatrixIDs[rNMatrices] = pTriList->MtxID[TriMtxIdx];
                rNMatrices++;
            }

            if( rNMatrices >= MAX_MTX_COUNT_IN_TRI_LIST )
                break;
        }

        if( rNMatrices >= MAX_MTX_COUNT_IN_TRI_LIST )
            break;

        //--- move on to next tri
        pTriList = pTriList->pNextTri;
    }


    //--- Do a sort to order the matrices
    if( rNMatrices <= 1 )
        return;

    xbool SortDone = FALSE;
    while( !SortDone )
    {
        SortDone = TRUE;
        for( i = 0; i < rNMatrices-1; i++ )
        {
            if( pMatrixIDs[i+1] < pMatrixIDs[i] )
            {
                u16 Temp        = pMatrixIDs[i];
                pMatrixIDs[i]   = pMatrixIDs[i+1];
                pMatrixIDs[i+1] = Temp;
                SortDone = FALSE;
            }
        }
    }
}

//==========================================================================

static xbool AllTriMatricesInList( GC_StripTri* pTri, GCTempStripGroup* pGroup )
{
    s32 i, j;
    s32 NMtxInList = 0;

    for( j = 0; j < 3; j++ )
    {
        for( i = 0; i < pGroup->NMatrices; i++ )
        {
            if( pTri->MtxID[j] == pGroup->MatrixIDs[i] )
            {
                NMtxInList++;
                break;
            }
        }
    }

    if( NMtxInList != 3 )
        return FALSE;

    return TRUE;
}

//==========================================================================

static GCTempStripGroup* BuildGroupList( GC_StripTri* pTris, s32 MaxMatricesInGroup )
{
    s32         i;
    s32         NMatrices;
    u16         MatrixIDs[MAX_MTX_COUNT_IN_TRI_LIST];
    GC_StripTri SentinelTri;

    GCTempStripGroup* pGroupList = NULL;

    SentinelTri.pNextTri = pTris;

    //--- loop until all the tris have been put into groups
    while( SentinelTri.pNextTri != NULL )
    {
        //--- get all the matrix IDs used in the current remaining tris
        GetMatrixIDs( SentinelTri.pNextTri, NMatrices, MatrixIDs );

        //--- create a new group and insert it into our current group list
        GCTempStripGroup* pNewGroup = (GCTempStripGroup*)x_malloc( sizeof(GCTempStripGroup) );
        ASSERT( pNewGroup != NULL );

        pNewGroup->NMatrices  = MIN(NMatrices, MaxMatricesInGroup);
        pNewGroup->pStripList = NULL;
        pNewGroup->pTriList   = NULL;
        pNewGroup->pNextGroup = pGroupList;

        pGroupList = pNewGroup;

        //--- copy the maximum number of matrix IDs to group
        for( i = 0; i < pNewGroup->NMatrices; i++ )
        {
            pNewGroup->MatrixIDs[i] = MatrixIDs[i];
        }

        //--- add tris with all 3 of its matrices in the current group to the groups tri list
        GC_StripTri* pPrevTri = &SentinelTri;
        GC_StripTri* pCurTri  = SentinelTri.pNextTri;
        while( pCurTri != NULL )
        {
            if( AllTriMatricesInList( pCurTri, pNewGroup ) )
            {
                pPrevTri->pNextTri  = pCurTri->pNextTri;
                pCurTri->pNextTri   = pNewGroup->pTriList;
                pNewGroup->pTriList = pCurTri;

                pCurTri = pPrevTri->pNextTri;
            }
            else
            {
                pPrevTri = pCurTri;
                pCurTri  = pCurTri->pNextTri;
            }
        }
    }

    return pGroupList;
}

//==========================================================================

static BuildGroupStrips( GCTempStripGroup* pGroup, s32 MaxVertsInStrip )
{
    s32          i;
    GCTempStrip  BestStrip;
    GCTempStrip* pStrip;

    pGroup->pStripList = NULL;

    //--- now build the strips
    while( GetUnusedTriCount( pGroup->pTriList ) )
    {
        ASSERT( x_MemSanityCheck() == 0 );

        //--- get the best strip possible (using our hybrid-greedy algorithm)
        FindBestStrip( pGroup->pTriList, BestStrip, MaxVertsInStrip );
        ASSERT( BestStrip.NVerts > 0 );

        //--- add the longest strip to the global strip list
        pStrip = (GCTempStrip*)x_malloc( sizeof(GCTempStrip) );
        ASSERT( pStrip != NULL );
        x_memcpy( pStrip, &BestStrip, sizeof(GCTempStrip) );

        pStrip->pNextStrip = pGroup->pStripList;
        pGroup->pStripList = pStrip;

        //--- go through the strip, marking its triangles as used
        for( i = 0; i < pStrip->NVerts - 2; i++ )
            pStrip->Tris[i]->bAddedToStrip = TRUE;
    }
}

//==========================================================================

void GCTRISTRIP_Build( GC_TStripGroup** pStripGroups,
                       s32&             rNGroups,
                       GC_StripTri*     pTris,
                       s32              NTris,
                       s32              MaxVertsInStrip,
                       s32              MaxMatricesInGroup )
{
    s32 i, S, V;
    GCTempStripGroup* pGroupList;
    GCTempStripGroup* pCurGroup;

    ASSERT( pStripGroups != NULL );
    ASSERT( pTris != NULL );

    //--- connect all the tris into one linked list
    ConnectTris( pTris, NTris );

    //--- build a linked list of groups(each group having tris for stripping)
    pGroupList = BuildGroupList( pTris, MaxMatricesInGroup );

    //--- build strips for each group
    pCurGroup = pGroupList;
    while( pCurGroup != NULL )
    {
        BuildGroupStrips( pCurGroup, MaxVertsInStrip );
        pCurGroup = pCurGroup->pNextGroup;
    }


    GC_TStripGroup* pGroupArray;
    GC_TStrip*      pStripArray;
    GCTempStrip*    pCurStrip;


    //--- Now that we have our data filled out, copy it to a format that will be returned
    //    to the caller.  Each group in our linked list will become an element in the final
    //    group array, and each strip in each group's strip list will become an element
    //    in a strip array(sounds more confusing than it is).
    //    Make sure to release memory used by linked list data.

    //--- get group count, and allocate a new array for them
    rNGroups = CountGroupsInLinkedList( pGroupList );

    pGroupArray = (GC_TStripGroup*)x_malloc( sizeof(GC_TStripGroup) * rNGroups );
    ASSERT( pGroupArray != NULL );
    *pStripGroups = pGroupArray;

    //--- loop through each group and copy data
    for( i = 0; i < rNGroups; i++ )
    {
        pCurGroup = pGroupList;

        //--- create matrix index array for group, and copy them to destination array
        pGroupArray[i].NMatrices = pCurGroup->NMatrices;
        pGroupArray[i].pMtxIDs = (u16*)x_malloc( sizeof(u16) * pCurGroup->NMatrices );
        ASSERT( pGroupArray[i].pMtxIDs != NULL );

        for( S = 0; S < pCurGroup->NMatrices; S++ )
            pGroupArray[i].pMtxIDs[S] = pCurGroup->MatrixIDs[S];

        //--- get number of triangle strips and allocate an array for them
        pGroupArray[i].NStrips = CountStripsInLinkedList( pCurGroup->pStripList );

        pStripArray = (GC_TStrip*)x_malloc( sizeof(GC_TStrip) * pGroupArray[i].NStrips );
        ASSERT( pStripArray != NULL );
        pGroupArray[i].pStrips = pStripArray;

        //--- loop through each strip and copy strip data
        for( S = 0; S < pGroupArray[i].NStrips; S++ )
        {
            pCurStrip = pCurGroup->pStripList;

            //--- create array of vertex indices and copy them to dest array
            pStripArray[S].NVerts = pCurStrip->NVerts;
            pStripArray[S].pVerts = (u16*)x_malloc( sizeof(u16) * pCurStrip->NVerts );
            ASSERT( pStripArray[S].pVerts != NULL );

            for( V = 0; V < pStripArray[S].NVerts; V++ )
                pStripArray[S].pVerts[V] = pCurStrip->Verts[V];

            //--- move to next strip, releasing the memory used by strip
            pCurGroup->pStripList = pCurStrip->pNextStrip;
            x_free( pCurStrip );
        }

        //--- move to next group, releasing memory used by group
        pGroupList = pCurGroup->pNextGroup;
        x_free( pCurGroup );
    }

}

//==========================================================================

void GCTRISTRIP_Free( GC_TStripGroup** ppTStripGroup, s32 NGroups )
{
    s32 i, j;

    ASSERT( ppTStripGroup != NULL );
    ASSERT( *ppTStripGroup != NULL );

    GC_TStripGroup* pGroups = *ppTStripGroup;

    for( i = 0; i < NGroups; i++ )
    {
        for( j = 0; j < pGroups[i].NStrips; j++ )
            x_free( pGroups[i].pStrips[j].pVerts );

        x_free( pGroups[i].pStrips );
        x_free( pGroups[i].pMtxIDs );
    }

    x_free( pGroups );

    *ppTStripGroup = NULL;
}

//==========================================================================

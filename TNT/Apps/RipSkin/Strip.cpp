////////////////////////////////////////////////////////////////////////////
//
// Strip.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"

#include "Strip.h"


////////////////////////////////////////////////////////////////////////////
// Building triangle strips - a description of the algorithm:
//
// 1) Create a list of the triangles which will be easier to work with.
// 2) for each remaining triangle:
//    a) attempt to create a strip with each edge
//    b) remember which strip was the longest
// 3) choose the triangle with the longest strip
// 4) create a triangle strip starting with that triangle, and remove the
//    "stripped" triangles from the global list
// 5) if triangles are left, goto step 2
// 6) Copy the triangle strips into a more useful form
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////

#define MAX_VERTS_IN_STRIP      150
#define MAX_TRIS_IN_STRIP       (MAX_VERTS_IN_STRIP - 2)
#define POINTS_FOR_TRIS         5
#define POINTS_FOR_NO_EDGE      10

////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////

typedef struct STempStrip
{
    s32         NVerts;
    s32         Verts[MAX_VERTS_IN_STRIP];
    s32         Tris[MAX_TRIS_IN_STRIP];
    s32         Score;      // concocted value to help determine the best strip
    STempStrip* pNextStrip;
} t_TempStrip;

typedef struct STempTri
{
    s32     Verts[3];
    xbool   InGlobalList; // TRUE if it has been added to a strip already
    xbool   InCurStrip;   // temp variable used by greedy algorithm
} t_TempTri;

////////////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////////////

static s32          s_NGlobalTris = 0;
static t_TempTri*   s_GlobalTriList = NULL;
static t_TempStrip* s_GlobalStripList = NULL;

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

static
s32 GetUnusedTriCount( void )
{
    s32 i;
    s32 count = 0;
    for ( i = 0; i < s_NGlobalTris; i++ )
    {
        if ( s_GlobalTriList[i].InGlobalList == FALSE )
            count++;
    }
    return count;
}

//==========================================================================

static
xbool IsSharedEdge( s32 Vert0, s32 Vert1 )
{
    s32 i;

    for ( i = 0; i < s_NGlobalTris; i++ )
    {
        if ( (s_GlobalTriList[i].InGlobalList == FALSE) &&
             (s_GlobalTriList[i].InCurStrip == FALSE) )
        {
            if ( ((s_GlobalTriList[i].Verts[0] == Vert0) ||
                  (s_GlobalTriList[i].Verts[1] == Vert0) ||
                  (s_GlobalTriList[i].Verts[2] == Vert0)) &&
                 ((s_GlobalTriList[i].Verts[0] == Vert1) ||
                  (s_GlobalTriList[i].Verts[1] == Vert1) ||
                  (s_GlobalTriList[i].Verts[2] == Vert1)) )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//==========================================================================

static
void CompleteStrip( t_TempStrip& rStrip, s32 MaxVertsInStrip )
{
    s32     i;
    xbool   done = FALSE;
    
    //---   mark all tris as not being in this strip
    for ( i = 0; i < s_NGlobalTris; i++ )
        s_GlobalTriList[i].InCurStrip = FALSE;

    //---   now, the first tri (which has already been filled) is in this strip
    s_GlobalTriList[rStrip.Tris[0]].InCurStrip = TRUE;

    //---   now go through the remaining tris, adding them to the strip
    while ( !done )
    {
        s32 Vert0, Vert1;

        //---   we know the first two verts, find a triangle that uses them
        Vert0 = rStrip.Verts[rStrip.NVerts - 2];
        Vert1 = rStrip.Verts[rStrip.NVerts - 1];
        for ( i = 0; i < s_NGlobalTris; i++ )
        {
            if ( (s_GlobalTriList[i].InGlobalList == FALSE) &&
                 (s_GlobalTriList[i].InCurStrip == FALSE) )
            {
                if ( ((s_GlobalTriList[i].Verts[0] == Vert0) ||
                      (s_GlobalTriList[i].Verts[1] == Vert0) ||
                      (s_GlobalTriList[i].Verts[2] == Vert0)) &&
                     ((s_GlobalTriList[i].Verts[0] == Vert1) ||
                      (s_GlobalTriList[i].Verts[1] == Vert1) ||
                      (s_GlobalTriList[i].Verts[2] == Vert1)) )
                {
                    s32 NextVert = -1;
                    
                    //===   woohoo, we have a shared edge

                    //---   find the next vert to add
                    if ( s_GlobalTriList[i].Verts[0] != Vert0 &&
                         s_GlobalTriList[i].Verts[0] != Vert1 )
                    {
                        NextVert = s_GlobalTriList[i].Verts[0];
                    }
                    if ( s_GlobalTriList[i].Verts[1] != Vert0 &&
                         s_GlobalTriList[i].Verts[1] != Vert1 )
                    {
                        NextVert = s_GlobalTriList[i].Verts[1];
                    }
                    if ( s_GlobalTriList[i].Verts[2] != Vert0 &&
                         s_GlobalTriList[i].Verts[2] != Vert1 )
                    {
                        NextVert = s_GlobalTriList[i].Verts[2];
                    }
                    ASSERT( NextVert != -1 );

                    //---   add the vert to the strip
                    rStrip.Verts[rStrip.NVerts] = NextVert;
                    rStrip.Tris[rStrip.NVerts-2] = i;
                    rStrip.NVerts++;

                    //---   mark this triangle as used
                    s_GlobalTriList[i].InCurStrip = TRUE;

                    //---   also add to its score
                    rStrip.Score += POINTS_FOR_TRIS;
                    if ( !IsSharedEdge( rStrip.Verts[rStrip.NVerts - 1], rStrip.Verts[rStrip.NVerts - 3] ) )
                        rStrip.Score += POINTS_FOR_NO_EDGE;

                    //---   break out of the loop
                    break;
                }
            }
        }

        //---   could we find another triangle?
        if ( i == s_NGlobalTris )
            done = TRUE;
        
        //---   have we maxed out our list?
        if ( rStrip.NVerts == MaxVertsInStrip )
            done = TRUE;

        if ( rStrip.NVerts == MAX_VERTS_IN_STRIP )
            done = TRUE;
    }
}

//==========================================================================

static
void FindBestStrip( t_TempStrip& rStrip, s32 MaxVertsInStrip )
{
    s32 i, j;

    rStrip.NVerts = 0;
    rStrip.Score = -1;

    //---   go through all possibilities of strips, finding the longest one
    for ( i = 0; i < s_NGlobalTris; i++ )
    {
        if ( s_GlobalTriList[i].InGlobalList == FALSE )
        {
            t_TempStrip     Strip1, Strip2, Strip3;
            t_TempStrip*    pBest;

            //===   Build three strips for each triangle, starting with different verts
            
            //---   strip one, starts with first vert
            Strip1.NVerts = 3;
            Strip1.Verts[0] = s_GlobalTriList[i].Verts[0];
            Strip1.Verts[1] = s_GlobalTriList[i].Verts[1];
            Strip1.Verts[2] = s_GlobalTriList[i].Verts[2];
            Strip1.Tris[0] = i;
            Strip1.Score = 0;
            CompleteStrip( Strip1, MaxVertsInStrip );
            
            //---   strip two, starts with second vert
            Strip2.NVerts = 3;
            Strip2.Verts[0] = s_GlobalTriList[i].Verts[1];
            Strip2.Verts[1] = s_GlobalTriList[i].Verts[2];
            Strip2.Verts[2] = s_GlobalTriList[i].Verts[0];
            Strip2.Tris[0] = i;
            Strip2.Score = 0;
            CompleteStrip( Strip2, MaxVertsInStrip );

            //---   strip three, starts with third vert
            Strip3.NVerts = 3;
            Strip3.Verts[0] = s_GlobalTriList[i].Verts[2];
            Strip3.Verts[1] = s_GlobalTriList[i].Verts[0];
            Strip3.Verts[2] = s_GlobalTriList[i].Verts[1];
            Strip3.Tris[0] = i;
            Strip3.Score = 0;
            CompleteStrip( Strip3, MaxVertsInStrip );

            //---   which of the 3 strips is the longest?
            pBest = &Strip1;
            if ( Strip2.Score > pBest->Score ) pBest = &Strip2;
            if ( Strip3.Score > pBest->Score ) pBest = &Strip3;

            //---   well, is it better than our current strip?
            if ( pBest->Score > rStrip.Score )
            {
                //---   this is now the best strip
                rStrip.Score = pBest->Score;
                rStrip.NVerts = pBest->NVerts;
                for ( j = 0; j < rStrip.NVerts; j++ )
                    rStrip.Verts[j] = pBest->Verts[j];
                for ( j = 0; j < rStrip.NVerts - 2; j++ )
                    rStrip.Tris[j] = pBest->Tris[j];
            }
        }
    }

    ASSERT( rStrip.NVerts > 0 );
}

//==========================================================================

s32 CountStripsInLinkedList( t_TempStrip* pList )
{
    s32             count = 0;
    t_TempStrip*    pCurr = pList;

    while ( pCurr )
    {
        count++;
        pCurr = pCurr->pNextStrip;
    }
    return count;
}

//==========================================================================

void BuildStrips( t_Triangle* pTris, s32 NTris, t_TriangleStrip** pStrips, s32& NStrips, s32 MaxVertsInStrip )
{
    s32                 i, j;
    t_TriangleStrip*    StripArray;

    s_GlobalStripList = NULL;

    //---   create the list of triangles to work from
    s_GlobalTriList = (t_TempTri*)x_malloc( NTris * sizeof(t_TempTri) );
    ASSERT( s_GlobalTriList );
    s_NGlobalTris = NTris;

    //---   copy the user's triangles into our list
    for ( i = 0; i < NTris; i++ )
    {
        s_GlobalTriList[i].Verts[0] = pTris[i].Index[0];
        s_GlobalTriList[i].Verts[1] = pTris[i].Index[1];
        s_GlobalTriList[i].Verts[2] = pTris[i].Index[2];
        s_GlobalTriList[i].InGlobalList = FALSE;
    }

    //---   now build the strips
    while ( GetUnusedTriCount() )
    {
        t_TempStrip     BestStrip;
        t_TempStrip*    pStrip;

        ASSERT( x_MemSanityCheck() == 0 );

        //---   get the best strip possible (using our hybrid-greedy algorithm)
        FindBestStrip( BestStrip, MaxVertsInStrip );
        ASSERT( BestStrip.NVerts );

        //---   add the longest strip to the global strip list (which is a linked list)
        pStrip = (t_TempStrip*)x_malloc( sizeof(t_TempStrip) );
        ASSERT( pStrip );
        x_memcpy( pStrip, &BestStrip, sizeof(t_TempStrip) );
        pStrip->pNextStrip = s_GlobalStripList;
        s_GlobalStripList = pStrip;

        //---   go through the strip, marking its triangles as used
        for ( i = 0; i < pStrip->NVerts - 2; i++ )
        {
            s_GlobalTriList[pStrip->Tris[i]].InGlobalList = TRUE;
        }
    }

    //---   at this point, we have a linked list of triangle strips
    //      put them into a form which the user expects
    NStrips = CountStripsInLinkedList( s_GlobalStripList );
    *pStrips = (t_TriangleStrip*)x_malloc( sizeof(t_TriangleStrip) * NStrips );
    ASSERT( *pStrips );
    StripArray = *pStrips;
    for ( i = 0; i < NStrips; i++ )
    {
        t_TempStrip*    pLink;

        //---   get a ptr to the current strip in the linked list
        pLink = s_GlobalStripList;
        ASSERT( pLink );

        //---   copy that strip into the final list
        StripArray[i].NVerts = pLink->NVerts;
        StripArray[i].pVerts = (s32*)x_malloc( sizeof(s32) * pLink->NVerts );
        ASSERT( StripArray[i].pVerts );
        for ( j = 0; j < StripArray[i].NVerts; j++ )
        {
            StripArray[i].pVerts[j] = pLink->Verts[j];
        }
        
        //---   now remove this strip from the linked list
        s_GlobalStripList = pLink->pNextStrip;
        x_free( pLink );
    }

    //---   free the global list of triangles
    x_free( s_GlobalTriList );
}

#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"

#include "RipGeom.h"
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
    vector2     UVs[MAX_VERTS_IN_STRIP];
    s32         Tris[MAX_TRIS_IN_STRIP];
    s32         Score;      // concocted value to help determine the best strip
    STempStrip* pNextStrip;
} t_TempStrip;

typedef struct STempTri
{
    s32     Verts[3];
    vector2 UVs[3];
    xbool   InGlobalList; // TRUE if it has been added to a strip already
    u32     InCurStrip;   // temp variable used by greedy algorithm
} t_TempTri;

////////////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////////////

static s32          s_NGlobalTris = 0;
static t_TempTri*   s_GlobalTriList = NULL;
static t_TempStrip* s_GlobalStripList = NULL;
static u32          s_StripPass = 0;
static s32          s_LowestUsed = 0;

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
//    for ( i = 0; i < s_NGlobalTris; i++ )
//        s_GlobalTriList[i].InCurStrip = FALSE;

    s_StripPass++;

    //---   now, the first tri (which has already been filled) is in this strip
    s_GlobalTriList[rStrip.Tris[0]].InCurStrip = s_StripPass;

    //---   now go through the remaining tris, adding them to the strip
    while ( !done )
    {
        s32 Vert0, Vert1;
        vector2 UV0, UV1;

        //---   we know the first two verts, find a triangle that uses them
        Vert0 = rStrip.Verts[rStrip.NVerts - 2];
        Vert1 = rStrip.Verts[rStrip.NVerts - 1];

        UV0 = rStrip.UVs[rStrip.NVerts - 2];
        UV1 = rStrip.UVs[rStrip.NVerts - 1];

        for ( i = 0; i < s_NGlobalTris; i++ )
        {
            if ( (s_GlobalTriList[i].InGlobalList == FALSE) &&
                 (s_GlobalTriList[i].InCurStrip != s_StripPass) )
            {
                if ( ((s_GlobalTriList[i].Verts[0] == Vert0) ||
                      (s_GlobalTriList[i].Verts[1] == Vert0) ||
                      (s_GlobalTriList[i].Verts[2] == Vert0)) &&
                     ((s_GlobalTriList[i].Verts[0] == Vert1) ||
                      (s_GlobalTriList[i].Verts[1] == Vert1) ||
                      (s_GlobalTriList[i].Verts[2] == Vert1)) &&

                     ((s_GlobalTriList[i].UVs[0] == UV0) ||
                      (s_GlobalTriList[i].UVs[1] == UV0) ||
                      (s_GlobalTriList[i].UVs[2] == UV0)) &&
                     ((s_GlobalTriList[i].UVs[0] == UV1) ||
                      (s_GlobalTriList[i].UVs[1] == UV1) ||
                      (s_GlobalTriList[i].UVs[2] == UV1))
                    )
                {
                    s32 NextVert = -1;
                    vector2 NextUV;
                    
                    //===   woohoo, we have a shared edge

                    //---   find the next vert to add
                    if ( s_GlobalTriList[i].Verts[0] != Vert0 &&
                         s_GlobalTriList[i].Verts[0] != Vert1
                       )
                    {
                        NextVert = s_GlobalTriList[i].Verts[0];
                        NextUV = s_GlobalTriList[i].UVs[0];
                    }
                    if ( s_GlobalTriList[i].Verts[1] != Vert0 &&
                         s_GlobalTriList[i].Verts[1] != Vert1
                       )
                    {
                        NextVert = s_GlobalTriList[i].Verts[1];
                        NextUV = s_GlobalTriList[i].UVs[1];
                    }
                    if ( s_GlobalTriList[i].Verts[2] != Vert0 &&
                         s_GlobalTriList[i].Verts[2] != Vert1
                       )
                    {
                        NextVert = s_GlobalTriList[i].Verts[2];
                        NextUV = s_GlobalTriList[i].UVs[2];
                    }
                    ASSERT( NextVert != -1 );

                    //---   add the vert to the strip
                    rStrip.Verts[rStrip.NVerts] = NextVert;
                    rStrip.UVs[rStrip.NVerts] = NextUV;
                    rStrip.Tris[rStrip.NVerts-2] = i;
                    rStrip.NVerts++;

                    //---   mark this triangle as used
                    s_GlobalTriList[i].InCurStrip = s_StripPass;

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
    for ( i = s_LowestUsed; i < s_NGlobalTris; i++ )
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
            Strip1.UVs[0] = s_GlobalTriList[i].UVs[0];
            Strip1.UVs[1] = s_GlobalTriList[i].UVs[1];
            Strip1.UVs[2] = s_GlobalTriList[i].UVs[2];
            Strip1.Tris[0] = i;
            Strip1.Score = 0;
            CompleteStrip( Strip1, MaxVertsInStrip );
            
            //---   strip two, starts with second vert
            Strip2.NVerts = 3;
            Strip2.Verts[0] = s_GlobalTriList[i].Verts[1];
            Strip2.Verts[1] = s_GlobalTriList[i].Verts[2];
            Strip2.Verts[2] = s_GlobalTriList[i].Verts[0];
            Strip2.UVs[0] = s_GlobalTriList[i].UVs[1];
            Strip2.UVs[1] = s_GlobalTriList[i].UVs[2];
            Strip2.UVs[2] = s_GlobalTriList[i].UVs[0];
            Strip2.Tris[0] = i;
            Strip2.Score = 0;
            CompleteStrip( Strip2, MaxVertsInStrip );

            //---   strip three, starts with third vert
            Strip3.NVerts = 3;
            Strip3.Verts[0] = s_GlobalTriList[i].Verts[2];
            Strip3.Verts[1] = s_GlobalTriList[i].Verts[0];
            Strip3.Verts[2] = s_GlobalTriList[i].Verts[1];
            Strip3.UVs[0] = s_GlobalTriList[i].UVs[2];
            Strip3.UVs[1] = s_GlobalTriList[i].UVs[0];
            Strip3.UVs[2] = s_GlobalTriList[i].UVs[1];
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
//                rStrip.NVerts = MIN( pBest->NVerts, 4 );  // wtf is this here for???!?!?!?!!?!!!!
                rStrip.NVerts = MIN( pBest->NVerts, MaxVertsInStrip );
                for ( j = 0; j < rStrip.NVerts; j++ )
                {
                    rStrip.Verts[j] = pBest->Verts[j];
                    rStrip.UVs[j] = pBest->UVs[j];
                }
                for ( j = 0; j < rStrip.NVerts - 2; j++ )
                    rStrip.Tris[j] = pBest->Tris[j];
            }
            break;
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

s32 CountTrisInLinkedList( t_TempStrip* pList )
{
    s32             count = 0;
    t_TempStrip*    pCurr = pList;

    while ( pCurr )
    {
        if( pCurr->NVerts == 3 )
            count++;
        pCurr = pCurr->pNextStrip;
    }
    return count;
}

static
vector2* MakeUVs( t_TempStrip* pStrip, xbool RotateUVs )
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    //  UV Adjustments
    //    This code was copied from the original StadRip... let's hope it has a desired effect
    ///////////////////////////////////////////////////////////////////////////////////////////////
    s32 MinU;
    s32 MinV;
    s32 BreakLoopCounter = 0;

    vector2* UV = new vector2[4];

    UV[0] = pStrip->UVs[0];
    UV[1] = pStrip->UVs[1];
    UV[2] = pStrip->UVs[2];
    UV[3] = pStrip->UVs[3];

    ///////////////////////////////////////////////////////////////////
    // find minimum U
    MinU = 0;
    if (UV[1].X < UV[MinU].X) MinU = 1;
    if (UV[2].X < UV[MinU].X) MinU = 2;
    if (UV[3].X < UV[MinU].X) MinU = 3;

    ////////////////////////////////////////
    // slide U values over
    if (UV[MinU].X<=0)
    {
        BreakLoopCounter = 0;
        while ( UV[MinU].X < 0 )
        {
            UV[0].X++;
            UV[1].X++;
            UV[2].X++;
            UV[3].X++;
            BreakLoopCounter++;
            if( BreakLoopCounter == 5000 ) break;
        }
    }
    else
    {
        BreakLoopCounter = 0;
        while ( (UV[MinU].X >= 1.0f) && (UV[MinU].X+1 >= 0.0f))
        {
            UV[0].X--;
            UV[1].X--;
            UV[2].X--;
            UV[3].X--;
            BreakLoopCounter++;
            if( BreakLoopCounter == 5000 ) break;
        }
    }

    ///////////////////////////////////////////////////////////////////
    // find minimum V
    MinV = 0;
    if (UV[1].Y < UV[MinV].Y) MinV = 1;
    if (UV[2].Y < UV[MinV].Y) MinV = 2;
    if (UV[3].Y < UV[MinV].Y) MinV = 3;

    ////////////////////////////////////////
    // slide V values over
    if (UV[MinV].Y<=0)
    {
        BreakLoopCounter = 0;
        while ( UV[MinV].Y < 0 )
        {
            UV[0].Y++;
            UV[1].Y++;
            UV[2].Y++;
            UV[3].Y++;
            BreakLoopCounter++;
            if( BreakLoopCounter == 5000 ) break;
        }
    }
    else
    {
        BreakLoopCounter = 0;
        while ( (UV[MinV].Y >= 1.0f) && (UV[MinV].Y+1 >= 0.0f))
        {
            UV[0].Y--;
            UV[1].Y--;
            UV[2].Y--;
            UV[3].Y--;
            BreakLoopCounter++;
            if( BreakLoopCounter == 5000 ) break;
        }
    }
    ASSERT( UV[0].X >= 0.0f );
    ASSERT( UV[1].X >= 0.0f );
    ASSERT( UV[2].X >= 0.0f );
    ASSERT( UV[3].X >= 0.0f );
    ASSERT( UV[0].Y >= 0.0f );
    ASSERT( UV[1].Y >= 0.0f );
    ASSERT( UV[2].Y >= 0.0f );
    ASSERT( UV[3].Y >= 0.0f );
    // END OF UV Adjustment Code

    // check if we need to rotate the UVs
    if( RotateUVs )
    {
        UV[0].X = pStrip->UVs[0].Y;
        UV[0].Y = 1.0f - pStrip->UVs[0].X;
        UV[1].X = pStrip->UVs[1].Y;
        UV[1].Y = 1.0f - pStrip->UVs[1].X;
        UV[2].X = pStrip->UVs[2].Y;
        UV[2].Y = 1.0f - pStrip->UVs[2].X;
        UV[3].X = pStrip->UVs[3].Y;
        UV[3].Y = 1.0f - pStrip->UVs[3].X;
    }

    return UV;
}

//==========================================================================

void BuildStrips( t_Triangle* pTris, s32 NTris, u16* SubMeshTriData, t_TriangleStrip** pStrips, s32& NStrips, t_Triangle** pOutTris, s32& NOutTris, s32 MaxVertsInStrip, xbool RotateUVs )
{
    s32                 i, j;
    t_TriangleStrip*    StripArray;
    t_Triangle*         TriArray;

    s_GlobalStripList = NULL;

    s_StripPass = 0;

    //---   create the list of triangles to work from
    s_GlobalTriList = (t_TempTri*)x_malloc( NTris * sizeof(t_TempTri) );
    ASSERT( s_GlobalTriList );
    s_NGlobalTris = NTris;

    s32 UnusedTriCount = 0;
    s_LowestUsed = 0;
    //---   copy the user's triangles into our list
    for ( i = 0; i < NTris; i++ )
    {
        s_GlobalTriList[i].Verts[0] = pTris[SubMeshTriData[i]].Index[0];
        s_GlobalTriList[i].Verts[1] = pTris[SubMeshTriData[i]].Index[1];
        s_GlobalTriList[i].Verts[2] = pTris[SubMeshTriData[i]].Index[2];
        s_GlobalTriList[i].UVs[0] = pTris[SubMeshTriData[i]].UVs[0];
        s_GlobalTriList[i].UVs[1] = pTris[SubMeshTriData[i]].UVs[1];
        s_GlobalTriList[i].UVs[2] = pTris[SubMeshTriData[i]].UVs[2];
        s_GlobalTriList[i].InGlobalList = FALSE;
        UnusedTriCount++;
    }

    //---   now build the strips
    while ( UnusedTriCount > 0 )
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

        s_LowestUsed = pStrip->Tris[0];
        //---   go through the strip, marking its triangles as used
        for ( i = 0; i < pStrip->NVerts - 2; i++ )
        {
            s_GlobalTriList[pStrip->Tris[i]].InGlobalList = TRUE;
            UnusedTriCount--;
        }
    }

    //---   at this point, we have a linked list of triangle strips
    //      put them into a form which the user expects
    NStrips = CountStripsInLinkedList( s_GlobalStripList );
    if( NStrips == 0 )
        NOutTris = CountTrisInLinkedList( s_GlobalStripList );
    *pStrips = (t_TriangleStrip*)x_malloc( sizeof(t_TriangleStrip) * NStrips );
    ASSERT( *pStrips );
    *pOutTris = (t_Triangle*)x_malloc( sizeof(t_Triangle) * NOutTris );
    ASSERT( *pOutTris );
    StripArray = *pStrips;
    TriArray = *pOutTris;
    s32 TriIndex = 0;
    for ( i = 0; i < NStrips; i++ )
    {
        t_TempStrip*    pLink;

        //---   get a ptr to the current strip in the linked list
        pLink = s_GlobalStripList;
        ASSERT( pLink );

        if( NStrips == 0 && NOutTris && pLink->NVerts == 3 )
        {
            //---   copy that tri into the final list
            TriArray[TriIndex].MaterialID = pTris[SubMeshTriData[0]].MaterialID;
            for ( j = 0; j < pLink->NVerts; j++ )
            {
                TriArray[TriIndex].Index[j] = pLink->Verts[j];
                TriArray[TriIndex].UVs[j] = pLink->UVs[j];
            }
            TriIndex++;
            i--;
            NStrips--;
        }
        else
        {
            //---   copy that strip into the final list
            if( pLink->NVerts < 4 )
            {
                pLink->NVerts = 4;
                pLink->Verts[3] = pLink->Verts[2];
                pLink->UVs[3] = pLink->UVs[2];
            }
            StripArray[i].NVerts = pLink->NVerts;
            StripArray[i].pVerts = (s32*)x_malloc( sizeof(s32) * pLink->NVerts );
            ASSERT( StripArray[i].pVerts );
            for ( j = 0; j < StripArray[i].NVerts; j++ )
            {
                StripArray[i].pVerts[j] = pLink->Verts[j];
            }
            StripArray[i].UVs = (vector2*)x_malloc( sizeof(vector2) * pLink->NVerts );
            ASSERT( StripArray[i].UVs );
//            vector2* ModUVs = MakeUVs( pLink, RotateUVs );
            for ( j = 0; j < StripArray[i].NVerts; j++ )
            {
//                StripArray[i].UVs[j] = ModUVs[j];
                StripArray[i].UVs[j] = pLink->UVs[j];
            }
//            delete[] ModUVs;
        }
        
        //---   now remove this strip from the linked list
        s_GlobalStripList = pLink->pNextStrip;
        x_free( pLink );
    }

    //---   free the global list of triangles
    x_free( s_GlobalTriList );
}

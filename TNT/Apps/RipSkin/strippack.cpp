///////////////////////////////////////////////////////////////////////////
// strippack.cpp
///////////////////////////////////////////////////////////////////////////
                                                        
#include "x_files.hpp"
#include "strippack.h"

////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef struct STri
{
    xbool   Stripped;       // Already used in a strip
    s32     Encountered;    // Already encountered in current stripping
    s32     VertID[3];      // Vertex/Cache indices
    s32     NeighborID[3];  // Neighbor indices
    s32     NNeighbors;     // Number of triangle neighbors
    s32     TriID;          // Index of which tri it is in main geometry
} t_Tri;

////////////////////////////////////////////////////////////////////////////
// STATICS
////////////////////////////////////////////////////////////////////////////

static t_StripPack* s_pPack;
static s32          s_NPacks;
static s32          s_NPacksAllocated;

static t_Tri*       s_pTri;
static s32          s_NTris;
static s32          s_MaxVerts;
static s32          s_NonStripped0NeighborBonus;
static s32          s_NonStripped1NeighborBonus;

////////////////////////////////////////////////////////////////////////////
// Externs
////////////////////////////////////////////////////////////////////////////

extern xbool g_ReportStripDetails;

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

static
void BuildTris( s32* pTriVertIndices, s32* pTriIndices, s32 NTris )
{
    s32 i, j;
    s32 TriA, TriB;

    //---   Allocate parallel info for tris
    s_NTris = NTris;
    s_pTri = (t_Tri*)x_malloc( sizeof(t_Tri) * s_NTris );
    ASSERT( s_pTri );

    //---   Copy in verts
    for( i = 0; i < s_NTris; i++ )
    {
        s_pTri[i].VertID[0] = pTriVertIndices[(3 * i) + 0];
        s_pTri[i].VertID[1] = pTriVertIndices[(3 * i) + 1];
        s_pTri[i].VertID[2] = pTriVertIndices[(3 * i) + 2];
        s_pTri[i].TriID     = pTriIndices[i];
    }

    //---   Build neighbors
    for( TriA = 0; TriA < s_NTris; TriA++ )
    {
        //---   Clear neighbors for TriA
        s_pTri[TriA].NeighborID[0] = -1;
        s_pTri[TriA].NeighborID[1] = -1;
        s_pTri[TriA].NeighborID[2] = -1;
        s_pTri[TriA].NNeighbors = 0;

        //---   Search for neighbors
        for( TriB = 0; TriB < s_NTris; TriB++ )
        {
            if( (TriA != TriB) && (s_pTri[TriA].NNeighbors < 3) )
            {
                //---   loop through two-vert combinations of TriA
                for( i = 0; i < 3; i++ )
                {
                    s32 VertA0, VertA1;

                    //---   get the two-vert combo from TriA
                    VertA0 = s_pTri[TriA].VertID[(i + 0) % 3];
                    VertA1 = s_pTri[TriA].VertID[(i + 1) % 3];

                    //---   loop through two-vert combinations of TriB
                    for( j = 0; j < 3; j++ )
                    {
                        s32 VertB0, VertB1;

                        //---   get the two-vert combo from TriB
                        VertB0 = s_pTri[TriB].VertID[(j + 0) % 3];
                        VertB1 = s_pTri[TriB].VertID[(j + 1) % 3];

                        //---   Is it a neighbor?
                        //      NOTE: In order to be a neighbor for stripping,
                        //      it has to not only share to verts, but the
                        //      verts should be ordered in opposite directions
                        //      so that a strip can be made.
                        if( (VertA0 == VertB1) && (VertA1 == VertB0) )
                        {
                            s_pTri[TriA].NeighborID[i] = TriB;
                            s_pTri[TriA].NNeighbors++;
                        }
                    }
                }
            }
        }
    }

    //---   do a sanity check with the neighbors
    for ( TriA = 0; TriA < s_NTris; TriA++ )
    {
        //---   make sure the same tri doesn't show up as a neighbor
        //      more than once
        ASSERT( (s_pTri[TriA].NeighborID[0] == -1) ||
                ((s_pTri[TriA].NeighborID[0] != s_pTri[TriA].NeighborID[1]) &&
                 (s_pTri[TriA].NeighborID[0] != s_pTri[TriA].NeighborID[2])) );
        ASSERT( (s_pTri[TriA].NeighborID[1] == -1) ||
                ((s_pTri[TriA].NeighborID[1] != s_pTri[TriA].NeighborID[0]) &&
                 (s_pTri[TriA].NeighborID[1] != s_pTri[TriA].NeighborID[2])) );
        ASSERT( (s_pTri[TriA].NeighborID[2] == -1) ||
                ((s_pTri[TriA].NeighborID[2] != s_pTri[TriA].NeighborID[0]) &&
                 (s_pTri[TriA].NeighborID[2] != s_pTri[TriA].NeighborID[1])) );
    }
}

//==========================================================================

static
void PackStrips( void )
{
    s32     i, j;
    xbool   LoopAgain = TRUE;

    //---   This routine merges packs into larger packs.

    while( LoopAgain )
    {
        LoopAgain = FALSE;

        //---   Sort packs in order of longest to shortest (bubble sort)
        i = 1;
        while( i )
        {
            i = 0;
            for( j = 0; j < s_NPacks - 1; j++ )
            {
                if( s_pPack[j+1].NVerts > s_pPack[j].NVerts )
                {
                    t_StripPack T;
                    T            = s_pPack[j+1];
                    s_pPack[j+1] = s_pPack[j];
                    s_pPack[j]   = T;
                    i = 1;
                }
            }
        }

        //---   Check if this pack has room
        for( i = 0; i < s_NPacks; i++ )
        {
            if( s_pPack[i].NVerts )
            {
                s32 NVertsRemaining;
                s32 BestPack;
                s32 BestNVerts;

                NVertsRemaining = s_MaxVerts - s_pPack[i].NVerts;
                BestPack        = -1;
                BestNVerts      =  0;

                //---   Try to find largest pack that fits in current pack
                for( j = 0; j < s_NPacks; j++ )
                {
                    if( (i != j) && s_pPack[j].NVerts )
                    {
                        s32 NVerts;

                        NVerts = s_pPack[j].NVerts;
                        if( NVerts > NVertsRemaining )
                            continue;

                        //---   Check if better than current
                        if( NVerts > BestNVerts )
                        {
                            BestPack = j;
                            BestNVerts = NVerts;
                        }
                    }
                }

                //---   Did we find a pack?
                if( BestPack != -1 )
                {
                    LoopAgain = TRUE;

                    //---   Copy over strip verts
                    for( j = 0; j < s_pPack[BestPack].NVerts; j++ )
                    {
                        s_pPack[i].VertID[s_pPack[i].NVerts + j] =
                            s_pPack[BestPack].VertID[j];
                    }

                    //---   Set ADC bits on first two verts in new strip
                    s_pPack[i].VertID[s_pPack[i].NVerts + 0] |= 0x8000;
                    s_pPack[i].VertID[s_pPack[i].NVerts + 1] |= 0x8000;

                    //---   Adjust pack lengths
                    s_pPack[i].NVerts += s_pPack[BestPack].NVerts;
                    s_pPack[BestPack].NVerts = 0;

                    //---   Copy over pack tris so we can keep track of which tris each
                    //      pack contains
                    for( j = 0; j < s_pPack[BestPack].NTris; j++ )
                    {
                        s_pPack[i].TriID[s_pPack[i].NTris + j] =
                            s_pPack[BestPack].TriID[j];
                    }

                    //---   Adjust tri counts
                    s_pPack[i].NTris += s_pPack[BestPack].NTris;
                    s_pPack[BestPack].NTris = 0;

                    //---   Start over from beginning of list
                    break;
                }
            }
        }
    }
}

//==========================================================================

static
s32 SolveStrip( s32          StartTri,   // Tri to start strip in
                s32          StartVert,  // Vert to start strip on
                xbool        Clockwise,  // Direction to start strip
                s32&         rScore,     // Return 'score' here
                t_StripPack* pPack       // Pack to build strip into. Can be NULL.
               )
{
    static s32  EncounterCount = 0;
    s32         i, j;
    s32         ExitVert;
    s32         CurrentTri;
    s32         NextTri;
    s32         NTris;

    //---   Increment encounter count
    EncounterCount++;

    //---   If recording strip, start the pack off with the first two verts
    if( pPack )
    {
        if( Clockwise )
        {
            pPack->VertID[0] = s_pTri[StartTri].VertID[(StartVert + 0) % 3];
            pPack->VertID[1] = s_pTri[StartTri].VertID[(StartVert + 1) % 3];
        }
        else
        {
            pPack->VertID[0] = s_pTri[StartTri].VertID[(StartVert + 0) % 3];
            pPack->VertID[1] = s_pTri[StartTri].VertID[(StartVert + 2) % 3];
        }
        pPack->NVerts = 2;
        pPack->NTris  = 0;
    }
    
    //---   The exit vert is the vert that hasn't been added yet, but will be
    if( Clockwise )
        ExitVert = (StartVert + 2) % 3;
    else
        ExitVert = (StartVert + 1) % 3;

    //---   Some starting values...
    CurrentTri  = StartTri;
    NTris       = 0;
    rScore      = 0;

    //---   give a bonus for the start tri having one neighbor
    if( s_pTri[StartTri].NNeighbors == 1 )
        rScore += 1000;

    while( 1 )
    {
        s32 NNonStrippedNeighbors = 0;

        ASSERT( CurrentTri >= 0 );

        //---   Update score for current tri
        NNonStrippedNeighbors = 0;
        for( i = 0; i < 3; i++ )
        {
            if( s_pTri[CurrentTri].NeighborID[i] != -1 )
            {
                //---   if this neighbor tri has already been encountered, count it for a bonus
                if( (!s_pTri[s_pTri[CurrentTri].NeighborID[i]].Stripped) &&
                    (s_pTri[s_pTri[CurrentTri].NeighborID[i]].Encountered != EncounterCount) )
                {
                    NNonStrippedNeighbors++;
                }
            }
        }
        if( NNonStrippedNeighbors == 0 )
            rScore += s_NonStripped0NeighborBonus;  // a bonus for no neighbors
        else
        if( NNonStrippedNeighbors == 1 )
            rScore += s_NonStripped1NeighborBonus;  // a bonus for one neighbor
        rScore++;   // the score for adding a new tri

        //---   Mark tri as visited
        s_pTri[CurrentTri].Encountered = EncounterCount;
        NTris++;

        //---   If building pack...
        if( pPack )
        {
            //---   Add CurrentTri to list
            pPack->TriID[pPack->NTris] = CurrentTri;
            pPack->NTris++;

            //---   Add exit vert and move to next tri
            s_pTri[CurrentTri].Stripped = TRUE;
            pPack->VertID[pPack->NVerts] = s_pTri[CurrentTri].VertID[ExitVert];
            pPack->NVerts++;
        }

        //---   Switch direction
        Clockwise = !Clockwise;

        //---   Find next tri
        if( Clockwise )
            NextTri = s_pTri[CurrentTri].NeighborID[(ExitVert + 0) % 3];
        else
            NextTri = s_pTri[CurrentTri].NeighborID[(ExitVert + 2) % 3];

        //---   Check if we have terminated strip
        if( (NextTri==-1) ||                // no more tris
            (s_pTri[NextTri].Stripped) ||   // the next tri has been stripped
            (s_pTri[NextTri].Encountered == EncounterCount) ||  // the next tri has been stripped
            ((NTris+2) == s_MaxVerts) )     // we are at our vert max
            break;

        //---   Decide new exit vert
        for( i = 0; i < 3; i++ )
        {
            for( j = 0; j < 3; j++ )
            {
                if( s_pTri[NextTri].VertID[i] == s_pTri[CurrentTri].VertID[j] )
                    break;
            }

            if( j == 3 )
            {
                ExitVert = i;
                break;
            }
        }

        //---   Switch tris
        CurrentTri = NextTri;
    }

    return NTris + 2;
}

//==========================================================================

static
void BuildStrips( void )
{
    s32 i, j;
    s32 NTrisRemaining;

    //---   Determine how many tris are still alive
    NTrisRemaining = 0;
    for( i = 0; i < s_NTris; i++ )
    {
        if( !s_pTri[i].Stripped )
            NTrisRemaining++;
    }

    //---   Keep finding longest strip
    while( NTrisRemaining )
    {
        s32 BestLen;
        s32 BestTri;
        s32 BestDir;
        s32 BestVert;
        s32 BestScore;
        s32 Tri;
        s32 Len;
        s32 Score;

        //---   Find strip with best score
        BestScore   = 0;
        BestLen     = -1000;
        BestTri     = -1;
        for( Tri = 0; Tri < s_NTris; Tri++ )
        {
            if( !s_pTri[Tri].Stripped )
            {
                //---   Loop through trying each vert as a starting point
                for( i = 0; i < 3; i++ )
                {
                    //---   try both clockwise and counter-clockwise
                    for( j = 0; j <= 1; j++ )
                    {
                        Len = SolveStrip( Tri, i, j, Score, NULL );
                        if( Score > BestScore )
                        {
                            BestLen     = Len;
                            BestScore   = Score;
                            BestTri     = Tri;
                            BestVert    = i;
                            BestDir     = j;
                        }
                    }
                }
            }
        }

        ASSERT( BestScore );
        ASSERT( BestLen != -1000 );
        ASSERT( BestTri != -1 );

        //---   Create a new pack if necessary
        if( s_NPacks == s_NPacksAllocated )
        {
            s_NPacksAllocated += 100;
            s_pPack = (t_StripPack*)x_realloc( s_pPack,
                                               sizeof(t_StripPack) * s_NPacksAllocated );
            ASSERT(s_pPack);
        }

        //---   Re-solve best strip into pack
        Len = SolveStrip( BestTri, BestVert, BestDir, Score, &s_pPack[s_NPacks] );
        ASSERT( Len == BestLen );
        s_pPack[s_NPacks].IsStrip = TRUE;
        s_NPacks++;
        NTrisRemaining -= (Len - 2);
    }
}

//==========================================================================

static 
void OptimizeStrips( s32 OptimizeTries )
{
    s32             i, j;
    s32             NTries;
    t_StripPack*    pBestPacks;
    s32             NBestPacks;

    //---   Allocate new packs
    pBestPacks   = (t_StripPack*)x_malloc(sizeof(t_StripPack) * s_NPacks);
    ASSERT( pBestPacks );

    //---   Copy original packs into best
    NBestPacks = s_NPacks;
    x_memcpy( pBestPacks, s_pPack, sizeof(t_StripPack) * s_NPacks );

    //---   let the user know what's going on
    if ( g_ReportStripDetails )
    {
        x_printf( "Attempting to optimize strips...\n" );
        x_printf( "NStrips %1d\n", s_NPacks );
    }

    //---   seed the random # generator with a fixed value. The reason the seed
    //      is fixed is so that for a second pass through the same tris (for
    //      environment mapping, etc.), we are guaranteed to get the same results.
    x_srand( 12345 );

    //---   Try variations
    NTries = OptimizeTries;
    while( NTries-- )
    {
        s32 Rate;

        //---   Decide bonuses
        s_NonStripped0NeighborBonus = x_irand( 5, 15 );
        s_NonStripped1NeighborBonus = x_irand( 1, 10 );

        //---   Decide what percentage of packs to eliminate
        Rate = x_irand( 20, 50 );

        //---   Copy over random packs
        s_NPacks = 0;
        for( i = 0; i < NBestPacks; i++ )
        {
            if( x_irand( 0, 100 ) < Rate )
            {
                s_pPack[s_NPacks] = pBestPacks[i];
                s_NPacks++;
            }
        }

        //---   Clear all tris to un-stripped state
        for( i = 0; i < s_NTris; i++ )
        {
            s_pTri[i].Stripped = FALSE;
            s_pTri[i].Encountered = -1;
        }

        //---   Mark tris already accounted for in copied packs
        for( i = 0; i < s_NPacks; i++ )
        {
            for( j = 0; j < s_pPack[i].NTris; j++ )
            {
                s_pTri[s_pPack[i].TriID[j]].Stripped = TRUE;
            }
        }

        //---   Strip remaining tris
        BuildStrips();

        //---   Check if this set is better than best
        if( s_NPacks < NBestPacks )
        {
            NBestPacks = s_NPacks;
            x_memcpy( pBestPacks, s_pPack, sizeof(t_StripPack) * s_NPacks );
            if ( g_ReportStripDetails )
            {
                x_printf( "NStrips %1d (%2d,%2d,%2d)\n",
                          NBestPacks,
                          s_NonStripped0NeighborBonus,
                          s_NonStripped1NeighborBonus,
                          Rate );
            }
        }
    }

    //---   Copy best back into main list
    s_NPacks = NBestPacks;
    x_memcpy( s_pPack, pBestPacks, sizeof(t_StripPack) * NBestPacks );
}

//==========================================================================

static 
void WriteOutPacks( t_StripPack* pPack, s32 NPacks )
{
    if ( g_ReportStripDetails )
    {
        s32 i;
        s32 TotalVerts = 0;
        for( i = 0; i < NPacks; i++ )
        {
            TotalVerts += pPack[i].NVerts;
            x_printf( "%3d", pPack[i].NVerts );
        }
        x_printf("  [%1d]\n",TotalVerts);
    }
}

//==========================================================================

void STRIPPACK_PackStrips( t_StripPack** pPacks,             // Ptr to allocate packs for
                           s32&          rNPacks,            // N packs created
                           s32           NTris,              // Total num tris
                           s32*          pTriVertIndices,    // Cache indices for tris
                           s32*          pTriIndices,        // Tri's index in geometry                            
                           s32           MaxVerts,           // Max verts allowed in pack
                           s32           OptimizeTries
                           )
{
    s32             i, j;
    s32             NAlivePacks;
    t_StripPack*    pPack;

    ASSERT( pPacks && pTriVertIndices && pTriIndices );
    ASSERT( MaxVerts <= MAX_VERTS_PER_PACK );

    //---   Clear packs
    s_MaxVerts          = MaxVerts;
    s_pPack             = NULL;
    s_NPacksAllocated   = 0;
    s_NPacks            = 0;

    //---   Build tri structures
    BuildTris( pTriVertIndices, pTriIndices, NTris );

    //---   Clear tri stripping info
    for( i = 0; i < s_NTris; i++ )
    {
        s_pTri[i].Stripped = FALSE;
        s_pTri[i].Encountered = -1;
    }

    //---   Build a pack per strip
    s_NonStripped0NeighborBonus = 10;
    s_NonStripped1NeighborBonus = 5;
    BuildStrips();

    //---   Write out initial packs for debugging
    WriteOutPacks( s_pPack, s_NPacks );

    if( OptimizeTries > 0 )
    {
        //---   Optimize packs
        OptimizeStrips( OptimizeTries );

        //---   Write out 'optimized' packs
        WriteOutPacks( s_pPack, s_NPacks );
    }

    //---   Pack strips into buffers <= MaxVerts long
    PackStrips();

    //---   Write out packed packs
    WriteOutPacks( s_pPack, s_NPacks );

    //---   Recount packs
    NAlivePacks = 0;
    for( i = 0; i < s_NPacks; i++ )
    {
        if( s_pPack[i].NVerts > 0 )
            NAlivePacks++;
    }

    //---   Allocate user copy and make duplicates
    pPack = (t_StripPack*)x_malloc( sizeof(t_StripPack) * NAlivePacks );
    ASSERT(pPack);

    NAlivePacks = 0;
    for( i = 0; i < s_NPacks; i++ )
    {
        if( s_pPack[i].NVerts > 0 )
        {
            pPack[NAlivePacks] = s_pPack[i];
            NAlivePacks++;
        }
    }

    //---   Convert triid's to user's triids
    for( i = 0; i < NAlivePacks; i++ )
        for( j = 0; j < pPack[i].NTris; j++ )
            pPack[i].TriID[j] = s_pTri[pPack[i].TriID[j]].TriID;

    //---   Release tri structures
    x_free( s_pTri );

    //---   Release pack list
    x_free( s_pPack );

    //---   Return to user
    rNPacks = NAlivePacks;
    *pPacks = pPack;
}

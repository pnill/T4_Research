///////////////////////////////////////////////////////////////////////////
// strippack.h
///////////////////////////////////////////////////////////////////////////

#ifndef __STRIPPACK_H_INCLUDED__
#define __STRIPPACK_H_INCLUDED__

////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define MAX_VERTS_PER_PACK  80

////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef struct SStripPack
{
    xbool   IsStrip;
    s32     NVerts;
    s32     VertID[MAX_VERTS_PER_PACK];
    s32     NTris;
    s32     TriID[MAX_VERTS_PER_PACK];
} t_StripPack;

////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////

void STRIPPACK_PackStrips( t_StripPack** pPacks,             // Ptr to allocate packs for
                           s32&          rNPacks,            // N packs created
                           s32           NTris,              // Total num tris
                           s32*          pTriVertIndices,    // Cache indices for tris
                           s32*          pTriIndices,        // Tri's index in geometry
                           s32           MaxVerts,           // Max verts allowed in pack
                           s32           OptimizeTries       // Attempts at optimizing
                           );

#endif // __STRIPPACK_H_INCLUDED__

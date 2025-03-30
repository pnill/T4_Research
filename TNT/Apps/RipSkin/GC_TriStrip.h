////////////////////////////////////////////////////////////////////////////
//
// GC_TriStrip.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef GC_TRISTRIP_H_INCLUDED
#define GC_TRISTRIP_H_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct GC_StripTri
{
    u16          Verts[3];      // vertex indices
    u16          MtxID[3];      // matrix indices for each vert

    s16          bAddedToStrip; // temp for build process
    s16          bInCurStrip;   // temp for build process
    GC_StripTri* pNextTri;      // temp for build process
};

struct GC_TStrip
{
    s32         NVerts;         // number of verts in this strip
    u16*        pVerts;         // vertex indices in strip
};

struct GC_TStripGroup
{
    s32         NMatrices;      // number of matrices used in this group
    u16*        pMtxIDs;        // matrix indices used
    s32         NStrips;        // number of tri strips in this group
    GC_TStrip*  pStrips;        // triangle strips
};


////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////

// Builds strip-groups based on matrices used by verts.
// User must create an array of GC_StripTri's, filling out the
//   vertex and matrix index fields.
// Resulting strip-group data is allocated and passed back into
//   pStripGroups, and the number of strip-groups in rNGroups.
//
void GCTRISTRIP_Build( GC_TStripGroup** pStripGroups,
                       s32&             rNGroups,
                       GC_StripTri*     pTris,
                       s32              NTris,
                       s32              MaxVertsInStrip,
                       s32              MaxMatricesInGroup );

// Call to free memory used by a strip-group.
void GCTRISTRIP_Free( GC_TStripGroup** ppTStripGroup, s32 NGroups );


////////////////////////////////////////////////////////////////////////////

#endif // GC_TRISTRIP_H_INCLUDED
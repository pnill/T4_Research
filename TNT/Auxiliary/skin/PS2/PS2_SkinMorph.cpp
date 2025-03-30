////////////////////////////////////////////////////////////////////////////
//
// PS2_SkinMorph.cpp
//  - implementation of QSkin morphing routines
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_memory.hpp"
#include "x_bitmap.hpp"
#include "x_time.hpp"
#include "x_math.hpp"

#include "Q_SMEM.hpp"

#include "Skin.hpp"


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

byte* QSkin::BuildMorphTargetData( s32 MeshID )
{
    s32                 TargetID;
    s32                 DeltaID;
    byte*               pDeltaData;
    s32                 FirstDeltaValue;
    s32                 FirstDeltaLoc;
    t_SkinDeltaLoc*     pLoc;
    t_SkinDeltaValue*   pDest;
    t_SkinDeltaValue    DeltaValue;

    //---   if the target ID is the default target, and there is no weight
    //      we can just return the default delta data
    if ( m_pMorphTargets[m_pMeshes[MeshID].FirstMorphTarget].Active &&
         m_pMorphTargets[m_pMeshes[MeshID].FirstMorphTarget].Weight == 1.0f )
    {
        //#### Read the comment in SetActiveTargets about my hackish bug fix.
        return &m_pDeltaData[m_pMeshes[MeshID].DeltaDataOffset];
    }

    //---   it appears we must build new morph target data, allocate some
    //      room of the scratch memory heap for that, and copy over the
    //      default delta data
    pDeltaData = SMEM_BufferAlloc( m_pMeshes[MeshID].DeltaDataSize + 16 );
    if ( ((u32)pDeltaData & 0xf) != 0 )
        pDeltaData += 0x10 - ((u32)pDeltaData & 0xf);  // align to 16 byte boundary
    ASSERT( pDeltaData );
    ASSERT( ((u32)pDeltaData & 0xf) == 0 );
    ASSERT( (m_pMeshes[MeshID].DeltaDataSize & 0xf) == 0 );
    x_memcpy( pDeltaData,
              &m_pDeltaData[m_pMeshes[MeshID].DeltaDataOffset],
              m_pMeshes[MeshID].DeltaDataSize );

    //---   now loop through each of the targets, adding deltas based on weight
    //      as appropriate. (Note we can skip the first one because it is just 0.0f anyway)
    for ( TargetID = (m_pMeshes[MeshID].FirstMorphTarget + 1);
          TargetID < (m_pMeshes[MeshID].FirstMorphTarget + m_pMeshes[MeshID].NMorphTargets);
          TargetID++ )
    {
        if ( m_pMorphTargets[TargetID].Active )
        {
            //---   set the appropriate offsets for this morph target
            FirstDeltaValue  = m_pMeshes[MeshID].FirstDeltaValue;
            FirstDeltaValue += m_pMeshes[MeshID].NDeltaLocations *
                               (TargetID - m_pMeshes[MeshID].FirstMorphTarget);
            FirstDeltaLoc    = m_pMeshes[MeshID].FirstDeltaLocation;

            for ( DeltaID = 0; DeltaID < m_pMeshes[MeshID].NDeltaLocations; DeltaID++ )
            {
                pLoc = &m_pDeltaLocations[FirstDeltaLoc + DeltaID];

                DeltaValue.XDelta = (s16)(m_pMorphTargets[TargetID].Weight * (f32)m_pDeltaValues[FirstDeltaValue + DeltaID].XDelta);
                DeltaValue.YDelta = (s16)(m_pMorphTargets[TargetID].Weight * (f32)m_pDeltaValues[FirstDeltaValue + DeltaID].YDelta);
                DeltaValue.ZDelta = (s16)(m_pMorphTargets[TargetID].Weight * (f32)m_pDeltaValues[FirstDeltaValue + DeltaID].ZDelta);

                pDest = (t_SkinDeltaValue*)&pDeltaData[*pLoc];
                pDest->XDelta += DeltaValue.XDelta;
                pDest->YDelta += DeltaValue.YDelta;
                pDest->ZDelta += DeltaValue.ZDelta;
            }
        }
    }
    return pDeltaData;
}
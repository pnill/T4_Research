#include "x_types.hpp"
#include "x_math.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"

#include "RipMorph.h"

#include "BasicMorph.h"

////////////////////////////////////////////////////////////////////////////
// Implementation of the RipMorph classes
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// The morph delta class
////////////////////////////////////////////////////////////////////////////

QRipMorphDelta::QRipMorphDelta( void )
{
    m_VertID = -1;
}

//==========================================================================

QRipMorphDelta::~QRipMorphDelta( void )
{
}

////////////////////////////////////////////////////////////////////////////
// The morph target class
////////////////////////////////////////////////////////////////////////////

QRipMorphTarget::QRipMorphTarget( void )
{
    m_Name[0] = '\0';
    m_ChunkName[0] = '\0';
    m_NDeltas = 0;
    m_pDeltas = NULL;
}

//==========================================================================

QRipMorphTarget::~QRipMorphTarget( void )
{
    if ( m_pDeltas )
        delete []m_pDeltas;
}

////////////////////////////////////////////////////////////////////////////
// The morph target set
////////////////////////////////////////////////////////////////////////////

QRipMorphTargetSet::QRipMorphTargetSet( void )
{
    m_Name[0] = '\0';
    m_MeshName[0] = '\0';
    m_NTargets = 0;
    m_pTargets = NULL;
}


//==========================================================================

QRipMorphTargetSet::~QRipMorphTargetSet( void )
{
    if ( m_pTargets )
        delete []m_pTargets;
}

//==========================================================================

xbool QRipMorphTargetSet::Load( char* Filename )
{
    xbool                   bStat;
    CTextFile               Tf;
    BasicMorph::CTargetSet  TargetSet;

    if ( m_pTargets )
        delete []m_pTargets;
    m_pTargets = NULL;

    bStat = Tf.LoadTextFile( Filename );
    if ( !bStat )
    {
        return FALSE;
    }

    TargetSet.Load( Tf );

    //---   copy from the export data into the proper classes
    {
        s32 i, j;

        x_strcpy( m_MeshName, TargetSet.GetMeshName() );
        x_strcpy( m_Name, TargetSet.GetName() );

        m_NTargets = TargetSet.GetNumTargets();

        if ( m_NTargets )
        {
            m_pTargets = new QRipMorphTarget[m_NTargets];
            ASSERT( m_pTargets );

            for ( i = 0; i < m_NTargets; i++ )
            {
                if ( TargetSet.GetTarget(i).GetName()[0] == '_' )
                    x_strcpy( m_pTargets[i].m_Name, &TargetSet.GetTarget(i).GetName()[1] );
                else
                    x_strcpy( m_Name, TargetSet.GetTarget(i).GetName() );
                x_strcpy( m_pTargets[i].m_ChunkName, TargetSet.GetTarget(i).GetChunkName() );

                m_pTargets[i].m_NDeltas = TargetSet.GetTarget(i).GetNumDeltas();
                if ( m_pTargets[i].m_NDeltas )
                {
                    m_pTargets[i].m_pDeltas = new QRipMorphDelta[m_pTargets[i].m_NDeltas];
                    ASSERT( m_pTargets[i].m_pDeltas );

                    for ( j = 0; j < m_pTargets[i].m_NDeltas; j++ )
                    {
                        m_pTargets[i].m_pDeltas[j].m_VertID = TargetSet.GetTarget(i).GetDelta(j).m_iVert;
                        m_pTargets[i].m_pDeltas[j].m_Delta  = TargetSet.GetTarget(i).GetDelta(j).m_vDelta;
                    }
                }
            }
        }
    }

    //######################################################################
    //######################################################################
    /*{
        s32 i, j;

        for ( i = 0; i < m_NTargets; i++ )
        {
            for ( j = 0; j < m_pTargets[i].m_NDeltas; j++ )
            {
                m_pTargets[i].m_pDeltas[j].m_Delta *= 100.0f;
            }
        }
    }*/
    //######################################################################
    //######################################################################

    return TRUE;
}

//==========================================================================

void QRipMorphTargetSet::AddTarget( QRipMorphTarget* pSrc )
{
    s32                 NTargets;
    QRipMorphTarget*    pNewSet;
    s32                 i, j;

    //---   create a new morph target set
    NTargets = m_NTargets + 1;
    pNewSet = new QRipMorphTarget[NTargets];
    ASSERT( pNewSet );

    //---   copy over the original data
    for ( i = 0; i < m_NTargets; i++ )
    {
        //---   memcpy isn't good enough, we actually need to create a new copy of the data
        x_strcpy( pNewSet[i].m_Name, m_pTargets[i].m_Name );
        x_strcpy( pNewSet[i].m_ChunkName, m_pTargets[i].m_ChunkName );
        pNewSet[i].m_NDeltas = m_pTargets[i].m_NDeltas;
        pNewSet[i].m_pDeltas = new QRipMorphDelta[pNewSet[i].m_NDeltas];
        ASSERT( pNewSet[i].m_pDeltas );
        for ( j = 0; j < pNewSet[i].m_NDeltas; j++ )
        {
            x_memcpy( &pNewSet[i].m_pDeltas[j], &m_pTargets[i].m_pDeltas[j], sizeof(QRipMorphDelta) );
        }
    }

    //---   add the new data
    x_strcpy( pNewSet[m_NTargets].m_Name, pSrc->m_Name );
    x_strcpy( pNewSet[m_NTargets].m_ChunkName, pSrc->m_ChunkName );
    pNewSet[m_NTargets].m_NDeltas = pSrc->m_NDeltas;
    pNewSet[m_NTargets].m_pDeltas = new QRipMorphDelta[pNewSet[m_NTargets].m_NDeltas];
    ASSERT( pNewSet[m_NTargets].m_pDeltas );
    for ( j = 0; j < pNewSet[m_NTargets].m_NDeltas; j++ )
    {
        x_memcpy( &pNewSet[m_NTargets].m_pDeltas[j], &pSrc->m_pDeltas[j], sizeof(QRipMorphDelta) );
    }

    //---   update this
    m_NTargets++;
    if ( m_pTargets )
        delete[] m_pTargets;
    m_pTargets = pNewSet;
}

//==========================================================================

void QRipMorphTargetSet::Merge( QRipMorphTargetSet* pSet )
{
    s32                 NTargets;
    QRipMorphTarget*    pNewSet;
    s32                 i, j;

    //---   bail out early if we can
    if ( pSet->m_NTargets == 0 )
        return;

    //---   create a new morph target set
    NTargets = m_NTargets + pSet->m_NTargets;
    pNewSet = new QRipMorphTarget[NTargets];
    ASSERT( pNewSet );

    //---   copy over the original data
    for ( i = 0; i < m_NTargets; i++ )
    {
        //---   memcpy isn't good enough, we actually need to create a new copy of the data
        x_strcpy( pNewSet[i].m_Name, m_pTargets[i].m_Name );
        x_strcpy( pNewSet[i].m_ChunkName, m_pTargets[i].m_ChunkName );
        pNewSet[i].m_NDeltas = m_pTargets[i].m_NDeltas;
        pNewSet[i].m_pDeltas = new QRipMorphDelta[pNewSet[i].m_NDeltas];
        ASSERT( pNewSet[i].m_pDeltas );
        for ( j = 0; j < pNewSet[i].m_NDeltas; j++ )
        {
            x_memcpy( &pNewSet[i].m_pDeltas[j], &m_pTargets[i].m_pDeltas[j], sizeof(QRipMorphDelta) );
        }
    }

    //---   copy in the new data
    for ( i = 0; i < pSet->m_NTargets; i++ )
    {
        //---   memcpy isn't good enough, we actually need to create a new copy of the data
        x_strcpy( pNewSet[i+m_NTargets].m_Name, pSet->m_pTargets[i].m_Name );
        x_strcpy( pNewSet[i+m_NTargets].m_ChunkName, pSet->m_pTargets[i].m_ChunkName );
        pNewSet[i+m_NTargets].m_NDeltas = pSet->m_pTargets[i].m_NDeltas;
        pNewSet[i+m_NTargets].m_pDeltas = new QRipMorphDelta[pNewSet[i+m_NTargets].m_NDeltas];
        ASSERT( pNewSet[i+m_NTargets].m_pDeltas );
        for ( j = 0; j < pNewSet[i+m_NTargets].m_NDeltas; j++ )
        {
            x_memcpy( &pNewSet[i+m_NTargets].m_pDeltas[j], &pSet->m_pTargets[i].m_pDeltas[j], sizeof(QRipMorphDelta) );
        }
    }

    //---   update this
    m_NTargets = NTargets;
    if ( m_pTargets )
        delete[] m_pTargets;
    m_pTargets = pNewSet;
}

//==========================================================================

void QRipMorphTargetSet::RotateY180( void )
{
    s32 i, j;

    for ( i = 0; i < m_NTargets; i++ )
    {
        for ( j = 0; j < m_pTargets[i].m_NDeltas; j++ )
        {
            m_pTargets[i].m_pDeltas[j].m_Delta.X = -m_pTargets[i].m_pDeltas[j].m_Delta.X;
            m_pTargets[i].m_pDeltas[j].m_Delta.Z = -m_pTargets[i].m_pDeltas[j].m_Delta.Z;
        }
    }
}
#ifndef __RIPMORPH_H_INCLUDED__
#define __RIPMORPH_H_INCLUDED__

////////////////////////////////////////////////////////////////////////////
// The morph delta class
////////////////////////////////////////////////////////////////////////////

class QRipMorphDelta
{
public:
             QRipMorphDelta( void );
    virtual ~QRipMorphDelta( void );

    s32             m_VertID;
    vector3         m_Delta;
};

////////////////////////////////////////////////////////////////////////////
// The morph target class
////////////////////////////////////////////////////////////////////////////

class QRipMorphTarget
{
public:
             QRipMorphTarget( void );
    virtual ~QRipMorphTarget( void );

    char                m_Name[256];
    char                m_ChunkName[256];
    s32                 m_NDeltas;
    QRipMorphDelta*     m_pDeltas;
};

////////////////////////////////////////////////////////////////////////////
// The morph target set
////////////////////////////////////////////////////////////////////////////

class QRipMorphTargetSet
{
public:
             QRipMorphTargetSet( void );
    virtual ~QRipMorphTargetSet( void );

    xbool   Load( char* Filename );

    void Merge( QRipMorphTargetSet* pSet );
    void AddTarget( QRipMorphTarget* pSrc );
    void RotateY180( void );

    char                m_Name[256];
    char                m_MeshName[256];
    s32                 m_NTargets;
    QRipMorphTarget*    m_pTargets;
};


#endif
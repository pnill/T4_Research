#ifndef __GEOMMERGE_H_INCLUDED__
#define __GEOMMERGE_H_INCLUDED__

#include "AsciiMesh.h"

#define MAX_NUM_MERGES      60

class QGeomMerge
{
public:
    QGeomMerge( void );
    ~QGeomMerge( void );

    void AddGeom( mesh::object*  pObject );
    void MergeObjects( mesh::object* pDst );

    void CopyGeom( mesh::object* pDst, mesh::object* pSrc );

    s32             m_NGeoms;
    mesh::object*   m_pGeoms[MAX_NUM_MERGES];
};

#endif
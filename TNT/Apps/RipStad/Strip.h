#ifndef __STRIP_H_INCLUDED__
#define __STRIP_H_INCLUDED__

////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////

typedef struct STriangleStrip
{
    s32     NVerts;
    s32*    pVerts;
    vector2* UVs;
} t_TriangleStrip;

////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////

// IMPORTANT NOTE: After calling BuildStrips, the user will be responsible
// for freeing the memory that is allocated by BuildStrips
void BuildStrips( t_Triangle* pTris, s32 NTris, u16* SubMeshTriData, t_TriangleStrip** pStrips, s32& NStrips, t_Triangle** pOutTris, s32& NOutTris, s32 MaxVertsInStrip, xbool RotateUVs );

#endif
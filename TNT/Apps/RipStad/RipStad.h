////////////////////////////////////////////////////////////////////////////
// RipGeom.h - header file for the RipGeom app (get included by the various
//             exporters)
////////////////////////////////////////////////////////////////////////////

#ifndef __RIPGEOM_H_INCLUDED__
#define __RIPGEOM_H_INCLUDED__

#include "x_stdio.hpp"
#include "x_files.hpp"

////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////
#define COLL_TRI_FLAGS_HIGH             0x0001
#define COLL_TRI_FLAGS_STOP             0x0002
#define COLL_TRI_FLAGS_HOMERUN          0x0004
#define COLL_TRI_FLAGS_MEDIUM           0x0008
#define COLL_TRI_FLAGS_LOW              0x0010
#define COLL_TRI_FLAGS_GROUND           0x0020
#define COLL_TRI_FLAGS_FOULPOLE         0x0040
#define COLL_TRI_FLAGS_ZEROPOINT        0x0080
#define COLL_TRI_FLAGS_AUDIO_WAREHOUSE  0x0100
#define COLL_TRI_FLAGS_AUDIO_WAVELAND   0x0200
#define COLL_TRI_FLAGS_AUDIO_SPLASH     0x0400
#define COLL_TRI_FLAGS_AUDIO_POOL       0x0800
#define COLL_TRI_FLAGS_AUDIO_TRAIN      0x1000
#define COLL_TRI_FLAGS_AUDIO_HILL       0x2000
#define COLL_TRI_FLAGS_FOUL				0x4000

////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////

typedef struct STriangle
{
    s32     Index[3];
    vector2 UVs[3];
    s32     MaterialID;
    xbool   Used;
    s32     iGrid;
    vector3 Centroid;

} t_Triangle;

typedef struct
{
    vector3 Verts[3];
    s32     Type;
} t_CollisionTri;

////////////////////////////////////////////////////////////////////////////
// Externs
////////////////////////////////////////////////////////////////////////////

extern s32      g_Verbose;
extern xbool    g_ExportTextures;
extern xbool    g_ExportPosPtrs;
extern xbool    g_ExportUVPtrs;
extern xbool    g_ExportRGBAPtrs;
extern xbool    g_ExportNormalPtrs;
extern xbool    g_BuildTriStrips;
extern xbool    g_EnableAlpha;
extern xbool    g_EnableAA;
extern xbool    g_UseMips;
extern xbool    g_NightVersion;
extern xbool    g_DynamicLighting;
extern X_FILE*  g_TextureNameFile;
extern X_FILE*  g_CollisionFile;
extern X_FILE*  g_GridPointFile;
extern X_FILE*  g_TextOut;

////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////

void PauseOnError( void );

#endif // __RIPGEOM_H_INCLUDED__
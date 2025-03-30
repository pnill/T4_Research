///////////////////////////////////////////////////////////////////////////
//
//  QRAW.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef _QRAW_H_
#define _QRAW_H_

///////////////////////////////////////////////////////////////////////////
//  INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_math.hpp"

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//  STRUCTURES
///////////////////////////////////////////////////////////////////////////

#define MAX_RAW_MESHES      512
#define MAX_RAW_MATERIALS   512


typedef struct
{
    vector3  XYZ[3];
    vector3  UV[3];
    vector3  Normal[3];
    vector3  Color[3];
    f32      Alpha[3];
    s32      MatrixID[3];
    s32      MaterialID;
    s32      MeshID;
} qtri;


typedef struct
{
    char    TextureName[128];
    s32     TextureWidth;
    s32     TextureHeight;
} qtexture;


typedef struct 
{
    char    MaterialName[128];
    s32     TextureIndex;          
    s32     UMirror;               // On/Off
    s32     VMirror;               // On/Off
    f32     UTile;
    f32     VTile;
    f32     UOffset;
    f32     VOffset;
    radian  Angle;
    s32     References;    
} qmaterial;


typedef struct
{
    s32             NMeshes;
    char            MeshName[MAX_RAW_MESHES][32];
    s32             NMaterials;
    qmaterial       Material[MAX_RAW_MATERIALS];
    s32             NQTris;
    s32             NTextures;
    qtexture        Texture[MAX_RAW_MATERIALS];
    qtri*           QTri;
//    u16             MeshFlags[MAX_MESHES];       
} qraw;

///////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
///////////////////////////////////////////////////////////////////////////
// General purpose functions.
///////////////////////////////////////////////////////////////////////////

err     QRAW_LoadQRaw       ( qraw* QRaw, char* FileName );
err     QRAW_WriteQRaw      ( qraw* QRaw, char* FileName );
                            
void    QRAW_InitInstance   ( qraw* QRaw );
void    QRAW_KillInstance   ( qraw* QRaw );

err     QRAW_CheckIntegrity ( qraw* QRaw );

err     QRAW_CleanQRaw      ( qraw* QRaw );
err     QRAW_GetMeshIndex   ( qraw* QRaw, s32* DestIndex, char* MeshName );
err     QRAW_MergeMesh      ( qraw* QRaw, char* MergeInto, char* MergeFrom );
err     QRAW_RenameMesh     ( qraw* QRaw, char* NameTo, char* NameFrom );
err     QRAW_DeleteMesh     ( qraw* QRaw, char* MeshName );
err     QRAW_CopyMesh       ( qraw* DestQRaw, char* DestMeshName, qraw* SrcQRaw, char* SrcMeshName );
void    QRAW_SortMeshes     ( qraw* QRaw );
void    QRAW_SortTextures   ( qraw* QRaw );
f32     QRAW_GetShortestEdge( qraw* QRaw );
void    QRAW_MergeVertices  ( qraw* QRaw, f32 Epsilon, s32* NXYZsMerged, s32* NUVsMerged, s32* NTsRemoved );

void    QRAW_Transform      ( qraw* QRaw, matrix4* M );
void    QRAW_Bounds         ( qraw* QRaw, vector3 ** Min, vector3** Max, f32* MaxDist );
void    QRAW_OrientedBounds ( qraw* QRaw, vector3 ** Bound );

void    QRAW_ComputeNormals ( qraw* QRaw );

void    QRAW_N64_AdjustBilerp( qraw* QRaw );
void    QRAW_Copy           ( qraw* Dest, qraw* Src );

///////////////////////////////////////////////////////////////////////////
// Specialized functions created for specific game requirements.
///////////////////////////////////////////////////////////////////////////

void    QRAW_CreateCullMaps( qraw* QRaw, s16** Offset, byte** Data, s32* DataSize );

///////////////////////////////////////////////////////////////////////////

#endif

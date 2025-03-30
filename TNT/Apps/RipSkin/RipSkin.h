///////////////////////////////////////////////////////////////////////////
//  RipSkin.h
///////////////////////////////////////////////////////////////////////////
#ifndef __RIPSKIN_H_INCLUDED__
#define __RIPSKIN_H_INCLUDED__

#include "x_math.hpp"
#include "x_color.hpp"

#define MAX_NUM_MATRIX       8
#define UNUSED_MATRIX_ID    -1

////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef struct SRipVert
{
    vector3 Pos;
    vector3 Normal;
    vector2 UV;
    color   Color;

    s32     TransformID;					// index into m_pTransform
    s32     NMatrices;						// # of matrices used by vert
    s16     MatrixID[MAX_NUM_MATRIX];		// which matrices used by vert
    f32     MatrixWeight[MAX_NUM_MATRIX];	// weight per matrix (Total to 1.0f)

    s32     FirstTri;						// First tri in m_pTri
    s32     NTris;							// # of tris in m_pTri

    xbool   HasMorphDelta;					// Vertex can be morphed.

    s32     CacheID;						// where this vert is in the cache this frame
    s32     NTrisDepending;					// # of tris which haven't been satisfied yet that use this vert

	s32		OldVertID;						// old vertex index before sort, to connect vert back to tris

} t_RipVert;


////////////////////////////////////////////////////////////////////////////
// The QRipSkin class
////////////////////////////////////////////////////////////////////////////

class QRipSkin
{
public:
    ////////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    ////////////////////////////////////////////////////////////////////////

             QRipSkin   ( void );
    virtual ~QRipSkin   ( void );

    ////////////////////////////////////////////////////////////////////////
    // Functions for the user  ( All pure virtual! Define per platform )
    ////////////////////////////////////////////////////////////////////////

    virtual void Clear              ( void )                                            = 0;
    virtual void SetSkinName        ( char* Name )                                      = 0;
    virtual void AddBone            ( char* Name )                                      = 0;
    virtual void NewTexture         ( char* TexturePath, s32 Width, s32 Height )        = 0;
    virtual void NewMesh            ( char* MeshName,
                                      xbool EnvMapped,
                                      xbool Shadow,
                                      xbool Alpha )                                     = 0;
    virtual void AddVert            ( t_RipVert& rRipVert )                             = 0;
    virtual void AddTri             ( s32   Vert1,
                                      s32   Vert2,
                                      s32   Vert3,
                                      char* pTextureName )                              = 0;
    virtual void NewMorphTarget     ( char* TargetName )                                = 0;
    virtual void AddMorphDelta      ( s32 VertID, vector3 Delta )                       = 0;
    virtual void ForceSingleMatrix  ( void )                                            = 0;
    virtual void ForceDoubleMatrix  ( void )                                            = 0;
    virtual void ForceTripleMatrix  ( void )                                            = 0;
    virtual void PrepareStructures  ( void )                                            = 0;
    virtual void DisplayStats       ( void )                                            = 0;
    virtual void Save               ( char* pFileName )                                 = 0;
};


//=====================================================================================================================================
// VertNode
//=====================================================================================================================================
struct VertNode
{
    s32   MeshID;
    s32   VertID;

    VertNode* pNext;
};


//=====================================================================================================================================
// VertList
//=====================================================================================================================================
struct VertList
{
    VertList( void );
    ~VertList( void );
    void AttachList( s32 MeshID, s32 VertID );
    void KillList( void );

    void AddNode( s32 MeshID, s32 VertID );

    //-------------------------------------------------------------------------------------------------------------------------------------
    xbool     bDynamic;
    VertNode* mpHead;
    VertList* mpNextList;
};


#endif // __RIPSKIN_H_INCLUDED__

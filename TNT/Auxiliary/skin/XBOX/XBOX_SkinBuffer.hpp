#ifndef __XBOX_SKIN_BUFFER_INCLUDED__
#define __XBOX_SKIN_BUFFER_INCLUDED__

#include "XBOX_Skin.hpp"
#include "x_types.hpp"

//=====================================================================================================================================
//
// XBOXSkinBuffer is a block of Scratch memory which is used to manipulate data in order to make the rendering
// of skins more efficient.  The matrix array (1 matrix per bone in a skeleton) is loaded for each skin that is
// going to be rendered.  Each submesh that is to be rendered also has a SubmeshGroupData packet loaded into 
// the skin buffer to help with sorting and optomizing the submesh rendering process.
// 
// It's organized like this:
//
//                  ____________
//       MatrixData |  Skin1   |
//                  |  Skin2   |
//                  |   ...    |
//                  |  SkinN-1 | // Where N is the max number of skins.
//                  |  SkinN   |
// SubmeshGroupData | Submesh1 |
//                  | Submesh2 |
//                  |   ....   |
//                  |SubmeshM-1| // Where M is the MaxSubmeshCountPerSkin * MaxNumberOfSkins
//                  | SubmeshM |
//                  ------------
//=====================================================================================================================================
struct XBOXSkinBuffer
{
    static void             Kill( void );
    static xbool            IsValid( void );

    static t_MatrixGroup*   GetMatrixGroup( s32 SkinOffset = -1 );     // Defaulting to -1 provides the address of the next Matrix Group.
    static t_SubMeshGroup*  GetSubMeshGroup( s32 SubmeshOffset = -1 ); // Defaulting to -1 provides the address of the next Submesh Group.

    static void             UpdateSkinCount( void );
    static void             AddSubmeshGroup( t_SubMeshGroup* pNewGroup );

    static t_MatrixGroup*   mpMatrices;                // The matrices used to render a skin (1 per skin in the skin buffer)
    static t_SubMeshGroup*  mpSubMeshGroup;            // Submeshes for all skins shared in this buffer.

    static byte*            mpSMEMData;
    static u32              mScratchDataSize;

    static s32              mMaxNumberOfSkins;
    static s32              mNSkinsInBuffer;

    static s32              mMaxNumberOfSubmeshes;
    static s32              mNSubmeshesInBuffer;
};



#endif // __XBOX_SKIN_BUFFER_INCLUDED__
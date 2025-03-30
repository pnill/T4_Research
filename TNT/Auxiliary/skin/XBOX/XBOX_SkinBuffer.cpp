#include "XBOX_SkinBuffer.hpp"
#include "x_debug.hpp"

t_MatrixGroup*  XBOXSkinBuffer::mpMatrices;
t_SubMeshGroup* XBOXSkinBuffer::mpSubMeshGroup;
byte*           XBOXSkinBuffer::mpSMEMData;
u32             XBOXSkinBuffer::mScratchDataSize;
s32             XBOXSkinBuffer::mMaxNumberOfSkins;
s32             XBOXSkinBuffer::mNSkinsInBuffer;
s32             XBOXSkinBuffer::mMaxNumberOfSubmeshes;
s32             XBOXSkinBuffer::mNSubmeshesInBuffer;

XBOXSkinBuffer SkinBuffer;

// Inlines.
//=====================================================================================================================================
void XBOXSkinBuffer::Kill( void )
{
    mMaxNumberOfSkins     = 0;
    mMaxNumberOfSubmeshes = 0;
    mNSkinsInBuffer       = 0;
    mNSubmeshesInBuffer   = 0;
    mpMatrices            = NULL;
    mpSubMeshGroup        = NULL;
    mpSMEMData            = NULL;
}

//=====================================================================================================================================
xbool XBOXSkinBuffer::IsValid( void )
{ 
    return (mpSMEMData != NULL);// && (mNSkinsInBuffer <= mMaxNumberOfSkins);
}

//=====================================================================================================================================
t_MatrixGroup* XBOXSkinBuffer::GetMatrixGroup( s32 SkinOffset )
{
    ASSERT( SkinOffset <= mMaxNumberOfSkins );
    
    // If Skin offset is negative, then provide the next available matrix group pointer.
    if( SkinOffset == -1 )
        SkinOffset = mNSkinsInBuffer;

    return &mpMatrices[SkinOffset];
}

//=====================================================================================================================================
t_SubMeshGroup* XBOXSkinBuffer::GetSubMeshGroup( s32 SubmeshOffset )
{
    ASSERT( SubmeshOffset <= mMaxNumberOfSubmeshes );
    
    // If SubmeshOffset is negative, then provide the next available submesh group pointer.
    if( SubmeshOffset == -1 )
        SubmeshOffset = mNSubmeshesInBuffer;

    return &mpSubMeshGroup[SubmeshOffset];
}

//=====================================================================================================================================
void XBOXSkinBuffer::UpdateSkinCount( void )
{
    mNSkinsInBuffer++;
//    ASSERT( mNSkinsInBuffer <= mMaxNumberOfSkins );
}


//=====================================================================================================================================
void XBOXSkinBuffer::AddSubmeshGroup( t_SubMeshGroup* pNewGroup )
{
    // Add the Submesh group to the skin buffer.
    mpSubMeshGroup[mNSubmeshesInBuffer] = *pNewGroup;

    mNSubmeshesInBuffer++;
    ASSERT( mNSubmeshesInBuffer < mMaxNumberOfSubmeshes );
}
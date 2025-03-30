#include "PC_SkinBuffer.hpp"
#include "x_debug.hpp"

t_MatrixGroup*  PCSkinBuffer::mpMatrices;
t_SubMeshGroup* PCSkinBuffer::mpSubMeshGroup;
byte*           PCSkinBuffer::mpSMEMData;
u32             PCSkinBuffer::mScratchDataSize;
s32             PCSkinBuffer::mMaxNumberOfSkins;
s32             PCSkinBuffer::mNSkinsInBuffer;
s32             PCSkinBuffer::mMaxNumberOfSubmeshes;
s32             PCSkinBuffer::mNSubmeshesInBuffer;

PCSkinBuffer SkinBuffer;

// Inlines.
//=====================================================================================================================================
void PCSkinBuffer::Kill( void )
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
xbool PCSkinBuffer::IsValid( void )
{ 
    return (mpSMEMData != NULL);// && (mNSkinsInBuffer <= mMaxNumberOfSkins);
}

//=====================================================================================================================================
t_MatrixGroup* PCSkinBuffer::GetMatrixGroup( s32 SkinOffset )
{
    ASSERT( SkinOffset <= mMaxNumberOfSkins );
    
    // If Skin offset is negative, then provide the next available matrix group pointer.
    if( SkinOffset == -1 )
        SkinOffset = mNSkinsInBuffer;

    return &mpMatrices[SkinOffset];
}

//=====================================================================================================================================
t_SubMeshGroup* PCSkinBuffer::GetSubMeshGroup( s32 SubmeshOffset )
{
    ASSERT( SubmeshOffset <= mMaxNumberOfSubmeshes );
    
    // If SubmeshOffset is negative, then provide the next available submesh group pointer.
    if( SubmeshOffset == -1 )
        SubmeshOffset = mNSubmeshesInBuffer;

    return &mpSubMeshGroup[SubmeshOffset];
}

//=====================================================================================================================================
void PCSkinBuffer::UpdateSkinCount( void )
{
    mNSkinsInBuffer++;
//    ASSERT( mNSkinsInBuffer <= mMaxNumberOfSkins );
}


//=====================================================================================================================================
void PCSkinBuffer::AddSubmeshGroup( t_SubMeshGroup* pNewGroup )
{
    // Add the Submesh group to the skin buffer.
    mpSubMeshGroup[mNSubmeshesInBuffer] = *pNewGroup;

    mNSubmeshesInBuffer++;
    ASSERT( mNSubmeshesInBuffer < mMaxNumberOfSubmeshes );
}
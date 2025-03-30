#ifndef GC_MEMORY_UNIT_RESOURCE_HPP
#define GC_MEMORY_UNIT_RESOURCE_HPP


//=====================================================================================================================================
// Required Includes.
//=====================================================================================================================================
#include <dolphin/card.h>
#include "Q_MemoryUnitResource.hpp"

//=====================================================================================================================================
// Definitions.
//=====================================================================================================================================
#define GCMU_USER_DATA_FILENAME_LENGTH    (CARD_FILENAME_MAX)


//=====================================================================================================================================
// GC Resource structures
//=====================================================================================================================================
struct GCMUResourceHeader
{
    s32 mResourceType;                  // Resource Type, see QuagMemoryUnitResourceTypes

    s32 mNumberOfSaveTypes;             // Number of unique types of saves for this game.
    s32 mOffsetToSaveTypes;             // Offset from the start of the file to the save type definitions.

    s32 mOffsetToIconInfo;              // Offset from the start of the file to the information about the icon
                                        // used by this game.
    s32 mOffsetToIconData;              // Offset from the start of the file to the actual icon data.

    s32 mSizeOfEntireResourceFile;      // Size of entire resource file to use to load entire resource if you wish.
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct GCMUUserDataFile
{
    char    mUserDataSaveFileName[GCMU_USER_DATA_FILENAME_LENGTH];  // The names of each user data file that is saved when using this save type.
    s32     mSizeOfUserDataFile;                                    // The exact number of bytes of user data stored within this file.
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct GCMUResourceSaveGameType
{
    // How many saves of this type are going to be used. Max is MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE
    s32                 mMaxNumberOfSaves;
    
    // Information on the user file that is special to this particular save type.
    GCMUUserDataFile    mUserDataFileInfo;
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct GCMUResourceIconInfo
{
    s32     mNumberOfIconFrames;        // The number of frames in the icon.
    s32     mIconAnimSpeed;             // Speed of the animated icon used by this
    s32     mIconDataSize;              // Size in bytes of the icon frames, and the clut.
};



//=====================================================================================================================================
// GameCube Resource File Layout.
//=====================================================================================================================================
// 
//-------------------------------------------------------------------------------------------------------------------------------------
//
// GCMUResourceHeader
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// GCMUResourceSaveGameType 0
// GCMUResourceSaveGameType 1
// GCMUResourceSaveGameType 2
// ..
// ..
// ..
// GCMUResourceSaveGameType X
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// GCMUResourceIconInfo
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// GameCube Icon Data
//
//=====================================================================================================================================

#endif // GC_MEMORY_UNIT_RESOURCE_HPP
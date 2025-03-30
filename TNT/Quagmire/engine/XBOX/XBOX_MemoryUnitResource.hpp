#ifndef XBOX_MEMORY_UNIT_RESOURCE_HPP
#define XBOX_MEMORY_UNIT_RESOURCE_HPP

//=====================================================================================================================================
// Required Includes.
//=====================================================================================================================================
#include "Q_MemoryUnitResource.hpp"

//=====================================================================================================================================
// Definitions.
//=====================================================================================================================================
#define XBOX_USER_DATA_FILENAME_LENGTH      32  // Don't konw what this is yet  so 1 will break things to wake me up.

//=====================================================================================================================================
// XBOX Resource structures
//=====================================================================================================================================
struct XBOXMUResourceHeader
{
    s32 mResourceType;                  // Resource Type, see QuagMemoryUnitResourceTypes

    s32 mNumberOfSaveTypes;             // Number of unique types of saves for this game.
    s32 mOffsetToSaveTypes;             // Offset from the start of the file to the save type definitions.

    s32 mSizeOfIconData;
    s32 mOffsetToIconData;              // Offset from the start of the file to the actual icon data.

    s32 mSizeOfEntireResourceFile;      // Size of entire resource file to use to load entire resource if you wish.
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct XBOXMUUserDataFile
{
    char    mUserDataSaveFileName[XBOX_USER_DATA_FILENAME_LENGTH];  // The names of each user data file that is saved when using this save type.
    s32     mSizeOfUserDataFile;                                    // The exact number of bytes of user data stored within this file.
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct XBOXMUResourceSaveGameType
{
    // How many saves of this type are going to be used. Max is MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE
    // There is one PS2MemCardBaseDirectory for each save of this type.
    s32                 mMaxNumberOfSaves;
    
    // Information on the user file that is special to this particular save type.
    XBOXMUUserDataFile  mUserDataFileInfo;
};


//=====================================================================================================================================
// XBOX Resource File Layout.
//=====================================================================================================================================
// 
//-------------------------------------------------------------------------------------------------------------------------------------
//
// XBOXMUResourceHeader
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// XBOXMUResourceSaveGameType 0
// XBOXMUResourceSaveGameType 1
// XBOXMUResourceSaveGameType 2
// ..
// ..
// ..
// XBOXMUResourceSaveGameType X
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// XBOX Icon Data
//
//=====================================================================================================================================

#endif // XBOX_MEMORY_UNIT_RESOURCE_HPP
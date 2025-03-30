#ifndef PS2_MEMORY_UNIT_RESOURCE_HPP
#define PS2_MEMORY_UNIT_RESOURCE_HPP

//=====================================================================================================================================
// Required Includes.
//=====================================================================================================================================
#include "Q_MemoryUnitResource.hpp"

//=====================================================================================================================================
// Definitions.
//=====================================================================================================================================
#define PS2_USER_DATA_FILENAME_LENGTH           32
#define PS2_BASEDIR_NAME_LENGTH                 32

#define PS2_STORED_ICON_DATA_FILE               "icon.ico"
#define PS2_STORED_ICON_INFO_FILE               "icon.sys"
#define PS2_STORED_BROWSE_TEXT_FILE             "BrowseText"
#define PS2_STORED_DIRECTORY_NAME_SIZE          32

//=====================================================================================================================================
// PS2 Resource structures
//=====================================================================================================================================
struct PS2MemCardResourceHeader
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
struct PS2MemCardUserDataFile
{
    char    mUserDataSaveFileName[PS2_USER_DATA_FILENAME_LENGTH];   // The names of each user data file that is saved when using this save type.
    s32     mSizeOfUserDataFile;                                    // The exact number of bytes of user data stored within this file.
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PS2MemCardBaseDirectory
{
    char    mActualBaseDirName[PS2_BASEDIR_NAME_LENGTH];
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PS2MemCardResourceSaveGameType
{
    // How many saves of this type are going to be used. Max is MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE
    // There is one PS2MemCardBaseDirectory for each save of this type.
    s32                     mMaxNumberOfSaves;
    PS2MemCardBaseDirectory mBaseDirectories[MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE];
    
    // Information on the user file that is special to this particular save type.
    PS2MemCardUserDataFile  mUserDataFileInfo;
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PS2MemCardResourceIconInfo
{
    u32     mBackgroundTransparency;    // 0 = Transparent, 128 = Opaque.
    color   mBGUpperLeftColor;          // R, G, B Values, 0 is none, 256 is full.  A is not used.
    color   mBGUpperRightColor;         // R, G, B Values, 0 is none, 256 is full.  A is not used.
    color   mBGLowerLeftColor;          // R, G, B Values, 0 is none, 256 is full.  A is not used.
    color   mBGLowerRightColor;         // R, G, B Values, 0 is none, 256 is full.  A is not used.

    // Dynamic lighting settings to apply to the 3D icon in the PS2 Browser.
    vector3 mLight1_Dir;                // Assuming the icon is at the origin, the location of a point light.
    color   mLight1_Color;              // Color of that point light.

    vector3 mLight2_Dir;                // Assuming the icon is at the origin, the location of a point light.
    color   mLight2_Color;              // Color of that point light.
    
    vector3 mLight3_Dir;                // Assuming the icon is at the origin, the location of a point light.
    color   mLight3_Color;              // Color of that point light.
    
    color   mAmbient_Color;             // Overall hue to apply to the icon.
    
    s32     mIconDataSize;              // Size of that Icon Data.
};


//=====================================================================================================================================
// PS2 Resource File Layout.
//=====================================================================================================================================
// 
//-------------------------------------------------------------------------------------------------------------------------------------
//
// PS2MemCardResourceHeader
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// PS2MemCardResourceSaveType 0
// PS2MemCardResourceSaveType 1
// PS2MemCardResourceSaveType 2
// ..
// ..
// ..
// PS2MemCardResourceSaveType X
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// PS2MemCardResourceIconInfo
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// PS2 Icon Data
//
//=====================================================================================================================================


#endif // PS2_MEMORY_UNIT_RESOURCE_HPP
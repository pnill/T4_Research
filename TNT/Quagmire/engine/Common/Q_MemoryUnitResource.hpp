#ifndef Q_MEMORY_UNIT_RESOURCE_HPP
#define Q_MEMORY_UNIT_RESOURCE_HPP


//=====================================================================================================================================
// Definitions.
//=====================================================================================================================================
#define MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE             8

//-------------------------------------------------------------------------------------------------------------------------------------
enum QuagMemoryUnitResourceTypes
{
    QUAG_MEMORY_UNIT_RESOURCE_TYPE_PS2,
    QUAG_MEMORY_UNIT_RESOURCE_TYPE_GAMECUBE,
    QUAG_MEMORY_UNIT_RESOURCE_TYPE_XBOX,
    QUAG_MEMORY_UNIT_RESOURCE_TYPE_PC,
    
    TOTAL_QUAG_MEMORY_UNIT_RESOURCE_TYPES,
};



//=====================================================================================================================================
// Special includes files
// These platform specific includes are at the end of the file to remove compliler problems with undeclared references
// to MemoryUnit class for the specific consoles.
//=====================================================================================================================================
#if   defined( TARGET_PS2     )
    #include "PS2_MemoryUnitResource.hpp"
#elif defined( TARGET_DOLPHIN )
    #include "GC_MemoryUnitResource.hpp"
#elif defined( TARGET_XBOX    )
    #include "XBOX_MemoryUnitResource.hpp"
#elif defined( TARGET_PC      )
    #include "PC_MemoryUnitResource.hpp"
#endif



#endif // Q_MEMORY_UNIT_RESOURCE_HPP
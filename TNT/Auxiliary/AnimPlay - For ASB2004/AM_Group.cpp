///////////////////////////////////////////////////////////////////////////
//
//  AM_GROUP.c
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////
#include "x_files.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "x_stdio.hpp"
#include "x_debug.hpp"
#include "AM_Group.hpp"
#include "AM_Play.hpp"

#if defined( TARGET_DOLPHIN )
    #include "AUX_RAM.hpp"
#endif


///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define VERSION_SUPPORTED   7

#define MAX_ANIMGROUPS      3


///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

//---   animation group registry is used to register animation groups
struct SAnimGroupInstance
{
    u32             ID;
    u32             ReferenceCount;
    anim_group      AnimGroup;

#ifdef LOAD_ANIMATION_NAMES
    char*           pAnimNames;
#endif
};

///////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////

static SAnimGroupInstance   AnimGroups[MAX_ANIMGROUPS];
//static anim_group*          AM_DefAnimGroup;            // default anim group
static s32                  AM_NumCacheStaticBuffers;
static s32                  AM_NumCacheDynamicBuffers;

xbool       AM_Inited         = FALSE;

void        ANIM_InitAnimGroup      (anim_group* AnimGroup, char* FileName);
void        ANIM_KillAnimGroup      (anim_group* AnimGroup);

static void ANIM_ClearGroupRegistry( void );
static void ANIM_KillGroupRegistry ( void );

#ifdef LOAD_ANIMATION_NAMES
static SAnimGroupInstance* ANIM_GetGroupInstance( anim_group* AnimGroup );
static void ANIM_LoadAnimGroupNames( SAnimGroupInstance& rGroupInstance, char* FileName );
#endif

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// ANIM INIT KILL
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////

void ANIM_InitModule( s32 NumCacheStaticBuffers, s32 NumCacheDynamicBuffers )
{
    //--- if the module has been initialized, only reinitialize if the static or dynamic buffers have changed
    if( AM_Inited &&
        (AM_NumCacheStaticBuffers == NumCacheStaticBuffers) && 
        (AM_NumCacheDynamicBuffers == NumCacheDynamicBuffers) )
    {
        return;
    }

    //--- if it has been previously initialized, kill it before re-initializing
    if( AM_Inited )
        ANIM_KillModule();

    ASSERT( !AM_Inited );

    //--- clear the group registry
    ANIM_ClearGroupRegistry();

    //--- number of buffers used to decompress motion blocks.  Static buffers are
    //    pre-allocated and dynamic buffers are allocated on the fly as needed if
    //    the number of free static buffers runs out.
    AM_NumCacheStaticBuffers  = NumCacheStaticBuffers;
    AM_NumCacheDynamicBuffers = NumCacheDynamicBuffers;

    //--- must set the AM_Inited variable to TRUE before calling ANIM_UseAnimGroup
    //    or it will try to call ANIM_InitModule again
    AM_Inited = TRUE;

    //--- set the default anim group
//    AM_DefAnimGroup = ANIM_UseAnimGroup( FileName );

    //--- initialize the cache
//    AMCACHE_InitModule( AM_NumCacheStaticBufffers, AM_NumCacheDynamicBuffers, AM_DefAnimGroup->MaxStreams, AM_DefAnimGroup->MaxFramesPerBlock );
}

///////////////////////////////////////////////////////////////////////////

void ANIM_KillModule( void )
{
    ASSERT( AM_Inited );

    //--- must set the AM_Inited variable to FALSE before calling ANIM_KillGroupRegistry
    //    or it will try to call ANIM_KillModule again
    AM_Inited = FALSE;

    //--- kill the group registry
    ANIM_KillGroupRegistry();

    //--- kill the cache
    AMCACHE_KillModule();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// ANIM GROUP REGISTRY
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////

static void ANIM_ClearGroupRegistry( void )
{
    x_memset( AnimGroups, 0, sizeof(SAnimGroupInstance) * MAX_ANIMGROUPS );
}

///////////////////////////////////////////////////////////////////////////

static void ANIM_KillGroupRegistry( void )
{
    s32 i;

    for( i = 0; i < MAX_ANIMGROUPS; i++ )
        if( AnimGroups[i].ID )
            ANIM_KillAnimGroup( &AnimGroups[i].AnimGroup );

    ANIM_ClearGroupRegistry();

    //--- if the animation system is still inited, kill it now
    if( AM_Inited )
        ANIM_KillModule();
}

///////////////////////////////////////////////////////////////////////////

#ifdef LOAD_ANIMATION_NAMES

static SAnimGroupInstance* ANIM_GetGroupInstance( anim_group* AnimGroup )
{
    s32 i;

    for( i = 0; i < MAX_ANIMGROUPS; i++ )
    {
        if( AnimGroup == &AnimGroups[i].AnimGroup )
            return &AnimGroups[i];
    }

    return NULL;
}

#endif

///////////////////////////////////////////////////////////////////////////

static u32 ANIM_MakeGroupID( char* FileName )
{
    u32     ID;
    u32     Len;
    char    Name[256];
    char*   pName;
    u32     TempVal;
    s32     i;

    //--- process the string before calculating the ID.
    x_strncpy( Name, FileName, 255 );

    //--- remove any back slashes or forward slashes, they can mis-match for two strings
    //    which refer to the same file.
    for( i = 0; Name[i] && i <= 255; ++i )
    {
        if( ( Name[i] >= 'a' ) && ( Name[i] <= 'z' ) )
            Name[i] += 'A' - 'a';

        if( ( Name[i] == '\\' ) || ( Name[i] == '/' ) )
        {
            pName = &Name[i--];
            while( *pName )
            {
                pName[0] = pName[1];
                ++pName;
            }
        }
    }

    //--- compose a 24bit check some of the file name (saving 8 bits for the string length)
    pName   = Name;
    TempVal = 0;
    ID      = 0;
    while( 1 )
    {
        ID += TempVal;
        TempVal = 0;
        TempVal |= (((u32)*pName++) & 0xff) << 24;  if(!*pName) break;
        TempVal |= (((u32)*pName++) & 0xff) << 16;  if(!*pName) break;
        TempVal |= (((u32)*pName++) & 0xff) << 8;   if(!*pName) break;
    }
    ID += TempVal;

    //--- use the last 8 bits for the file length
    Len = ((u32)pName - (u32)Name) % 256;

    ID += Len;

    return ID;
}

///////////////////////////////////////////////////////////////////////////

//anim_group* ANIM_GetDefAnimGroup( void )
//{
//    return AM_DefAnimGroup;
//}

///////////////////////////////////////////////////////////////////////////

anim_group* ANIM_GetAnimGroup( mocap_player* pPlayer )
{
    ASSERT( pPlayer );

    return pPlayer->m_AnimGroup;
}

///////////////////////////////////////////////////////////////////////////

anim_group* ANIM_UseAnimGroup( char* FileName )
{
    u32 ID = ANIM_MakeGroupID( FileName );
    s32 i;
    s32 OpenSlot=-1;

    //--- the animation system must be initialized
    if( !AM_Inited )
        ANIM_InitModule();

    //--- look for the group to have already been initialized
    for( i = 0; i < MAX_ANIMGROUPS; i++ )
    {
        //--- if the group has been found, return it
        if( AnimGroups[i].ID == ID )
        {
            AnimGroups[i].ReferenceCount++;
            return &AnimGroups[i].AnimGroup;
        }

        //--- if no open slot has yet been found, and this is
        //    an open slot, store it to be used if this
        //    group has to be created
        if( (OpenSlot == -1) && (AnimGroups[i].ID == 0) )
            OpenSlot = i;
    }

    //--- if there is no room to add this group, return NULL
    if( OpenSlot == -1 )
        return NULL;

    //--- add this group to the registry
    ANIM_InitAnimGroup( &AnimGroups[OpenSlot].AnimGroup, FileName );

#ifdef LOAD_ANIMATION_NAMES
    //--- load the animation names if possible
    ANIM_LoadAnimGroupNames( AnimGroups[OpenSlot], FileName );
#endif

    //--- make sure there is enough room in the cache for this animation group
    AMCACHE_InitModule( AM_NumCacheStaticBuffers,
                        AM_NumCacheDynamicBuffers,
                        AnimGroups[OpenSlot].AnimGroup.MaxStreams,
                        AnimGroups[OpenSlot].AnimGroup.FramesPerBlock );

    AnimGroups[OpenSlot].ID = ID;
    AnimGroups[OpenSlot].ReferenceCount=1;

    return &AnimGroups[OpenSlot].AnimGroup;
}

///////////////////////////////////////////////////////////////////////////

void ANIM_DoneWithAnimGroup( char* FileName )
{
    ANIM_DoneWithAnimGroup( ANIM_MakeGroupID( FileName ) );
}

///////////////////////////////////////////////////////////////////////////

void ANIM_DoneWithAnimGroup( anim_group* pAnimGroup )
{
    s32 i;

    //--- look for the group to have already been initialized
    for( i = 0; i < MAX_ANIMGROUPS; i++ )
        if( pAnimGroup == &AnimGroups[i].AnimGroup )
            break;

    ANIM_DoneWithAnimGroup( AnimGroups[i].ID );
}

///////////////////////////////////////////////////////////////////////////

void ANIM_DoneWithAnimGroup( u32 ID )
{
    s32 i;

    //--- look for the group to have already been initialized
    for( i = 0; i < MAX_ANIMGROUPS; i++ )
        if( AnimGroups[i].ID == ID )
            break;

    //--- if there is no room to add this group, return NULL
    if( i == MAX_ANIMGROUPS )
        return;

    //--- decrement the reference count
    AnimGroups[i].ReferenceCount--;
    if( AnimGroups[i].ReferenceCount == 0 )
    {
        //--- remove this group
        ANIM_KillAnimGroup( &AnimGroups[i].AnimGroup );
        AnimGroups[i].ID = 0;
        x_memset( &AnimGroups[i].AnimGroup, 0, sizeof(anim_group) );

#ifdef LOAD_ANIMATION_NAMES
        //--- free the animation names if loaded
        x_free( AnimGroups[i].pAnimNames );
        AnimGroups[i].pAnimNames = NULL;
#endif

        //--- if this was the last group, kill the module
        for( i = 0; i < MAX_ANIMGROUPS; i++ )
            if( AnimGroups[i].ID != 0 )
                break;

        //--- if none were found, kill the module
        if( (i == MAX_ANIMGROUPS) && AM_Inited )
            ANIM_KillModule();
    }
}


///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void ANIM_InitAnimGroup( anim_group* AnimGroup, char* FileName )
{
    X_FILE*     fp;
    byte*       Raw;
    s32         i;

#ifdef ANIMPLAY_PRIMARY_STORAGE
    s32         FileSize;

    // Open file and grab AnimGroup structure
    fp = x_fopen( FileName, "rb" );
    ASSERT( fp );

    // Get file size
    x_fseek( fp, 0, X_SEEK_END );
    FileSize = x_ftell( fp );
    x_fseek( fp, 0, X_SEEK_SET );


#if defined( TARGET_DOLPHIN )

    s32   CompDataSize;
    void* pCompData;
    byte* pARAMData;
    s32   NBytesCopied;
    s32   NBytesToCopy;

    // Grab AnimGroup structure
    x_fread( AnimGroup, sizeof(anim_group), 1, fp );

    // Check version
    if( AnimGroup->Version != VERSION_SUPPORTED )
    {
        ASSERTS( FALSE, "Unsupported version of AnimGroup" );
    }

    // Do a malloc load of the info portion file
    Raw = (byte*)x_malloc( (s32)AnimGroup->AnimCompData - sizeof(anim_group) );
    ASSERT( Raw != NULL );
    x_fread( Raw, ((s32)AnimGroup->AnimCompData - sizeof(anim_group)), 1, fp );

    // Load anim data into temp main ram buffer, then transfer to ARAM in 2 MB chunks
    #define ANIMDATA_BUFFER_SIZE    (2 * 1024*1024)

    CompDataSize            = FileSize - x_ftell( fp );
    pCompData               = x_malloc( ANIMDATA_BUFFER_SIZE );
    AnimGroup->AnimCompData = AUXRAM_malloc( AUXRAM_ROUNDUP32(CompDataSize) );

    ASSERT( pCompData != NULL );
    ASSERT( AnimGroup->AnimCompData != NULL );

    // Read in chunks of anim data to buffer, send to ARAM, and repeat until data is loaded
    pARAMData    = (byte*)(AnimGroup->AnimCompData);
    NBytesCopied = 0;

    while( NBytesCopied < CompDataSize )
    {
        NBytesToCopy = MIN( (CompDataSize - NBytesCopied), ANIMDATA_BUFFER_SIZE );

        x_fread( pCompData, sizeof(byte), NBytesToCopy, fp );
        AUXRAM_SendToAram( pCompData, pARAMData, AUXRAM_ROUNDUP32(NBytesToCopy) );

        NBytesCopied += NBytesToCopy;
        pARAMData    += NBytesToCopy;
    }

    // Free temp MainRam buffer
    x_free( pCompData );

    // Close file
    x_fclose( fp );

    // Relink ptrs to data
    AnimGroup->AnimBlockSize = (u8*)         (((s32)AnimGroup->AnimBlockSize - sizeof(anim_group)) + ((u32)Raw));
    AnimGroup->AnimEvent     = (anim_event*) (((s32)AnimGroup->AnimEvent     - sizeof(anim_group)) + ((u32)Raw));
    AnimGroup->AnimInfo      = (anim_info*)  (((s32)AnimGroup->AnimInfo      - sizeof(anim_group)) + ((u32)Raw));
    AnimGroup->Bone          = (anim_bone*)  (((s32)AnimGroup->Bone          - sizeof(anim_group)) + ((u32)Raw));

    // Setup anim IDs
    for( i = 0; i < AnimGroup->NAnimations; i++ )
        AnimGroup->AnimInfo[i].ID = (s16)i;

#else // not defined( TARGET_DOLPHIN )

    // Malloc monster data for animation data
    Raw = (byte*)x_malloc( FileSize );
    ASSERT( Raw );

    // Read in entire file
    x_fread( Raw, FileSize, 1, fp );

    // Close file
    x_fclose( fp );

    // Setup AnimGroup info
    *AnimGroup = *((anim_group*)Raw);

    // Check version
    if( AnimGroup->Version != VERSION_SUPPORTED )
    {
        ASSERTS( FALSE, "Unsupported version of AnimGroup" );
    }

    // Relink ptrs to data
    AnimGroup->AnimBlockSize = (u8*)         (((s32)AnimGroup->AnimBlockSize ) + ((u32)Raw));
    AnimGroup->AnimEvent     = (anim_event*) (((s32)AnimGroup->AnimEvent     ) + ((u32)Raw));
    AnimGroup->AnimInfo      = (anim_info*)  (((s32)AnimGroup->AnimInfo      ) + ((u32)Raw));
    AnimGroup->Bone          = (anim_bone*)  (((s32)AnimGroup->Bone          ) + ((u32)Raw));
    AnimGroup->AnimCompData  = (void*)       (((s32)AnimGroup->AnimCompData  ) + ((u32)Raw));

    // Setup anim IDs
    for( i = 0; i < AnimGroup->NAnimations; i++ )
        AnimGroup->AnimInfo[i].ID = (s16)i;

#endif // defined( TARGET_DOLPHIN )

#else

    // Open file and grab AnimGroup structure
    fp = x_fopen( FileName, "rb" );
    ASSERT( fp );
    x_fread( AnimGroup, sizeof(anim_group), 1, fp );

    // Check version
    if( AnimGroup->Version != VERSION_SUPPORTED )
    {
        ASSERTS( FALSE, "Unsupported version of AnimGroup" );
    }

    // Do a malloc load of the info portion file
    Raw = (byte*)x_malloc( (s32)AnimGroup->AnimCompData - sizeof(anim_group) );
    ASSERT( Raw );
    x_fread( Raw, ((s32)AnimGroup->AnimCompData - sizeof(anim_group)), 1, fp );

    // Relink ptrs to data
    AnimGroup->AnimBlockSize = (u8*)         (((s32)AnimGroup->AnimBlockSize - sizeof(anim_group)) + ((u32)Raw));
    AnimGroup->AnimEvent     = (anim_event*) (((s32)AnimGroup->AnimEvent     - sizeof(anim_group)) + ((u32)Raw));
    AnimGroup->AnimInfo      = (anim_info*)  (((s32)AnimGroup->AnimInfo      - sizeof(anim_group)) + ((u32)Raw));
    AnimGroup->Bone          = (anim_bone*)  (((s32)AnimGroup->Bone          - sizeof(anim_group)) + ((u32)Raw));

    // Increment animation offsets to point from beginning of file
    for( i = 0; i < AnimGroup->NAnimations; i++ )
    {
        AnimGroup->AnimInfo[i].ID = (s16)i;
        AnimGroup->AnimInfo[i].BlockOffset += (u32)AnimGroup->AnimCompData;
    }

    // Turn CompData offset into fileptr
    AnimGroup->AnimCompData  = (void*)fp;

#endif
}

///////////////////////////////////////////////////////////////////////////

void ANIM_KillAnimGroup( anim_group* AnimGroup )
{
    ASSERT( AnimGroup != NULL );

#ifdef ANIMPLAY_PRIMARY_STORAGE

  #if defined( TARGET_DOLPHIN )
    x_free( AnimGroup->AnimBlockSize );
    AUXRAM_free( AnimGroup->AnimCompData );
  #else
    x_free( (void*)(((u32)AnimGroup->AnimBlockSize) - sizeof(anim_group)) );
  #endif // defined( TARGET_DOLPHIN )

#else
    x_free( AnimGroup->AnimBlockSize );
    x_fclose( (X_FILE *)AnimGroup->AnimCompData );
#endif

}


////////////////////////////////////////////////////////////////////////////

#ifdef LOAD_ANIMATION_NAMES

static void GetS( X_FILE* fp, char* ReadBuff, s32 ReadBuffSize, s32& rReadPos, s32& rReadLen, char* S, s32 Len )
{
    s32  i;
    char c;

    i = 0;
    while( i < Len )
    {
        //--- if the buffer is dry, re-fill it
        if( rReadPos >= rReadLen )
        {
            if( x_feof(fp) )
                break;

            //--- attempt to read the next character
            s32 SizeRead = x_fread( ReadBuff, 1, ReadBuffSize, fp );
            rReadLen = SizeRead;
            rReadPos = 0;
        }

        //--- get the next character
        c = ReadBuff[ rReadPos++ ];

        //--- if the character is a carriage return, the end of the line
        //    has been hit, break the loop.
        if( c == 0xD )
            break;

        //--- skip any line feed characters (usually sitting after CR characters)
        if( c == 0xA )
            continue;

        //--- store the character
        S[ i++ ] = c;
    }

    //--- cap it off
    S[ (i==Len) ? Len-1 : i ] = '\0';
}

////////////////////////////////////////////////////////////////////////////

#define MAX_ANIMNAME_LENGTH     30

static void ANIM_LoadAnimGroupNames( SAnimGroupInstance& rGroupInstance, char* FileName )
{
    char    HeaderFile[256];
    s32     StrLen = x_strlen( FileName );

    //--- set the names to NULL to start with
    rGroupInstance.pAnimNames = NULL;

    //--- if the file path is too long, exit now
    if( (StrLen > 255) || (StrLen < 3) )
        return;

    //--- make a copy of the file
    x_strncpy( HeaderFile, FileName, 255 );

    //--- change the extention to a header file extention
    s32 i = StrLen - 1;
    if     ( HeaderFile[i] == '.' )     HeaderFile[i]   = '\0';
    else if( HeaderFile[i-1] == '.' )   HeaderFile[i-1] = '\0';
    else if( HeaderFile[i-2] == '.' )   HeaderFile[i-2] = '\0';
    else if( HeaderFile[i-3] == '.' )   HeaderFile[i-3] = '\0';
    x_strcat( HeaderFile, ".h" );

    //--- attempt to open the header file
    X_FILE* fp;
    fp = x_fopen( HeaderFile, "rt" );

    //--- if the attempt failed, exit now
    if( !fp )
        return;

    //--- allocate a buffer large enough to hold the animation names
    s32   BuffSize = rGroupInstance.AnimGroup.NAnimations * MAX_ANIMNAME_LENGTH;
    char* NameBuff = (char*)x_malloc( BuffSize );

    //--- if the buffer wasn't allocated, close the file and exit
    if( NameBuff == NULL )
    {
        x_fclose( fp );
        return;
    }

    //--- scan in the anim names
    s32     CurAnim = 0;
    char    Line[256];
    char*   pNameBuff;
    char    ReadBuff[1024];
    s32     ReadPos = 0;
    s32     ReadLen = 0;
    s32     c;

    //--- skip to the first define
    Line[0] = 0;
    while( Line[0] != '#' )
        GetS( fp, ReadBuff, 1024, ReadPos, ReadLen, Line, 256 );

    do
    {
        if( Line[0] == '#' )
        {
            i = 11; // past the "#define ID_" portion
            c = 0;
            pNameBuff = &NameBuff[ CurAnim*MAX_ANIMNAME_LENGTH ];

            while( (c < (MAX_ANIMNAME_LENGTH-1)) && (i < 255) && (Line[i] != ' ') )
                pNameBuff[c++] = Line[i++];

            pNameBuff[c++] = '\0';

            //--- as long as we are at the end, lets make sure that it wasn't exported with a -1
            //    if it was, then we will not want to use it in the list
            if( Line[i+1] != '-' )
                CurAnim++;
        }

        GetS( fp, ReadBuff, 1024, ReadPos, ReadLen, Line, 256 );
    }
    while( (CurAnim != rGroupInstance.AnimGroup.NAnimations) && !x_feof( fp ) )
    {   //empty loop
    }

    //--- close the file
    x_fclose( fp );

    //--- store the name buffer
    rGroupInstance.pAnimNames = NameBuff;
}

////////////////////////////////////////////////////////////////////////////

char* ANIM_GetAnimName( anim_group* pAnimGroup, s32 AnimID )
{
    ASSERT( ( AnimID >= 0 ) && ( AnimID < pAnimGroup->NAnimations ) );

    //--- get the group instance
    SAnimGroupInstance* pGroupInstance = ANIM_GetGroupInstance( pAnimGroup );

    //--- if the names weren't loaded for this group, return now
    if( pGroupInstance->pAnimNames == NULL )
        return "";

    //--- return the name of this animation
    return &pGroupInstance->pAnimNames[ AnimID * MAX_ANIMNAME_LENGTH ];
}

#else

////////////////////////////////////////////////////////////////////////////

char* ANIM_GetAnimName( anim_group* /*pAnimGroup*/, s32 /*AnimID*/ )
{
    return "";
}

#endif

///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////

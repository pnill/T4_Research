////////////////////////////////////////////////////////////////////////////
//
//  PS2 CD Utility
//
//  Features a much-imprived replacement for the Sony sceCdSearch function.
//  The Sony function will only scan the first sector of a directory, limiting
//  the layout to 30 files per directory.
//
//  This utility package follows the ISO-9660 format for structure, except
//  where Sony has made modifications, such as the forced 8.3 naming style.
//
//  During initialization, temporary memory required will be 
//  PS2CDU_DIRECTORY_BUFFER_SIZE * 2048 bytes.  After initialization, this
//  storage is released.  Permanent memory requirements will vary depending
//  on how many directories and files are found on the CD :
//
//  sizeof( PS2CDUDir    ) * NumDirectories +
//  sizeof( PS2CDURecord ) * NumFiles
//
//  The root directory also counts as a directory entry. "." and ".." are not
//  included.
//
////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"
#include "Q_Engine.hpp"

#include <eetypes.h>
#include <eekernel.h>
#include <stdio.h>
#include <string.h>
#include <sifdev.h>
#include <libcdvd.h>
#include "PS2_CDUtil.hpp"


////////////////////////////////////////////////////////////////////////////

// Debug compile-time switches
//#define PS2CDUTIL_DEBUG

//--------------------------------------------------------------------------

#if defined( PS2CD_DEBUG ) && defined( X_DEBUG )
    #define DBG_EXP( exp )    exp
#else
    #define DBG_EXP( exp )
#endif


////////////////////////////////////////////////////////////////////////////
//
// # of sectors to read into the internal buffer.
// This value can be changed if you know the range of sectors
// occupied by the directory description data.
// The first sector will always be sector 22.
// Typically, you can find out where the directory table ends by
// looking in the burner software.  The Sony CD/DVD-Gen software has
// a 'Layout' view that shows how files are positioned.  The
// entries at the top are system and directory sectors. 
//
//

#ifdef TARGET_PS2_DEMO
    #define PS2CDU_DIRECTORY_BUFFER_SIZE            160
#else
    #define PS2CDU_DIRECTORY_BUFFER_SIZE            900
#endif


////////////////////////////////////////////////////////////////////////////
//
//  ISO 9660 DIRECTORY RECORD FORMAT
//
//
//      length
//     in bytes  contents
//     --------  ---------------------------------------------------------
//        1      R, the number of bytes in the record (which must be even)
//        1      0 [number of sectors in extended attribute record]
//        8      number of the first sector of file data or directory
//                 (zero for an empty file), as a both endian double word
//        8      number of bytes of file data or length of directory,
//                 excluding the extended attribute record,
//                 as a both endian double word
//        1      number of years since 1900
//        1      month, where 1=January, 2=February, etc.
//        1      day of month, in the range from 1 to 31
//        1      hour, in the range from 0 to 23
//        1      minute, in the range from 0 to 59
//        1      second, in the range from 0 to 59
//                 (for DOS this is always an even number)
//        1      offset from Greenwich Mean Time, in 15-minute intervals,
//                 as a twos complement signed number, positive for time
//                 zones east of Greenwich, and negative for time zones
//                 west of Greenwich
//        1      flags, with bits as follows:
//                 bit     value
//                 ------  ------------------------------------------
//                 0 (LS)  0 for a norma1 file, 1 for a hidden file
//                 1       0 for a file, 1 for a directory
//                 2       0 [1 for an associated file]
//                 3       0 [1 for record format specified]
//                 4       0 [1 for permissions specified]
//                 5       0
//                 6       0
//                 7 (MS)  0 [1 if not the final record for the file]
//        1      0 [file unit size for an interleaved file]
//        1      0 [interleave gap size for an interleaved file]
//        4      1, as a both endian word [volume sequence number]
//        1      N, the identifier length
//        N      identifier
//        P      padding byte: if N is even, P = 1byte and this field contains
//                 a zero; if N is odd, P = 0bytes and this field is omitted
//    R-33-N-P   unspecified field for system use; must contain an even
//                 number of bytes ('R' is first byte, see top)
//
//
////////////////////////////////////////////////////////////////////////////
//  STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct Iso9660DirRecordHeader
{
    u8          RecordLength;
    u8          NumEARSectors;
    s32         StartSector;
    s32         Pad1;
    s32         DataSizeInBytes;
    s32         Pad2;
    u8          Year;
    u8          Month;
    u8          Day;
    u8          Hour;
    u8          Minute;
    u8          Second;
    u8          GMTOffset;
    u8          Flags;
    u8          Pad3;
    u8          Pad4;
    s32         VolSeqNum;
    u8          NameLength;
    char        Name[12];
};


struct PS2CDURecord
{
    char        Name[20];           // Only need 15 so the rest is just struct padding
    s32         LSN;                // Logical sector number
    s32         Size;               // Size in bytes
    s32         SubDirIdx;          // Index into subdir array if valid
};

struct PS2CDUDir
{
    s32             NRecords;       // # Records in directory
    PS2CDURecord*   Rec;            // Dyn-array of records (ptr into Files array)
};

struct PS2CDUDList
{
    s32     LSN;                    // Logical sector number
    s32     Size;                   // Size 
};


////////////////////////////////////////////////////////////////////////////
//
//  STATIC LOCAL VARIABLES
//
////////////////////////////////////////////////////////////////////////////
//
//  Read Buffers
//
static char Buffer[2048] __attribute__ ((aligned (64)));  // CD sector buffer
static char Sec16[2048]  __attribute__ ((aligned (64)));  // Copy of sector 16 (PVD)

//
//  Settings
//
static sceCdRMode       CDmode;             // Mode settings for CD reading

//
// Directory tree containers
//
static PS2CDUDir*       Dirs        = NULL;
static PS2CDURecord*    Files       = NULL;

//
// Vars used temporarily when constructing the directory tree
//
static s32              ListMax;
static s32              ListHead;
static s32              ListTail;
static s32              LastFile;
static PS2CDUDList*     pDirList;
static s32              MaxSec = 0;
static char*            TempDiscImage;
static s32              MemReq;
static s32              NDirs;
static s32              NFiles;

////////////////////////////////////////////////////////////////////////////
//
//  SHARED GLOBAL VARIABLES - referenced by PS2_CD.cpp
//
////////////////////////////////////////////////////////////////////////////

xbool                   gPS2CDUtilReady = FALSE;


//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//
//  FUNCTIONS
//
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================



////////////////////////////////////////////////////////////////////////////

static
s32 MakeS32(s32 Start, char* Buffer)
{
    return(  ((s32)(Buffer[Start  ])&0xFF)      |
            (((s32)(Buffer[Start+1])&0xFF)<<8)  |
            (((s32)(Buffer[Start+2])&0xFF)<<16) | 
            (((s32)(Buffer[Start+3])&0xFF)<<24) );
}

////////////////////////////////////////////////////////////////////////////

static
void BuildHeaderFromBuffer( Iso9660DirRecordHeader* H, char* Buf )
{
    H->RecordLength    = Buf[0];
    H->NumEARSectors   = Buf[1];
    H->StartSector     = MakeS32( 2, Buf );
    H->DataSizeInBytes = MakeS32( 10, Buf );
    H->Year            = Buf[18];
    H->Month           = Buf[19];
    H->Day             = Buf[20];
    H->Hour            = Buf[21];
    H->Minute          = Buf[22];
    H->Second          = Buf[23];
    H->GMTOffset       = Buf[24];
    H->Flags           = Buf[25];
    H->VolSeqNum       = MakeS32( 28, Buf );
    H->NameLength      = Buf[32];

    s32     i;
    for (i=0;i<H->NameLength;i++)
    {
        H->Name[i] = Buf[33+i];
    }
    H->Name[H->NameLength] = 0;
}

////////////////////////////////////////////////////////////////////////////

static
void ReadHard(s32 NSectors, s32 LSN, char* Buf)
{
    s32             Result = 0;
    static s32      RetryCount = 0;

    if (LSN > MaxSec)
        MaxSec = LSN;

    do
    {
        RetryCount++;
        ASSERTS(RetryCount<5,"ERROR READING FROM DISC");

        Result = sceCdRead(LSN,NSectors,(void *)Buf,&CDmode);
        sceCdSync(0);
    }
    while (SCECdErNO != sceCdGetError());

    RetryCount = 0;
}

////////////////////////////////////////////////////////////////////////////

static
void Read(s32 NSectors, s32 LSN, char* Buf)
{
	char	strNumFiles[64];

	x_sprintf(strNumFiles, "Increase the file number to %d in PS2_CDUtil.cpp", LSN);

    if (LSN > MaxSec)
        MaxSec = LSN;

    LSN -= 0x10;
    ASSERTS(LSN < PS2CDU_DIRECTORY_BUFFER_SIZE,strNumFiles);
    memcpy( Buf, &TempDiscImage[LSN*2048], NSectors*2048 );
}

////////////////////////////////////////////////////////////////////////////

static
void PreviewDirStructure( s32 StartSec, s32 NSectors )
{
    s32     Sec;

    // Read in the records
    for ( Sec = 0; Sec < NSectors; Sec++ )
    {
        Read( 1, StartSec + Sec, Buffer );

        s32     Idx = 0;

        while (Buffer[Idx])
        {
            Iso9660DirRecordHeader      H;

            BuildHeaderFromBuffer( &H, &Buffer[Idx] );

            // Only recurse if the entry is a directory, and it is
            // not the "." or ".." directories.
            if  (!(((H.NameLength == 1) && (H.Name[0] == 0)) ||
                ((H.NameLength == 1) && (H.Name[0] == 1))))
            {
                if (H.Flags & 0x02)
                {
                    NDirs++;

                    pDirList[ListTail].LSN = H.StartSector;
                    pDirList[ListTail].Size = H.DataSizeInBytes;

                    ListTail ++;
                    if (ListTail == ListMax )
                    {
                        // Grow the list;
                        ListMax += 128;
                        pDirList = (PS2CDUDList*)x_realloc( pDirList, sizeof(PS2CDUDList) * ListMax );
                        ASSERT(pDirList);
                    }
                }
                else
                {
                    NFiles++;
                }
            }
            //////////

            Idx += H.RecordLength;
        }
    }

    if (ListHead < ListTail)
    {
        ListHead++;

#ifdef TARGET_PS2_DEMO
        PreviewDirStructure( pDirList[ListHead-1].LSN, pDirList[ListHead-1].Size/2048 );
#else
        PreviewDirStructure( pDirList[ListHead-1].LSN, (pDirList[ListHead-1].Size +2047)/2048 );
#endif  //cd return sizes in sectors, dvd return sizes in absolute
    }

}

////////////////////////////////////////////////////////////////////////////

void ScanDirectory( s32 CurDir, s32 StartSec, s32 NSectors )
{
    s32     Sec;
    s32     NRecords = 0;
    s32     Idx = 0;

    ASSERT(CurDir < NDirs);

    // Count records
    for ( Sec = 0; Sec < NSectors; Sec++ )
    {
        Read( 1, StartSec + Sec, Buffer );
        Idx=0;

        while (Buffer[Idx])
        {
            Iso9660DirRecordHeader      H;
            BuildHeaderFromBuffer( &H, &Buffer[Idx] );

            if ( ((H.NameLength == 1) && (H.Name[0] == 0)) || 
                 ((H.NameLength == 1) && (H.Name[0] == 1))  )
            {
            }
            else
            {
                NRecords++;
            }

            Idx += H.RecordLength;
        }
    }

    PS2CDUDir*  DirEntry = &Dirs[CurDir];

    // Allocate storage
    DirEntry->NRecords = NRecords;
    DirEntry->Rec      = &(Files[LastFile]);
    LastFile += NRecords;

    s32     i=0;

    // Read in the records
    for ( Sec = 0; Sec < NSectors; Sec++ )
    {
        Read( 1, StartSec + Sec, Buffer );
        
        s32     Idx = 0;

        while (Buffer[Idx])
        {
            Iso9660DirRecordHeader      H;
            char                        Name[13];
            xbool                       Add = TRUE;

            BuildHeaderFromBuffer( &H, &Buffer[Idx] );

            if ( (H.NameLength == 1) && (H.Name[0] == 0) )
            {
                strcpy(Name,".");
                Add=FALSE;
            }
            else if ((H.NameLength == 1) && (H.Name[0] == 1) )
            {
                strcpy(Name,"..");
                Add=FALSE;
            }
            else
            {
                s32     i;
                for (i=0;i<H.NameLength;i++)
                {
                    Name[i] = H.Name[i];
                }
                Name[H.NameLength] = 0;
            }

           //DBG_EXP( x_printf("%16s = LSN[%ld] %d/%d/%d %d:%d:%d\n",Name,H.StartSector,H.Day,H.Month,H.Year+1900,H.Hour,H.Minute,H.Second) );

            if (Add)
            {
                DirEntry->Rec[i].LSN = H.StartSector;
                DirEntry->Rec[i].Size = H.DataSizeInBytes;
                if (H.Flags & 0x02)
                {
                    DirEntry->Rec[i].SubDirIdx = ListTail+1;
                    ListTail++;
                }
                else
                {
                    DirEntry->Rec[i].SubDirIdx = -1;
                }

                strcpy( DirEntry->Rec[i].Name, Name );

                i++;
            }

            //////////

            Idx += H.RecordLength;
        }
    }

    for (i=0;i<NRecords;i++)
    {
        if (DirEntry->Rec[i].SubDirIdx > 0)
        {
#ifdef TARGET_PS2_DEMO
            ScanDirectory( DirEntry->Rec[i].SubDirIdx, DirEntry->Rec[i].LSN, DirEntry->Rec[i].Size/2048 );
#else
            ScanDirectory( DirEntry->Rec[i].SubDirIdx, DirEntry->Rec[i].LSN, (DirEntry->Rec[i].Size + 2047)/2048 );
#endif  //cd return sizes in sectors, dvd return sizes in absolute
        }
    }
}

////////////////////////////////////////////////////////////////////////////

xbool ScanDisc( void )
{
    TempDiscImage = (char*)x_malloc( PS2CDU_DIRECTORY_BUFFER_SIZE * 2048 );
    ASSERT( TempDiscImage );
    ReadHard( PS2CDU_DIRECTORY_BUFFER_SIZE,0x10, TempDiscImage );

    // Scan the disc and perform validity checking, as well as read in the
    // PVD (Primay Volume Descriptor)
    Read(1,0x10,Sec16);

    // Check to see if the disc is ISO
    if (x_strncmp(&Sec16[1],"CD001",5) != 0)
    {
        DBG_EXP( x_printf("PS2CDU : Disc is not an ISO image\n") );
        return FALSE;
    }

    // Building the file structure records requires 2 passes
    // 1st pass determines how much memory will be required
    // 2nd pass allocates the memory block, fills it with the filename
    //     strings, and sets up the filename pointer array.
    s32     RootDirSector;
    s32     RootDirSize;
    s32     NDirSectors;
    s32     CurSector;

    RootDirSector = MakeS32(158,Sec16);
    RootDirSize   = MakeS32(166,Sec16);

#ifdef TARGET_PS2_DEMO
    NDirSectors   = RootDirSize/ 2048;
#else
    NDirSectors   = (RootDirSize + 2047)/ 2048;
#endif  //cd return sizes in sectors, dvd return sizes in absolute

    CurSector     = RootDirSector;

    NDirs  = 1;         // +1 for root directory
    NFiles = 0;

    // Need to do this with malloc because there is a chace we will be
    // reallocing
    ListMax  = 1024;
    ListHead = 0;
    ListTail = 0;
    LastFile = 0;
    pDirList = (PS2CDUDList*)x_malloc( sizeof(PS2CDUDList) * ListMax );

    PreviewDirStructure( RootDirSector, NDirSectors );

    x_printf("NDirs    = %ld\n",NDirs);
    x_printf("NFiles   = %ld\n",NFiles);
    NFiles += NDirs;
    x_printf("NRecords = %ld\n",NFiles);

    Dirs  = new PS2CDUDir   [ NDirs+1 ];
    Files = new PS2CDURecord[ NFiles  ];

    ASSERT(Dirs);
    ASSERT(Files);

    MemReq = sizeof(PS2CDUDir)*(NDirs+1);
    MemReq += sizeof(PS2CDURecord)*NFiles;

    ListTail = 0;

    ScanDirectory( 0, RootDirSector, NDirSectors );

    x_free( pDirList );
    x_free(TempDiscImage);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


void PS2_CDUtilInit(void)
{
    CDmode.trycount = 5;
    CDmode.spindlctrl = SCECdSpinMax;
    CDmode.datapattern = SCECdSecS2048;

    if (Dirs)
        delete[] Dirs;
    if (Files)
        delete[] Files;

    MemReq = 0;

    ScanDisc();

    DBG_EXP( x_printf("LARGEST DFD SEC = %ld\n",MaxSec) );
    DBG_EXP( x_printf("MEM = %ld\n",MemReq) );

    // Flag the directory tree as usable
    gPS2CDUtilReady = TRUE;
    return;
}

//==========================================================================

void PS2_CDUtilKill( void )
{
    if (Dirs)
        delete[] Dirs;
    if (Files)
        delete[] Files;

    gPS2CDUtilReady = FALSE;
}

//==========================================================================

void PS2CD_PrintDirToScreen( s32 ID, s32 First )
{
    if (!gPS2CDUtilReady)
        return;

    PS2CDUDir*  D = &Dirs[ID];

    s32     i;
    for (i=First;i<D->NRecords;i++)
    {
        char*   Tag="-F-";

        if (D->Rec[i].SubDirIdx>=0)
            Tag="DIR";

        DBG_EXP( x_printfxy(1,2+i-First,"%s %14s %07ld %09ld",Tag,D->Rec[i].Name,D->Rec[i].LSN,D->Rec[i].Size) );
    }
}

//==========================================================================

s32 PS2CD_GetDirID( s32 ID, s32 Record )
{
    if (!gPS2CDUtilReady)
        return -1;

    PS2CDUDir*  D = &Dirs[ID];

    if (D->Rec[Record].SubDirIdx >= 0)
        return D->Rec[Record].SubDirIdx;

    return -1;
};

//==========================================================================

static
xbool IsFileInDir( PS2CDUDir* D, const char* Name, PS2CDURecord** File, s32* SubDir )
{
    s32     i;

    for (i=0;i<D->NRecords;i++)
    {
        if (x_strcmp(D->Rec[i].Name,Name)==0)
        {
            // Fill out dir info if Name is a directory
            if (D->Rec[i].SubDirIdx>=0)
            {
                if (SubDir)
                    *SubDir = D->Rec[i].SubDirIdx;
                if (File)
                    *File = NULL;
                return TRUE;
            }
            else
            {
                if (SubDir)
                    *SubDir = -1;
                if (File)
                    *File = &D->Rec[i];
                return TRUE;
            }
        }
    }
    return FALSE;
}

//==========================================================================

s32 PS2CD_SearchFile( sceCdlFILE* pFD, const char* File )
{
    ASSERT(pFD);
    ASSERT(File);

    if (!gPS2CDUtilReady)
        return 0;

    char            Name[16];
    PS2CDUDir*      D;
    PS2CDURecord*   FileRec;
    char            Filename[1024];
    char*           NamePtr = Filename;

    D = &Dirs[0];

    x_strcpy( Filename, File );
    x_strtoupper( Filename );

    DBG_EXP( x_printf("PS2CD_SearchFile( \"%s\" )",Filename) );

    while (*NamePtr)
    {
        char*   N;
        char*   WorkPtr;
        s32     Len = 0;

        N = Name;
        N[0] = '\0';
        WorkPtr=&N[0];
        NamePtr++;

        while(*NamePtr != '\\' && *NamePtr != '\0')
        {
            *WorkPtr++ = *NamePtr++;
            Len++;
        }

        *WorkPtr = '\0';

        if(*NamePtr=='\0')
        {
            if (Name[Len-2]!=';' || Name[Len-1]!='1')
            {
                x_strcat( Name,";1" );
            }
            // find file in root directory
            if(IsFileInDir(D,Name,&FileRec,NULL) )
            {
                pFD->lsn  = FileRec->LSN;
                pFD->size = FileRec->Size;
                DBG_EXP( x_printf("Found LSN[%09ld] Sz[%09ld]\n",pFD->lsn, pFD->size) );
                return 1;
            }
            else
            {
                DBG_EXP( x_printf("NOT FOUND\n") );
                return 0;
            }
        }
        else
        {
            // check that directory exists and use it's ID in the next loop
            s32 Idx;
            if( !IsFileInDir(D,Name,NULL,&Idx) )
            {
                DBG_EXP( x_printf("NOT FOUND\n") );
                return 0;
            }
            D = &Dirs[Idx];
        }
    }

    DBG_EXP( x_printf("NOT FOUND\n") );
    return 0;
}

//==========================================================================

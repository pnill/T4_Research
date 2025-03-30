////////////////////////////////////////////////////////////////////////////
//
// MemCardProject.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef MEMCARDPROJECT_H_INCLUDED
#define MEMCARDPROJECT_H_INCLUDED


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define MCW_CUR_VERSION		"MCW_PROJ_VER_3"


enum MEMCARD_TYPE
{
    MEMCARD_TYPE_PS2,
    MEMCARD_TYPE_GAMECUBE,
    MEMCARD_TYPE_XBOX,
    MEMCARD_TYPE_PC,

    MEMCARD_TYPE_COUNT,
};


// PS2 Data Constants
#define PS2_MIN_BASEDIRS			1
#define PS2_MAX_BASEDIRS			8
#define PS2_BASEDIR_NAME_SIZE		32
#define PS2_FILETYPE_NAME_SIZE		32
#define PS2_FILETYPE_SIZE_MAX_CHARS	7

// GC Data Constants
#define GC_MIN_FILES				1
#define GC_MAX_FILES				8
#define GC_FILETYPE_NAME_SIZE		32
#define GC_FILETYPE_SIZE_MAX_CHARS	7

#define GC_ICON_ANIMSPEED_FAST		1
#define GC_ICON_ANIMSPEED_NORMAL	2 
#define GC_ICON_ANIMSPEED_SLOW		3
#define GC_ICON_NFRAMES_MIN			1
#define GC_ICON_NFRAMES_MAX			6   // According to the gamecube, it can support up to 8, but we limit it to 6 frames
                                        // for better alignment in the save games.

// XBOX Data Constants
#define XBOX_MIN_FILES					1
#define XBOX_MAX_FILES					8
#define XBOX_FILETYPE_NAME_SIZE			32
#define XBOX_FILETYPE_SIZE_MAX_CHARS	7

// PC Data Constants
#define PC_MIN_FILES					1
#define PC_MAX_FILES					8
#define PC_FILETYPE_NAME_SIZE			32
#define PC_FILETYPE_SIZE_MAX_CHARS	7

// MemCardWiz Project Constants
#define MCW_PS2_GAMEID_NAME_SIZE		16
#define MCW_PS2_GAMEID_NAME_SIZE_USED	12
#define MCW_PS2_BASEDIR_NAME_SIZE		7

#define MCW_ICON_PATH_SIZE				256


#define MCW_HEADER_STR_SIZE				16
#define MCW_MAX_SAVETYPES				8
#define MCW_SAVETYPE_NAME_SIZE			32


////////////////////////////////////////////////////////////////////////////
// PS2 EXPORT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

struct MC_PS2Header
{
    long			m_MCardType;
    long			m_NSaveTypes;
    long			m_SaveTypeOffset;
    long			m_IconInfoOffset;
    long			m_IconDataOffset;
    long			m_TotalFileSize;

	void Init( void );
};

//--------------------------------------------------------------------------

struct MC_PS2FileType
{
	long			m_MaxSaves;
	char			m_BaseDirs[PS2_MAX_BASEDIRS][PS2_BASEDIR_NAME_SIZE];
	char			m_Filename[PS2_FILETYPE_NAME_SIZE];
	long			m_FileSize;

	void Init( void );
};

//--------------------------------------------------------------------------

struct MC_PS2IconInfo
{
	long			m_BK_Alpha;

	unsigned char	m_BK_TL_R;
	unsigned char	m_BK_TL_G;
	unsigned char	m_BK_TL_B;
	unsigned char	m_BK_TL_A;

	unsigned char	m_BK_TR_R;
	unsigned char	m_BK_TR_G;
	unsigned char	m_BK_TR_B;
	unsigned char	m_BK_TR_A;

	unsigned char	m_BK_BL_R;
	unsigned char	m_BK_BL_G;
	unsigned char	m_BK_BL_B;
	unsigned char	m_BK_BL_A;

	unsigned char	m_BK_BR_R;
	unsigned char	m_BK_BR_G;
	unsigned char	m_BK_BR_B;
	unsigned char	m_BK_BR_A;

	float			m_L1_X;
	float			m_L1_Y;
	float			m_L1_Z;
	unsigned char	m_L1_R;
	unsigned char	m_L1_G;
	unsigned char	m_L1_B;
	unsigned char	m_L1_A;

	float			m_L2_X;
	float			m_L2_Y;
	float			m_L2_Z;
	unsigned char	m_L2_R;
	unsigned char	m_L2_G;
	unsigned char	m_L2_B;
	unsigned char	m_L2_A;

	float			m_L3_X;
	float			m_L3_Y;
	float			m_L3_Z;
	unsigned char	m_L3_R;
	unsigned char	m_L3_G;
	unsigned char	m_L3_B;
	unsigned char	m_L3_A;

	unsigned char	m_AMB_R;
	unsigned char	m_AMB_G;
	unsigned char	m_AMB_B;
	unsigned char	m_AMB_A;

	long			m_IconDataSize;

	void Init( void );
};


////////////////////////////////////////////////////////////////////////////
// GAMECUBE EXPORT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

struct MC_GCHeader
{
    long			m_MCardType;
    long			m_NSaveTypes;
    long			m_SaveTypeOffset;
    long			m_IconInfoOffset;
    long			m_IconDataOffset;
    long			m_TotalFileSize;

	void Init( void );
};

//--------------------------------------------------------------------------

struct MC_GCFileType
{
	long			m_MaxSaves;
	char			m_Filename[GC_FILETYPE_NAME_SIZE];
	long			m_FileSize;

	void Init( void );
};

//--------------------------------------------------------------------------

struct MC_GCIconInfo
{
	long			m_NFrames;
	long			m_AnimSpeed;
	long			m_IconDataSize;

	void Init( void );
};


////////////////////////////////////////////////////////////////////////////
// XBOX EXPORT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

struct MC_XBOXHeader
{
    long			m_MCardType;
    long			m_NSaveTypes;
    long			m_SaveTypeOffset;
    long			m_IconDataSize;
    long			m_IconDataOffset;
    long			m_TotalFileSize;

	void Init( void );
};

//--------------------------------------------------------------------------

struct MC_XBOXFileType
{
	long			m_MaxSaves;
	char			m_Filename[XBOX_FILETYPE_NAME_SIZE];
	long			m_FileSize;

	void Init( void );
};

////////////////////////////////////////////////////////////////////////////
// PC EXPORT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

struct MC_PCHeader
{
    long			m_MCardType;
    long			m_NSaveTypes;
    long			m_SaveTypeOffset;
    long			m_IconDataSize;
    long			m_IconDataOffset;
    long			m_TotalFileSize;

	void Init( void );
};

//--------------------------------------------------------------------------

struct MC_PCFileType
{
	long			m_MaxSaves;
	char			m_Filename[PC_FILETYPE_NAME_SIZE];
	long			m_FileSize;

	void Init( void );
};


////////////////////////////////////////////////////////////////////////////
// MEMCARDWIZ PROJECT DATA STRUCTS
////////////////////////////////////////////////////////////////////////////

struct MCWPrj_PS2SaveType
{
	long				m_FileSize;
	long				m_MaxFiles;
	char				m_Filename[PS2_FILETYPE_NAME_SIZE];
	char				m_BaseDirs[PS2_MAX_BASEDIRS][MCW_PS2_BASEDIR_NAME_SIZE];

	void Init( void );
};

//--------------------------------------------------------------------------

struct MCWPrj_PS2Data
{
	char				m_GameID[MCW_PS2_GAMEID_NAME_SIZE];
	char				m_IconFile[MCW_ICON_PATH_SIZE];
	MC_PS2IconInfo		m_IconInfo;
	MCWPrj_PS2SaveType	m_SaveTypes[MCW_MAX_SAVETYPES];

	void Init( void );
};

//==========================================================================

struct MCWPrj_GCSaveType
{
	long				m_FileSize;
	long				m_MaxFiles;
	char				m_Filename[GC_FILETYPE_NAME_SIZE];

	void Init( void );
};

//--------------------------------------------------------------------------

struct MCWPrj_GCData
{
	char				m_IconFile[MCW_ICON_PATH_SIZE];
	MC_GCIconInfo		m_IconInfo;
	MCWPrj_GCSaveType	m_SaveTypes[MCW_MAX_SAVETYPES];

	void Init( void );
};

//==========================================================================

struct MCWPrj_XBOXSaveType
{
	long				m_FileSize;
	long				m_MaxFiles;
	char				m_Filename[XBOX_FILETYPE_NAME_SIZE];

	void Init( void );
};

//--------------------------------------------------------------------------

struct MCWPrj_XBOXData
{
	char				m_IconFile[MCW_ICON_PATH_SIZE];
	MCWPrj_XBOXSaveType	m_SaveTypes[MCW_MAX_SAVETYPES];

	void Init( void );
};

//==========================================================================

struct MCWPrj_PCSaveType
{
	long				m_FileSize;
	long				m_MaxFiles;
	char				m_Filename[PC_FILETYPE_NAME_SIZE];

	void Init( void );
};

//--------------------------------------------------------------------------

struct MCWPrj_PCData
{
	char				m_IconFile[MCW_ICON_PATH_SIZE];
	MCWPrj_PCSaveType	m_SaveTypes[MCW_MAX_SAVETYPES];

	void Init( void );
};

//==========================================================================

struct MCW_Project
{
	char				m_HdrStr[MCW_HEADER_STR_SIZE];

	long				m_NSaveTypes;
	char				m_SaveTypeNames[MCW_MAX_SAVETYPES][MCW_SAVETYPE_NAME_SIZE];

	MCWPrj_PS2Data		m_PS2;
	MCWPrj_GCData		m_GC;
	MCWPrj_XBOXData		m_XBOX;
	MCWPrj_PCData		m_PC;

	void Init( void );

	void Export( const char* pFilename, MEMCARD_TYPE ExpTarget );

  protected:
	BOOL ExportPS2 ( FILE* pFile );
	BOOL ExportGC  ( FILE* pFile );
	BOOL ExportXBOX( FILE* pFile );
	BOOL ExportPC  ( FILE* pFile );
};


////////////////////////////////////////////////////////////////////////////

#endif // MEMCARDPROJECT_H_INCLUDED
#ifndef _ExportFile_h_
#define	_ExportFile_h_
// ExportFile.h : header file
//

#include <afx.h>

#include "x_types.h"

/////////////////////////////////////////////////////////////////////////////
//	Export Targets
enum
{
	EXPORT_NULL		= 0,

	EXPORT_GAMECUBE	= 1,
	EXPORT_PC,
	EXPORT_XBOX,
	EXPORT_PS2,
} ;
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CExportFile

class CExportFile : public CMemFile
{

// Attributes
public:
	int				m_Target ;				// Target Machine to Output for
	bool			m_BigEndian ;			// Write in BigEndian Format

	int				m_nBones ;				// Number of Bones

// Constructors
public:
	CExportFile() ;
	~CExportFile() ;

// Operations
public:
	bool Create( int Target ) ;

	bool CheckChanged( const char *pFileName, s32 Offset ) ;
	bool WriteToDisk( const char *pFileName ) ;
	bool AppendToDisk( const char *pFileName ) ;

	//---	Export Functions
	bool Writeu8 ( u8  Value ) ;
	bool Writeu16( u16 Value ) ;
	bool Writeu32( u32 Value ) ;
	bool Writes8 ( s8  Value ) ;
	bool Writes16( s16 Value ) ;
	bool Writes32( s32 Value ) ;
	bool Writef32( f32 Value ) ;
	bool WriteMem( u8 *Mem, int Length ) ;
	bool WriteString( const char *pString ) ;
	bool WriteStringFmt( const char *Fmt, ... ) ;
	bool WritePaddedString( const char *pString, int nChars ) ;
	bool PadToBoundry( int Boundry ) ;
	bool PatchPtr( int FilePos, int Value ) ;

	bool Cat( CExportFile *pFile ) ;
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_ExportFile_h_

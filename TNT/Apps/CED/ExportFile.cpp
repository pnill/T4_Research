// ExportFile.cpp : implementation file
//

#include "stdafx.h"

#include "ExportFile.h"
#include "x_memory.hpp"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CExportFile Constructors

CExportFile::CExportFile()
{
	m_Target = EXPORT_NULL ;
	m_BigEndian = false ;
}

CExportFile::~CExportFile()
{
}

/////////////////////////////////////////////////////////////////////////////
// CExportFile Implementation

bool CExportFile::Create( int Target )
{
	m_Target = Target ;

	switch( Target )
	{
	case EXPORT_GAMECUBE:
		m_BigEndian = true ;
		break ;
	case EXPORT_PC:
		m_BigEndian = false ;
		break ;
	case EXPORT_XBOX:
		m_BigEndian = false ;
		break ;
	case EXPORT_PS2:
		m_BigEndian = false ;
		break ;
	default:
		ASSERT( 0 ) ;
		break ;
	}

	return true ;
}

bool CExportFile::CheckChanged( const char *pFileName, s32 Offset )
{
	bool	Same = false ;
	CFile	File ;
	s32		Length = (s32)GetLength();

	//---	if the file doesn't exist, return TRUE
	if( !File.Open( pFileName, CFile::modeRead ) )
		return TRUE;

	//---	if the file is not long enough to contain this data, return TRUE
	if( File.GetLength() < (u32)(Offset + Length) )
	{
		File.Close() ;
		return TRUE;
	}

	//---	go to the start position
	File.Seek( Offset, CFile::begin );

	//---	File is long enough to hold this buffer, check Byte by Byte
	u8	*pOld = new u8[Length] ;
	ASSERT( pOld ) ;
	if( pOld )
	{
		File.Read( pOld, Length ) ;
		u8 *pNew = Detach( ) ;
		if( memcmp( pOld, pNew, Length ) == 0 )
		{
			Same = true ;
		}
		Attach( pNew, Length ) ;
		delete []pOld ;
	}

	File.Close() ;

	return !Same ;
}


bool CExportFile::WriteToDisk( const char *pFileName )
{
	CFile		File ;

	//---	Different, Save new File
	try
	{
		if( File.Open( pFileName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary ) )
		{
			File.SetLength( 0 );
			s32	Len = (s32)GetLength() ;
			u8	*p = Detach() ;
			File.Write( p, Len ) ;
			free( p ) ;
			File.Close() ;
		}
		else
		{
			CString	Error ;
			Error.Format( "Can't write file '%s'", pFileName ) ;
			::MessageBox (NULL, Error, "Export Error!", MB_ICONSTOP) ;
			return FALSE;
		}
	}
	catch(...)
	{
		CString	Error ;
		Error.Format( "Can't write file '%s'", pFileName ) ;
		::MessageBox (NULL, Error, "Export Error!", MB_ICONSTOP) ;
		return FALSE;
	}

	return TRUE;
}


bool CExportFile::WriteToDiskIfDifferent( const char *pFileName, bool* pbActuallyWritten )
{
	CFile		File ;
	bool		bSame = FALSE;

	//---	if the file already exists, check to see if they are the same
	CFileStatus s;
	if( CFile::GetStatus( pFileName, s ) )
	{
		u8* pBuff = (u8*) malloc( (size_t)s.m_size );
		if( File.Open( pFileName, CFile::modeRead | CFile::typeBinary ) )
		{
			File.Read( pBuff, (u32)s.m_size );
			s32	Len = (s32)GetLength() ;
			if( Len == s.m_size )
			{
				u8	*p = Detach() ;
				if( memcmp( p, pBuff, Len ) == 0 )
					bSame = TRUE;
				Attach( p, Len );
			}
			File.Close();
		}

		if( bSame )
		{
			if( pbActuallyWritten ) *pbActuallyWritten = FALSE;
			return TRUE;
		}
	}

	if( pbActuallyWritten ) *pbActuallyWritten = TRUE;
	return WriteToDisk( pFileName );
}


bool CExportFile::AppendToDisk( const char *pFileName )
{
	CFile		File ;

	//---	Different, Save new File
	try
	{
		if( File.Open( pFileName, CFile::modeWrite | CFile::typeBinary ) )
		{
			File.SeekToEnd();
			s32	Len = (s32)GetLength() ;
			u8	*p = Detach() ;
			File.Write( p, Len ) ;
			free( p ) ;
			File.Close() ;
		}
		else
		{
			CString	Error ;
			Error.Format( "Can't write file '%s'", pFileName ) ;
			::MessageBox (NULL, Error, "Export Error!", MB_ICONSTOP) ;
			return FALSE;
		}
	}
	catch(...)
	{
		CString	Error ;
		Error.Format( "Can't write file '%s'", pFileName ) ;
		::MessageBox (NULL, Error, "Export Error!", MB_ICONSTOP) ;
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CExportFile Export Primitive Functions

bool CExportFile::Writeu8( u8 Value )
{
	bool	Success = true ;			// Success Code

	try
	{
		Write(&Value,sizeof(u8)) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::Writeu16( u16 Value )
{
	bool	Success = true ;			// Success Code
	u16		tValue ;

	try
	{
		if (m_BigEndian)
			tValue = ((Value<<8)&0xff00) | ((Value>>8)&0x00ff) ;
		else
			tValue = Value ;
		Write(&tValue,sizeof(u16)) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::Writeu32( u32 Value )
{
	bool	Success = true ;			// Success Code
	u32		tValue ;

	try
	{
		if (m_BigEndian)
			tValue = ((Value>>24)&0x000000ff) |
					 ((Value>> 8)&0x0000ff00) |
					 ((Value<< 8)&0x00ff0000) |
					 ((Value<<24)&0xff000000) ;
		else
			tValue = Value ;
		Write(&tValue,sizeof(u32)) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::Writes8( s8 Value )
{
	bool	Success = true ;			// Success Code

	try
	{
		Write(&Value,sizeof(s8)) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::Writes16( s16 Value )
{
	bool	Success = true ;			// Success Code
	u16		tValue ;

	try
	{
		if (m_BigEndian)
			tValue = ((Value<<8)&0xff00) | ((Value>>8)&0x00ff) ;
		else
			tValue = Value ;
		Write(&tValue,sizeof(s16)) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::Writes32( s32 Value )
{
	bool	Success = true ;			// Success Code
	u32		tValue ;

	try
	{
		if (m_BigEndian)
			tValue = ((Value>>24)&0x000000ff) |
					 ((Value>> 8)&0x0000ff00) |
					 ((Value<< 8)&0x00ff0000) |
					 ((Value<<24)&0xff000000) ;
		else
			tValue = Value ;
		Write(&tValue,sizeof(s32)) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::Writef32( f32 Value )
{
	bool	Success = true ;			// Success Code
	u32		tValue = *((u32*)&Value) ;

	try
	{
		if (m_BigEndian)
			tValue = (((tValue)>>24)&0x000000ff) |
					 (((tValue)>> 8)&0x0000ff00) |
					 (((tValue)<< 8)&0x00ff0000) |
					 (((tValue)<<24)&0xff000000) ;
		Write(&tValue,sizeof(u32)) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::WriteMem( u8 *Mem, int Length )
{
	bool	Success = true ;			// Success Code

	try
	{
		Write(Mem,Length) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::WriteString( const char *String )
{
	bool	Success = true ;			// Success Code

	try
	{
		Write( String, strlen(String) ) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::WriteStringFmt( const char *Fmt, ... )
{
	bool	Success = true ;			// Success Code

	char	Buffer[4096] = {0} ;
	va_list	ArgList ;
	va_start( ArgList, Fmt ) ;
	int nChars = _vsnprintf( Buffer, 4090, Fmt, ArgList ) ;

	try
	{
		Write( Buffer, nChars ) ;
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::WritePaddedString( const char *String, int nChars )
{
	bool	Success = true ;			// Success Code
	u8		Pad = 0 ;

	try
	{
		const char	*p = String ;
		int			Count = 0 ;
		while( (Count < (nChars-1)) && (*p != 0) )
		{
			Write( p++, 1) ;
			Count++ ;
		}
		while( Count < nChars )
		{
			Write( &Pad, 1) ;
			Count++ ;
		}
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::PadToBoundry( int Boundry )
{
	bool	Success = true ;			// Success Code
	u8		Pad = 0 ;

	try
	{
		while ((GetPosition() % Boundry) != 0)
		{
			Write( &Pad, 1 ) ;
		}
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::PatchPtr( int FilePos, int Value )
{
	bool	Success = true ;			// Success Code

	try
	{
		int Pos = (s32)GetPosition( ) ;
		if( (FilePos >= 0) && (FilePos < Pos) )
		{
			Seek( FilePos, CFile::begin ) ;
			Writes32( Value ) ;
			Seek( Pos, CFile::begin ) ;
		}
	}
	catch(...)
	{
		Success = false ;
	}

	//---	Return Success Code
	return Success ;
}

bool CExportFile::Cat( CExportFile *pFile )
{
	if( pFile->GetLength() > 0 )
	{
		//---	Get Data Buffer from ExportFile
		s32	Len = (s32)pFile->GetLength() ;
		u8	*pData = pFile->Detach() ;

		//---	Write Data to Current ExportFile
		WriteMem( pData, Len ) ;

		//---	Copy Data Back to ExportFile
		u8	*pNewData = pFile->Alloc( Len ) ;
		pFile->Attach( pNewData, Len, 1024 ) ;
		pFile->WriteMem( pData, Len ) ;

		//---	Free Old Buffer
		pFile->Free( pData ) ;
	}

	return true ;
}

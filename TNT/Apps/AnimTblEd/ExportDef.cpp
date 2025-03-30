// ExportDef.cpp: implementation of the CExportDef class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "ExportDef.h"
#include "ExportConfigDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExportDef::CExportDef()
{
	m_Name.Format( "export" );
	m_Flags = 0;
	m_ExportDataSeparately = FALSE;
}

CExportDef::~CExportDef()
{

}

CString& CExportDef::GetName()
{
	return m_Name;
}

void CExportDef::SetName(const char *lpszName)
{
	if (!lpszName)
		return;

	m_Name.Format( "%s", lpszName );
}

CString& CExportDef::GetPath()
{
	return m_Path;
}

void CExportDef::SetPath(const char *lpszPath)
{
	if (!lpszPath)
		return;

	m_Path.Format( "%s", lpszPath );
} /* SetPath */

int CExportDef::ExportDataSeparately()
{
	return m_ExportDataSeparately ? TRUE : FALSE;
}

void CExportDef::ExportDataSeparately( int NewValue )
{
	m_ExportDataSeparately = NewValue ? TRUE : FALSE;
}

CString CExportDef::GetDataPath()
{
	CString Path = GetPath();
	int ext=Path.FindOneOf(".");
	if(ext != -1)
		Path = Path.Left( ext );
	Path += ".DAT";
	return Path;
}

int CExportDef::GetEnabled()
{
	return (m_Flags & EXPORT_FLAG_ENABLE) ? TRUE : FALSE;
}

int CExportDef::SetEnabled(int Enable)
{
	int	Current = (m_Flags & EXPORT_FLAG_ENABLE) ? TRUE : FALSE;

	if (Enable)
		m_Flags |= EXPORT_FLAG_ENABLE;
	else
		m_Flags &= ~EXPORT_FLAG_ENABLE;

	return Current;
}

int CExportDef::SetFlags(int NewFlags)
{
	int OldFlags = m_Flags;
	m_Flags = NewFlags;
	return OldFlags;
}

int CExportDef::GetFlags()
{
	return m_Flags;
}

void CExportDef::Serialize(CArchive &ar)
{
	int version;

	if (ar.IsStoring())
	{
		//---	Write Version
		version = 1;
		ar << version;

		//---********  ADD NEW SERIALIZATIONS HERE ***********

		ar << m_ExportDataSeparately;

		ar.WriteString( m_Name );
		ar.WriteString( "\xd\xa" );
		ar.WriteString( m_Path );
		ar.WriteString( "\xd\xa" );
		ar << m_Flags;
	}
	else
	{
		ar >> version;

		switch( version )
		{
		case 1:
			ar >> m_ExportDataSeparately;
		case 0:
			ar.ReadString( m_Name );
			ar.ReadString( m_Path );
			ar >> m_Flags;
			break;
		}
	}
}

void CExportDef::Copy(CExportDef &ExportDefCopy)
{
	ExportDefCopy.SetFlags( m_Flags );
	ExportDefCopy.SetName( m_Name );
	ExportDefCopy.SetPath( m_Path );
	ExportDefCopy.ExportDataSeparately( m_ExportDataSeparately );
}

int CExportDef::Edit()
{
	CExportConfigDlg	ExportConfigDlg;

	this->Copy( ExportConfigDlg.m_ExportDef );

	if (ExportConfigDlg.DoModal() == IDOK)
	{
		this->SetFlags( ExportConfigDlg.m_ExportDef.GetFlags() );
		this->SetName( ExportConfigDlg.m_ExportDef.GetName() );
		this->SetPath( ExportConfigDlg.m_ExportDef.GetPath() );
		this->ExportDataSeparately( ExportConfigDlg.m_ExportDef.ExportDataSeparately() );
//		ExportConfigDlg.m_ExportDef.Copy( *this );
		return TRUE;
	}

	return FALSE;
}

int CExportDef::IsValid()
{
	return !m_Path.IsEmpty();
}

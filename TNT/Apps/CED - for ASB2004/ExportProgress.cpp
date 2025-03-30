// ExportProgress.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "ExportProgress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportProgress dialog


CExportProgress::CExportProgress(CWnd* pParent /*=NULL*/)
	: CDialog(CExportProgress::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportProgress)
	//}}AFX_DATA_INIT

	m_Closed = false ;
}


void CExportProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportProgress)
	DDX_Control(pDX, IDOK, m_CtrlOk);
	DDX_Control(pDX, IDC_LIST, m_CtrlList);
	DDX_Control(pDX, IDC_PROGRESS, m_CtrlProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportProgress, CDialog)
	//{{AFX_MSG_MAP(CExportProgress)
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportProgress message handlers

void CExportProgress::Fmt( const char *pFormat, ... )
{
	va_list		Args ;
	va_start( Args, pFormat ) ;

	char Buffer[4096] = {0} ;
	_vsnprintf( Buffer, 4095, pFormat, Args ) ;

	int Index = m_CtrlList.AddString( Buffer ) ;
	m_CtrlList.SetTopIndex( Index ) ;
	m_CtrlList.RedrawWindow() ;
	
	va_end( Args ) ;
}

void CExportProgress::SetRange( int Min, int Max )
{
	m_CtrlProgress.SetRange( Min, Max ) ;
	m_CtrlProgress.RedrawWindow() ;
}

void CExportProgress::SetPos( int Pos )
{
	m_CtrlProgress.SetPos( Pos ) ;
	m_CtrlProgress.RedrawWindow() ;
}

void CExportProgress::EnableOk( bool State )
{
	m_CtrlOk.EnableWindow( State ) ;
}

void CExportProgress::OnOK() 
{
	// TODO: Add extra validation here
	
//	CDialog::OnOK();

	m_Closed = true ;
	DestroyWindow() ;
}

void CExportProgress::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

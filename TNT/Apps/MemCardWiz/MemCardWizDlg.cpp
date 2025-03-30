// MemCardWizDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MemCardWiz.h"
#include "MemCardWizDlg.h"

#include "TextEntryDlg.h"
#include "PS2IconPropDlg.h"
#include "GCIconPropDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMemCardWizDlg dialog

CMemCardWizDlg::CMemCardWizDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMemCardWizDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMemCardWizDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon( IDI_ICON_SMALL );

	m_MCProj.Init();

	m_CurFType = 0;
	m_bCurView = MEMCARD_TYPE_PS2;
}

//==========================================================================

BOOL CMemCardWizDlg::IsPS2View( void )
{
	return m_bCurView == MEMCARD_TYPE_PS2;
}

//==========================================================================

BOOL CMemCardWizDlg::IsGCView( void )
{
	return m_bCurView == MEMCARD_TYPE_GAMECUBE;
}

//==========================================================================

BOOL CMemCardWizDlg::IsXBOXView( void )
{
	return m_bCurView == MEMCARD_TYPE_XBOX;
}

//==========================================================================

BOOL CMemCardWizDlg::IsPCView( void )
{
	return m_bCurView == MEMCARD_TYPE_PC;
}

//==========================================================================

void CMemCardWizDlg::InitAPPControls( void )
{
	CButton* pBtn;

	m_bCurView = MEMCARD_TYPE_PS2;

	pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEW_PS2 );
	pBtn->SetCheck( TRUE );

	pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEW_GC );
	pBtn->SetCheck( FALSE );

	pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEW_XBOX );
	pBtn->SetCheck( FALSE );

	pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEW_PC );
	pBtn->SetCheck( FALSE );
}

//==========================================================================

void CMemCardWizDlg::InitPS2Controls( void )
{
	CSpinButtonCtrl* pSpin;
	CEdit* pEdit;

	pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES );
	pSpin->SetRange32( PS2_MIN_BASEDIRS, PS2_MAX_BASEDIRS );
	pSpin->SetPos( PS2_MIN_BASEDIRS );

	PS2_UpdateMaxFile( PS2_MIN_BASEDIRS );

	pEdit = (CEdit*)GetDlgItem( IDC_EDIT_PS2_GAMEID );
	pEdit->SetLimitText( MCW_PS2_GAMEID_NAME_SIZE_USED );

	pEdit = (CEdit*)GetDlgItem( IDC_EDIT_PS2_FILESIZE );
	pEdit->SetLimitText( PS2_FILETYPE_SIZE_MAX_CHARS );

	pEdit = (CEdit*)GetDlgItem( IDC_EDIT_PS2_FILENAME );
	pEdit->SetLimitText( PS2_FILETYPE_NAME_SIZE - 1 );

	pEdit = (CEdit*)GetDlgItem( IDC_EDIT_PS2_DIR1 );
	pEdit->SetLimitText( MCW_PS2_BASEDIR_NAME_SIZE - 1 );
	pEdit = (CEdit*)GetDlgItem( IDC_EDIT_PS2_DIR2 );
	pEdit->SetLimitText( MCW_PS2_BASEDIR_NAME_SIZE - 1 );
	pEdit = (CEdit*)GetDlgItem( IDC_EDIT_PS2_DIR3 );
	pEdit->SetLimitText( MCW_PS2_BASEDIR_NAME_SIZE - 1 );
	pEdit = (CEdit*)GetDlgItem( IDC_EDIT_PS2_DIR4 );
	pEdit->SetLimitText( MCW_PS2_BASEDIR_NAME_SIZE - 1 );
}

//==========================================================================

void CMemCardWizDlg::InitGCControls( void )
{
	// USES PS2 CONTROLS
}

//==========================================================================

void CMemCardWizDlg::InitXBOXControls( void )
{
	// USES PS2 CONTROLS
}

//==========================================================================

void CMemCardWizDlg::InitPCControls( void )
{
	// USES PS2 CONTROLS
}

//==========================================================================

void CMemCardWizDlg::EnableIconControls( BOOL bEnable )
{
	GetDlgItem( IDC_STATIC_ICONFILE )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_ICON_FILE  )->EnableWindow( bEnable );
	GetDlgItem( IDC_BTN_ICON_BROWSE )->EnableWindow( bEnable );
	GetDlgItem( IDC_BTN_ICON_EDIT   )->EnableWindow( bEnable );
}

//==========================================================================

void CMemCardWizDlg::EnablePS2Controls( BOOL bEnable )
{
	GetDlgItem( IDC_SPIN_PS2_NFILES   )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_NFILES   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_NFILES )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_GAMEID   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_GAMEID )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_FILESIZE )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_FILENAME   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_FILENAME )->EnableWindow( bEnable );

	GetDlgItem( IDC_STATIC_PS2_DIR1 )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_DIR2 )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_DIR3 )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_DIR4 )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_DIR5 )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_DIR6 )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_DIR7 )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_DIR8 )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_DIR1 )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_DIR2 )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_DIR3 )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_DIR4 )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_DIR5 )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_DIR6 )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_DIR7 )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_DIR8 )->EnableWindow( bEnable );
}

//==========================================================================

void CMemCardWizDlg::EnableGCControls( BOOL bEnable )
{
	GetDlgItem( IDC_SPIN_PS2_NFILES   )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_NFILES   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_NFILES )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_FILESIZE )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_FILENAME   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_FILENAME )->EnableWindow( bEnable );
}

//==========================================================================

void CMemCardWizDlg::EnableXBOXControls( BOOL bEnable )
{
	GetDlgItem( IDC_SPIN_PS2_NFILES   )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_NFILES   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_NFILES )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_FILESIZE )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_FILENAME   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_FILENAME )->EnableWindow( bEnable );
}

//==========================================================================

void CMemCardWizDlg::EnablePCControls( BOOL bEnable )
{
	GetDlgItem( IDC_SPIN_PS2_NFILES   )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT_PS2_NFILES   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_NFILES )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_FILESIZE )->EnableWindow( bEnable );

	GetDlgItem( IDC_EDIT_PS2_FILENAME   )->EnableWindow( bEnable );
	GetDlgItem( IDC_STATIC_PS2_FILENAME )->EnableWindow( bEnable );
}

//==========================================================================

void CMemCardWizDlg::ShowPS2Controls( BOOL bShow )
{
	GetDlgItem( IDC_SPIN_PS2_NFILES   )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_NFILES   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_NFILES )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_GAMEID   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_GAMEID )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_FILESIZE )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_FILENAME   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_FILENAME )->ShowWindow( bShow );

	GetDlgItem( IDC_STATIC_PS2_DIR1 )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_DIR2 )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_DIR3 )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_DIR4 )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_DIR5 )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_DIR6 )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_DIR7 )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_DIR8 )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_DIR1 )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_DIR2 )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_DIR3 )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_DIR4 )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_DIR5 )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_DIR6 )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_DIR7 )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_DIR8 )->ShowWindow( bShow );
}

//==========================================================================

void CMemCardWizDlg::ShowGCControls( BOOL bShow )
{
	GetDlgItem( IDC_SPIN_PS2_NFILES   )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_NFILES   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_NFILES )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_FILESIZE )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_FILENAME   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_FILENAME )->ShowWindow( bShow );
}

//==========================================================================

void CMemCardWizDlg::ShowXBOXControls( BOOL bShow )
{
	GetDlgItem( IDC_SPIN_PS2_NFILES   )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_NFILES   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_NFILES )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_FILESIZE )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_FILENAME   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_FILENAME )->ShowWindow( bShow );
}

//==========================================================================

void CMemCardWizDlg::ShowPCControls( BOOL bShow )
{
	GetDlgItem( IDC_SPIN_PS2_NFILES   )->ShowWindow( bShow );
	GetDlgItem( IDC_EDIT_PS2_NFILES   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_NFILES )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_FILESIZE )->ShowWindow( bShow );

	GetDlgItem( IDC_EDIT_PS2_FILENAME   )->ShowWindow( bShow );
	GetDlgItem( IDC_STATIC_PS2_FILENAME )->ShowWindow( bShow );
}

//==========================================================================

void CMemCardWizDlg::GetFileTypeInfoFromControls( void )
{
	int i;
	CListBox* pLBox;

	pLBox = (CListBox*)GetDlgItem( IDC_LIST_FILETYPES );

	m_MCProj.m_NSaveTypes = pLBox->GetCount();

	for( i = 0; i < m_MCProj.m_NSaveTypes; i++ )
	{
		pLBox->GetText( i, m_MCProj.m_SaveTypeNames[i] );
	}
}

//==========================================================================

void CMemCardWizDlg::SetControlsFromFileTypeInfo( void )
{
	int i;
	CListBox* pLBox;

	pLBox = (CListBox*)GetDlgItem( IDC_LIST_FILETYPES );

	pLBox->ResetContent();

	for( i = 0; i < m_MCProj.m_NSaveTypes; i++ )
	{
		pLBox->AddString( m_MCProj.m_SaveTypeNames[i] );
	}

	m_CurFType = 0;

	pLBox->SetCurSel( 0 );
}

//==========================================================================

void CMemCardWizDlg::GetDataFromControls( void )
{
	if( IsPS2View() )
	{
		PS2_GetDataFromControls();
	}
	else if( IsGCView() )
	{
		GC_GetDataFromControls();
	}
	else if( IsXBOXView() )
	{
		XBOX_GetDataFromControls();
	}
	else if( IsPCView() )
	{
		PC_GetDataFromControls();
	}
}

//==========================================================================

void CMemCardWizDlg::SetControlsFromData( void )
{
	if( IsPS2View() )
	{
		PS2_SetControlsFromData();
	}
	else if( IsGCView() )
	{
		GC_SetControlsFromData();
	}
	else if( IsXBOXView() )
	{
		XBOX_SetControlsFromData();
	}
	else if( IsPCView() )
	{
		PC_SetControlsFromData();
	}
}


//==========================================================================

void CMemCardWizDlg::PS2_UpdateMaxFile( int MaxFiles )
{
	if( MaxFiles < PS2_MIN_BASEDIRS )
		MaxFiles = PS2_MAX_BASEDIRS;
	else if( MaxFiles > PS2_MAX_BASEDIRS )
		MaxFiles = PS2_MIN_BASEDIRS;

	const int StaticCtrlID[PS2_MAX_BASEDIRS] =
	{
		IDC_STATIC_PS2_DIR1,
		IDC_STATIC_PS2_DIR2,
		IDC_STATIC_PS2_DIR3,
		IDC_STATIC_PS2_DIR4,
		IDC_STATIC_PS2_DIR5,
		IDC_STATIC_PS2_DIR6,
		IDC_STATIC_PS2_DIR7,
		IDC_STATIC_PS2_DIR8,
	};
	const int EditCtrlID[PS2_MAX_BASEDIRS] =
	{
		IDC_EDIT_PS2_DIR1,
		IDC_EDIT_PS2_DIR2,
		IDC_EDIT_PS2_DIR3,
		IDC_EDIT_PS2_DIR4,
		IDC_EDIT_PS2_DIR5,
		IDC_EDIT_PS2_DIR6,
		IDC_EDIT_PS2_DIR7,
		IDC_EDIT_PS2_DIR8,
	};

	int i;

	for( i = 0; i < PS2_MAX_BASEDIRS; i++ )
	{
		GetDlgItem( StaticCtrlID[i] )->ShowWindow( MaxFiles >= (i+1) );
		GetDlgItem( EditCtrlID[i]   )->ShowWindow( MaxFiles >= (i+1) );
	}
}

//==========================================================================

void CMemCardWizDlg::PS2_GetDataFromControls( void )
{
	int  i;
	char TempStr[16];

	const int EditCtrlID[PS2_MAX_BASEDIRS] =
	{
		IDC_EDIT_PS2_DIR1,
		IDC_EDIT_PS2_DIR2,
		IDC_EDIT_PS2_DIR3,
		IDC_EDIT_PS2_DIR4,
		IDC_EDIT_PS2_DIR5,
		IDC_EDIT_PS2_DIR6,
		IDC_EDIT_PS2_DIR7,
		IDC_EDIT_PS2_DIR8,
	};

	GetDlgItem( IDC_EDIT_ICON_FILE )->GetWindowText( m_MCProj.m_PS2.m_IconFile, MCW_ICON_PATH_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_GAMEID )->GetWindowText( m_MCProj.m_PS2.m_GameID, MCW_PS2_GAMEID_NAME_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_FILENAME )->GetWindowText( m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_Filename, MCW_SAVETYPE_NAME_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE )->GetWindowText( TempStr, sizeof(TempStr)-1 );
	sscanf( TempStr, "%d", &(m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_FileSize) );

	m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_MaxFiles = ((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES ))->GetPos();

	for( i = 0; i < PS2_MAX_BASEDIRS; i++ )
	{
		GetDlgItem( EditCtrlID[i] )->GetWindowText( m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_BaseDirs[i], PS2_BASEDIR_NAME_SIZE-1 );
	}
}

//==========================================================================

void CMemCardWizDlg::PS2_SetControlsFromData( void )
{
	int  i;
	char TempStr[16];

	const int EditCtrlID[PS2_MAX_BASEDIRS] =
	{
		IDC_EDIT_PS2_DIR1,
		IDC_EDIT_PS2_DIR2,
		IDC_EDIT_PS2_DIR3,
		IDC_EDIT_PS2_DIR4,
		IDC_EDIT_PS2_DIR5,
		IDC_EDIT_PS2_DIR6,
		IDC_EDIT_PS2_DIR7,
		IDC_EDIT_PS2_DIR8,
	};


	GetDlgItem( IDC_EDIT_ICON_FILE )->SetWindowText( m_MCProj.m_PS2.m_IconFile );

	GetDlgItem( IDC_EDIT_PS2_GAMEID )->SetWindowText( m_MCProj.m_PS2.m_GameID );

	GetDlgItem( IDC_EDIT_PS2_FILENAME )->SetWindowText( m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_Filename );

	sprintf( TempStr, "%d", m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_FileSize );
	GetDlgItem( IDC_EDIT_PS2_FILESIZE )->SetWindowText( TempStr );

	((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES ))->SetPos( m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_MaxFiles );

	for( i = 0; i < PS2_MAX_BASEDIRS; i++ )
	{
		GetDlgItem( EditCtrlID[i] )->SetWindowText( m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_BaseDirs[i] );
	}

	PS2_UpdateMaxFile( m_MCProj.m_PS2.m_SaveTypes[m_CurFType].m_MaxFiles );
}

//==========================================================================

void CMemCardWizDlg::GC_GetDataFromControls( void )
{
	char TempStr[16];

	GetDlgItem( IDC_EDIT_ICON_FILE )->GetWindowText( m_MCProj.m_GC.m_IconFile, MCW_ICON_PATH_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_FILENAME )->GetWindowText( m_MCProj.m_GC.m_SaveTypes[m_CurFType].m_Filename, MCW_SAVETYPE_NAME_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE )->GetWindowText( TempStr, sizeof(TempStr)-1 );
	sscanf( TempStr, "%d", &(m_MCProj.m_GC.m_SaveTypes[m_CurFType].m_FileSize) );

	m_MCProj.m_GC.m_SaveTypes[m_CurFType].m_MaxFiles = ((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES ))->GetPos();
}

//==========================================================================

void CMemCardWizDlg::GC_SetControlsFromData( void )
{
	char TempStr[16];

	GetDlgItem( IDC_EDIT_ICON_FILE )->SetWindowText( m_MCProj.m_GC.m_IconFile );

	GetDlgItem( IDC_EDIT_PS2_FILENAME )->SetWindowText( m_MCProj.m_GC.m_SaveTypes[m_CurFType].m_Filename );

	sprintf( TempStr, "%d", m_MCProj.m_GC.m_SaveTypes[m_CurFType].m_FileSize );
	GetDlgItem( IDC_EDIT_PS2_FILESIZE )->SetWindowText( TempStr );

	((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES ))->SetPos( m_MCProj.m_GC.m_SaveTypes[m_CurFType].m_MaxFiles );
}

//==========================================================================

void CMemCardWizDlg::XBOX_GetDataFromControls( void )
{
	char TempStr[16];

	GetDlgItem( IDC_EDIT_ICON_FILE )->GetWindowText( m_MCProj.m_XBOX.m_IconFile, MCW_ICON_PATH_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_FILENAME )->GetWindowText( m_MCProj.m_XBOX.m_SaveTypes[m_CurFType].m_Filename, MCW_SAVETYPE_NAME_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE )->GetWindowText( TempStr, sizeof(TempStr)-1 );
	sscanf( TempStr, "%d", &(m_MCProj.m_XBOX.m_SaveTypes[m_CurFType].m_FileSize) );

	m_MCProj.m_XBOX.m_SaveTypes[m_CurFType].m_MaxFiles = ((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES ))->GetPos();
}

//==========================================================================

void CMemCardWizDlg::XBOX_SetControlsFromData( void )
{
	char TempStr[16];

	GetDlgItem( IDC_EDIT_ICON_FILE )->SetWindowText( m_MCProj.m_XBOX.m_IconFile );

	GetDlgItem( IDC_EDIT_PS2_FILENAME )->SetWindowText( m_MCProj.m_XBOX.m_SaveTypes[m_CurFType].m_Filename );

	sprintf( TempStr, "%d", m_MCProj.m_XBOX.m_SaveTypes[m_CurFType].m_FileSize );
	GetDlgItem( IDC_EDIT_PS2_FILESIZE )->SetWindowText( TempStr );

	((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES ))->SetPos( m_MCProj.m_XBOX.m_SaveTypes[m_CurFType].m_MaxFiles );
}

//==========================================================================

void CMemCardWizDlg::PC_GetDataFromControls( void )
{
	char TempStr[16];

	GetDlgItem( IDC_EDIT_ICON_FILE )->GetWindowText( m_MCProj.m_PC.m_IconFile, MCW_ICON_PATH_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_FILENAME )->GetWindowText( m_MCProj.m_PC.m_SaveTypes[m_CurFType].m_Filename, MCW_SAVETYPE_NAME_SIZE-1 );

	GetDlgItem( IDC_EDIT_PS2_FILESIZE )->GetWindowText( TempStr, sizeof(TempStr)-1 );
	sscanf( TempStr, "%d", &(m_MCProj.m_PC.m_SaveTypes[m_CurFType].m_FileSize) );

	m_MCProj.m_PC.m_SaveTypes[m_CurFType].m_MaxFiles = ((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES ))->GetPos();
}

//==========================================================================

void CMemCardWizDlg::PC_SetControlsFromData( void )
{
	char TempStr[16];

	GetDlgItem( IDC_EDIT_ICON_FILE )->SetWindowText( m_MCProj.m_PC.m_IconFile );

	GetDlgItem( IDC_EDIT_PS2_FILENAME )->SetWindowText( m_MCProj.m_PC.m_SaveTypes[m_CurFType].m_Filename );

	sprintf( TempStr, "%d", m_MCProj.m_PC.m_SaveTypes[m_CurFType].m_FileSize );
	GetDlgItem( IDC_EDIT_PS2_FILESIZE )->SetWindowText( TempStr );

	((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_PS2_NFILES ))->SetPos( m_MCProj.m_PC.m_SaveTypes[m_CurFType].m_MaxFiles );
}

//==========================================================================

void CMemCardWizDlg::ImportProject( const CString& Filename )
{
	FILE* pFile;

	pFile = fopen( Filename, "rb" );

	if( pFile == NULL )
		return;

	char HdrStr[MCW_HEADER_STR_SIZE];

	fread( &HdrStr[0], sizeof(HdrStr), 1, pFile );

	if( strcmp( HdrStr, MCW_CUR_VERSION ) )
	{
		fclose( pFile );
		return;
	}

	fseek( pFile, 0, SEEK_END );

	if( ftell( pFile ) != sizeof(MCW_Project) )
	{
		fclose( pFile );
		return;
	}

	fseek( pFile, 0, SEEK_SET );

	fread( &m_MCProj, sizeof(m_MCProj), 1, pFile );

	fclose( pFile );
}

//==========================================================================

void CMemCardWizDlg::ExportProject( const CString& Filename )
{
	FILE* pFile;

	pFile = fopen( Filename, "wb" );
	if( pFile == NULL )
		return;

	fwrite( &m_MCProj, sizeof(m_MCProj), 1, pFile );

	fclose( pFile );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void CMemCardWizDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMemCardWizDlg)
	//}}AFX_DATA_MAP
}

////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMemCardWizDlg, CDialog)
	//{{AFX_MSG_MAP(CMemCardWizDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_OPEN, OnBtnOpen)
	ON_BN_CLICKED(IDC_BTN_SAVE, OnBtnSave)
	ON_BN_CLICKED(IDC_BTN_EXPORT, OnBtnExport)
	ON_BN_CLICKED(IDC_BTN_ADDTYPE, OnBtnAddtype)
	ON_BN_CLICKED(IDC_BTN_DELETETYPE, OnBtnDeletetype)
	ON_BN_CLICKED(IDC_BTN_RENAMETYPE, OnBtnRenametype)
	ON_LBN_SELCHANGE(IDC_LIST_FILETYPES, OnSelchangeListFiletypes)
	ON_BN_CLICKED(IDC_RADIO_VIEW_PS2, OnRadioViewPs2)
	ON_BN_CLICKED(IDC_RADIO_VIEW_GC, OnRadioViewGc)
	ON_BN_CLICKED(IDC_RADIO_VIEW_XBOX, OnRadioViewXbox)
	ON_BN_CLICKED(IDC_RADIO_VIEW_PC, OnRadioViewPC)
	ON_BN_CLICKED(IDC_BTN_ICON_BROWSE, OnBtnIconBrowse)
	ON_BN_CLICKED(IDC_BTN_ICON_EDIT, OnBtnIconEdit)
	ON_WM_SYSCOMMAND()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PS2_NFILES, OnDeltaposSpinPs2Nfiles)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemCardWizDlg message handlers

BOOL CMemCardWizDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	InitAPPControls();
	InitPS2Controls();
	InitGCControls();
	InitXBOXControls();
	InitPCControls();

	SetControlsFromFileTypeInfo();
	SetControlsFromData();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//==========================================================================

void CMemCardWizDlg::OnPaint() 
{
	// If you add a minimize button to your dialog, you will need the code below
	//  to draw the icon.  For MFC applications using the document/view model,
	//  this is automatically done for you by the framework.

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//==========================================================================

HCURSOR CMemCardWizDlg::OnQueryDragIcon()
{
	// The system calls this to obtain the cursor to display while the user drags
	//  the minimized window.

	return (HCURSOR) m_hIcon;
}

//==========================================================================

void CMemCardWizDlg::OnSysCommand(UINT nID, LPARAM lParam) 
{
    // If system message is Close(i.e. mouse click on X button), kill dialog
    if( nID == SC_CLOSE )
        this->EndDialog( IDD );

    CDialog::OnSysCommand( nID, lParam );
}

//==========================================================================

void CMemCardWizDlg::OnOK( void )
{
    // overloaded to keep default input from closing window
}

//==========================================================================

void CMemCardWizDlg::OnCancel( void )
{
    // overloaded to keep default input from closing window
}

//==========================================================================


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void CMemCardWizDlg::OnBtnOpen() 
{
	const char FILE_OPEN_FILTER[] = "MemCard Project Files(*.crd)|*.crd||";
    const long FILE_DLG_FLAGS = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

    CString FilenameToOpen;

    CFileDialog OpenDlg( TRUE,              //Open file dialog
                         "crd",             //Default extension
                         NULL,              //initial filename
                         FILE_DLG_FLAGS,    //Flags
                         FILE_OPEN_FILTER,  //Filter
                         this );            //Parent Wnd

    if( IDCANCEL == OpenDlg.DoModal() )
        return;


    FilenameToOpen = OpenDlg.GetPathName();

	// Got filename, now import
	ImportProject( FilenameToOpen ); 

	SetControlsFromFileTypeInfo();
	SetControlsFromData();
}

//==========================================================================

void CMemCardWizDlg::OnBtnSave() 
{
	const char FILE_SAVE_FILTER[] = "MemCard Project Files(*.crd)|*.crd||";
    const long FILE_DLG_FLAGS = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    CString FilenameToSave;

    CFileDialog SaveDlg( FALSE,             //Open file dialog
                         "crd",             //Default extension
                         NULL,              //initial filename
                         FILE_DLG_FLAGS,    //Flags
                         FILE_SAVE_FILTER,  //Filter
                         this );            //Parent Wnd

    if( IDCANCEL == SaveDlg.DoModal() )
        return;

    FilenameToSave = SaveDlg.GetPathName();

	GetFileTypeInfoFromControls();
	GetDataFromControls();

	// Got filename, now save
	ExportProject( FilenameToSave );
}

//==========================================================================

void CMemCardWizDlg::OnBtnExport() 
{
	const char FILE_SAVE_FILTER[] = "MemCard Export Files(*.mcf)|*.mcf||";
    const long FILE_DLG_FLAGS = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    CString FilenameToSave;

    CFileDialog SaveDlg( FALSE,             //Open file dialog
                         "mcf",             //Default extension
                         "MemCard",         //initial filename
                         FILE_DLG_FLAGS,    //Flags
                         FILE_SAVE_FILTER,  //Filter
                         this );            //Parent Wnd

    if( IDCANCEL == SaveDlg.DoModal() )
        return;

    FilenameToSave = SaveDlg.GetPathName();

	MEMCARD_TYPE ExpTarget;

	if( IsPS2View() )
		ExpTarget = MEMCARD_TYPE_PS2;
	else if( IsGCView() )
		ExpTarget = MEMCARD_TYPE_GAMECUBE;
	else if( IsXBOXView() )
		ExpTarget = MEMCARD_TYPE_XBOX;
	else if( IsPCView() )
		ExpTarget = MEMCARD_TYPE_PC;

	GetFileTypeInfoFromControls();
	GetDataFromControls();


	// Got filename, now save
	m_MCProj.Export( FilenameToSave, ExpTarget );
}

//==========================================================================

void CMemCardWizDlg::OnBtnAddtype() 
{
	CListBox* pLBox = (CListBox*)GetDlgItem( IDC_LIST_FILETYPES );

	if( pLBox->GetCount() >= MCW_MAX_SAVETYPES )
		return;

	CTextEntryDlg TEDialog( MCW_SAVETYPE_NAME_SIZE, "Enter New Save Type Name", this );

	if( IDCANCEL == TEDialog.DoModal() )
		return;

	if( TEDialog.m_TextEntry.GetLength() <= 0 )
		return;

	int NewIndex = pLBox->AddString( TEDialog.m_TextEntry );

	if( NewIndex == LB_ERR || NewIndex == LB_ERRSPACE )
		return;

	m_MCProj.m_PS2.m_SaveTypes[NewIndex].Init();
	m_MCProj.m_GC.m_SaveTypes[NewIndex].Init();
	m_MCProj.m_XBOX.m_SaveTypes[NewIndex].Init();
	m_MCProj.m_PC.m_SaveTypes[NewIndex].Init();

	pLBox->SetCurSel( NewIndex );

	this->GetFileTypeInfoFromControls();
}

//==========================================================================

void CMemCardWizDlg::OnBtnDeletetype() 
{
	CListBox* pLBox = (CListBox*)GetDlgItem( IDC_LIST_FILETYPES );

	int ItemCount = pLBox->GetCount();

	if( ItemCount <= 1 )
		return;

	int CurSel = pLBox->GetCurSel();

	if( CurSel == LB_ERR )
		return;

	pLBox->DeleteString( CurSel );

	ItemCount--;

	if( CurSel >= ItemCount )
		CurSel = ItemCount - 1;
	else
	{
		// TODO: Handle removing a filetype, shift all types of higher index down
	}

	pLBox->SetCurSel( CurSel );
}

//==========================================================================

void CMemCardWizDlg::OnBtnRenametype() 
{
	CListBox* pLBox = (CListBox*)GetDlgItem( IDC_LIST_FILETYPES );

	int CurSel = pLBox->GetCurSel();

	if( CurSel == LB_ERR )
		return;

	CTextEntryDlg TEDialog( MCW_SAVETYPE_NAME_SIZE, "Rename Save Type", this );

	if( IDCANCEL == TEDialog.DoModal() )
		return;

	if( TEDialog.m_TextEntry.GetLength() <= 0 )
		return;

	pLBox->DeleteString( CurSel );

	int NewIndex = pLBox->InsertString( CurSel, TEDialog.m_TextEntry );

	if( NewIndex == LB_ERR || NewIndex == LB_ERRSPACE )
		return;

	pLBox->SetCurSel( NewIndex );

	this->GetFileTypeInfoFromControls();
}

//==========================================================================

void CMemCardWizDlg::OnSelchangeListFiletypes() 
{
	CListBox* pLBox = (CListBox*)GetDlgItem( IDC_LIST_FILETYPES );

	int CurSel = pLBox->GetCurSel();

	GetDlgItem( IDC_BTN_DELETETYPE )->EnableWindow( CurSel != LB_ERR );
	GetDlgItem( IDC_BTN_RENAMETYPE )->EnableWindow( CurSel != LB_ERR );

	if( IsPS2View() )
	{
		EnablePS2Controls( CurSel != LB_ERR );
	}
	else if( IsGCView() )
	{
		EnableGCControls( CurSel != LB_ERR );
	}
	else if( IsXBOXView() )
	{
		EnableXBOXControls( CurSel != LB_ERR );
	}
	else if( IsPCView() )
	{
		EnablePCControls( CurSel != LB_ERR );
	}

	if( CurSel == LB_ERR )
		return;

	// fill in control data with current file type selection
	this->GetDataFromControls();

	m_CurFType = CurSel;

	this->SetControlsFromData();
}

//==========================================================================

void CMemCardWizDlg::OnRadioViewPs2() 
{
	this->GetDataFromControls();

	m_bCurView = MEMCARD_TYPE_PS2;

	EnableIconControls( TRUE );

	ShowGCControls( FALSE );
	ShowXBOXControls( FALSE );
	ShowPCControls( FALSE );
	ShowPS2Controls( TRUE );

	this->SetControlsFromData();
}

//==========================================================================

void CMemCardWizDlg::OnRadioViewGc() 
{
	this->GetDataFromControls();

	m_bCurView = MEMCARD_TYPE_GAMECUBE;

	EnableIconControls( TRUE );

	ShowPS2Controls( FALSE );
	ShowXBOXControls( FALSE );
	ShowPCControls( FALSE );
	ShowGCControls( TRUE );

	this->SetControlsFromData();
}

//==========================================================================

void CMemCardWizDlg::OnRadioViewXbox() 
{
	this->GetDataFromControls();

	m_bCurView = MEMCARD_TYPE_XBOX;

	EnableIconControls( TRUE );

	ShowPS2Controls( FALSE );
	ShowGCControls( FALSE );
	ShowPCControls( FALSE );
	ShowXBOXControls( TRUE );

	this->SetControlsFromData();
}
//==========================================================================

void CMemCardWizDlg::OnRadioViewPC() 
{
	this->GetDataFromControls();

	m_bCurView = MEMCARD_TYPE_PC;

	EnableIconControls( TRUE );

	ShowPS2Controls( FALSE );
	ShowGCControls( FALSE );
	ShowXBOXControls( FALSE );
	ShowPCControls( TRUE );

	this->SetControlsFromData();
}

//==========================================================================

void CMemCardWizDlg::OnBtnIconBrowse() 
{
	const char FILE_OPEN_FILTER[] = "All Files|*.*||";
    const long FILE_DLG_FLAGS = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

    CString IconFilename;

    CFileDialog OpenDlg( TRUE,              //Open file dialog
                         NULL,              //Default extension
                         NULL,              //initial filename
                         FILE_DLG_FLAGS,    //Flags
                         FILE_OPEN_FILTER,  //Filter
                         this );            //Parent Wnd

    if( IDCANCEL == OpenDlg.DoModal() )
        return;

    IconFilename = OpenDlg.GetPathName();

	GetDlgItem( IDC_EDIT_ICON_FILE )->SetWindowText( IconFilename );

	this->GetDataFromControls();
}

//==========================================================================

void CMemCardWizDlg::OnBtnIconEdit() 
{
	if( this->IsPS2View() )
	{
		CPS2IconPropDlg PS2IconDlg( this );

		PS2IconDlg.SetIconInfo( m_MCProj.m_PS2.m_IconInfo );

		if( IDCANCEL == PS2IconDlg.DoModal() )
			return;

		PS2IconDlg.GetIconInfo( m_MCProj.m_PS2.m_IconInfo );
	}
	else if( this->IsGCView() )
	{
		CGCIconPropDlg GCIconDlg( this );

		GCIconDlg.SetIconInfo( m_MCProj.m_GC.m_IconInfo );

		if( IDCANCEL == GCIconDlg.DoModal() )
			return;

		GCIconDlg.GetIconInfo( m_MCProj.m_GC.m_IconInfo );
	}
	else if( this->IsXBOXView() )
	{
		// XBOX has no icon info
		//CXBOXIconPropDlg XBOXIconDlg( this );

		//XBOXIconDlg.SetIconInfo( m_MCProj.m_XBOX.m_IconInfo );

		//if( IDCANCEL == XBOXIconDlg.DoModal() )
		//	return;

		//XBOXIconDlg.GetIconInfo( m_MCProj.m_XBOX.m_IconInfo );
	}
	else if( this->IsPCView() )
	{
	}
}

//==========================================================================

void CMemCardWizDlg::OnDeltaposSpinPs2Nfiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int NewPos = pNMUpDown->iPos + pNMUpDown->iDelta;

	if( this->IsPS2View() )
		PS2_UpdateMaxFile( NewPos );

	*pResult = 0;

	this->GetDataFromControls();
}

//==========================================================================

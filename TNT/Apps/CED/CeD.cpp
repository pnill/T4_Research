// CeD.cpp : Defines the class behaviors for the application.
//
#define _WIN32_WINNT
#include <windows.h>
#include "stdafx.h"
#include "CeD.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "CeDDoc.h"
#include "CeDView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "ProjectFrame.h"
#include "ProjectView.h"

#include "x_files.hpp"

/////////////////////////////////////////////////////////////////////////////
// CCeDApp

BEGIN_MESSAGE_MAP(CCeDApp, CWinApp)
	//{{AFX_MSG_MAP(CCeDApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_ENABLE_D3D, OnEnableD3d)
	ON_UPDATE_COMMAND_UI(ID_ENABLE_D3D, OnUpdateEnableD3d)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCeDApp construction

CCeDApp::CCeDApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCeDApp object

CCeDApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCeDApp initialization

BOOL CCeDApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

    x_Init();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Iguana Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_CEDTYPE,
		RUNTIME_CLASS(CCeDDoc),
		RUNTIME_CLASS(CProjectFrame), // custom MDI child frame
		RUNTIME_CLASS(CProjectView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

#ifdef CED_D3D
	// Get D3D hardware caps for reference
	CD3DView::BuildDeviceList();
#endif

	// The main window has been initialized, so show and update it.
	m_nCmdShow = SW_SHOWMAXIMIZED ;
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CCeDApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CCeDApp commands

int CCeDApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	x_KillSTDIO( ) ;
	x_MemoryKill( ) ;

	return CWinApp::ExitInstance();
}

BOOL CCeDApp::OnIdle(LONG lCount) 
{
#ifndef NO_TIMESTAMP_CHECKING
	int NumDocuments = this->m_pDocManager->GetOpenDocumentCount();
	POSITION TemplatePos = m_pDocManager->GetFirstDocTemplatePosition();
	CDocTemplate* pDocTemplate;
	CCeDDoc* pDocument;
	POSITION DocPos;

	while( TemplatePos )
	{
		pDocTemplate = m_pDocManager->GetNextDocTemplate( TemplatePos );
		DocPos = pDocTemplate->GetFirstDocPosition();

		while( DocPos )
		{
			pDocument = (CCeDDoc*)pDocTemplate->GetNextDoc( DocPos );
			if( pDocument->CheckFileChange() )
			{
				int ret = m_pMainWnd->MessageBox( "Source Document changed. Do you want to reload?", "FILE CHANGED EXTERNALY", MB_ICONWARNING|MB_YESNO );
				if( ret==IDYES )
				{
					CString PathName = pDocument->GetPathName();
					pDocument->SetModifiedFlag( FALSE );
					pDocument->OnCloseDocument();
					OpenDocumentFile( PathName );
				}
			}
		}

	}
#endif
	
	return CWinApp::OnIdle(lCount);
}

//==========================================================================
void CCeDApp::OnEnableD3d() 
{
	if( m_bD3DEnable )
	{
		m_bD3DEnable = FALSE;
	}
	else
	{
		m_bD3DEnable = TRUE;
	}
	EnableViewD3D( m_bD3DEnable );
}

//==========================================================================
void CCeDApp::OnUpdateEnableD3d(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bD3DEnable ? TRUE : FALSE );
}

//==========================================================================
void CCeDApp::EnableViewD3D( bool bEnableDisable )
{
	POSITION pos = this->GetFirstDocTemplatePosition();

	while( pos )
	{
		CDocTemplate* pDocTemplate = GetNextDocTemplate( pos );
		POSITION docpos = pDocTemplate->GetFirstDocPosition();
		while( docpos )
		{
			CCeDDoc* pDoc = (CCeDDoc*) pDocTemplate->GetNextDoc( docpos );
			pDoc->EnableViewD3D( bEnableDisable );
		}
	}
}


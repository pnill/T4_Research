// RollupBar.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "RollupBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "RollupPane.h"

/////////////////////////////////////////////////////////////////////////////
// cribbed from AFXIMPL.H

extern const TCHAR _afxWndControlBar[]; // controls with grey backgrounds

//#define AfxDeferRegisterClass(fClass) \
//	((afxRegisteredClasses & fClass) ? TRUE : AfxEndDeferRegisterClass(fClass))

//extern BOOL AFXAPI AfxEndDeferRegisterClass(short fClass);

//#define AFX_WNDCOMMCTLS_REG     (0x0010)

/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
// CRollupBar

CRollupBar::CRollupBar( )
{
	m_Width = 128 ;
}

CRollupBar::~CRollupBar()
{
	//---	Taken from CDialogBar
	DestroyWindow() ;
}

/////////////////////////////////////////////////////////////////////////////
// SetWidth

void CRollupBar::SetWidth( int Width )
{
	m_Width = Width+4 ;
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		CFrameWnd *pFrame = (CFrameWnd*)GetParent() ;
		ASSERT( pFrame->IsKindOf( RUNTIME_CLASS( CFrameWnd ) ) ) ;
		pFrame->RecalcLayout( ) ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// AddPaneDialog

CRollupPane *CRollupBar::AddPaneDialog( CString Name, UINT nTemplateID, CDialog *pDialog )
{
	return m_RollupCtrl.AddPaneDialog( Name, nTemplateID, pDialog ) ;
}

/////////////////////////////////////////////////////////////////////////////
// Create

bool CRollupBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	ASSERT(pParentWnd != NULL);

	//---	Initialize common controls
//	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));

	//---	Allow chance to modify styles
	dwStyle |= CBRS_BORDER_3D ;
	m_dwStyle = dwStyle ;
	CREATESTRUCT cs ;
	memset( &cs, 0, sizeof( cs ) ) ;
	cs.lpszClass	= _afxWndControlBar;
	cs.style		= (DWORD)dwStyle | WS_CLIPCHILDREN|WS_VISIBLE ;
	cs.dwExStyle	= WS_EX_CLIENTEDGE ;
	cs.hMenu		= (HMENU)nID ;
	cs.hInstance	= AfxGetInstanceHandle( ) ;
	cs.hwndParent	= pParentWnd->GetSafeHwnd( ) ;
	if( !PreCreateWindow( cs ) )
		return FALSE ;

	//---	Create the Bar Window & the Rollup Control
	CRect r( 0, 0, m_Width, 0 ) ;
	CWnd::Create( cs.lpszClass, NULL, cs.style, r, pParentWnd, nID, NULL ) ;
	m_RollupCtrl.Create( NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, 0 ) ;

	//---	Return Success
	return TRUE ;
}

/////////////////////////////////////////////////////////////////////////////
// CControlBar overrides

void CRollupBar::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
{
}

CSize CRollupBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CRect r ;
	GetParent()->GetClientRect( &r ) ;
	CSize sz = CSize( m_Width, 0 ) ;
	if( bStretch &&  bHorz ) sz.cx = r.Width() ;
	if( bStretch && !bHorz ) sz.cy = r.Height() ;
	return sz ;
}

CSize CRollupBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	CRect r ;
	GetParent()->GetClientRect( &r ) ;
	CSize sz = CSize( m_Width, 0 ) ;
	if(  (dwMode & LM_HORZ) && (dwMode & LM_HORZDOCK) ) sz.cx = r.Width() ;
	if( !(dwMode & LM_HORZ) && (dwMode & LM_VERTDOCK) ) sz.cy = r.Height() ;
	return sz ;
}





BEGIN_MESSAGE_MAP(CRollupBar, CControlBar)
	//{{AFX_MSG_MAP(CRollupBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRollupBar message handlers



void CRollupBar::OnSize(UINT nType, int cx, int cy) 
{
	CControlBar::OnSize(nType, cx, cy);

	//---	Make CRollupCtrl track size of CRollupBar
	if( ::IsWindow(m_RollupCtrl.GetSafeHwnd()) )
	{
		m_RollupCtrl.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER ) ;
	}
}

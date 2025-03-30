// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "CeD.h"

#include "ChildFrm.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "CeDDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_CLOSE()
	ON_WM_MDIACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

//	cs.dwExStyle |= WS_EX_TOOLWINDOW ;

	return CMDIChildWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

void CChildFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default

	//---	Tell Document this Frame Window is closing
	CCeDDoc	*pDoc = (CCeDDoc*)GetActiveDocument() ;
	if( pDoc )
	{
		pDoc->ClosingFrameWindow( this ) ;
	}

	CMDIChildWnd::OnClose();
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	
	// TODO: Add your message handler code here
	
	//---	Tell Document it is becoming Active
	if( bActivate )
	{
		CCeDDoc	*pDoc = (CCeDDoc*)GetActiveDocument() ;
		if( pDoc )
		{
			pDoc->ActivateDocument() ;
		}
	}
}

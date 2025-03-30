// ProjectFrame.cpp : implementation file
//

#include "stdafx.h"
#include "CeD.h"
#include "ProjectFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Skeleton.h"

/////////////////////////////////////////////////////////////////////////////
// CProjectFrame

IMPLEMENT_DYNCREATE(CProjectFrame, CChildFrame)

CProjectFrame::CProjectFrame()
{
}

CProjectFrame::~CProjectFrame()
{
}


BEGIN_MESSAGE_MAP(CProjectFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CProjectFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectFrame message handlers

int CProjectFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if( CMDIChildWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1 ;

	return 0 ;
}

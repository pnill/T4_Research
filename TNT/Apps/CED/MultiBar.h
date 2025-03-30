// MultiBar.h: interface for the CMultiBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTIBAR_H__4189EE81_02F4_11D2_9507_00207811EE70__INCLUDED_)
#define AFX_MULTIBAR_H__4189EE81_02F4_11D2_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "RollupBar.h"
#include "DlgLayer.h"
#include "DlgActorList.h"
#include "DlgKey.h"
#include "DlgShotList.h"
#include "DlgMoviePreview.h"

class CMultiBar : public CRollupBar  
{
public:
	CMovieView		    *m_pView ;
	CTabCtrl		    m_Tab ;
	CImageList		    m_TabImages ;
	int					m_LastSelection ;

	CDlgLayer			m_DlgLayer ;			// Layer Dialog
	CDlgActorList	    m_DlgActorList ;		// Actor List Dialog
	CDlgKey			    m_DlgKey ;				// Key Dialog
	CDlgShotList	    m_DlgShotList ;			// Shot List Dialog
    CDlgMoviePreview    m_DlgMoviePreview ;     // Movie Preview
public:
	CMultiBar();
	virtual ~CMultiBar();

	//---	Create Function
	bool		Create				( CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | CBRS_RIGHT, UINT nID = 0 ) ;

	void		SetupCharacter		( ) ;
	void		SetupCamera			( ) ;
	void		SetupLight			( ) ;
	void		SetupScenery		( ) ;
	void		SetupShotList		( ) ;
	void		SetupPreview		( ) ;

	void		DoneCharacter		( ) ;
	void		DoneCamera			( ) ;
	void		DoneLight			( ) ;
	void		DoneScenery			( ) ;
	void		DoneShotList		( ) ;
	void		DonePreview			( ) ;

	CDlgLayer		*CMultiBar::GetDlgLayer			( ) ;
	CDlgActorList	*CMultiBar::GetDlgActorList		( ) ;
	CDlgKey			*CMultiBar::GetDlgKey			( ) ;

	//{{AFX_MSG(CMultiBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	//}}AFX_MSG

	afx_msg void TabSelchange( NMHDR * pNotifyStruct, LRESULT * result ) ;

	DECLARE_MESSAGE_MAP()

};

#endif // !defined(AFX_MULTIBAR_H__4189EE81_02F4_11D2_9507_00207811EE70__INCLUDED_)

// CeDView.h : interface of the CCeDView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CEDVIEW_H__68F53CF3_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_CEDVIEW_H__68F53CF3_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CeDDoc.h"
#include "Camera.h"
#include "RenderContext.h"


class CCeDView : public CView
{
protected: // create from serialization only
	CCeDView();
	DECLARE_DYNCREATE(CCeDView)

// Attributes
public:
	CCeDDoc* GetDocument();

	CDC			*m_pBackDC ;
	CBitmap		*m_pBackBitmap ;
	CBitmap		*m_pBackOldBitmap ;
	CRect		m_BackRect ;

	CCamera		m_Camera ;
	CPoint		m_MouseLast ;
	CPoint		m_MouseDelta ;
	bool		m_MouseOverItem ;

	int			m_ViewState ;

// Operations
public:

	void CreateBackBuffer( CDC *pDC ) ;
	virtual void UpdateMenuItems( CMenu *pMenu ) ;
	
	void GetCursorRay( vector3d *pRayBase, vector3d *pRay, CPoint p ) ;
	s32 GetViewState( void ) { return m_ViewState; }
	xbool MouseMoved( void ) { return ( m_MouseDelta.x || m_MouseDelta.y ) ? TRUE : FALSE; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeDView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCeDView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCeDView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CeDView.cpp
inline CCeDDoc* CCeDView::GetDocument()
   { return (CCeDDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEDVIEW_H__68F53CF3_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_)

// CeDView.h : interface of the CCeDView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CEDVIEW_H__68F53CF3_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_CEDVIEW_H__68F53CF3_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CeDDoc.h"
#include "CeDView.h"
#include "Camera.h"
#include "RenderContext.h"

#ifdef CED_D3D
#include "D3DView.h"
#include "D3D8.h"
#endif

#ifdef CED_D3D
class CRenderContextD3D;
#endif


#define	MOD_COLOR			0.85f

#define	GRID_COLOR_MINOR	RGB(MOD_COLOR*224,MOD_COLOR*255,MOD_COLOR*224)
#define	GRID_COLOR_MAJOR	RGB(MOD_COLOR*128,MOD_COLOR*255,MOD_COLOR*128)
#define	BACKGROUND_COLOR	RGB(MOD_COLOR*255,MOD_COLOR*255,MOD_COLOR*255)



#ifdef CED_D3D
class CCeDView : public CD3DView
#else
class CCeDView : public CView
#endif
{
protected: // create from serialization only
	CCeDView();
	DECLARE_DYNCREATE(CCeDView)

// Attributes
public:
	CCeDDoc* GetDocument();

	CDC					*m_pBackDC ;
	CBitmap				*m_pBackBitmap ;
	CBitmap				*m_pBackOldBitmap ;
	CRect				m_BackRect ;
						
	CCamera				m_Camera ;
	CPoint				m_MouseLast ;
	CPoint				m_MouseDelta ;
	bool				m_MouseOverItem ;
						
	int					m_ViewState ;

#ifdef CED_D3D
	CRenderContextD3D*	m_pD3DRC;
#endif


// Operations
public:

	void CreateBackBuffer( CDC *pDC ) ;
	virtual void UpdateMenuItems( CMenu *pMenu ) ;
	
	void GetCursorRay( vector3  *pRayBase, vector3  *pRay, CPoint p ) ;
	s32 GetViewState( void ) { return m_ViewState; }
	xbool MouseMoved( void ) { return ( m_MouseDelta.x || m_MouseDelta.y ) ? TRUE : FALSE; }

	//---	draw the view
	virtual void		DrawView		( CRenderContext *pRC );

	virtual void ReleaseDevices			( void );
	virtual void AquireDevices			( void );

	virtual bool GetD3DPermission		( void );	// used to ask the application if D3D is allowed at startup

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeDView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
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
	afx_msg void OnMove(int x, int y);
	afx_msg void OnDestroy();
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

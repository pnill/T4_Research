#if !defined(AFX_PROJECTVIEW_H__FB5CF722_E2CA_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_PROJECTVIEW_H__FB5CF722_E2CA_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ProjectView.h : header file
//

#include <afxcview.h>

class CCeDDoc ;

/////////////////////////////////////////////////////////////////////////////
// CProjectView view

class CProjectView : public CTreeView
{
protected:
	CProjectView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProjectView)

// Attributes
public:
	CImageList		m_ImageList ;

// Operations
public:
	CCeDDoc*	GetDocument();

	void		TreeRebuild( ) ;
	void		TreeAddMotions( CTreeCtrl &Tree, HTREEITEM hParent, CCharacter *pCharacter ) ;
	void		TreeAddActors( CTreeCtrl &Tree, HTREEITEM hParent, CMovie *pMovie ) ;

	void		UpdateAddCharacter( ) ;
	void		UpdateAddMotion( ) ;
	void		UpdateDeleteMotion( ) ;
	void		UpdateAddMovie( ) ;
	void		UpdateAddActor( ) ;
	void		UpdateDeleteActor( ) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CProjectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CProjectView)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCharacterNew();
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMovieNew();
	afx_msg void OnCharacterDelete();
	afx_msg void OnMovieDelete();
	afx_msg void OnMeshNew();
	afx_msg void OnMeshDelete();
	afx_msg void OnCharacterLoadmotions();
	afx_msg void OnCharacterExport();
	afx_msg void OnCharacterImportSkel();
	afx_msg void OnCharacterImportMotion();
	afx_msg void OnUpdateMenuCharacterOption(CCmdUI* pCmdUI);
	afx_msg void OnMovieExport();
	afx_msg void OnMovieExportproperties();
	afx_msg void OnCharacterImportSkin();
	afx_msg void OnCharacterFacedirrecipe();
	afx_msg void OnCharacterSkins();
	afx_msg void OnUpdateCharacterSkins(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCharacterFacedirrecipe(CCmdUI* pCmdUI);
	afx_msg void OnCharacterExportall();
	afx_msg void OnUpdateCharacterExportall(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCharacterProperties(CCmdUI* pCmdUI);
	afx_msg void OnCharacterProperties();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CeDView.cpp
inline CCeDDoc* CProjectView::GetDocument()
   { return (CCeDDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTVIEW_H__FB5CF722_E2CA_11D1_9507_00207811EE70__INCLUDED_)

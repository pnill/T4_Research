// AnimTblEdView.h : interface of the CAnimTblEdView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMTBLEDVIEW_H__CE6EAFEF_57C5_11D2_ABF7_00A024569875__INCLUDED_)
#define AFX_ANIMTBLEDVIEW_H__CE6EAFEF_57C5_11D2_ABF7_00A024569875__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

enum
{
	VP_OPTION_NONE=-1,

	VP_OPTION_NAME,
	VP_OPTION_LRID,
	VP_OPTION_PRIORITY,
	VP_OPTION_LINK,
	VP_OPTION_METHOD,
	VP_OPTION_FLAGS,
	VP_OPTION_STATE,
	VP_OPTION_COMMENT,

	NUM_VP_OPTIONS
};



class CAnimTblEdView : public CTreeView
{
protected: // create from serialization only
	CAnimTblEdView();
	DECLARE_DYNCREATE(CAnimTblEdView)

// Attributes
public:
	CAnimTblEdDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimTblEdView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void Serialize(CArchive& ar);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	void ApplyTreeViewProperties( HTREEITEM hItem=(HTREEITEM)0xffffffff );
	int m_VPDisplayOrder[NUM_VP_OPTIONS+1];
	CAnimTreeNode * GetSelectedNode();
	HTREEITEM m_hDropTarget;
	HTREEITEM m_hDragItem;
	void BuildTree(HTREEITEM hParent, CAnimTreeNode *pAnimTreeNode, HTREEITEM insertAfter = TVI_LAST);
	HTREEITEM SearchItem(DWORD ItemData);
	virtual ~CAnimTblEdView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAnimTblEdView)
	afx_msg void OnProjectSourcefiles();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnGroupEdit();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnGroupAddgroup();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnGroupDelete();
	afx_msg void OnGroupAddanimation();
	afx_msg void OnAnimEdit();
	afx_msg void OnAnimDelete();
	afx_msg void OnAnimAddequiv();
	afx_msg void OnEquivEdit();
	afx_msg void OnEquivDelete();
	afx_msg void OnProjectEditstates();
	afx_msg void OnProjectFlags();
	afx_msg void OnProjectPriorities();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnAnimAddtrans();
	afx_msg void OnViewProperties();
	afx_msg void OnProjectMethods();
	afx_msg void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGroupAddtrans();
	afx_msg void OnGroupAddanimations();
	afx_msg void OnAnimScansourcecode();
	afx_msg void OnGroupScansourcecode();
	afx_msg void OnTreeitemIgnore();
	afx_msg void OnUpdateTreeitemIgnore(CCmdUI* pCmdUI);
	afx_msg void OnDependancyDisable();
	afx_msg void OnUpdateDependancyDisable(CCmdUI* pCmdUI);
	afx_msg void OnTreeitemEdit();
	afx_msg void OnUpdateTreeitemEdit(CCmdUI* pCmdUI);
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintSetup();
	afx_msg void OnAnimSwapequiv();
	afx_msg void OnGroupSwapequiv();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HTREEITEM SearchItemR(HTREEITEM CurItem, DWORD ItemData);
};

#ifndef _DEBUG  // debug version in AnimTblEdView.cpp
inline CAnimTblEdDoc* CAnimTblEdView::GetDocument()
   { return (CAnimTblEdDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMTBLEDVIEW_H__CE6EAFEF_57C5_11D2_ABF7_00A024569875__INCLUDED_)

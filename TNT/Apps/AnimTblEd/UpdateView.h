// UpdateView.h: interface for the CUpdateView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPDATEVIEW_H__A342B1C1_78B0_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_UPDATEVIEW_H__A342B1C1_78B0_11D2_9248_00105A29F8F3__INCLUDED_

#include "AnimTreeNode.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUpdateView  
{
public:
	CAnimTreeNode * m_pRelativeNode;
	CAnimTreeNode *m_pUpdateParentNode;
	void UpdateView(CTreeView *pView, HTREEITEM TreeItem, HTREEITEM ParentTreeItem = NULL, HTREEITEM RelativeItem = NULL);
	void SetUpdateAction(int UpdateAction, CAnimTreeNode *pUpdateNode, CAnimTreeNode *pUpdateParentNode = NULL, CAnimTreeNode *pRelativeNode = NULL);
	CAnimTreeNode *m_pUpdateNode;
	int m_UpdateAction;
	CUpdateView();
	virtual ~CUpdateView();

};

enum
{
	UPDATEVIEW_ACTION_NONE,
	UPDATEVIEW_ACTION_REBUILDTREE,
	UPDATEVIEW_ACTION_ADDITEM,
	UPDATEVIEW_ACTION_DELETEITEM,
	UPDATEVIEW_ACTION_UPDATEITEM,
};

#endif // !defined(AFX_UPDATEVIEW_H__A342B1C1_78B0_11D2_9248_00105A29F8F3__INCLUDED_)

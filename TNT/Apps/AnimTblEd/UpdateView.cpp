// UpdateView.cpp: implementation of the CUpdateView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "AnimTblEdView.h"

#include "UpdateView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUpdateView::CUpdateView()
{
	m_UpdateAction = UPDATEVIEW_ACTION_NONE;
	m_pUpdateNode = NULL;
}

CUpdateView::~CUpdateView()
{

}

void CUpdateView::SetUpdateAction(int UpdateAction, CAnimTreeNode *pUpdateNode, CAnimTreeNode *pUpdateParentNode, CAnimTreeNode *pRelativeNode)
{
	ASSERT( pUpdateNode );
	ASSERT( ( UpdateAction != UPDATEVIEW_ACTION_ADDITEM ) || pUpdateParentNode );

	m_UpdateAction = UpdateAction;
	m_pUpdateNode = pUpdateNode;
	m_pUpdateParentNode = pUpdateParentNode;
	m_pRelativeNode = pRelativeNode;
}

void CUpdateView::UpdateView(CTreeView *pView, HTREEITEM TreeItem, HTREEITEM ParentTreeItem, HTREEITEM RelativeItem)
{
	switch (m_UpdateAction)
	{
	case UPDATEVIEW_ACTION_NONE :
		//---	Do nothing.
		break;

	case UPDATEVIEW_ACTION_REBUILDTREE :
		//---	Rebuild the entire tree view from the document data.
		pView->GetTreeCtrl().DeleteAllItems();
		((CAnimTblEdView*)pView)->BuildTree( NULL, &(((CAnimTblEdView*)pView)->GetDocument()->m_AnimTree) );
		break;

	case UPDATEVIEW_ACTION_ADDITEM :
		//---	Add an item to a group.
		if (!RelativeItem)
			RelativeItem = TVI_FIRST;

		((CAnimTblEdView*)pView)->BuildTree( ParentTreeItem, m_pUpdateNode, RelativeItem );
		break;

	case UPDATEVIEW_ACTION_DELETEITEM :
		//---	Delete the item from the tree.
		pView->GetTreeCtrl().DeleteItem( TreeItem );
		break;

	case UPDATEVIEW_ACTION_UPDATEITEM :
		//---	Set the text for a tree item.
		pView->GetTreeCtrl().SetItemText( TreeItem, m_pUpdateNode->GetVPString( ((CAnimTblEdView*)pView)->m_VPDisplayOrder ) );
		pView->GetTreeCtrl().SetItemImage( TreeItem, m_pUpdateNode->GetImage(), m_pUpdateNode->GetSelectedImage() );
		pView->GetTreeCtrl().SetItemState( TreeItem, INDEXTOSTATEIMAGEMASK(m_pUpdateNode->GetStateImage()), TVIS_STATEIMAGEMASK );
		break;
	}
}

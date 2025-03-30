// AnimTblEdView.cpp : implementation of the CAnimTblEdView class
//

#include "stdafx.h"
#include "AnimTblEd.h"

#include "SourceFilesDlg.h"

#include "AnimTblEdDoc.h"
#include "AnimTblEdView.h"
#include "StateBitsEdit.h"
#include "FlagsEditDlg.h"
#include "PrioritiesEdit.h"
#include "AnimTblEdDefs.h"
#include "ViewPropertiesDlg.h"
#include "MethodEditDlg.h"
#include "SourceIDDlg.h"
#include "AnimationEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MEMORYLEAK 0

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdView

IMPLEMENT_DYNCREATE(CAnimTblEdView, CTreeView)

BEGIN_MESSAGE_MAP(CAnimTblEdView, CTreeView)
	//{{AFX_MSG_MAP(CAnimTblEdView)
	ON_COMMAND(ID_PROJECT_SOURCEFILES, OnProjectSourcefiles)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_GROUP_EDIT, OnGroupEdit)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_GROUP_ADDGROUP, OnGroupAddgroup)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_GROUP_DELETE, OnGroupDelete)
	ON_COMMAND(ID_GROUP_ADDANIMATION, OnGroupAddanimation)
	ON_COMMAND(ID_ANIM_EDIT, OnAnimEdit)
	ON_COMMAND(ID_ANIM_DELETE, OnAnimDelete)
	ON_COMMAND(ID_ANIM_ADDEQUIV, OnAnimAddequiv)
	ON_COMMAND(ID_EQUIV_EDIT, OnEquivEdit)
	ON_COMMAND(ID_EQUIV_DELETE, OnEquivDelete)
	ON_COMMAND(ID_PROJECT_EDITSTATES, OnProjectEditstates)
	ON_COMMAND(ID_PROJECT_FLAGS, OnProjectFlags)
	ON_COMMAND(ID_PROJECT_PRIORITIES, OnProjectPriorities)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_KEYUP()
	ON_COMMAND(ID_ANIM_ADDTRANS, OnAnimAddtrans)
	ON_COMMAND(ID_VIEW_PROPERTIES, OnViewProperties)
	ON_COMMAND(ID_PROJECT_METHODS, OnProjectMethods)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemexpanding)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemexpanded)
	ON_COMMAND(ID_GROUP_ADDTRANS, OnGroupAddtrans)
	ON_COMMAND(ID_GROUP_ADDANIMATIONS, OnGroupAddanimations)
	ON_COMMAND(ID_ANIM_SCANSOURCECODE, OnAnimScansourcecode)
	ON_COMMAND(ID_GROUP_SCANSOURCECODE, OnGroupScansourcecode)
	ON_COMMAND(ID_TREEITEM_IGNORE, OnTreeitemIgnore)
	ON_UPDATE_COMMAND_UI(ID_TREEITEM_IGNORE, OnUpdateTreeitemIgnore)
	ON_COMMAND(ID_DEPENDANCY_DISABLE, OnDependancyDisable)
	ON_UPDATE_COMMAND_UI(ID_DEPENDANCY_DISABLE, OnUpdateDependancyDisable)
	ON_COMMAND(ID_TREEITEM_EDIT, OnTreeitemEdit)
	ON_UPDATE_COMMAND_UI(ID_TREEITEM_EDIT, OnUpdateTreeitemEdit)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_SETUP, OnFilePrintSetup)
	ON_COMMAND(ID_ANIM_SWAPEQUIV, OnAnimSwapequiv)
	ON_COMMAND(ID_GROUP_SWAPEQUIV, OnGroupSwapequiv)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdView construction/destruction

CAnimTblEdView::CAnimTblEdView()
{
	// TODO: add construction code here
	m_hDragItem = NULL;
	m_hDropTarget = NULL;

	for(int i=0; i<(NUM_VP_OPTIONS+1); i++)
		m_VPDisplayOrder[i] = VP_OPTION_NONE;

	m_VPDisplayOrder[0] = VP_OPTION_NAME;
}

CAnimTblEdView::~CAnimTblEdView()
{
}

BOOL CAnimTblEdView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdView drawing

void CAnimTblEdView::OnDraw(CDC* pDC)
{
	CAnimTblEdDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

void CAnimTblEdView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	// TODO: You may populate your TreeView with items by directly accessing
	//  its tree control through a call to GetTreeCtrl().
	BuildTree( NULL, &(GetDocument()->m_AnimTree) );
}

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdView printing

BOOL CAnimTblEdView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CAnimTblEdView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CAnimTblEdView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdView diagnostics

#ifdef _DEBUG
void CAnimTblEdView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CAnimTblEdView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CAnimTblEdDoc* CAnimTblEdView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAnimTblEdDoc)));
	return (CAnimTblEdDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdView message handlers


void CAnimTblEdView::OnProjectSourcefiles() 
{
	CSourceFilesDlg	SourceFilesDlg;
	CString			FileName;
	POSITION		ListPos;
	CProjFile		*pProjFile;
	
	//---	Load the files into the dialog.
	SourceFilesDlg.m_FilesString = "";
	ListPos = GetDocument()->m_ProjFileList.GetHeadPosition();
	while (ListPos)
	{
		pProjFile = GetDocument()->m_ProjFileList.GetNext(ListPos);

		SourceFilesDlg.m_FilesString += pProjFile->GetFileName();
		SourceFilesDlg.m_FilesString += "|";
	}

	//---	Open editor dialog for current source (.h) files.
	if ((SourceFilesDlg.DoModal() == IDOK) && SourceFilesDlg.m_Modified)
	{
		//---	Destroy the current file list.
		GetDocument()->RemoveAllSourceProjFiles();

		//---	Update the current source files for the project.
		while (!SourceFilesDlg.m_FilesString.IsEmpty())
		{
			SourceFilesDlg.m_FilesString.TrimLeft();
			FileName = SourceFilesDlg.m_FilesString.SpanExcluding( "|" );
			GetDocument()->AddSourceProjFile(FileName);
			SourceFilesDlg.m_FilesString.Delete(0, FileName.GetLength()+1);
			SourceFilesDlg.m_FilesString.TrimLeft();
		}
	}

	GetDocument()->BuildIDList();
}


int CAnimTblEdView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
#if !MEMORYLEAK
	//---	Initialize the Tree.
	CImageList	*pImageList = new CImageList;
	pImageList->Create( IDR_TREEICONS, 16, 20, (128<<16)+(128<<8)+128 );
	GetTreeCtrl().SetImageList( pImageList, TVSIL_NORMAL );

	pImageList = new CImageList;
	pImageList->Create( IDR_STATUSICONS, 16, 15, (128<<16)+(128<<8)+128 );
	GetTreeCtrl().SetImageList( pImageList, TVSIL_STATE );
#endif	
	return 0;
}

void CAnimTblEdView::OnDestroy() 
{
	CTreeView::OnDestroy();
#if !MEMORYLEAK
	// TODO: Add your message handler code here
	CImageList *pImageList;
	pImageList = GetTreeCtrl().SetImageList( NULL, TVSIL_NORMAL );
	delete pImageList;

	pImageList = GetTreeCtrl().SetImageList( NULL, TVSIL_STATE );
	delete pImageList;

#endif
//	if (!GetTreeCtrl().DeleteAllItems())
//	{
//		ASSERT(0);
//	}
}

void CAnimTblEdView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	HTREEITEM		TreeNode;
	CAnimTreeNode	*pAnimTreeNode;
	CMenu			MouseMenu;
	CMenu			*pMenu;
	CRect			WindowRect;
	int				SubMenuIndex;

	TreeNode = GetTreeCtrl().HitTest( point );

	GetTreeCtrl().SelectItem( TreeNode );

	//TreeNode = GetTreeCtrl().HitTest( point );
	CTreeView::OnRButtonUp(nFlags, point);

	if (TreeNode)
	{
		pAnimTreeNode = (CAnimTreeNode *)GetTreeCtrl().GetItemData( TreeNode );
		if (pAnimTreeNode)
		{
			MouseMenu.LoadMenu( IDR_MOUSEMENU );
			SubMenuIndex = pAnimTreeNode->GetMouseMenuIndex();
			if (SubMenuIndex >= 0)
			{
				pMenu = MouseMenu.GetSubMenu( SubMenuIndex );
				this->GetWindowRect( &WindowRect );

				switch (pAnimTreeNode->m_Type)
				{
				case ANIMTREENODE_TYPE_PROJECT:
				case ANIMTREENODE_TYPE_ENUM:
				case ANIMTREENODE_TYPE_UNUSED:
				case ANIMTREENODE_TYPE_GROUP:
						MouseMenu.EnableMenuItem( ID_GROUP_ADDGROUP, pAnimTreeNode->GetAddGroupOptionEnabled() );
						MouseMenu.EnableMenuItem( ID_GROUP_ADDANIMATION, pAnimTreeNode->GetAddAnimOptionEnabled() );
						MouseMenu.EnableMenuItem( ID_GROUP_EDIT, pAnimTreeNode->GetEditOptionEnabled() );
						MouseMenu.EnableMenuItem( ID_GROUP_DELETE, pAnimTreeNode->GetDeleteOptionEnabled() );
                        MouseMenu.EnableMenuItem( ID_GROUP_SCANSOURCECODE, pAnimTreeNode->GetScanSourceCodeOptionEnabled() );
					break;
				case ANIMTREENODE_TYPE_ANIM:
						MouseMenu.EnableMenuItem( ID_ANIM_ADDEQUIV, pAnimTreeNode->GetAddTransOptionEnabled() );
						MouseMenu.EnableMenuItem( ID_ANIM_EDIT, pAnimTreeNode->GetEditOptionEnabled() );
						MouseMenu.EnableMenuItem( ID_ANIM_DELETE, pAnimTreeNode->GetDeleteOptionEnabled() );
                        MouseMenu.EnableMenuItem( ID_ANIM_SCANSOURCECODE, pAnimTreeNode->GetScanSourceCodeOptionEnabled() );
					break;
				case ANIMTREENODE_TYPE_TRANS:
						MouseMenu.EnableMenuItem( ID_EQUIV_EDIT, pAnimTreeNode->GetEditOptionEnabled() );
						MouseMenu.EnableMenuItem( ID_EQUIV_DELETE, pAnimTreeNode->GetDeleteOptionEnabled() );
					break;
				default:
					ASSERT( 0 );
				}

				pMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x + WindowRect.left, point.y + WindowRect.top, this );
			}
		}
	}

//	CTreeView::OnRButtonDown(nFlags, point);
}


void CAnimTblEdView::OnGroupEdit() 
{
	HTREEITEM		TreeNode;
	CAnimTreeNode	*pAnimTreeNode;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pAnimTreeNode = (CAnimTreeNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pAnimTreeNode)
		return;

	if (pAnimTreeNode->Edit())
	{
		GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_UPDATEITEM, pAnimTreeNode, pAnimTreeNode->GetParent() );
		GetDocument()->UpdateAllViews( NULL );
	}
//	else
//	{
//		this->MessageBox( "Cannot Edit this group.", "Error", MB_ICONERROR);
//	}
}

void CAnimTblEdView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CTreeView::OnRButtonUp(nFlags, point);
}

void CAnimTblEdView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	HTREEITEM	SelectedNode;
	HTREEITEM	HitNode;

	SelectedNode = GetTreeCtrl().GetSelectedItem();
	HitNode = GetTreeCtrl().HitTest( point );

	if (SelectedNode == HitNode)
		OnGroupEdit();
	
	//CTreeView::OnLButtonDblClk(nFlags, point);
}

void CAnimTblEdView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	HTREEITEM	TreeItem = NULL;
	HTREEITEM	ParentTreeItem = NULL;
	HTREEITEM	RelativeItem = NULL;

	//---	Search for the relavent tree items.
	TreeItem = SearchItem( (DWORD)GetDocument()->m_Update.m_pUpdateNode );
	ParentTreeItem = SearchItem( (DWORD)GetDocument()->m_Update.m_pUpdateParentNode );
	if ((DWORD)GetDocument()->m_Update.m_pRelativeNode == 0xffffffff)
		RelativeItem = TVI_LAST;
	else
		RelativeItem = SearchItem( (DWORD)GetDocument()->m_Update.m_pRelativeNode );

	//---	Let the CUpdate object update the view.
	GetDocument()->m_Update.UpdateView( this, TreeItem, ParentTreeItem, RelativeItem );
}

HTREEITEM CAnimTblEdView::SearchItem(DWORD ItemData)
{
	if (!ItemData)
		return NULL;

	return SearchItemR( this->GetTreeCtrl().GetRootItem(), ItemData );
}

HTREEITEM CAnimTblEdView::SearchItemR(HTREEITEM CurItem, DWORD ItemData)
{
	static HTREEITEM	SearchItem;

	//---	Loop through siblings searching on each child.
	while (CurItem && ((this->GetTreeCtrl().GetItemData( CurItem ) != ItemData)))
	{
		if (SearchItem = SearchItemR( this->GetTreeCtrl().GetChildItem( CurItem ), ItemData ))
			return SearchItem;

		CurItem = this->GetTreeCtrl().GetNextSiblingItem( CurItem );
	}

	return CurItem;
}

//void CAnimTblEdView::OnRButtonDblClk(UINT nFlags, CPoint point) 
//{
	// TODO: Add your message handler code here and/or call default
	
//	CTreeView::OnRButtonDblClk(nFlags, point);
//}

void CAnimTblEdView::OnGroupAddgroup() 
{
	HTREEITEM		TreeNode;
	CGroupNode		*pGroupNode;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pGroupNode = (CGroupNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pGroupNode)
		return;

	//---	Allow the group to be edited.
	CGroupNode		*pNewGroup = new CGroupNode( GetDocument(), pGroupNode );
	if (!pNewGroup->Edit())
	{
		//---	If they cancelled the edit then do not add the group.
		delete pNewGroup;
		return;
	}

	pGroupNode->AddChild( pNewGroup );

	//---	Add the new group and setup the CUpdateView structure.
	GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_ADDITEM, pNewGroup, pGroupNode, (CAnimTreeNode*)0xffffffff );

	//---	Tell all of the views to update.
	GetDocument()->UpdateAllViews( NULL );

	GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_UPDATEITEM, pGroupNode );
	GetDocument()->UpdateAllViews( NULL );
}


void CAnimTblEdView::OnGroupDelete() 
{
	// TODO: Add your command handler code here
	HTREEITEM		TreeNode;
	CAnimTreeNode	*pAnimTreeNode;
	CAnimTreeNode	*pParent;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pAnimTreeNode = (CAnimTreeNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pAnimTreeNode)
		return;

	if (!pAnimTreeNode->HasChildren() || (this->MessageBox( "All contents of this group will be lost.  Continue?", "Warning",  MB_ICONWARNING | MB_YESNO ) == 6))
	{
		//---	Mark the document as having changed.
		GetDocument()->SetModifiedFlag();

		//---	Remove the item's data storage.
		pAnimTreeNode->Remove( GetDocument()->m_IDList );

		//---	Remove the item from the tree.
		pParent = pAnimTreeNode->GetParent();
		if (pParent)
			pParent->RemoveChild( pAnimTreeNode );

		//---	Add the new group and setup the CUpdateView structure.
		GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_DELETEITEM, pAnimTreeNode, pParent );
		GetDocument()->UpdateAllViews( NULL );
		GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_UPDATEITEM, pParent );
		GetDocument()->UpdateAllViews( NULL );
	}
}

void CAnimTblEdView::OnGroupAddanimations() 
{
	HTREEITEM		TreeNode;
	CAnimNode		*pNewAnim;
	CGroupNode		*pGroupNode;
	int				i;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pGroupNode = (CGroupNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pGroupNode)
		return;

	CSourceIDDlg	SourceIDDlg;
	CString			AnimID = CAnimationEditDlg::GetLastLeftID();

	SourceIDDlg.m_AnimID = AnimID;
	SourceIDDlg.m_pIDList = &GetDocument()->m_IDList;
	SourceIDDlg.m_MultipleSelections = TRUE;

	if ((SourceIDDlg.DoModal() == IDOK) && SourceIDDlg.m_Modified)
	{
		for( i=0; i<SourceIDDlg.m_NumSelections; i++ )
		{
			pNewAnim = new CAnimNode( GetDocument(), pGroupNode );

			AnimID = SourceIDDlg.m_AnimIDs[i];
			AnimID.MakeUpper();
			pNewAnim->SetName( AnimID );

			AnimID = SourceIDDlg.m_AnimIDs[i];
			pNewAnim->m_RLIDs.SetLeftID( AnimID, &GetDocument()->m_IDList );
//			pNewAnim->m_RLIDs.SetRightID( AnimID, &GetDocument()->m_IDList );

			//---	Add the new animation to the group.
			pGroupNode->AddChild( pNewAnim );

			//---	Add the new group and setup the CUpdateView structure.
			GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_ADDITEM, pNewAnim, pGroupNode, (CAnimTreeNode*)0xffffffff );

			//---	Tell all of the views to update.
			GetDocument()->UpdateAllViews( NULL );
		}
	}
}

void CAnimTblEdView::OnGroupAddanimation() 
{
	HTREEITEM		TreeNode;
	CAnimNode		*pNewAnim;
	CGroupNode		*pGroupNode;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pGroupNode = (CGroupNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pGroupNode)
		return;

	//---	Allow the group to be edited.
	pNewAnim = new CAnimNode( GetDocument(), pGroupNode );
	if (!pNewAnim->Edit())
	{
		//---	If they cancelled the edit then do not add the group.
		delete pNewAnim;
		return;
	}

	//---	Add the new animation to the group.
	pGroupNode->AddChild( pNewAnim );

	//---	Add the new group and setup the CUpdateView structure.
	GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_ADDITEM, pNewAnim, pGroupNode, (CAnimTreeNode*)0xffffffff );

	//---	Tell all of the views to update.
	GetDocument()->UpdateAllViews( NULL );
}

void CAnimTblEdView::OnAnimEdit() 
{
	//---	There is no difference between this command and the group edit command so I am calling it from here.
	OnGroupEdit();
}

void CAnimTblEdView::OnAnimDelete() 
{
	//---	There is no difference between this command and the group delete command so I am calling it from here.
	OnGroupDelete();	
}

void CAnimTblEdView::BuildTree(HTREEITEM hParent, CAnimTreeNode *pAnimTreeNode, HTREEITEM insertAfter)
{
	HTREEITEM		NewNode;
	CAnimTreeNode	*pChildNode;

	ASSERT (pAnimTreeNode);

	//---	Add the correct node to the tree for the node we currently have.
	NewNode = GetTreeCtrl().InsertItem
	(
		pAnimTreeNode->GetVPString( m_VPDisplayOrder ),
		pAnimTreeNode->GetImage(),
		pAnimTreeNode->GetSelectedImage(),
		hParent,
		insertAfter
	);

	GetTreeCtrl().SetItemData( NewNode, (DWORD)pAnimTreeNode );
	GetTreeCtrl().SetItemState( NewNode, INDEXTOSTATEIMAGEMASK(pAnimTreeNode->GetStateImage()), TVIS_STATEIMAGEMASK );

	if (pAnimTreeNode->HasChildren())
	{
		pChildNode = pAnimTreeNode->GetFirstChild();
		while (pChildNode)
		{
			BuildTree( NewNode, pChildNode );
			pChildNode = pAnimTreeNode->GetNextChild();
		}
	}
}

void CAnimTblEdView::OnAnimAddequiv() 
{
	HTREEITEM		TreeNode;
	CAnimTreeNode	*pNode;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pNode = (CAnimTreeNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pNode)
		return;

	//---	Allow the group to be edited.
	CAnimTrans *pNewDependancy = new CAnimTrans( this->GetDocument(), pNode );
	if (!pNewDependancy->Edit())
	{
		//---	If they cancelled the edit then do not add the group.
		delete pNewDependancy;
		return;
	}

	//---	Add the new Equivalency to the animation.
	pNode->AddChild( pNewDependancy );

	//---	recount the number of dependancies
	if( pNode->m_Type == ANIMTREENODE_TYPE_ANIM )
		((CAnimNode*)pNode)->GetNumDependancies( TRUE );
	else
		((CGroupNode*)pNode)->GetNumDependancies( TRUE );

	//---	Add the new group and setup the CUpdateView structure.
	GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_ADDITEM, pNewDependancy, pNode, (CAnimTreeNode*)0xffffffff );

	//---	Tell all of the views to update.
	GetDocument()->UpdateAllViews( NULL );
}

void CAnimTblEdView::OnEquivEdit() 
{
	//---	There is no difference between this command and the group edit command so I am calling it from here.
	OnGroupEdit();
}

void CAnimTblEdView::OnEquivDelete() 
{
	//---	There is no difference between this command and the group delete command so I am calling it from here.
	OnGroupDelete();	
}

void CAnimTblEdView::OnProjectEditstates() 
{
	CStateBitsEdit	StateBitsEdit;
	
	StateBitsEdit.m_TotalBits			= GetDocument()->m_MaxBitsInBitfield;
	StateBitsEdit.m_StateClassList		= GetDocument()->m_StateClassList;
	StateBitsEdit.m_TotalPriorityBits	= GetDocument()->m_Priorities.GetTotalPriorityBits();
	StateBitsEdit.m_TotalFlagBits		= GetDocument()->m_FlagList.GetTotalFlagBits();
	StateBitsEdit.m_TotalMethodBits		= GetDocument()->m_Methods.GetTotalMethodBits();

	if (StateBitsEdit.DoModal() == IDOK)
	{
		GetDocument()->m_StateClassList = StateBitsEdit.m_StateClassList;

		int operation;
		int stateclass;
		int state;
		int bit;
		int numbits;

		StateBitsEdit.GetChange(operation, stateclass, state, bit, numbits);
		while (operation != 0)
		{
			//---	add/remove the given flag bits from all of the animation data
			GetDocument()->m_StateClassList.ShiftStateClasses(&GetDocument()->m_AnimTree, operation, stateclass, state, bit, numbits);

			//---	get the next change
			StateBitsEdit.GetChange(operation, stateclass, state, bit, numbits);
		}
	}
}

void CAnimTblEdView::OnProjectFlags() 
{
	CFlagsEditDlg	FlagsEdit;
	
	FlagsEdit.m_TotalBits			= GetDocument()->m_MaxBitsInBitfield;
	FlagsEdit.m_FlagList			= GetDocument()->m_FlagList;
	FlagsEdit.m_TotalStateBits		= GetDocument()->m_StateClassList.GetTotalStateBits();
	FlagsEdit.m_TotalPriorityBits	= GetDocument()->m_Priorities.GetTotalPriorityBits();
	FlagsEdit.m_TotalMethodBits		= GetDocument()->m_Methods.GetTotalMethodBits();

	if (FlagsEdit.DoModal() == IDOK)
	{
		GetDocument()->m_FlagList = FlagsEdit.m_FlagList;

		int operation;
		int flag;

		FlagsEdit.GetChange(operation, flag);
		while (operation != 0)
		{
			//---	add/remove the given flag bits from all of the animation data
			GetDocument()->m_FlagList.ShiftFlags(&GetDocument()->m_AnimTree, operation, flag);

			//---	get the next change
			FlagsEdit.GetChange(operation, flag);
		}
	}
}

void CAnimTblEdView::OnProjectPriorities() 
{
	CPrioritiesEdit	PrioritiesEdit;
	
	PrioritiesEdit.m_TotalBits			= GetDocument()->m_MaxBitsInBitfield;
	PrioritiesEdit.m_TotalFlagBits		= GetDocument()->m_FlagList.GetTotalFlagBits();
	PrioritiesEdit.m_TotalStateBits		= GetDocument()->m_StateClassList.GetTotalStateBits();
	PrioritiesEdit.m_Priorities			= GetDocument()->m_Priorities;
	PrioritiesEdit.m_TotalMethodBits	= GetDocument()->m_Methods.GetTotalMethodBits();

	if (PrioritiesEdit.DoModal() == IDOK)
	{
		GetDocument()->m_Priorities = PrioritiesEdit.m_Priorities;

		int operation;
		int priority;

		PrioritiesEdit.GetChange(operation, priority);
		while (operation != 0)
		{
			//---	add/remove the given flag bits from all of the animation data
			GetDocument()->m_Priorities.ShiftPriorities(&GetDocument()->m_AnimTree, operation, priority);

			//---	get the next change
			PrioritiesEdit.GetChange(operation, priority);
		}
	}
}

void CAnimTblEdView::OnProjectMethods() 
{
	CMethodEditDlg	MethodsEdit;
	
	MethodsEdit.m_TotalBits			= GetDocument()->m_MaxBitsInBitfield;
	MethodsEdit.m_TotalFlagBits		= GetDocument()->m_FlagList.GetTotalFlagBits();
	MethodsEdit.m_TotalStateBits	= GetDocument()->m_StateClassList.GetTotalStateBits();
	MethodsEdit.m_TotalPriorityBits	= GetDocument()->m_Priorities.GetTotalPriorityBits();
	MethodsEdit.m_Methods			= GetDocument()->m_Methods;

	if (MethodsEdit.DoModal() == IDOK)
	{
		GetDocument()->m_Methods = MethodsEdit.m_Methods;

		int operation;
		int method;

		MethodsEdit.GetChange(operation, method);
		while (operation != 0)
		{
			//---	add/remove the given flag bits from all of the animation data
			GetDocument()->m_Methods.ShiftMethods(&GetDocument()->m_AnimTree, operation, method);

			//---	get the next change
			MethodsEdit.GetChange(operation, method);
		}
	}
}

void CAnimTblEdView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (m_hDragItem)
	{
		if (!(nFlags & 1))
		{
			GetTreeCtrl().SelectDropTarget(NULL);
			m_hDragItem = NULL;
			m_hDropTarget = NULL;
		}
		else
		{
			HTREEITEM hNewDropTarget = GetTreeCtrl().HitTest(point);

			if (hNewDropTarget == m_hDragItem)
			{
				GetTreeCtrl().SelectDropTarget(NULL);
				m_hDropTarget = m_hDragItem;
			}
			else if ((hNewDropTarget != NULL) && (hNewDropTarget != m_hDropTarget))
			{
				GetTreeCtrl().SelectDropTarget(hNewDropTarget);
				m_hDropTarget = hNewDropTarget;
			}
		}
	}
	
	CTreeView::OnMouseMove(nFlags, point);
}


void CAnimTblEdView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	unsigned int	flags;

	//---	set the selected item for movement
	m_hDragItem = GetTreeCtrl().HitTest(point, &flags);
	if (!(flags & TVHT_ONITEM))
		m_hDragItem = NULL;
	
	CTreeView::OnLButtonDown(nFlags, point);
}

void CAnimTblEdView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	//---	If we have tree items selected from dragging and dropping then handle them here.
	if (m_hDragItem && m_hDropTarget && (m_hDragItem != m_hDropTarget))
	{
		int		InsertFlags;
		CRect	TargetRect;
		CPoint	TopLeft, BottomRight;
		int		Height;

		//---	We are going to see where the item was dropped.
		GetTreeCtrl().GetItemRect( m_hDropTarget, TargetRect, FALSE );

		TopLeft = TargetRect.TopLeft();
		BottomRight = TargetRect.BottomRight();
		Height = TargetRect.Height();

		//---	If it was dropped in the top half then mark it as insert before.
		InsertFlags = 0;
		if ((point.y - TopLeft.y) < Height/2)
			InsertFlags |= DROPTARGET_INSERTBEFORE;

		//---	If it was dropped in the middle third then mark it as insert inside.
		if (((point.y - TopLeft.y) >= (Height/3))									// above the bottom third.
			&& (((point.y - TopLeft.y) < (2*Height/3))								// and below the top third
				||(GetTreeCtrl().GetItemState( m_hDropTarget, TVIS_EXPANDED ) & TVIS_EXPANDED)))	// or item expanded.
			InsertFlags |= DROPTARGET_INSERTINSIDE;

		//---	If the control button was held copy the tree item.
		if (nFlags & MK_CONTROL)
			InsertFlags |= DROPTARGET_INSERTCOPY;

		//---	Move the node to the new position.
		this->GetDocument()->MoveAnimTreeNode( (CAnimTreeNode*)GetTreeCtrl().GetItemData( m_hDragItem ), (CAnimTreeNode*)GetTreeCtrl().GetItemData( m_hDropTarget ), InsertFlags );
	}

	GetTreeCtrl().SelectDropTarget(NULL);
	m_hDragItem = NULL;
	m_hDropTarget = NULL;
	
	CTreeView::OnLButtonUp(nFlags, point);
}

void CAnimTblEdView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CAnimTblEdView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CTreeView::OnChar(nChar, nRepCnt, nFlags);
}

void CAnimTblEdView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	UINT			ScanCode;
	CAnimTreeNode	*pAnimTreeNode;

	pAnimTreeNode = this->GetSelectedNode();
	if (pAnimTreeNode)
	{
		ScanCode = nFlags & 0xff;
		switch (ScanCode)
		{
		case 0x1c:// "Enter"
			break;
		case 0x39:// "Space"
				this->OnGroupEdit();
			break;
		case 0x52:// "Insert"
				if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ANIM)
					this->OnAnimAddequiv();
				else if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_GROUP)
					this->OnGroupAddanimation();
				else
					this->OnGroupAddgroup();

			break;
		case 0x53:// "Delete"
				this->OnGroupDelete();
			break;
		}
	}

	CTreeView::OnKeyUp(nChar, nRepCnt, nFlags);
}

CAnimTreeNode * CAnimTblEdView::GetSelectedNode()
{
	HTREEITEM		TreeNode;
	CAnimTreeNode	*pAnimNode;

	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return NULL;

	pAnimNode = (CAnimNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pAnimNode)
		return NULL;

	return pAnimNode;
}

void CAnimTblEdView::OnViewProperties() 
{
	CViewPropertiesDlg Dialog;
	int i;
	
	for( i=0; i<NUM_VP_OPTIONS; i++ )
		Dialog.m_VPDisplayOrder[i] = m_VPDisplayOrder[i];

	if (Dialog.DoModal() == IDOK)
	{
		for( i=0; i<NUM_VP_OPTIONS; i++ )
			m_VPDisplayOrder[i] = Dialog.m_VPDisplayOrder[i];

		//---	apply the new view properties
		ApplyTreeViewProperties();
	}

}

void CAnimTblEdView::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	// storing code
		ar << 1L;

		//---	save the view properties display order
		ar << (int)NUM_VP_OPTIONS;
		for(int i=0; i<NUM_VP_OPTIONS; i++)
			ar << m_VPDisplayOrder[i];
	}
	else
	{	// loading code
		s32 Version;

		ar >> Version;

		//---	load view properties, display order
		int count, i, option;
		ar >> count;

		for(i=0; i<count; i++)
		{
			ar >> option;
			if (i < NUM_VP_OPTIONS)
				m_VPDisplayOrder[i] = option;
		}

		for(; i<NUM_VP_OPTIONS; i++)
			m_VPDisplayOrder[i] = VP_OPTION_NONE;
	}
}


void CAnimTblEdView::ApplyTreeViewProperties( HTREEITEM hItem )
{
#if 0
	//---	return on trivial case
	if (hItem == NULL)
		return;
	
	//---	if this is the first call, start the process from the begining
	if (hItem == (HTREEITEM)0xffffffff)
	{
		ApplyTreeViewProperties( GetTreeCtrl().GetFirstVisibleItem() );
		return;
	}

	//---	set the items string
	{
		CAnimTreeNode *pItem = (CAnimTreeNode*) GetTreeCtrl().GetItemData( hItem );
		ASSERT(pItem);
		GetTreeCtrl().SetItemText( hItem, pItem->GetVPString( m_VPDisplayOrder ) );
	}
	
	//---	update the sibling
	ApplyTreeViewProperties( GetTreeCtrl().GetNextVisibleItem( hItem ) );
#else
	//---	return on trivial case
	if (hItem == NULL)
		return;
	
	//---	if this is the first call, start the process from the begining
	if (hItem == (HTREEITEM)0xffffffff)
	{
		ApplyTreeViewProperties( GetTreeCtrl().GetRootItem() );
		return;
	}

	//---	set the items string
	{
		CAnimTreeNode *pItem = (CAnimTreeNode*) GetTreeCtrl().GetItemData( hItem );
		ASSERT(pItem);
		GetTreeCtrl().SetItemText( hItem, pItem->GetVPString( m_VPDisplayOrder ) );
	}
	
	//---	update the items children
	if (GetTreeCtrl().ItemHasChildren( hItem ))
	{
		HTREEITEM hChildItem = GetTreeCtrl().GetChildItem( hItem );
	
//		do
//		{
			ApplyTreeViewProperties( hChildItem );
//			hChildItem = GetTreeCtrl().GetNextItem( hChildItem, TVGN_NEXT );
//		} while (hChildItem != NULL);
	}

	//---	update the sibling
	ApplyTreeViewProperties( GetTreeCtrl().GetNextItem( hItem, TVGN_NEXT ) );
#endif
}

void CAnimTblEdView::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
/*	

	CAnimTreeNode *pItem = (CAnimTreeNode*) GetTreeCtrl().GetItemData( pNMTreeView->itemNew.hItem );
	ASSERT(pItem);
	GetTreeCtrl().SetItemText( pNMTreeView->itemNew.hItem, pItem->GetVPString( m_VPDisplayOrder ) );

	//---	update the items children
	if (GetTreeCtrl().ItemHasChildren( hItem ))
	{
		HTREEITEM hChildItem = GetTreeCtrl().GetChildItem( hItem );
	
		do
		{
			ApplyTreeViewProperties( hChildItem );
			hChildItem = GetTreeCtrl().GetNextItem( hChildItem, TVGN_NEXT );
		} while (hChildItem != NULL);
	}
*/	
	*pResult = 0;
}

void CAnimTblEdView::OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CAnimTblEdView::OnGroupAddtrans() 
{
	OnAnimAddtrans();
}


void CAnimTblEdView::OnAnimAddtrans() 
{
	HTREEITEM		TreeNode;
	CAnimTreeNode	*pNode;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pNode = (CAnimNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pNode)
		return;

	//---	Allow the group to be edited.
	CAnimTrans		*pAnimTrans = new CAnimTrans( this->GetDocument(), pNode );
	if (!pAnimTrans->Edit( pNode->GetName() ))
	{
		//---	If they cancelled the edit then do not add the group.
		delete pAnimTrans;
		return;
	}

	//---	Add the new Equivalency to the animation.
	pNode->AddChild( pAnimTrans, pNode->GetFirstChild() );

	//---	recount the number of dependancies
	if( pNode->m_Type == ANIMTREENODE_TYPE_ANIM )
		((CAnimNode*)pNode)->GetNumDependancies( TRUE );
	else
		((CGroupNode*)pNode)->GetNumDependancies( TRUE );

	//---	Add the new group and setup the CUpdateView structure.
	GetDocument()->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_ADDITEM, pAnimTrans, pNode, pNode->GetFirstChild() );

	//---	Tell all of the views to update.
	GetDocument()->UpdateAllViews( NULL );
}

void CAnimTblEdView::OnAnimScansourcecode() 
{
	HTREEITEM		TreeNode;
	CAnimTreeNode	*pNode;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pNode = (CAnimNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pNode)
		return;

	GetDocument()->ScanSourceCode(pNode);
}

void CAnimTblEdView::OnGroupScansourcecode() 
{
	HTREEITEM		TreeNode;
	CAnimTreeNode	*pNode;
	
	TreeNode = GetTreeCtrl().GetSelectedItem();
	if (!TreeNode)
		return;

	pNode = (CAnimNode *)GetTreeCtrl().GetItemData( TreeNode );
	if (!pNode)
		return;

	GetDocument()->ScanSourceCode(pNode);	
}

void CAnimTblEdView::OnTreeitemIgnore() 
{
	HTREEITEM		hTreeNode;
	CAnimTreeNode	*pNode;
	
	hTreeNode = GetTreeCtrl().GetSelectedItem();
	if (!hTreeNode)
		return;

	pNode = (CAnimNode *)GetTreeCtrl().GetItemData( hTreeNode );
	if (!pNode)
		return;

	pNode->SetIgnore( pNode->GetIgnore() ? FALSE : TRUE );
}

void CAnimTblEdView::OnUpdateTreeitemIgnore(CCmdUI* pCmdUI) 
{
	HTREEITEM		hTreeNode;
	CAnimTreeNode	*pNode;
	
	hTreeNode = GetTreeCtrl().GetSelectedItem();
	if (!hTreeNode)
		return;

	pNode = (CAnimNode *)GetTreeCtrl().GetItemData( hTreeNode );
	if (!pNode)
		return;

	// TODO: Add your command update UI handler code here
	if (pNode->GetIgnore())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CAnimTblEdView::OnDependancyDisable() 
{
	HTREEITEM		hTreeNode;
	CAnimTreeNode	*pNode;
	
	hTreeNode = GetTreeCtrl().GetSelectedItem();
	if (!hTreeNode)
		return;

	pNode = (CAnimNode *)GetTreeCtrl().GetItemData( hTreeNode );
	if (!pNode)
		return;

	((CAnimTrans*)pNode)->SetDisabled( ((CAnimTrans*)pNode)->GetDisabled() ? FALSE : TRUE );
}

void CAnimTblEdView::OnUpdateDependancyDisable(CCmdUI* pCmdUI) 
{
	HTREEITEM		hTreeNode;
	CAnimTreeNode	*pNode;
	
	hTreeNode = GetTreeCtrl().GetSelectedItem();
	if (!hTreeNode)
		return;

	pNode = (CAnimNode *)GetTreeCtrl().GetItemData( hTreeNode );
	if (!pNode)
		return;

	// TODO: Add your command update UI handler code here
	if (((CAnimTrans*)pNode)->GetDisabled())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CAnimTblEdView::OnTreeitemEdit() 
{
	// TODO: Add your command handler code here
	
}

void CAnimTblEdView::OnUpdateTreeitemEdit(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CAnimTblEdView::OnFilePrint() 
{
	// TODO: Add your command handler code here
	CPrintDialog PrintDlg( FALSE );

	if (PrintDlg.DoModal() == IDOK)
	{
	}
}

void CAnimTblEdView::OnFilePrintSetup() 
{
	// TODO: Add your command handler code here
	
}

void CAnimTblEdView::OnAnimSwapequiv() 
{
	HTREEITEM		hTreeNode;
	CAnimTreeNode	*pNode;
	
	hTreeNode = GetTreeCtrl().GetSelectedItem();
	if (!hTreeNode)
		return;

	pNode = (CAnimTreeNode *)GetTreeCtrl().GetItemData( hTreeNode );
	if (!pNode)
		return;

	pNode->SwapEquivalency();
}


void CAnimTblEdView::OnGroupSwapequiv() 
{
	HTREEITEM		hTreeNode;
	CAnimTreeNode	*pNode;
	
	hTreeNode = GetTreeCtrl().GetSelectedItem();
	if (!hTreeNode)
		return;

	pNode = (CAnimTreeNode *)GetTreeCtrl().GetItemData( hTreeNode );
	if (!pNode)
		return;

	pNode->SwapEquivalency();
}

// AnimTreeNode.cpp: implementation of the CAnimTreeNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "AnimTreeNode.h"
#include "IDList.h"
#include "AnimID1.h"

#include "EquivNode.h"
#include "AnimNode.h"
#include "GroupNode.h"

#include "ScanSourceCodeDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int CurrentExportTransIndex;
extern int CurrentExportCount;
extern int CurrentExportType;
extern int LastTokenType;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAnimTreeNode::CAnimTreeNode()
{
	m_Type = ANIMTREENODE_TYPE_UNDEF;
	m_pDoc = NULL;
	m_pParent = NULL;
	m_ChildPosStackIndex = 0;
    m_pSSCDlg = NULL;
	m_Ignore = FALSE;
	m_bSearchMatched = FALSE;
}

CAnimTreeNode::CAnimTreeNode(class CAnimTblEdDoc *pDoc, CAnimTreeNode *pParent)
{
	m_Type = ANIMTREENODE_TYPE_UNDEF;
	m_pParent = pParent;
	m_pDoc = pDoc;
	m_ChildPosStackIndex = 0;
    m_pSSCDlg = NULL;
	m_Ignore = FALSE;
	m_bSearchMatched = FALSE;
}

CAnimTreeNode::~CAnimTreeNode()
{
    DetachScanSourceDlg();
}


//////////////////////////////////////////////////////////////////////
// Set/Get Member Variables
//////////////////////////////////////////////////////////////////////

void CAnimTreeNode::SetType(int newType)
{
	ASSERT( (newType == ANIMTREENODE_TYPE_UNDEF) || 
			(newType == ANIMTREENODE_TYPE_PROJECT) || 
			(newType == ANIMTREENODE_TYPE_ENUM) || 
			(newType == ANIMTREENODE_TYPE_GROUP) || 
			(newType == ANIMTREENODE_TYPE_ANIM) );

	//---	Assign the new type for the node.
	m_Type = newType;
}

void CAnimTreeNode::Remove(CIDList &IDList)
{

}



const CString& CAnimTreeNode::GetName()
{
	static CString AnimName = "UNNAMED ANIMATION";
	static CString TransName = "UNNAMED TRANSITION";
	static CString GroupName = "UNNAMED GROUP";

	if (m_Type == ANIMTREENODE_TYPE_ANIM)
		return AnimName;
	else if (m_Type == ANIMTREENODE_TYPE_TRANS)
		return TransName;
	else
		return GroupName;
}

int CAnimTreeNode::GetMouseMenuIndex()
{
	//---	If the node is not a group or an animation then there is no menu for it.
	return -1;
}

bool CAnimTreeNode::Edit()
{
	return FALSE;
}

int CAnimTreeNode::GetImage()
{
	//---	The default group does not have an image.
	return -1;
}

int CAnimTreeNode::GetStateImage()
{
	int ret;

	//---	The default group does not have an image.
	if (GetState() & STATE_ERROR_BIT)
		ret = 1;
	else
		ret = 0;

	//---	if we are to ignore this item, add 2
	if( GetIgnore() )
		ret += 2;

	//---	if this item fits the last search item
	if( GetSearchMatched() )
		ret += 4;

	return ret;
}

int CAnimTreeNode::GetSelectedImage()
{
	//---	The default group does not have an image.
	return -1;
}

bool CAnimTreeNode::HasChildren()
{
	ASSERT(0);  // This should be a purely virtual function.
	return FALSE;
}


void CAnimTreeNode::Serialize(CArchive &ar)
{
	int version;

	if (ar.IsStoring())
	{
		ar << 1;
		ar << m_Ignore;
	}
	else
	{
		//---	get the document
		m_pDoc = (CAnimTblEdDoc*)ar.m_pDocument;

		//---	the very first versions of this file didn't store anything for the anim node, so a check
		//		has to be done in order to load those based on the file version.
		if( !m_pDoc->m_bOldFileVersion )
		{
			ar >> version;
			if( version != 1 )
			{
				MessageBox( NULL, "Error loading file", "Error loading file", MB_ICONERROR );
				ar.Abort();
				return;
			}

			ar >> m_Ignore;
		}
	}
}

CAnimTreeNode* CAnimTreeNode::ReadTreeItem(CArchive &ar, CAnimTreeNode *pParent)
{
	int				node_type;
	CAnimTreeNode	*pAnimTreeNode;

	ar >> node_type;
	switch (node_type)
	{
	case ANIMTREENODE_TYPE_PROJECT:
	case ANIMTREENODE_TYPE_ENUM:
	case ANIMTREENODE_TYPE_UNUSED:
	case ANIMTREENODE_TYPE_GROUP:
		pAnimTreeNode = new CGroupNode( (CAnimTblEdDoc*)ar.m_pDocument, pParent, node_type );
		break;

	case ANIMTREENODE_TYPE_ANIM:
		pAnimTreeNode = new CAnimNode( (CAnimTblEdDoc*)ar.m_pDocument, pParent );
		break;

	case ANIMTREENODE_TYPE_EQUIV:
		pAnimTreeNode = new CEquivNode( (CAnimTblEdDoc*)ar.m_pDocument, pParent );
		break;

	case ANIMTREENODE_TYPE_TRANS:
		pAnimTreeNode = new CAnimTrans( (CAnimTblEdDoc*)ar.m_pDocument, pParent );
		break;

	case ANIMTREENODE_TYPE_UNDEF:
	default:
		ASSERT( 0 );
		break;
	}

	pAnimTreeNode->Serialize( ar );

	return pAnimTreeNode;
}


void CAnimTreeNode::SetFirstChild()
{
	//---	The virtual function does nothing.
}

void CAnimTreeNode::WriteTreeItem(CArchive &ar)
{
	ar << m_Type;
	Serialize( ar );
}

int CAnimTreeNode::GetAddGroupOptionEnabled()
{
	return 0;
}

int CAnimTreeNode::GetAddAnimOptionEnabled()
{
	return 0;
}

int CAnimTreeNode::GetAddTransOptionEnabled()
{
	return 0;
}

int CAnimTreeNode::GetEditOptionEnabled()
{
	return 0;
}

int CAnimTreeNode::GetDeleteOptionEnabled()
{
	return 0;
}

int CAnimTreeNode::GetScanSourceCodeOptionEnabled()
{
    return MF_GRAYED;
}

void CAnimTreeNode::Export(FILE *pFile)
{

}

void CAnimTreeNode::DoExport(FILE *pFile, int ExportType)
{
	CurrentExportType = ExportType;

	if (this->IsExported())
		Export( pFile );
}

int CAnimTreeNode::GetLastTokenType()
{
	return LastTokenType;
};		

void CAnimTreeNode::SetLastTokenType(int token)
{
	LastTokenType = token;
};

int CAnimTreeNode::GetCurrentExportType()
{
	return CurrentExportType;
};

void CAnimTreeNode::SetCurrentExportType(int type)
{
	CurrentExportType = type;
};

void CAnimTreeNode::ClearCount()
{
	CurrentExportCount = 0; // ASSUME THE ANIM_NONE anim
};

int CAnimTreeNode::GetCount()
{
	if( GetIgnore() )
		return -1;

	return CurrentExportCount;
};

void CAnimTreeNode::IncCount()
{
	if( !GetIgnore() )
		CurrentExportCount++;
};

void CAnimTreeNode::DecCount()
{
	if( !GetIgnore() )
		CurrentExportCount--;
}

void CAnimTreeNode::CopyBaseNode(CAnimTreeNode &AnimTreeNode)
{
	//---	This function is called by the copy functions of the derived classes to copy the important base class fields.
	//---	The m_pParent field should already be set up and should not be overwritten here.
	m_pDoc = AnimTreeNode.m_pDoc;
	m_Type = AnimTreeNode.m_Type;
	m_Ignore = AnimTreeNode.m_Ignore;
}

int CAnimTreeNode::CanHaveChildren()
{
	return FALSE;
}

int CAnimTreeNode::IsAncestor(CAnimTreeNode *pAnimTreeNode)
{
	CAnimTreeNode *pAncestor = this;

	if (!pAnimTreeNode)
		return FALSE;

	while (pAncestor && (pAncestor != pAnimTreeNode))
		pAncestor = pAncestor->GetParent();

	return (pAncestor == pAnimTreeNode);
}

int CAnimTreeNode::GetNumAnims()
{
	return 0;
}

int CAnimTreeNode::GetNumExportedAnims()
{
	return 0;
}

CAnimTreeNode * CAnimTreeNode::Clone()
{
	return NULL;
}

int CAnimTreeNode::GetTotalNumDependancies( bool Recount, bool CountDisabledOnes )
{
	return 0;
}

int CAnimTreeNode::GetNumAnimsWithDependancies( bool Recount, bool CountDisabledOnes )
{
	return 0;
}

int CAnimTreeNode::GetTotalNumDependanciesWithLink( bool Recount, bool CountDisabledOnes )
{
	return 0;
}

int CAnimTreeNode::GetNumAnimsWithDependanciesWithLink( bool Recount, bool CountDisabledOnes )
{
	return 0;
}

int CAnimTreeNode::GetNumAnimsWithExtras()
{
	return 0;
}

void CAnimTreeNode::ClearExportTransIndex()
{
	CurrentExportTransIndex = 1;
}

int CAnimTreeNode::GetCurrentExportTransIndex()
{
	return CurrentExportTransIndex;
}

void CAnimTreeNode::IncExportTransIndex(int Inc)
{
	CurrentExportTransIndex += Inc;
}

int CAnimTreeNode::CountLocomotionAnimations(int Count)
{
	if (this == NULL)
		return Count;

	m_Count = -1;
	if (m_Type == ANIMTREENODE_TYPE_ANIM)
	{
		if (!GetIgnore() && IsExported() && ((CAnimNode*) this)->RateAnalogControlled())
			m_Count = Count++;
	}
	else if (HasChildren())
	{
		CAnimTreeNode *pNode;
		pNode = GetFirstChild();
		while (pNode)
		{
			Count = pNode->CountLocomotionAnimations( Count );
			pNode = GetNextChild();
		}
	}

	return Count;	
}

void CAnimTreeNode::AddToListBox(CListBox *pListBox, int nFlags)
{
	static CAnimTreeNode *pAnimTreeNode;

	pAnimTreeNode = this->GetFirstChild();
	while (pAnimTreeNode)
	{
		switch (pAnimTreeNode->m_Type)
		{
			case ANIMTREENODE_TYPE_ANIM:
					//---	If we are adding anims to the list then add this one.
					if (nFlags & LISTBOX_FLAG_INCLUDEANIMS)
						pListBox->AddString( pAnimTreeNode->GetName() );
				break;

			case ANIMTREENODE_TYPE_GROUP:
					//---	If we are adding groups to the list then add this one.
					if (nFlags & LISTBOX_FLAG_INCLUDEGROUPS)
					{
						CString NameOfGroup;
						NameOfGroup.Format( "[%s]", pAnimTreeNode->GetName() );
						pListBox->AddString( NameOfGroup );
					}
					//---	Now add the children to the list.
					pAnimTreeNode->AddToListBox( pListBox, nFlags );
				break;

			case ANIMTREENODE_TYPE_ENUM:
			case ANIMTREENODE_TYPE_PROJECT:
					//---	Now add the children to the list.
					pAnimTreeNode->AddToListBox( pListBox, nFlags );
				break;
		}

		pAnimTreeNode = this->GetNextChild();
	}

}

void CAnimTreeNode::ExportChildren(FILE *pFile)
{
	CAnimTreeNode *pAnimTreeNode;
	PushChildStep();
	pAnimTreeNode = this->GetFirstChild();

	while (pAnimTreeNode)
	{
		if (pAnimTreeNode->IsExported())
			pAnimTreeNode->Export( pFile );
		pAnimTreeNode = this->GetNextChild();
	}
	PopChildStep();
}

//////////////////////////////////////////////////////////////////////////////
//
//	FindAnimTreeNodeByName
//
//		Gets an aniamtion tree node given the name of the node.
//
//		NOTE: it uses recursion to taverse the given node's children
//
CAnimTreeNode* CAnimTreeNode::FindTreeNodeByName(CString &Name)
{
	if (this->GetName() == Name)
		return this;

	return NULL;
}

int CAnimTreeNode::GetState()
{
	return 0;
}

int CAnimTreeNode::IsExported()
{
	return TRUE;
}

void CAnimTreeNode::PushChildStep()
{
	ASSERT(m_ChildPosStackIndex < (CHILD_POS_STACK_SIZE-1));
	m_ChildPosStack[ m_ChildPosStackIndex++ ] = m_ChildPos;
}

void CAnimTreeNode::PopChildStep()
{
	ASSERT(m_ChildPosStackIndex > 0);
	m_ChildPos = m_ChildPosStack[ --m_ChildPosStackIndex ];
}


CAnimTreeNode * CAnimTreeNode::GetFirstChild()
{
	m_ChildPos = NULL;
	if (m_ChildList.IsEmpty())
		return NULL;
	else
	{
		m_ChildPos = m_ChildList.GetHeadPosition();
		return m_ChildList.GetNext( m_ChildPos );
	}
}

CAnimTreeNode * CAnimTreeNode::GetNextChild(CAnimTreeNode *pNodeBefore)
{
	CAnimTreeNode *pAnimTreeNode;

	if (pNodeBefore)
	{
		//---	If a node was specified then find it in the list and return the following child.
		m_ChildPos = m_ChildList.Find( pNodeBefore );

		pAnimTreeNode = m_ChildList.GetNext( m_ChildPos );
//		pAnimTreeNode->SetFirstChild();	// If this is a group then we must reset m_ChildPos.
		return m_ChildList.GetNext( m_ChildPos );
	}
	else
	{
		//---	Otherwise assume the m_ChildPos points to the node we want the following child from.
		if (m_ChildPos)
		{
			pAnimTreeNode = m_ChildList.GetNext( m_ChildPos );
//			pAnimTreeNode->SetFirstChild(); // If this is a group then we must reset m_ChildPos.
			return pAnimTreeNode;
		}
		else
			return NULL;
	}
}

CAnimTreeNode * CAnimTreeNode::GetPrevChild(CAnimTreeNode *pNodeAfter)
{
	//---	If a node was specified then find it in the list and return the following child.
	if (pNodeAfter)
		m_ChildPos = m_ChildList.Find( pNodeAfter );

	//---	Otherwise assume the m_ChildPos points to the node we want the following child from.
	if (m_ChildPos)
	{
		m_ChildList.GetPrev( m_ChildPos );

		if (m_ChildPos)
			return m_ChildList.GetAt( m_ChildPos );;
	}

	return NULL;
}


void CAnimTreeNode::AddChild(CAnimTreeNode *pAnimTreeNode, CAnimTreeNode *pRelativeNode, int InsertAfter)
{
	POSITION	ListPos;

	ASSERT(pAnimTreeNode);

	pAnimTreeNode->SetParent( this );
	m_nChildren++;

	if (pRelativeNode)
	{
		ListPos = m_ChildList.Find( pRelativeNode );	
		if (ListPos)
		{
			if (InsertAfter)
				m_ChildList.InsertAfter( ListPos, pAnimTreeNode );
			else
				m_ChildList.InsertBefore( ListPos, pAnimTreeNode );

			return;
		}
	}

	m_ChildList.AddTail( pAnimTreeNode );
}

void CAnimTreeNode::RemoveChild(CAnimTreeNode *pFindNode)
{
	POSITION		ChildListPos;

	ChildListPos = m_ChildList.GetHeadPosition();
	while (ChildListPos && (m_ChildList.GetAt( ChildListPos ) != pFindNode))
		m_ChildList.GetNext( ChildListPos );

	if (ChildListPos)
	{
		m_ChildList.RemoveAt( ChildListPos );
		delete pFindNode;
	}

	m_nChildren--;
}

CAnimTreeNode * CAnimTreeNode::SeverChild(CAnimTreeNode *pAnimTreeNode)
{
	//	This function removes a child from the list without deleting the storage.
	POSITION		ListPos;

	ListPos = m_ChildList.Find( pAnimTreeNode );

	if (ListPos)
	{
		m_ChildList.RemoveAt( ListPos );
		m_nChildren--;
		return pAnimTreeNode;
	}

	return NULL;
}

void CAnimTreeNode::AttachScanSourceDlg( CScanSourceCodeDlg* dlg )
{
    DetachScanSourceDlg();

    m_pSSCDlg = dlg;
}

void CAnimTreeNode::DetachScanSourceDlg()
{
    if (m_pSSCDlg)
    {
        delete m_pSSCDlg;
        m_pSSCDlg = NULL;
    }
}

void CAnimTreeNode::SetIgnore( bool Ignore )
{
	//---	if the parent is set to be ignored, then do not process any attempt to set the ignored flag for a tree item
	if( m_pParent && m_pParent->GetIgnore() )
		return;

	//---	because the parent is not set to be ignored, it is allowed to set this tree item's ignore flag,
	//		but lets only set it if it has changed value.
	if( !((m_Ignore?TRUE:FALSE) ^ (Ignore?TRUE:FALSE)) )
		return;

	//---	if we are removing and ignore, set it now so that the children will process,
	//		do not want to set an ignore==TRUE because the children wouldn't be processed
	if( !Ignore )
		m_Ignore = Ignore;

	//---	set the ignores of the children
	POSITION pos = this->m_ChildList.GetHeadPosition();
	while( pos )
		m_ChildList.GetNext( pos )->SetIgnore( Ignore );

	//---	We must set the ignore value now if it hasn't already been set
	if( Ignore )
		m_Ignore = Ignore;

	//---	if this is a group or anim, we will have to increment/decrement the name and ID uses
	if(( m_Type == ANIMTREENODE_TYPE_ANIM ) ||
	   ( m_Type == ANIMTREENODE_TYPE_GROUP ))
	{
		if( m_Ignore )
		{
			m_pDoc->m_AnimList.DecrementUses( GetName() );

			if( m_Type == ANIMTREENODE_TYPE_ANIM )
			{
				m_pDoc->m_IDList.DecrementUses( ((CAnimNode*)this)->m_RLIDs.GetLeftID() );
				m_pDoc->m_IDList.DecrementUses( ((CAnimNode*)this)->m_RLIDs.GetRightID() );
			}
		}
		else
		{
			m_pDoc->m_AnimList.IncrementUses( GetName() );

			if( m_Type == ANIMTREENODE_TYPE_ANIM )
			{
				m_pDoc->m_IDList.IncrementUses( ((CAnimNode*)this)->m_RLIDs.GetLeftID() );
				m_pDoc->m_IDList.IncrementUses( ((CAnimNode*)this)->m_RLIDs.GetRightID() );
			}
		}
	}

	//---	modify the view for this item
	ModifyViews();
}

bool CAnimTreeNode::GetIgnore()
{
	if( m_Ignore )
		return TRUE;

	if( m_pParent && m_pParent->GetIgnore() )
		return TRUE;

	return FALSE;
}

void CAnimTreeNode::SetSearchMatched( bool SearchMatched )
{
	if( m_pParent && SearchMatched )
		m_pParent->SetSearchMatched( TRUE );

	if (m_bSearchMatched == SearchMatched)
		return;

	m_bSearchMatched = SearchMatched;

	// Update the view.
	m_pDoc->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_UPDATEITEM, this );
	m_pDoc->UpdateAllViews( NULL );
}

bool CAnimTreeNode::GetSearchMatched( void )
{
	return m_bSearchMatched;
}

void CAnimTreeNode::ModifyViews()
{
	if (m_pDoc)
	{
		m_pDoc->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_UPDATEITEM, this );
		m_pDoc->UpdateAllViews( NULL );

		POSITION pos = m_ChildList.GetHeadPosition();
		while( pos )
		{
			m_pDoc->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_UPDATEITEM, m_ChildList.GetNext( pos ) );
			m_pDoc->UpdateAllViews( NULL );
		}
	}
}

void CAnimTreeNode::SwapEquivalency( void )
{
	POSITION pos = this->m_ChildList.GetHeadPosition();

	while( pos )
		m_ChildList.GetNext( pos )->SwapEquivalency();
}

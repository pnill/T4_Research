// NodeSearchMap.cpp: implementation of the CNodeSearchMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "NodeSearchMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeSearchMap::CNodeSearchMap()
{
	m_bANDSearch			= TRUE;

	m_bSearchName			= FALSE;
	m_bSearchLeftID			= FALSE;
	m_bSearchRightID		= FALSE;
	m_bSearchFlags			= FALSE;
	m_bSearchMethod			= FALSE;
	m_bSearchPriority		= FALSE;
	m_bSearchCallback		= FALSE;
	m_bSearchSelectCallback	= FALSE;
	m_bSearchCutTable		= FALSE;
}

CNodeSearchMap::~CNodeSearchMap()
{

}

BOOL CNodeSearchMap::IsSearchable()
{
	return	m_bSearchName || 
			m_bSearchLeftID || 
			m_bSearchRightID || 
			m_bSearchFlags || 
			m_bSearchMethod || 
			m_bSearchPriority || 
			m_bSearchCallback || 
			m_bSearchSelectCallback || 
			m_bSearchCutTable;
}

BOOL CNodeSearchMap::IsGroupSearchable()
{
	if (!m_bSearchGroups)
		return FALSE;

	return IsORSearch() || (!m_bSearchLeftID && !m_bSearchRightID);
}

bool CNodeSearchMap::IsAnimSearchable()
{
	return IsORSearch() || !m_bSearchSelectCallback;
}

int CNodeSearchMap::DoSearch(CAnimTblEdDoc *pDoc)
{
	return DoSearch( &pDoc->m_AnimTree );
}

int CNodeSearchMap::DoSearch(CAnimTreeNode *pAnimTreeNode)
{
	int nSearchMatches = 0;

	//---	Do the compare on this node and if matched mark it as matched as well as it's parents.
	nSearchMatches += IsMatch( pAnimTreeNode );
	
	//---	We want to stop the search at Animations because we do not search dependancies.
	if ((pAnimTreeNode->m_Type != ANIMTREENODE_TYPE_ANIM) && pAnimTreeNode->HasChildren())
	{
		pAnimTreeNode->PushChildStep();

		CAnimTreeNode *pChildNode = pAnimTreeNode->GetFirstChild();
		while (pChildNode)
		{
			nSearchMatches += DoSearch( pChildNode );

			pChildNode = pAnimTreeNode->GetNextChild();
		}

		pAnimTreeNode->PopChildStep();
	}

	return nSearchMatches;
}

int CNodeSearchMap::IsMatch(CAnimTreeNode *pAnimTreeNode)
{
	int ret = 0;

	if (!((pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_PROJECT) || (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ENUM)))
	{
		if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_GROUP)
			ret = IsMatchGroup( (CGroupNode*)pAnimTreeNode );
		else if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ANIM)
			ret = IsMatchAnim( (CAnimNode*)pAnimTreeNode );
	}

	pAnimTreeNode->SetSearchMatched( ret ? TRUE : FALSE );
	return ret;
}

// Macro Used in Match functions
#define MATCHCHECK(expression)		\
{									\
if (IsANDSearch() && !(expression))	\
	return 0;						\
if (IsORSearch() && (expression))	\
	return 1;						\
}

int CNodeSearchMap::IsMatchGroup(CGroupNode *pGroupNode)
{
	CString String;

	//---	If this is an AND search and we have non-group parameters then return false.
	if (!IsGroupSearchable())
		return 0;
	
	//---	Check all search parameters.
	// Search Name?
	if ( m_bSearchName )
		MATCHCHECK(m_sName.Compare( pGroupNode->GetName() ) == 0);

	// Search Flags?
	if ( m_bSearchFlags )
		MATCHCHECK((pGroupNode->BS_GetFlags() & m_nFlags) == m_nFlags);

	// Search Method?
	if ( m_bSearchMethod )
		MATCHCHECK(pGroupNode->BS_GetMethod() == m_nMethod);

	// Search Priority?
	if ( m_bSearchPriority )
		MATCHCHECK(pGroupNode->BS_GetPriority() == m_nPriority);

	// Search Callback?
	if (m_bSearchCallback)
		MATCHCHECK(m_sCallback.Compare( pGroupNode->GetCallback() ) == 0);

	// Search SelectCallback?
	if (m_bSearchSelectCallback)
		MATCHCHECK(m_sSelectCallback.Compare( pGroupNode->GetSelectCallback() ) == 0);

	// Search CutTable?
	if (m_bSearchCutTable)
		MATCHCHECK(m_sCutTable.Compare( pGroupNode->GetCutTable()->GetExportName() ) == 0);

	//---	Default Returns
	// If we were doing an AND search and everything matched then return a 1.
	if (IsANDSearch())
		return 1;

	// If we were doing an OR search and nothing matched then return a 0.
	else //IsORSearch()
		return 0;
}

int CNodeSearchMap::IsMatchAnim(CAnimNode *pAnimNode)
{
	CString String;

	//---	If this is an AND search and we have non-group parameters then return false.
	if (!IsAnimSearchable())
		return 0;

	//---	Check all search parameters.
	// Search Name?
	if ( m_bSearchName )
		MATCHCHECK(m_sName.Compare( pAnimNode->GetName() ) == 0);

	// Search LeftID?
	if ( m_bSearchLeftID )
		MATCHCHECK(m_sLeftID.Compare( pAnimNode->m_RLIDs.GetLeftID() ) == 0);

	// Search RightID?
	if ( m_bSearchRightID )
		MATCHCHECK(m_sRightID.Compare( pAnimNode->m_RLIDs.GetRightID() ) == 0);

	// Search Flags?
	if ( m_bSearchFlags )
		MATCHCHECK((pAnimNode->BS_GetFlags() & m_nFlags) == m_nFlags);

	// Search Method?
	if ( m_bSearchMethod )
		MATCHCHECK(pAnimNode->BS_GetMethod() == m_nMethod);

	// Search Priority?
	if ( m_bSearchPriority )
		MATCHCHECK(pAnimNode->BS_GetPriority() == m_nPriority);

	// Search Callback?
	if (m_bSearchCallback)
		MATCHCHECK(m_sCallback.Compare( pAnimNode->GetCallback() ) == 0);

	// Search CutTable?
	if (m_bSearchCutTable)
		MATCHCHECK(m_sCutTable.Compare( pAnimNode->GetCutTable()->GetExportName() ) == 0);

	
	//---	Default Returns
	// If we were doing an AND search and everything matched then return a 1.
	if (IsANDSearch())
		return 1;

	// If we were doing an OR search and nothing matched then return a 0.
	else //IsORSearch()
		return 0;
}

void CNodeSearchMap::SetANDSearch()
{
	m_bANDSearch = TRUE;
}

void CNodeSearchMap::SetORSearch()
{
	m_bANDSearch = FALSE;
}

void CNodeSearchMap::SetSearchGroups(bool SearchGroups)
{
	m_bSearchGroups = SearchGroups;
}

void CNodeSearchMap::SetSearchName(CString Name)
{
	//---	Flag the search TRUE
	m_bSearchName = TRUE;

	//---	Copy the name.
	m_sName.Format( Name );
}

void CNodeSearchMap::SetSearchLeftID(CString LeftID)
{
	//---	Flag the search TRUE
	m_bSearchLeftID = TRUE;

	//---	Copy the LeftID.
	m_sLeftID.Format( LeftID );
}

void CNodeSearchMap::SetSearchRightID(CString RightID)
{
	//---	Flag the search TRUE
	m_bSearchRightID = TRUE;

	//---	Copy the RightID.
	m_sRightID.Format( RightID );
}

void CNodeSearchMap::SetSearchFlags(int Flags)
{
	m_bSearchFlags = Flags ? TRUE : FALSE;
	m_nFlags = Flags;
}

void CNodeSearchMap::SetSearchMethod(int Method)
{
	m_bSearchMethod = TRUE;
	m_nMethod = Method;
}

void CNodeSearchMap::SetSearchPriority(int Priority)
{
	m_bSearchPriority = TRUE;
	m_nPriority = Priority;
}

void CNodeSearchMap::SetSearchCallback(CString Callback)
{
	//---	Flag the search TRUE
	m_bSearchCallback = TRUE;

	//---	Copy the Callback.
	m_sCallback.Format( Callback );
}

void CNodeSearchMap::SetSearchSelectCallback(CString SelectCallback)
{
	//---	Flag the search TRUE
	m_bSearchSelectCallback = TRUE;

	//---	Copy the SelectCallback.
	m_sCallback.Format( SelectCallback );
}

void CNodeSearchMap::SetSearchCutTable(CString CutTable)
{
	//---	Flag the search TRUE
	m_bSearchCutTable = TRUE;

	//---	Copy the CutTable.
	m_sCallback.Format( CutTable );
}



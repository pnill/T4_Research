// AnimTransList.cpp: implementation of the CAnimTransList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTransList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAnimTransList::CAnimTransList()
{
	m_nDependancies = 0;
}

CAnimTransList::~CAnimTransList()
{
	this->Delete();
}

void CAnimTransList::Add(CAnimTrans *pAnimTrans)
{
	CAnimTrans *pAnimTransInsertBefore = NULL;

	ASSERT(pAnimTrans);

	m_nDependancies++;

	//---	With AnimTrans structures we want to put the Prerequisites before the Transitions.
	if (pAnimTrans->IsPrerequisite())
	{
		pAnimTransInsertBefore = this->GetFirst();
		while (pAnimTransInsertBefore
			&& pAnimTransInsertBefore->IsPrerequisite()
			&& (strcmp( pAnimTransInsertBefore->GetFromAnim(), "<any>" ) != 0)
			&& (pAnimTransInsertBefore->GetTag() != "<def> "))	// if it is tagged, it is read only and the other should be added first
			pAnimTransInsertBefore = this->GetNext();
	}
	else
	{
		//---	If we have a catch-all transition we must insert before it.
		pAnimTransInsertBefore = this->GetFirst();
		while (pAnimTransInsertBefore
			&& (strcmp( pAnimTransInsertBefore->GetToAnim(), "<any>" ) != 0)
			&& ((pAnimTransInsertBefore->GetTag() != "<def> ")
				|| (pAnimTransInsertBefore->IsPrerequisite())))
			pAnimTransInsertBefore = this->GetNext();
	}

	//---	If the insert before pointer is set then we know our m_ListPos holds the position of the item we want to come before.
	if (pAnimTransInsertBefore)
		m_List.InsertBefore( m_ListPos, pAnimTrans );
	else  // otherwise put it on the end.
		m_List.AddTail( pAnimTrans );
}

void CAnimTransList::Delete()
{
	CAnimTrans	*pAnimTrans;

	while (!m_List.IsEmpty())
	{

		pAnimTrans = m_List.RemoveTail();
		delete pAnimTrans;
	}
}

CAnimTrans * CAnimTransList::GetFirst()
{
	m_ListPos = m_List.GetHeadPosition();

	if (m_ListPos)
		return m_List.GetAt( m_ListPos );
	else
		return NULL;
}

CAnimTrans * CAnimTransList::GetNext()
{
	if (m_ListPos)
	{
		m_List.GetNext( m_ListPos );

		if (m_ListPos)
			return m_List.GetAt( m_ListPos );
	}

	return NULL;
}

CAnimTrans * CAnimTransList::GetNth(int Nth)
{
	CAnimTrans	*pAnimTrans;

	pAnimTrans = this->GetFirst();

	while (pAnimTrans && Nth)
	{
		pAnimTrans = this->GetNext();
		Nth--;
	}

	return pAnimTrans;
}

CAnimTrans * CAnimTransList::RemoveTail()
{
	if (!m_List.IsEmpty())
	{
		m_nDependancies--;

		return m_List.RemoveTail();
	}

	return NULL;
}

void CAnimTransList::SetDefaultPrerequisite(CAnimTrans *pAnimTrans)
{
	ASSERT(pAnimTrans->IsPrerequisite());

	//---	Remove the prerequisite from where it is in the list and put it first.
	this->Find( pAnimTrans );

	m_List.RemoveAt( m_ListPos );

	m_List.AddHead( pAnimTrans );
}

CAnimTrans * CAnimTransList::Find(CAnimTrans *pAnimTrans)
{
	m_ListPos = m_List.Find( pAnimTrans );

	return m_List.GetAt( m_ListPos );
}

CAnimTrans * CAnimTransList::Remove(CAnimTrans *pAnimTrans)
{
	pAnimTrans = this->Find( pAnimTrans );

	if (pAnimTrans)
		m_nDependancies--;

	m_List.RemoveAt( m_ListPos );

	return pAnimTrans;
}

void CAnimTransList::Copy(CAnimTransList &AnimTransList)
{
	CAnimTrans	*pAnimTrans;
	CAnimTrans	*pNewAnimTrans;

	pAnimTrans = AnimTransList.GetFirst();
	while (pAnimTrans)
	{
		pNewAnimTrans = new CAnimTrans( ((CAnimTreeNode*)pAnimTrans)->m_pDoc, pAnimTrans->m_pParent );
		pNewAnimTrans->Copy( *pAnimTrans );
		this->Add( pNewAnimTrans );

		pAnimTrans = AnimTransList.GetNext();
	}
}

int CAnimTransList::GetNumDependancies(bool CountDisabled)
{
	POSITION	m_ListPos;
	CAnimTrans	*pAnimTrans;
	int			Count = 0;

	if (CountDisabled)
		return m_nDependancies;
	
	m_ListPos = m_List.GetHeadPosition();
	while (m_ListPos)
	{
		pAnimTrans = m_List.GetNext( m_ListPos );

		if (!pAnimTrans->GetDisabled())
			Count++;
	}

	return Count;
}

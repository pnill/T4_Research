// AnimTree.cpp: implementation of the CAnimTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTreeNode.h"
#include "AnimTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAnimTree::CAnimTree()
{
	m_nRootNodes = 0;
}

CAnimTree::~CAnimTree()
{

}

int CAnimTree::IsEmpty()
{
	return m_AnimTreeList.IsEmpty();
}

void CAnimTree::Empty()
{
	CAnimTreeNode	*pAnimTreeNode;

	while (!m_AnimTreeList.IsEmpty())
	{
		pAnimTreeNode = m_AnimTreeList.RemoveHead();
		pAnimTreeNode->Remove();
		delete pAnimTreeNode;
	}
}

bool CAnimTree::AddTail(CAnimTreeNode *pAnimTreeNode)
{
	m_AnimTreeList.AddTail( pAnimTreeNode );

	m_nRootNodes++ ;

	return TRUE;
}

POSITION CAnimTree::GetHeadPosition()
{
	return m_AnimTreeList.GetHeadPosition();
}

CAnimTreeNode * CAnimTree::GetNext(POSITION &AnimTreePos)
{
	return m_AnimTreeList.GetNext( AnimTreePos );
}

void CAnimTree::Serialize(CArchive &ar)
{
	POSITION		TreePos;
	CAnimTreeNode	*pAnimTreeNode;
	int				node_type;
	char			buffer[SIZE_ID+1];
	char			buffer2[SIZE_ID+1];

	if (ar.IsStoring())
	{
		ar << m_nRootNodes;
		TreePos = m_AnimTreeList.GetHeadPosition();
		while (TreePos)
		{
			pAnimTreeNode = m_AnimTreeList.GetNext( TreePos );
			pAnimTreeNode->Serialize( ar );
		}
	}
	else
	{
		ASSERT( m_nRootNodes == 0 );

		ar >> m_nRootNodes;
		
		for (int i = 0; i < m_nRootNodes; i++)
		{

		}
	}
}

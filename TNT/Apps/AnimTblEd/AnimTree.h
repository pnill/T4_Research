// AnimTree.h: interface for the CAnimTree class.
//
//////////////////////////////////////////////////////////////////////

#include <afxtempl.h>

#if !defined(AFX_ANIMTREE_H__3ADFC660_A3CE_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMTREE_H__3ADFC660_A3CE_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAnimTree : public CObject  
{
public:
	int m_nRootNodes;
	void Serialize(CArchive& ar);
	CAnimTreeNode * GetNext(POSITION &AnimTreePos);
	POSITION GetHeadPosition();
	bool AddTail(CAnimTreeNode *pAnimTreeNode);
	void Empty();
	int IsEmpty();
	CTypedPtrList<CObList, CAnimTreeNode*> m_AnimTreeList;
	CAnimTree();
	virtual ~CAnimTree();

};

#endif // !defined(AFX_ANIMTREE_H__3ADFC660_A3CE_11D2_9248_00105A29F8F3__INCLUDED_)

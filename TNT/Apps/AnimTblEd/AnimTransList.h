// AnimTransList.h: interface for the CAnimTransList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMTRANSLIST_H__67F8F501_DBB8_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMTRANSLIST_H__67F8F501_DBB8_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "AnimTrans.h"

class CAnimTransList  
{
private:
	CTypedPtrList<CObList, class CAnimTrans*>	m_List;
	POSITION									m_ListPos;
	int											m_nDependancies;

public:
	void Copy(CAnimTransList &AnimTransList);
	CAnimTrans * Remove(CAnimTrans *pAnimTrans);
	CAnimTrans * Find(CAnimTrans *pAnimTrans);
	void SetDefaultPrerequisite(CAnimTrans *pAnimTrans);
	CAnimTrans * RemoveTail();
	bool IsEmpty()
	{return !m_nDependancies;};
	int GetNumDependancies(bool CountDisabled);
	CAnimTrans * GetNth(int Nth);
	CAnimTrans * GetNext();
	CAnimTrans * GetFirst();
	void Delete();
	void Add(CAnimTrans *pAnimTrans);
	CAnimTransList();
	virtual ~CAnimTransList();

};

#endif // !defined(AFX_ANIMTRANSLIST_H__67F8F501_DBB8_11D2_9248_00105A29F8F3__INCLUDED_)

// IDList.h: interface for the CIDList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDLIST_H__F89033E1_8F63_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_IDLIST_H__F89033E1_8F63_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CIDList : public CObject 
{
public:
	void Clear();
	int GetFilteringEnabled();
	int GetFilterCount();
	int EnableFiltering(int Set);
	int m_EnableFilter;
	void RemoveFilter(const char *lpszPrefix);
	class CAnimID * GetNextFilter();
	class CAnimID * GetFirstFilter();
	void AddFilter(const char *lpszIDName);
	class CAnimID * RemoveDec(const char *lpszIDName);
	class CAnimID * AddInc(const char * lpszIDName);
	int m_nAnims;
	class CAnimID * Remove(const char *lpszIDName);
	POSITION FindIDPos(const char *lpszIDName);
	class CAnimID * Add(const char *lpszIDName);
	void Serialize(CArchive& ar);
	CAnimID * FindID(const char *lpszIDName);
	int DecrementUses(const char *lpszIDName);
	int IncrementUses(const char *lpszIDName);
	class CAnimID * GetNext();
	class CAnimID * GetHead();
	class CAnimID * AddTail(const char * IDName);
	class CAnimID * RemoveTail();
	int IsEmpty();
	void Delete();
	POSITION m_FilterListPos;
	CTypedPtrList<CObList, class CAnimID*>	m_FilterList;
	POSITION m_ListPos;
	CTypedPtrList<CObList, class CAnimID*>	m_IDList;
	CIDList();
	virtual ~CIDList();

protected:
	POSITION FindFilter(const char *lpszPrefix);
};

#endif // !defined(AFX_IDLIST_H__F89033E1_8F63_11D2_9248_00105A29F8F3__INCLUDED_)

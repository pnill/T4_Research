// EquivNode.h: interface for the CEquivNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EQUIVNODE_H__4CE615A0_B3B7_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_EQUIVNODE_H__4CE615A0_B3B7_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AnimTreeNode.h"

class CEquivNode : public CAnimTreeNode  
{
public:
	void Swap();
	CAnimTreeNode * Clone();
	int CanHaveChildren();
	void Copy(CAnimTreeNode &AnimTreeNode);
	CEquivNode();
	void Export(FILE *pFile);
	void Serialize(CArchive &ar);
	virtual bool Edit();
	void SetRightID(const char *lpszRightID, CIDList *pIDList);
	void SetLeftID(const char *lpszLeftID, CIDList *pIDList);
	void Remove(CIDList &IDList);
	int GetDeleteOptionEnabled();
	int GetEditOptionEnabled();
	int GetAddTransOptionEnabled();
	int GetAddAnimOptionEnabled();
	int GetAddGroupOptionEnabled();
	bool HasChildren();
	int GetMouseMenuIndex();
//	const char * GetName();
	const CString& GetName(){return m_Name;}
	int GetSelectedImage();
	int GetImage();
	bool GetRightReversed();
	bool GetRightMirrored();
	void SetRightReversed(bool Set);
	void SetRightMirrored(bool Set);
	bool GetLeftReversed();
	bool GetLeftMirrored();
	void SetLeftReversed(bool Set);
	void SetLeftMirrored(bool Set);
	const CString& GetRightID();
	const CString& GetLeftID();

	CEquivNode(class CAnimTblEdDoc *pDoc, CAnimTreeNode *pParent);
	virtual ~CEquivNode();

private:
	void SetDefaultName();
	int m_Flags;
	CString m_RightID;
	CString m_LeftID;
	CString m_Name;
};

#endif // !defined(AFX_EQUIVNODE_H__4CE615A0_B3B7_11D2_9248_00105A29F8F3__INCLUDED_)

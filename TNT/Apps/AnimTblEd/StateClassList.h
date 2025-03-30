// StateClassList.h: interface for the CStateClassList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATECLASSLIST_H__86A11B20_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_)
#define AFX_STATECLASSLIST_H__86A11B20_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_

#include <afxtempl.h>
#include "StateClass.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStateClassList : public CObject  
{
private:
	CTypedPtrList<CObList, CStateClass*>	m_StateClassList;

public:
	int FindStateTransAnim(int Bitset, class CAnimTreeNode*pNode);
	void ExportStateTransAnims(FILE *pFile, int BitSet, class CAnimTreeNode *pNode);
	void ExportStateTransList(FILE *pFile, class CAnimTblEdDoc *pDoc, int BitSet, int State, CString *StateTransLists, int &ListIndex, CString Comment);
	void ExportStateTransTable( FILE *pFile, class CAnimTblEdDoc *pDoc);
	void ExportMacros(FILE *pFile, int Shift);
	void ExportDefines(FILE *pFile, int Shift);
	void ShiftStateClasses(class CAnimTreeNode *pNode, int Operation, int StateClass, int State, int Bit, int Num);
	void Serialize(CArchive& ar);
	int GetMask(int StartBit);
	CStateClass* FindStateClass(CString &Name, CStateClass *pSkipStateClass=NULL);
	void SetStateClassName(int Index, CString &Name);
	int GetStateClassStartBit(CStateClass *pClass);
	int GetStateClassStartBit(int StateClass);
	CStateClassList& operator= (CStateClassList&);
	void MakeEmpty(void);
	int GetCount(void);
	CStateClass* GetStateClass(int Index);
	int RemoveStateClass(int Index);
	int RemoveStateClass(CString &Name);
	int RemoveStateClass(CStateClass *pStateClass);
	int AddStateClass(CString &Name=CString("NEW_CLASS"));
	int GetTotalStateBits(void);
	CStateClassList();
	virtual ~CStateClassList();

};

#endif // !defined(AFX_STATECLASSLIST_H__86A11B20_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_)

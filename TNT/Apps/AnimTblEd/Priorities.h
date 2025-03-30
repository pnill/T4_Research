// Priorities.h: interface for the CPriorities class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRIORITIES_H__A4F7C707_E046_11D2_9248_00105A29F84B__INCLUDED_)
#define AFX_PRIORITIES_H__A4F7C707_E046_11D2_9248_00105A29F84B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPriorities : public CObject  
{
private:
	CStringList m_Priorities;

public:
	void ExportMacros(FILE *pFile, int Shift);
	void ExportDefines(FILE *pFile, int Shift);
	void ShiftPriorities(class CAnimTreeNode *pNode, int Operation, int Priority);
	void Serialize(CArchive &ar);
	CPriorities& operator =(CPriorities &Other);
	int GetMask(int StartBit);
	void AddPriority(CString &Name=CString("NEW_PRIORITY"), int Index = -1);
	int CountNumBits(int Value);
	CString& GetPriority(int Index);
	int FindPriority(CString &Name);
	void RemovePriority(int Index);
	void MakeEmpty(void);
	int GetTotalPriorityBits(void);
	int GetCount(void);
	CPriorities();
	virtual ~CPriorities();

};

#endif // !defined(AFX_PRIORITIES_H__A4F7C707_E046_11D2_9248_00105A29F84B__INCLUDED_)

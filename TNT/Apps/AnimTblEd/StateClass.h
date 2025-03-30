// StateClass.h: interface for the CStateClass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATECLASS_H__64B841E1_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_)
#define AFX_STATECLASS_H__64B841E1_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_

#include <afxtempl.h>
#include "State.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStateClass : public CObject  
{
private:
	int CountNumBits(int Value);
	class CStateClassList *m_pParentList;
	int m_NumBits;
	CTypedPtrList<CObList, CState*>	m_States;
	CString m_Name;

public:
	void Serialize(CArchive& ar);
	void SetSetting(int &BitSet, int Value);
	int GetSetting(int BitSet);
	int GetMask();
	CState* FindState(CString &Name, CState *pSkip=NULL);
	CStateClass& operator =(CStateClass &Other);
	CState* GetState(int Index);
	void MakeEmpty(void);
	CString & GetName(void);
	void RemoveState(int Index);
	void RemoveState(CString &Name);
	void RemoveState(CState *pState);
	int SetName(CString &Name);
	void AddState(CString &Name=CString("NEW_STATE"), int Index=-1);
	int GetStartBit(void);
	int GetNumBits(void);
	int GetNumStates(void);
	CStateClass(class CStateClassList *pParent, CString &Name=CString("NEW_CLASS"), int StartBit=0);
	virtual ~CStateClass();

};

#endif // !defined(AFX_STATECLASS_H__64B841E1_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_)

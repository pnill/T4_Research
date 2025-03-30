// State.h: interface for the CState class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATE_H__64B841E3_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_)
#define AFX_STATE_H__64B841E3_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CState : public CObject  
{
private:
	CString m_Name;

public:
	void Serialize(CArchive &ar);
	CState& operator =(CState &Other);
	CString& GetName(void);
	void SetName(CString &Name);
	CState(CString &Name=CString("NEW STATE"));
	virtual ~CState();

};

#endif // !defined(AFX_STATE_H__64B841E3_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_)

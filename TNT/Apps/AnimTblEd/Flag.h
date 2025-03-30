// Flag.h: interface for the CFlag class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLAG_H__86A11B23_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_)
#define AFX_FLAG_H__86A11B23_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFlag : public CObject  
{
private:
	class CFlagList* m_pParent;
	CString m_Name;

public:
	void Serialize(CArchive &ar);
	int GetMask(void);
	int GetBit(void);
	CFlag& operator= (CFlag &Other);
	void SetName(CString &Name);
	CString& GetName();
	CFlag(CFlagList *pParent, CString &Name=CString("NEW FLAG"));
	virtual ~CFlag();

};

#endif // !defined(AFX_FLAG_H__86A11B23_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_)

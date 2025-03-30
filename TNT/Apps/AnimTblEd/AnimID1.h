// AnimID1.h: interface for the CAnimID class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMID1_H__46CE1A21_749F_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMID1_H__46CE1A21_749F_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAnimID : public CObject  
{
public:
	int m_nOccurances;
	int GetUses();
	bool IsFound();
	void MarkFound();
	void MarkUnfound();
	int DecrementUses();
	int IncrementUses();
	CAnimID(const char *lpszAnimID);
	CString m_AnimID;
	CAnimID();
	virtual ~CAnimID();

};

#endif // !defined(AFX_ANIMID1_H__46CE1A21_749F_11D2_9248_00105A29F8F3__INCLUDED_)

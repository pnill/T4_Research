// AnimTrans.h: interface for the CAnimTrans class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMTRANS_H__67F8F500_DBB8_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMTRANS_H__67F8F500_DBB8_11D2_9248_00105A29F8F3__INCLUDED_

#include "IDList.h"
#include "AnimTreeNode.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAnimTrans : public CAnimTreeNode
{
public:
//	const char * GetName();
	const CString& GetTag( void ){ return m_Tag; }
	void SetTag( const CString& Tag ){ m_Tag = Tag; }
	CString GetTagName( void ){ return GetTag() + GetName(); }
	const CString& GetName( void );
	void SetName( CString& Name ){ m_Name = Name; }
	int GetImage(void);
	int GetSelectedImage(void);
	CAnimTrans& operator =(CAnimTrans &Other);
	void Export(FILE *pFile);
	bool Edit();
	int GetMouseMenuIndex();
	int GetDeleteOptionEnabled();
	int GetEditOptionEnabled();
	bool HasChildren();
	void Copy(CAnimTrans &AnimTrans);
	int SetDisabled(int Set);
	int GetDisabled();
	void Export(FILE *pFile, const char *lpszAnimName);
	bool Edit(const char *lpszAnimName);
	void Serialize(CArchive &ar);
	void SetBlendFrames(int nBlendFrames)
		{m_nBlendFrames = nBlendFrames;};
//	bool Edit(CIDList *pAnimList, CAnimNode *pAnimNode);
	bool IsTransition();
	bool IsPrerequisite();
	const char * GetPlayAnim()
		{return m_PlayAnim;};
	const char * GetToAnim()
		{return m_ToAnim;};
	const char * GetFromAnim()
		{return m_FromAnim;};
	int GetBlendFrames()
		{return m_nBlendFrames;};
	void GetNLDescription(CString &Description, const char *lpszFromAnim, const char *lpszToAnim, const char *lpszPlayAnim);
	void GetNLDescription(CString &Description);
	void SetPlayAnim(const char *lpszAnim);
	void SetToAnim(const char *lpszAnim);
	void SetFromAnim(const char *lpszAnim);
	CAnimTrans(class CAnimTblEdDoc *pDoc, class CAnimTreeNode *pParent);
	virtual ~CAnimTrans();

private:
	CString m_Name;
	int m_Disable;
	int m_nBlendFrames;
	CString m_PlayAnim;
	CString m_ToAnim;
	CString m_FromAnim;
	CString m_Tag;		// used in a dialog box to tag dependancies which come from a parent.
};

#endif // !defined(AFX_ANIMTRANS_H__67F8F500_DBB8_11D2_9248_00105A29F8F3__INCLUDED_)

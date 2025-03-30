// AnimTreeNode.h: interface for the CAnimTreeNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMTREENODE_H__2DE9B941_6F29_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMTREENODE_H__2DE9B941_6F29_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

#define	CHILD_POS_STACK_SIZE	10

#define		STATE_ERROR_BIT		(1 << 0)
class CScanSourceCodeDlg;

class CAnimTreeNode : public CObject  
{
public:
	CString m_VPString;
	int m_Type;
	int m_Count;
	class CAnimTblEdDoc *m_pDoc;
	CAnimTreeNode * m_pParent;

public:
	virtual void SwapEquivalency( void );
	virtual void SwitchCutTablePointer( void ){};
	int GetNumChildren( void ){ return m_nChildren; }
	virtual int IsExported();
	virtual int GetState();
	virtual CAnimTreeNode* FindTreeNodeByName( CString& Name );
	virtual void ExportChildren(FILE *pFile);
	void AddToListBox(CListBox *pListBox, int nFlags);
	int CountLocomotionAnimations(int Count=0);
	void IncExportTransIndex(int Inc);
	int GetCurrentExportTransIndex();
	void ClearExportTransIndex();
	virtual int GetNumAnimsWithExtras();
	virtual int GetNumAnimsWithDependancies( bool Recount, bool CountDisabledOnes );
	virtual int GetTotalNumDependancies( bool Recount, bool CountDisabledOnes );
	virtual int GetNumAnimsWithDependanciesWithLink( bool Recount, bool CountDisabledOnes );
	virtual int GetTotalNumDependanciesWithLink( bool Recount, bool CountDisabledOnes );
	virtual CAnimTreeNode * Clone();
	virtual int GetNumAnims();
	virtual int GetNumExportedAnims();
	virtual CAnimTreeNode * GetPrevChild(CAnimTreeNode *pNodeAfter);
	int IsAncestor(CAnimTreeNode *pAnimTreeNode);
	virtual int CanHaveChildren();
	void CopyBaseNode(CAnimTreeNode &AnimTreeNode);
	virtual void Copy(CAnimTreeNode &AnimTreeNode)
	{ASSERT(0 && "Derived class must define Copy funtion.");}
	void DecCount();
	CAnimTreeNode();
	class CAnimTblEdDoc * GetDocument()
	{return m_pDoc;};
	virtual void AddChild(CAnimTreeNode *pAnimTreeNode, CAnimTreeNode *pRelativeNode = NULL, int InsertAfter = 0);
	void RemoveChild(CAnimTreeNode *pFindNode);
	CAnimTreeNode* SeverChild(CAnimTreeNode *pAnimTreeNode);
	CAnimTreeNode* GetNextChild(CAnimTreeNode *pNodeBefore = NULL);
	CAnimTreeNode* GetFirstChild();
	int GetAddTransOptionEnabled();
	void DoExport(FILE *pFile, int ExportType);
	static int m_CurrentExportType;
	static int m_LastTokenType;
	virtual void Export(FILE *pFile);
	virtual int GetDeleteOptionEnabled();
	virtual int GetEditOptionEnabled();
	virtual int GetAddAnimOptionEnabled();
	virtual int GetAddGroupOptionEnabled();
    virtual int GetScanSourceCodeOptionEnabled();
	void WriteTreeItem(CArchive &ar);
	virtual void SetFirstChild();
//	virtual CAnimTreeNode* GetNextNode();
	CAnimTreeNode* ReadTreeItem(CArchive &ar, CAnimTreeNode *pParent);
	void Serialize(CArchive &ar);
	virtual bool HasChildren();
	CAnimTreeNode * GetParent()
		{return m_pParent;};
	void SetParent(CAnimTreeNode *pAnimTreeNode)
		{m_pParent = pAnimTreeNode;};
	virtual CString& GetVPString( int* pDisplayOrder ){m_VPString = GetName(); return m_VPString;};
	virtual int GetSelectedImage();
	virtual int GetImage();
	virtual int GetStateImage();
	virtual bool Edit();
	virtual int GetMouseMenuIndex();
	virtual const CString& GetName();
//	virtual const char * GetName();
	virtual void Remove(class CIDList &IDList);
	void SetType(int newType);
	CAnimTreeNode(class CAnimTblEdDoc *pDoc, CAnimTreeNode *pParent = NULL);
	virtual ~CAnimTreeNode();
	void PushChildStep();
	void PopChildStep();
	void ModifyViews();
	void SetIgnore( bool Ignore );
	bool GetIgnore();
	void SetSearchMatched( bool SearchMatched );
	bool GetSearchMatched( void );

    CScanSourceCodeDlg* GetAttachedScanSourceDlg() { return m_pSSCDlg; }
    void AttachScanSourceDlg( CScanSourceCodeDlg* dlg );
    void DetachScanSourceDlg();

protected:
	POSITION m_ChildPosStack[CHILD_POS_STACK_SIZE];
	int	m_ChildPosStackIndex;

	POSITION m_ChildPos;
	CTypedPtrList<CObList, CAnimTreeNode*> m_ChildList;
	int m_nChildren;

	int GetLastTokenType();
	void SetLastTokenType(int token);
	int GetCurrentExportType();
	void SetCurrentExportType(int type);
	void ClearCount();
	int GetCount();
	void IncCount();
	BOOL m_Ignore;
	bool m_bSearchMatched;	// flags whether the last search applied to this tree item

private:
    CScanSourceCodeDlg *m_pSSCDlg;
};

#endif // !defined(AFX_ANIMTREENODE_H__2DE9B941_6F29_11D2_9248_00105A29F8F3__INCLUDED_)

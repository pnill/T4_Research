// NodeSearchMap.h: interface for the CNodeSearchMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODESEARCHMAP_H__FB625EF7_3AE0_4361_A0AB_9297EC7B3839__INCLUDED_)
#define AFX_NODESEARCHMAP_H__FB625EF7_3AE0_4361_A0AB_9297EC7B3839__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAnimTblEdDoc;
class CAnimNode;
class CGroupNode;
class CAnimTreeNode;

class CNodeSearchMap  
{
public:
	void SetSearchGroups(bool SearchGroups);
	bool IsAnimSearchable();
	void SetSearchCutTable(CString CutTable);
	void SetSearchSelectCallback(CString SelectCallback);
	void SetSearchCallback(CString Callback);
	void SetSearchPriority(int Priority);
	void SetSearchMethod(int Method);
	void SetSearchFlags(int Flags);
	void SetORSearch();
	void SetANDSearch();
	BOOL IsANDSearch() {return m_bANDSearch;};
	BOOL IsORSearch() {return !m_bANDSearch;};
	int IsMatchAnim(CAnimNode *pAnimNode);
	int IsMatchGroup(CGroupNode *pGroupNode);
	BOOL IsGroupSearchable();
	int IsMatch(CAnimTreeNode *pAnimTreeNode);
	int DoSearch(CAnimTreeNode *pAnimTreeNode);
	int DoSearch( CAnimTblEdDoc *pDoc );
	void SetSearchRightID(CString RightID);
	void SetSearchLeftID(CString LeftID);
	void SetSearchName( CString Name );
	BOOL IsSearchable();
	CNodeSearchMap();
	virtual ~CNodeSearchMap();

protected:
	BOOL m_bANDSearch;
	bool m_bSearchGroups;

	//---	Search Criteria
	BOOL			m_bSearchName;
	CString			m_sName;

	BOOL			m_bSearchLeftID;
	CString			m_sLeftID;

	BOOL			m_bSearchRightID;
	CString			m_sRightID;

	BOOL			m_bSearchMethod;
	int				m_nMethod;

	BOOL			m_bSearchPriority;
	int				m_nPriority;

	BOOL			m_bSearchFlags;
	int				m_nFlags; 

	BOOL			m_bSearchCallback;
	CString			m_sCallback;

	BOOL			m_bSearchSelectCallback;
	CString			m_sSelectCallback;

	BOOL			m_bSearchCutTable;
	CString			m_sCutTable;
};

#endif // !defined(AFX_NODESEARCHMAP_H__FB625EF7_3AE0_4361_A0AB_9297EC7B3839__INCLUDED_)

#if !defined(AFX_SOURCEIDDLG_H__6C7DE8A3_7A57_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_SOURCEIDDLG_H__6C7DE8A3_7A57_11D2_9248_00105A29F8F3__INCLUDED_

#include "AnimTblEdDoc.h"	// Added by ClassView
#include "AnimNode.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SourceIDDlg.h : header file
//

#define	MAX_SELECTIONS		256

/////////////////////////////////////////////////////////////////////////////
// CSourceIDDlg dialog

class CSourceIDDlg : public CDialog
{
// Construction
public:
	CSourceIDDlg(CWnd* pParent = NULL);   // standard constructor

	void FindString(const char *StringToFind);
	void BuildFilterList();
	void BuildList();

	int		m_Modified;
	CString m_AnimID;
	CIDList *m_pIDList;

	//---	following variables only used to implement multiple selection
	int		m_MultipleSelections;
	CString	m_AnimIDs[MAX_SELECTIONS];
	int		m_NumSelections;

// Dialog Data
	//{{AFX_DATA(CSourceIDDlg)
	enum { IDD = IDD_SOURCEID };
	CListBox	m_SourceIDListMult;
	CListBox	m_SourceIDList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSourceIDDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	static int m_Mode;
	CListBox*	m_pSourceIDList;

	// Generated message map functions
	//{{AFX_MSG(CSourceIDDlg)
	afx_msg void OnRadioassigned();
	afx_msg void OnRadiounassigned();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkSourceidlist();
	afx_msg void OnCheckEnablefilters();
	afx_msg void OnButtonNewfilter();
	afx_msg void OnButtonDeletefilter();
	afx_msg void OnSelchangeListFilter();
	afx_msg void OnSelcancelListFilter();
	afx_msg void OnSelchangeDisplaytab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOURCEIDDLG_H__6C7DE8A3_7A57_11D2_9248_00105A29F8F3__INCLUDED_)

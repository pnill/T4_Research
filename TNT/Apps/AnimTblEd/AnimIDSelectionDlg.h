#if !defined(AFX_ANIMIDSELECTIONDLG_H__67F8F503_DBB8_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMIDSELECTIONDLG_H__67F8F503_DBB8_11D2_9248_00105A29F8F3__INCLUDED_

#include "IDList.h"
#include "AnimTblEdDoc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnimIDSelectionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimIDSelectionDlg dialog

class CAnimIDSelectionDlg : public CDialog
{
// Construction
public:
	void SetOmitChoice(const char *lpszAnimID);
	void AddExtraChoice(const char *lpszExtraChoice);
	void BuildList();
	CAnimIDSelectionDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimIDSelectionDlg)
	enum { IDD = IDD_ANIMID };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimIDSelectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	CAnimTblEdDoc * m_pDoc;
	CString m_OmitChoice;
	CString m_ExtraChoice;
	CIDList * m_pAnimList;
	CString m_SelectedAnim;

	static int		m_Mode;	// mode of display

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimIDSelectionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListAnimations();
	virtual void OnOK();
	afx_msg void OnCheckAnimgrouplist();
	afx_msg void OnSelchangeDisplaytab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMIDSELECTIONDLG_H__67F8F503_DBB8_11D2_9248_00105A29F8F3__INCLUDED_)

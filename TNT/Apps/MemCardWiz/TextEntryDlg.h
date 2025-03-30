#if !defined(AFX_TEXTENTRYDLG_H__E34037BF_6891_4FA3_AB80_D2509DEAAC44__INCLUDED_)
#define AFX_TEXTENTRYDLG_H__E34037BF_6891_4FA3_AB80_D2509DEAAC44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextEntryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextEntryDlg dialog

class CTextEntryDlg : public CDialog
{
// Construction
public:
	CTextEntryDlg(int MaxChars, const char* pWindowText = NULL, CWnd* pParent = NULL);   // standard constructor

	CString m_TitleText;
	int     m_MaxChars;

// Dialog Data
	//{{AFX_DATA(CTextEntryDlg)
	enum { IDD = IDD_DLG_TEXTEDIT };
	CString	m_TextEntry;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTextEntryDlg)
	virtual BOOL OnInitDialog();
    virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTENTRYDLG_H__E34037BF_6891_4FA3_AB80_D2509DEAAC44__INCLUDED_)

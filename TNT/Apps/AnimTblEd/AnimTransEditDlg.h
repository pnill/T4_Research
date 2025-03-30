#if !defined(AFX_ANIMTRANSEDITDLG_H__67F8F502_DBB8_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMTRANSEDITDLG_H__67F8F502_DBB8_11D2_9248_00105A29F8F3__INCLUDED_

#include "IDList.h"
#include "AnimTrans.h"
#include "AnimNode.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnimTransEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimTransEditDlg dialog

class CAnimTransEditDlg : public CDialog
{
// Construction
public:
	CAnimTblEdDoc * m_pDoc;
	int m_OldDisable;
	CString m_AnimName;
	int m_OldBlendFrames;
	void FillLocalVars();
	CString m_PlayAnim;
	CString m_ToAnim;
	CString m_FromAnim;
	void DrawDialog();
//	CIDList * m_pAnimList;
	CAnimTrans * m_pAnimTrans;
	CAnimTransEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimTransEditDlg)
	enum { IDD = IDD_TRANSITIONEDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimTransEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimTransEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonChangefrom();
	afx_msg void OnButtonChangeplay();
	afx_msg void OnButtonChangeto();
	afx_msg void OnSelchangeComboPrereqtrans();
	virtual void OnOK();
	afx_msg void OnUpdateEditBlendframes();
	virtual void OnCancel();
	afx_msg void OnCheckDisable();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMTRANSEDITDLG_H__67F8F502_DBB8_11D2_9248_00105A29F8F3__INCLUDED_)

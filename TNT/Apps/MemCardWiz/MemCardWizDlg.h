// MemCardWizDlg.h : header file
//

#if !defined(AFX_MEMCARDWIZDLG_H__989C8CF6_A48F_47D0_B668_816CFEC1A92A__INCLUDED_)
#define AFX_MEMCARDWIZDLG_H__989C8CF6_A48F_47D0_B668_816CFEC1A92A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MemCardProject.h"


/////////////////////////////////////////////////////////////////////////////
// CMemCardWizDlg dialog

class CMemCardWizDlg : public CDialog
{
// Construction
public:
	CMemCardWizDlg(CWnd* pParent = NULL);	// standard constructor

	MCW_Project		m_MCProj;
	int				m_CurFType;
	int				m_bCurView;

// Dialog Data
	//{{AFX_DATA(CMemCardWizDlg)
	enum { IDD = IDD_DLG_MEMCARDWIZ };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMemCardWizDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	BOOL IsPS2View ( void );
	BOOL IsGCView  ( void );
	BOOL IsXBOXView( void );
	BOOL IsPCView( void );

	void InitAPPControls ( void );
	void InitPS2Controls ( void );
	void InitGCControls  ( void );
	void InitXBOXControls( void );
	void InitPCControls( void );

	void EnableIconControls( BOOL bEnable );
	void EnablePS2Controls ( BOOL bEnable );
	void EnableGCControls  ( BOOL bEnable );
	void EnableXBOXControls( BOOL bEnable );
	void EnablePCControls( BOOL bEnable );

	void ShowPS2Controls ( BOOL bShow );
	void ShowGCControls  ( BOOL bShow );
	void ShowXBOXControls( BOOL bShow );
	void ShowPCControls( BOOL bShow );

	void GetFileTypeInfoFromControls( void );
	void SetControlsFromFileTypeInfo( void );

	void GetDataFromControls( void );
	void SetControlsFromData( void );

	void PS2_UpdateMaxFile( int MaxFiles );
	void PS2_GetDataFromControls( void );
	void PS2_SetControlsFromData( void );

	void GC_GetDataFromControls( void );
	void GC_SetControlsFromData( void );

	void XBOX_GetDataFromControls( void );
	void XBOX_SetControlsFromData( void );

	void PC_GetDataFromControls( void );
	void PC_SetControlsFromData( void );

	void ImportProject( const CString& Filename );
	void ExportProject( const CString& Filename );

	// Generated message map functions
	//{{AFX_MSG(CMemCardWizDlg)
	virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnOpen();
	afx_msg void OnBtnSave();
	afx_msg void OnBtnExport();
	afx_msg void OnBtnAddtype();
	afx_msg void OnBtnDeletetype();
	afx_msg void OnBtnRenametype();
	afx_msg void OnSelchangeListFiletypes();
	afx_msg void OnRadioViewPs2();
	afx_msg void OnRadioViewGc();
	afx_msg void OnRadioViewXbox();
	afx_msg void OnRadioViewPC();
	afx_msg void OnBtnIconBrowse();
	afx_msg void OnBtnIconEdit();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDeltaposSpinPs2Nfiles(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMCARDWIZDLG_H__989C8CF6_A48F_47D0_B668_816CFEC1A92A__INCLUDED_)

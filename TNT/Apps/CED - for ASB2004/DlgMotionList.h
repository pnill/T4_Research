#if !defined(AFX_DLGMOTIONLIST_H__29EF31E0_0132_11D2_AEC0_00A024569FF3__INCLUDED_)
#define AFX_DLGMOTIONLIST_H__29EF31E0_0132_11D2_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgMotionList.h : header file
//

class CMotionEditView ;
class CCharacter ;
class CCeDDoc ;
class CMotion ;

/////////////////////////////////////////////////////////////////////////////
// CDlgMotionList dialog

class CDlgMotionList : public CDialog
{
//---	Attributes
public:
	CMotionEditView	*m_pView ;								// Pointer to View
	int				m_Updating ;

//---	Operations
public:
	void	UpdateMotionList	( CCharacter *pCharacter ) ;					// Update List of Motions

	void	UpdateLinkCharacterList( CCeDDoc *pDoc, CMotion *pControlMotion ) ;
	void	UpdateLinkMotionList( CMotion *pControlMotion ) ;

	void	UpdateFaceMoveDir( ) ;
	void	UpdateMotionSymbol( ) ;

	void	UpdateExportInfo();

// Construction
public:
	void UpdateAutoResetMoveDir();
	void UpdateAutoResetFaceDir();
	CDlgMotionList(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgMotionList)
	enum { IDD = IDD_MOTIONLIST };
	CButton	m_ExportLink;
	CButton	m_ExportMotionFPSButton;
	CEdit	m_ExportMotionFPS;
	CSpinButtonCtrl	m_ExportMotionFPSSpin;
	CSpinButtonCtrl	m_ExportMotionBitsSpin;
	CButton	m_ExportMotionBitsButton;
	CEdit	m_ExportMotionBits;
	CButton	m_NoExport;
	CButton	m_ButtonAutoResetMove;
	CButton	m_ButtonAutoResetFace;
	CEdit	m_Symbol;
	CEdit	m_MoveDir;
	CEdit	m_FaceDir;
	CComboBox	m_LinkMotion;
	CComboBox	m_LinkCharacter;
	CListBox	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMotionList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnToolTipNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );

	// Generated message map functions
	//{{AFX_MSG(CDlgMotionList)
	afx_msg void OnMotionAdd();
	afx_msg void OnMotionDelete();
	afx_msg void OnSelchangeMotionList();
	afx_msg void OnSelchangeLinkcharacter();
	afx_msg void OnSelchangeLinkmotion();
	afx_msg void OnChangeFacedir();
	afx_msg void OnChangeMovedir();
	afx_msg void OnResetface();
	afx_msg void OnResetmove();
	afx_msg void OnChangeSymbol();
	afx_msg void OnSetallFace();
	afx_msg void OnSetallMove();
	afx_msg void OnMotionFlags();
	afx_msg void OnSetRemainingMoveDirs();
	afx_msg void OnSetRemainingFaceDirs();
	afx_msg void OnCheckAutoclearFaceDirMotionlist();
	afx_msg void OnCheckAutoclearMoveDirMotionlist();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkMotionList();
	afx_msg void OnChangeMotionExportbits();
	afx_msg void OnDeltaposMotionExportbitsSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeMotionExportfps();
	afx_msg void OnDeltaposMotionExportfpsSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNoexport();
	afx_msg void OnKillfocusMotionExportfps();
	afx_msg void OnKillfocusMotionExportbits();
	afx_msg void OnMotionExportbitsButton();
	afx_msg void OnMotionExportfpsButton();
	afx_msg void OnExportlink();
	//}}AFX_MSG

	afx_msg void OnOk();
	afx_msg void OnCancel();
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMOTIONLIST_H__29EF31E0_0132_11D2_AEC0_00A024569FF3__INCLUDED_)

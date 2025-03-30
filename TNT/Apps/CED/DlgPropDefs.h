#if !defined(AFX_DLGPROPDEFS_H__48AADB2C_29A0_4C78_A6D0_A1EA8B99B3D0__INCLUDED_)
#define AFX_DLGPROPDEFS_H__48AADB2C_29A0_4C78_A6D0_A1EA8B99B3D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgPropDefs.h : header file
//
#include "x_types.hpp"

class CMotionEditView;
class CCharacter;
class CPropDef;

/////////////////////////////////////////////////////////////////////////////
// CDlgPropDefs dialog

class CDlgPropDefs : public CDialog
{
public:

	CMotionEditView*	m_pView;
	bool				m_bInUpdateDetails;

	void		UpdateDetails			( void );
				
	void		FillPropDefList			( void );
	void		FillBoneCombo			( s32 Select );
	void		FillLinkCharacterCombo	( const CCharacter* pSelect );
				
	void		EnablePropDef			( bool bEnable );
	void		EnableDialog			( bool bEnable );

	CPropDef*	UpdateControlReady		( void );
	void		UpdateMotionView		( void );

	void		SetSelectedPropDef		( s32 Prop );

// Construction
public:
	CDlgPropDefs(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgPropDefs)
	enum { IDD = IDD_PROPDEFS };
	CButton	m_MirrorWithAnim;
	CButton	m_Mirrored;
	CButton	m_FacingRelativeRot;
	CButton	m_UseLinkedRot;
	CButton	m_UseLinkedPos;
	CButton	m_Scaled;
	CButton	m_FacingRelativePos;
	CButton	m_StaticRot;
	CButton	m_StaticPos;
	CEdit	m_RotZ;
	CEdit	m_RotY;
	CEdit	m_RotX;
	CEdit	m_Name;
	CListBox	m_PropDefList;
	CEdit	m_PosZ;
	CEdit	m_PosY;
	CEdit	m_PosX;
	CComboBox	m_LinkCharacterCombo;
	CComboBox	m_BoneCombo;
	CButton	m_Exported;
	CButton	m_Active;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPropDefs)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgPropDefs)
	afx_msg void OnSelchangePropdeflist();
	afx_msg void OnActive();
	afx_msg void OnExported();
	afx_msg void OnStaticpos();
	afx_msg void OnStaticrot();
	afx_msg void OnChangePropname();
	afx_msg void OnChangePosx();
	afx_msg void OnChangePosy();
	afx_msg void OnChangePosz();
	afx_msg void OnChangeRotx();
	afx_msg void OnChangeRoty();
	afx_msg void OnChangeRotz();
	virtual void OnOK();
	afx_msg void OnSelchangeLinkcharactercombo();
	afx_msg void OnSelchangeBonecombo();
	afx_msg void OnLoad();
	afx_msg void OnFacingrelative();
	afx_msg void OnScaled();
	afx_msg void OnUselinkedpos();
	afx_msg void OnUselinkedrot();
	afx_msg void OnFacingrelativerot();
	afx_msg void OnHelpbutton();
	afx_msg void OnMirrored();
	afx_msg void OnMirrorwithanim();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPROPDEFS_H__48AADB2C_29A0_4C78_A6D0_A1EA8B99B3D0__INCLUDED_)

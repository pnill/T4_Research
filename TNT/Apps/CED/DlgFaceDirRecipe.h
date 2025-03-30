#if !defined(AFX_DLGFACEDIRRECIPE_H__07559CCF_C939_4977_BFE5_B6D103A9703D__INCLUDED_)
#define AFX_DLGFACEDIRRECIPE_H__07559CCF_C939_4977_BFE5_B6D103A9703D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Character.h"

// DlgFaceDirRecipe.h : header file
//
struct SDlgBoneWeight
{
	CComboBox*			pBone;
	CEdit*				pWeight;
	CSpinButtonCtrl*	pSpin;
	CEdit*				pPercent;
};

class CCharacter;

#define	FACEDIR_RECIPE_PAGE_LENGTH	MAX_FACEDIR_RECIPE_LENGTH

/////////////////////////////////////////////////////////////////////////////
// CDlgFaceDirRecipe dialog

class CDlgFaceDirRecipe : public CDialog
{
// Construction
public:
	CDlgFaceDirRecipe(CWnd* pParent = NULL);   // standard constructor

	CCharacter*		m_pCharacter;
	s32				m_RecipeLength;
	SDlgBoneWeight	m_Recipe[FACEDIR_RECIPE_PAGE_LENGTH];
	xbool			m_bInitCalled;

// Dialog Data
	//{{AFX_DATA(CDlgFaceDirRecipe)
	enum { IDD = IDD_FACEDIR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgFaceDirRecipe)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateDisplay( void );
	void SelchangeBone( s32 Index );
	void ChangeWeight( s32 Index );

	// Generated message map functions
	//{{AFX_MSG(CDlgFaceDirRecipe)
	afx_msg void OnAddbone();
	afx_msg void OnRemovebone();
	afx_msg void OnSelchangeBone0();
	afx_msg void OnSelchangeBone1();
	afx_msg void OnSelchangeBone2();
	afx_msg void OnSelchangeBone3();
	afx_msg void OnSelchangeBone4();
	afx_msg void OnSelchangeBone5();
	afx_msg void OnChangeWeight0();
	afx_msg void OnChangeWeight1();
	afx_msg void OnChangeWeight2();
	afx_msg void OnChangeWeight3();
	afx_msg void OnChangeWeight4();
	afx_msg void OnChangeWeight5();
	virtual BOOL OnInitDialog();
	afx_msg void OnCancelMode();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGFACEDIRRECIPE_H__07559CCF_C939_4977_BFE5_B6D103A9703D__INCLUDED_)

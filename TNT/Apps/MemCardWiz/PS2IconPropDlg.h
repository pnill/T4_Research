#if !defined(AFX_PS2ICONPROPDLG_H__6303FEA9_DED3_4D2A_A6B2_995409BC1AD7__INCLUDED_)
#define AFX_PS2ICONPROPDLG_H__6303FEA9_DED3_4D2A_A6B2_995409BC1AD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PS2IconPropDlg.h : header file
//

#include "MemCardProject.h"


/////////////////////////////////////////////////////////////////////////////
// CPS2IconPropDlg dialog

class CPS2IconPropDlg : public CDialog
{
// Construction
public:
	CPS2IconPropDlg(CWnd* pParent = NULL);   // standard constructor

	void SetIconInfo( const MC_PS2IconInfo& rIconInfo );
	void GetIconInfo( MC_PS2IconInfo& rIconInfo ) const;

// Dialog Data
	//{{AFX_DATA(CPS2IconPropDlg)
	enum { IDD = IDD_DLG_PS2ICON };
	BYTE	m_AMB_B;
	BYTE	m_AMB_G;
	BYTE	m_AMB_R;
	BYTE	m_BK_Alpha;
	BYTE	m_BK_BL_B;
	BYTE	m_BK_BL_G;
	BYTE	m_BK_BL_R;
	BYTE	m_BK_BR_B;
	BYTE	m_BK_BR_G;
	BYTE	m_BK_BR_R;
	BYTE	m_BK_TL_B;
	BYTE	m_BK_TL_G;
	BYTE	m_BK_TL_R;
	BYTE	m_BK_TR_B;
	BYTE	m_BK_TR_G;
	BYTE	m_BK_TR_R;
	BYTE	m_L1_B;
	BYTE	m_L1_G;
	BYTE	m_L1_R;
	float	m_L1_X;
	float	m_L1_Y;
	float	m_L1_Z;
	BYTE	m_L2_B;
	BYTE	m_L2_G;
	BYTE	m_L2_R;
	float	m_L2_X;
	float	m_L2_Y;
	float	m_L2_Z;
	BYTE	m_L3_B;
	BYTE	m_L3_G;
	BYTE	m_L3_R;
	float	m_L3_X;
	float	m_L3_Y;
	float	m_L3_Z;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPS2IconPropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPS2IconPropDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PS2ICONPROPDLG_H__6303FEA9_DED3_4D2A_A6B2_995409BC1AD7__INCLUDED_)

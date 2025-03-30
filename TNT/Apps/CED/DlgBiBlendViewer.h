#if !defined(AFX_DLGBIBLENDVIEWER_H__8A6F7E5D_AA17_47A1_B631_8BA2FF89E49F__INCLUDED_)
#define AFX_DLGBIBLENDVIEWER_H__8A6F7E5D_AA17_47A1_B631_8BA2FF89E49F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgBiBlendViewer.h : header file
//

#include "x_types.hpp"
#include "x_math.hpp"

class CMotionEditView;
class CMotion;

/////////////////////////////////////////////////////////////////////////////
// DlgBiBlendViewer dialog

class DlgBiBlendViewer : public CDialog
{
// Construction
public:
	void SetAnimRatio( f32 Ratio, s32 WhichOne );
	void Reset( void );
	DlgBiBlendViewer(CWnd* pParent = NULL);   // standard constructor

	CMotionEditView*	m_pView;

	CString				m_Motion0;
	CString				m_Motion1;

	CMotion*			m_pMotion0;
	CMotion*			m_pMotion1;

	bool				m_bChangeRatio;
	f32					m_Ratio;

// Dialog Data
	//{{AFX_DATA(DlgBiBlendViewer)
	enum { IDD = IDD_BIDIRECTIONALBLENDVIEWER };
	CSliderCtrl	m_Slider;
	CEdit	m_Anim1Percent;
	CEdit	m_Anim0Percent;
	CComboBox	m_Anim1;
	CComboBox	m_Anim0;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgBiBlendViewer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgBiBlendViewer)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeAnim0();
	afx_msg void OnEditchangeAnim1();
	afx_msg void OnChangeAnim0percent();
	afx_msg void OnChangeAnim1percent();
	afx_msg void OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusAnim0percent();
	afx_msg void OnKillfocusAnim1percent();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeAnim1();
	afx_msg void OnSelchangeAnim0();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBIBLENDVIEWER_H__8A6F7E5D_AA17_47A1_B631_8BA2FF89E49F__INCLUDED_)

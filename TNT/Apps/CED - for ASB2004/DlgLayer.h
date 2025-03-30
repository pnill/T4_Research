#if !defined(AFX_DLGLAYER_H__6D1B943A_F8FB_4821_A7A7_3818B7054145__INCLUDED_)
#define AFX_DLGLAYER_H__6D1B943A_F8FB_4821_A7A7_3818B7054145__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgLayer.h : header file
//

class CMovieView ;

/////////////////////////////////////////////////////////////////////////////
// CDlgLayer dialog

class CDlgLayer : public CDialog
{
//---	Attributes
public:
	CMovieView		*m_pView ;								// Pointer to View

protected:
	int				m_SelectedLayer ;

//---	Operations
public:
	int		GetViewLayer();
	int		GetEditLayer();
	void	UpdateDetails	( ) ;							// Update Details
	void	EnsureViewable	( int Layer );					// make sure that a layer is viewable


// Construction
public:
	CDlgLayer(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgLayer)
	enum { IDD = IDD_EDIT_LAYER };
	CButton	m_ViewAllLayers;
	CComboBox	m_Layer;
	CButton	m_DeleteLayer;
	CButton	m_AddLayer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgLayer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgLayer)
	afx_msg void OnEditchangeLayer();
	afx_msg void OnDeletelayer();
	afx_msg void OnAddlayer();
	afx_msg void OnSelchangeLayer();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditlayer();
	afx_msg void OnViewalllayers();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGLAYER_H__6D1B943A_F8FB_4821_A7A7_3818B7054145__INCLUDED_)

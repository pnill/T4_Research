#if !defined(AFX_DLGACTORLIST_H__570E9704_FFC1_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_DLGACTORLIST_H__570E9704_FFC1_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgActorList.h : header file
//

class CMovieView ;
class CActor ;

/////////////////////////////////////////////////////////////////////////////
// CDlgActorList dialog

class CDlgActorList : public CDialog
{
//---	Attributes
public:
	CMovieView		*m_pView ;								// Pointer to View
	bool			m_BuildList ;

//---	Operations
public:
	void	Reset				( ) ;						// Reset List
	void	AddActor			( CActor *pActor ) ;		// Add Actor
	void	SelectActor			( CActor *pActor ) ;		// Select Actor
	void	UpdateDetails		( CActor *pActor ) ;		// Update Details
	void	RebuildLayerDropList( void ) ;

// Construction
public:
	CDlgActorList(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgActorList)
	enum { IDD = IDD_ACTORLIST };
	CComboBox	m_Layer;
	CButton	m_Edit;
	CButton	m_DisplayPath;
	CEdit	m_CharacterName;
	CListCtrl	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgActorList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgActorList)
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	virtual BOOL OnInitDialog();
	afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDisplaypath();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEdit();
	afx_msg void OnSelchangeActorlistLayer();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	afx_msg void OnOk();
	afx_msg void OnCancel();
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGACTORLIST_H__570E9704_FFC1_11D1_9507_00207811EE70__INCLUDED_)

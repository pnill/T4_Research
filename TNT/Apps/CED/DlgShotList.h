#if !defined(AFX_DLGSHOTLIST_H__674CE080_06E3_11D2_AEC0_00A024569FF3__INCLUDED_)
#define AFX_DLGSHOTLIST_H__674CE080_06E3_11D2_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgShotList.h : header file
//

class CMovieView ;
class CMovie ;
class CShot ;

/////////////////////////////////////////////////////////////////////////////
// CDlgShotList dialog

class CDlgShotList : public CDialog
{
//---	Attributes
public:
	bool			m_Updating ;							// Updating Edits, do not react to change messages
	bool			m_BuildingShotList ;

	CMovieView		*m_pView ;								// Pointer to View
	CMovie			*m_pMovie ;								// Pointer to Movie

//---	Operations
public:
	void	SetupShotList		( CMovie *pMovie ) ;
	void	SetupActorLists		( CMovie *pMovie ) ;
	void	SelectShot			( CShot *pShot, bool Details=true ) ;
	void	UpdateDetails		( ) ;								// Update Details
	void	SetUseShotList		( bool State ) ;

//	void	SetActor			( CActor *pActor ) ;				// Set Actor to reference
//	void	SetupMotions		( CCharacter *pCharacter ) ;		// Setup Motions
//	void	SelectMotion		( CMotion *pMotion ) ;				// Select a Motion
//	void	SelectKey			( CKey *pKey ) ;					// Select Key

// Construction
public:
	CDlgShotList(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgShotList)
	enum { IDD = IDD_SHOTLIST };
	CButton	m_Interpolate;
	CEdit	m_Speed;
	CEdit	m_Frame;
	CComboBox	m_Target;
	CComboBox	m_Eye;
	CListCtrl	m_ShotList;
	CEdit		m_FOV;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgShotList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgShotList)
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnItemchangedShotlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeEye();
	afx_msg void OnSelchangeTarget();
	afx_msg void OnChangeFrame();
	virtual BOOL OnInitDialog();
	afx_msg void OnBeginlabeleditShotlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditShotlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeFov();
	afx_msg void OnChangeSpeed();
	afx_msg void OnInterp();
	afx_msg void OnClickShotlist(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	afx_msg void OnOk();
	afx_msg void OnCancel();
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSHOTLIST_H__674CE080_06E3_11D2_AEC0_00A024569FF3__INCLUDED_)

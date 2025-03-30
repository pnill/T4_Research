#if !defined(AFX_DLGACTORNEW_H__43282AC0_016B_11D2_9507_00207811EE70__INCLUDED_)
#define AFX_DLGACTORNEW_H__43282AC0_016B_11D2_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgActorNew.h : header file
//

class CCharacterList ;
class CCharacter ;
class CMeshList ;
class CMesh ;
class CSkin ;

/////////////////////////////////////////////////////////////////////////////
// CDlgActorNew dialog

class CDlgActorNew : public CDialog
{
// Attributes
public:
	CCharacterList	*m_pCharacterList ;			// Pointer to Character List
	CCharacter		*m_pCharacter ;				// Pointer to Character
	int				m_Skin ;					// the Character's Skin to use
	CMeshList		*m_pMeshList ;				// Pointer to Mesh List
	CMesh			*m_pMesh ;					// Pointer to Mesh

	bool			m_LockType;

// Operations
public:

protected:
	void			FillSkinList( void );

// Construction
public:
	CDlgActorNew(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgActorNew)
	enum { IDD = IDD_ACTOR_NEW };
	CComboBox	m_SkinSelect;
	CComboBox	m_Meshes;
	CComboBox	m_Characters;
	CString	m_Name;
	int		m_Type;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgActorNew)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgActorNew)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnSelchangeCharacters();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGACTORNEW_H__43282AC0_016B_11D2_9507_00207811EE70__INCLUDED_)

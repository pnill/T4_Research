#if !defined(AFX_DLGSELECTMESH_H__2EC0AC81_1847_11D3_A3E2_00104B5EB1F8__INCLUDED_)
#define AFX_DLGSELECTMESH_H__2EC0AC81_1847_11D3_A3E2_00104B5EB1F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSelectMesh.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSelectMesh dialog

class CMeshList ;
class CMesh ;

class CDlgSelectMesh : public CDialog
{
// Attributes
public:
	CMeshList		*m_pMeshList ;				// Pointer to Mesh List
	CMesh			*m_pMesh ;					// Pointer to Mesh

// Construction
public:
	CDlgSelectMesh(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSelectMesh)
	enum { IDD = IDD_SELECT_MESH };
	CComboBox	m_Meshes;
	CString	m_Name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSelectMesh)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSelectMesh)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSELECTMESH_H__2EC0AC81_1847_11D3_A3E2_00104B5EB1F8__INCLUDED_)

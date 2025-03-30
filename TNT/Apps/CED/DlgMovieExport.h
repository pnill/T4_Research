#if !defined(AFX_DLGMOVIEEXPORT_H__11034A73_4E5F_4C0E_906D_369AAA26A4E8__INCLUDED_)
#define AFX_DLGMOVIEEXPORT_H__11034A73_4E5F_4C0E_906D_369AAA26A4E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMovieExport.h : header file
//

#include "Movie.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgMovieExport dialog

class CDlgMovieExport : public CDialog
{
// Construction
public:
	CDlgMovieExport(CWnd* pParent = NULL);   // standard constructor

	CString		m_ExportPaths[NUM_EXPORT_TYPES];
	bool		m_ExportActive[NUM_EXPORT_TYPES];

// Dialog Data
	//{{AFX_DATA(CDlgMovieExport)
	enum { IDD = IDD_MOVIE_EXPORT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMovieExport)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void LoadFromMemberVars( void );
	void StoreToMemberVars( void );
	bool Browse( CString& Path );

	// Generated message map functions
	//{{AFX_MSG(CDlgMovieExport)
	afx_msg void OnExportEnableGamecube();
	afx_msg void OnExportEnablePc();
	afx_msg void OnExportEnableXbox();
	afx_msg void OnExportEnablePs2();
	afx_msg void OnExportBrowseGamecube();
	afx_msg void OnExportBrowsePc();
	afx_msg void OnExportBrowseXbox();
	afx_msg void OnExportBrowsePs2();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMOVIEEXPORT_H__11034A73_4E5F_4C0E_906D_369AAA26A4E8__INCLUDED_)

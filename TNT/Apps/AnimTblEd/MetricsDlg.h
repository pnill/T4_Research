#if !defined(AFX_METRICSDLG_H__984C2B00_25A0_11D3_9248_00105A29F8F3__INCLUDED_)
#define AFX_METRICSDLG_H__984C2B00_25A0_11D3_9248_00105A29F8F3__INCLUDED_

#include "AnimTblEdDoc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MetricsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMetricsDlg dialog

class CMetricsDlg : public CDialog
{
// Construction
public:
	CAnimTblEdDoc * pDoc;
	CMetricsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMetricsDlg)
	enum { IDD = IDD_METRICS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetricsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMetricsDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METRICSDLG_H__984C2B00_25A0_11D3_9248_00105A29F8F3__INCLUDED_)

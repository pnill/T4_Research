#if !defined(AFX_DEFINEDEVENTLISTCTRL_H__AA440F64_D029_11D3_88A5_00105A29F84C__INCLUDED_)
#define AFX_DEFINEDEVENTLISTCTRL_H__AA440F64_D029_11D3_88A5_00105A29F84C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DefinedEventListCtrl.h : header file
//

#include "SortedListCtrl.h"
#include "EventList.h"
#include "Skeleton.h"

#define ELC_EVENT_NAME		0
#define ELC_EVENT_RADIUS	1
#define ELC_EVENT_BONE		2

/////////////////////////////////////////////////////////////////////////////
// CDefinedEventListCtrl window

class CDefinedEventListCtrl : public CSortedListCtrl
{
// Construction
public:
	CDefinedEventListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefinedEventListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetDataPointer( CEventList* pEventList );
	void SetBonePointer(CSkeleton * pSkeleton );
	virtual ~CDefinedEventListCtrl();

	// Generated message map functions
protected:
	CEventList* m_pEventList;
	CSkeleton* m_pSkeleton;
	int CompareItems(LPARAM lParam1, LPARAM lParam2);
	//{{AFX_MSG(CDefinedEventListCtrl)
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFINEDEVENTLISTCTRL_H__AA440F64_D029_11D3_88A5_00105A29F84C__INCLUDED_)

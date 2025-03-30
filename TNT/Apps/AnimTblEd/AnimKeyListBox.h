#if !defined(AFX_ANIMKEYLISTBOX_H__D89CD546_386B_11D4_9398_0050DA2C723D__INCLUDED_)
#define AFX_ANIMKEYLISTBOX_H__D89CD546_386B_11D4_9398_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnimKeyListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimKeyListBox window

class CAnimKeyListBox : public CListBox
{
// Construction
public:
	CAnimKeyListBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimKeyListBox)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAnimKeyListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAnimKeyListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CAnimKeyListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMKEYLISTBOX_H__D89CD546_386B_11D4_9398_0050DA2C723D__INCLUDED_)

////////////////////////////////////////////////////////////////////////////
//
// SwapIndexDlg.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef SWAPINDEXDLG_H_INCLUDED
#define SWAPINDEXDLG_H_INCLUDED

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCK_STDAFX_H_INCLUDED
    #error "You must include StdAfx.h before this file."
#endif

#include "Resource.h"


/////////////////////////////////////////////////////////////////////////////
// CSwapIndexDlg dialog
////////////////////////////////////////////////////////////////////////////

class CSwapIndexDlg : public CDialog
{
public:
    CSwapIndexDlg( int Index, int MaxIndex, CWnd* pParent = NULL );

    int GetNewIndex( void ) { return m_Index; }

    //{{AFX_DATA(CSwapIndexDlg)
    enum { IDD = IDD_SETINDEX_DIALOG };
        // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSwapIndexDlg)
    //}}AFX_VIRTUAL

protected:

    int m_Index;
    int m_MaxIndex;

    // Generated message map functions
    //{{AFX_MSG(CSwapIndexDlg)
    virtual void OnCancel( void );
    virtual void OnOK( void );
    virtual BOOL OnInitDialog( void );
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // SWAPINDEXDLG_H_INCLUDED

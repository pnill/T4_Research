////////////////////////////////////////////////////////////////////////////
//
// SwapIndexDlg.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "SwapIndexDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CSwapIndexDlg dialog IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

CSwapIndexDlg::CSwapIndexDlg( int Index, int MaxIndex, CWnd* pParent )
    : CDialog( CSwapIndexDlg::IDD, pParent )
{
    //{{AFX_DATA_INIT(CSwapIndexDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_Index    = Index;
    m_MaxIndex = MaxIndex;
}

////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSwapIndexDlg, CDialog)
    //{{AFX_MSG_MAP(CSwapIndexDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSwapIndexDlg MESSAGE HANDLERS
////////////////////////////////////////////////////////////////////////////

BOOL CSwapIndexDlg::OnInitDialog( void )
{
    CDialog::OnInitDialog();
    
    CString EditText;

    CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_SELINDEX );

    // Create initial string in edit box
    EditText.Format( "%ld", this->m_Index );
    pEdit->SetLimitText( 5 );
    pEdit->SetWindowText( EditText );

    // Set focus to edit control and select all the text
    pEdit->SetFocus();
    pEdit->SetSel( 0, -1 );

    return FALSE; //TRUE;  // return TRUE unless you set the focus to a control
}

//==========================================================================

void CSwapIndexDlg::OnCancel( void ) 
{
    CDialog::OnCancel();
}

//==========================================================================

void CSwapIndexDlg::OnOK( void ) 
{
    CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_SELINDEX );

    char EditText[8];
    int  NewIndex;

    // Get text from edit control
    if( 0 < pEdit->GetWindowText( EditText, 7 ) )
    {
        // convert the text into an integer
        sscanf( EditText, "%ld", &NewIndex );

        // if the new value is the same, nothing to do
        if( NewIndex == m_Index )
        {
            OnCancel();
            return;
        }

        if( (NewIndex <= m_MaxIndex) && (NewIndex >= 0) )
            m_Index = NewIndex;
    }

    CDialog::OnOK();
}

//==========================================================================

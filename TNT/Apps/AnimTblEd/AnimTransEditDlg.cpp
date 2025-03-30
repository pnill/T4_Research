// AnimTransEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTransEditDlg.h"
#include "AnimIDSelectionDlg.h"
#include "AnimTblEdGlobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimTransEditDlg dialog


CAnimTransEditDlg::CAnimTransEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimTransEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimTransEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAnimTransEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimTransEditDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimTransEditDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimTransEditDlg)
	ON_BN_CLICKED(IDC_BUTTON_CHANGEFROM, OnButtonChangefrom)
	ON_BN_CLICKED(IDC_BUTTON_CHANGEPLAY, OnButtonChangeplay)
	ON_BN_CLICKED(IDC_BUTTON_CHANGETO, OnButtonChangeto)
	ON_CBN_SELCHANGE(IDC_COMBO_PREREQTRANS, OnSelchangeComboPrereqtrans)
	ON_EN_UPDATE(IDC_EDIT_BLENDFRAMES, OnUpdateEditBlendframes)
	ON_BN_CLICKED(IDC_CHECK_DISABLE, OnCheckDisable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimTransEditDlg message handlers

BOOL CAnimTransEditDlg::OnInitDialog() 
{
	CComboBox	*pComboBox;
	CString		Num;

	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	pComboBox = (CComboBox*)this->GetDlgItem( IDC_COMBO_PREREQTRANS );
	pComboBox->SetCurSel( m_pAnimTrans->IsTransition() );

	FillLocalVars();
	
	DrawDialog();

	//---	Blend frames spin control read-out.
	this->SetDlgItemInt( IDC_EDIT_BLENDFRAMES, m_pAnimTrans->GetBlendFrames() );

	((CSpinButtonCtrl*)this->GetDlgItem( IDC_SPIN_BLENDFRAMES ))->SetRange( 0, 15 );

	m_OldDisable = m_pAnimTrans->GetDisabled();
	((CButton*)this->GetDlgItem( IDC_CHECK_DISABLE ))->SetCheck( m_OldDisable );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnimTransEditDlg::OnButtonChangefrom() 
{
	CAnimIDSelectionDlg		AnimIDSelectionDlg;

	AnimIDSelectionDlg.m_pDoc = this->m_pDoc;
	AnimIDSelectionDlg.AddExtraChoice( "<any>" );
	AnimIDSelectionDlg.SetOmitChoice( m_AnimName );

	if (AnimIDSelectionDlg.DoModal() == IDOK)
	{
		m_FromAnim.Format( "%s", AnimIDSelectionDlg.m_SelectedAnim );
		this->DrawDialog();
	}
}

void CAnimTransEditDlg::OnButtonChangeplay() 
{
	CAnimIDSelectionDlg		AnimIDSelectionDlg;

	AnimIDSelectionDlg.m_pDoc = this->m_pDoc;
	AnimIDSelectionDlg.AddExtraChoice( "<any>" );
	AnimIDSelectionDlg.SetOmitChoice( m_AnimName );

	if (AnimIDSelectionDlg.DoModal() == IDOK)
	{
		m_PlayAnim.Format( "%s", AnimIDSelectionDlg.m_SelectedAnim );
		this->DrawDialog();
	}
}

void CAnimTransEditDlg::OnButtonChangeto() 
{
	CAnimIDSelectionDlg		AnimIDSelectionDlg;

	AnimIDSelectionDlg.m_pDoc = this->m_pDoc;
	AnimIDSelectionDlg.AddExtraChoice( "<any>" );
	AnimIDSelectionDlg.SetOmitChoice( m_AnimName );

	if (AnimIDSelectionDlg.DoModal() == IDOK)
	{
		m_ToAnim.Format( "%s", AnimIDSelectionDlg.m_SelectedAnim );
		this->DrawDialog();
	}
}

void CAnimTransEditDlg::DrawDialog()
{
	CComboBox	*pComboBox;
	CString		Description;
	int			Transition;

	pComboBox = (CComboBox*)this->GetDlgItem( IDC_COMBO_PREREQTRANS );
	Transition = pComboBox->GetCurSel( );
	
	if (Transition)
	{
		((CWnd*)this->GetDlgItem( IDC_BUTTON_CHANGEFROM ))->EnableWindow( FALSE );
		this->SetDlgItemText( IDC_EDIT_FROMANIM, m_FromAnim );

		((CWnd*)this->GetDlgItem( IDC_BUTTON_CHANGETO ))->EnableWindow( TRUE );
		this->SetDlgItemText( IDC_EDIT_TOANIM, m_ToAnim );

		((CWnd*)this->GetDlgItem( IDC_EDIT_PLAYANIM ))->ShowWindow( TRUE );
		((CWnd*)this->GetDlgItem( IDC_BUTTON_CHANGEPLAY ))->ShowWindow( TRUE );
		((CWnd*)this->GetDlgItem( IDC_STATIC_PLAY ))->ShowWindow( TRUE );
		this->SetDlgItemText( IDC_EDIT_PLAYANIM, m_PlayAnim );
	}
	else
	{
		((CWnd*)this->GetDlgItem( IDC_BUTTON_CHANGEFROM ))->EnableWindow( TRUE );
		this->SetDlgItemText( IDC_EDIT_FROMANIM, m_FromAnim );

		((CWnd*)this->GetDlgItem( IDC_BUTTON_CHANGETO ))->EnableWindow( FALSE );
		this->SetDlgItemText( IDC_EDIT_TOANIM, m_ToAnim );

		((CWnd*)this->GetDlgItem( IDC_EDIT_PLAYANIM ))->ShowWindow( FALSE );
		((CWnd*)this->GetDlgItem( IDC_BUTTON_CHANGEPLAY ))->ShowWindow( FALSE );
		((CWnd*)this->GetDlgItem( IDC_STATIC_PLAY ))->ShowWindow( FALSE );
		//		this->SetDlgItemText( IDC_EDIT_PLAYANIM, "" );
	}

	//---	Description.
	m_pAnimTrans->GetNLDescription( Description, m_FromAnim, m_ToAnim, m_PlayAnim );
	this->SetDlgItemText( IDC_STATIC_DESCRIPTION, Description );
}

void CAnimTransEditDlg::OnSelchangeComboPrereqtrans() 
{
	//---	Start with the source variables that have not been changed.
	FillLocalVars();

	//---	Draw the new box.
	this->DrawDialog();	
}

void CAnimTransEditDlg::FillLocalVars()
{
	int			Transition;

	Transition = ((CComboBox*)this->GetDlgItem( IDC_COMBO_PREREQTRANS ))->GetCurSel( );

	m_FromAnim.Format( "%s", m_pAnimTrans->GetFromAnim() );
	m_ToAnim.Format( "%s", m_pAnimTrans->GetToAnim() );
	m_PlayAnim.Format( "%s", m_pAnimTrans->GetPlayAnim() );

	if (Transition)
	{
		m_FromAnim.Format( "<this>" );
		if (m_ToAnim.Compare( "<this>" ) == 0)
			m_ToAnim.Format( "<any>" );
	}
	else
	{
		m_ToAnim.Format( "<this>" );
		if (m_FromAnim.Compare( "<this>" ) == 0)
			m_FromAnim.Format( "<any>" );
	}

	m_OldBlendFrames = m_pAnimTrans->GetBlendFrames();
}

void CAnimTransEditDlg::OnOK() 
{
	CString		ErrorMsg;
	int			Transition;

	Transition = ((CComboBox*)this->GetDlgItem( IDC_COMBO_PREREQTRANS ))->GetCurSel( );

	if (Transition)
	{
		if (strlen( m_ToAnim ) == 0)
			ErrorMsg.Format( "Must select a 'to' animation." );

		if (strlen( m_PlayAnim ) == 0)
			ErrorMsg.Format( "Must select a 'play' animation." );

//		if (m_ToAnim.Compare( m_PlayAnim ) == 0)
//			ErrorMsg.Format( "'To' animation and 'Play' animation must be different." );

		//---	If the dependancy works to any animation then it must at least set blending.
		if ((m_PlayAnim.Compare( "<any>" ) == 0) && (m_pAnimTrans->GetBlendFrames() == 0))
			ErrorMsg.Format( "Must set a number of frames to blend greater than 0." );
	}
	else
	{
		//---	Must have a prerequisite animation selected.
		if (strlen( m_FromAnim ) == 0)
			ErrorMsg.Format( "Must select a 'from' animation." );

		//---	If the dependancy works from any animation then it must at least set blending.
		if ((m_FromAnim.Compare( "<any>" ) == 0) && (m_pAnimTrans->GetBlendFrames() == 0))
			ErrorMsg.Format( "Must set a number of frames to blend greater than 0." );
	}

		//---	If it gets this far then it is clear for writing.
	if (ErrorMsg.IsEmpty())
	{
		m_pAnimTrans->SetFromAnim( m_FromAnim );
		m_pAnimTrans->SetToAnim( m_ToAnim );
		m_pAnimTrans->SetPlayAnim( m_PlayAnim );

		//---	Mark the document as having changed.
		GBL_SetModifiedFlag( TRUE );

		CDialog::OnOK();
	}
	else
		this->MessageBox( ErrorMsg, "Error!" );
}

void CAnimTransEditDlg::OnUpdateEditBlendframes() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
#if 0
	CString		Description;
	CString		Num;
	
	//---	Blend frames spin control read-out.
	Num.Format( "%d", m_pAnimTrans->GetBlendFrames() );
	this->SetDlgItemText( IDC_EDIT_BLENDFRAMES, Num );

	//---	Description.
	m_pAnimTrans->GetNLDescription( Description, m_FromAnim, m_ToAnim, m_PlayAnim );
	this->SetDlgItemText( IDC_STATIC_DESCRIPTION, Description );
#endif
	CString		Description;

	m_pAnimTrans->SetBlendFrames( this->GetDlgItemInt( IDC_EDIT_BLENDFRAMES ) );
	
	//---	Description.
	m_pAnimTrans->GetNLDescription( Description, m_FromAnim, m_ToAnim, m_PlayAnim );
	this->SetDlgItemText( IDC_STATIC_DESCRIPTION, Description );
}


void CAnimTransEditDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	m_pAnimTrans->SetBlendFrames( m_OldBlendFrames );
	m_pAnimTrans->SetDisabled( m_OldDisable );
	
	CDialog::OnCancel();
}

void CAnimTransEditDlg::OnCheckDisable() 
{
	CString Description;

	// TODO: Add your control notification handler code here
	m_pAnimTrans->SetDisabled( ((CButton*)this->GetDlgItem( IDC_CHECK_DISABLE ))->GetCheck() );

	//---	Description.
	m_pAnimTrans->GetNLDescription( Description, m_FromAnim, m_ToAnim, m_PlayAnim );
	this->SetDlgItemText( IDC_STATIC_DESCRIPTION, Description );
}

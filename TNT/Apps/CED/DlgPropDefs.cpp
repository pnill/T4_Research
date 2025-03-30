// DlgPropDefs.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgPropDefs.h"
#include "MotionEditView.h"
#include "Character.h"
#include "Prop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPropDefs dialog


CDlgPropDefs::CDlgPropDefs(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPropDefs::IDD, pParent)
{
	m_pView				= NULL;
	m_bInUpdateDetails	= FALSE;

	//{{AFX_DATA_INIT(CDlgPropDefs)
	//}}AFX_DATA_INIT
}


void CDlgPropDefs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPropDefs)
	DDX_Control(pDX, IDC_MIRRORWITHANIM, m_MirrorWithAnim);
	DDX_Control(pDX, IDC_MIRRORED, m_Mirrored);
	DDX_Control(pDX, IDC_FACINGRELATIVEROT, m_FacingRelativeRot);
	DDX_Control(pDX, IDC_USELINKEDROT, m_UseLinkedRot);
	DDX_Control(pDX, IDC_USELINKEDPOS, m_UseLinkedPos);
	DDX_Control(pDX, IDC_SCALED, m_Scaled);
	DDX_Control(pDX, IDC_FACINGRELATIVE, m_FacingRelativePos);
	DDX_Control(pDX, IDC_STATICROT, m_StaticRot);
	DDX_Control(pDX, IDC_STATICPOS, m_StaticPos);
	DDX_Control(pDX, IDC_ROTZ, m_RotZ);
	DDX_Control(pDX, IDC_ROTY, m_RotY);
	DDX_Control(pDX, IDC_ROTX, m_RotX);
	DDX_Control(pDX, IDC_PROPNAME, m_Name);
	DDX_Control(pDX, IDC_PROPDEFLIST, m_PropDefList);
	DDX_Control(pDX, IDC_POSZ, m_PosZ);
	DDX_Control(pDX, IDC_POSY, m_PosY);
	DDX_Control(pDX, IDC_POSX, m_PosX);
	DDX_Control(pDX, IDC_LINKCHARACTERCOMBO, m_LinkCharacterCombo);
	DDX_Control(pDX, IDC_BONECOMBO, m_BoneCombo);
	DDX_Control(pDX, IDC_EXPORTED, m_Exported);
	DDX_Control(pDX, IDC_ACTIVE, m_Active);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgPropDefs, CDialog)
	//{{AFX_MSG_MAP(CDlgPropDefs)
	ON_LBN_SELCHANGE(IDC_PROPDEFLIST, OnSelchangePropdeflist)
	ON_BN_CLICKED(IDC_ACTIVE, OnActive)
	ON_BN_CLICKED(IDC_EXPORTED, OnExported)
	ON_BN_CLICKED(IDC_STATICPOS, OnStaticpos)
	ON_BN_CLICKED(IDC_STATICROT, OnStaticrot)
	ON_EN_CHANGE(IDC_PROPNAME, OnChangePropname)
	ON_EN_CHANGE(IDC_POSX, OnChangePosx)
	ON_EN_CHANGE(IDC_POSY, OnChangePosy)
	ON_EN_CHANGE(IDC_POSZ, OnChangePosz)
	ON_EN_CHANGE(IDC_ROTX, OnChangeRotx)
	ON_EN_CHANGE(IDC_ROTY, OnChangeRoty)
	ON_EN_CHANGE(IDC_ROTZ, OnChangeRotz)
	ON_CBN_SELCHANGE(IDC_LINKCHARACTERCOMBO, OnSelchangeLinkcharactercombo)
	ON_CBN_SELCHANGE(IDC_BONECOMBO, OnSelchangeBonecombo)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_FACINGRELATIVE, OnFacingrelative)
	ON_BN_CLICKED(IDC_SCALED, OnScaled)
	ON_BN_CLICKED(IDC_USELINKEDPOS, OnUselinkedpos)
	ON_BN_CLICKED(IDC_USELINKEDROT, OnUselinkedrot)
	ON_BN_CLICKED(IDC_FACINGRELATIVEROT, OnFacingrelativerot)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	ON_BN_CLICKED(IDC_MIRRORED, OnMirrored)
	ON_BN_CLICKED(IDC_MIRRORWITHANIM, OnMirrorwithanim)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPropDefs message handlers


//===========================================================================
void CDlgPropDefs::UpdateDetails( void )
{
	//---	if there is no dialog, exit now
	if( !GetSafeHwnd() )
		return;

	//---	if there is no view, or character, disable the dialog and return
	if( !m_pView || !m_pView->m_pCharacter )
	{
		EnableDialog( FALSE );
		return;
	}

	EnableDialog( TRUE );

	//---	flag that we are about to update the dialog globally.  This will prevent
	//		indevidual dialog callbacks from trying to reset any part of the dialog.
	m_bInUpdateDetails = TRUE;

	//---	fill the prop definition list
	FillPropDefList();

	//---	Set the dialog data
	s32 CurSel = m_PropDefList.GetCurSel();
	if( CurSel != -1 )
	{
		CPropDef& rPropDef = m_pView->m_pCharacter->GetPropDef( CurSel );

		FillBoneCombo( rPropDef.GetBone()+1 );
		FillLinkCharacterCombo( rPropDef.GetLinkedCharacter() );

		m_Name.SetWindowText( rPropDef.GetPropName() );
		m_Active.SetCheck( rPropDef.IsActive() );
		m_Exported.SetCheck( rPropDef.IsExported() );

		CString Text;
		Text.Format( "%9.4f", rPropDef.GetPosition().X );
		m_PosX.SetWindowText( Text );
		Text.Format( "%9.4f", rPropDef.GetPosition().Y );
		m_PosY.SetWindowText( Text );
		Text.Format( "%9.4f", rPropDef.GetPosition().Z );
		m_PosZ.SetWindowText( Text );
		Text.Format( "%9.4f", rPropDef.GetRotation().X );
		m_RotX.SetWindowText( Text );
		Text.Format( "%9.4f", rPropDef.GetRotation().Y );
		m_RotY.SetWindowText( Text );
		Text.Format( "%9.4f", rPropDef.GetRotation().Z );
		m_RotZ.SetWindowText( Text );

		m_StaticPos.SetCheck( rPropDef.IsPositionStatic() );
		m_StaticRot.SetCheck( rPropDef.IsRotationStatic() );
		m_FacingRelativePos.SetCheck( rPropDef.IsPosFacingRelative() );
		m_FacingRelativeRot.SetCheck( rPropDef.IsRotFacingRelative() );
		m_Scaled.SetCheck( rPropDef.IsScaled() );
		m_UseLinkedPos.SetCheck( rPropDef.IsUseLinkedPosition()?TRUE:FALSE );
		m_UseLinkedRot.SetCheck( rPropDef.IsUseLinkedRotation()?TRUE:FALSE );
		m_MirrorWithAnim.SetCheck( rPropDef.IsMirrorWithAnim()?TRUE:FALSE );
		m_Mirrored.SetCheck( rPropDef.IsMirrored()?TRUE:FALSE );

		EnablePropDef( rPropDef.IsActive() );
	}
	else
	{
		FillBoneCombo( 0 );
		FillLinkCharacterCombo( NULL );

		m_Name.SetWindowText( "" );
		m_Exported.SetCheck( FALSE );
		m_PosX.SetWindowText( "0.0" );
		m_PosY.SetWindowText( "0.0" );
		m_PosZ.SetWindowText( "0.0" );
		m_RotX.SetWindowText( "0.0" );
		m_RotY.SetWindowText( "0.0" );
		m_RotZ.SetWindowText( "0.0" );

		m_StaticPos.SetCheck( FALSE );
		m_StaticRot.SetCheck( FALSE );

		m_Mirrored.SetCheck( FALSE );

		EnablePropDef( FALSE );
	}

	//---	Clear the flag so that the indevidual controls will be able to affect any
	//		required changes.
	m_bInUpdateDetails = FALSE;
}

//===========================================================================
void CDlgPropDefs::FillPropDefList( void )
{
	CString Text;
	s32		CurSel = m_PropDefList.GetCurSel();
	s32		i;

	m_PropDefList.ResetContent();

	for( i=0; i<MAX_NUM_PROPS; i++ )
	{
		CPropDef& rPropDef = m_pView->m_pCharacter->GetPropDef(i);

		if( rPropDef.IsActive() && rPropDef.IsExported() )
			Text.Format( "%1d) %s", i, rPropDef.GetPropName() );
		else if( rPropDef.IsActive() )
			Text.Format( "%1d) (%s)", i, rPropDef.GetPropName() );
		else
			Text.Format( "%1d) <inactive>", i );

		m_PropDefList.AddString( Text );
	}

	if( CurSel != -1 )
		m_PropDefList.SetCurSel( CurSel );
}

//===========================================================================
void CDlgPropDefs::FillLinkCharacterCombo( const CCharacter* pSelect )
{
	s32	Index;
	s32	Select = 0;

	m_LinkCharacterCombo.ResetContent();
	Index = m_LinkCharacterCombo.AddString( "<no linked character>" );
	m_LinkCharacterCombo.SetItemData( Index, NULL );

	POSITION pos = m_pView->GetDocument()->m_CharacterList.GetHeadPosition();
	CCharacter* pCharacter;
	while( pos )
	{
		pCharacter	= m_pView->GetDocument()->m_CharacterList.GetNext( pos );
		Index		= m_LinkCharacterCombo.AddString( pCharacter->GetName() );
		m_LinkCharacterCombo.SetItemData( Index, (u32)pCharacter );
	}

	//---	look for the character to select
	for( Index=0; Index<m_LinkCharacterCombo.GetCount(); Index++ )
		if( m_LinkCharacterCombo.GetItemData( Index ) == (u32)pSelect )
			break;

	if( Index<m_LinkCharacterCombo.GetCount() )
		m_LinkCharacterCombo.SetCurSel( Index );
}

//===========================================================================
void CDlgPropDefs::FillBoneCombo( s32 Select )
{
	CString		Text;
	CSkeleton*	pSkeleton;
	SSkelBone*	pBone;
	s32			i, j;

	pSkeleton	= m_pView->m_pCharacter->GetSkeleton();

	m_BoneCombo.ResetContent();

	s32 Index = m_BoneCombo.AddString( "<no bone>" );
	m_BoneCombo.SetItemData( Index, -1 );

	for( i=0; i<pSkeleton->GetNumBones(); i++ )
	{
		pBone = pSkeleton->GetBoneFromIndex( i );
		ASSERT(pBone);
		Text = "";
		j = pSkeleton->GetBoneNumParents( pBone );
		while( j-- )
			Text += " ";
		Text += CString( pBone->BoneName );
		Index = m_BoneCombo.AddString( Text );
		m_BoneCombo.SetItemData( i+1, i );
	}

	m_BoneCombo.SetCurSel( Select );
}

//===========================================================================
void CDlgPropDefs::EnablePropDef( bool bEnable )
{
	//---	enable all of the static text
	GetDlgItem( IDC_NAMESTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_LINKCHARACTERSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_BONESTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_POSITIONSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_ROTATIONSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_POSXSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_POSYSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_POSZSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_ROTXSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_ROTYSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_ROTZSTATIC )->EnableWindow( bEnable );
	GetDlgItem( IDC_LOADTITLE )->EnableWindow( bEnable );
	GetDlgItem( IDC_LOAD )->EnableWindow( bEnable );

	//---	enable all of the controls
	m_Name.EnableWindow( bEnable );
	m_LinkCharacterCombo.EnableWindow( bEnable );
	m_BoneCombo.EnableWindow( bEnable );
	m_PosX.EnableWindow( bEnable );
	m_PosY.EnableWindow( bEnable );
	m_PosZ.EnableWindow( bEnable );
	m_RotX.EnableWindow( bEnable );
	m_RotY.EnableWindow( bEnable );
	m_RotZ.EnableWindow( bEnable );
	m_StaticPos.EnableWindow( bEnable );
	m_StaticRot.EnableWindow( bEnable );
	m_Exported.EnableWindow( bEnable );
	m_FacingRelativePos.EnableWindow( bEnable && m_StaticPos.GetCheck() );
	m_FacingRelativeRot.EnableWindow( bEnable && m_StaticRot.GetCheck() );
	m_Scaled.EnableWindow( bEnable );
	m_Mirrored.EnableWindow( bEnable );
	m_UseLinkedPos.EnableWindow( bEnable );
	m_UseLinkedRot.EnableWindow( bEnable );
	m_MirrorWithAnim.EnableWindow( bEnable );
}

//===========================================================================
void CDlgPropDefs::EnableDialog( bool bEnable )
{
	//---	enable all static text
	GetDlgItem( IDC_PROPLISTSTATIC )->EnableWindow( bEnable );

	m_PropDefList.EnableWindow( bEnable );
	m_Active.EnableWindow( bEnable );

	EnablePropDef( m_Active.GetCheck() && bEnable );
}

//===========================================================================
void CDlgPropDefs::SetSelectedPropDef( s32 Prop )
{
	if(( Prop < 0 ) || ( Prop >= m_PropDefList.GetCount() ))
		return;

	m_PropDefList.SetCurSel( Prop );
}

//===========================================================================
void CDlgPropDefs::OnSelchangePropdeflist() 
{
	UpdateDetails();	
}

//===========================================================================
CPropDef* CDlgPropDefs::UpdateControlReady( void )
{
	if( m_bInUpdateDetails )
		return NULL;

	s32 CurSel = m_PropDefList.GetCurSel();
	if( CurSel == -1 )
		return NULL;

	return &m_pView->m_pCharacter->GetPropDef( CurSel );
}

//===========================================================================
void CDlgPropDefs::UpdateMotionView( void )
{
	if( m_pView )
		m_pView->GetDocument()->UpdateAllViews( NULL );
}

//===========================================================================
void CDlgPropDefs::OnActive() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetActive( m_Active.GetCheck()?TRUE:FALSE );
	UpdateDetails();

	//---	update the view to show the change
	UpdateMotionView();

	//---	update the prop list for the currently selected motion
	m_pView->UpdatePropDetails();
}

//===========================================================================
void CDlgPropDefs::OnExported() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetExported( m_Exported.GetCheck()?TRUE:FALSE );
	UpdateDetails();
}

//===========================================================================
void CDlgPropDefs::OnStaticpos() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetPositionStatic( m_StaticPos.GetCheck()?TRUE:FALSE );

	//---	enable the facing relative check box based on the state of the static check boxes
	m_FacingRelativePos.EnableWindow( m_StaticPos.GetCheck() );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnStaticrot() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetRotationStatic( m_StaticRot.GetCheck()?TRUE:FALSE );

	//---	enable the facing relative check box based on the state of the static check boxes
	m_FacingRelativeRot.EnableWindow( m_StaticRot.GetCheck() );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnFacingrelative() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetFacingRelativePos( m_FacingRelativePos.GetCheck()?TRUE:FALSE );
}

//============================================================================
void CDlgPropDefs::OnFacingrelativerot() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetFacingRelativeRot( m_FacingRelativeRot.GetCheck()?TRUE:FALSE );
}

//===========================================================================
void CDlgPropDefs::OnScaled() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetScaled( m_Scaled.GetCheck()?TRUE:FALSE );
}

//===========================================================================
void CDlgPropDefs::OnMirrored() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetMirrored( m_Mirrored.GetCheck()?TRUE:FALSE );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnUselinkedpos() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetUseLinkedPosition( m_UseLinkedPos.GetCheck()?TRUE:FALSE );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnUselinkedrot() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetUseLinkedRotation( m_UseLinkedRot.GetCheck()?TRUE:FALSE );

	//---	update the view to show the change
	UpdateMotionView();
}


//===========================================================================
void CDlgPropDefs::OnMirrorwithanim() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetMirrorWithAnim( m_MirrorWithAnim.GetCheck()?TRUE:FALSE );

	//---	update the view to show the change
//	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnChangePropname() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	CString Name;
	m_Name.GetWindowText( Name );
	pPropDef->SetPropName( Name );
	FillPropDefList();

	//---	update the prop list for the currently selected motion
	m_pView->UpdatePropDetails();
}

//===========================================================================
void CDlgPropDefs::OnSelchangeLinkcharactercombo() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	CCharacter* pNewLinkedCharacter = (CCharacter*)m_LinkCharacterCombo.GetItemData( m_LinkCharacterCombo.GetCurSel() );

	if( pPropDef->GetLinkedCharacter() == pNewLinkedCharacter )
	{
		UpdateDetails();
		return;
	}

	if( pPropDef->GetLinkedCharacter() )
	{
		//---	warn the user that a change to the linked character will modify all of the linked motions and as
		//		him if he wants to continue.
		s32 result = MessageBox( "A change to the linked character will cause all motion props linked to motions inside this character"
								 " to search for motions with equivilent names.  Unless this is desired and planned for, all motion links"
								 " will most likely be broken.  Do you wish to continue?", "Are you sure?", MB_ICONWARNING|MB_YESNO );
		if( result != IDYES )
		{
			UpdateDetails();
			return;
		}
	}

	//---	set the new linked character
	pPropDef->SetLinkedCharacter( pNewLinkedCharacter );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnSelchangeBonecombo() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	pPropDef->SetBone( m_BoneCombo.GetCurSel()-1 );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnChangePosx() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;

	vector3 Pos = pPropDef->GetPosition();
	CString Text;
	m_PosX.GetWindowText( Text );
	Pos.X = (f32)x_atof( Text.GetBuffer( 0 ) );
	Text.ReleaseBuffer();
	pPropDef->SetPosition( Pos );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnChangePosy() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;
	
	vector3 Pos = pPropDef->GetPosition();
	CString Text;
	m_PosY.GetWindowText( Text );
	Pos.Y = (f32)x_atof( Text.GetBuffer( 0 ) );
	Text.ReleaseBuffer();
	pPropDef->SetPosition( Pos );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnChangePosz() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;
	
	vector3 Pos = pPropDef->GetPosition();
	CString Text;
	m_PosZ.GetWindowText( Text );
	Pos.Z = (f32)x_atof( Text.GetBuffer( 0 ) );
	Text.ReleaseBuffer();
	pPropDef->SetPosition( Pos );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnChangeRotx() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;
	
	radian3 Rot = pPropDef->GetRotation();
	CString Text;
	m_RotX.GetWindowText( Text );
	Rot.X = (f32)x_atof( Text.GetBuffer( 0 ) );
	Text.ReleaseBuffer();
	pPropDef->SetRotation( Rot );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnChangeRoty() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;
	
	radian3 Rot = pPropDef->GetRotation();
	CString Text;
	m_RotY.GetWindowText( Text );
	Rot.Y = (f32)x_atof( Text.GetBuffer( 0 ) );
	Text.ReleaseBuffer();
	pPropDef->SetRotation( Rot );

	//---	update the view to show the change
	UpdateMotionView();
}

//===========================================================================
void CDlgPropDefs::OnChangeRotz() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;
	
	radian3 Rot = pPropDef->GetRotation();
	CString Text;
	m_RotZ.GetWindowText( Text );
	Rot.Z = (f32)x_atof( Text.GetBuffer( 0 ) );
	Text.ReleaseBuffer();
	pPropDef->SetRotation( Rot );

	//---	update the view to show the change
	UpdateMotionView();
}

//============================================================================
void CDlgPropDefs::OnOK() 
{
	if( !m_pView )
		return;

	//---	remove the prop defs dialog.
	m_pView->DisplayPropDefs( FALSE );
}

//============================================================================
void CDlgPropDefs::OnLoad() 
{
	CPropDef* pPropDef = UpdateControlReady();
	if( !pPropDef )
		return;
	
	CPropData& rProp = m_pView->m_pCharacter->m_pCurMotion->GetProp( pPropDef->GetIndex() );

	vector3 Pos = rProp.GetPosition( m_pView->m_pCharacter->GetCurFrame() );
	radian3 Rot = rProp.GetRotation( m_pView->m_pCharacter->GetCurFrame() );

	pPropDef->SetPosition( Pos );
	pPropDef->SetRotation( Rot );

	UpdateDetails();
}

//============================================================================
void CDlgPropDefs::OnHelpbutton() 
{
	CString HelpText =
					"The definition of each prop describes how it is to be related to the character."
					" A prop can be placed at any position/rotation relative to any bone of a character or"
					" not to any bone at all.\n\n"
					"The first important flags for each prop is the ACTIVE and EXPORTED flags.  If a prop is"
					" active, it will be displayed in the CED and may be set to be exported.  If a prop is exported"
					" it's data will be accessable at run-time in the game.  These flags are redundant for each motion"
					" as well. This lets indevidual motions define how a prop relates to itself.  However, if the prop"
					" definition's flag is off for active or exported, all motion flags are ignored and the prop is not"
					" active and/or exported.\n\n"
					"Setting the MIRRORED flag will let the application know to mirror the attached prop mesh.  It has"
					" no affect on the exported data for the prop.  If the application does nothing with the flag, it will"
					" have no effect.\n\n"
					"The prop definition has several flag pairs which apply a similar effect to either the position or"
					" the rotation.  Each of these flags is described below:\n\n"
					"STATIC POS/ROT:\n"
					"    In this case Static means to use the position/rotation value as is without transformation by"
					" the bone's matrix.  In other words, static means that the position/rotation is a final world position"
					"/rotation.  A non-static position is transformed by the bone's matrix, whereas a static position"
					" is added to the final world position of the bone directly.  Same goes for rotation\n\n"
					"FACING RELATIVE:\n"
					"    If a prop is transformed by a bone's matrix, it will automatically be facing with the actor."
					" However, if it is relative to the world coordinate of the player, it will always allign the same"
					" position/rotation off of the actor, unless the Facing Relative flag is set.  This means to rotate"
					" the static position/rotation so that it is relative to the actor's facing.  This rotation is kept"
					" on the 2D XZ plane.\n\n"
					"USE LINKED POS/ROT:\n"
					"    Motion's can be linked to prop's on a motion by motion basis.  If these flags are checked, the"
					" position/rotation value from the linked motion's root node is used as exact final position and"
					" rotation of the prop relative to the bone.  This requires that the linked motion's data must be"
					" modified to be relative to the bone that that prop is relative to. If these flags are not checked"
					" the data from the linked motion is assumed to be already relative to the bone and will be used as is."
					" The most obvious difference between checking this flag and not is that when the flag is checked,"
					" altering the relative bone will not change the position of the prop about the actor.  the data will"
					" have been modified to keep the final position/rotation of the prop the same as the one given by the motion."
					" When this flag is not checked, however, changing the relative bone will cause the prop to jump its location"
					" drastically because the data from the linked motion is applied directly to a new bone."
					"MIRROR WITH ANIM POS/ROT:\n"
					"   These flags specify that the prop data should be mirrored with the motion if it is mirrored during"
					" playback. This doesn't change the exported data, mearly how the prop is drawn.  For example, if a prop"
					" is used to draw a hand on the player's left side, the game wouldn't want to mirror the hand yaw rotation"
					" relative to the arm bone just because the motion is rotated.  This would align it incorrectly off of the"
					" arm.\n";
	MessageBox( HelpText, "Props Definition Help", MB_ICONINFORMATION );
}

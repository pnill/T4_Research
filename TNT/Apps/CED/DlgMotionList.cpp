// DlgMotionList.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgMotionList.h"
#include "Character.h"
#include "MFCutils.h"
#include "MotionEditView.h"
#include "CedDoc.h"
#include "MotionFlags.h"
#include "TransitionViewer.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMotionList dialog


CDlgMotionList::CDlgMotionList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMotionList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMotionList)
	//}}AFX_DATA_INIT

	m_pView = NULL ;
	m_Updating = 0 ;
}

void CDlgMotionList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMotionList)
	DDX_Control(pDX, IDC_EXPORTDEFAULT, m_ExportDefault);
	DDX_Control(pDX, IDC_QUATERNION_ON, m_Quaternion);
	DDX_Control(pDX, IDC_EXPORTQUATW_ON, m_ExportQuatW);
	DDX_Control(pDX, IDC_MOTION_EXPORTFPS_BUTTON, m_ExportMotionFPSButton);
	DDX_Control(pDX, IDC_MOTION_EXPORTFPS, m_ExportMotionFPS);
	DDX_Control(pDX, IDC_MOTION_EXPORTFPS_SPIN, m_ExportMotionFPSSpin);
	DDX_Control(pDX, IDC_MOTION_EXPORTBITS_SPIN, m_ExportMotionBitsSpin);
	DDX_Control(pDX, IDC_MOTION_EXPORTBITS_BUTTON, m_ExportMotionBitsButton);
	DDX_Control(pDX, IDC_MOTION_EXPORTBITS, m_ExportMotionBits);
	DDX_Control(pDX, IDC_NOEXPORT, m_NoExport);
	DDX_Control(pDX, IDC_CHECK_AUTOCLEAR_MOVE_DIR_MOTIONLIST, m_ButtonAutoResetMove);
	DDX_Control(pDX, IDC_CHECK_AUTOCLEAR_FACE_DIR_MOTIONLIST, m_ButtonAutoResetFace);
	DDX_Control(pDX, IDC_MOVEDIR, m_MoveDir);
	DDX_Control(pDX, IDC_FACEDIR, m_FaceDir);
	DDX_Control(pDX, IDC_MOTION_LIST, m_List);
	//}}AFX_DATA_MAP
}



void CDlgMotionList::UpdateMotionList( CCharacter *pCharacter )
{
	int Index = m_List.GetCurSel();

	//---	Reset the List
	m_List.SetRedraw( FALSE ) ;
	m_List.ResetContent() ;
	if( pCharacter )
		m_List.InitStorage( pCharacter->GetNumMotions()+1, (pCharacter->GetNumMotions()+1)*32 ) ;
	m_List.AddString( "<no motion>" ) ;

	m_List.EnableWindow( pCharacter != NULL ) ;

	//---	Add All Motions
	if( pCharacter )
	{
		CString Name;
		for( int i = 0 ; i < pCharacter->GetNumMotions() ; i++ )
		{
			CMotion *pMotion = pCharacter->GetMotion(i) ;
			char FileName[_MAX_FNAME] ;
			char Ext[_MAX_EXT] ;
			_splitpath( pMotion->GetPathName(), NULL, NULL, FileName, Ext ) ;
			if( !pMotion->m_bExport )			Name = "zzz-(";
			else if( pMotion->m_bExport == 2)	Name = "zzy-(";
			else								Name = "";

#if 1
			Name += pMotion->GetExportName();
#else
			Name += FileName ;
			Name += Ext ;
#endif
			if( !pMotion->m_bExport )			Name += ")";
			else if( pMotion->m_bExport == 2)	Name += ")";
			int Index = m_List.AddString( Name ) ;
			m_List.SetItemData( Index, (DWORD)pMotion ) ;
		}
	}

	//---	Set Current Selection
	int CurrentSel = 0 ;
	if( pCharacter )
	{
		if( pCharacter->GetCurMotion() )
		{
			if( !pCharacter->GetCurMotion()->m_ChangedExport )
			{
				CurrentSel = MFCU_FindListBoxItemByData( &m_List, (DWORD)pCharacter->GetCurMotion() ) ;
				if( CurrentSel == -1 ) CurrentSel = 0 ;
			}
			else
			{
				CurrentSel = Index;
				if(!pCharacter->GetCurMotion()->m_bExport)
					Index++;
				pCharacter->GetCurMotion()->m_ChangedExport = FALSE;
			}
		}
		else
		{
			CurrentSel = Index;
		}
	}
	if( Index >= m_List.GetCount() )
		Index = m_List.GetCount()-1;

	m_List.SetSel( CurrentSel ) ;

	//---	Redraw Control
	m_List.SetRedraw( ) ;
	m_List.RedrawWindow() ;
}


void CDlgMotionList::UpdateLinkCharacterList( CCeDDoc *pDoc, CMotion *pControlMotion )
{
/*
	//---	Reset the List
	m_LinkCharacter.ResetContent() ;
	int Index = m_LinkCharacter.AddString( "<no character>" ) ;
	m_LinkCharacter.SetItemData( Index, 0 ) ;

	m_LinkCharacter.EnableWindow( pControlMotion != NULL ) ;

	//---	Add All Motions
	if( pControlMotion )
	{
		POSITION Pos = pDoc->m_CharacterList.GetHeadPosition() ;
		while( Pos )
		{
			CCharacter *pCharacter = pDoc->m_CharacterList.GetNext( Pos ) ;
			int Index = m_LinkCharacter.AddString( pCharacter->GetName() ) ;
			m_LinkCharacter.SetItemData( Index, (DWORD)pCharacter ) ;
		}
	}

	//---	Set Current Selection
	int CurrentSel = -1 ;
	if( pControlMotion )
		CurrentSel = MFCU_FindComboBoxItemByData( &m_LinkCharacter, (DWORD)pControlMotion->m_pLinkCharacter ) ;
	if( CurrentSel == -1 ) CurrentSel = 0 ;
	m_LinkCharacter.SetCurSel( CurrentSel ) ;

	//---	Redraw Control
	m_LinkCharacter.RedrawWindow() ;
	*/
}

void CDlgMotionList::UpdateLinkMotionList( CMotion *pControlMotion )
{
/*
	bool SelChange = FALSE;

	//---	Reset the List
	m_LinkMotion.ResetContent() ;
	int Index = m_LinkMotion.AddString( "<no motions>" ) ;
	m_LinkMotion.SetItemData( Index, 0 ) ;

	//---   
	// 
	// If a character has been selected for linking a motion to
	// the control motion then enable the link motion combo box
	// 
	//---   
	m_LinkMotion.EnableWindow( pControlMotion && (pControlMotion->m_pLinkCharacter != NULL) ) ;

	//---   
	// 
	// Add All Motions
	// 
	//---   
	if( pControlMotion && pControlMotion->m_pLinkCharacter )
	{
		CCharacter *pCharacter = pControlMotion->m_pLinkCharacter ;

		POSITION Pos = pCharacter->m_MotionList.GetHeadPosition() ;
		while( Pos )
		{
			CMotion *pMotion = pCharacter->m_MotionList.GetNext( Pos ) ;
			char FileName[_MAX_FNAME] ;
			char Ext[_MAX_EXT] ;
			_splitpath( pMotion->GetPathName(), NULL, NULL, FileName, Ext ) ;
			CString Name = FileName ;
			Name += Ext ;
			int Index = m_LinkMotion.AddString( Name ) ;
			m_LinkMotion.SetItemData( Index, (DWORD)pMotion ) ;
		}
	}

	int CurrentSel = -1;

	if ( pControlMotion )
	{
		//---   
		// 
		// Check to see if the motion passed in has a linked motion.
		// If so, set the selected item in the link motion combo box 
		// to the appropriate linked motion.
		// 
		//---   
		if		( pControlMotion->m_pLinkMotion )
		{
			CurrentSel = MFCU_FindComboBoxItemByData( &m_LinkMotion, (DWORD)pControlMotion->m_pLinkMotion ) ;
		}
		//---   
		// 
		// If the motion passed in didn't have a linked motion then check to see if the 
		// motion passed in has a linked character. If so set the selected item in the 
		// link motion combo box to the next item in the list past the passed in motion. 
		// 
		//---   
		else if ( pControlMotion->m_pLinkCharacter )
		{
			CurrentSel = MFCU_FindComboBoxItemByData( &m_LinkMotion, (DWORD)pControlMotion );

			if (( CurrentSel != -1 ) && ((CurrentSel+1) < m_LinkMotion.GetCount()))
			{
				CurrentSel += 1;
				SelChange   = TRUE;
			}
		}

	}

	//---   
	// 
	// Finally if we still haven't found a motion to set selected in 
	// the linked motion list combo box, set the selected item to the 
	// first item in the list.
	// 
	//---   
	if ( CurrentSel == -1 ) 	CurrentSel = 0 ;


	//---	Set Current Selection
	m_LinkMotion.SetCurSel( CurrentSel ) ;

	//---   
	// 
	// If we selected a new motion for a linked character then 
	// call OnSelchangeLinkmotion to complete the linked motion
	// 
	//---   
	if ( SelChange )			OnSelchangeLinkmotion();

	//---	Redraw Control
	m_LinkMotion.RedrawWindow() ;
*/
}


void CDlgMotionList::UpdateFaceMoveDir( )
{
	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			int	FaceDir = 0 ;
			int	MoveDir = 0 ;

			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				s32 ExportAngle360 = (1<<pMotion->GetExportBits());
				int Frame = (s32)(pCharacter->GetCurFrame()+0.5f) ;
				FaceDir = (int)(pMotion->GetFaceDir( Frame ) * ExportAngle360 / R_360 + 0.5) % ExportAngle360 ;
				MoveDir = (int)(pMotion->GetMoveDir( Frame ) * ExportAngle360 / R_360 + 0.5) % ExportAngle360 ;
				while( FaceDir < 0 ) FaceDir += ExportAngle360 ;
				while( MoveDir < 0 ) MoveDir += ExportAngle360 ;
			}

			m_FaceDir.EnableWindow( pMotion != NULL ) ;
			m_MoveDir.EnableWindow( pMotion != NULL ) ;

			if( m_Updating == 0 )
			{
				m_Updating++ ;
				MFCU_UpdateEditControl( m_FaceDir, FaceDir ) ;
				MFCU_UpdateEditControl( m_MoveDir, MoveDir ) ;
				m_Updating-- ;
			}
		}
	}
}

void CDlgMotionList::UpdateMotionSymbol( )
{
/*	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				MFCU_UpdateEditControl( m_Symbol, pMotion->m_ExportName ) ;
			}
		}
	}*/
}







BEGIN_MESSAGE_MAP(CDlgMotionList, CDialog)
	//{{AFX_MSG_MAP(CDlgMotionList)
	ON_BN_CLICKED(IDC_MOTION_ADD, OnMotionAdd)
	ON_BN_CLICKED(IDC_MOTION_DELETE, OnMotionDelete)
	ON_LBN_SELCHANGE(IDC_MOTION_LIST, OnSelchangeMotionList)
	ON_EN_CHANGE(IDC_FACEDIR, OnChangeFacedir)
	ON_EN_CHANGE(IDC_MOVEDIR, OnChangeMovedir)
	ON_BN_CLICKED(IDC_RESETFACE, OnResetface)
	ON_BN_CLICKED(IDC_RESETMOVE, OnResetmove)
	ON_BN_CLICKED(IDC_SETALL_FACE, OnSetallFace)
	ON_BN_CLICKED(IDC_SETALL_MOVE, OnSetallMove)
	ON_BN_CLICKED(IDC_MOTION_FLAGS, OnMotionFlags)
	ON_BN_CLICKED(IDC_CHECK_AUTOCLEAR_FACE_DIR_MOTIONLIST, OnCheckAutoclearFaceDirMotionlist)
	ON_BN_CLICKED(IDC_CHECK_AUTOCLEAR_MOVE_DIR_MOTIONLIST, OnCheckAutoclearMoveDirMotionlist)
	ON_LBN_DBLCLK(IDC_MOTION_LIST, OnDblclkMotionList)
	ON_EN_CHANGE(IDC_MOTION_EXPORTBITS, OnChangeMotionExportbits)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MOTION_EXPORTBITS_SPIN, OnDeltaposMotionExportbitsSpin)
	ON_EN_CHANGE(IDC_MOTION_EXPORTFPS, OnChangeMotionExportfps)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MOTION_EXPORTFPS_SPIN, OnDeltaposMotionExportfpsSpin)
	ON_BN_CLICKED(IDC_NOEXPORT, OnNoexport)
	ON_EN_KILLFOCUS(IDC_MOTION_EXPORTFPS, OnKillfocusMotionExportfps)
	ON_EN_KILLFOCUS(IDC_MOTION_EXPORTBITS, OnKillfocusMotionExportbits)
	ON_BN_CLICKED(IDC_MOTION_EXPORTBITS_BUTTON, OnMotionExportbitsButton)
	ON_BN_CLICKED(IDC_MOTION_EXPORTFPS_BUTTON, OnMotionExportfpsButton)
	ON_BN_CLICKED(IDC_EDITMOTION, OnEditmotion)
	ON_BN_CLICKED(IDC_QUATERNION_ON, OnQuaternionOn)
	ON_BN_CLICKED(IDC_EXPORTQUATW_ON, OnExportquatwOn)
	ON_BN_CLICKED(IDC_EXPORTDEFAULT, OnExportdefault)
	ON_BN_CLICKED(IDC_MOTION_IMPORT, OnMotionImport)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify )

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMotionList message handlers

/*
ResetAllMoveDir
*/

void CDlgMotionList::OnOk( )
{
}

void CDlgMotionList::OnCancel( )
{
}

void CDlgMotionList::OnMotionAdd() 
{
	m_pView->GetDocument()->CharacterNewMotion( m_pView->m_pCharacter );
}

void CDlgMotionList::OnMotionImport() 
{
	ASSERT( m_pView ) ;
	m_pView->MotionAdd( ) ;
}

s32 CDlgMotionList::GetSelectedMotions( CMotion** pMotionList, s32 MaxCount )
{
	CMotion*	pMotion;
	s32			i, c;

	int  SelCount	= m_List.GetSelCount( );
	if( SelCount > MaxCount )
		SelCount = MaxCount;

	int IndexArray[MAX_SELECTED_MOTIONS];

	c = 0;
	m_List.GetSelItems( SelCount, IndexArray );
	for( i=0; i<SelCount; i++ )
	{
		pMotion = (CMotion*)m_List.GetItemData( IndexArray[i] ) ;
		if( pMotion )
			pMotionList[c++] = pMotion;
	}

	return c;
}

void CDlgMotionList::OnMotionDelete() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;

	int Sure = MessageBox( "Delete Selected Items?", "Delete Motion", MB_ICONWARNING|MB_YESNO ) ;
	if( Sure != IDYES )
		return;

	CMotion* pMotion;

	int  SelCount	= m_List.GetSelCount( );
	int* IndexArray = new int[SelCount];
	int  TopIndex	= m_List.GetTopIndex();

	m_List.GetSelItems( SelCount, IndexArray );

	//---   
	// 
	// Set the new motion to be the one prior to the first in the list
	// 
	//---   
	CMotion* pNewCurrentMotion = ( CMotion* )m_List.GetItemData( IndexArray[0]-1 );

	if ( SelCount > 1 )
	{

		for ( int i = SelCount-1; i >= 0; i-- )
		{
 			TopIndex = m_List.GetTopIndex();

			pMotion = ( CMotion* )m_List.GetItemData( IndexArray[i] ) ;

			if( pMotion )
			{
				m_pView->MotionDelete( pMotion, pNewCurrentMotion ) ;
				m_List.SetSel( IndexArray[i], FALSE );
   				m_List.DeleteString( IndexArray[i] );
			}

			m_List.SetTopIndex( TopIndex );
			m_List.SetRedraw( ) ;
			m_List.RedrawWindow() ;
 			Sleep( 20 );
		}
	}
	else
	{
		pMotion = ( CMotion* )m_List.GetItemData( IndexArray[0] ) ;

		if( pMotion )
		{
			m_pView->MotionDelete( pMotion, pNewCurrentMotion ) ;
		}
	}

 	m_pView->MotionsDeleted();

 	m_List.SetSel( -1, FALSE );
// 		m_List.SetTopIndex( m_List.GetSel( IndexArray[0] - 1 ) );
 	m_List.SetSel( IndexArray[0] - 1, TRUE );
	m_List.SetRedraw( ) ;
	m_List.RedrawWindow() ;

	delete [] IndexArray;
/*
	
	CMotion *pMotion = (CMotion *)m_List.GetItemData( m_List.GetCurSel() ) ;
	if( pMotion )
	{
		CMotion *pNewCurrentMotion = (CMotion*)m_List.GetItemData( min(m_List.GetCount()-1, m_List.GetCaretIndex()+1) ) ;
		if( pNewCurrentMotion == pMotion )
			pNewCurrentMotion = (CMotion*)m_List.GetItemData( max(0, m_List.GetCaretIndex()-1) ) ;
		m_pView->MotionDelete( pMotion, pNewCurrentMotion ) ;
	}
*/
}

void CDlgMotionList::OnSelchangeMotionList() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;
	ASSERT( ((u32)m_pView&0xf0000000) == 0 ); // I don't know what corrupted the m_pView pointer, but this will hopefully catch it sooner.

	CMotion *pMotion = (CMotion *)m_List.GetItemData( m_List.GetCurSel() ) ;
	m_pView->MotionSetCurrent( pMotion ) ;
}

void CDlgMotionList::OnChangeFacedir() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here

	if( m_Updating == 0 )
	{
		if( m_pView )
		{
			CCharacter *pCharacter = m_pView->m_pCharacter ;
			if( pCharacter )
			{
				CMotion *pMotion = pCharacter->GetCurMotion( ) ;
				if( pMotion )
				{
					int Frame = (s32)(pCharacter->GetCurFrame()+0.5f) ;
					CString String ;
					m_FaceDir.GetWindowText( String ) ;
					pMotion->SetFaceDir( Frame, atoi(String) * R_360 / (f32)(1<<pMotion->GetExportBits()) ) ;
					m_Updating++ ;
					m_pView->RedrawWindow( ) ;
					m_Updating-- ;
				}
			}
		}
	}
}

void CDlgMotionList::OnChangeMovedir() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( m_Updating == 0 )
	{
		if( m_pView )
		{
			CCharacter *pCharacter = m_pView->m_pCharacter ;
			if( pCharacter )
			{
				CMotion *pMotion = pCharacter->GetCurMotion( ) ;
				if( pMotion )
				{
					int Frame = (s32)(pCharacter->GetCurFrame( )+0.5f) ;
					CString String ;
					m_MoveDir.GetWindowText( String ) ;
					pMotion->SetMoveDir( Frame, atoi(String) * R_360 / (f32)(1<<pMotion->GetExportBits()) ) ;
					m_Updating++ ;
					m_pView->RedrawWindow( ) ;
					m_Updating-- ;
				}
			}
		}
	}
}

void CDlgMotionList::OnResetface() 
{
	// TODO: Add your control notification handler code here
	if( m_Updating == 0 )
	{
		if( m_pView )
		{
			CCharacter *pCharacter = m_pView->m_pCharacter ;
			if( pCharacter )
			{
				CMotion *pMotion = pCharacter->GetCurMotion( ) ;
				if( pMotion )
				{
                    pMotion->ResetAllFaceDir( TRUE );
					m_Updating++ ;
					m_pView->RedrawWindow( ) ;
					m_Updating-- ;
				}
			}
		}
	}
}

void CDlgMotionList::OnResetmove() 
{
	// TODO: Add your control notification handler code here
	if( m_Updating == 0 )
	{
		if( m_pView )
		{
			CCharacter *pCharacter = m_pView->m_pCharacter ;
			if( pCharacter )
			{
				CMotion *pMotion = pCharacter->GetCurMotion( ) ;
				if( pMotion )
				{
                    pMotion->ResetAllMoveDir();
					//int Frame = pCharacter->GetCurFrame( ) ;
					//CString String ;
					//m_MoveDir.GetWindowText( String ) ;
					//pMotion->SetMoveDir( Frame, atoi(String) * R_360 / (f32)(1<<m_pView->m_pCharacter->m_ExportBits) ) ;
					m_Updating++ ;
					m_pView->RedrawWindow( ) ;
					m_Updating-- ;
				}
			}
		}
	}
}

void CDlgMotionList::OnChangeSymbol() 
{
/*
	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				CString String ;
				m_Symbol.GetWindowText( String ) ;
				pMotion->m_ExportName = String ;
			}
		}
	}
*/
}

void CDlgMotionList::OnSetallFace() 
{
	// TODO: Add your control notification handler code here
	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				int Frame = (s32)(pCharacter->GetCurFrame( )+0.5f) ;
				f32 FaceDir = pMotion->GetFaceDir( Frame ) ;
				pMotion->SetAllFaceDir( FaceDir ) ;
				m_pView->RedrawWindow( ) ;
			}
		}
	}
}

void CDlgMotionList::OnSetallMove() 
{
	// TODO: Add your control notification handler code here
	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				int Frame = (s32)(pCharacter->GetCurFrame( )+0.5f) ;
				f32 MoveDir = pMotion->GetMoveDir( Frame ) ;
				pMotion->SetAllMoveDir( MoveDir ) ;
				m_pView->RedrawWindow( ) ;
			}
		}
	}
}

#define EX1(_x_,_y_) _x_=_y_
#define EX2(_x_,_y_) _y_=_x_

void CDlgMotionList::OnMotionFlags() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;

	bool	HaveSelected = false ;
	CCeDDoc *pDoc = m_pView->GetDocument( ) ;

	CMotionFlags mf ;

	//---	Setup State of Flag Bits -1 == no state yet, 0 = OFF, 1 = ON, 2 = GRAY
	for( int i = 0 ; i < m_List.GetCount() ; i++ )
	{
		if( m_List.GetSel( i ) )
		{
			CMotion *pMotion = (CMotion *)m_List.GetItemData( i ) ;
			if( pMotion )
			{
				HaveSelected = true ;

				for( int j = 0 ; j < MOTION_FLAG_BITS ; j++ )
				{
					if( pMotion->m_FlagBit[j] )
					{
						if( mf.m_Bit[j] == -1 ) mf.m_Bit[j] = 1 ;
						if( mf.m_Bit[j] ==  0 ) mf.m_Bit[j] = 2 ;
					}
					else
					{
						if( mf.m_Bit[j] == -1 ) mf.m_Bit[j] = 0 ;
						if( mf.m_Bit[j] ==  1 ) mf.m_Bit[j] = 2 ;
					}
				}
			}
		}
	}

	if( HaveSelected )
	{
		//---	Exchange Define & Text info with Dialog
		CString *pFlagDefine = pDoc->m_FlagDefine ;
		CString *pFlagDescription = pDoc->m_FlagDescription ;
		EX1(mf.m_Def0,pFlagDefine[0]) ;
		EX1(mf.m_Def1,pFlagDefine[1]) ;
		EX1(mf.m_Def2,pFlagDefine[2]) ;
		EX1(mf.m_Def3,pFlagDefine[3]) ;
		EX1(mf.m_Def4,pFlagDefine[4]) ;
		EX1(mf.m_Def5,pFlagDefine[5]) ;
		EX1(mf.m_Def6,pFlagDefine[6]) ;
		EX1(mf.m_Def7,pFlagDefine[7]) ;
		EX1(mf.m_Def8,pFlagDefine[8]) ;
		EX1(mf.m_Def9,pFlagDefine[9]) ;
		EX1(mf.m_Def10,pFlagDefine[10]) ;
		EX1(mf.m_Def11,pFlagDefine[11]) ;
		EX1(mf.m_Def12,pFlagDefine[12]) ;
		EX1(mf.m_Def13,pFlagDefine[13]) ;
		EX1(mf.m_Def14,pFlagDefine[14]) ;
		EX1(mf.m_Def15,pFlagDefine[15]) ;
		EX1(mf.m_Txt0,pFlagDescription[0]) ;
		EX1(mf.m_Txt1,pFlagDescription[1]) ;
		EX1(mf.m_Txt2,pFlagDescription[2]) ;
		EX1(mf.m_Txt3,pFlagDescription[3]) ;
		EX1(mf.m_Txt4,pFlagDescription[4]) ;
		EX1(mf.m_Txt5,pFlagDescription[5]) ;
		EX1(mf.m_Txt6,pFlagDescription[6]) ;
		EX1(mf.m_Txt7,pFlagDescription[7]) ;
		EX1(mf.m_Txt8,pFlagDescription[8]) ;
		EX1(mf.m_Txt9,pFlagDescription[9]) ;
		EX1(mf.m_Txt10,pFlagDescription[10]) ;
		EX1(mf.m_Txt11,pFlagDescription[11]) ;
		EX1(mf.m_Txt12,pFlagDescription[12]) ;
		EX1(mf.m_Txt13,pFlagDescription[13]) ;
		EX1(mf.m_Txt14,pFlagDescription[14]) ;
		EX1(mf.m_Txt15,pFlagDescription[15]) ;

		if( mf.DoModal( ) == IDOK )
		{
			//---	Retreive State of Flag Bits -1 == no state yet, 0 = OFF, 1 = ON, 2 = GRAY
			for( int i = 0 ; i < m_List.GetCount() ; i++ )
			{
				if( m_List.GetSel( i ) )
				{
					CMotion *pMotion = (CMotion *)m_List.GetItemData( i ) ;
					if( pMotion )
					{
						for( int j = 0 ; j < MOTION_FLAG_BITS ; j++ )
						{
							if( (mf.m_Bit[j] == 0) || (mf.m_Bit[j] == 1) )
								pMotion->m_FlagBit[j] = !!mf.m_Bit[j] ;
						}
					}
				}
			}

			//---	Exchange Define & Text info with Dialog
			EX2(mf.m_Def0,pFlagDefine[0]) ;
			EX2(mf.m_Def1,pFlagDefine[1]) ;
			EX2(mf.m_Def2,pFlagDefine[2]) ;
			EX2(mf.m_Def3,pFlagDefine[3]) ;
			EX2(mf.m_Def4,pFlagDefine[4]) ;
			EX2(mf.m_Def5,pFlagDefine[5]) ;
			EX2(mf.m_Def6,pFlagDefine[6]) ;
			EX2(mf.m_Def7,pFlagDefine[7]) ;
			EX2(mf.m_Def8,pFlagDefine[8]) ;
			EX2(mf.m_Def9,pFlagDefine[9]) ;
			EX2(mf.m_Def10,pFlagDefine[10]) ;
			EX2(mf.m_Def11,pFlagDefine[11]) ;
			EX2(mf.m_Def12,pFlagDefine[12]) ;
			EX2(mf.m_Def13,pFlagDefine[13]) ;
			EX2(mf.m_Def14,pFlagDefine[14]) ;
			EX2(mf.m_Def15,pFlagDefine[15]) ;
			EX2(mf.m_Txt0,pFlagDescription[0]) ;
			EX2(mf.m_Txt1,pFlagDescription[1]) ;
			EX2(mf.m_Txt2,pFlagDescription[2]) ;
			EX2(mf.m_Txt3,pFlagDescription[3]) ;
			EX2(mf.m_Txt4,pFlagDescription[4]) ;
			EX2(mf.m_Txt5,pFlagDescription[5]) ;
			EX2(mf.m_Txt6,pFlagDescription[6]) ;
			EX2(mf.m_Txt7,pFlagDescription[7]) ;
			EX2(mf.m_Txt8,pFlagDescription[8]) ;
			EX2(mf.m_Txt9,pFlagDescription[9]) ;
			EX2(mf.m_Txt10,pFlagDescription[10]) ;
			EX2(mf.m_Txt11,pFlagDescription[11]) ;
			EX2(mf.m_Txt12,pFlagDescription[12]) ;
			EX2(mf.m_Txt13,pFlagDescription[13]) ;
			EX2(mf.m_Txt14,pFlagDescription[14]) ;
			EX2(mf.m_Txt15,pFlagDescription[15]) ;
		}
	}
}

void CDlgMotionList::UpdateAutoResetFaceDir()
{
	BOOL bReset = TRUE;
	BOOL bMotion = FALSE;

	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				bReset = pMotion->GetAutoResetFaceDir();
				bMotion = TRUE;
			}
		}
	}
	m_ButtonAutoResetFace.EnableWindow( bMotion );
	m_ButtonAutoResetFace.SetCheck( bReset ? 1 : 0 );
	m_FaceDir.SendMessage( EM_SETREADONLY, bReset ? TRUE : FALSE, 0 );
	GetDlgItem( IDC_RESETFACE )->EnableWindow( bReset ? FALSE : TRUE );
	GetDlgItem( IDC_SETALL_FACE )->EnableWindow( bReset ? FALSE : TRUE );
}

void CDlgMotionList::UpdateAutoResetMoveDir()
{
	BOOL bReset = TRUE;
	BOOL bMotion = FALSE;

	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				bReset = pMotion->GetAutoResetMoveDir();
				bMotion = TRUE;
			}
		}
	}
	m_ButtonAutoResetMove.EnableWindow( bMotion );
	m_ButtonAutoResetMove.SetCheck( bReset ? 1 : 0 );
	m_MoveDir.SendMessage( EM_SETREADONLY, bReset ? TRUE : FALSE, 0 );
	GetDlgItem( IDC_RESETMOVE )->EnableWindow( bReset ? FALSE : TRUE );
	GetDlgItem( IDC_SETALL_MOVE )->EnableWindow( bReset ? FALSE : TRUE );
}

void CDlgMotionList::OnCheckAutoclearFaceDirMotionlist() 
{
	INT CheckState;
	CDocument* pDocument = NULL;

	CheckState = m_ButtonAutoResetFace.GetCheck();
	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				pMotion->SetAutoResetFaceDir( ( CheckState == 1 ) ? TRUE : FALSE );
				/*
				if( CheckState == 1)
				{
					OnResetface();
				}
				*/
			}
		}
		pDocument = m_pView->GetDocument();
		if( pDocument )
		{
			pDocument->UpdateAllViews( NULL );
		}
	}
}

void CDlgMotionList::OnCheckAutoclearMoveDirMotionlist() 
{
	INT CheckState;
	CDocument* pDocument = NULL;

	CheckState = m_ButtonAutoResetMove.GetCheck();
	if( m_pView )
	{
		CCharacter *pCharacter = m_pView->m_pCharacter ;
		if( pCharacter )
		{
			CMotion *pMotion = pCharacter->GetCurMotion( ) ;
			if( pMotion )
			{
				pMotion->SetAutoResetMoveDir( ( CheckState == 1 ) ? TRUE : FALSE );
				/*
				if( CheckState == 1)
				{
					OnResetmove();
				}
				*/
			}
		}
		pDocument = m_pView->GetDocument();
		if( pDocument )
		{
			pDocument->UpdateAllViews( NULL );
		}
	}
}

BOOL CDlgMotionList::OnToolTipNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pTTTStruct;    
    UINT nID = pTTTStruct->idFrom;

    if (pTTT->uFlags & TTF_IDISHWND)    
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);        
        if(nID)        
        {
            pTTT->lpszText = MAKEINTRESOURCE(nID);
            pTTT->hinst = AfxGetResourceHandle();            
            return TRUE;
        }    
    }    return FALSE;
}

BOOL CDlgMotionList::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ExportMotionBitsSpin.SetRange( 0, 16 );
	m_ExportMotionFPSSpin.SetRange( 0, 60 );
	
	EnableToolTips( TRUE );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMotionList::OnDblclkMotionList()
{
	if( m_List.GetCurSel() == -1 )
		return;

	CMotion* pMotion = (CMotion*) m_List.GetItemData( m_List.GetCurSel() );

	if( !pMotion )
		return;

	if( m_pView->m_bAnimTransViewerDisplayed )
		m_pView->m_AnimTransViewer.AddTransition( pMotion );
	else
		m_pView->MotionModify( pMotion );
}


void CDlgMotionList::UpdateExportInfo()
{
	bool Enable, bExportBits=FALSE, bExportFPS=FALSE;
	CCharacter *pCharacter = NULL;
	CMotion *pMotion = NULL;
	static bool InFunc=0;

	if( InFunc )
		return;
	InFunc = TRUE;

	if( m_pView )
		pCharacter = m_pView->m_pCharacter ;
	if( pCharacter )
		pMotion = pCharacter->GetCurMotion( ) ;

	if( pMotion )
	{
		CString Text;
		Text.Format( "%d", pMotion->m_ExportBits );
		m_ExportMotionBits.SetWindowText( Text );
		m_ExportMotionBitsSpin.SetPos( pMotion->m_ExportBits );
		m_ExportMotionBitsButton.SetCheck( pMotion->m_bExportBits ? TRUE : FALSE );
		bExportBits = pMotion->m_bExportBits;


		Text.Format( "%5.2f", pMotion->m_ExportFrameRate );
		m_ExportMotionFPS.SetWindowText( Text );
		m_ExportMotionFPSSpin.SetPos( (s32)pMotion->m_ExportFrameRate );
		m_ExportMotionFPSButton.SetCheck( pMotion->m_bExportFrameRate ? TRUE : FALSE );
		bExportFPS = pMotion->m_bExportFrameRate;

		m_ExportQuatW.SetCheck( pMotion->m_bExportQuatW ? TRUE : FALSE );

		m_NoExport.SetCheck( pMotion->m_bExport );
		m_ExportDefault.SetCheck( pMotion->m_bExportDefault ? TRUE : FALSE );

		Enable = TRUE;
	}
	else
		Enable = FALSE;

	m_ExportMotionBits.EnableWindow( Enable && bExportBits );
	m_ExportMotionBitsSpin.EnableWindow( Enable && bExportBits );
	m_ExportMotionBitsButton.EnableWindow( Enable );

	m_ExportMotionFPS.EnableWindow( Enable && bExportFPS );
	m_ExportMotionFPSSpin.EnableWindow( Enable && bExportFPS );
	m_ExportMotionFPSButton.EnableWindow( Enable );

	m_ExportDefault.EnableWindow( Enable );
	m_Quaternion.EnableWindow( Enable && !m_ExportDefault.GetCheck() );
	m_ExportQuatW.EnableWindow( Enable && !m_ExportDefault.GetCheck() && m_Quaternion.GetCheck() );

	m_NoExport.EnableWindow( Enable );

	InFunc = FALSE;
}

void CDlgMotionList::OnChangeMotionExportbits() 
{
	CString Bits;

	if( !m_ExportMotionBits.m_hWnd )
		return;

	m_ExportMotionBits.GetWindowText( Bits );	
	s32 eBits = atoi( Bits );
	if( eBits > 16 )		eBits = 16;
	else if( eBits < 0 )	eBits = 0;
	m_ExportMotionBitsSpin.SetPos( eBits );


#if 1

	CMotion*	pMotion;
	s32			iMotion = m_pView->GetFirstSelectedMotion();

	while( pMotion = m_pView->GetNextSelectedMotion( iMotion ) )
		pMotion->m_ExportBits = eBits;

#else

	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	pMotion->m_ExportBits = eBits;

#endif

	//---	set the direction values to reflect the new export bits
	m_pView->UpdateMotionControls();
	m_pView->UpdateEventControls();
	m_pView->Invalidate();
}

void CDlgMotionList::OnDeltaposMotionExportbitsSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CDlgMotionList::OnChangeMotionExportfps() 
{
	CMotion dummyMotion;
	
	if( !m_ExportMotionFPS.m_hWnd )
		return;

	dummyMotion.m_bExportFrameRate = TRUE;

	CString FrameRate;
	m_ExportMotionFPS.GetWindowText( FrameRate );
	f32 fFrameRate = (f32)atof( FrameRate );
	dummyMotion.m_ExportFrameRate = fFrameRate;

	//---	keep the value within range
	if(( fFrameRate > 60.0f ) || ( fFrameRate < 0.0f ))
	{
		if( fFrameRate > 10.0f )
		{
			fFrameRate = 60.0f;
			FrameRate.Format( "%5.2f", fFrameRate );
		}
		else
		{
			fFrameRate = 0.0f;
			FrameRate.Format( "%5.2f", fFrameRate );
		}
		m_ExportMotionFPS.SetWindowText( FrameRate );
	}

	fFrameRate = dummyMotion.GetExportFrameRate();
	m_ExportMotionFPSSpin.SetPos( (s32)fFrameRate );

#if 1

	CMotion*	pMotion;
	s32			iMotion = m_pView->GetFirstSelectedMotion();

	while( pMotion = m_pView->GetNextSelectedMotion( iMotion ) )
		pMotion->m_ExportFrameRate = fFrameRate;	

	m_pView->Invalidate();

#else

	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;

	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	pMotion->m_ExportFrameRate = fFrameRate;	
	m_pView->Invalidate();

#endif

}

void CDlgMotionList::OnDeltaposMotionExportfpsSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	float Change = 1.0f*(float)pNMUpDown->iDelta;
	CString Text;
	
	m_ExportMotionFPS.GetWindowText( Text );
	f32 Value = (f32)atof( Text );
	Value += Change;
	Text.Format( "%5.2f", Value );
	m_ExportMotionFPS.SetWindowText( Text );
	
	*pResult = 0;
}

void CDlgMotionList::OnNoexport() 
{
#if 1

	CMotion*	pMotion;
	s32			iMotion = m_pView->GetFirstSelectedMotion();

	while( pMotion = m_pView->GetNextSelectedMotion( iMotion ) )
		pMotion->m_bExport = m_NoExport.GetCheck();

#else

	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	pMotion->m_bExport = m_NoExport.GetCheck();

#endif
}

void CDlgMotionList::OnKillfocusMotionExportfps() 
{
	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	CString FrameRate;
	FrameRate.Format( "%5.2f", pMotion->m_ExportFrameRate );
	m_ExportMotionFPS.SetWindowText( FrameRate );
}

void CDlgMotionList::OnKillfocusMotionExportbits() 
{
	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	CString Bits;
	Bits.Format( "%d", pMotion->m_ExportBits );
	m_ExportMotionBits.SetWindowText( Bits );

	//---	set the direction values to reflect the new export bits
	m_pView->UpdateMotionControls();
	m_pView->UpdateEventControls();
}

void CDlgMotionList::OnMotionExportbitsButton() 
{
#if 1

	CMotion*	pMotion;
	s32			iMotion = m_pView->GetFirstSelectedMotion();

	while( pMotion = m_pView->GetNextSelectedMotion( iMotion ) )
	{
		pMotion->m_bExportBits = m_ExportMotionBitsButton.GetCheck() ? TRUE : FALSE;
		pMotion->ForceCompressedVersionUpdate();
	}

	UpdateExportInfo();
	m_pView->Invalidate();

#else

	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	pMotion->m_bExportBits = m_ExportMotionBitsButton.GetCheck() ? TRUE : FALSE;
	UpdateExportInfo();
	m_pView->Invalidate();

#endif
}

void CDlgMotionList::OnMotionExportfpsButton() 
{
#if 1

	CMotion*	pMotion;
	s32			iMotion = m_pView->GetFirstSelectedMotion();

	while( pMotion = m_pView->GetNextSelectedMotion( iMotion ) )
	{
		pMotion->m_bExportFrameRate = m_ExportMotionFPSButton.GetCheck() ? TRUE : FALSE;
		pMotion->ForceCompressedVersionUpdate();
	}

	UpdateExportInfo();
	m_pView->Invalidate();

#else

	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	pMotion->m_bExportFrameRate = m_ExportMotionFPSButton.GetCheck() ? TRUE : FALSE;
	UpdateExportInfo();
	m_pView->Invalidate();

#endif
}

void CDlgMotionList::OnEditmotion() 
{
	OnDblclkMotionList();
}

void CDlgMotionList::OnQuaternionOn() 
{
#if 1

	if( !m_pView )		return;

	CMotion*	pMotion;
	s32			iMotion = m_pView->GetFirstSelectedMotion();

	while( pMotion = m_pView->GetNextSelectedMotion( iMotion ) )
	{
		pMotion->m_bExportQuaternion	= m_Quaternion.GetCheck() ? TRUE : FALSE;
		pMotion->ForceCompressedVersionUpdate();
	}

	UpdateExportInfo();
	m_pView->Invalidate();

#else

	if( !m_pView )		return;
	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	pMotion->m_bExportQuaternion	= m_Quaternion.GetCheck() ? TRUE : FALSE;
	pMotion->ForceCompressedVersionUpdate();

	UpdateExportInfo();
	m_pView->Invalidate();

#endif
}


void CDlgMotionList::OnExportquatwOn() 
{
#if 1

	if( !m_pView )		return;

	CMotion*	pMotion;
	s32			iMotion = m_pView->GetFirstSelectedMotion();

	while( pMotion = m_pView->GetNextSelectedMotion( iMotion ) )
	{
		pMotion->m_bExportQuatW	= m_ExportQuatW.GetCheck() ? TRUE : FALSE;
		pMotion->ForceCompressedVersionUpdate();
	}

	UpdateExportInfo();
	m_pView->Invalidate();

#else

	if( !m_pView )		return;
	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	pMotion->m_bExportQuatW	= m_ExportQuatW.GetCheck() ? TRUE : FALSE;
	pMotion->ForceCompressedVersionUpdate();
	UpdateExportInfo();
	m_pView->Invalidate();

#endif
}

void CDlgMotionList::OnExportdefault() 
{
#if 1

	if( !m_pView )		return;

	CMotion*	pMotion;
	s32			iMotion = m_pView->GetFirstSelectedMotion();

	while( pMotion = m_pView->GetNextSelectedMotion( iMotion ) )
	{
		pMotion->m_bExportDefault	= m_ExportDefault.GetCheck() ? TRUE : FALSE;
		pMotion->ForceCompressedVersionUpdate();
	}

	UpdateExportInfo();
	m_pView->Invalidate();

#else

	if( !m_pView )		return;
	CCharacter *pCharacter = m_pView->m_pCharacter ;
	if( !pCharacter )	return;
	CMotion *pMotion = pCharacter->GetCurMotion( ) ;
	if( !pMotion )		return;

	pMotion->m_bExportDefault	= m_ExportDefault.GetCheck() ? TRUE : FALSE;
	pMotion->ForceCompressedVersionUpdate();
	UpdateExportInfo();
	m_pView->Invalidate();

#endif
}

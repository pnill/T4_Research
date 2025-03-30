// MotionEditView.cpp : implementation file
//

#include "stdafx.h"
#include "CeD.h"
#include "MotionEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "MotionEditFrame.h"
#include "Skeleton.h"
#include "RenderContext.h"
#include "ExportFile.h"
#include "MFCUtils.h"
#include "DlgCharacter.h"
#include "ExportParams.h"

#include <mmsystem.h>

/////////////////////////////////////////////////////////////////////////////
// CMotionEditView

IMPLEMENT_DYNCREATE(CMotionEditView, CCeDView)

CMotionEditView::CMotionEditView()
{
	m_pCharacter				= NULL ;
	m_ViewState					= MEVS_SELECT ;
	m_CameraControlEditsLocked	= false ;
	m_CameraLockedBone			= 0 ;
	m_Playing					= false ;
	m_TimerEvent				= 0 ;
	m_nTimerMessages			= 0 ;
	m_ViewAllFaceDirs			= false ;
	m_ViewAllMoveDirs			= false ;
	m_ViewPrimaryMotion			= true ;
	m_ViewCompressedMotion		= false ;
	m_ViewMotionBlended			= true ;
	m_ViewMirroredSkeleton		= false ;
	m_TimeAccuratePlay			= false ;
	m_StartTiming				= false ;
	m_pEventUnderMouse			= NULL ;
	m_bInitialUpdate			= FALSE;
}

CMotionEditView::~CMotionEditView()
{
	if( m_TimerEvent )
	{
		MMRESULT res = timeKillEvent( m_TimerEvent ) ;
        ASSERT( res == TIMERR_NOERROR ) ;
		m_TimerEvent = 0 ;
        Sleep( 100 ) ;
	}
}


BEGIN_MESSAGE_MAP(CMotionEditView, CView)
	//{{AFX_MSG_MAP(CMotionEditView)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_STATE_SELECT, OnStateSelect)
	ON_COMMAND(ID_STATE_PAN, OnStatePan)
	ON_COMMAND(ID_STATE_ROTATE, OnStateRotate)
	ON_COMMAND(ID_STATE_ZOOM, OnStateZoom)
	ON_COMMAND(ID_ZOOM_ALL, OnZoomAll)
	ON_COMMAND(ID_STATE_SELECTMOVE, OnStateSelectmove)
	ON_COMMAND(ID_STATE_SELECTROTATE, OnStateSelectrotate)
	ON_COMMAND(ID_ANIM_GOTO_START, OnAnimGotoStart)
	ON_COMMAND(ID_ANIM_PREVIOUS_FRAME, OnAnimPreviousFrame)
	ON_COMMAND(ID_ANIM_PLAY, OnAnimPlay)
	ON_COMMAND(ID_ANIM_NEXT_FRAME, OnAnimNextFrame)
	ON_COMMAND(ID_ANIM_GOTO_END, OnAnimGotoEnd)
	ON_COMMAND(ID_STATE_MOVEPLANEY, OnStateMoveplaney)
	ON_COMMAND(ID_STATE_FACEDIR, OnStateFacedir)
	ON_WM_TIMER()
	ON_COMMAND(ID_CHARACTER_IMPORT_SKEL, OnCharacterImportSkel)
	ON_COMMAND(ID_CHARACTER_IMPORT_MOTION, OnCharacterImportMotion)
	ON_COMMAND(ID_VIEW_ALL_FACEDIRS, OnViewAllFacedirs)
	ON_COMMAND(ID_VIEW_ALL_MOVEDIRS, OnViewAllMovedirs)
	ON_COMMAND(ID_STATE_MOVEDIR, OnStateMovedir)
	ON_COMMAND(ID_VIEW_MIRRORED_SKELETON, OnViewMirroredSkeleton)
	ON_COMMAND(ID_VIEW_COMPRESSED_MOTION, OnViewCompressedMotion)
	ON_COMMAND(ID_VIEW_PRIMARY_MOTION, OnViewPrimaryMotion)
	ON_COMMAND(ID_VIEW_MOTION_BLENDED, OnViewMotionBlended)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_CHARACTER_EXPORT, OnCharacterExport)
	ON_UPDATE_COMMAND_UI(ID_STATE_FACEDIR, OnUpdateStateFacedir)
	ON_UPDATE_COMMAND_UI(ID_STATE_MOVEDIR, OnUpdateStateMovedir)
	ON_COMMAND(ID_CHARACTER_LOADMOTIONS, OnCharacterLoadmotions)
	ON_COMMAND(ID_STATE_EVENT, OnStateEvent)
	ON_UPDATE_COMMAND_UI(ID_STATE_EVENT, OnUpdateStateEvent)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED( IDC_FRAME_START, OnClkFrameStart )
	ON_BN_CLICKED( IDC_FRAME_END, OnClkFrameEnd )

	ON_COMMAND(ID_FACEDIR_INTERP, OnFaceDirInterpolate)
	ON_COMMAND(ID_MOVEDIR_INTERP, OnMoveDirInterpolate)
	ON_UPDATE_COMMAND_UI(ID_MOVEDIR_INTERP, OnUpdateMoveDirInterpolate)
	ON_UPDATE_COMMAND_UI(ID_FACEDIR_INTERP, OnUpdateFaceDirInterpolate)

END_MESSAGE_MAP()
 
/////////////////////////////////////////////////////////////////////////////
// CMotionEditView drawing

void CMotionEditView::DrawSkeleton( CRenderContext *pRC, CCharacter *pCharacter )
{
	if( pCharacter )
	{
		CSkeleton	*pSkeleton = pCharacter->GetSkeleton() ;
		if( pSkeleton )
		{
			pSkeleton->Render( pRC, RGB(0,0,0), true ) ;
		}
	}
}

void CMotionEditView::DrawView( CRenderContext *pRC )
{
	CDC *pDC = pRC->m_pDC ;

	CCeDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	//---	Make Sure Character is correctly posed before continuing
	if( m_pCharacter )
	{
		//---	Pose Skeleton
		m_pCharacter->SetPose( GetCurMotion(), m_pCharacter->GetCurFrame(), m_ViewMotionBlended ) ;
	}

	//---	Update Camera Controls
	if( m_pCharacter && m_pCharacter->GetSkeleton() )
	{
		if( m_CameraLockedBone != 0 )
		{
			vector3d	v ;
			M4_GetTranslation( m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(m_CameraLockedBone-1), &v ) ;
			m_Camera.SetTarget( &v ) ;
			m_Camera.ComputeMatrix() ;
			UpdateCameraControls( ) ;
		}
	}

	//---	Update Motion Controls
	if( GetCurMotion() )
	{
		UpdateMotionControls() ;
	}

	//---	Setup Camera with parameters from CDC to render to
	CRect	r ;
	GetClientRect( &r ) ;
	m_Camera.SetProjectCenter( (f32)r.Width() / 2, (f32)r.Height() / 2, 0, 0 ) ;

	//---	Get a Matrix for positioning objects in the world
	matrix4		m ;
	M4_Identity( &m ) ;

	//---	Draw Ground Plane
	if( pDoc->m_DrawGroundPlane )
	{
        radian3d r = {0};
        vector3d v = {0};
		M4_SetRotationsXYZ( &m, &r ) ;
		M4_SetTranslation( &m, &v ) ;
		pRC->PushMatrix( &m ) ;
		pRC->RENDER_Grid( RGB(128,255,128), RGB(224,255,224) ) ;
		pRC->PopMatrix( ) ;
	}

	//---	Draw Construction Plane
	if( pDoc->m_DrawConstructionPlane )
	{
        radian3d r = {0};
		M4_SetRotationsXYZ( &m, &r ) ;
		M4_SetTranslation( &m, &pDoc->m_ConPlanePos ) ;
		pRC->PushMatrix( &m ) ;
		pRC->RENDER_Grid( RGB(255,128,128), RGB(255,224,224) ) ;
		pRC->PopMatrix( ) ;
	}

	//---	Draw MoveDir Arrow
	if( GetCurMotion() )
	{
		CMotion *pMotion = GetCurMotion() ;
		int iCurrentFrame = (int)(m_pCharacter->GetCurFrame()+0.5f) ;
		int nFrames = pMotion->GetNumFrames() ;

		//---	Draw Arrows for Every Frame?
		if( m_ViewAllFaceDirs )
		{
			for( int i = 0 ; i < nFrames ; i++ )
			{
				int d = (i - iCurrentFrame) ; if( d < 0 ) d = -d ; if( d > (nFrames/2) ) d = nFrames-d ; d = min( d, 1 ) ;
				int	c = d*192 ;

				vector3d	t ;
                radian3d    r = {0};
				pMotion->GetTranslation( &t, i ) ;
                r.Y = pMotion->GetFaceDir( i );
				M4_SetRotationsXYZ( &m, &r ) ;
				M4_SetTranslation( &m, &t ) ;
				pRC->PushMatrix( &m ) ;
				pRC->RENDER_Arrow( 20, RGB(c,c,255) ) ;
				pRC->PopMatrix( ) ;
			}
		}
		if( m_ViewAllMoveDirs )
		{
			for( int i = 0 ; i < nFrames ; i++ )
			{
				int d = (i - iCurrentFrame) ; if( d < 0 ) d = -d ; if( d > (nFrames/2) ) d = nFrames-d ; d = min( d, 1 ) ;
				int	c = d*192 ;

				vector3d	t ;
                radian3d    r = {0};
				pMotion->GetTranslation( &t, i ) ;
                r.Y = pMotion->GetMoveDir( i );
				M4_SetRotationsXYZ( &m, &r ) ;
				M4_SetTranslation( &m, &t ) ;
				pRC->PushMatrix( &m ) ;
				pRC->RENDER_Arrow( 20, RGB(255,c,c) ) ;
				pRC->PopMatrix( ) ;
			}
		}

		//---	Draw Arrows for Current Frame
		vector3d	t ;
		pMotion->GetTranslation( &t, (int)(m_pCharacter->GetCurFrame()+0.5f) ) ;

		if( (!m_ViewAllFaceDirs && !m_ViewAllMoveDirs) || (m_ViewState == MEVS_SET_FACEDIR ) )
		{
            radian3d r = {0};
            r.Y = pMotion->GetFaceDir( iCurrentFrame );
			M4_SetRotationsXYZ( &m, &r ) ;
			M4_SetTranslation( &m, &t ) ;
			pRC->PushMatrix( &m ) ;
			pRC->RENDER_Arrow( 20, RGB(0,0,255) ) ;
			pRC->PopMatrix( ) ;
		}

		if( (!m_ViewAllFaceDirs && !m_ViewAllMoveDirs) || (m_ViewState == MEVS_SET_MOVEDIR ) )
		{
            radian3d r = {0};
            r.Y = pMotion->GetMoveDir( iCurrentFrame );
			M4_SetRotationsXYZ( &m, &r ) ;
			M4_SetTranslation( &m, &t ) ;
			pRC->PushMatrix( &m ) ;
			pRC->RENDER_Arrow( 20, RGB(255,0,0) ) ;
			pRC->PopMatrix( ) ;
		}
	}

	//---	Draw the Character related items
	if( m_pCharacter )
	{
		//---	Draw Skeleton
		if( m_ViewPrimaryMotion )
			DrawSkeleton( pRC, m_pCharacter ) ;

		//---	Draw Mirrored Skeleton
		if( m_ViewMirroredSkeleton )
		{
            vector3d s = {-1,1,1};
			M4_Identity( &m ) ;
			M4_SetScale( &m, &s ) ;
			pRC->PushMatrix( &m ) ;
			DrawSkeleton( pRC, m_pCharacter ) ;
			pRC->PopMatrix( ) ;
		}

		//---	Draw Compressed Motion at this frame
		if( m_ViewCompressedMotion && GetCurMotion() )
		{
			CMotion* pCompVersion = GetCurMotion()->GetCompressedVersion();
			m_pCharacter->SetPose( pCompVersion, ((f32)m_pCharacter->GetCurFrame()*(pCompVersion->m_ExportFrameRate/60.0f)), m_ViewMotionBlended );
			DrawSkeleton( pRC, m_pCharacter );
		}

		//---	Draw Skeleton Points
		if( GetCurMotion() == NULL )
		{
			POSITION Pos = m_pCharacter->m_SkelPointList.GetHeadPosition( ) ;
			while( Pos )
			{
				//---	Get SkelPoint
				CSkelPoint *pSkelPoint = m_pCharacter->m_SkelPointList.GetNext( Pos ) ;
				ASSERT( pSkelPoint ) ;

				//---	Draw SkelPoint
				vector3d v ;
				pSkelPoint->GetPosition( &v ) ;
				M4_Identity( &m ) ;
				M4_SetTranslation( &m, &v ) ;
				pRC->PushMatrix( &m ) ;
				int c = 0 ;
				pRC->RENDER_Sphere( pSkelPoint->GetRadius(), RGB(c,c,c), RGB(c,c,c), RGB(c,c,c) ) ;
				pRC->PopMatrix( ) ;
			}
		}

		//---	Draw Linked Character if available
		CMotion *pControlMotion = GetCurMotion() ;
		if( pControlMotion )
		{
			CCharacter *pLinkCharacter = pControlMotion->m_pLinkCharacter ;
			if( pLinkCharacter )
			{
				CMotion *pMotion = pControlMotion->m_pLinkMotion ;
				if( pMotion )
				{
					//---	Calculate Frame for Linked Character
					int nFrame = (int)(m_pCharacter->GetCurFrame()+0.5f) - pMotion->GetStartFrame()-pControlMotion->GetStartFrame() ;
					if( (nFrame >= 0) && (nFrame < pMotion->GetNumFrames()) )
					{
						//---	Set Linked Character into correct pose
						pLinkCharacter->SetPose( pMotion, (f32)nFrame ) ;

						//---	Draw Skeleton
						if( m_ViewPrimaryMotion )
							DrawSkeleton( pRC, pLinkCharacter ) ;

						//---	Draw Mirrored Skeleton
						if( m_ViewMirroredSkeleton )
						{
                            vector3d s = {-1,1,1};
							M4_Identity( &m ) ;
							M4_SetScale( &m, &s ) ;
							pRC->PushMatrix( &m ) ;
							DrawSkeleton( pRC, pLinkCharacter ) ;
							pRC->PopMatrix( ) ;
						}

						//---	Draw Compressed Motion
						if( m_ViewCompressedMotion )
						{
							CMotion* pCompVersion = pMotion->GetCompressedVersion();
							pLinkCharacter->SetPose( pCompVersion, ((f32)nFrame*(pCompVersion->m_ExportFrameRate/60.0f)), m_ViewMotionBlended );
							DrawSkeleton( pRC, pLinkCharacter );
						}
					}
				}
			}
		}
	}

	//---	Draw Events for Current Motion
	if( GetCurMotion() )
	{
		CMotion *pMotion = GetCurMotion() ;
		int iCurrentFrame = (int)(m_pCharacter->GetCurFrame()+0.5f) ;

		//---	Loop through all Events
		for( int i = 0 ; i < pMotion->GetNumEvents() ; i++ )
		{
			//---	Get Event Pointer
			CEvent *pEvent = pMotion->GetEvent( i ) ;

			//---	Draw the Event
			vector3d v ;
			pEvent->GetPosition( &v ) ;
			M4_Identity( &m ) ;
			M4_SetTranslation( &m, &v ) ;
			pRC->PushMatrix( &m ) ;
			int c = (pEvent->GetFrame() == m_pCharacter->GetCurFrame()) ? 0 : 192 ;
			pRC->RENDER_Sphere( pEvent->GetRadius(), RGB(255,c,c), RGB(c,255,c), RGB(c,c,255) ) ;
			pRC->PopMatrix( ) ;
		}
	}
}

void CMotionEditView::OnDraw(CDC* pDC)
{
	//---	Ensure we have a Back Buffer
	CreateBackBuffer( pDC ) ;

	//---	Render
	if( m_pBackDC )
	{
		CRect r ;
		GetClientRect( &r ) ;
		m_pBackDC->FillSolidRect( &r, RGB(255,255,255) ) ;
		CRenderContext rc( m_pBackDC, &m_Camera ) ;
		DrawView( &rc ) ;
		pDC->BitBlt( 0, 0, r.Width(), r.Height(), m_pBackDC, 0, 0, SRCCOPY ) ;
	}
	else
	{
		CRect r ;
		GetClientRect( &r ) ;
		pDC->FillSolidRect( &r, RGB(255,255,255) ) ;
		CRenderContext rc( pDC, &m_Camera ) ;
		DrawView( &rc ) ;
	}

//	m_nTimerMessages = 0 ;
/*	if( m_Playing )
	{
		MSG		Message ;
		if( !::PeekMessage( &Message, GetSafeHwnd(), 0, 0, PM_NOREMOVE ) )
		{
			::PostMessage( (HWND)GetSafeHwnd(), WM_TIMER, 0, 0 ) ;
		}
	}
*/
}

CMotionEditFrame *CMotionEditView::GetFrame( )
{
	CMotionEditFrame *pFrame = (CMotionEditFrame*)GetParent( ) ;
	ASSERT( pFrame ) ;
	return pFrame ;
}

void CMotionEditView::UpdateMenuItems( CMenu *pMenu )
{
	if( pMenu )
	{
		pMenu->CheckMenuItem( ID_VIEW_PRIMARY_MOTION, m_ViewPrimaryMotion ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem( ID_VIEW_COMPRESSED_MOTION, m_ViewCompressedMotion ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem( ID_VIEW_MOTION_BLENDED, m_ViewMotionBlended ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem (ID_VIEW_MIRRORED_SKELETON, m_ViewMirroredSkeleton ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem (ID_VIEW_ALL_FACEDIRS, m_ViewAllFaceDirs ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem (ID_VIEW_ALL_MOVEDIRS, m_ViewAllMoveDirs ? MF_CHECKED : MF_UNCHECKED ) ;
	}
}

CMotion *CMotionEditView::GetCurMotion( )
{
	CMotion *pMotion = NULL ;

	if( m_pCharacter && m_pCharacter->GetCurMotion() )
		pMotion = m_pCharacter->GetCurMotion() ;

	return pMotion ;
}


/////////////////////////////////////////////////////////////////////////////
// CMotionEditView diagnostics

#ifdef _DEBUG
void CMotionEditView::AssertValid() const
{
	CView::AssertValid();
}

void CMotionEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMotionEditView message handlers

BOOL CMotionEditView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default

	return 0 ;
//	return CView::OnEraseBkgnd(pDC);
}

void CMotionEditView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	SetCapture() ;

	CPoint	Point ;
	GetCursorPos( &Point ) ;
	ScreenToClient( &Point ) ;
	m_MouseLast = Point ;
	OnMouseMove( nFlags, Point ) ;

	CView::OnLButtonDown(nFlags, point);
}

void CMotionEditView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture() ;

	CView::OnLButtonUp(nFlags, point);
}

void CMotionEditView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	SetCapture() ;

	CPoint	Point ;
	GetCursorPos( &Point ) ;
	ScreenToClient( &Point ) ;
	m_MouseLast = Point ;
	OnMouseMove( nFlags, Point ) ;

	CView::OnRButtonDown(nFlags, point);
}

void CMotionEditView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture() ;

	CView::OnRButtonUp(nFlags, point);
}

void CMotionEditView::UpdateCharacterList()
{
	CCeDDoc *pDoc = GetDocument() ;
	ASSERT( pDoc ) ;
	CMotionEditFrame *pFrame = GetFrame() ;
	ASSERT( pFrame ) ;
	CDlgCharacter *pDlgCharacter = &pFrame->m_DlgCharacter ;
	ASSERT( pDlgCharacter ) ;

	//---	Reset List of Characters
	pDlgCharacter->Reset( ) ;

	//---	Add all Characters to Combo
	int	CurrentSel = 0 ;
	for( int i = 0 ; i < pDoc->m_CharacterList.GetCount() ; i++ )
	{
		CCharacter *pCharacter = (CCharacter*)pDoc->m_CharacterList.IndexToPtr( i ) ;
		ASSERT( pCharacter ) ;
		pDlgCharacter->AddCharacter( pCharacter ) ;
	}

	//---	Set Current Selection
	pDlgCharacter->SelectCharacter( m_pCharacter ) ;
}

void CMotionEditView::UpdateCameraControls( )
{
	CString	OldStr, NewStr ;

	CMotionEditFrame *pFrame = GetFrame() ;
	ASSERT( pFrame ) ;

	//---	Get Current Target
	vector3d	v ;
	m_Camera.GetTarget( &v ) ;

	pFrame->m_DlgCamera.SetTarget( &v ) ;
	pFrame->m_DlgCamera.SetRotation( m_Camera.GetRotation() ) ;
	pFrame->m_DlgCamera.SetElevation( m_Camera.GetElevation() ) ;

/*	//---	Lock Edit Controls to stop infinite loop
	bool CameraControlEditsLocked = m_CameraControlEditsLocked ;
	m_CameraControlEditsLocked = true ;

	//---	Set Strings into Controls
	pDialogBar->GetDlgItemText( IDC_CAMERA_TARGETX, OldStr ) ;
	if( ((f32)atof( OldStr ) != v.X) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%.03f", v.X ) ;
		pDialogBar->SetDlgItemText( IDC_CAMERA_TARGETX, NewStr ) ;
		pDialogBar->GetDlgItem( IDC_CAMERA_TARGETX )->RedrawWindow() ;
	}

	pDialogBar->GetDlgItemText( IDC_CAMERA_TARGETY, OldStr ) ;
	if( ((f32)atof( OldStr ) != v.Y) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%.03f", v.Y ) ;
		pDialogBar->SetDlgItemText( IDC_CAMERA_TARGETY, NewStr ) ;
		pDialogBar->GetDlgItem( IDC_CAMERA_TARGETY )->RedrawWindow() ;
	}

	pDialogBar->GetDlgItemText( IDC_CAMERA_TARGETZ, OldStr ) ;
	if( ((f32)atof( OldStr ) != v.Z) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%.03f", v.Z ) ;
		pDialogBar->SetDlgItemText( IDC_CAMERA_TARGETZ, NewStr ) ;
		pDialogBar->GetDlgItem( IDC_CAMERA_TARGETZ )->RedrawWindow() ;
	}

	pDialogBar->GetDlgItemText( IDC_CAMERA_ROTATION, OldStr ) ;
	if( (DEG_TO_RAD((f32)atof( OldStr )) != m_Camera.GetRotation()) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%.03f", RAD_TO_DEG(m_Camera.GetRotation()) ) ;
		pDialogBar->SetDlgItemText( IDC_CAMERA_ROTATION, NewStr ) ;
		pDialogBar->GetDlgItem( IDC_CAMERA_ROTATION )->RedrawWindow() ;
	}

	pDialogBar->GetDlgItemText( IDC_CAMERA_ELEVATION, OldStr ) ;
	if( (DEG_TO_RAD((f32)atof( OldStr )) != m_Camera.GetElevation()) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%.03f", RAD_TO_DEG(m_Camera.GetElevation()) ) ;
		pDialogBar->SetDlgItemText( IDC_CAMERA_ELEVATION, NewStr ) ;
		pDialogBar->GetDlgItem( IDC_CAMERA_ELEVATION )->RedrawWindow() ;
	}

	//---	Enable Updates again
	m_CameraControlEditsLocked = CameraControlEditsLocked ;
*/
}

void CMotionEditView::UpdateCameraBoneList()
{
	CMotionEditFrame *pFrame = GetFrame() ;
	ASSERT( pFrame ) ;

	//---	Reset the Combo & Add all Bones
	pFrame->m_DlgCamera.Reset( ) ;
	pFrame->m_DlgCamera.EnableBoneList( m_pCharacter != NULL ) ;

	if( m_pCharacter != NULL )
	{
		CSkeleton *pSkel = m_pCharacter->GetSkeleton() ;
		if( pSkel )
		{
			for( int i = 0 ; i < pSkel->GetNumBones() ; i++ )
			{
				CString	s ;
				for( int j = 0 ; j < pSkel->GetBoneNumParents( pSkel->GetBoneFromIndex(i) ) ; j++ )
				{
					s += "  " ;
				}
				s += pSkel->GetBoneFromIndex(i)->BoneName ;

				pFrame->m_DlgCamera.AddBone( s ) ;
			}
		}
	}

	//---	Set Current Selection
	pFrame->m_DlgCamera.SelectBone( m_CameraLockedBone ) ;
}

void CMotionEditView::UpdateMotionControls( )
{
	int Frame = 0 ;

	CMotionEditFrame *pFrame = GetFrame() ;

	//---	Update AnimBar Control
	CMotion *pMotion = NULL ;
	if( m_pCharacter )
	{
		pMotion = GetCurMotion() ;
		Frame = (int)(m_pCharacter->GetCurFrame()+0.5f) ;
	}

	pFrame->m_wndAnimBar.SetMotion( pMotion ) ;
	pFrame->m_wndAnimBar.SetFrame( Frame ) ;
	pFrame->m_DlgMotionList.UpdateFaceMoveDir( ) ;
	pFrame->m_DlgMotionList.UpdateMotionSymbol( ) ;
	pFrame->m_DlgMotionList.UpdateAutoResetFaceDir();
	pFrame->m_DlgMotionList.UpdateAutoResetMoveDir();
//	pFrame->m_DlgMotionList.UpdateExportInfo();
}

void CMotionEditView::UpdateMotionList()
{
	CMotionEditFrame *pFrame = GetFrame() ;
	pFrame->m_DlgMotionList.UpdateMotionList( m_pCharacter ) ;
	pFrame->m_DlgMotionList.UpdateLinkCharacterList( GetDocument(), GetCurMotion() ) ;
	pFrame->m_DlgMotionList.UpdateLinkMotionList( GetCurMotion() ) ;
	pFrame->m_DlgMotionList.UpdateFaceMoveDir( ) ;
	pFrame->m_DlgMotionList.UpdateMotionSymbol( ) ;
	pFrame->m_DlgMotionList.UpdateExportInfo();
}

void CMotionEditView::MotionSetCurrent( CMotion *pMotion )
{
	// TODO: Add your command handler code here

	if( m_pCharacter )
	{
		m_pCharacter->SetCurMotion( pMotion ) ;
		CMotionEditFrame *pFrame = GetFrame() ;
		pFrame->m_DlgMotionList.UpdateLinkCharacterList( GetDocument(), GetCurMotion() ) ;
		pFrame->m_DlgMotionList.UpdateLinkMotionList( GetCurMotion() ) ;
		pFrame->m_DlgMotionList.UpdateFaceMoveDir( ) ;
		pFrame->m_DlgMotionList.UpdateMotionSymbol( ) ;
		pFrame->m_DlgMotionList.UpdateExportInfo();
		UpdateMotionControls() ;
		UpdateEventList() ;
		UpdateEventControls() ;
		UpdateSkelPointList() ;
		UpdateSkelPointControls() ;
		Invalidate() ;
	}
}

void CMotionEditView::MotionAdd( ) 
{
	OnCharacterImportMotion() ;
}

void CMotionEditView::MotionDelete( CMotion *pMotion, CMotion *pNewCurrentMotion, int PromptUser) 
{
	int	piResult	= 1;
	ASSERT( pMotion ) ;

	if (PromptUser == 1)
	{
		int Sure = MessageBox( "Are You Sure?", "Delete Motion", MB_ICONWARNING|MB_YESNO ) ;
		if( Sure != IDYES )
		{
			piResult	= 0;
		}
	}

	if( piResult == 1 )
	{
		m_pCharacter->DeleteMotion( pMotion ) ;

		m_pCharacter->SetCurMotion( pNewCurrentMotion ) ;

		GetDocument()->MotionDeleted( pMotion ) ;

		GetDocument()->UpdateAllViews( NULL, HINT_MOTION_DELETE, m_pCharacter ) ;
	}
}

void CMotionEditView::MotionModify( CMotion *pMotion )
{
	if( pMotion->Modify() )
	{
		GetDocument()->UpdateAllViews( NULL ) ;//, HINT_MOTION_ADD, m_pCharacter ) ;
		GetDocument()->SetModifiedFlag();
	}
}

void CMotionEditView::MotionSetLinkCharacter( CCharacter *pCharacter ) 
{
	CMotion *pMotion = GetCurMotion() ;
	if( pMotion )
	{
		if( pMotion->m_pLinkCharacter != pCharacter )
		{
			pMotion->m_pLinkCharacter = pCharacter ;
			pMotion->m_pLinkMotion = NULL ;
			CMotionEditFrame *pFrame = GetFrame() ;
			pFrame->m_DlgMotionList.UpdateLinkCharacterList( GetDocument(), GetCurMotion() ) ;
			pFrame->m_DlgMotionList.UpdateLinkMotionList( GetCurMotion() ) ;
			RedrawWindow() ;
		}
	}
}

void CMotionEditView::MotionSetLinkMotion( CMotion *pLinkMotion ) 
{
	CMotion *pMotion = GetCurMotion() ;
	if( pMotion )
	{
		if( pMotion->m_pLinkMotion != pLinkMotion )
		{
			pMotion->m_pLinkMotion = pLinkMotion ;
			RedrawWindow() ;
		}
	}
}






void CMotionEditView::UpdateEventList()
{
	CMotionEditFrame *pFrame = GetFrame() ;
	pFrame->m_DlgEventList.UpdateEventList( GetCurMotion() ) ;
}

void CMotionEditView::UpdateEventControls( )
{
	CMotionEditFrame *pFrame = GetFrame() ;
	pFrame->m_DlgEventList.UpdateEventControls( GetCurMotion() ) ;
}

void CMotionEditView::EventSetCurrent( CEvent *pEvent )
{
	CMotion *pMotion = GetCurMotion( ) ;
	if( pMotion )
	{
		pMotion->SetCurEvent( pEvent ) ;
		if( pEvent )
		{
			m_pCharacter->SetCurFrame( (f32)pEvent->GetFrame() ) ;
		}
		UpdateEventControls() ;
		Invalidate() ;
	}
}

void CMotionEditView::EventAdd( ) 
{
	if( GetCurMotion() )
	{
		CEvent *pEvent = GetCurMotion()->AddEvent() ;
		ASSERT( pEvent ) ;
		vector3d	v ;
		m_Camera.GetTarget( &v ) ;
		pEvent->SetPosition( &v ) ;
		pEvent->SetFrame( (int)(m_pCharacter->GetCurFrame()+0.5f) ) ;

		GetDocument()->SetModifiedFlag( TRUE ) ;
		GetDocument()->UpdateAllViews( NULL, HINT_EVENT_NEW ) ;
	}
}

void CMotionEditView::EventDelete( CEvent *pEvent, CEvent *pNewCurrentEvent )
{
	int Sure = MessageBox( "Are You Sure?", "Delete Event", MB_ICONWARNING|MB_YESNO ) ;
	if( Sure == IDYES )
	{
		CMotion *pMotion = GetCurMotion() ;
		if( pMotion )
		{
			pMotion->DeleteEvent( pEvent ) ;
			pMotion->SetCurEvent( pNewCurrentEvent ) ;

			GetDocument()->SetModifiedFlag( TRUE ) ;
			GetDocument()->UpdateAllViews( NULL, HINT_EVENT_DELETE, pMotion ) ;
		}
	}
}

void CMotionEditView::ModifyEventPosition( f32 dx, f32 dy, f32 dz )
{
	if( GetCurMotion() )
	{
		CEvent *pEvent = GetCurMotion()->GetCurEvent() ;
		if( pEvent )
		{
			vector3d v ;
			pEvent->GetPosition( &v ) ;
			v.X += dx ;
			v.Y += dy ;
			v.Z += dz ;
			pEvent->SetPosition( &v ) ;
			GetDocument()->SetModifiedFlag( TRUE ) ;
			UpdateEventControls() ;
			RedrawWindow() ;
		}
	}
}

void CMotionEditView::ModifyEventRadius( f32 dr )
{
	if( GetCurMotion() )
	{
		CEvent *pEvent = GetCurMotion()->GetCurEvent() ;
		if( pEvent )
		{
			pEvent->SetRadius( pEvent->GetRadius() + dr ) ;
			GetDocument()->SetModifiedFlag( TRUE ) ;
			UpdateEventControls() ;
			RedrawWindow() ;
		}
	}
}



void CMotionEditView::UpdateSkelPointList()
{
	CMotionEditFrame *pFrame = GetFrame() ;
	pFrame->m_DlgSkelPointList.UpdateList( m_pCharacter ) ;
}

void CMotionEditView::UpdateSkelPointControls( )
{
	CMotionEditFrame *pFrame = GetFrame() ;
	pFrame->m_DlgSkelPointList.UpdateControls( m_pCharacter ) ;
}

void CMotionEditView::SkelPointSetCurrent( CSkelPoint *pSkelPoint )
{
	if( m_pCharacter )
	{
		m_pCharacter->SetCurSkelPoint( pSkelPoint ) ;
		UpdateSkelPointControls() ;
		Invalidate() ;
	}
}

void CMotionEditView::SkelPointAdd( ) 
{
	if( m_pCharacter )
	{
		CSkelPoint *pSkelPoint = m_pCharacter->AddSkelPoint() ;
		ASSERT( pSkelPoint ) ;
		vector3d	v ;
		m_Camera.GetTarget( &v ) ;
		pSkelPoint->SetPosition( &v ) ;
		pSkelPoint->SetBone( m_CameraLockedBone ) ;

		GetDocument()->SetModifiedFlag( TRUE ) ;
		GetDocument()->UpdateAllViews( NULL, HINT_SKELPOINT_NEW ) ;
	}
}

void CMotionEditView::SkelPointDelete( CSkelPoint *pSkelPoint, CSkelPoint *pNewCurrentSkelPoint )
{
	int Sure = MessageBox( "Are You Sure?", "Delete Skeleton Point", MB_ICONWARNING|MB_YESNO ) ;
	if( Sure == IDYES )
	{
		if( m_pCharacter )
		{
			m_pCharacter->DeleteSkelPoint( pSkelPoint ) ;
			m_pCharacter->SetCurSkelPoint( pNewCurrentSkelPoint ) ;

			GetDocument()->SetModifiedFlag( TRUE ) ;
			GetDocument()->UpdateAllViews( NULL, HINT_SKELPOINT_DELETE, m_pCharacter ) ;
		}
	}
}

void CMotionEditView::ModifySkelPointPosition( f32 dx, f32 dy, f32 dz )
{
	if( m_pCharacter )
	{
		CSkelPoint *pSkelPoint = m_pCharacter->GetCurSkelPoint() ;
		if( pSkelPoint )
		{
			vector3d v ;
			pSkelPoint->GetPosition( &v ) ;
			v.X += dx ;
			v.Y += dy ;
			v.Z += dz ;
			pSkelPoint->SetPosition( &v ) ;
			GetDocument()->SetModifiedFlag( TRUE ) ;
			UpdateSkelPointControls() ;
			RedrawWindow() ;
		}
	}
}

void CMotionEditView::ModifySkelPointRadius( f32 dr )
{
	if( m_pCharacter )
	{
		CSkelPoint *pSkelPoint = m_pCharacter->GetCurSkelPoint() ;
		if( pSkelPoint )
		{
			pSkelPoint->SetRadius( pSkelPoint->GetRadius() + dr ) ;
			GetDocument()->SetModifiedFlag( TRUE ) ;
			UpdateSkelPointControls() ;
			RedrawWindow() ;
		}
	}
}













void CMotionEditView::SetCharacter( CCharacter *pCharacter ) 
{
	m_pCharacter = pCharacter ;

	UpdateCameraControls( ) ;
	UpdateCameraBoneList() ;
	UpdateMotionControls() ;
	UpdateMotionList() ;
	UpdateEventList() ;
	UpdateEventControls() ;
	UpdateSkelPointList() ;
	UpdateSkelPointControls() ;
	UpdateEventProjectEvents();

	Invalidate() ;
}

void CMotionEditView::SetCameraBone( int iBone )
{
	m_CameraLockedBone = iBone ;
	Invalidate() ;
}

void CMotionEditView::SetTimeAccuratePlayback( int State )
{
	m_StartTiming = 1 ;
	m_TimeAccuratePlay = !!State ;
}

















CEvent *CMotionEditView::PickGetEvent( CPoint p )
{
	CEvent *pFound = NULL ;

	//---	Get Ray Base and Vector
	vector3d RayBase, Ray ;
	GetCursorRay( &RayBase, &Ray, p ) ;

	//---	Only proceed with Valid Motion
	CMotion *pMotion = GetCurMotion( ) ;
	if( pMotion )
	{
		//---	Check each Event for intersection
		for( int i = 0 ; i < pMotion->GetNumEvents() ; i++ )
		{
			CEvent *pEvent = pMotion->GetEvent( i ) ;
			ASSERT( pEvent ) ;
			vector3d Center ;
			pEvent->GetPosition( &Center ) ;
			if( IntersectRaySphere( &RayBase, &Ray, &Center, pEvent->GetRadius() ) )
			{
				pFound = pEvent ;
			}
		}
	}

	return pFound ;
}


void CMotionEditView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CCeDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	//---	Handle Delta Mouse Movement
	m_MouseDelta = point - m_MouseLast ;
	m_MouseLast = point ;

	//---	Check for Events
	m_pEventUnderMouse = PickGetEvent( point ) ;

	//---	Handle SELECT
	if( (m_ViewState == MEVS_SELECT) && (GetCapture() == this) )
	{
	}

	//---	Handle SET FACEDIR
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_SET_FACEDIR) && (GetCapture() == this) )
	{
		vector3d	Plane ;
		V3_Set( &Plane, 0, 0, 0 ) ;

		//---	Get ROOT Bone position
		if( m_pCharacter && m_pCharacter->GetSkeleton() )
			M4_GetTranslation( m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0), &Plane ) ;

		//---	Get Point on Ground Plane given Cursor position
		vector3d v ;
		if( GetPointOnPlane( &v, point, Plane.Y ) )
		{
			//---	Make Relative to ROOT Bone
			V3_Sub( &v, &v, &Plane ) ;

			//---	Calculate Rotation of FaceDir Vector from Origin to v
			if( GetCurMotion() )
			{
				GetCurMotion()->SetFaceDir( (int)(m_pCharacter->GetCurFrame()+0.5f), x_atan2( v.X, v.Z ) ) ;
				pDoc->SetModifiedFlag() ;
				Invalidate() ;
			}
		}
	}

	//---	Handle SET MOVEDIR
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_SET_MOVEDIR) && (GetCapture() == this) )
	{
		vector3d	Plane ;
		V3_Set( &Plane, 0, 0, 0 ) ;

		//---	Get ROOT Bone position
		if( m_pCharacter && m_pCharacter->GetSkeleton() )
			M4_GetTranslation( m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0), &Plane ) ;

		//---	Get Point on Ground Plane given Cursor position
		vector3d v ;
		if( GetPointOnPlane( &v, point, Plane.Y ) )
		{
			//---	Make Relative to ROOT Bone
			V3_Sub( &v, &v, &Plane ) ;

			//---	Calculate Rotation of MoveDir Vector from Origin to v
			if( GetCurMotion() )
			{
				GetCurMotion()->SetMoveDir( (int)(m_pCharacter->GetCurFrame()+0.5f), x_atan2( v.X, v.Z ) ) ;
				pDoc->SetModifiedFlag() ;
				Invalidate() ;
			}
		}
	}

	//---	Handle MOVE EVENT X/Z
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_SET_EVENT) && (GetCapture() == this) )
	{
		vector3d	Plane ;
		V3_Set( &Plane, 0, 0, 0 ) ;

		//---	Get ROOT Bone position
		if( m_pCharacter && m_pCharacter->GetSkeleton() )
			M4_GetTranslation( m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0), &Plane ) ;

		//---	Get Point on Ground Plane given Cursor position
		vector3d v ;
		if( GetPointOnPlane( &v, point, 0.0f ) )
		{
			//---	Make Relative to ROOT Bone
			V3_Sub( &v, &v, &Plane ) ;

			//---	Calculate Rotation of MoveDir Vector from Origin to v
			if( GetCurMotion() && GetCurMotion()->GetCurEvent() )
			{
				vector3d cv;
				GetCurMotion()->GetCurEvent()->GetPosition( &cv );
				cv.X = v.X;
				cv.Z = v.Z;
				GetCurMotion()->GetCurEvent()->SetPosition( &cv );
				UpdateEventControls();
				pDoc->SetModifiedFlag() ;
				Invalidate() ;
			}
		}
	}

	//---	Handle MOVE EVENT Y
	if( (nFlags & MK_RBUTTON) && (m_ViewState == MEVS_SET_EVENT) && (GetCapture() == this) )
	{
		if( GetCurMotion() && GetCurMotion()->GetCurEvent() )
		{
			vector3d cv;
			GetCurMotion()->GetCurEvent()->GetPosition( &cv );
			cv.Y -= (f32)m_MouseDelta.y/5.0f;
			GetCurMotion()->GetCurEvent()->SetPosition( &cv );
			UpdateEventControls();
			Invalidate() ;
		}
	}

	//---	Handle MOVEPLANEY
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_MOVEPLANEY) && (GetCapture() == this) )
	{
		GetDocument()->m_ConPlanePos.Y -= (f32)m_MouseDelta.y / 20 ;
		Invalidate() ;
	}

	//---	Handle PAN
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_CAMERA_PAN) && (GetCapture() == this) )
	{
		matrix4		m ;
		vector3d	moveSrc ;
		vector3d	moveDst ;
		vector3d	Target ;

		M4_Identity( &m ) ;
		M4_RotateYOn( &m, -m_Camera.GetRotation() ) ;

		V3_Set( &moveSrc, (f32)m_MouseDelta.x, 0, (f32)m_MouseDelta.y ) ;
		V3_Scale( &moveSrc, &moveSrc, m_Camera.GetDistance() / 1000 ) ;
		M4_TransformVerts( &m, &moveDst, &moveSrc, 1 ) ;

		m_Camera.GetTarget( &Target ) ;
		V3_Add( &Target, &Target, &moveDst ) ;
		m_Camera.SetTarget( &Target ) ;

		m_Camera.ComputeMatrix() ;
		UpdateCameraControls( ) ;

		Invalidate() ;
	}

	//---	Handle ROTATE
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_CAMERA_ROTATE) && (GetCapture() == this) )
	{
		m_Camera.SetRotation( m_Camera.GetRotation() + (radian)m_MouseDelta.x / 200 ) ;
		m_Camera.SetElevation( m_Camera.GetElevation() - (radian)m_MouseDelta.y / 200 ) ;

		m_Camera.ComputeMatrix() ;
		UpdateCameraControls( ) ;
		
		Invalidate() ;
	}

	//---	Handle ZOOM
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_CAMERA_ZOOM) && (GetCapture() == this) )
	{
		m_Camera.SetDistance( m_Camera.GetDistance() + (f32)m_MouseDelta.y ) ;
		if( m_Camera.GetDistance() < 10 ) m_Camera.SetDistance( 10 ) ;

		m_Camera.ComputeMatrix() ;
		UpdateCameraControls( ) ;

		Invalidate() ;
	}

	CView::OnMouseMove(nFlags, point);
}

void CMotionEditView::UpdateCursorState( )
{
	switch( m_ViewState )
	{
	case MEVS_SELECT:
		if( m_pEventUnderMouse )
			SetCursor (AfxGetApp()->LoadCursor( IDC_INVERSE )) ;
		else
			SetCursor (AfxGetApp()->LoadStandardCursor( IDC_ARROW )) ;
		break ;
	case MEVS_CAMERA_PAN:
		SetCursor (AfxGetApp()->LoadCursor( IDC_PAN )) ;
		break ;
	case MEVS_CAMERA_ROTATE:
		SetCursor (AfxGetApp()->LoadCursor( IDC_ROTATE )) ;
		break ;
	case MEVS_CAMERA_ZOOM:
		SetCursor (AfxGetApp()->LoadCursor( IDC_ZOOM )) ;
		break ;
	case MEVS_MOVEPLANEY:
		SetCursor (AfxGetApp()->LoadCursor( IDC_MOVEPLANEY )) ;
		break ;
	case MEVS_SET_FACEDIR:
		SetCursor (AfxGetApp()->LoadCursor( IDC_FACEDIR )) ;
		break ;
	case MEVS_SET_MOVEDIR:
		SetCursor (AfxGetApp()->LoadCursor( IDC_MOVEDIR )) ;
		break ;
	case MEVS_SET_EVENT:
		SetCursor (AfxGetApp()->LoadCursor( IDC_EVENT )) ;
		break ;
	} ;
}

BOOL CMotionEditView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default

	//---	Set the Cursor
	if( nHitTest == HTCLIENT )
	{
		UpdateCursorState( ) ;
		return 0 ;
	}

	return CView::OnSetCursor(pWnd, nHitTest, message) ;
}

bool CMotionEditView::GetPointOnPlane( vector3d *pPoint, CPoint p, f32 yPlane ) 
{
	bool		Intersected = false ;

	//---	Get Ray from Camera through Cursor
	vector3d RayBase, Ray ;
	GetCursorRay( &RayBase, &Ray, p ) ;

	//---	Get Point RAY intersects on plane or return Zero if can't intersect
	if( (x_fabs(Ray.Y) > 0.01) )
	{
		f32	d = (RayBase.Y-yPlane) / -Ray.Y ;
		if( d >= 0 )
		{
			V3_Scale( &Ray, &Ray, d ) ;
			V3_Add( pPoint, &RayBase, &Ray ) ;
			Intersected = true ;
		}
	}
	else
	{
		V3_Zero( pPoint ) ;
	}

	return Intersected ;
}

void CMotionEditView::SetViewState( int ViewState )
{
	m_ViewState = ViewState ;

	CMotionEditFrame *pFrame = GetFrame() ;
	CToolBarCtrl& ToolBar = pFrame->m_wndToolBar.GetToolBarCtrl( ) ;

	ToolBar.IsButtonEnabled( ID_STATE_FACEDIR );

	ToolBar.CheckButton( ID_STATE_SELECT,		m_ViewState == MEVS_SELECT ) ;
	ToolBar.CheckButton( ID_STATE_PAN,			m_ViewState == MEVS_CAMERA_PAN ) ;
	ToolBar.CheckButton( ID_STATE_ROTATE,		m_ViewState == MEVS_CAMERA_ROTATE ) ;
	ToolBar.CheckButton( ID_STATE_ZOOM,			m_ViewState == MEVS_CAMERA_ZOOM ) ;
	ToolBar.CheckButton( ID_STATE_MOVEPLANEY,	m_ViewState == MEVS_MOVEPLANEY ) ;
	ToolBar.CheckButton( ID_STATE_FACEDIR,		m_ViewState == MEVS_SET_FACEDIR ) ;
	ToolBar.CheckButton( ID_STATE_MOVEDIR,		m_ViewState == MEVS_SET_MOVEDIR ) ;
	ToolBar.CheckButton( ID_STATE_EVENT,		m_ViewState == MEVS_SET_EVENT ) ;

	UpdateCursorState() ;
	Invalidate() ;
}

void CMotionEditView::OnStateSelect() 
{
	// TODO: Add your command handler code here
	SetViewState( MEVS_SELECT ) ;	
}

void CMotionEditView::OnStatePan() 
{
	// TODO: Add your command handler code here
	SetViewState( MEVS_CAMERA_PAN ) ;	
}

void CMotionEditView::OnStateRotate() 
{
	// TODO: Add your command handler code here
	SetViewState( MEVS_CAMERA_ROTATE ) ;	
}

void CMotionEditView::OnStateZoom() 
{
	// TODO: Add your command handler code here
	SetViewState( MEVS_CAMERA_ZOOM ) ;	
}

void CMotionEditView::OnZoomAll() 
{
	// TODO: Add your command handler code here
	
}

void CMotionEditView::OnStateSelectmove() 
{
	// TODO: Add your command handler code here
	
}

void CMotionEditView::OnStateSelectrotate() 
{
	// TODO: Add your command handler code here
	
}

void CMotionEditView::OnAnimGotoStart() 
{
	// TODO: Add your command handler code here
	if( m_pCharacter )
		m_pCharacter->AnimGotoStart() ;
	Invalidate() ;
}

void CMotionEditView::OnAnimPreviousFrame() 
{
	// TODO: Add your command handler code here
	if( m_pCharacter )
		m_pCharacter->AnimPreviousFrame() ;
	Invalidate() ;
}

void CMotionEditView::OnAnimPlay() 
{
	// TODO: Add your command handler code here
	m_StartTiming = true ;
    m_nTimerMessages = 0 ;
	m_Playing = !m_Playing ;
	Invalidate() ;
}

void CMotionEditView::OnAnimNextFrame() 
{
	// TODO: Add your command handler code here
	if( m_pCharacter )
		m_pCharacter->AnimNextFrame() ;
	Invalidate() ;
}

void CMotionEditView::OnAnimGotoEnd() 
{
	// TODO: Add your command handler code here
	if( m_pCharacter )
		m_pCharacter->AnimGotoEnd() ;
	Invalidate() ;
}

static void CALLBACK MMTimerCallback( UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 )
{
	CMotionEditView *pView = (CMotionEditView*)dwUser ;

	if( pView->m_Playing && (pView->m_nTimerMessages == 0) )
	{
		::PostMessage( (HWND)pView->GetSafeHwnd(), WM_TIMER, 0, 0 ) ;
		pView->m_nTimerMessages++ ;
	}
}

void CMotionEditView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class

	//---	Grab Character Pointer from Frame and set View Pointers in all Dialogs
	CMotionEditFrame *pFrame = GetFrame( ) ;
	ASSERT( pFrame ) ;
	m_pCharacter = pFrame->m_pCharacter ;
	pFrame->m_pView = this ;
	pFrame->m_DlgCharacter.m_pView = this ;
	pFrame->m_DlgCamera.m_pView = this ;
	pFrame->m_DlgMotionList.m_pView = this ;
	pFrame->m_DlgEventList.m_pView = this ;
	pFrame->m_DlgSkelPointList.m_pView = this ;

	//---	Set AnimControlBar to point to this View
	GetFrame()->m_wndAnimBar.SetView( this ) ;

	SetViewState( MEVS_CAMERA_ROTATE ) ;	
	UpdateCharacterList() ;
	UpdateCameraControls( ) ;
	UpdateCameraBoneList() ;
	UpdateMotionControls() ;
	UpdateMotionList() ;
	UpdateEventList() ;
	UpdateEventControls() ;
	UpdateSkelPointList() ;
	UpdateSkelPointControls() ;

	UpdateEventProjectEvents();

	//---	Setup Multimedia Timer
	m_TimerEvent = timeSetEvent( 17, 1, MMTimerCallback, (DWORD)this, TIME_CALLBACK_FUNCTION | TIME_PERIODIC ) ;
    ASSERT( m_TimerEvent ) ;

	m_bInitialUpdate = TRUE;
//	SetTimer( 1, 16, NULL ) ;
}

void CMotionEditView::OnStateMoveplaney() 
{
	// TODO: Add your command handler code here
	SetViewState( MEVS_MOVEPLANEY ) ;
}

void CMotionEditView::OnStateFacedir() 
{
	// TODO: Add your command handler code here
	SetViewState( MEVS_SET_FACEDIR ) ;	
}

void CMotionEditView::OnStateMovedir() 
{
	// TODO: Add your command handler code here
	SetViewState( MEVS_SET_MOVEDIR ) ;	
}

void CMotionEditView::OnStateEvent() 
{
	// TODO: Add your command handler code here
	SetViewState( MEVS_SET_EVENT ) ;	
}

static s32 GetTimeMS( )
{
	MMTIME	mmt ;
	mmt.wType = TIME_MS ;
	timeGetSystemTime( &mmt, sizeof(MMTIME) ) ;
	return mmt.u.ms ;
}

void CMotionEditView::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default

	if( m_pCharacter )
	{
		if( m_Playing && (m_nTimerMessages > 0) )
		{
			if( m_TimeAccuratePlay )
			{
				//---	Initialize Timing
				if( m_StartTiming )
				{
					m_StartTiming = false ;
					m_LastTime = GetTimeMS( ) ;
					m_LastFrac = 0 ;
				}

				//---	Play according to elapsed time
				s32 delta = GetTimeMS( ) - m_LastTime ;
				m_LastTime = GetTimeMS( ) ;
				delta += (s32)m_LastFrac ;
				int nFrames = (int)(delta / (1000.0/60.0)) ;
				if( nFrames > 32 ) nFrames = 32 ;
				for( int i = 0 ; i < nFrames ; i++ )
				{
					m_pCharacter->AnimNextFrame() ;
				}
				m_LastFrac = (f32)(delta - (nFrames * (1000.0/60.0))) ;
				Invalidate() ;
			}
			else
			{
				//---	Play a Frame at a Time
				m_pCharacter->AnimNextFrame() ;
				Invalidate() ;
			}
			m_nTimerMessages-- ;
		}
	}

	CView::OnTimer(nIDEvent);
}

void CMotionEditView::OnCharacterImportSkel() 
{
	GetDocument()->CharacterImportSkeleton( m_pCharacter );
}

void CMotionEditView::OnCharacterImportMotion() 
{
	GetDocument()->CharacterImportMotion( m_pCharacter );
}

void CMotionEditView::OnCharacterLoadmotions() 
{
	GetDocument()->CharacterLoadAllMotions( m_pCharacter );
}

void CMotionEditView::OnClkFrameStart( )
{
	GetFrame()->m_wndAnimBar.SetSelStart( ) ;
}

void CMotionEditView::OnClkFrameEnd( )
{
	GetFrame()->m_wndAnimBar.SetSelEnd( ) ;
}

void CMotionEditView::OnFaceDirInterpolate( )
{
	int		Start, End ;

	if( GetCurMotion() )
	{
		GetFrame()->m_wndAnimBar.GetSelection( &Start, &End ) ;
		if( Start < End )
		{
			radian StartDir	= GetCurMotion()->GetFaceDir( Start ) ;
			radian EndDir	= GetCurMotion()->GetFaceDir( End ) ;
			radian DeltaDir = EndDir - StartDir ;

			while( DeltaDir > DEG_TO_RAD( 180) ) DeltaDir -= DEG_TO_RAD(360) ;
			while( DeltaDir < DEG_TO_RAD(-180) ) DeltaDir += DEG_TO_RAD(360) ;

			int		Delta = End-Start ;
			for( int i = 1 ; i < Delta ; i++ )
			{
				radian FaceDir = StartDir+(DeltaDir)*i/Delta ;
				GetCurMotion()->SetFaceDir( Start+i, FaceDir ) ;
				GetDocument()->SetModifiedFlag( TRUE ) ;
			}
			Invalidate() ;
		}
	}
}

void CMotionEditView::OnMoveDirInterpolate( )
{
	int		Start, End ;

	if( GetCurMotion() )
	{
		GetFrame()->m_wndAnimBar.GetSelection( &Start, &End ) ;
		if( Start < End )
		{
			radian StartDir	= GetCurMotion()->GetMoveDir( Start ) ;
			radian EndDir	= GetCurMotion()->GetMoveDir( End ) ;
			radian DeltaDir = EndDir - StartDir ;

			while( DeltaDir > DEG_TO_RAD( 180) ) DeltaDir -= DEG_TO_RAD(360) ;
			while( DeltaDir < DEG_TO_RAD(-180) ) DeltaDir += DEG_TO_RAD(360) ;

			int		Delta = End-Start ;
			for( int i = 1 ; i < Delta ; i++ )
			{
				radian MoveDir = StartDir+(DeltaDir)*i/Delta ;
				GetCurMotion()->SetMoveDir( Start+i, MoveDir ) ;
				GetDocument()->SetModifiedFlag( TRUE ) ;
			}
			Invalidate() ;
		}
	}
}

void CMotionEditView::OnViewAllFacedirs() 
{
	// TODO: Add your command handler code here
	m_ViewAllFaceDirs = !m_ViewAllFaceDirs ;
	Invalidate() ;
}

void CMotionEditView::OnViewAllMovedirs() 
{
	// TODO: Add your command handler code here
	m_ViewAllMoveDirs = !m_ViewAllMoveDirs ;
	Invalidate() ;
}

void CMotionEditView::OnViewMirroredSkeleton() 
{
	// TODO: Add your command handler code here
	m_ViewMirroredSkeleton = !m_ViewMirroredSkeleton ;
	Invalidate() ;
}

void CMotionEditView::OnViewPrimaryMotion()
{
	m_ViewPrimaryMotion = !m_ViewPrimaryMotion;
	Invalidate();
}

void CMotionEditView::OnViewCompressedMotion()
{
	m_ViewCompressedMotion = !m_ViewCompressedMotion;
	Invalidate();
}

void CMotionEditView::OnViewMotionBlended()
{
	m_ViewMotionBlended = !m_ViewMotionBlended;
	Invalidate();
}

void CMotionEditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}



void CMotionEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	UpdateCharacterList() ;
	UpdateMotionControls() ;
	UpdateMotionList() ;
	UpdateEventList() ;
	UpdateEventControls() ;
	UpdateSkelPointList() ;
	UpdateSkelPointControls() ;
	if( m_bInitialUpdate )
	{
		UpdateEventProjectEvents();
	}

	switch( lHint )
	{
	case HINT_REDRAWALL:
	case HINT_MOTION_NEW:
	case HINT_MOTION_DELETE:
	case HINT_EVENT_NEW:
	case HINT_EVENT_DELETE:
	case HINT_SKELPOINT_NEW:
	case HINT_SKELPOINT_DELETE:
		RedrawWindow() ;
		GetFrame()->RedrawWindow() ;
		break ;
	default:
		break ;
	}
}

void CMotionEditView::OnCharacterExport() 
{
	CCeDDoc *pDoc = GetDocument( ) ;
	ASSERT( pDoc ) ;

	// TODO: Add your command handler code here
	if( m_pCharacter && m_pCharacter->GetSkeleton() )
	{
		CExportParams ep ;

		ep.m_CharacterIncludeFile	= pDoc->m_CharacterIncludeFile ;
		ep.m_EnableGAMECUBE			= m_pCharacter->m_ExportEnableGAMECUBE ;
		ep.m_EnablePC				= m_pCharacter->m_ExportEnablePC ;
		ep.m_EnableXBOX				= m_pCharacter->m_ExportEnableXBOX ;
		ep.m_EnablePS2				= m_pCharacter->m_ExportEnablePS2 ;
		ep.m_FileGAMECUBE			= m_pCharacter->m_ExportFileGAMECUBE ;
		ep.m_FilePC					= m_pCharacter->m_ExportFilePC ;
		ep.m_FileXBOX				= m_pCharacter->m_ExportFileXBOX ;
		ep.m_FilePS2				= m_pCharacter->m_ExportFilePS2 ;
		ep.m_OptForceUpper			= m_pCharacter->m_ExportOptForceUpper ;
		ep.m_OptCollisionBubbles	= m_pCharacter->m_ExportOptCollisionBubbles ;
		ep.m_OptMotionFlags			= m_pCharacter->m_ExportOptMotionFlags ;
		ep.m_FramesPerBlock			= m_pCharacter->m_ExportFramesPerBlock ;
		ep.m_OptFrontEndSpecific	= m_pCharacter->m_ExportFrontEndSpecific;

		if( ep.DoModal( ) == IDOK )
		{
			pDoc->m_CharacterIncludeFile				= ep.m_CharacterIncludeFile ;
			m_pCharacter->m_ExportEnableGAMECUBE		= !!ep.m_EnableGAMECUBE ;
			m_pCharacter->m_ExportEnablePC				= !!ep.m_EnablePC ;
			m_pCharacter->m_ExportEnableXBOX			= !!ep.m_EnableXBOX ;
			m_pCharacter->m_ExportEnablePS2				= !!ep.m_EnablePS2 ;
			m_pCharacter->m_ExportFileGAMECUBE			= ep.m_FileGAMECUBE ;
			m_pCharacter->m_ExportFilePC				= ep.m_FilePC ;
			m_pCharacter->m_ExportFileXBOX				= ep.m_FileXBOX ;
			m_pCharacter->m_ExportFilePS2				= ep.m_FilePS2 ;
			m_pCharacter->m_ExportOptForceUpper			= !!ep.m_OptForceUpper ;
			m_pCharacter->m_ExportOptCollisionBubbles	= !!ep.m_OptCollisionBubbles ;
			m_pCharacter->m_ExportOptMotionFlags		= !!ep.m_OptMotionFlags ;
			m_pCharacter->m_ExportFramesPerBlock		= ep.m_FramesPerBlock;
			m_pCharacter->m_ExportFrontEndSpecific		= !!ep.m_OptFrontEndSpecific;

			if( m_pCharacter->m_ExportEnableGAMECUBE )
				GetDocument()->CharacterExport( EXPORT_GAMECUBE, m_pCharacter->m_ExportFileGAMECUBE, m_pCharacter ) ;

			if( m_pCharacter->m_ExportEnablePC )
				GetDocument()->CharacterExport( EXPORT_PC, m_pCharacter->m_ExportFilePC, m_pCharacter ) ;

            if( m_pCharacter->m_ExportEnableXBOX )
				GetDocument()->CharacterExport( EXPORT_XBOX, m_pCharacter->m_ExportFileXBOX, m_pCharacter ) ;

            if( m_pCharacter->m_ExportEnablePS2 )
				GetDocument()->CharacterExport( EXPORT_PS2, m_pCharacter->m_ExportFilePS2, m_pCharacter ) ;
		}
	}
	else
	{
		MessageBox( "Must have a Skeleton before exporting", "Error", MB_ICONSTOP ) ;
	}
}

void CMotionEditView::UpdateEventProjectEvents()
{
	CMotionEditFrame *pFrame = GetFrame();
	pFrame->m_DlgEventList.UpdateEventDefinedEvents();
}

CEventList* CMotionEditView::GetProjectEvents()
{
	return GetDocument()->GetProjectEventList(); 
}

CEvent* CMotionEditView::EventGetCurrent()
{
	CMotion *pMotion = GetCurMotion();
	if( pMotion )
	{
		return pMotion->GetCurEvent();
	}
	return NULL;
}

CEventList* CMotionEditView::GetAttachedEvents()
{
	if( m_pCharacter )
	{
		return m_pCharacter->GetAttachedEvents();
	}
	else
	{
		return NULL;
	}
}

CSkeleton* CMotionEditView::GetSkeleton()
{
	if( m_pCharacter )
	{
		return m_pCharacter->GetSkeleton();
	}
	else
	{
		return NULL;
	}
}

void CMotionEditView::AttachBoneCurrentEvent(INT iBone)
{
	CString CurrentEventName;
	CMotion* pSaveCurrentMotion = NULL;
	CEvent* pSaveCurrentEvent = NULL;
	float SaveCurrentFrame;

	ASSERT( m_pCharacter );

	pSaveCurrentMotion = m_pCharacter->GetCurMotion();
	ASSERT( pSaveCurrentMotion );
	pSaveCurrentEvent = pSaveCurrentMotion->GetCurEvent();
	ASSERT( pSaveCurrentEvent );
	CurrentEventName = pSaveCurrentEvent->GetName();
	SaveCurrentFrame = m_pCharacter->GetCurFrame();

	{
		INT nMotions;
		INT iMotions;
		CMotion* pMotion = NULL;

		nMotions = m_pCharacter->GetNumMotions();
		for( iMotions = 0; iMotions < nMotions; iMotions++ )
		{
			INT nEvents;
			INT iEvent;
			CEvent* pEvent = NULL;

			pMotion = m_pCharacter->GetMotion( iMotions );
			ASSERT( pMotion );

			nEvents = pMotion->GetNumEvents();
			for( iEvent = 0; iEvent < nEvents; iEvent++ )
			{
				pEvent = pMotion->GetEvent( iEvent );
				ASSERT( pEvent );

				if( pEvent->GetName() == CurrentEventName )
				{
					CSkeleton* pSkeleton = NULL;
					vector3d Position;

					pSkeleton = m_pCharacter->GetSkeleton();
					ASSERT( pSkeleton );
					pEvent->SetBone( iBone );
					m_pCharacter->SetPose( pMotion, (f32)pEvent->GetFrame() );
					M4_GetTranslation( pSkeleton->GetPosedBoneMatrix( iBone ), &Position );
					pEvent->SetPosition( &Position );
				}
			}
		}
	}

	m_pCharacter->SetCurMotion( pSaveCurrentMotion );
	pSaveCurrentMotion->SetCurEvent( pSaveCurrentEvent );
	m_pCharacter->SetCurFrame( SaveCurrentFrame );

	GetDocument()->UpdateAllViews( NULL );
}

void CMotionEditView::OnUpdateStateFacedir(CCmdUI* pCmdUI) 
{
	BOOL bResetFace = TRUE;
	CMotion *pMotion = NULL;

	if( m_pCharacter )
	{
		pMotion = GetCurMotion();
		if( pMotion )
		{
			bResetFace = pMotion->GetAutoResetFaceDir();
		}
	}

	pCmdUI->Enable( ( pMotion && !bResetFace ) ? TRUE : FALSE );
}

void CMotionEditView::OnUpdateStateMovedir(CCmdUI* pCmdUI) 
{
	BOOL bResetMove = TRUE;
	CMotion *pMotion = NULL;

	if( m_pCharacter )
	{
		pMotion = GetCurMotion();
		if( pMotion )
		{
			bResetMove = pMotion->GetAutoResetMoveDir();
		}
	}

	pCmdUI->Enable( ( pMotion && !bResetMove ) ? TRUE : FALSE );
}

void CMotionEditView::OnUpdateMoveDirInterpolate(CCmdUI* pCmdUI) 
{
	BOOL bResetMove = TRUE;
	CMotion *pMotion = NULL;

	if( m_pCharacter )
	{
		pMotion = GetCurMotion();
		if( pMotion )
		{
			bResetMove = pMotion->GetAutoResetMoveDir();
		}
	}

	pCmdUI->Enable( ( pMotion && !bResetMove ) ? TRUE : FALSE );
}

void CMotionEditView::OnUpdateFaceDirInterpolate(CCmdUI* pCmdUI) 
{
	BOOL bResetFace = TRUE;
	CMotion *pMotion = NULL;

	if( m_pCharacter )
	{
		pMotion = GetCurMotion();
		if( pMotion )
		{
			bResetFace = pMotion->GetAutoResetFaceDir();
		}
	}

	pCmdUI->Enable( ( pMotion && !bResetFace ) ? TRUE : FALSE );
}

void CMotionEditView::OnUpdateStateEvent(CCmdUI* pCmdUI) 
{
	CMotion *pMotion = NULL;
	xbool EventSelected=FALSE;

	if( m_pCharacter )
	{
		pMotion = GetCurMotion();
		if( pMotion )
			EventSelected = pMotion->GetCurEvent() ? TRUE : FALSE;
	}

	pCmdUI->Enable( EventSelected );
}


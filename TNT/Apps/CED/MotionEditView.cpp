// MotionEditView.cpp : implementation file
//

#include "stdafx.h"
#include "CeD.h"
#include "MotionEditView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "MotionEditFrame.h"
#include "Skeleton.h"
#include "RenderContext.h"
#include "ExportFile.h"
#include "MFCUtils.h"
#include "DlgCharacter.h"
#include "DlgSkin.h"
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
	m_ViewMirroredSkeleton		= false ;
	m_TimeAccuratePlay			= false ;
	m_StartTiming				= false ;
	m_pEventUnderMouse			= NULL ;
	m_bInitialUpdate			= FALSE ;
	m_bBiBlendViewerDisplayed	= FALSE;
	m_bBiBlendViewerCreated		= FALSE ;
	m_bAnimTransViewerDisplayed	= FALSE ;
	m_bAnimTransViewerCreated	= FALSE ;
	m_bPropDefsViewerDisplayed	= FALSE ;
	m_bPropDefsViewerCreated	= FALSE ;
	m_bLockCameraToFaceDir		= FALSE ;
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

	if( m_bAnimTransViewerCreated )
		m_AnimTransViewer.DestroyWindow();
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
	ON_COMMAND(ID_CHARACTER_IMPORT_SKIN, OnCharacterImportSkin)
	ON_WM_DESTROY()
	ON_COMMAND(ID_ENABLE_D3D, OnEnableD3d)
	ON_UPDATE_COMMAND_UI(ID_ENABLE_D3D, OnUpdateEnableD3d)
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_COMMAND(ID_TRANSVIEW, OnTransview)
	ON_UPDATE_COMMAND_UI(ID_TRANSVIEW, OnUpdateTransview)
	ON_COMMAND(ID_BIBLENDVIEW, OnBiblendview)
	ON_UPDATE_COMMAND_UI(ID_BIBLENDVIEW, OnUpdateBiblendview)
	ON_COMMAND(ID_PROPDEF_DIALOG_BUTTON, OnPropdefDialogButton)
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
	if( !pCharacter )					return;
	if( !pCharacter->GetSkeleton() )	return;
	matrix4 Identity;
	Identity.Identity();
	pRC->RENDER_Character( pCharacter, Identity, RGB(0,0,0), RFLAG_BONES|RFLAG_AXES|RFLAG_FACETS );
}

//==========================================================================
void CMotionEditView::PlaceAndDrawProp( CRenderContext *pRC, CMotion* pMotion, s32 iProp, f32 fFrame )
{
	CPropData&	rProp		= pMotion->GetProp( iProp );
	CPropDef&	rPropDef	= *rProp.GetDefinition();

	//---	get the prop position and rotation
	vector3	Pos = rProp.GetPosition( fFrame );
	radian3 Rot = rProp.GetRotation( fFrame );

	//---	if there is a bone specified with this prop, relate the position and rotation to it.
	if( rPropDef.GetBone()!=-1 )
	{
		matrix4	BoneMat = *m_pCharacter->GetSkeleton()->GetPosedBoneMatrix( rPropDef.GetBone() );
		vector3 BonePos	= BoneMat.GetTranslation();
		radian3 BoneRot = BoneMat.GetRotationXYZ();

		//
		//	if the position is static it will have the bones position added to it, otherwise it will be
		//	rotated about the bone as well.
		//
		if( !rPropDef.IsPositionStatic() )
		{
			Pos.RotateX( BoneRot.X );
			Pos.RotateY( BoneRot.Y );
			Pos.RotateZ( BoneRot.Z );
			Pos += BonePos;
		}
		else
			Pos += BonePos;

		//
		//	If the rotation of the prop is static, nothing happens to the rotation values.
		//
		if( !rPropDef.IsRotationStatic() )
		{
			matrix4 m;
			m.Identity();
			m.SetRotationXYZ( Rot );
			m.RotateXYZ( BoneRot );
			Rot = m.GetRotationXYZ();
		}
	}

	//---	draw the prop.  If there is a linked character, draw it.
	DrawProp( pRC, rProp, rPropDef, Pos, Rot );
}

//==========================================================================
void CMotionEditView::DrawView( CRenderContext *pRC )
{
#define	FRAME_TO_COMPFRAME(F)	((f32)(F)*(pCompMotion->m_FrameRate/60.0f))
#define	COMP_ARROR_COLOR_SHIFT	0.7f

	int iCurrentFrame=0;
	f32	fCompressedFrame=0;
	CMotion*	pMotion	= NULL;
	CMotion*	pCompMotion = NULL;
	bool		bViewBlended = FALSE;
	s32			i;
	
	CCeDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	//---	Make Sure Character is correctly posed before continuing
	if( m_pCharacter && GetCurMotion() )
	{
		//---	Pose Skeleton
		iCurrentFrame = (s32)m_pCharacter->GetCurFrame() ;
		pMotion = GetCurMotion();

		if( m_ViewCompressedMotion )
		{
			pCompMotion = pMotion->GetCompressedVersion();
			fCompressedFrame = FRAME_TO_COMPFRAME(m_pCharacter->GetCurFrame());
		}

		bViewBlended = m_pCharacter->m_ViewMotionBlended;

//		m_pCharacter->SetPose( pMotion, (f32)iCurrentFrame, bViewBlended ) ;
		m_pCharacter->SetPose( bViewBlended );
	}

	//---	Update Camera Controls
	if( m_pCharacter && m_pCharacter->GetSkeleton() )
	{
		if( m_CameraLockedBone != 0 )
		{
			vector3 	v ;
			v = m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(m_CameraLockedBone-1)->GetTranslation();
			m_Camera.SetTarget( v ) ;
			m_Camera.ComputeMatrix() ;
			UpdateCameraControls( ) ;
		}
	}

	//---	Update Motion Controls
	if( GetCurMotion() )
	{
		//---	if the camera has been locked to the animation face direction,
		//		set it to this frames face direction now.
		if( m_bLockCameraToFaceDir )
		{
			matrix4 m;
            radian3 r(0, GetCurMotion()->GetFaceDir( iCurrentFrame ), 0);
			m.Identity();
			m.SetRotationXYZ( r );
			m.Transform( m_pCharacter->m_WorldBase );
			r = m.GetRotationXYZ();
			m_Camera.SetRotation( -r.Y );
		}

		//---	update the motion controls dialogs
		UpdateMotionControls() ;
	}

	//---	Setup Camera with parameters from CDC to render to
	CRect	r ;
	GetClientRect( &r ) ;
	m_Camera.SetProjectWindow( 0, 0, (f32)r.Width(), (f32)r.Height() ) ;

	//---	set the view matrix
	SetViewMatrix( m_Camera.ComputeMatrixD3D() );
	SetProjection( m_Camera.ComputeProjMatrixD3D() ) ;
//	SetProjection( m_Camera.m_NZ, m_Camera.m_FZ, DEG_TO_RAD(m_Camera.m_FOV), r.Width(), r.Height() );

	//---	Get a Matrix for positioning objects in the world
	matrix4		m ;
	m.Identity();

	//---	unless otherwise specified, use the world base matrix for the character.
	//		if a new matrix is created for some reason, remember to apply the world base matrix
	//		to any new matrix.
	pRC->PushMatrix( &m_pCharacter->m_WorldBase );

	//---	Draw Ground Plane
	if( pDoc->m_DrawGroundPlane )
	{
        radian3 r(0,0,0);
        vector3  v(0,0,0);
		m.SetRotationXYZ( r );
		m.SetTranslation( v );
		pRC->PushMatrix( &m ) ;
		pRC->RENDER_Grid( GRID_COLOR_MAJOR, GRID_COLOR_MINOR ) ;
		pRC->PopMatrix( ) ;
	}

	//---	Draw Construction Plane
	if( pDoc->m_DrawConstructionPlane )
	{
        radian3 r(0,0,0);
		m.SetRotationXYZ( r );
		m.SetTranslation( pDoc->m_ConPlanePos );
		pRC->PushMatrix( &m ) ;
		pRC->RENDER_Grid( GRID_COLOR_MAJOR, GRID_COLOR_MINOR ) ;
		pRC->PopMatrix( ) ;
	}

	//---	Draw MoveDir Arrow
	if( GetCurMotion() )
	{
		CMotion *pMotion = GetCurMotion() ;
		int nFrames = pMotion->GetNumFrames() ;

		//---	Draw Arrows for Every Frame?
		if( m_ViewAllFaceDirs )
		{
			for( int i = 0 ; i < nFrames ; i++ )
			{
				int d = (i - iCurrentFrame) ; if( d < 0 ) d = -d ; if( d > (nFrames/2) ) d = nFrames-d ; d = min( d, 1 ) ;
				int	c = d*192 ;

				vector3	t ;
                radian3	r(0,0,0);
				if( m_ViewPrimaryMotion )
				{
					pMotion->GetTranslation( t, i ) ;
					r.Yaw = pMotion->GetFaceDir( i );
					m.SetRotationXYZ( r );
					m.SetTranslation( t );
					m.Transform( m_pCharacter->m_WorldBase );
					pRC->PushMatrix( &m ) ;
					pRC->RENDER_Arrow( 20, RGB(c,c,255) ) ;
					pRC->PopMatrix( ) ;
				}

				if( pCompMotion )
				{
					if( bViewBlended )
					{
						pCompMotion->GetTranslationBlend( t, FRAME_TO_COMPFRAME(i) ) ;
						r.Yaw = pCompMotion->GetFaceDirBlend( FRAME_TO_COMPFRAME(i) );
					}
					else
					{
						pCompMotion->GetTranslation( t, (s32)FRAME_TO_COMPFRAME(i) ) ;
						r.Yaw = pCompMotion->GetFaceDir( (s32)FRAME_TO_COMPFRAME(i) );
					}
					m.SetRotationXYZ( r );
					m.SetTranslation( t );
					m.Transform( m_pCharacter->m_WorldBase );
					pRC->PushMatrix( &m ) ;
					pRC->RENDER_Arrow( 20, RGB(c,c,255*COMP_ARROR_COLOR_SHIFT) ) ;
					pRC->PopMatrix( ) ;
				}
			}
		}

		if( m_ViewAllMoveDirs )
		{
			for( int i = 0 ; i < nFrames ; i++ )
			{
				int d = (i - iCurrentFrame) ; if( d < 0 ) d = -d ; if( d > (nFrames/2) ) d = nFrames-d ; d = min( d, 1 ) ;
				int	c = d*192 ;

				vector3 	t ;
                radian3    r(0,0,0);
				if( m_ViewPrimaryMotion )
				{
					pMotion->GetTranslation( t, i ) ;
					r.Yaw = pMotion->GetMoveDir( i );
					m.SetRotationXYZ( r );
					m.SetTranslation( t );
					m.Transform( m_pCharacter->m_WorldBase );
					pRC->PushMatrix( &m ) ;
					pRC->RENDER_Arrow( 20, RGB(255,c,c) ) ;
					pRC->PopMatrix( ) ;
				}

				if( pCompMotion )
				{
					if( bViewBlended )
						pCompMotion->GetTranslationBlend( t, FRAME_TO_COMPFRAME(i) ) ;
					else
						pCompMotion->GetTranslation( t, (s32)FRAME_TO_COMPFRAME(i) ) ;
					r.Yaw = pCompMotion->GetMoveDir( (s32)FRAME_TO_COMPFRAME(i) ); // move direction shouldn't be blended
					m.SetRotationXYZ( r );
					m.SetTranslation( t );
					m.Transform( m_pCharacter->m_WorldBase );
					pRC->PushMatrix( &m ) ;
					pRC->RENDER_Arrow( 20, RGB(255*COMP_ARROR_COLOR_SHIFT,c,c) ) ;
					pRC->PopMatrix( ) ;
				}
			}
		}

		//---	Draw Arrows for Current Frame
		vector3 	t ;
		pMotion->GetTranslation( t, (s32)m_pCharacter->GetCurFrame() ) ;

		if( (!m_ViewAllFaceDirs && !m_ViewAllMoveDirs) || (m_ViewState == MEVS_SET_FACEDIR ) )
		{
			radian3 r(0,0,0);
			if( m_ViewPrimaryMotion )
			{
				r.Yaw = pMotion->GetFaceDir( iCurrentFrame );
				m.SetRotationXYZ( r );
				m.SetTranslation( t );
				m.Transform( m_pCharacter->m_WorldBase );
				pRC->PushMatrix( &m ) ;
				pRC->RENDER_Arrow( 20, RGB(0,0,255) ) ;
				pRC->PopMatrix( ) ;
			}

			if( m_ViewCompressedMotion )
			{
				if( bViewBlended )
				{
					pCompMotion->GetTranslationBlend( t, fCompressedFrame ) ;
					r.Yaw = pCompMotion->GetFaceDirBlend( fCompressedFrame );
				}
				else
				{
					pCompMotion->GetTranslation( t, (s32)fCompressedFrame ) ;
					r.Yaw = pCompMotion->GetFaceDir( (s32)fCompressedFrame );
				}
				m.SetRotationXYZ( r );
				m.SetTranslation( t );
				m.Transform( m_pCharacter->m_WorldBase );
				pRC->PushMatrix( &m ) ;
				pRC->RENDER_Arrow( 20, RGB(0,0,255*COMP_ARROR_COLOR_SHIFT) ) ;
				pRC->PopMatrix( ) ;
			}
		}

		if( (!m_ViewAllFaceDirs && !m_ViewAllMoveDirs) || (m_ViewState == MEVS_SET_MOVEDIR ) )
		{
			radian3 r(0,0,0);
			if( m_ViewPrimaryMotion )
			{
				pMotion->GetTranslation( t, iCurrentFrame ) ;
				r.Yaw = pMotion->GetMoveDir( iCurrentFrame );
				m.SetRotationXYZ( r );
				m.SetTranslation( t );
				m.Transform( m_pCharacter->m_WorldBase );
				pRC->PushMatrix( &m ) ;
				pRC->RENDER_Arrow( 20, RGB(255,0,0) ) ;
				pRC->PopMatrix( ) ;
			}

			if( m_ViewCompressedMotion )
			{
				if( bViewBlended )
					pCompMotion->GetTranslationBlend( t, fCompressedFrame ) ;
				else
					pCompMotion->GetTranslation( t, (s32)fCompressedFrame ) ;
				r.Yaw = pCompMotion->GetMoveDir( (s32)fCompressedFrame ); // move direction shouldn't be blended
				m.SetRotationXYZ( r );
				m.SetTranslation( t );
				m.Transform( m_pCharacter->m_WorldBase );
				pRC->PushMatrix( &m ) ;
				pRC->RENDER_Arrow( 20, RGB(255*COMP_ARROR_COLOR_SHIFT,0,0) ) ;
				pRC->PopMatrix( ) ;
			}
		}
	}

	//---	Draw the Character related items
	if( m_pCharacter )
	{
		//---	Draw Skeleton
		if( m_ViewPrimaryMotion )
		{
//			m_pCharacter->SetPose( pMotion, (f32)iCurrentFrame, bViewBlended ) ;
			m_pCharacter->SetPose( bViewBlended ) ;
			DrawSkeleton( pRC, m_pCharacter ) ;

			for( i=0; i<MAX_NUM_PROPS; i++ )
			{
				//---	if both the character and the motion exist, draw them.
				if( GetCurMotion() && GetCurMotion()->GetProp(i).IsFullyActive() )
					PlaceAndDrawProp( pRC, GetCurMotion(), i, m_pCharacter->GetCurFrame() );
			}
		}

		//---	Draw Mirrored Skeleton
		if( m_ViewMirroredSkeleton )
		{
            vector3  s(-1,1,1);
			m.Identity();
			m.Scale( s );
			m.Transform( m_pCharacter->m_WorldBase );
			pRC->PushMatrix( &m ) ;
			DrawSkeleton( pRC, m_pCharacter ) ;
			pRC->PopMatrix( ) ;
		}

		//---	Draw Compressed Motion at this frame
		if( m_ViewCompressedMotion && pCompMotion )
		{
			m_pCharacter->SetPose( pCompMotion, fCompressedFrame, bViewBlended );
			DrawSkeleton( pRC, m_pCharacter );

			for( i=0; i<MAX_NUM_PROPS; i++ )
			{
				//---	if both the character and the motion exist, draw them.
				if( pCompMotion->GetProp(i).IsFullyActive() )
					PlaceAndDrawProp( pRC, pCompMotion, i, fCompressedFrame );
			}
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
				vector3  v ;
				pSkelPoint->GetPosition( &v ) ;
				m.Identity();
				m.SetTranslation( v );
				m.Transform( m_pCharacter->m_WorldBase );
				pRC->PushMatrix( &m ) ;
				int r = (int)(128.0f*MOD_COLOR);
				int g = (int)0;
				int b = (int)(128.0f*MOD_COLOR);
				pRC->RENDER_Sphere( pSkelPoint->GetRadius(), RGB(r,g,b), RGB(r,g,b), RGB(r,g,b) ) ;
				pRC->PopMatrix( ) ;
			}
		}
	}

	//---	Draw Events for Current Motion
	if( GetCurMotion() )
	{
		CMotion *pMotion = GetCurMotion() ;

		//---	Loop through all Events
		for( int i = 0 ; i < pMotion->GetNumEvents() ; i++ )
		{
			//---	Get Event Pointer
			CEvent *pEvent = pMotion->GetEvent( i ) ;

			//---	Draw the Event
			vector3  v ;
			pEvent->GetPosition( &v ) ;
			m.Identity();
			m.SetTranslation( v );
			pRC->PushMatrix( &m ) ;
			int c = (pEvent->GetFrame() == m_pCharacter->GetCurFrame()) ? 0 : (int)(192.0f*MOD_COLOR) ;
			pRC->RENDER_Sphere( pEvent->GetRadius(), RGB(255*MOD_COLOR,c,c), RGB(c,255*MOD_COLOR,c), RGB(c,c,255*MOD_COLOR) ) ;
			pRC->PopMatrix( ) ;
		}
	}

	pRC->PopMatrix( ) ;
}

//=================================================================================================
void CMotionEditView::DrawProp( CRenderContext *pRC, CPropData& rProp, CPropDef& rPropDef, vector3& Pos, radian3& Rot )
{
	//---	if this prop has a loaded peice of geometry, draw the geometry.
	

	//---	if there is a linked character to this prop, draw its skeleton/skin
	CCharacter*	pCharacter	= (CCharacter*) rProp.GetLinkedCharacter();
	if( pCharacter )
	{
		if( rProp.GetLinkedMotion() )
		{
			int nFrame = (int)rProp.GetCharacter()->GetCurFrame() - rProp.GetLinkedMotion()->GetStartFrame() - rProp.GetMotion()->GetStartFrame();

			if( (nFrame >= 0) && (nFrame < rProp.GetLinkedMotion()->GetNumFrames()) )
				pCharacter->SetPoseAsProp( Pos, Rot, rProp.GetLinkedMotion(), (f32)nFrame, TRUE, rPropDef.IsMirrored() );
			else
				pCharacter->SetPoseAsProp( Pos, Rot, NULL, 0, TRUE, rPropDef.IsMirrored() );
		}
		else
			pCharacter->SetPoseAsProp( Pos, Rot, NULL, 0, TRUE, rPropDef.IsMirrored() );

		DrawSkeleton( pRC, pCharacter );
		return;
	}

	//---	since neither of the first two cases exist, draw a simple shape to represent the prop.
	matrix4 m;
	m.Identity();
	m.RotateXYZ( Rot );
	m.SetTranslation( Pos );
	pRC->PushMatrix( &m );
	pRC->RENDER_Prop( 8.0f, RGB(255.0f*MOD_COLOR,   0.0f*MOD_COLOR,   0.0f*MOD_COLOR),
							RGB(  0.0f*MOD_COLOR, 255.0f*MOD_COLOR,   0.0f*MOD_COLOR),
							RGB(  0.0f*MOD_COLOR,   0.0f*MOD_COLOR, 255.0f*MOD_COLOR) );
	pRC->PopMatrix();
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
		pMenu->CheckMenuItem( ID_VIEW_MOTION_BLENDED, (m_pCharacter && m_pCharacter->m_ViewMotionBlended) ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem (ID_VIEW_MIRRORED_SKELETON, m_ViewMirroredSkeleton ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem (ID_VIEW_ALL_FACEDIRS, m_ViewAllFaceDirs ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem (ID_VIEW_ALL_MOVEDIRS, m_ViewAllMoveDirs ? MF_CHECKED : MF_UNCHECKED ) ;
	}
}

/////////////////////////////////////////////////////////////////////////////

CMotion *CMotionEditView::GetCurMotion( )
{
	CMotion *pMotion = NULL ;

	if( m_pCharacter && m_pCharacter->GetCurMotion() )
		pMotion = m_pCharacter->GetCurMotion();

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

	pDlgCharacter->UpdateDetails() ;
}

void CMotionEditView::UpdatePropDetails( bool bNewMotionSelected )
{
	CCeDDoc *pDoc = GetDocument() ;
	ASSERT( pDoc ) ;
	CMotionEditFrame *pFrame = GetFrame() ;
	ASSERT( pFrame ) ;

	pFrame->m_DlgProps.UpdateDetails( bNewMotionSelected );
}

void CMotionEditView::UpdateSkinList()
{
	CCeDDoc *pDoc = GetDocument() ;
	ASSERT( pDoc ) ;
	CMotionEditFrame *pFrame = GetFrame() ;
	ASSERT( pFrame ) ;
	CDlgSkin *pDlgSkin = &pFrame->m_DlgSkin ;
	ASSERT( pDlgSkin ) ;

	//---	Reset List of Characters
	pDlgSkin->Reset( ) ;

	//---	Add all Characters to Combo
	int	CurrentSel = 0 ;
	for( int i = 0 ; i < m_pCharacter->GetNumSkins() ; i++ )
	{
		CSkin *pSkin = m_pCharacter->GetSkin( i ) ;
		ASSERT( pSkin ) ;
		pDlgSkin->AddSkin( pSkin ) ;
	}

	//---	Set Current Selection
	pDlgSkin->SelectSkin( m_pCharacter->GetCurSkin() ) ;
}

void CMotionEditView::UpdateCameraControls( )
{
	CString	OldStr, NewStr ;

	CMotionEditFrame *pFrame = GetFrame() ;
	ASSERT( pFrame ) ;

	//---	Get Current Target
	vector3 	v ;
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

///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
void CMotionEditView::UpdateMotionControls( )
{
	int Frame = 0 ;

	CMotionEditFrame *pFrame = GetFrame() ;

	//---	Update AnimBar Control
	CMotion *pMotion = NULL ;
	if( m_pCharacter )
	{
		pMotion = GetCurMotion() ;
		Frame = (int)m_pCharacter->GetCurFrame() ;
	}

	//---	if the trans viewer is being used, we must set the scroll bar differently
	if( m_bAnimTransViewerDisplayed && m_pCharacter )
	{
		s32 ScrollRange;
		s32 ScrollBase;
 		m_AnimTransViewer.GetScrollProperties( ScrollBase, ScrollRange );

		Frame -= (s32)m_pCharacter->m_CurMotion_StartFrame;

		pFrame->m_wndAnimBar.SetRange( ScrollRange ) ;
		pFrame->m_wndAnimBar.SetFrame( ScrollBase + Frame ) ;
	}
	else if( m_bBiBlendViewerDisplayed && m_pCharacter )
	{
		s32 ScrollRange;
		f32	NFramesRatio;
		if( m_BiBlendViewer.m_pMotion0 )
		{
			if( m_BiBlendViewer.m_pMotion1 )
			{
				f32 Frames0 = (f32)m_BiBlendViewer.m_pMotion0->GetNumFrames();
				f32	Frames1 = (f32)m_BiBlendViewer.m_pMotion1->GetNumFrames();
				ScrollRange	= (s32)(Frames1 + (Frames0-Frames1)*m_BiBlendViewer.m_Ratio);
				NFramesRatio	= (f32)m_BiBlendViewer.m_pMotion1->GetNumFrames() / (f32)m_BiBlendViewer.m_pMotion0->GetNumFrames();
				Frame			= (s32)(Frame*m_BiBlendViewer.m_Ratio + Frame*NFramesRatio*(1.0f-m_BiBlendViewer.m_Ratio));
			}
			else
			{
				ScrollRange = m_BiBlendViewer.m_pMotion0->GetNumFrames();
			}
		}
		else
		{
			ScrollRange = 10;
			Frame = 0;
		}

		pFrame->m_wndAnimBar.SetRange( ScrollRange );
		pFrame->m_wndAnimBar.SetFrame( Frame ) ;
	}
	else
	{
		pFrame->m_wndAnimBar.SetMotion( pMotion ) ;
		pFrame->m_wndAnimBar.SetFrame( Frame ) ;
	}

	pFrame->m_DlgMotionList.UpdateFaceMoveDir( ) ;
	pFrame->m_DlgMotionList.UpdateMotionSymbol( ) ;
	pFrame->m_DlgMotionList.UpdateAutoResetFaceDir();
	pFrame->m_DlgMotionList.UpdateAutoResetMoveDir();
//	pFrame->m_DlgMotionList.UpdateExportInfo();
}

///////////////////////////////////////////////////////////////////////////////
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
		UpdatePropDetails( TRUE ) ;
		UpdateSkelPointList() ;
		UpdateSkelPointControls() ;
		Invalidate() ;
	}
}

//====================================================================================
//
//	GetSelectedMotions fills a motion list with the currently selected motions in the
//	list.
//
s32 CMotionEditView::GetSelectedMotions( CMotion** pMotionList, s32 MaxCount )
{
	CMotionEditFrame *pFrame = GetFrame() ;
	return pFrame->m_DlgMotionList.GetSelectedMotions( pMotionList, MaxCount );
}

//====================================================================================
s32 CMotionEditView::GetFirstSelectedMotion( void )
{
	m_SelectedMotionCount = GetSelectedMotions( m_SelectedMotionList, MAX_SELECTED_MOTIONS );
	return 0;
}

//====================================================================================
CMotion* CMotionEditView::GetNextSelectedMotion( s32& CurIndex )
{
	ASSERT(CurIndex >= 0);
	if(( CurIndex < 0 ) || ( CurIndex >= m_SelectedMotionCount ))
		return NULL;

	return m_SelectedMotionList[CurIndex++];
}

//====================================================================================
void CMotionEditView::MotionAdd( ) 
{
	OnCharacterImportMotion() ;
}

void CMotionEditView::MotionDelete( CMotion *pMotion, CMotion *pNewCurrentMotion ) 
{
	ASSERT( pMotion ) ;

//	int Sure = MessageBox( "Are You Sure?", "Delete Motion", MB_ICONWARNING|MB_YESNO ) ;
//	if( Sure == IDYES )
//	{
		m_pCharacter->DeleteMotion( pMotion ) ;
 		m_pCharacter->SetCurMotion( pNewCurrentMotion ) ;

		GetDocument()->MotionDeleted( pMotion ) ;
//  		GetDocument()->UpdateAllViews( NULL, HINT_MOTION_DELETE, m_pCharacter ) ;
//	}
}

void CMotionEditView::MotionsDeleted( void )
{
	GetDocument()->UpdateAllViews( NULL, HINT_MOTION_DELETE, m_pCharacter ) ;
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
/*
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
	*/
}

void CMotionEditView::MotionSetLinkMotion( CMotion *pLinkMotion ) 
{
	/*
	CMotion *pMotion = GetCurMotion() ;
	if( pMotion )
	{
		if( pMotion->m_pLinkMotion != pLinkMotion )
		{
			pMotion->m_pLinkMotion = pLinkMotion ;
			RedrawWindow() ;
		}
	}
	*/
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
		vector3 	v ;
		m_Camera.GetTarget( &v ) ;
		pEvent->SetPosition( &v ) ;
		pEvent->SetFrame( (int)m_pCharacter->GetCurFrame() ) ;

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
			vector3  v ;
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
		vector3 	v ;
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
			vector3  v ;
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
	UpdatePropDetails( TRUE ) ;
	UpdateSkelPointList() ;
	UpdateSkelPointControls() ;
	UpdateEventProjectEvents();

	Invalidate() ;
}

void CMotionEditView::SetSkin( int iSkin ) 
{
	if( iSkin < 0 )
		return;

	m_pCharacter->SetCurSkin( iSkin-1 ) ;

//	UpdateCameraControls( ) ;
//	UpdateCameraBoneList() ;
//	UpdateMotionControls() ;
//	UpdateMotionList() ;
//	UpdateEventList() ;
//	UpdateEventControls() ;
//	UpdatePropDetails() ;
//	UpdateSkelPointList() ;
//	UpdateSkelPointControls() ;
//	UpdateEventProjectEvents();

	Invalidate() ;
}

void CMotionEditView::SetSkinTranslucent( bool bTranslucent ) 
{
	m_pCharacter->RenderSkinTranslucent( bTranslucent ) ;
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

void CMotionEditView::SetLockCameraToFaceDir( bool Setting )
{
	m_bLockCameraToFaceDir = !!Setting;
	Invalidate() ;
}


CEvent *CMotionEditView::PickGetEvent( CPoint p )
{
	CEvent *pFound = NULL ;

	//---	Get Ray Base and Vector
	vector3  RayBase, Ray ;
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
			vector3  Center ;
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
		vector3 	Plane(0,0,0) ;

		//---	Get ROOT Bone position
		if( m_pCharacter && m_pCharacter->GetSkeleton() )
		{
			Plane = m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0)->GetTranslation();
//			M4_GetTranslation( m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0), &Plane ) ;
		}

		//---	Get Point on Ground Plane given Cursor position
		vector3  v ;
		if( GetPointOnPlane( &v, point, Plane.Y ) )
		{
			//---	Make Relative to ROOT Bone
			v -= Plane;
//			V3_Sub( &v, &v, &Plane ) ;

			//---	Calculate Rotation of FaceDir Vector from Origin to v
			if( GetCurMotion() )
			{
				GetCurMotion()->SetFaceDir( (int)m_pCharacter->GetCurFrame(), x_atan2( v.X, v.Z ) ) ;
				pDoc->SetModifiedFlag() ;
				Invalidate() ;
			}
		}
	}

	//---	Handle SET MOVEDIR
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_SET_MOVEDIR) && (GetCapture() == this) )
	{
		vector3 	Plane( 0,0,0 ) ;

		//---	Get ROOT Bone position
		if( m_pCharacter && m_pCharacter->GetSkeleton() )
		{
			Plane = m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0)->GetTranslation();
//			M4_GetTranslation( m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0), &Plane ) ;
		}

		//---	Get Point on Ground Plane given Cursor position
		vector3  v ;
		if( GetPointOnPlane( &v, point, Plane.Y ) )
		{
			//---	Make Relative to ROOT Bone
			v -= Plane;
//			V3_Sub( &v, &v, &Plane ) ;

			//---	Calculate Rotation of MoveDir Vector from Origin to v
			if( GetCurMotion() )
			{
				GetCurMotion()->SetMoveDir( (int)m_pCharacter->GetCurFrame(), x_atan2( v.X, v.Z ) ) ;
				pDoc->SetModifiedFlag() ;
				Invalidate() ;
			}
		}
	}

	//---	Handle MOVE EVENT X/Z
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MEVS_SET_EVENT) && (GetCapture() == this) )
	{
		vector3 	Plane( 0,0,0 ) ;

		//---	Get ROOT Bone position
		if( m_pCharacter && m_pCharacter->GetSkeleton() )
		{
			Plane = m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0)->GetTranslation();
//			M4_GetTranslation( m_pCharacter->GetSkeleton()->GetPosedBoneMatrix(0), &Plane ) ;
		}

		//---	Get Point on Ground Plane given Cursor position
		vector3  v ;
		if( GetPointOnPlane( &v, point, 0.0f ) )
		{
			//---	Make Relative to ROOT Bone
			v -= Plane;
//			V3_Sub( &v, &v, &Plane ) ;

			//---	Calculate Rotation of MoveDir Vector from Origin to v
			if( GetCurMotion() && GetCurMotion()->GetCurEvent() )
			{
				vector3  cv;
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
			vector3  cv;
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
		vector3 	moveSrc ;
		vector3 	moveDst ;
		vector3 	Target ;

		m.Identity();
		m.RotateY( -m_Camera.GetRotation() );
//		M4_Identity( &m ) ;
//		M4_RotateYOn( &m, -m_Camera.GetRotation() ) ;

		moveSrc.Set( (f32)m_MouseDelta.x, 0, (f32)m_MouseDelta.y ) ;
		moveSrc.Scale( m_Camera.GetDistance() / 1000 );
		m.Transform( &moveDst, &moveSrc, 1 ) ;
//		V3_Set( &moveSrc, (f32)m_MouseDelta.x, 0, (f32)m_MouseDelta.y ) ;
//		V3_Scale( &moveSrc, &moveSrc, m_Camera.GetDistance() / 1000 ) ;
//		M4_TransformVerts( &m, &moveDst, &moveSrc, 1 ) ;

		m_Camera.GetTarget( &Target ) ;
		Target += moveDst;
//		V3_Add( &Target, &Target, &moveDst ) ;
		m_Camera.SetTarget( Target ) ;

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

bool CMotionEditView::GetPointOnPlane( vector3  *pPoint, CPoint p, f32 yPlane ) 
{
	bool		Intersected = false ;

	//---	Get Ray from Camera through Cursor
	vector3  RayBase, Ray ;
	GetCursorRay( &RayBase, &Ray, p ) ;

	//---	Get Point RAY intersects on plane or return Zero if can't intersect
	if( (x_abs(Ray.Y) > 0.01) )
	{
		f32	d = (RayBase.Y-yPlane) / -Ray.Y ;
		if( d >= 0 )
		{
			Ray.Scale( d );
			*pPoint = RayBase + Ray;
//			V3_Scale( &Ray, &Ray, d ) ;
//			V3_Add( pPoint, &RayBase, &Ray ) ;
			Intersected = true ;
		}
	}
	else
	{
		pPoint->Zero();
//		V3_Zero( pPoint ) ;
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

xbool CMotionEditView::Play()
{
	OnAnimPlay();
	return m_Playing;
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
	CCeDView::OnInitialUpdate();
	
	//---	Grab Character Pointer from Frame and set View Pointers in all Dialogs
	CMotionEditFrame *pFrame = GetFrame( ) ;
	ASSERT( pFrame ) ;
	m_pCharacter = pFrame->m_pCharacter ;
	pFrame->m_pView = this ;
	pFrame->m_DlgCharacter.m_pView = this ;
	pFrame->m_DlgSkin.m_pView = this ;
	pFrame->m_DlgCamera.m_pView = this ;
	pFrame->m_DlgMotionList.m_pView = this ;
	pFrame->m_DlgEventList.m_pView = this ;
	pFrame->m_DlgProps.m_pView = this;
	pFrame->m_DlgSkelPointList.m_pView = this ;

	//---	Set AnimControlBar to point to this View
	GetFrame()->m_wndAnimBar.SetView( this ) ;

	SetViewState( MEVS_CAMERA_ROTATE ) ;	
	UpdateCharacterList() ;
	UpdateSkinList() ;
	UpdateCameraControls( ) ;
	UpdateCameraBoneList() ;
	UpdateMotionControls() ;
	UpdateMotionList() ;
	UpdateEventList() ;
	UpdateEventControls() ;
	UpdatePropDetails() ;
	UpdateSkelPointList() ;
	UpdateSkelPointControls() ;

	UpdateEventProjectEvents();

	//---	Setup Multimedia Timer
	m_TimerEvent = timeSetEvent( 17, 1, MMTimerCallback, (DWORD)this, TIME_CALLBACK_FUNCTION | TIME_PERIODIC ) ;
    ASSERT( m_TimerEvent ) ;

	m_bInitialUpdate = TRUE;
//	SetTimer( 1, 16, NULL ) ;

	// TODO: Add your specialized code here and/or call the base class
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
				AdvanceMotion( (f32)nFrames );
//				for( int i = 0 ; i < nFrames ; i++ )
//					m_pCharacter->AnimNextFrame() ;
				m_LastFrac = (f32)(delta - (nFrames * (1000.0/60.0))) ;
				Invalidate() ;
			}
			else
			{
				//---	Play a Frame at a Time
				AdvanceMotion( 1 );
//				m_pCharacter->AnimNextFrame() ;
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

void CMotionEditView::OnCharacterImportSkin() 
{
	GetDocument()->CharacterImportSkin( m_pCharacter );
	UpdateSkinList();
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
	if( m_pCharacter )
		m_pCharacter->m_ViewMotionBlended = !m_pCharacter->m_ViewMotionBlended;
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
	UpdatePropDetails();
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
	case HINT_D3DENABLE:
		CD3DView::EnableD3D( TRUE );
		OnUpdate( NULL, 0, NULL );
		break ;
	case HINT_D3DDISABLE:
		CD3DView::EnableD3D( FALSE );
		OnUpdate( NULL, 0, NULL );
		break ;
	default:
		break ;
	}
}

void CMotionEditView::OnCharacterExport() 
{
	CCeDDoc *pDoc = GetDocument( ) ;
	ASSERT( pDoc ) ;

	pDoc->CharacterExportFull( m_pCharacter );
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
					vector3  Position;

					pSkeleton = m_pCharacter->GetSkeleton();
					ASSERT( pSkeleton );
					pEvent->SetBone( iBone );
					m_pCharacter->SetPose( pMotion, (f32)pEvent->GetFrame() );
					Position = pSkeleton->GetPosedBoneMatrix( iBone )->GetTranslation();
//					M4_GetTranslation( pSkeleton->GetPosedBoneMatrix( iBone ), &Position );
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


void CMotionEditView::OnDestroy() 
{
	CCeDView::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CMotionEditView::OnEnableD3d() 
{
	CD3DView::OnEnableD3d();
	OnUpdate( NULL, 0, 0 );
}

void CMotionEditView::OnUpdateEnableD3d(CCmdUI* pCmdUI) 
{
	CD3DView::OnUpdateEnableD3d( pCmdUI );	
}

void CMotionEditView::OnMove(int x, int y) 
{
	CCeDView::OnMove(x, y);
}

void CMotionEditView::OnSize(UINT nType, int cx, int cy) 
{
	CCeDView::OnSize(nType, cx, cy);
}

void CMotionEditView::OnTransview() 
{
	ToggleTransitionViewer();
}

///////////////////////////////////////////////////////////////////////////////
void CMotionEditView::ToggleTransitionViewer()
{
	CTransitionViewer* pViewer;
	pViewer = &m_AnimTransViewer;

	//---	if the window hasn't been created, create it now
	if( !m_bAnimTransViewerCreated )
	{
		pViewer->Create( IDD_TRANSITIONVIEWER, NULL );
		m_bAnimTransViewerCreated = TRUE;
		pViewer->m_pView = this;
	}

	m_bAnimTransViewerDisplayed = !m_bAnimTransViewerDisplayed;

	//---	hide/show the window
	if( m_bAnimTransViewerDisplayed )
	{
		pViewer->ShowWindow( SW_SHOW );

		//---	can only use one of these dialogs at a time
		if( m_bBiBlendViewerDisplayed )
			ToggleBiBlendViewer();
	}
	else
	{
		pViewer->ShowWindow( SW_HIDE );

		if( m_pCharacter )
			m_pCharacter->m_WorldBase.Identity();
	}
}

///////////////////////////////////////////////////////////////////////////////
void CMotionEditView::ToggleBiBlendViewer()
{
	DlgBiBlendViewer* pViewer;
	pViewer = &m_BiBlendViewer;

	//---	if the window hasn't been created, create it now
	if( !m_bBiBlendViewerCreated )
	{
		pViewer->Create( IDD_BIDIRECTIONALBLENDVIEWER, NULL );
		m_bBiBlendViewerCreated = TRUE;
		pViewer->m_pView = this;
		pViewer->Reset();
	}

	m_bBiBlendViewerDisplayed = !m_bBiBlendViewerDisplayed;

	//---	hide/show the window
	if( m_bBiBlendViewerDisplayed )
	{
		//---	can only use one of these dialogs at a time
		if( m_bAnimTransViewerDisplayed )
			ToggleTransitionViewer();

		pViewer->Reset();
		pViewer->ShowWindow( SW_SHOW );
	}
	else
	{
		pViewer->ShowWindow( SW_HIDE );
	}
}

///////////////////////////////////////////////////////////////////////////////
void CMotionEditView::DisplayPropDefs( bool bDisplay, s32 SuggestedProp )
{
	//---	if the prop defs haven't yet been created, create the dialog
	if( !m_bPropDefsViewerCreated )
	{
		m_PropDefsViewer.Create( IDD_PROPDEFS );
		m_PropDefsViewer.m_pView = this;
		m_bPropDefsViewerCreated = TRUE;
	}

	if( bDisplay )
	{
		m_PropDefsViewer.ShowWindow( SW_SHOW );
		m_PropDefsViewer.SetWindowPos( &CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
		m_PropDefsViewer.SetSelectedPropDef( SuggestedProp );
		m_PropDefsViewer.UpdateDetails();
	}
	else
		m_PropDefsViewer.ShowWindow( SW_HIDE );
}

///////////////////////////////////////////////////////////////////////////////
void CMotionEditView::OnUpdateTransview(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bAnimTransViewerDisplayed );	
}

//==========================================================================
void CMotionEditView::AdvanceMotion( f32 nFrames )
{
	if( !m_bBiBlendViewerDisplayed )
		m_pCharacter->SetSecondMotion( NULL, 0.0f );

	//---	if the anim trans viewer isn't active, advance the frame simply and go on
	if( !m_bAnimTransViewerDisplayed )
	{
		for( int i = 0 ; i < nFrames ; i++ )
			m_pCharacter->AnimNextFrame() ;
		return;
	}

	if( m_bAnimTransViewerDisplayed )
	{
		//---	determine if the current motion is the currently selection motion in the viewer
		CMotion* pMotion;
		CMotion* pLastMotion;
		f32 BlendFrames, StartFrame, EndFrame;
		f32 LastFrame;
		m_AnimTransViewer.GetCurrentPlayMotion( m_pCharacter->m_WorldBase,
												pMotion,
												BlendFrames,
												StartFrame,
												EndFrame,
												pLastMotion,
												LastFrame );
		if( pMotion != m_pCharacter->GetCurMotion() )
		{
			m_pCharacter->SetCurMotion( pMotion,
										BlendFrames,
										StartFrame,
										EndFrame,
										pLastMotion,
										LastFrame );
			m_pCharacter->SetCurFrame( StartFrame );

			GetFrame()->m_DlgMotionList.UpdateMotionList( m_pCharacter );
			return;
		}

		//---	if there is no motion set, may as well return now.
		if( pMotion==NULL )
			return;

		//---	we must determine if the new frame will be inside of the current motion's play range
		//		if it won't be, increment to the next motion.
		f32 Frame = m_pCharacter->GetCurFrame();
		f32 NewFrame = Frame + nFrames;

		if( NewFrame < StartFrame )
			NewFrame = StartFrame;
		else if( NewFrame > EndFrame )
		{
			NewFrame = StartFrame + NewFrame - (EndFrame-1);
			m_AnimTransViewer.GetNextPlayMotion( m_pCharacter->m_WorldBase,
												 pMotion,
												 BlendFrames,
												 StartFrame,
												 EndFrame,
												 pLastMotion,
												 LastFrame );
			m_pCharacter->SetCurMotion( pMotion, BlendFrames, StartFrame, EndFrame, pLastMotion, LastFrame );
			AdvanceMotion( NewFrame - m_pCharacter->GetCurFrame() );
	//		m_pCharacter->SetCurFrame( StartFrame + NewFrame - (EndFrame-1) );

			GetFrame()->m_DlgMotionList.UpdateMotionList( m_pCharacter );
		}

		//---	set the new frame
		m_pCharacter->SetCurFrame( NewFrame );
	}
}

///////////////////////////////////////////////////////////////////////////////
void CMotionEditView::OnUpdateBiblendview(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bBiBlendViewerDisplayed );	
}

///////////////////////////////////////////////////////////////////////////////
void CMotionEditView::OnBiblendview() 
{
	ToggleBiBlendViewer();	
}

//==========================================================================
void CMotionEditView::SetAnimScrollFrame( s32 Frame )
{
	//---	if we are playing the animation, exit now.  Only allow a SetAnimScrollFrame() when
	//		the motion isn't playing (i.e. when the slider is moved manually with the cursor)
	if( m_Playing )
		return;

	if( m_bAnimTransViewerDisplayed )
	{
		CMotion* pMotion;
		CMotion* pLastMotion;
		f32 BlendFrames, StartFrame, EndFrame;
		f32 LastFrame;
		s32 NewFrame;

		//---	based on the scroll frame determine which animation should be set
		m_AnimTransViewer.GetScrollPlayMotion(	Frame,
												NewFrame,
												m_pCharacter->m_WorldBase,
												pMotion,
												BlendFrames,
												StartFrame,
												EndFrame,
												pLastMotion,
												LastFrame );

		//---	if there is no motion set, may as well return now.
		if( pMotion==NULL )
			return;

		//---	we must determine if the new frame will be inside of the current motion's play range
		//		if it won't be, increment to the next motion.
		ASSERT( NewFrame <= EndFrame );
		m_pCharacter->SetCurMotion( pMotion, BlendFrames, StartFrame, EndFrame, pLastMotion, LastFrame );
		m_pCharacter->SetCurFrame( (f32)NewFrame );

		GetFrame()->m_DlgMotionList.UpdateMotionList( m_pCharacter );

		//---	set the new frame
		Invalidate() ;
	}
	else if( m_bBiBlendViewerDisplayed )
	{
		if( m_BiBlendViewer.m_pMotion0 )
		{
			if( m_BiBlendViewer.m_pMotion1 )
			{
				f32 Frames0 = (f32)m_BiBlendViewer.m_pMotion0->GetNumFrames();
				f32 Frames1 = (f32)m_BiBlendViewer.m_pMotion1->GetNumFrames();
				f32 NFramesRatio =  Frames1 / Frames0;
				f32 Range = Frames1 + (Frames0-Frames1)*m_BiBlendViewer.m_Ratio;

				f32 NewFrame = Frame*m_BiBlendViewer.m_Ratio + Frame*NFramesRatio*(1.0f-m_BiBlendViewer.m_Ratio);
				m_pCharacter->SetCurFrame( NewFrame );
				Invalidate();
			}
			else
			{
				m_pCharacter->SetCurFrame( (f32)Frame ) ;
				Invalidate() ;
			}
		}
		else
		{
			m_pCharacter->SetCurFrame( 0.0f ) ;
			Invalidate() ;
		}
	}
	else
	{
		m_pCharacter->SetCurFrame( (f32)Frame ) ;
		Invalidate() ;
	}
}

//==========================================================================
void CMotionEditView::OnPropdefDialogButton() 
{
	if( m_pCharacter )
		DisplayPropDefs( TRUE );	
}

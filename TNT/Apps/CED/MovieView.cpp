// MovieView.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "MovieView.h"
#include "MovieFrame.h"
#include "Actor.h"
#include "Key.h"
#include "MovieBar.h"
#include "MFCutils.h"
#include "DlgActorNew.h"
#include "MultiBar.h"
#include "NumFrames.h"
#include "ExportFile.h"
#include "Shot.h"
#include "DlgMovieMerge.h"
#include "Mesh.h"

#include <mmsystem.h>

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovieView

IMPLEMENT_DYNCREATE(CMovieView, CCeDView)

CMovieView::CMovieView()
{
	m_pMovie = NULL ;
	m_MoveX = false ;
	m_MoveY = false ;
	m_MoveZ = false ;
	m_ActorType = ACTOR_CHARACTER ;
	m_MouseOverKey = false ;
	m_MouseOverActor = false ;

	//---	set the view flags initially without the cameras
	m_ViewFlags = ~(MVF_VIEWCAMERAS|MVF_VIEWCAMERATARGETS);
	m_ViewLayer	= -1; // all layers

    m_StartTiming = true ;
    m_Playing = false ;
    m_TimeAccuratePlay = true ;
    m_nTimerMessages = 0 ;

    m_Camera.SetDistance( 1000 ) ;
	m_Camera.ComputeMatrix( ) ;
}

CMovieView::~CMovieView()
{
	if( m_TimerEvent )
	{
		MMRESULT res = timeKillEvent( m_TimerEvent ) ;
        ASSERT( res == TIMERR_NOERROR ) ;
		m_TimerEvent = 0 ;
        Sleep( 100 ) ;
	}
}


CMovieFrame *CMovieView::GetFrameWnd( )
{
	CMovieFrame *pFrame = (CMovieFrame*)GetParent( ) ;
	ASSERT( pFrame ) ;
	return pFrame ;
}

void CMovieView::UpdateMenuItems( CMenu *pMenu )
{
	if( pMenu )
	{
//		pMenu->CheckMenuItem (ID_VIEW_COMPRESSED_MOTION, m_ViewCompressedMotion ? MF_CHECKED : MF_UNCHECKED ) ;
//		pMenu->CheckMenuItem (ID_VIEW_MIRRORED_SKELETON, m_ViewMirroredSkeleton ? MF_CHECKED : MF_UNCHECKED ) ;
//		pMenu->CheckMenuItem (ID_VIEW_ALL_FACEDIRS, m_ViewAllFaceDirs ? MF_CHECKED : MF_UNCHECKED ) ;
//		pMenu->CheckMenuItem (ID_VIEW_ALL_MOVEDIRS, m_ViewAllMoveDirs ? MF_CHECKED : MF_UNCHECKED ) ;
	}
}

CActor *CMovieView::GetCurActor( )
{
	CActor *pActor = NULL ;

	ASSERT( m_pMovie ) ;
	if( m_pMovie->GetCurActor() )
		pActor = m_pMovie->GetCurActor() ;

	return pActor ;
}

void CMovieView::MovieLayerDeleted( int Layer )
{
	if(( GetCurActor()==NULL ) || ( m_pDlgLayer->GetEditLayer() == -1 ) || ( GetCurActor()->GetLayer() == m_pDlgLayer->GetEditLayer() ))
		m_pDlgActorList->RebuildLayerDropList();
	else
		ActorSetCurrent( NULL );

	RedrawWindow();
}

void CMovieView::MovieLayerSelected( void )
{
	if(( GetCurActor()!=NULL ) && ( m_pDlgLayer->GetEditLayer() != -1 ) && ( GetCurActor()->GetLayer() != m_pDlgLayer->GetEditLayer() ))
		ActorSetCurrent( NULL );

	ActorUpdateList();
	RedrawWindow();
}

void CMovieView::MovieLayerChanged( void )
{
	m_pDlgActorList->RebuildLayerDropList();
}

CActor *CMovieView::ActorNew( )
{
	CActor *pActor = NULL ;

	ASSERT( m_pMovie ) ;
	CDlgActorNew	Dlg ;
	Dlg.m_pCharacterList = &GetDocument()->m_CharacterList ;
	Dlg.m_Type = GetActorType( ) ;
	Dlg.m_pMeshList = &GetDocument()->m_MeshList ;

	if( Dlg.DoModal() == IDOK )
	{
		pActor = new CActor ;
		ASSERT( pActor ) ;
		pActor->SetName( Dlg.m_Name ) ;
		if( Dlg.m_Type == ACTOR_CHARACTER )
		{
			pActor->SetCharacter( Dlg.m_pCharacter ) ;
			pActor->SetSkin( Dlg.m_Skin );
		}
		if( Dlg.m_Type == ACTOR_SCENERY )
		{
			pActor->SetMesh( Dlg.m_pMesh ) ;
			pActor->SetName( Dlg.m_pMesh->GetFileName() ) ;
		}
		pActor->SetType( Dlg.m_Type ) ;
		pActor->SetLayer( m_pDlgLayer->GetEditLayer() );
		m_pMovie->m_ActorList.AddTail( pActor ) ;
		GetDocument()->SetModifiedFlag( ) ;

		ActorSetCurrent( pActor ) ;
		ActorUpdateList( ) ;

		//---	Add PositionKey for Actor
		CKey *pKey = pActor->KeyNew( GetFrame() ) ;
		vector3  v ;
		m_Camera.GetTarget( &v ) ;
		v.X += 50 ;
		v.Z += 50 ;
		pKey->SetPosition( &v ) ;
		KeyUpdateList( ) ;
		KeySetCurrent( pKey ) ;
	}

	return pActor ;
}

void CMovieView::ActorDel( CActor *pActor, CActor *pNewCurrentActor )
{
	ASSERT( m_pMovie ) ;

	if( pActor )
	{
		int Sure = MessageBox( "Are You Sure?", "Delete Actor", MB_ICONWARNING|MB_YESNO ) ;
		if( Sure == IDYES )
		{
			m_pMovie->m_ActorList.Remove( pActor ) ;
			delete pActor ;
			ActorSetCurrent( pNewCurrentActor ) ;
			ActorUpdateList( ) ;
		}
	}
}

void CMovieView::ActorSetCurrent( CActor *pActor )
{
	ASSERT( m_pMovie ) ;
//	if( m_pMovie->GetCurActor( ) != pActor )
	{
		ActorSelectAll( false ) ;
		m_pMovie->SetCurActor( pActor ) ;
		m_pDlgActorList->SelectActor( pActor ) ;
		if( pActor )
			m_pDlgLayer->EnsureViewable( pActor->GetLayer() );
		else
			ActorUpdateList();
		KeyUpdateList( ) ;
		KeyUpdateMotions( ) ;
		KeyUpdateDetails( KeyGetCurrent() ) ;
	}
}

void CMovieView::ActorUpdateList( )
{
	int EditLayer = m_pDlgLayer->GetEditLayer();

	m_pDlgActorList->Reset( ) ;

	ASSERT( m_pMovie ) ;

	POSITION Pos = m_pMovie->m_ActorList.GetHeadPosition( ) ;
	while( Pos )
	{
		CActor *pActor = m_pMovie->m_ActorList.GetNext( Pos ) ;
		if(( pActor->GetType() == m_ActorType ) && ((EditLayer == -1) || (pActor->GetLayer() == EditLayer))) 
		{
			m_pDlgActorList->AddActor( pActor ) ;
		}
	}

	m_pDlgActorList->SelectActor( m_pMovie->GetCurActor( ) ) ;
}

void CMovieView::ActorLookAt( )
{
	ASSERT( m_pMovie ) ;

	CActor *pActor = m_pMovie->GetCurActor( ) ;
	if( pActor )
	{
		vector3  v ;
		pActor->GetPositionAtTime( &v, m_pMovie->GetFrame() ) ;
		m_Camera.SetTarget( v ) ;
		m_Camera.ComputeMatrix() ;
		RedrawWindow( ) ;
	}
}

void CMovieView::ActorSelectAll( bool Select )
{
	POSITION Pos = m_pMovie->m_ActorList.GetHeadPosition() ;
	while( Pos )
	{
		CActor *pActor = m_pMovie->m_ActorList.GetNext( Pos ) ;
		ASSERT( pActor ) ;
		pActor->m_Selected = Select ;
	}
}






CKey *CMovieView::KeyAdd( )
{
	CKey *pKey = NULL ;

	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor( ) )
	{
		CActor *pActor = m_pMovie->GetCurActor( ) ;
		CKey *pKey = pActor->KeyNew( GetFrame() ) ;
		vector3  v ;
		pKey->GetPosition( &v ) ;
		v.X += 50 ;
		v.Z += 50 ;
		pKey->SetPosition( &v ) ;
		GetDocument()->SetModifiedFlag( ) ;
		KeySetCurrent( pKey ) ;
		KeyUpdateList( ) ;
		KeyUpdateDetails( pKey ) ;
	}

	return pKey ;
}

void CMovieView::KeyDelete( CKey *pKey, CKey *pNewCurrentKey )
{
	ASSERT( m_pMovie ) ;


	int Sure = MessageBox( "Are You Sure?", "Delete Key", MB_ICONWARNING|MB_YESNO ) ;
	if( Sure == IDYES )
	{
		CActor *pActor = m_pMovie->GetCurActor( ) ;
		if( pActor )
		{
			pActor->KeyDelete( pKey ) ;
			KeyUpdateList( ) ;
			KeyUpdateDetails( pNewCurrentKey ) ;
			KeySetCurrent( pNewCurrentKey ) ;
			GetDocument()->SetModifiedFlag( ) ;
		}
	}

}

void CMovieView::KeyUpdateList( )
{
	ASSERT( m_pMovie ) ;
	m_pDlgKey->SetActor( m_pMovie->GetCurActor() ) ;
}

void CMovieView::KeySetCurrent( CKey *pKey )
{	
	static bool	Semaphore = false ;

	ASSERT( m_pMovie ) ;

	if( !Semaphore )
	{
		Semaphore = true ;
		if( m_pMovie->GetCurActor( ) )
		{
			m_pMovie->GetCurActor()->SetCurKey( pKey ) ;
			m_pDlgKey->SelectKey( pKey ) ;
			KeyUpdateDetails( pKey ) ;
			if( pKey )
				SetFrame( pKey->GetFrame() ) ;
		}
		Semaphore = false ;
	}
}

CKey *CMovieView::KeyGetCurrent( )
{
	CKey *pKey = NULL ;

	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor( ) )
	{
		pKey = m_pMovie->GetCurActor()->GetCurKey( ) ;
	}

	return pKey ;
}

void CMovieView::KeyMove( f32 dx, f32 dy, f32 dz )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetPosition( &v ) ;
		v.X += dx ;
		v.Y += dy ;
		v.Z += dz ;
		pKey->SetPosition( &v ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeyMoveV( f32 dx, f32 dy, f32 dz )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetVector( &v ) ;
		v.X += dx ;
		v.Y += dy ;
		v.Z += dz ;
		pKey->SetVector( &v ) ;
		m_pMovie->GetCurActor()->KeySyncDirection( pKey );
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeyMoveR( f32 dx, f32 dy, f32 dz )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  r ;
		pKey->GetRotation( &r ) ;
		r.X += dx ;
		r.Y += dy ;
		r.Z += dz ;
		pKey->SetRotation( &r ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeyUpdateDetails( CKey *pKey )
{
	CActor *pActor = m_pMovie->GetCurActor() ;
	if( pActor )
	{
		pActor->CalculateSplineCache( ) ;
	}
	m_pDlgKey->UpdateDetails( pKey ) ;
	RedrawWindow( ) ;
}

void CMovieView::KeySetX( f32 x )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetPosition( &v ) ;
		v.X = x ;
		pKey->SetPosition( &v ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetY( f32 y )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetPosition( &v ) ;
		v.Y = y ;
		pKey->SetPosition( &v ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetZ( f32 z )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetPosition( &v ) ;
		v.Z = z ;
		pKey->SetPosition( &v ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetVX( f32 x )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetVector( &v ) ;
		v.X = x ;
		pKey->SetVector( &v ) ;
		m_pMovie->GetCurActor()->KeySyncDirection( pKey );

		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetVY( f32 y )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetVector( &v ) ;
		v.Y = y ;
		pKey->SetVector( &v ) ;
		m_pMovie->GetCurActor()->KeySyncDirection( pKey );

		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetVZ( f32 z )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetVector( &v ) ;
		v.Z = z ;
		pKey->SetVector( &v ) ;
		m_pMovie->GetCurActor()->KeySyncDirection( pKey );

		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetRX( f32 x )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetRotation( &v ) ;
		v.X = x ;
		pKey->SetRotation( &v ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetRY( f32 y )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetRotation( &v ) ;
		v.Y = y ;
		pKey->SetRotation( &v ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetRZ( f32 z )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
		vector3  v ;
		pKey->GetRotation( &v ) ;
		v.Z = z ;
		pKey->SetRotation( &v ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateDetails( pKey ) ;
	}
}

void CMovieView::KeySetFrame( int Frame )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CActor *pActor = m_pMovie->GetCurActor() ;
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;

		//---	Keep list sorted while changing Frame
		POSITION Pos = pActor->m_Keys.Find( pKey ) ;
		pActor->m_Keys.RemoveAt( Pos ) ;
		pKey->SetFrame( Frame ) ;
		pActor->m_Keys.InsertSorted( pKey ) ;

		//---	if this motion is the same as the last key's motion, or next key's motion
		//		set their start frames to sink up with this motion
		pActor->KeySyncMotion( pKey );
		pActor->KeySyncDirection( pKey );

		GetDocument()->SetModifiedFlag( TRUE ) ;
		KeyUpdateList( ) ;
		KeyUpdateDetails( pKey ) ;
		Invalidate();
	}
}

void CMovieView::KeyLookAt( )
{
	vector3  v ;

	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CActor *pActor = m_pMovie->GetCurActor() ;
		CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;

		pKey->GetPosition( &v ) ;
		m_Camera.SetTarget( v ) ;
		m_Camera.ComputeMatrix() ;
		RedrawWindow( ) ;
	}
}

void CMovieView::KeySelectAll( bool Select )
{
	CActor *pActor = m_pMovie->GetCurActor() ;
	if( pActor )
	{
		pActor->KeySelectAll( Select ) ;
		Invalidate( ) ;
	}
}

void CMovieView::KeyUpdateMotions( )
{
	ASSERT( m_pMovie ) ;
	CActor *pActor = m_pMovie->GetCurActor() ;
	if( pActor )
	{
		if( pActor->GetType() == ACTOR_CHARACTER )
		{
			m_pDlgKey->SetupMotions( pActor->GetCharacter() ) ;
		}
	}
}

void CMovieView::KeySetAutoLinkDir( bool setting )
{
	ASSERT( m_pMovie ) ;
	CActor *pActor = m_pMovie->GetCurActor() ;
	if( pActor )
	{
		if( pActor->GetType() == ACTOR_CHARACTER )
		{
			CKey *pKey = pActor->GetCurKey() ;
			if( pKey )
			{
				pKey->m_AutoLinkDir = setting ;
				if( pKey->m_AutoLinkDir )
				{
					if( pActor->KeySyncDirection( pKey ) )
					{
						KeyUpdateDetails( pKey );
						Invalidate();
					}
				}
			}
		}
	}
}

void CMovieView::KeySetMotion( CMotion *pMotion )
{
	ASSERT( m_pMovie ) ;
	CActor *pActor = m_pMovie->GetCurActor() ;
	if( pActor )
	{
		if( pActor->GetType() == ACTOR_CHARACTER )
		{
			CKey *pKey = pActor->GetCurKey() ;
			if( pKey )
			{
				pKey->SetCharacter( pActor->GetCharacter() ) ;
				pKey->SetMotion( pMotion ) ;

				//---	if this motion is the same as the last key's motion, or next key's motion
				//		set their start frames to sink up with this motion
				pActor->KeySyncMotion( pKey );
				pActor->KeySyncDirection( pKey );

				KeyUpdateDetails( pKey ) ;
				Invalidate( ) ;
			}
		}
	}
}

void CMovieView::KeySetStartFrame( int StartFrame )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CActor *pActor = m_pMovie->GetCurActor() ;

		if( pActor->GetType() == ACTOR_CHARACTER )
		{
			CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
			pKey->SetStartFrame( StartFrame ) ;

			//---	if this motion is the same as the last key's motion, or next key's motion
			//		set their start frames to sink up with this motion
			bool KeyChanged = pActor->KeySyncMotion( pKey );
			if( KeyChanged )
				pActor->KeySyncDirection( pKey );

			pActor->CalculateSplineCache( ) ;
			if( KeyChanged ) KeyUpdateDetails( pKey ) ;
			Invalidate( ) ;
		}
	}
}


void CMovieView::KeySetFrameRate( float Rate )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CActor *pActor = m_pMovie->GetCurActor() ;

		if( pActor->GetType() == ACTOR_CHARACTER )
		{
			CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
			pKey->SetFrameRate( Rate ) ;
			pActor->CalculateSplineCache( ) ;
			Invalidate( ) ;
		}
	}
}


void CMovieView::KeySetBlendFrames( float Frames )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CActor *pActor = m_pMovie->GetCurActor() ;

		if( pActor->GetType() == ACTOR_CHARACTER )
		{
			CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
			pKey->SetBlendFrames( Frames ) ;
			Invalidate( ) ;
		}
	}
}


void CMovieView::KeySetMirror( bool Mirror )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CActor *pActor = m_pMovie->GetCurActor() ;

		if( pActor->GetType() == ACTOR_CHARACTER )
		{
			CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
			pKey->SetMirror( Mirror ) ;
			pActor->CalculateSplineCache( ) ;
			Invalidate( ) ;
		}
	}
}


void CMovieView::KeySetScale( f32 scale )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CActor *pActor = m_pMovie->GetCurActor() ;

		if( pActor->GetType() == ACTOR_SCENERY )
		{
			CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
			pKey->SetScale( scale ) ;
			Invalidate( ) ;
		}
	}
}

f32	CMovieView::KeyGetScale( )
{
	if( m_pMovie->GetCurActor() && m_pMovie->GetCurActor()->GetCurKey() )
	{
		CActor *pActor = m_pMovie->GetCurActor() ;

		if( pActor->GetType() == ACTOR_SCENERY )
		{
			CKey *pKey = m_pMovie->GetCurActor()->GetCurKey() ;
			return pKey->GetScale();
		}
		else
		{
			return 1.0f;
		}
	}
	else
	{
		return 1.0f;
	}
}

void CMovieView::ShotListInitialize( )
{
	m_pDlgShotList->SetupShotList( m_pMovie ) ;
	m_pDlgShotList->SetupActorLists( m_pMovie ) ;
	m_pDlgShotList->SelectShot( m_pMovie->GetCurShot() ) ;
}

CShot *CMovieView::ShotAdd( )
{
	CShot *pShot = NULL ;

	ASSERT( m_pMovie ) ;

//	if( )
	{
		CShot *pShot = new CShot ;
		if( pShot )
		{
			pShot->SetMovie( m_pMovie ) ;
			pShot->SetFrame( GetFrame() ) ;
			m_pMovie->m_ShotList.InsertSorted( pShot ) ;
			GetDocument()->SetModifiedFlag( ) ;
			ShotListInitialize( ) ;
		}
	}

	return pShot ;
}

void CMovieView::ShotDelete( CShot *pShot )
{
	ASSERT( m_pMovie ) ;

	int Sure = MessageBox( "Are You Sure?", "Delete Shot", MB_ICONWARNING|MB_YESNO ) ;
	if( Sure == IDYES )
	{
		m_pMovie->ShotDelete( pShot ) ;
		ShotListInitialize( ) ;
		GetDocument()->SetModifiedFlag( ) ;
	}
}

void CMovieView::ShotSetFrame( CShot *pShot, int Frame )
{
	ASSERT( m_pMovie ) ;

	if( m_pMovie && pShot )
	{
		//---	Keep list sorted while changing Frame
		POSITION Pos = m_pMovie->m_ShotList.Find( pShot ) ;
		m_pMovie->m_ShotList.RemoveAt( Pos ) ;
		pShot->SetFrame( Frame ) ;
		m_pMovie->m_ShotList.InsertSorted( pShot ) ;
		GetDocument()->SetModifiedFlag( TRUE ) ;
		m_pDlgShotList->SetupShotList( m_pMovie ) ;
		m_pDlgShotList->SelectShot( pShot, false ) ;
	}
}

int	CMovieView::SetFrame( int nFrame )
{
	ASSERT( m_pMovie ) ;
	m_pMovie->SetFrame( nFrame ) ;
	if( m_pMovieBar )
		m_pMovieBar->SetFrame( GetFrame() ) ;
	Invalidate() ;
	return m_pMovie->GetFrame() ;
}

int	CMovieView::GetFrame( )
{
	ASSERT( m_pMovie ) ;
	return m_pMovie->GetFrame() ;
}

int	CMovieView::SetNumFrames( int NumFrames )
{
	ASSERT( m_pMovie ) ;
	m_pMovie->SetNumFrames( NumFrames ) ;
	if( m_pMovieBar )
		m_pMovieBar->SetNumFrames( GetNumFrames() ) ;
	return m_pMovie->GetNumFrames() ;
}

int	CMovieView::GetNumFrames( )
{
	ASSERT( m_pMovie ) ;
	return m_pMovie->GetNumFrames() ;
}

void CMovieView::UpdateMovieBar( )
{
	m_pMovieBar->SetNumFrames( GetNumFrames() ) ;
	m_pMovieBar->SetFrame( GetFrame() ) ;
}





void CMovieView::SetDragPlane( vector3  *pVector )
{
}

bool CMovieView::GetCursorOnDragPlane( vector3  *pCursor, CPoint p, f32 y )
{
	//---	Get Intersection of Ray with Plane
	bool		Intersected = false ;

	//---	Get Cursor Ray
	vector3 	RayBase, Ray ;
	GetCursorRay( &RayBase, &Ray, p ) ;

	//---	Get Point RAY intersects on plane or return Zero if can't intersect
	if( (x_abs(Ray.Y) > 0.01) )
	{
		f32	d = (RayBase.Y-y) / -Ray.Y ;
		if( d >= 0 )
		{
			Ray.Scale( d );
			*pCursor = RayBase + Ray;
//			V3_Scale( &Ray, &Ray, d ) ;
//			V3_Add( pCursor, &RayBase, &Ray ) ;
			pCursor->Y = y ;
			Intersected = true ;
		}
	}
	else
	{
		pCursor->Zero();
//		V3_Zero( pCursor ) ;
	}

	return Intersected ;
}

void CMovieView::SetUseShotList( bool State )
{
	if( m_pMovie )
	{
		m_pMovie->m_UseShotList = State ;
		Invalidate( ) ;
	}
}

bool CMovieView::GetUseShotList( )
{
    bool State = false ;
    if( m_pMovie )
	{
		State = m_pMovie->m_UseShotList ;
	}
    return State ;
}









BEGIN_MESSAGE_MAP(CMovieView, CCeDView)
	//{{AFX_MSG_MAP(CMovieView)
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_STATE_SELECT, OnStateSelect)
	ON_COMMAND(ID_STATE_ROTATE, OnStateRotate)
	ON_COMMAND(ID_STATE_ZOOM, OnStateZoom)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_AXISXZ, OnAxisxz)
	ON_COMMAND(ID_AXISX, OnAxisx)
	ON_COMMAND(ID_AXISY, OnAxisy)
	ON_COMMAND(ID_AXISZ, OnAxisz)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_STATE_MOVE, OnStateMove)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MOVIE_INSERTFRAMES, OnMovieInsertframes)
	ON_COMMAND(ID_MOVIE_DELETEFAMES, OnMovieDeletefames)
	ON_COMMAND(ID_ACTOR_INSERTFRAMES, OnActorInsertframes)
	ON_COMMAND(ID_ACTOR_DELETEFRAMES, OnActorDeleteframes)
	ON_COMMAND(ID_ACTOR_COPY, OnActorCopy)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_MOVIE_EXPORTGAMECUBE, OnMovieExportGAMECUBE)
	ON_COMMAND(ID_MOVIE_EXPORTPC, OnMovieExportPC)
	ON_COMMAND(ID_MOVIE_EXPORTXBOX, OnMovieExportXBOX)
	ON_COMMAND(ID_MOVIE_EXPORTPS2, OnMovieExportPS2)
	ON_COMMAND(ID_MOVIE_MERGE, OnMovieMerge)
	ON_WM_TIMER()
	ON_COMMAND(ID_MOVIES_EXPORTMOVIEASASCII, OnMoviesExportmovieasascii)
	ON_COMMAND(ID_MOVIEVIEW_LAYERSELECT, OnMovieviewLayerselect)
	ON_UPDATE_COMMAND_UI(ID_MOVIEVIEW_LAYERSELECT, OnUpdateMovieviewLayerselect)
	ON_COMMAND(ID_MOVIE_EXPORTPROPERTIES, OnMovieExportproperties)
	ON_COMMAND(ID_MOVIE_EXPORT, OnMovieExport)
	ON_UPDATE_COMMAND_UI(ID_ENABLE_D3D, OnUpdateEnableD3d)
	ON_COMMAND(ID_ENABLE_D3D, OnEnableD3d)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


 
/////////////////////////////////////////////////////////////////////////////
// CMotionEditView drawing

void CMovieView::DrawSkeleton( CRenderContext *pRC, matrix4& rTransform, COLORREF C, CCharacter *pCharacter )
{
	if( !pCharacter )					return;
	if( !pCharacter->GetSkeleton() )	return;
	pRC->RENDER_Character( pCharacter, rTransform, C, RFLAG_BONES|RFLAG_FACETS );
}

/////////////////////////////////////////////////////////////////////////////
// CMovieView drawing

void CMovieView::DrawView( CRenderContext *pRC )
{
	int ViewLayer = m_pDlgLayer->GetViewLayer();

	CCeDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	//---	Setup Camera with parameters from CDC to render to
	CRect	r, cr ;
	GetClientRect( &cr ) ;
    if( m_pMovie->m_UseShotList )
    {
        r.SetRect( CPoint(0,0), CPoint(m_pMovie->GetPreviewWidth(),m_pMovie->GetPreviewHeight()) ) ;
        r.OffsetRect( (cr.Width()/2)-(r.Width()/2),(cr.Height()/2)-(r.Height()/2) ) ;
    }
    else
    {
        r = cr ;
    }

	//---	set the projection area
	m_Camera.SetProjectWindow( (f32)r.left, (f32)r.top, (f32)r.Width(), (f32)r.Height() ) ;

	//---	Setup Camera Target and Rotation / Elevation from ShotList if required
	if( GetUseShotList() )
	{
		m_Camera.SetFOV( m_pMovie->ShotGetFOV( GetFrame() ) );

		CActor *pEye	= m_pMovie->ShotGetEye( GetFrame() ) ;
		CActor *pTarget	= m_pMovie->ShotGetTarget( GetFrame() ) ;
		
		if( pEye && pTarget )
		{
			vector3  ve, vt ;

			pEye->GetPositionAtTime( &ve, GetFrame() ) ;
			pTarget->GetPositionAtTime( &vt, GetFrame() ) ;
	
			m_Camera.SetEyeTarget( ve, vt );
		}
	}
	else
	{
		m_Camera.SetFOV( 60.0f );
	}

	//---	set the view matrix
	SetViewMatrix( m_Camera.ComputeMatrixD3D() );
	SetProjection( m_Camera.ComputeProjMatrixD3D() ) ;
//	SetProjection( m_Camera.m_NZ, m_Camera.m_FZ, DEG_TO_RAD(m_Camera.m_FOV), r.Width(), r.Height() );
		
	//---	Get a Matrix for positioning objects in the world
	matrix4		m ;
	m.Identity();

	//---	Draw Ground Plane
	pRC->PushMatrix( &m ) ;
	{
		CPen PenMajor( PS_SOLID, 1, GRID_COLOR_MAJOR ) ;
		CPen PenMinor( PS_SOLID, 1, GRID_COLOR_MINOR ) ;
		CPen *pOldPen = pRC->SelectObject( &PenMinor ) ;
		pRC->RENDER_GridLines( -960, 960,  48*2, -2160, 2160,  48*2 ) ;
		pRC->SelectObject( &PenMajor ) ;
		pRC->RENDER_GridLines( -960, 960, 240*2, -2160, 2160, 240*2 ) ;
		pRC->SelectObject( pOldPen ) ;
	}
	pRC->PopMatrix( ) ;

	ASSERT( m_pMovie ) ;

	POSITION Pos = m_pMovie->m_ActorList.GetHeadPosition() ;
	while( Pos )
	{
		//---	Draw Path for Actor unless displaying with ShotList
		CActor *pActor = m_pMovie->m_ActorList.GetNext( Pos ) ;
		ASSERT( pActor ) ;

		//---	Draw Actor at position along Path
		if( pActor->m_Keys.GetCount() > 0 )
		{
			//---	Handle Character Actor
			if( pActor->GetType() == ACTOR_CHARACTER )
			{
				//---	if we are not viewing cameras, skip this
				if( !(m_ViewFlags & MVF_VIEWCHARACTERS) || ((ViewLayer != -1) && (pActor->GetLayer() != ViewLayer)))
					continue;

				if( !m_pMovie->m_UseShotList )
					pActor->DrawPath( pRC, pActor->m_Selected ) ;

				if( pActor->m_pCharacter && pActor->m_pCharacter->GetSkeleton() )
				{
					//---	if there is a skin selected, set it now.
					if(( pActor->GetSkin() >= 0 ) && ( pActor->GetSkin() < pActor->m_pCharacter->GetNumSkins() ))
						pActor->m_pCharacter->SetCurSkin( pActor->GetSkin() );

					//---	Get Skeleton Pointer
					CSkeleton *pSkel = pActor->m_pCharacter->GetSkeleton() ;
					pActor->PoseSkeleton( m_pMovie->GetFrame() ) ;

					//---	Get Position
					vector3  p ;
					pActor->GetPositionAtTime( &p, m_pMovie->GetFrame() ) ;

					//---	Setup Matrix
					matrix4 m ;
					m.Identity();
//					M4_Identity( &m ) ;
					radian Yaw = 0 ;
					CKey *pKey = pActor->GetKeyAtTime( m_pMovie->GetFrame() ) ;
					if( pKey )
					{
						if( pKey->m_AMCbased )
						{
							pActor->GetOrientationAtTime( &Yaw, pKey->GetFrame() ) ;
						}
						else
						{
							vector3  v, DeltaMove ;
							CMotion *pMotion ;
							int nFrame ;
							radian FaceDir, MoveDir ;

							pActor->GetDirectionAtTime( &v, m_pMovie->GetFrame() ) ;
							pActor->GetMotionInfoAtTime( &pMotion, &nFrame, &DeltaMove, &FaceDir, &MoveDir, m_pMovie->GetFrame() ) ;
							if( pMotion )
							{
								FaceDir = pMotion->GetFaceDir( pKey->GetStartFrame() % pMotion->GetNumFrames() ) ;
								Yaw = x_atan2(v.X,v.Z) - FaceDir ;
							}
						}
					}

					//---	Render when not using shot list, or when Key is not NULL using shot list
					if( (!m_pMovie->m_UseShotList) || (pKey != NULL) )
					{
                        radian3 r(0,0,0);
                        r.Yaw = Yaw;
						m.SetRotationXYZ( r );
						if (pKey && (pActor->GetType() == ACTOR_SCENERY))
						{
                            vector3  s;
                            s.X = s.Y = s.Z = pKey->GetScale();
							m.Scale( s );
						}
						m.Translate( p );

						//---	Set Appropriate Actor Color
						COLORREF Color ;
						if( pActor->m_Selected )
						{
							if( pKey == NULL )
								Color = RGB(224,192,192) ;
							else
								Color = RGB(255,0,0) ;
						}
						else
						{
							if( pKey == NULL )
								Color = RGB(192,192,192) ;
							else
								Color = RGB(0,0,0) ;
						}

						//---	Push Matrix / Render / Pop Matrix
						pRC->PushMatrix( &m ) ;
#if 1
						DrawSkeleton( pRC, m, Color, pActor->GetCharacter() );
#else
						pSkel->Render( pRC, Color ) ;
#endif
						pRC->PopMatrix( ) ;
					}
				}

				//---	remove the selected skin
				pActor->m_pCharacter->SetCurSkin( -1 );
			}
			else if( pActor->GetType() == ACTOR_CAMERA )// && (#######))
			{
				//---	if we are not viewing cameras, skip this
				if( !(m_ViewFlags & MVF_VIEWCAMERAS) || ((ViewLayer != -1) && (pActor->GetLayer() != ViewLayer)) )
					continue;

				if( !m_pMovie->m_UseShotList )
					pActor->DrawPath( pRC, pActor->m_Selected ) ;

				//---	Only Draw Camera when not using ShotList to Render
				if( !m_pMovie->m_UseShotList )
				{
					//---	Get Position and Info
					vector3  p ;
					pActor->GetPositionAtTime( &p, m_pMovie->GetFrame() ) ;

					//---	Setup Matrix
					matrix4 m ;
					m.Identity();
					m.SetTranslation( p );

					//---	Push Matrix / Render / Pop Matrix
					COLORREF Color = (pActor == GetCurActor()) ? RGB(255,0,0) : RGB(0,0,0) ;
					pRC->PushMatrix( &m ) ;
					pRC->RENDER_Sphere( 20, RGB(255,0,0), RGB(0,255,0), RGB(0,0,255) ) ;
					pRC->PopMatrix( ) ;
				}
			}
			else if( pActor->GetType() == ACTOR_SCENERY )
			{
				//---	if we are not viewing cameras, skip this
				if( !(m_ViewFlags & MVF_VIEWSCENERY) || ((ViewLayer != -1) && (pActor->GetLayer() != ViewLayer)) )
					continue;

				if( !m_pMovie->m_UseShotList )
					pActor->DrawPath( pRC, pActor->m_Selected ) ;

				//---	Get Position and Info
				vector3  p, r ;
				matrix4 OldCameraMatrix;
				matrix4 IdentityMatrix;
/*
				radian Yaw ;
				CKey *pKey = (CKey*)pActor->m_Keys.GetHead( ) ;
				if( pKey )
					pKey->GetVector( &v ) ;
				else
					V3_Zero( &v ) ;
				Yaw = x_atan2(v.X,v.Z) ;
*/
				pActor->GetPositionAtTime( &p, m_pMovie->GetFrame() ) ;
				pActor->GetRotationAtTime( &r, m_pMovie->GetFrame() ) ;
				CKey *pKey = pActor->GetKeyAtTime( m_pMovie->GetFrame() ) ;

				if ( m_pMovie->m_UseShotList )
				{
					// If this key is locked to the camera, preserve old camera matrix
					// and set the current matrix to be camera relative
					if (pKey && pKey->m_CameraLock)
					{
						IdentityMatrix.Identity();
//						M4_Identity( &IdentityMatrix );
						m_Camera.GetMatrix(&OldCameraMatrix);
						m_Camera.SetMatrix(&IdentityMatrix);
					}
				}
				//---	Setup Matrix
				matrix4 m ;

				m.Identity();
				m.SetRotationXYZ( *((radian3*)(&r)) );
				if ( pActor->GetType() == ACTOR_SCENERY )
				{
                    vector3  s;
                    s.X = s.Y = s.Z = pActor->GetScaleAtTime(m_pMovie->GetFrame());
					m.Scale( s );
				}
				m.Translate( p );

				//---	Push Matrix / Render / Pop Matrix
				COLORREF Color;
				if (pActor == GetCurActor() )
				{
					if (pKey && (pKey->m_Visible) )
					{
						Color = RGB(255,0,0);			// Visible and selected
					}
					else
					{	
						Color = RGB(224,192,192);		// Not visible but selected
					}
				}
				else
				{
					if ( pKey && (pKey->m_Visible) )
					{
						Color = RGB(0,0,0);				// Visible but not selected
					}
					else
					{
						Color = RGB(192,192,192);		// Not visible and not selected
					}
				}
				pRC->PushMatrix( &m ) ;
				if( pActor->m_pMesh )
				{
					pActor->m_pMesh->Render( pRC, Color ) ;
				}
				pRC->PopMatrix( ) ;
				if( m_pMovie->m_UseShotList )
				{
					if (pKey && pKey->m_CameraLock )
					{
						// Restore old camera matrix
						m_Camera.SetMatrix(&OldCameraMatrix);
					}
				}

			}
			else // character unknown
			{
				ASSERT(0);
				if( !m_pMovie->m_UseShotList )
					pActor->DrawPath( pRC, pActor->m_Selected ) ;
			}
		}
	}

    if( m_pMovie->m_UseShotList )
    {
        pRC->RENDER_FillSolidRect( 0, 0, r.left, cr.Height(), RGB(0,0,0) ) ;
        pRC->RENDER_FillSolidRect( r.right, 0, cr.Width()-r.right, cr.Height(), RGB(0,0,0) ) ;
        pRC->RENDER_FillSolidRect( r.left, 0, r.Width(), r.top, RGB(0,0,0) ) ;
        pRC->RENDER_FillSolidRect( r.left, r.bottom, r.Width(), cr.Height()-r.bottom, RGB(0,0,0) ) ;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CMovieView diagnostics

#ifdef _DEBUG
void CMovieView::AssertValid() const
{
	CCeDView::AssertValid();
}

void CMovieView::Dump(CDumpContext& dc) const
{
	CCeDView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMovieView message handlers


static void CALLBACK MMTimerCallback( UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 )
{
	CMovieView *pView = (CMovieView*)dwUser ;

	if( pView->m_Playing && (pView->m_nTimerMessages == 0) )
	{
		::PostMessage( (HWND)pView->GetSafeHwnd(), WM_TIMER, 0, 0 ) ;
		pView->m_nTimerMessages++ ;
	}
}

void CMovieView::OnInitialUpdate() 
{
	CCeDView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class

	//---	Get Pointer to Frame
	CMovieFrame *pFrame = GetFrameWnd() ;
	ASSERT( pFrame ) ;

	//---	Grab Movie Pointer from Frame
	m_pMovie = pFrame->m_pMovie ;

	//---	Set View Pointer in MultiBar
	pFrame->m_RollupBar.m_pView = this ;

	//---	Set View Pointer in Dialogs
	m_pDlgLayer					= &pFrame->m_RollupBar.m_DlgLayer ;
	m_pDlgActorList				= &pFrame->m_RollupBar.m_DlgActorList ;
	m_pDlgKey					= &pFrame->m_RollupBar.m_DlgKey ;
	m_pDlgShotList				= &pFrame->m_RollupBar.m_DlgShotList ;
	m_pDlgMoviePreview			= &pFrame->m_RollupBar.m_DlgMoviePreview ;

	//---	give the dialogs pointers to this view
	m_pDlgLayer->m_pView		= this;
	m_pDlgActorList->m_pView	= this ;
	m_pDlgKey->m_pView			= this ;
	m_pDlgShotList->m_pView		= this ;
	m_pDlgMoviePreview->m_pView	= this ;

    //---	Initialize ToolBar Buttons
	pFrame->m_ToolBar.SetButtonStyle( pFrame->m_ToolBar.CommandToIndex( ID_STATE_SELECT ), TBBS_CHECKBOX ) ;
	pFrame->m_ToolBar.SetButtonStyle( pFrame->m_ToolBar.CommandToIndex( ID_STATE_MOVE ), TBBS_CHECKBOX ) ;
	pFrame->m_ToolBar.SetButtonStyle( pFrame->m_ToolBar.CommandToIndex( ID_STATE_ROTATE ), TBBS_CHECKBOX ) ;
	pFrame->m_ToolBar.SetButtonStyle( pFrame->m_ToolBar.CommandToIndex( ID_STATE_ZOOM ), TBBS_CHECKBOX ) ;

	pFrame->m_ToolBar.SetButtonStyle( pFrame->m_ToolBar.CommandToIndex( ID_AXISXZ ), TBBS_CHECKBOX ) ;
	pFrame->m_ToolBar.SetButtonStyle( pFrame->m_ToolBar.CommandToIndex( ID_AXISX ), TBBS_CHECKBOX ) ;
	pFrame->m_ToolBar.SetButtonStyle( pFrame->m_ToolBar.CommandToIndex( ID_AXISY ), TBBS_CHECKBOX ) ;
	pFrame->m_ToolBar.SetButtonStyle( pFrame->m_ToolBar.CommandToIndex( ID_AXISZ ), TBBS_CHECKBOX ) ;

	//---	Set MovieControlBar to point to this View
	m_pMovieBar = &GetFrameWnd()->m_MovieBar ;
	m_pMovieBar->SetView( this ) ;

	m_pDlgLayer->UpdateDetails();
	ActorUpdateList( ) ;
	KeyUpdateList( ) ;
	KeyUpdateDetails( KeyGetCurrent() ) ;
	UpdateMovieBar( ) ;

	//---	Set Start Mode
	SetViewState( MOVS_CAMERA_ROTATE ) ;	
	SetMoveState( ID_AXISXZ ) ;

//	err Error = QRAW_LoadQRaw( &m_QRaw, "f:\\projects\\qbc99\\art\\ingame\\stadiums\\giants\\gidstad.asc" ) ;
//	if( Error != ERR_SUCCESS )
//	{
//		MessageBox( "QRAW Error", "Error", MB_ICONWARNING ) ;
//	}

	//---	Setup Multimedia Timer
	m_TimerEvent = timeSetEvent( 17, 1, MMTimerCallback, (DWORD)this, TIME_CALLBACK_FUNCTION | TIME_PERIODIC ) ;
    ASSERT( m_TimerEvent ) ;
}

void CMovieView::UpdateCursorState( )
{
	if( m_MouseOverKey )
	{
		SetCursor (AfxGetApp()->LoadCursor( IDC_KEY )) ;
		return ;
	}

	switch( m_ViewState )
	{
	case MOVS_SELECT:
		SetCursor (AfxGetApp()->LoadStandardCursor( IDC_ARROW )) ;
		break ;
	case MOVS_CAMERA_MOVE:
		SetCursor (AfxGetApp()->LoadCursor( IDC_PAN )) ;
		break ;
	case MOVS_CAMERA_ROTATE:
		SetCursor (AfxGetApp()->LoadCursor( IDC_ROTATE )) ;
		break ;
	case MOVS_CAMERA_ZOOM:
		SetCursor (AfxGetApp()->LoadCursor( IDC_ZOOM )) ;
		break ;
	case MOVS_DRAG_KEY:
	case MOVS_DRAG_KEYVECTOR:
		SetCursor (AfxGetApp()->LoadCursor( IDC_INVERSE )) ;
		break ;
	} ;
}

BOOL CMovieView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default

	//---	Set the Cursor
	if( nHitTest == HTCLIENT )
	{
		UpdateCursorState( ) ;
		return 0 ;
	}

	return CCeDView::OnSetCursor(pWnd, nHitTest, message);
}

void CMovieView::SetViewState( int ViewState )
{
	m_ViewState = ViewState ;

	CMovieFrame *pFrame = GetFrameWnd() ;
	CToolBarCtrl& ToolBar = pFrame->m_ToolBar.GetToolBarCtrl( ) ;

	ToolBar.CheckButton( ID_STATE_SELECT,		m_ViewState == MOVS_SELECT ) ;
	ToolBar.CheckButton( ID_STATE_MOVE,			m_ViewState == MOVS_CAMERA_MOVE ) ;
	ToolBar.CheckButton( ID_STATE_ROTATE,		m_ViewState == MOVS_CAMERA_ROTATE ) ;
	ToolBar.CheckButton( ID_STATE_ZOOM,			m_ViewState == MOVS_CAMERA_ZOOM ) ;

	UpdateCursorState() ;
	Invalidate() ;
}

void CMovieView::SetViewLayer( s32 ViewLayer )
{
	m_ViewLayer = ViewLayer;
}

void CMovieView::ModifyViewFlags( s32 ClearFlags, s32 SetFlags )
{
	m_ViewFlags &= ~ClearFlags;
	m_ViewFlags |= SetFlags;
}

void CMovieView::OnStateSelect() 
{
	// TODO: Add your command handler code here
	SetViewState( MOVS_SELECT ) ;	
}

void CMovieView::OnStateMove() 
{
	// TODO: Add your command handler code here
	SetViewState( MOVS_CAMERA_MOVE ) ;
}

void CMovieView::OnStateRotate() 
{
	// TODO: Add your command handler code here
	SetViewState( MOVS_CAMERA_ROTATE ) ;	
}

void CMovieView::OnStateZoom() 
{
	// TODO: Add your command handler code here
	SetViewState( MOVS_CAMERA_ZOOM ) ;	
}

void CMovieView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	SetCapture() ;

	CPoint	Point ;
	GetCursorPos( &Point ) ;
	ScreenToClient( &Point ) ;
	m_MouseLast = Point ;

	//---	Check for Clicking on a Key Item
	int SubMode ;
	CActor *pActor = m_pMovie->GetCurActor() ;
	CKey *pKey = PickGetKey( &SubMode, point, false ) ;
	if( pKey && m_pMovie && !m_pMovie->m_UseShotList )
	{
		KeySetCurrent( pKey ) ;

		//---	Handle Selection States of Keys
		if( !pKey->m_Selected )
		{
			//---	Key is not Selected
			if( (nFlags & MK_CONTROL) )
			{
				pKey->m_Selected = !pKey->m_Selected ;
			}
			else
			{
				pActor->KeySelectAll( false ) ;
				pKey->m_Selected = true ;
			}
		}
		else
		{
			//---	Key is Selected
			if( (nFlags & MK_CONTROL) )
			{
				pKey->m_Selected = !pKey->m_Selected ;
			}
			else
			{
			}
		}

		//---	Setup all Dragging variables
		if( pKey->m_Selected )
		{
			m_DragOldViewState = m_ViewState ;
			m_pDragKey = pKey ;
			vector3  v ;
			pKey->GetPosition( &v ) ;
			if( SubMode == 1 )
			{
				vector3  v2 ;
				pKey->GetVector( &v2 ) ;
				v += v2;
//				V3_Add( &v, &v, &v2 ) ;
			}
			vector3  Cursor ;
			if( GetCursorOnDragPlane( &Cursor, point, v.Y ) )
			{
				m_DragOffset = Cursor - v;
//				V3_Sub( &m_DragOffset, &Cursor, &v ) ;
				if( SubMode == 0 )
					SetViewState( MOVS_DRAG_KEY ) ;
				else
					SetViewState( MOVS_DRAG_KEYVECTOR ) ;
			}
	
			OnMouseMove( MK_LBUTTON, Point ) ;
		}
	}
	else
	{
		//---	Nothing Clicked
	}

	CCeDView::OnLButtonDown(nFlags, point);
}

void CMovieView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	//---	Check for Clicking on a Key Item
	int SubMode ;
	CKey *pKey = PickGetKey( &SubMode, point, false ) ;
	if( pKey )
	{
		KeySetCurrent( pKey ) ;
		KeyLookAt( ) ;
	}

	CCeDView::OnLButtonDblClk(nFlags, point);
}

void CMovieView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture() ;

	if( (m_ViewState == MOVS_DRAG_KEY) || (m_ViewState == MOVS_DRAG_KEYVECTOR) )
	{
		SetViewState( m_DragOldViewState ) ;
	}

	CCeDView::OnLButtonUp(nFlags, point);
}

CKey *CMovieView::PickGetKey( int *pSubMode, CPoint p, bool VectorPriority )
{
	CKey *pFound = NULL ;

	//---	Get Ray Base and Vector
	vector3  RayBase, Ray ;
	GetCursorRay( &RayBase, &Ray, p ) ;

	POSITION PosActor = m_pMovie->m_ActorList.GetHeadPosition() ;
	while( PosActor )
	{
		//---	Get Actor and check for intersection on Keys
		CActor *pActor = m_pMovie->m_ActorList.GetNext( PosActor ) ;
		if( pActor && pActor->m_Selected )
		{
			POSITION Pos = pActor->m_Keys.GetHeadPosition() ;
			while( Pos )
			{
				CKey *pKey = pActor->m_Keys.GetNext( Pos ) ;
				ASSERT( pKey ) ;
				vector3  Center ;
				vector3  Vector ;
				pKey->GetPosition( &Center ) ;
				pKey->GetVector( &Vector ) ;
				if( IntersectRaySphere( &RayBase, &Ray, &Center, 8 ) )
				{
					pFound = pKey ;
					if( pSubMode ) *pSubMode = 0 ;
					if( !VectorPriority )
						break ;
				}
				Center += Vector;
//				V3_Add( &Center, &Center, &Vector ) ;
				if( IntersectRaySphere( &RayBase, &Ray, &Center, 8 ) )
				{
					pFound = pKey ;
					if( pSubMode ) *pSubMode = 1 ;
					break ;
				}
			}
		}
	}

	return pFound ;
}

void CMovieView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CCeDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	//---	Handle Delta Mouse Movement
	m_MouseDelta = point - m_MouseLast ;
	m_MouseLast = point ;
	m_MouseOverKey = false ;

	//---	Check for Moving over an Item
	int SubMode ;
	CKey *pKey = PickGetKey( &SubMode, point, false ) ;
	if( pKey )
	{
		m_MouseOverKey = true ;
	}

	//---	Handle Dragging Keys
	if( (nFlags & (MK_LBUTTON|MK_RBUTTON)) && (m_ViewState == MOVS_DRAG_KEY) && (GetCapture() == this) )
	{
		//---	Get Cursor Drag Plane intersection point
		vector3 	v, Cursor ;
		m_pDragKey->GetPosition( &v ) ;
		if( GetCursorOnDragPlane( &Cursor, point, v.Y ) )
		{
			//---	Offset for where Dragging began
			Cursor -= m_DragOffset;

			//---	Get Delta to Move Selected Keys by
			Cursor.Y = v.Y ;
			vector3 	OldPosition ;
			vector3 	Delta ;
			m_pDragKey->GetPosition( &OldPosition ) ;
			Delta = Cursor - OldPosition;

			CActor *pActor = m_pMovie->GetCurActor() ;
			if( pActor )
			{
				pActor->KeyDrag( &Delta ) ;
			}

			Invalidate( ) ;
			GetDocument()->SetModifiedFlag( TRUE ) ;
			KeyUpdateDetails( m_pDragKey ) ;
		}
	}

	//---	Handle Dragging Key Vectors
	if( (nFlags & (MK_LBUTTON|MK_RBUTTON)) && (m_ViewState == MOVS_DRAG_KEYVECTOR) && (GetCapture() == this) )
	{
		//---	Get Cursor Drag Plane intersection point
		vector3 	v1, v2, Cursor ;
		m_pDragKey->GetPosition( &v1 ) ;
		m_pDragKey->GetVector( &v2 ) ;
		v1 += v2;

		if( GetCursorOnDragPlane( &Cursor, point, v1.Y ) )
		{
			//---	Offset for where Dragging began
			Cursor -= m_DragOffset;

			//---	Set the new KeyPosiiton vector
			vector3 	v ;
			m_pDragKey->GetPosition( &v ) ;
			Cursor -= v;
			Cursor.Y = v2.Y ;
			m_pDragKey->SetVector( &Cursor ) ;
			GetCurActor()->KeySyncDirection( m_pDragKey );
			Invalidate( ) ;
			GetDocument()->SetModifiedFlag( TRUE ) ;
			KeyUpdateDetails( m_pDragKey ) ;
		}
	}

	//---	Handle PAN
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MOVS_CAMERA_MOVE) && (GetCapture() == this) )
	{
		matrix4		m ;
		vector3 	moveSrc ;
		vector3 	moveDst ;
		vector3 	Target ;

		m.Identity();
		m.RotateY( -m_Camera.GetRotation() );

		moveSrc.Set( (f32)m_MouseDelta.x, 0, (f32)m_MouseDelta.y ) ;
		moveSrc.Scale( m_Camera.GetDistance() / 1000 ) ;
		m.Transform( &moveDst, &moveSrc, 1 ) ;

		m_Camera.GetTarget( &Target ) ;
		Target += moveDst;

		m_Camera.SetTarget( Target ) ;

		m_Camera.ComputeMatrix() ;

		Invalidate() ;
	}

	//---	Handle ROTATE
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MOVS_CAMERA_ROTATE) && (GetCapture() == this) )
	{
		m_Camera.SetRotation( m_Camera.GetRotation() + (radian)m_MouseDelta.x / 200 ) ;
		m_Camera.SetElevation( m_Camera.GetElevation() - (radian)m_MouseDelta.y / 200 ) ;

		m_Camera.ComputeMatrix() ;
		
		Invalidate() ;
	}

	//---	Handle ZOOM
	if( (nFlags & MK_LBUTTON) && (m_ViewState == MOVS_CAMERA_ZOOM) && (GetCapture() == this) )
	{
		m_Camera.SetDistance( m_Camera.GetDistance() + (f32)m_MouseDelta.y*4 ) ;
		if( m_Camera.GetDistance() < 10 ) m_Camera.SetDistance( 10 ) ;

		m_Camera.ComputeMatrix() ;

		Invalidate() ;
	}
	
	CCeDView::OnMouseMove(nFlags, point);
}

BOOL CMovieView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default

	return 0 ;
//	return CCeDView::OnEraseBkgnd(pDC);
}

void CMovieView::OnAxisxz() 
{
	// TODO: Add your command handler code here
	SetMoveState( ID_AXISXZ ) ;
}

void CMovieView::OnAxisx() 
{
	// TODO: Add your command handler code here
	SetMoveState( ID_AXISX ) ;
}

void CMovieView::OnAxisy() 
{
	// TODO: Add your command handler code here
	SetMoveState( ID_AXISY ) ;
}

void CMovieView::OnAxisz() 
{
	// TODO: Add your command handler code here
	SetMoveState( ID_AXISZ ) ;
}

void CMovieView::SetMoveState( int ID )
{
	CMovieFrame *pFrame = GetFrameWnd() ;
	CToolBarCtrl& ToolBar = pFrame->m_ToolBar.GetToolBarCtrl( ) ;

	switch( ID )
	{
	case ID_AXISXZ:
		m_MoveX = true ;
		m_MoveY = false ;
		m_MoveZ = true ;
		break ;
	case ID_AXISX:
		m_MoveX = true ;
		m_MoveY = false ;
		m_MoveZ = false ;
		break ;
	case ID_AXISY:
		m_MoveX = false ;
		m_MoveY = true ;
		m_MoveZ = false ;
		break ;
	case ID_AXISZ:
		m_MoveX = false ;
		m_MoveY = false ;
		m_MoveZ = true ;
		break ;
	}

	ToolBar.CheckButton( ID_AXISXZ,		 m_MoveX && !m_MoveY &&  m_MoveZ ) ;
	ToolBar.CheckButton( ID_AXISX,		 m_MoveX && !m_MoveY && !m_MoveZ ) ;
	ToolBar.CheckButton( ID_AXISY,		!m_MoveX &&  m_MoveY && !m_MoveZ ) ;
	ToolBar.CheckButton( ID_AXISZ,		!m_MoveX && !m_MoveY &&  m_MoveZ ) ;
}

void CMovieView::SetActorType( int ActorType )
{
	m_ActorType = ActorType ;
	ActorUpdateList( ) ;
}

int CMovieView::GetActorType( )
{
	return m_ActorType ;
}

void CMovieView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CMenu menu ;
	VERIFY(menu.LoadMenu(IDR_MOVIE_POPUP)) ;
	CMenu* pPopup = menu.GetSubMenu(0) ;
	ASSERT(pPopup != NULL) ;

	//---	Enable Menu Items Accordingly
	CCeDDoc	*pDoc = GetDocument() ;
	if( pDoc )
	{
		pDoc->UpdateMenuItems( pPopup ) ;
	}

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this) ;
}

void CMovieView::OnMovieInsertframes() 
{
	// TODO: Add your command handler code here
	ASSERT( m_pMovie ) ;
	CNumFrames	Dlg ;
	Dlg.m_Title = "Insert Frames (All Actors)" ;
	if( Dlg.DoModal( ) == IDOK )
	{
		m_pMovie->InsertFrames( GetFrame(), Dlg.m_NumFrames ) ;
		KeyUpdateList( ) ;
		KeyUpdateDetails( KeyGetCurrent() ) ;
		KeyUpdateMotions( ) ;
		ShotListInitialize( ) ;
	}
}

void CMovieView::OnMovieDeletefames() 
{
	// TODO: Add your command handler code here
	ASSERT( m_pMovie ) ;
	CNumFrames	Dlg ;
	Dlg.m_Title = "Insert Frames (All Actors)" ;
	if( Dlg.DoModal( ) == IDOK )
	{
		m_pMovie->DeleteFrames( GetFrame(), Dlg.m_NumFrames ) ;
		KeyUpdateList( ) ;
		KeyUpdateDetails( KeyGetCurrent() ) ;
		KeyUpdateMotions( ) ;
		ShotListInitialize( ) ;
	}
}

void CMovieView::OnActorInsertframes() 
{
	// TODO: Add your command handler code here
	ASSERT( m_pMovie ) ;
	CActor *pActor = GetCurActor( ) ;
	if( pActor )
	{
		CNumFrames	Dlg ;
		Dlg.m_Title = "Insert Frames (All Actors)" ;
		if( Dlg.DoModal( ) == IDOK )
		{
			pActor->InsertFrames( GetFrame(), Dlg.m_NumFrames ) ;
			KeyUpdateList( ) ;
			KeyUpdateDetails( KeyGetCurrent() ) ;
			KeyUpdateMotions( ) ;
		}
	}
}

void CMovieView::OnActorDeleteframes() 
{
	// TODO: Add your command handler code here
	ASSERT( m_pMovie ) ;
	CActor *pActor = GetCurActor( ) ;
	if( pActor )
	{
		CNumFrames	Dlg ;
		Dlg.m_Title = "Insert Frames (All Actors)" ;
		if( Dlg.DoModal( ) == IDOK )
		{
			pActor->DeleteFrames( GetFrame(), Dlg.m_NumFrames ) ;
			KeyUpdateList( ) ;
			KeyUpdateDetails( KeyGetCurrent() ) ;
			KeyUpdateMotions( ) ;
		}
	}
}

void CMovieView::OnActorCopy() 
{
	// TODO: Add your command handler code here
	ASSERT( m_pMovie ) ;
	CActor *pActor = GetCurActor( ) ;
	if( pActor )
	{
		CActor *pNewActor = new CActor ;
		ASSERT( pNewActor ) ;
		if( pNewActor )
		{
			*pNewActor = *pActor ;
			m_pMovie->m_ActorList.AddTail( pNewActor ) ;
			pNewActor->KeySelectAll( true ) ;
			vector3  v( 50, 0, 50 );
			pNewActor->KeyDrag( &v ) ;

			ActorUpdateList( ) ;
			ActorSetCurrent( pNewActor ) ;
		}
	}
}

void CMovieView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	SetCapture() ;

	CPoint	Point ;
	GetCursorPos( &Point ) ;
	ScreenToClient( &Point ) ;
	m_MouseLast = Point ;

	//---	Check for Clicking on a Key Item
	int SubMode ;
	CActor *pActor = m_pMovie->GetCurActor() ;
	CKey *pKey = PickGetKey( &SubMode, point, true ) ;
	if( pKey )
	{
		KeySetCurrent( pKey ) ;

		//---	Handle Selection States of Keys
		if( !pKey->m_Selected )
		{
			//---	Key is not Selected
			if( (nFlags & MK_CONTROL) )
			{
				pKey->m_Selected = !pKey->m_Selected ;
			}
			else
			{
				pActor->KeySelectAll( false ) ;
				pKey->m_Selected = true ;
			}
		}
		else
		{
			//---	Key is Selected
			if( (nFlags & MK_CONTROL) )
			{
				pKey->m_Selected = !pKey->m_Selected ;
			}
			else
			{
			}
		}

		//---	Setup all Dragging variables
		if( pKey->m_Selected )
		{
			m_DragOldViewState = m_ViewState ;
			m_pDragKey = pKey ;
			vector3  v ;
			pKey->GetPosition( &v ) ;
			if( SubMode == 1 )
			{
				vector3  v2 ;
				pKey->GetVector( &v2 ) ;
				v += v2;
//				V3_Add( &v, &v, &v2 ) ;
			}
			vector3  Cursor ;
			if( GetCursorOnDragPlane( &Cursor, point, v.Y ) )
			{
				m_DragOffset = Cursor - v;
//				V3_Sub( &m_DragOffset, &Cursor, &v ) ;
				if( SubMode == 0 )
					SetViewState( MOVS_DRAG_KEY ) ;
				else
					SetViewState( MOVS_DRAG_KEYVECTOR ) ;
			}
	
			OnMouseMove( MK_LBUTTON, Point ) ;
		}
	}
	else
	{
		//---	Nothing Clicked
		CCeDView::OnRButtonDown(nFlags, point);
	}
}

void CMovieView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture() ;

	if( (m_ViewState == MOVS_DRAG_KEY) || (m_ViewState == MOVS_DRAG_KEYVECTOR) )
	{
		SetViewState( m_DragOldViewState ) ;
	}
	else
		CCeDView::OnRButtonUp(nFlags, point);
}

void CMovieView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	switch( lHint )
	{
	case HINT_D3DENABLE:
		CD3DView::EnableD3D( TRUE );
		OnUpdate( NULL, 0, NULL );
		break ;
	case HINT_D3DDISABLE:
		CD3DView::EnableD3D( FALSE );
		OnUpdate( NULL, 0, NULL );
		break ;
	}
	
	if( m_pMovie )
	{
		KeyUpdateList( ) ;
		KeyUpdateDetails( KeyGetCurrent() ) ;
		KeyUpdateMotions( ) ;
		RedrawWindow( ) ;
	}
}

void CMovieView::OnMovieExportGAMECUBE() 
{
	GetDocument()->MovieExport( EXPORT_GAMECUBE, m_pMovie ) ;
}

void CMovieView::OnMovieExportPC() 
{
	GetDocument()->MovieExport( EXPORT_PC, m_pMovie ) ;
}

void CMovieView::OnMovieExportXBOX() 
{
	GetDocument()->MovieExport( EXPORT_XBOX, m_pMovie ) ;
}

void CMovieView::OnMovieExportPS2() 
{
	GetDocument()->MovieExport( EXPORT_PS2, m_pMovie ) ;
}


void CMovieView::OnMovieMerge() 
{
	// TODO: Add your command handler code here

	//---	Display Dialog to Select Movie to Merge
	if( m_pMovie )
	{
		CDlgMovieMerge	Merge ;
		Merge.m_pMovie = m_pMovie ;
		Merge.m_pDoc = GetDocument( ) ;

		//---	Check for Selection and OK
		if( Merge.DoModal() == IDOK )
		{
			//---	Have Movie to Merge Now
			if( Merge.m_pMergeMovie != NULL )
			{
				m_pMovie->Merge( Merge.m_pMergeMovie ) ;
				GetDocument()->SetModifiedFlag( true ) ;
				GetDocument()->UpdateAllViews( NULL ) ;
			}
		}
	}
}

static s32 GetTimeMS( )
{
	MMTIME	mmt ;
	mmt.wType = TIME_MS ;
	timeGetSystemTime( &mmt, sizeof(MMTIME) ) ;
	return mmt.u.ms ;
}

void CMovieView::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
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
				SetFrame( (GetFrame()+1) % m_pMovie->GetNumFrames() ) ;
			}
			m_LastFrac = (f32)(delta - (nFrames * (1000.0/60.0))) ;
			Invalidate() ;
		}
		else
		{
			//---	Play a Frame at a Time
    		SetFrame( (GetFrame()+1) % m_pMovie->GetNumFrames() ) ;
			Invalidate() ;
		}
		m_nTimerMessages-- ;
	}

    CCeDView::OnTimer(nIDEvent);
}

void CMovieView::OnMoviesExportmovieasascii() 
{
	// TODO: Add your command handler code here
	
	if( m_pMovie )
	{
		CFileDialog fd( FALSE, NULL, NULL ) ;

		fd.m_ofn.lpstrTitle = "Export Movie As ASCII" ;
		fd.m_ofn.lpstrFilter = "All Files (*.*)\000*.*\000" ;

		if( fd.DoModal() == IDOK )
		{
			CString FileName = fd.GetPathName( ) ;
			GetDocument()->MovieExportASCII( FileName, m_pMovie ) ;
		}
	}
}

int glbvar = 1;	

void CMovieView::OnMovieviewLayerselect() 
{
}

void CMovieView::OnUpdateMovieviewLayerselect(CCmdUI* pCmdUI) 
{
	glbvar = 1;	
}

void CMovieView::OnMovieExportproperties() 
{
	if( m_pMovie == NULL )
		return;

	GetDocument()->MovieExportProperties( m_pMovie );	
}

void CMovieView::OnMovieExport() 
{
	if( m_pMovie == NULL )
		return;

	GetDocument()->MovieExport( 0, m_pMovie );	
}

void CMovieView::OnEnableD3d() 
{
	CD3DView::OnEnableD3d();
	OnUpdate( NULL, 0, 0 );
}

void CMovieView::OnUpdateEnableD3d(CCmdUI* pCmdUI) 
{
	CD3DView::OnUpdateEnableD3d( pCmdUI );	
}

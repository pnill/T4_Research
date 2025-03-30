// TransitionViewer.cpp : implementation file
//






#include "stdafx.h"
#include "ced.h"
#include "TransitionViewer.h"
#include "MotionEditView.h"
#include "Motion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTransitionViewer dialog


CTransitionViewer::CTransitionViewer(CWnd* pParent /*=NULL*/)
	: CDialog(CTransitionViewer::IDD, pParent)
{
	m_CurrentPlayIndex	= -1;
	m_TotalFrames		= 0;
	m_pView				= NULL;
	m_bCycle			= FALSE;

	//{{AFX_DATA_INIT(CTransitionViewer)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTransitionViewer::~CTransitionViewer()
{
}


void CTransitionViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTransitionViewer)
	DDX_Control(pDX, IDC_ENDFRAMETOEND, m_EndFrameToEnd);
	DDX_Control(pDX, IDC_ENDFRAMESSPINNER, m_EndFrameSpin);
	DDX_Control(pDX, IDC_ENDFRAME, m_EndFrame);
	DDX_Control(pDX, IDC_STARTFRAME, m_StartFrame);
	DDX_Control(pDX, IDC_STARTFRAMESSPINNER, m_StartFrameSpin);
	DDX_Control(pDX, IDC_BLENDFRAMESSPINNER, m_BlendFramesSpin);
	DDX_Control(pDX, IDC_ORDERDOWN, m_DOWN);
	DDX_Control(pDX, IDC_ORDERUP, m_UP);
	DDX_Control(pDX, IDC_TRANSITIONLIST, m_TransitionList);
	DDX_Control(pDX, IDC_BLENDFRAMES, m_BlendFrames);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTransitionViewer, CDialog)
	//{{AFX_MSG_MAP(CTransitionViewer)
	ON_BN_CLICKED(IDC_ORDERUP, OnOrderup)
	ON_BN_CLICKED(IDC_ORDERDOWN, OnOrderdown)
	ON_NOTIFY(NM_DBLCLK, IDC_TRANSITIONLIST, OnDblclkTransitionlist)
	ON_NOTIFY(NM_CLICK, IDC_TRANSITIONLIST, OnClickTransitionlist)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_TRANSITIONLIST, OnBegindragTransitionlist)
	ON_EN_CHANGE(IDC_BLENDFRAMES, OnChangeBlendframes)
	ON_EN_CHANGE(IDC_ENDFRAME, OnChangeEndframe)
	ON_EN_CHANGE(IDC_STARTFRAME, OnChangeStartframe)
	ON_BN_CLICKED(IDC_ENDFRAMETOEND, OnEndframetoend)
	ON_BN_CLICKED(IDC_PLAYPAUSE, OnPlaypause)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_WM_ACTIVATE()
	ON_BN_CLICKED(IDC_CYCLE, OnCycle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransitionViewer message handlers

BOOL CTransitionViewer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_TransitionList.InsertColumn( 0, "Motion Name", LVCFMT_LEFT, 268 );
	m_TransitionList.InsertColumn( 1, "Blend Frames", LVCFMT_RIGHT, 50 );
	m_TransitionList.InsertColumn( 2, "Start Frame", LVCFMT_RIGHT, 50 );
	m_TransitionList.InsertColumn( 3, "End Frame", LVCFMT_RIGHT, 50 );

	m_ImageList.Create( IDR_TRANSITIONVIEW, 16, 2, RGB(0,0,0) );
	m_TransitionList.SetImageList( &m_ImageList, LVSIL_SMALL );

	m_BlendFramesSpin.SetRange( 0, 15000 );
	m_StartFrameSpin.SetRange( 0, 15000 );
	m_EndFrameSpin.SetRange( 0, 15000 );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=============================================================================
void CTransitionViewer::OnOK() 
{
	m_pView->ToggleTransitionViewer();
}

//=============================================================================
void CTransitionViewer::SetSelected( s32* Selected, s32 count )
{
	radian rotY(0);
	vector3 trans(0,0,0);
	radian3 rEnd, rStart;
	vector3 tEnd, tStart;
	char str[10];
	POSITION pos;
	CAnimTransData* pTrans;
	s32 i;

	m_bSettingSelected = TRUE;

	if( count == 0 )
		i = -1;
	else
		i = Selected[0];

	if( i == -1 )
	{
		m_UP.EnableWindow( FALSE );
		m_DOWN.EnableWindow( FALSE );
		m_BlendFrames.EnableWindow( FALSE );
		m_BlendFramesSpin.EnableWindow( FALSE );
		m_StartFrame.EnableWindow( FALSE );
		m_StartFrameSpin.EnableWindow( FALSE );
		m_EndFrame.EnableWindow( FALSE );
		m_EndFrameSpin.EnableWindow( FALSE );
		m_EndFrameToEnd.EnableWindow( FALSE );

		m_BlendFrames.SetWindowText( "" );
		m_StartFrame.SetWindowText( "" );
		m_EndFrame.SetWindowText( "" );
	}
	else
	{
		m_UP.EnableWindow( (i == 0) ? FALSE : TRUE );
		m_DOWN.EnableWindow( (Selected[count-1] == m_TransitionList.GetItemCount()-1 ) ? FALSE : TRUE );
		m_BlendFrames.EnableWindow( TRUE );
		m_BlendFramesSpin.EnableWindow( TRUE );
		m_StartFrame.EnableWindow( TRUE );
		m_StartFrameSpin.EnableWindow( TRUE );
		m_EndFrame.EnableWindow( TRUE );
		m_EndFrameSpin.EnableWindow( TRUE );
		m_EndFrameToEnd.EnableWindow( TRUE );

		pos = m_AnimTransList.FindIndex( i );
		ASSERT(pos);
		pTrans = m_AnimTransList.GetAt( pos );

		m_BlendFrames.SetWindowText( itoa( pTrans->BlendFrames, str, 10 ) );
		m_StartFrame.SetWindowText( itoa( pTrans->StartFrame, str, 10 ) );
		m_EndFrame.SetWindowText( itoa( pTrans->EndFrame, str, 10 ) );
	}

	s32 TotalFrames = 0;
	if( count )
	{
		//---	reset the selected items as selected
		bool SelectedCurrentlyPlayed = FALSE;
		m_TransitionList.SetSelectionMark( *Selected );
		for( s32 j=0; j<count; j++ )
		{
			m_TransitionList.SetItemState( Selected[j], LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED );

			pTrans = m_AnimTransList.GetAt( m_AnimTransList.FindIndex( Selected[j] ) );
			TotalFrames += pTrans->EndFrame - pTrans->StartFrame + 1;

			//---	subtract the start translation
			pTrans->pMotion->GetTranslation( tStart, pTrans->StartFrame );
			pTrans->pMotion->GetRotation( rStart, pTrans->StartFrame );
			pTrans->pMotion->GetTranslation( tEnd, pTrans->EndFrame );
			pTrans->pMotion->GetRotation( rEnd, pTrans->EndFrame );

			//---	remove any initial rotation from the motion's direction
			rotY -= rStart.Y;

			//---	rotate the start and end translations by the rotation which will be applied to this
			//		motion.
			tStart.RotateY( rotY );
			tEnd.RotateY( rotY );

			//---	remove the start offset from the overall translation
			trans.X -= tStart.X;
			trans.Z -= tStart.Z;

			//---	build this motion's orientation matrix using the current rotation and translation
			pTrans->Orient.Identity();
			pTrans->Orient.RotateY( rotY );
			pTrans->Orient.Translate( trans );

			//---	apply the changes to the rotation and translation which will occur during the playback of this motion.
			trans.X += tEnd.X;
			trans.Z += tEnd.Z;
			rotY += rEnd.Y;

			if( Selected[j] == m_CurrentPlayIndex )
				SelectedCurrentlyPlayed = TRUE;
		}

		//---	make sure that the current play index is one of the selected motions
		if( count && !SelectedCurrentlyPlayed )
		{
			for( j=0; j<count; j++ )
				if( Selected[j] > m_CurrentPlayIndex )
				{
					m_CurrentPlayIndex = Selected[j];
					break;
				}

			if( j==count )
				m_CurrentPlayIndex = Selected[0];
		}
	}
	else
	{
	
		POSITION pos = m_AnimTransList.GetHeadPosition();
		while( pos )
		{
			pTrans		= m_AnimTransList.GetNext( pos );
			TotalFrames	+= pTrans->EndFrame - pTrans->StartFrame + 1;

			//---	subtract the start translation
			pTrans->pMotion->GetTranslation( tStart, pTrans->StartFrame );
			pTrans->pMotion->GetRotation( rStart, pTrans->StartFrame );
			pTrans->pMotion->GetTranslation( tEnd, pTrans->EndFrame );
			pTrans->pMotion->GetRotation( rEnd, pTrans->EndFrame );

			//---	remove any initial rotation from the motion's direction
			rotY -= rStart.Y;

			//---	rotate the start and end translations by the rotation which will be applied to this
			//		motion.
			tStart.RotateY( rotY );
			tEnd.RotateY( rotY );

			//---	remove the start offset from the overall translation
			trans.X -= tStart.X;
			trans.Z -= tStart.Z;

			//---	build this motion's orientation matrix using the current rotation and translation
			pTrans->Orient.Identity();
			pTrans->Orient.RotateY( rotY );
			pTrans->Orient.Translate( trans );

			//---	apply the changes to the rotation and translation which will occur during the playback of this motion.
			trans.X += tEnd.X;
			trans.Z += tEnd.Z;
			rotY += rEnd.Y;
		}
	}

	m_TotalFrames = TotalFrames;

	//---	set the scroll range
	m_pView->GetFrame()->m_wndAnimBar.SetRange( TotalFrames );

	m_bSettingSelected = FALSE;
}

//=============================================================================
void CTransitionViewer::OrderUp( s32 i )
{
	s32 SelectedItem = i;

	POSITION pos = m_AnimTransList.FindIndex( SelectedItem );
	POSITION pos2 = m_AnimTransList.FindIndex( SelectedItem - 1 );
	ASSERT(pos && pos2);
	if( !pos || !pos2 )
		return;

	CAnimTransData* pTransItem = m_AnimTransList.GetAt( pos );
	m_AnimTransList.RemoveAt( pos );
	m_AnimTransList.InsertBefore( pos2, pTransItem );
}

//=============================================================================
void CTransitionViewer::OrderDown( s32 i )
{
	s32 SelectedItem = i;

	POSITION pos = m_AnimTransList.FindIndex( SelectedItem );
	POSITION pos2 = m_AnimTransList.FindIndex( SelectedItem + 1 );
	ASSERT(pos && pos2);
	if( !pos || !pos2 )
		return;

	CAnimTransData* pTransItem = m_AnimTransList.GetAt( pos );
	m_AnimTransList.RemoveAt( pos );
	m_AnimTransList.InsertAfter( pos2, pTransItem );
}

//==========================================================================
void CTransitionViewer::SetBlendFrames( s32 i, s32 v )
{
	if( m_bSettingSelected )
		return;

	if( i==-1)
		return;

	POSITION pos = m_AnimTransList.FindIndex( i );
	if( !pos )
		return;

	CAnimTransData* pTrans = m_AnimTransList.GetAt( pos );
	pTrans->BlendFrames = v;
}

//==========================================================================
void CTransitionViewer::SetStartFrame( s32 i, s32 v )
{
	if( m_bSettingSelected )
		return;

	if( i==-1)
		return;

	POSITION pos = m_AnimTransList.FindIndex( i );
	if( !pos )
		return;

	CAnimTransData* pTrans = m_AnimTransList.GetAt( pos );

	if( v < pTrans->EndFrame )
		pTrans->StartFrame = v;
}

//==========================================================================
void CTransitionViewer::SetEndFrame( s32 i, s32 v )
{
	if( m_bSettingSelected )
		return;

	if( i==-1)
		return;

	POSITION pos = m_AnimTransList.FindIndex( i );
	if( !pos )
		return;

	CAnimTransData* pTrans = m_AnimTransList.GetAt( pos );

	if( v > pTrans->StartFrame )
		pTrans->EndFrame = v;
}

//=============================================================================
void CTransitionViewer::OnOrderup() 
{
	s32 SelectedItems[MAX_NUM_TRANSITIONS];
	s32 CountSelectedItems=0;
	s32 top = 0;
	s32 i;
	POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();

	while( pos )
	{
		i = m_TransitionList.GetNextSelectedItem( pos );
		if( (i-1) >= top )
		{
			OrderUp( i );
			SelectedItems[CountSelectedItems++] = i-1;
		}
		else
			SelectedItems[CountSelectedItems++] = i;
		top++; // for each selected item, the highest position which it can be moved to increases.
	}

	UpdateTransList();
	SetSelected( SelectedItems, CountSelectedItems );
}

//=============================================================================
void CTransitionViewer::OnOrderdown() 
{
	s32 SelectedItems[MAX_NUM_TRANSITIONS];
	s32 CountSelectedItems=0;
	s32 bottom = m_AnimTransList.GetCount() - 1;
	s32 i;
	POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();

	while( pos )
	{
		i = m_TransitionList.GetNextSelectedItem( pos );
		if( (i+1) <= bottom )
		{
			OrderDown( i );
			SelectedItems[CountSelectedItems++] = i+1;
		}
		else
			SelectedItems[CountSelectedItems++] = i;
		bottom--; // for each selected item, the lowest position which it can be moved to decreases.
	}

	UpdateTransList();
	SetSelected( SelectedItems, CountSelectedItems );
}

//==========================================================================
void CTransitionViewer::OnChangeBlendframes() 
{
	if( m_bSettingSelected )
		return;

	POSITION pos;
	HWND hWnd = GetSafeHwnd();
	if( hWnd==NULL )
		return;

	if( m_TransitionList.GetSafeHwnd() == NULL )
		return;

	pos = m_TransitionList.GetFirstSelectedItemPosition();
	if( !pos )
		return;

	CString str;
	m_BlendFrames.GetWindowText( str );
	s32 NewValue = atoi( str );
	s32 i;

	while( pos )
	{
		i = m_TransitionList.GetNextSelectedItem( pos );
		SetBlendFrames( i, NewValue );
	}

	UpdateTransList( TRUE );
}

//==========================================================================
void CTransitionViewer::OnChangeEndframe() 
{
	if( m_bSettingSelected )
		return;

	POSITION pos;
	HWND hWnd = GetSafeHwnd();
	if( hWnd==NULL )
		return;

	if( m_TransitionList.GetSafeHwnd() == NULL )
		return;

	pos = m_TransitionList.GetFirstSelectedItemPosition();
	if( !pos )
		return;

	CString str;
	m_EndFrame.GetWindowText( str );
	s32 NewValue = atoi( str );
	s32 i;

	while( pos )
	{
		i = m_TransitionList.GetNextSelectedItem( pos );
		SetEndFrame( i, NewValue );
	}

	UpdateTransList( TRUE );
}

//==========================================================================
void CTransitionViewer::OnChangeStartframe() 
{
	if( m_bSettingSelected )
		return;

	POSITION pos;
	HWND hWnd = GetSafeHwnd();
	if( hWnd==NULL )
		return;

	if( m_TransitionList.GetSafeHwnd() == NULL )
		return;

	pos = m_TransitionList.GetFirstSelectedItemPosition();
	if( !pos )
		return;

	CString str;
	m_StartFrame.GetWindowText( str );
	s32 NewValue = atoi( str );
	s32 i;

	while( pos )
	{
		i = m_TransitionList.GetNextSelectedItem( pos );
		SetStartFrame( i, NewValue );
	}

	UpdateTransList( TRUE );
}

//=============================================================================
s32 CTransitionViewer::AddTransition( CMotion* pMotion )
{
	//---	limit the total count of transitions
	if( m_AnimTransList.GetCount() >= MAX_NUM_TRANSITIONS )
		return -1;

	CAnimTransData* pNewAnimTrans = new CAnimTransData;
	if( !pNewAnimTrans )
		return -1;

	pNewAnimTrans->pMotion		= pMotion;
	pNewAnimTrans->BlendFrames	= 0;
	pNewAnimTrans->StartFrame	= 0;
	pNewAnimTrans->EndFrame		= pMotion->GetNumFrames()-1;

	if( pNewAnimTrans->BlendFrames > pMotion->GetNumFrames() / 4 )
		pNewAnimTrans->BlendFrames = pMotion->GetNumFrames() / 4;

	ASSERT(m_TransitionList.GetItemCount() == m_AnimTransList.GetCount());

	s32 CurSelect = m_TransitionList.GetSelectionMark();
	if( CurSelect != -1 )
	{
		POSITION pos = m_AnimTransList.FindIndex( CurSelect );
		ASSERT( pos );
		if( pos )
			m_AnimTransList.InsertBefore( pos, pNewAnimTrans );
	}
	else
		m_AnimTransList.AddTail( pNewAnimTrans );

	UpdateTransList();
	
	s32 ScrollBase;
	s32 ScrollRange;
	GetScrollProperties( ScrollBase, ScrollRange );
	m_pView->GetFrame()->m_wndAnimBar.SetRange( ScrollRange );

	return (CurSelect == -1) ? -1 : CurSelect;
}

//=============================================================================
s32 CTransitionViewer::FindTransition( CMotion* pMotion )
{
	POSITION pos = m_AnimTransList.GetHeadPosition();
	s32 i = -1;

	while( pos )
	{
		i++;
		if( m_AnimTransList.GetNext(pos)->pMotion == pMotion )
			break;
	}

	return i;
}

//=============================================================================
void CTransitionViewer::RemoveTransition( s32 Index )
{
	if( Index == -1 )
		return;

	POSITION pos = m_AnimTransList.FindIndex( Index );
	ASSERT(pos);
	if( !pos )
		return;

	if( m_CurrentPlayIndex == Index )
		OnReset();

	CAnimTransData* pAnimTrans = m_AnimTransList.GetAt( pos );
	m_AnimTransList.RemoveAt( pos );
	delete pAnimTrans;

	UpdateTransList();
}

//=============================================================================
void CTransitionViewer::OnDblclkTransitionlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE)pNMHDR;

	s32 Selected = pItem->iItem; //m_TransitionList.GetSelectionMark();
	if( Selected == -1 )
		return;

	RemoveTransition( Selected );
	
	*pResult = 0;
}

//=============================================================================
void CTransitionViewer::OnClickTransitionlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMITEMACTIVATE *pClick = (NMITEMACTIVATE*)pNMHDR;

	s32 SelectedItems[MAX_NUM_TRANSITIONS];
	s32 SelectedCount=0;
	POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();
	while( pos )
		SelectedItems[SelectedCount++] = m_TransitionList.GetNextSelectedItem( pos );
	SetSelected( SelectedItems, SelectedCount );
	
	if( SelectedCount )
	{
		pos = m_AnimTransList.FindIndex( SelectedItems[0] );
		CAnimTransData* pTrans = m_AnimTransList.GetAt( pos );
		ASSERT( m_pView );
		ASSERT( m_pView->m_pCharacter );
		m_pView->m_pCharacter->SetCurMotion( pTrans->pMotion );
		m_pView->m_pCharacter->SetCurFrame( (f32)pTrans->StartFrame );
		m_pView->Invalidate();
	}

	UpdateTransList( TRUE );

	*pResult = 0;
}

//=============================================================================
void CTransitionViewer::UpdateTransList( xbool bResetSelected )
{
	s32 SelectedItems[MAX_NUM_TRANSITIONS];
	s32 SelectedCount=0;
	s32 TopIndex;
	s32 PageCount;
	
	if( bResetSelected )
	{
		POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();
		while( pos )
			SelectedItems[SelectedCount++] = m_TransitionList.GetNextSelectedItem( pos );

		TopIndex = m_TransitionList.GetTopIndex();
		PageCount = m_TransitionList.GetCountPerPage();
	}

	m_TransitionList.DeleteAllItems();

	CAnimTransData* pAnimTrans;
	POSITION pos = m_AnimTransList.GetHeadPosition();
	s32 i=0;
	CString s;

	while( pos )
	{
		pAnimTrans = m_AnimTransList.GetNext( pos );

		m_TransitionList.InsertItem( i, pAnimTrans->pMotion->GetExportName(), (i==m_CurrentPlayIndex) ? 1 : 0 );

		s.Format( "%d", pAnimTrans->BlendFrames );
		m_TransitionList.SetItemText( i, 1, s );

		s.Format( "%d", pAnimTrans->StartFrame );
		m_TransitionList.SetItemText( i, 2, s );

		s.Format( "%d", pAnimTrans->EndFrame );
		m_TransitionList.SetItemText( i, 3, s );

		m_TransitionList.SetItemData( i, (s32)pAnimTrans );

		i++;
	}

	if( bResetSelected )
	{
		SetSelected( SelectedItems, SelectedCount );
		for( i=0; i<PageCount; i++ )
			m_TransitionList.EnsureVisible( TopIndex+PageCount-i-1, FALSE );
	}
}

//==========================================================================
void CTransitionViewer::OnBegindragTransitionlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

//==========================================================================
void CTransitionViewer::OnReset( void )
{
	m_CurrentPlayIndex = -1;
}

//==========================================================================
void CTransitionViewer::OnPlaypause( void )
{
	m_pView->Play();
}

//==========================================================================
void CTransitionViewer::OnEndframetoend( void )
{
	POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();
	s32 i;
	CAnimTransData* pTrans;
	while( pos )
	{
		i = m_TransitionList.GetNextSelectedItem( pos );
		pTrans = m_AnimTransList.GetAt( m_AnimTransList.FindIndex( i ) );
		SetEndFrame( i, pTrans->pMotion->GetNumFrames()-1 );
	}
}

//==========================================================================
void CTransitionViewer::GetCurrentPlayMotion( matrix4& rBaseMat, CMotion*& rNewMotion, f32& rBlendFrames, f32& rStartFrame, f32& rEndFrame, CMotion*& rLastMotion, f32& rLastFrame )
{
	POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();
	s32 CurIndex=-1;
	s32	LastIndex=-1;

	//---	if the list is empty, get out now
	if( m_AnimTransList.GetCount() == 0 )
	{
		rNewMotion		= NULL;
		rBlendFrames	= 0;
		rStartFrame		= 0;
		rEndFrame		= 0;
		rLastMotion		= NULL;
		rLastFrame		= 0;
		return;
	}

	//---	if none are selected, increment one at a time through the list
	//		otherwise, increment through the selected list
	if( pos != NULL )
	{
		while( pos )
		{
			LastIndex = CurIndex;
			CurIndex = m_TransitionList.GetNextSelectedItem( pos );
			if( CurIndex >= m_CurrentPlayIndex )
				break;
		}

		//---	if we are cycling, get the last motion details from the last animation selected
		if( m_bCycle && (LastIndex == -1) )
		{
			POSITION pos2 = m_TransitionList.GetFirstSelectedItemPosition();
			while( pos2 )
			{
				m_TransitionList.GetNextSelectedItem( pos2 );
				LastIndex++;
			}
		}
	}
	else
	{
		CurIndex = m_CurrentPlayIndex;
		LastIndex = CurIndex-1;
		if(( CurIndex<0 ) || ( CurIndex >= m_AnimTransList.GetCount() ))
		{
			OnReset();
			GetNextPlayMotion( rBaseMat, rNewMotion, rBlendFrames, rStartFrame, rEndFrame, rLastMotion, rLastFrame );
			return;
		}

		//---	if we are cycling, get the last motion details from the last animation selected
		if( m_bCycle && (LastIndex == -1) )
			LastIndex = m_AnimTransList.GetCount()-1;
	}

	//---	fill the output values
	pos = m_AnimTransList.FindIndex( CurIndex );
	if(!pos)
	{
		rNewMotion		= NULL;
		rBlendFrames	= 0;
		rStartFrame		= 0;
		rEndFrame		= 0;
		rLastMotion		= NULL;
		rLastFrame		= 0;
		return;
	}
	ASSERT(pos);
	CAnimTransData* pTrans = m_AnimTransList.GetAt( pos );
	rNewMotion		= pTrans->pMotion;
	rBlendFrames	= (f32)pTrans->BlendFrames;
	rStartFrame		= (f32)pTrans->StartFrame;
	rEndFrame		= (f32)pTrans->EndFrame;
	rBaseMat		= pTrans->Orient;

	//---	get the last animation info
	if( LastIndex != -1 )
	{
		pos = m_AnimTransList.FindIndex( LastIndex );
		ASSERT(pos);
		CAnimTransData* pLastTrans = m_AnimTransList.GetAt( pos );
		rLastMotion		= pLastTrans->pMotion;
		rLastFrame		= (f32)pLastTrans->EndFrame;
	}
	else
	{
		rLastMotion		= NULL;
		rLastFrame		= 0;
	}
}


//==========================================================================
void CTransitionViewer::GetNextPlayMotion( matrix4& rBaseMat, CMotion*& rNewMotion, f32& rBlendFrames, f32& rStartFrame, f32& rEndFrame, CMotion*& rLastMotion, f32& rLastFrame )
{
	POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();
	s32 NextIndex=-1;
	s32 LastIndex=-1;

	//---	if none are selected, increment one at a time through the list
	//		otherwise, increment through the selected list
	if( pos != NULL )
	{
		while( pos )
		{
			LastIndex = NextIndex;
			NextIndex = m_TransitionList.GetNextSelectedItem( pos );
			if( NextIndex > m_CurrentPlayIndex )
				break;
		}

		//---	if we are cycling, get the last motion details from the last animation selected
		if( m_bCycle && (LastIndex == -1) )
		{
			POSITION pos2 = m_TransitionList.GetFirstSelectedItemPosition();
			while( pos2 )
			{
				m_TransitionList.GetNextSelectedItem( pos2 );
				LastIndex++;
			}
		}
	}
	else
	{
		NextIndex = m_CurrentPlayIndex + 1;
		if( NextIndex >= m_AnimTransList.GetCount() )
			NextIndex = 0;
		LastIndex = NextIndex-1;

			//---	if we are cycling, get the last motion details from the last animation selected
		if( m_bCycle && (LastIndex == -1) )
			LastIndex = m_AnimTransList.GetCount() - 1;
	}

	//---	if the next index is less than the current index, roll it back to the beginning and restart the process.
	if( NextIndex <= m_CurrentPlayIndex )
	{
		OnReset();
		GetNextPlayMotion( rBaseMat, rNewMotion, rBlendFrames, rStartFrame, rEndFrame, rLastMotion, rLastFrame );
		return;
	}

	//---	fill the output values
	m_CurrentPlayIndex = NextIndex;

	//---	fill the output values
	pos = m_AnimTransList.FindIndex( m_CurrentPlayIndex );
	ASSERT(pos);
	CAnimTransData* pTrans = m_AnimTransList.GetAt( pos );
	rNewMotion		= pTrans->pMotion;
	rBlendFrames	= (f32)pTrans->BlendFrames;
	rStartFrame		= (f32)pTrans->StartFrame;
	rEndFrame		= (f32)pTrans->EndFrame;
	rBaseMat		= pTrans->Orient;

	//---	get the last animation info
	if( LastIndex != -1 )
	{
		pos = m_AnimTransList.FindIndex( LastIndex );
		ASSERT(pos);
		CAnimTransData* pLastTrans = m_AnimTransList.GetAt( pos );
		rLastMotion		= pLastTrans->pMotion;
		rLastFrame		= (f32)pLastTrans->EndFrame;
	}
	else
	{
		rLastMotion		= NULL;
		rLastFrame		= 0;
	}

	UpdateTransList( TRUE );
}

//==========================================================================
void CTransitionViewer::GetScrollPlayMotion( s32 Frame, s32& rFrame, matrix4& rBaseMat, CMotion*& rNewMotion, f32& rBlendFrames, f32& rStartFrame, f32& rEndFrame, CMotion*& rLastMotion, f32& rLastFrame )
{
	s32 TotalFrames = 0;
	s32 index, Range;
	CAnimTransData* pLastTrans = NULL;
	CAnimTransData* pTrans = NULL;

	POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();
	if( pos )
	{
		while( pos )
		{
			index = m_TransitionList.GetNextSelectedItem( pos );
			pTrans = m_AnimTransList.GetAt( m_AnimTransList.FindIndex( index ) );

			Range = pTrans->EndFrame - pTrans->StartFrame + 1;
			if( Frame < Range )
				break;

			Frame -= Range;
			pLastTrans = pTrans;
		}

		//---	if we are cycling, get the last motion details from the last animation selected
		if( m_bCycle && (pLastTrans == NULL) )
		{
			POSITION pos2 = m_TransitionList.GetFirstSelectedItemPosition();
			while( pos2 )
			{
				index = m_TransitionList.GetNextSelectedItem( pos2 );
				pLastTrans = m_AnimTransList.GetAt( m_AnimTransList.FindIndex( index ) );
			}
		}
	}
	else
	{
		index = 0;
		pos = m_AnimTransList.GetHeadPosition();
		while( pos )
		{
			pTrans		= m_AnimTransList.GetNext( pos );

			Range = pTrans->EndFrame - pTrans->StartFrame + 1;
			if( Frame < Range )
				break;

			Frame -= Range;
			pLastTrans = pTrans;
			index++;
		}


		//---	if we are cycling, get the last motion details from the last animation selected
		if( m_bCycle && (pLastTrans == NULL) )
			pLastTrans = m_AnimTransList.GetAt( m_AnimTransList.FindIndex( m_AnimTransList.GetCount() - 1 ) );
	}

	//---	if a transition was found, set the values
	if( pTrans )
	{
		Frame += pTrans->StartFrame;

		rNewMotion		= pTrans->pMotion;
		rBlendFrames	= (f32)pTrans->BlendFrames;
		rStartFrame		= (f32)pTrans->StartFrame;
		rEndFrame		= (f32)pTrans->EndFrame;
		rBaseMat		= pTrans->Orient;

		//---	get the last animation info
		if( pLastTrans )
		{
			rLastMotion	= pLastTrans->pMotion;
			rLastFrame	= (f32)pLastTrans->EndFrame;
		}
		else
		{
			rLastMotion	= NULL;
			rLastFrame	= 0;
		}

		//---	udpate the current play index to match the scrolled position
		m_CurrentPlayIndex = index;
		UpdateTransList( TRUE );
	}
	else
	{
		rNewMotion		= NULL;
		rBlendFrames	= 0;
		rStartFrame		= 0;
		rEndFrame		= 0;
		rLastMotion		= NULL;
		rLastFrame		= 0;
		rBaseMat.Identity();
	}

	rFrame = Frame;
}

//==========================================================================
void CTransitionViewer::GetScrollProperties( s32& ScrollBase, s32& ScrollRange )
{
	s32 index, Range, TotalFrames = 0;
	CAnimTransData* pLastTrans = NULL;
	CAnimTransData* pTrans = NULL;

	//---	count up to the scroll base
	ScrollBase = 0;
	POSITION pos = m_TransitionList.GetFirstSelectedItemPosition();
	if( pos )
	{
		while( pos )
		{
			index = m_TransitionList.GetNextSelectedItem( pos );

			pTrans		= m_AnimTransList.GetAt( m_AnimTransList.FindIndex( index ) );
			Range		= pTrans->EndFrame - pTrans->StartFrame + 1;
			TotalFrames += Range;
			if( index < m_CurrentPlayIndex )
				ScrollBase	+= Range;
		}
	}
	else
	{
		index = 0;
		pos = m_AnimTransList.GetHeadPosition();
		while( pos )
		{
			pTrans		= m_AnimTransList.GetNext( pos );
			Range		= pTrans->EndFrame - pTrans->StartFrame + 1;
			TotalFrames += Range;
			if( index < m_CurrentPlayIndex )
				ScrollBase	+= Range;
			index++;
		}
	}

	//---	set the scroll range to the previously calculated value
	ScrollRange = TotalFrames;
}

void CTransitionViewer::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

//	if( m_pView && GetSafeHwnd() )
//		UpdateTransList( TRUE );
}

void CTransitionViewer::OnCycle() 
{
	m_bCycle = !m_bCycle;	
}

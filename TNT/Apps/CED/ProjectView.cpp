// ProjectView.cpp : implementation file
//

#include "stdafx.h"
#include "CeD.h"
#include "CeDDoc.h"
#include "ProjectView.h"
#include "Mesh.h"
#include "actor.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectView

IMPLEMENT_DYNCREATE(CProjectView, CTreeView)

CProjectView::CProjectView()
{
}

CProjectView::~CProjectView()
{
}


BEGIN_MESSAGE_MAP(CProjectView, CTreeView)
	//{{AFX_MSG_MAP(CProjectView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_CHARACTER_NEW, OnCharacterNew)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_COMMAND(ID_MOVIE_NEW, OnMovieNew)
	ON_COMMAND(ID_CHARACTER_DELETE, OnCharacterDelete)
	ON_COMMAND(ID_MOVIE_DELETE, OnMovieDelete)
	ON_COMMAND(ID_MESH_NEW, OnMeshNew)
	ON_COMMAND(ID_MESH_DELETE, OnMeshDelete)
	ON_COMMAND(ID_CHARACTER_LOADMOTIONS, OnCharacterLoadmotions)
	ON_COMMAND(ID_CHARACTER_EXPORT, OnCharacterExport)
	ON_COMMAND(ID_CHARACTER_IMPORT_SKEL, OnCharacterImportSkel)
	ON_COMMAND(ID_CHARACTER_IMPORT_MOTION, OnCharacterImportMotion)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_DELETE, OnUpdateMenuCharacterOption)
	ON_COMMAND(ID_MOVIE_EXPORT, OnMovieExport)
	ON_COMMAND(ID_MOVIE_EXPORTPROPERTIES, OnMovieExportproperties)
	ON_COMMAND(ID_CHARACTER_IMPORT_SKIN, OnCharacterImportSkin)
	ON_COMMAND(ID_CHARACTER_FACEDIRRECIPE, OnCharacterFacedirrecipe)
	ON_COMMAND(ID_CHARACTER_SKINS, OnCharacterSkins)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_SKINS, OnUpdateCharacterSkins)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_FACEDIRRECIPE, OnUpdateCharacterFacedirrecipe)
	ON_COMMAND(ID_CHARACTER_EXPORTALL, OnCharacterExportall)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_EXPORTALL, OnUpdateCharacterExportall)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_EXPORT, OnUpdateMenuCharacterOption)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_IMPORT_MOTION, OnUpdateMenuCharacterOption)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_IMPORT_SKEL, OnUpdateMenuCharacterOption)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_LOADMOTIONS, OnUpdateMenuCharacterOption)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_IMPORT_SKIN, OnUpdateMenuCharacterOption)
	ON_UPDATE_COMMAND_UI(ID_CHARACTER_PROPERTIES, OnUpdateCharacterProperties)
	ON_COMMAND(ID_CHARACTER_PROPERTIES, OnCharacterProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectView drawing

void CProjectView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CProjectView diagnostics

#ifdef _DEBUG
void CProjectView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CProjectView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CCeDDoc* CProjectView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCeDDoc)));
	return (CCeDDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CProjectView message handlers


void CProjectView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class

	CTreeCtrl &Tree = GetTreeCtrl() ;

	//---	Load and Set Image List
	BOOL Success = m_ImageList.Create( IDB_PROJECTTREE, 16, 256, RGB(255,255,255) ) ;
	ASSERT( Success ) ;
	Tree.SetImageList( &m_ImageList, TVSIL_NORMAL ) ;

	//---	Add Characters entry to Tree
	TreeRebuild( ) ;
}

BOOL CProjectView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS | TVS_SHOWSELALWAYS ;

	return CTreeView::PreCreateWindow(cs);
}

void CProjectView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CTreeCtrl &Tree = GetTreeCtrl() ;
	*pResult = 1;

	HTREEITEM hItem = Tree.GetSelectedItem() ;
	if( hItem )
	{
		CObject *pObject = (CObject*)Tree.GetItemData( hItem ) ;
		if( !pObject )
			return;

		//---	Double Clicked Character
		if( pObject->IsKindOf(RUNTIME_CLASS(CCharacter)) )
		{
			GetDocument()->CreateCharacterView( (CCharacter*)pObject ) ;
		}

		//---	Double Clicked Motion
		if( pObject->IsKindOf(RUNTIME_CLASS(CMotion)) )
		{
			if (((CMotion*)pObject)->Modify())
			{
				GetDocument()->UpdateAllViews( NULL );
				GetDocument()->SetModifiedFlag();
			}
		}

		//---	Double Clicked Movie
		if( pObject->IsKindOf(RUNTIME_CLASS(CMovie)) )
		{
			GetDocument()->CreateMovieView( (CMovie*)pObject ) ;
		}
	}
}

void CProjectView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	switch( lHint )
	{
	case HINT_CHARACTER_NEW:
		UpdateAddCharacter( ) ;
		break ;
	case HINT_MOTION_NEW:
		UpdateAddMotion( ) ;
		break ;
	case HINT_MOTION_DELETE:
		UpdateDeleteMotion( ) ;
		break ;
	case HINT_MOVIE_NEW:
		UpdateAddMovie( ) ;
		break ;
	case HINT_ACTOR_NEW:
		UpdateAddActor( ) ;
		break ;
	case HINT_ACTOR_DELETE:
		UpdateDeleteActor( ) ;
		break ;
	default:
		TreeRebuild( ) ;
		break ;
	}
}

void CProjectView::TreeRebuild( )
{
	CCeDDoc *pDoc = GetDocument() ;
	ASSERT( pDoc ) ;

	CTreeCtrl &Tree = GetTreeCtrl() ;

	//---	Clear Tree
	Tree.SetRedraw( FALSE ) ;
	Tree.DeleteAllItems() ;

//////////////////////////////////////////////////////////////////////////////
//	Add Characters

	//---	Add Characters Header
	HTREEITEM hCharacters = Tree.InsertItem( "Characters", 1, 1 ) ;
	ASSERT( hCharacters != NULL ) ;

	//---	Add Each Character
	POSITION Pos = pDoc->m_CharacterList.GetHeadPosition() ;
	while( Pos )
	{
		CCharacter *pCharacter = pDoc->m_CharacterList.GetNext( Pos ) ;
		HTREEITEM hItem = Tree.InsertItem( pCharacter->GetName(), 1, 1, hCharacters ) ;
		if( hItem )
		{
			Tree.SetItemData( hItem, (DWORD)pCharacter ) ;
			TreeAddMotions( Tree, hItem, pCharacter ) ;
		}
	}

	//---	Expand Characters Header
	Tree.Expand( hCharacters, TVE_EXPAND ) ;

//////////////////////////////////////////////////////////////////////////////
//	Add Meshes

	//---	Add Meshs Header
	HTREEITEM hMeshes = Tree.InsertItem( "Meshes", 1, 1 ) ;
	ASSERT( hMeshes != NULL ) ;

	//---	Add Each Mesh
	Pos = pDoc->m_MeshList.GetHeadPosition() ;
	while( Pos )
	{
		CMesh *pMesh = pDoc->m_MeshList.GetNext( Pos ) ;
		HTREEITEM hItem = Tree.InsertItem( pMesh->GetPathName(), 1, 1, hMeshes ) ;
		if( hItem )
		{
			Tree.SetItemData( hItem, (DWORD)pMesh ) ;
		}
	}

	//---	Expand Meshes Header
	Tree.Expand( hMeshes, TVE_EXPAND ) ;

//////////////////////////////////////////////////////////////////////////////
//	Add Movies

	//---	Add Movies Header
	HTREEITEM hMovies = Tree.InsertItem( "Movies", 1, 1 ) ;
	ASSERT( hMovies != NULL ) ;

	//---	Add Each Movie
	Pos = pDoc->m_MovieList.GetHeadPosition() ;
	while( Pos )
	{
		CMovie *pMovie = pDoc->m_MovieList.GetNext( Pos ) ;
		HTREEITEM hItem = Tree.InsertItem( pMovie->GetName(), 1, 1, hMovies ) ;
		if( hItem )
		{
			Tree.SetItemData( hItem, (DWORD)pMovie ) ;
			TreeAddActors( Tree, hItem, pMovie ) ;
		}
	}

	//---	Expand Movies Header
	Tree.Expand( hMovies, TVE_EXPAND ) ;

	//---	Force Redraw of Tree Control
	Tree.SetRedraw( TRUE ) ;
	Tree.RedrawWindow() ;
}

void CProjectView::TreeAddMotions( CTreeCtrl &Tree, HTREEITEM hParent, CCharacter *pCharacter )
{
	HTREEITEM hMotions = Tree.InsertItem( "Motions", 2, 2, hParent ) ;
	ASSERT( hMotions != NULL ) ;

	s32 ExportedIndex=0;

	for( int i = 0 ; i < pCharacter->GetNumMotions() ; i++ )
	{
		CMotion *pMotion = pCharacter->GetMotion( i ) ;
		CString String ;
		if( !pMotion->m_bExport )
			String.Format( "**** - [%s]", pMotion->GetPathName() ) ;
		else
			String.Format( "%04d - %s", ExportedIndex++, pMotion->GetPathName() ) ;
		HTREEITEM hItem = Tree.InsertItem( String, 2, 2, hMotions ) ;
		if( hItem )
		{
			Tree.SetItemData( hItem, (DWORD)pMotion ) ;
		}
	}

	Tree.Expand( hMotions, TVE_EXPAND ) ;
}

void CProjectView::TreeAddActors( CTreeCtrl &Tree, HTREEITEM hParent, CMovie *pMovie )
{
	//---	Add Actors Header
	HTREEITEM hActors = Tree.InsertItem( "Actors", 2, 2, hParent ) ;
	ASSERT( hActors != NULL ) ;

/*
	//---	Add Each Actor in Turn
	for( int i = 0 ; i < pMovie->GetNumActors() ; i++ )
	{
		CActor *pActor = pMovie->GetActor( i ) ;
		HTREEITEM hItem = Tree.InsertItem( pActor->GetName(), 2, 2, hActors ) ;
		if( hItem )
		{
			Tree.SetItemData( hItem, (DWORD)pActor ) ;
		}
	}
*/
	//---	Expand Actors Tree
	Tree.Expand( hActors, TVE_EXPAND ) ;
}





void CProjectView::UpdateAddCharacter( )
{
	TreeRebuild() ;
}

void CProjectView::UpdateAddMotion( )
{
	TreeRebuild() ;
}

void CProjectView::UpdateDeleteMotion( )
{
	TreeRebuild() ;
}

void CProjectView::UpdateAddMovie( )
{
	TreeRebuild() ;
}

void CProjectView::UpdateAddActor( )
{
	TreeRebuild() ;
}

void CProjectView::UpdateDeleteActor( )
{
	TreeRebuild() ;
}




void CProjectView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here

	//---	Default FAILED
	*pResult = 0;

	CTreeCtrl &Tree = GetTreeCtrl() ;
	CObject *pObject = (CObject*)Tree.GetItemData( pTVDispInfo->item.hItem ) ;

	//---	Check for Character
	if( pObject && (pObject->IsKindOf( RUNTIME_CLASS(CCharacter) )) )
	{
		//---	Change Name and set Success
		CCharacter *pCharacter = (CCharacter*)pObject ;
		pCharacter->SetName( pTVDispInfo->item.pszText ) ;
		*pResult = 1 ;
	}

	//---	Check for Movie
	if( pObject && (pObject->IsKindOf( RUNTIME_CLASS(CMovie) )) )
	{
		//---	Change Name and set Success
		CMovie *pMovie = (CMovie*)pObject ;
		pMovie->SetName( pTVDispInfo->item.pszText ) ;
		*pResult = 1 ;
	}
}

void CProjectView::OnMovieNew() 
{
	// TODO: Add your command handler code here
	
	GetDocument()->CreateNewMovie() ;
}

void CProjectView::OnCharacterNew() 
{
	// TODO: Add your command handler code here

	GetDocument()->CreateNewCharacter() ;
}

void CProjectView::OnCharacterDelete() 
{
	// TODO: Add your command handler code here
	CCeDDoc *pDoc = GetDocument() ;
	ASSERT( pDoc ) ;
	
	CTreeCtrl &Tree = GetTreeCtrl() ;
	HTREEITEM hItem = Tree.GetSelectedItem() ;
	if( hItem )
	{
		CObject *pObject = (CObject*)Tree.GetItemData( hItem ) ;

		//---	Double Clicked Character
		if( pObject && pObject->IsKindOf(RUNTIME_CLASS(CCharacter)) )
		{
			CCharacter *pCharacter = (CCharacter*)pObject ;

			//---	Check if user is sure
			int Sure = MessageBox( "Are You Sure?", "Delete Character", MB_ICONWARNING|MB_YESNO ) ;
			if( Sure == IDYES )
			{
				pDoc->m_CharacterList.Remove( pCharacter ) ;
				pDoc->CharacterDeleted( pCharacter ) ;
				delete pCharacter ;
				pDoc->UpdateAllViews( NULL ) ;
			}
		}
	}
}

void CProjectView::OnMovieDelete() 
{
	// TODO: Add your command handler code here
	CCeDDoc *pDoc = GetDocument() ;
	ASSERT( pDoc ) ;
	
	CTreeCtrl &Tree = GetTreeCtrl() ;
	HTREEITEM hItem = Tree.GetSelectedItem() ;
	if( hItem )
	{
		CObject *pObject = (CObject*)Tree.GetItemData( hItem ) ;

		//---	Movie?
		if( pObject && pObject->IsKindOf(RUNTIME_CLASS(CMovie)) )
		{
			CMovie *pMovie = (CMovie*)pObject ;

			//---	Check if user is sure
			int Sure = MessageBox( "Are You Sure?", "Delete Movie", MB_ICONWARNING|MB_YESNO ) ;
			if( Sure == IDYES )
			{
				pDoc->m_MovieList.Remove( pMovie ) ;
//				pDoc->MovieDeleted( pMovie ) ;
				delete pMovie ;
				pDoc->UpdateAllViews( NULL ) ;
			}
		}
	}
}

void CProjectView::OnMeshNew() 
{
	// TODO: Add your command handler code here
	GetDocument()->CreateNewMesh() ;
}

void CProjectView::OnMeshDelete() 
{
	// TODO: Add your command handler code here
	CCeDDoc *pDoc = GetDocument() ;
	ASSERT( pDoc ) ;
	
	CTreeCtrl &Tree = GetTreeCtrl() ;
	HTREEITEM hItem = Tree.GetSelectedItem() ;
	if( hItem )
	{
		CObject *pObject = (CObject*)Tree.GetItemData( hItem ) ;

		//---	Movie?
		if( pObject && pObject->IsKindOf(RUNTIME_CLASS(CMesh)) )
		{
			CMesh *pMesh = (CMesh*)pObject ;

			bool MeshInUse;

			MeshInUse = false;

			POSITION PosMovie = pDoc->m_MovieList.GetHeadPosition() ;
			CMovie *pMovie;
			while( PosMovie && !MeshInUse )
			{
				pMovie = pDoc->m_MovieList.GetNext( PosMovie ) ;
				ASSERT( pMovie ) ;
				POSITION PosActors = pMovie->m_ActorList.GetHeadPosition();
				while( PosActors && !MeshInUse )
				{
					CActor *pActor = pMovie->m_ActorList.GetNext( PosActors );
					if ( (pActor->GetType() == ACTOR_SCENERY) && 
						 (pActor->GetMesh() == pMesh) )
					{
						MeshInUse = true;
					}
				}

			}
			// Scan through movie list, looking for this mesh in the movie
			// scenery list

			if (MeshInUse)
			{
				CString String ;
				String.Format( "Mesh '%s' is in use by movie '%s'", pMesh->GetFileName(),pMovie->GetName() ) ;
				::MessageBox( NULL, String, "Cannot Delete Mesh", MB_ICONSTOP ) ;
			}
			else
			{
				int Sure = MessageBox( "Are You Sure?", "Delete Mesh", MB_ICONWARNING|MB_YESNO ) ;
				if( Sure == IDYES )
				{
					pDoc->m_MeshList.Remove( pMesh ) ;
					delete pMesh ;
					pDoc->UpdateAllViews( NULL ) ;
				}
			}
		}
	}
}

void CProjectView::OnCharacterLoadmotions() 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( !hItem )
		return;

	CObject *pObject = (CObject*)Tree.GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		GetDocument()->CharacterLoadAllMotions( (CCharacter*)pObject );
}

void CProjectView::OnCharacterExport() 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( !hItem )
		return;

	CObject *pObject = (CObject*)Tree.GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		GetDocument()->CharacterExportFull( (CCharacter*)pObject );
}

void CProjectView::OnCharacterImportSkel() 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( !hItem )
		return;

	CObject *pObject = (CObject*)Tree.GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		GetDocument()->CharacterImportSkeleton( (CCharacter*)pObject );
}

void CProjectView::OnCharacterImportSkin() 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( !hItem )
		return;

	CObject *pObject = (CObject*)Tree.GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		GetDocument()->CharacterImportSkin( (CCharacter*)pObject );
}

void CProjectView::OnCharacterImportMotion() 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( !hItem )
		return;

	CObject *pObject = (CObject*)Tree.GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		GetDocument()->CharacterImportMotion( (CCharacter*)pObject );
}

void CProjectView::OnUpdateMenuCharacterOption(CCmdUI* pCmdUI) 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( hItem )
	{
		CObject *pObject = (CObject*)Tree.GetItemData( hItem );
		if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		{
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	pCmdUI->Enable( FALSE );
}

void CProjectView::OnMovieExport() 
{
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();

	CObject *pObject = (CObject*)GetTreeCtrl().GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CMovie) ) )
		GetDocument()->MovieExport( 0, (CMovie*)pObject );
}

void CProjectView::OnMovieExportproperties() 
{
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();

	CObject *pObject = (CObject*)GetTreeCtrl().GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CMovie) ) )
		GetDocument()->MovieExportProperties( (CMovie*)pObject );
}

void CProjectView::OnCharacterFacedirrecipe() 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( !hItem )
		return;

	CObject *pObject = (CObject*)Tree.GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		GetDocument()->CharacterEditFaceDirRecipe( (CCharacter*)pObject );
}

void CProjectView::OnUpdateCharacterFacedirrecipe(CCmdUI* pCmdUI) 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( hItem )
	{
		CObject *pObject = (CObject*)Tree.GetItemData( hItem );
		if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		{
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	pCmdUI->Enable( FALSE );
}

void CProjectView::OnCharacterSkins() 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( !hItem )
		return;

	CObject *pObject = (CObject*)Tree.GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		GetDocument()->CharacterEditSkins( (CCharacter*)pObject );
}

void CProjectView::OnUpdateCharacterSkins(CCmdUI* pCmdUI) 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( hItem )
	{
		CObject *pObject = (CObject*)Tree.GetItemData( hItem );
		if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		{
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	pCmdUI->Enable( FALSE );
}

void CProjectView::OnCharacterExportall() 
{
	GetDocument()->CharacterExportAll();
}

void CProjectView::OnUpdateCharacterExportall(CCmdUI* pCmdUI) 
{
}

void CProjectView::OnUpdateCharacterProperties(CCmdUI* pCmdUI) 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( hItem )
	{
		CObject *pObject = (CObject*)Tree.GetItemData( hItem );
		if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		{
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	pCmdUI->Enable( FALSE );
}

void CProjectView::OnCharacterProperties() 
{
	CTreeCtrl &Tree = GetTreeCtrl();
	HTREEITEM hItem = Tree.GetSelectedItem();
	if( !hItem )
		return;

	CObject *pObject = (CObject*)Tree.GetItemData( hItem );
	if( pObject && pObject->IsKindOf( RUNTIME_CLASS(CCharacter) ) )
		GetDocument()->CharacterProperties( (CCharacter*)pObject );
}

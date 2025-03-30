// CeDDoc.cpp : implementation of the CCeDDoc class
//

#include "stdafx.h"
#include "CeD.h"

#include "CeDDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "CeDView.h"
#include "ChildFrm.h"
#include "MotionEditView.h"
#include "ProjectView.h"
#include "ProjectFrame.h"
#include "MovieView.h"
#include "MovieFrame.h"
#include "ieArchive.h"
#include "CharacterList.h"
#include "ExportFile.h"
#include "CharacterExport.h"
#include "mfcutils.h"
#include "MovieExport.h"
#include "Actor.h"
#include "Mesh.h"
#include "DefinedEventsDlg.h"
#include "DlgFilePaths.h"
#include "ExportParams.h"

extern CCeDApp theApp ;

/////////////////////////////////////////////////////////////////////////////
// CCeDDoc

IMPLEMENT_DYNCREATE(CCeDDoc, CDocument)

BEGIN_MESSAGE_MAP(CCeDDoc, CDocument)
	//{{AFX_MSG_MAP(CCeDDoc)
	ON_COMMAND(ID_VIEW_GROUND_PLANE, OnViewGroundPlane)
	ON_COMMAND(ID_VIEW_CONSTRUCTION_PLANE, OnViewConstructionPlane)
	ON_COMMAND(ID_VIEW_PROJECT, OnViewProject)
	ON_COMMAND(ID_VIEW_MOTIONEDIT, OnViewMotionedit)
	ON_COMMAND(ID_MOVIE_IMPORT, OnMovieImport)
	ON_COMMAND(ID_EVENTS_DEFINEDEVENTS, OnEventsDefinedevents)
	ON_COMMAND(ID_EDIT_FILEPATHS, OnEditFilepaths)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCeDDoc construction/destruction

CCeDDoc::CCeDDoc()
{
	// TODO: add one-time construction code here
	m_DrawGroundPlane = true ;
	m_DrawConstructionPlane = false ;
	V3_Zero( &m_ConPlanePos ) ;
}

CCeDDoc::~CCeDDoc()
{
}


void CCeDDoc::ActivateDocument( )
{
}

void CCeDDoc::UpdateMenu( )
{
	CCeDApp *pApp = (CCeDApp*)AfxGetApp() ;
	CMenu *pMenu = pApp->m_pMainWnd->GetMenu() ;
	UpdateMenuItems( pMenu ) ;
}

void CCeDDoc::UpdateMenuItems( CMenu *pMenu )
{
	if( pMenu )
	{
		pMenu->CheckMenuItem (ID_VIEW_GROUND_PLANE, m_DrawGroundPlane ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem (ID_VIEW_CONSTRUCTION_PLANE, m_DrawConstructionPlane ? MF_CHECKED : MF_UNCHECKED ) ;
		pMenu->CheckMenuItem (ID_VIEW_PROJECT, GetProjectView() ? MF_CHECKED : MF_UNCHECKED ) ;

		//---	Call Active View to Update Menu Also
		CCeDView *pView = (CCeDView*)MFCU_GetActiveView() ;
		if( pView && pView->IsKindOf(RUNTIME_CLASS(CCeDView)))
		{
			pView->UpdateMenuItems( pMenu ) ;
		}
	}
}

void CCeDDoc::ClosingFrameWindow( CFrameWnd *pFrame )
{
}

/////////////////////////////////////////////////////////////////////////////
// Project Functions

CProjectView *CCeDDoc::GetProjectView()
{
	CProjectView	*pProjectView = NULL ;

	POSITION Pos = GetFirstViewPosition() ;
	while( Pos )
	{
		CView *pView = GetNextView( Pos ) ;
		if( pView->IsKindOf(RUNTIME_CLASS(CProjectView)) )
		{
			pProjectView = (CProjectView*)pView ;
			break ;
		}
	}

	return pProjectView ;
}

/////////////////////////////////////////////////////////////////////////////
// Character Functions

void CCeDDoc::CreateCharacterView( CCharacter *pCharacter )
{
	// TODO: Add your command handler code here

	CMotionEditFrame *pMotionEditFrame = new CMotionEditFrame ;
	ASSERT( pMotionEditFrame ) ;
	ASSERT_KINDOF( CFrameWnd, pMotionEditFrame ) ;
	pMotionEditFrame->m_pCharacter = pCharacter ;

	CCreateContext context ;
	context.m_pNewViewClass = RUNTIME_CLASS(CMotionEditView) ;
	context.m_pCurrentDoc = this ;
	context.m_pNewDocTemplate = NULL ;
	context.m_pLastView = NULL ;
	context.m_pCurrentFrame = NULL ;
	pMotionEditFrame->LoadFrame( IDR_MOTIONEDIT, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, &context ) ;
	pMotionEditFrame->InitialUpdateFrame( this, TRUE ) ;
}

CCharacter *CCeDDoc::CreateNewCharacter()
{
	CCharacter *pCharacter = new CCharacter ;
	ASSERT( pCharacter ) ;
	pCharacter->m_pDoc = this ;
	pCharacter->Create() ;
	m_CharacterList.AddTail( pCharacter ) ;
	UpdateAllViews( NULL, HINT_CHARACTER_NEW ) ;

	SetModifiedFlag() ;

	return pCharacter ;
}


void CCeDDoc::CharacterImportSkeleton( CCharacter* pCharacter )
{
	if( !pCharacter )
	{
		MessageBox( NULL, "No Character Selected", "Error", MB_ICONSTOP ) ;
		return;
	}

	CFileDialog fd( TRUE, NULL, NULL ) ;

	fd.m_ofn.lpstrTitle = "Import Skeleton" ;
	fd.m_ofn.lpstrFilter = "ASF Files (*.asf)\000*.asf\000" ;

	if( fd.DoModal() == IDOK )
	{
		CString PathName = fd.GetPathName() ;
		
		bool Success = CharacterImportSkeletonFromFile( pCharacter, PathName ) ;
		if( !Success )
		{
			CString String ;
			String.Format( "File '%s' is not a valid Skeleton", PathName ) ;
			MessageBox( NULL, String, "Import Failed", MB_ICONSTOP ) ;
			return;
		}
/*
		UpdateCameraBoneList() ;
		UpdateCameraControls() ;
		UpdateMotionList() ;
		UpdateMotionControls() ;
		UpdateEventList() ;
		UpdateEventControls() ;
		UpdateSkelPointList() ;
		UpdateSkelPointControls() ;
		Invalidate() ;
*/
		//---	Update All Views
		UpdateAllViews( NULL );//, HINT_MOTION_NEW, pCharacter ) ;
	}
}

void CCeDDoc::CharacterImportMotion( CCharacter* pCharacter ) 
{
	if( !pCharacter )
	{
		MessageBox( NULL, "No Character Selected", "Error", MB_ICONSTOP ) ;
		return;
	}

	if( !pCharacter->GetSkeleton() )
	{
		MessageBox( NULL, "Must have a Skeleton before importing Motions", "Error", MB_ICONSTOP ) ;
		return;
	}


	char	*pFileNameBuffer = (char*)malloc( 16*1024 ) ;
	ASSERT( pFileNameBuffer );

	pFileNameBuffer[0] = 0 ;

	CFileDialog fd( TRUE, NULL, NULL, OFN_ALLOWMULTISELECT ) ;

	fd.m_ofn.lpstrTitle = "Import Motion Files" ;
	fd.m_ofn.lpstrFilter = "AMC Files (*.amc)\000*.amc\000ANM Files (*.anm)\000*.anm\000All Files (*.*)\000*.*\000" ;

	fd.m_ofn.lpstrFile = pFileNameBuffer ;
	fd.m_ofn.nMaxFile = 16*1024 ;

	if( fd.DoModal() == IDOK )
	{
		POSITION Pos = fd.GetStartPosition() ;
		while( Pos )
		{
			CString PathName = fd.GetNextPathName( Pos ) ;

			CMotion *pMotion = CharacterImportMotionFromFile( pCharacter, PathName ) ;
			if( !pMotion )
			{
				CString String ;
				String.Format( "File '%s' is not a valid Motion", PathName ) ;
				MessageBox( NULL, String, "Import Failed", MB_ICONSTOP ) ;
			}
			else
			{
				pCharacter->SetCurMotion( pMotion ) ;
/*				
				UpdateMotionList() ;
				UpdateEventList() ;
				UpdateEventControls() ;
				UpdateSkelPointList() ;
				UpdateSkelPointControls() ;
				UpdateMotionControls() ;
				Invalidate() ;*/
			}
		}

		//---	Update All Views
//		UpdateAllViews( this, HINT_MOTION_NEW, pCharacter ) ;
		UpdateAllViews( NULL );//this, HINT_MOTION_NEW, pCharacter ) ;
	}

	free( pFileNameBuffer ) ;
}

bool CCeDDoc::CharacterImportSkeletonFromFile( CCharacter *pCharacter, const char *pPathName )
{
	SetModifiedFlag() ;
	return pCharacter->ImportSkeletonFromFile( pPathName ) ;
}

CMotion *CCeDDoc::CharacterImportMotionFromFile( CCharacter *pCharacter, const char *pPathName )
{
	SetModifiedFlag() ;
	return pCharacter->ImportMotionFromFile( pPathName ) ;
}

void CCeDDoc::CharacterLoadAllMotions( CCharacter* pCharacter )
{
	CExportProgress Progress;
	Progress.Create( IDD_EXPORT_PROGRESS, NULL );
	Progress.EnableOk( false ) ;
	Progress.Fmt( "Loading Motions..." ) ;
	Progress.Fmt( " " ) ;
	Progress.SetRange( 0, pCharacter->GetNumMotions()-1 ) ;
	s32 Count=0;
	Progress.SetWindowText( "Loading all motions" );

	s32 Result;
	CMotion* pMotion;
	POSITION pos = pCharacter->m_MotionList.GetHeadPosition();
	while( pos )
	{
		pMotion = (CMotion*) pCharacter->m_MotionList.GetNext( pos );
		Progress.Fmt( "%s", pMotion->GetPathName() ) ;
		pMotion->ReloadFromFile( TRUE, &Result );
		if( Result )
		{
			if( Result & CMotion::LOAD_RESULT_ERROR )
				Progress.Fmt( "  ERROR - AMC DID NOT LOAD" ) ;
			if( Result & CMotion::LOAD_RESULT_ONEFRAME )
				Progress.Fmt( "  WARNING - Animation has one one frame." ) ;
			if( Result & CMotion::LOAD_RESULT_FRAMESCHANGED )
				Progress.Fmt( "  WARNING - The number of frames have changed since last loaded." ) ;
			if( Result & CMotion::LOAD_RESULT_EVENTOUTSIDEANIM )
				Progress.Fmt( "  ERROR - The one or more events are outside of the animation." ) ;
		}
		Progress.SetPos( Count++ ) ;
	}

	Progress.EnableOk( true );
//	Progress.RunModalLoop();
}

void CCeDDoc::CharacterExportFull( CCharacter *pCharacter )
{
	ASSERT( pCharacter );

	// TODO: Add your command handler code here
	if( !pCharacter->GetSkeleton() )
	{
		MessageBox( NULL, "Must have a Skeleton before exporting", "Error", MB_ICONSTOP ) ;
		return;
	}

	CExportParams ep ;

	ep.m_CharacterIncludeFile	= m_CharacterIncludeFile ;
	ep.m_EnableGAMECUBE			= pCharacter->m_ExportEnableGAMECUBE ;
	ep.m_EnablePC				= pCharacter->m_ExportEnablePC ;
	ep.m_EnableXBOX				= pCharacter->m_ExportEnableXBOX ;
	ep.m_EnablePS2				= pCharacter->m_ExportEnablePS2 ;
	ep.m_FileGAMECUBE			= pCharacter->m_ExportFileGAMECUBE ;
	ep.m_FilePC					= pCharacter->m_ExportFilePC ;
	ep.m_FileXBOX				= pCharacter->m_ExportFileXBOX ;
	ep.m_FilePS2				= pCharacter->m_ExportFilePS2 ;
	ep.m_OptForceUpper			= pCharacter->m_ExportOptForceUpper ;
	ep.m_OptCollisionBubbles	= pCharacter->m_ExportOptCollisionBubbles ;
	ep.m_OptMotionFlags			= pCharacter->m_ExportOptMotionFlags ;
	ep.m_OptFrontEndSpecific	= pCharacter->m_ExportFrontEndSpecific;

	if( ep.DoModal( ) == IDOK )
	{
		m_CharacterIncludeFile					= ep.m_CharacterIncludeFile ;
		pCharacter->m_ExportEnableGAMECUBE		= !!ep.m_EnableGAMECUBE ;
		pCharacter->m_ExportEnablePC			= !!ep.m_EnablePC ;
		pCharacter->m_ExportEnableXBOX			= !!ep.m_EnableXBOX ;
		pCharacter->m_ExportEnablePS2			= !!ep.m_EnablePS2 ;
		pCharacter->m_ExportFileGAMECUBE		= ep.m_FileGAMECUBE ;
		pCharacter->m_ExportFilePC				= ep.m_FilePC ;
		pCharacter->m_ExportFileXBOX			= ep.m_FileXBOX ;
		pCharacter->m_ExportFilePS2				= ep.m_FilePS2 ;
		pCharacter->m_ExportOptForceUpper		= !!ep.m_OptForceUpper ;
		pCharacter->m_ExportOptCollisionBubbles	= !!ep.m_OptCollisionBubbles ;
		pCharacter->m_ExportOptMotionFlags		= !!ep.m_OptMotionFlags ;
		pCharacter->m_ExportFrontEndSpecific	= !!ep.m_OptFrontEndSpecific;

		if( pCharacter->m_ExportEnableGAMECUBE )
			CharacterExport( EXPORT_GAMECUBE, pCharacter->m_ExportFileGAMECUBE, pCharacter ) ;

		if( pCharacter->m_ExportEnablePC )
			CharacterExport( EXPORT_PC, pCharacter->m_ExportFilePC, pCharacter ) ;

        if( pCharacter->m_ExportEnableXBOX )
			CharacterExport( EXPORT_XBOX, pCharacter->m_ExportFileXBOX, pCharacter ) ;

        if( pCharacter->m_ExportEnablePS2 )
			CharacterExport( EXPORT_PS2, pCharacter->m_ExportFilePS2, pCharacter ) ;
	}
}

bool CCeDDoc::CharacterExport( int Target, const char *pFileName, CCharacter *pCharacter )
{
	CCharacterExport	e( Target ) ;
	
	UpdateAllViews( NULL, HINT_REDRAWALL ) ;

	//---	Export the Character
	e.Export( pFileName, pCharacter ) ;

	return true ;
}

CCharacter *CCeDDoc::GetMatchingCharacter( CCharacter *pCharacter )
{
	CCharacter *pFound = NULL ;

	//---	Attempt to Find Matching Character
	if( pCharacter )
	{
		POSITION Pos = m_CharacterList.GetHeadPosition( ) ;
		while( Pos )
		{
			CCharacter *pScan = m_CharacterList.GetNext( Pos ) ;
			ASSERT( pScan ) ;
			{
				if( pScan->m_Name.CompareNoCase( pCharacter->m_Name ) == 0 )
				{
					pFound = pScan ;
				}
			}
		}
	}

	//---	Return Character
	return pFound ;
}

/////////////////////////////////////////////////////////////////////////////
// Movie Functions

void CCeDDoc::CreateMovieView( CMovie *pMovie )
{
	// TODO: Add your command handler code here

	CMovieFrame *pMovieFrame = new CMovieFrame ;
	ASSERT( pMovieFrame ) ;
	ASSERT_KINDOF( CFrameWnd, pMovieFrame ) ;
	pMovieFrame->m_pMovie = pMovie ;

	CCreateContext context ;
	context.m_pNewViewClass = RUNTIME_CLASS(CMovieView) ;
	context.m_pCurrentDoc = this ;
	context.m_pNewDocTemplate = NULL ;
	context.m_pLastView = NULL ;
	context.m_pCurrentFrame = NULL ;
	pMovieFrame->LoadFrame( IDR_MOVIE, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, &context ) ;
	pMovieFrame->InitialUpdateFrame( this, TRUE ) ;
}

CMovie *CCeDDoc::CreateNewMovie()
{
	CMovie *pMovie = new CMovie ;
	ASSERT( pMovie ) ;

	pMovie->Create() ;
	m_MovieList.AddTail( pMovie ) ;
	UpdateAllViews( NULL, HINT_MOVIE_NEW ) ;

	SetModifiedFlag() ;

	return pMovie ;
}

bool CCeDDoc::MovieExport( int Target, const char *pFileName, CMovie *pMovie )
{
	CMovieExport	e( Target ) ;

	UpdateAllViews( NULL, HINT_REDRAWALL ) ;

	e.Export( pFileName, pMovie, this ) ;

	return true ;
}

bool CCeDDoc::MovieExportASCII( const char *pFileName, CMovie *pMovie )
{
//	CMovieExportASCII	e( Target ) ;
//	UpdateAllViews( NULL, HINT_REDRAWALL ) ;
//	e.Export( pFileName, pMovie, this ) ;

	return true ;
}




void CCeDDoc::MotionDeleted( CMotion *pMotion )
{
	POSITION Pos = m_MovieList.GetHeadPosition() ;
	while( Pos )
	{
		CMovie *pMovie = m_MovieList.GetNext( Pos ) ;
		ASSERT( pMovie ) ;
		pMovie->MotionDeleted( pMotion ) ;
	}
}

void CCeDDoc::CharacterDeleted( CCharacter *pCharacter )
{
	POSITION Pos = m_MovieList.GetHeadPosition() ;
	while( Pos )
	{
		CMovie *pMovie = m_MovieList.GetNext( Pos ) ;
		ASSERT( pMovie ) ;
		pMovie->CharacterDeleted( pCharacter ) ;
	}
}




/////////////////////////////////////////////////////////////////////////////
// CCeDDoc serialization

void CCeDDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		//---	Create Iguana Archive Object
		CMemFile	mf ;
		CieArchive	a(&mf, !!ar.IsStoring(), this) ;

		//---	Write Project Header Tags
		a.WriteTaggedString (IE_TAG_ARCHIVE_TYPE, CString("Iguana Character Editor")) ;
		a.WriteTaggedString (IE_TAG_ARCHIVE_NAME, CString("Iguana")) ;
		a.WriteTaggedu32 (IE_TAG_ARCHIVE_VERSION, 0x00010000) ;
		SYSTEMTIME	Date ;
		GetLocalTime (&Date) ;
		CString DateString ;
		int DateLen = GetDateFormat (LOCALE_USER_DEFAULT, DATE_LONGDATE, &Date, NULL, DateString.GetBuffer(256), 256) ;
		DateString.ReleaseBuffer(DateLen) ;
		a.WriteTaggedString (IE_TAG_ARCHIVE_DATE, DateString) ;

		//---	Write Project Information
		m_CharacterList.Serialize(a) ;
		m_MeshList.Serialize(a) ;
		m_MovieList.Serialize(a) ;
		m_ProjectEventList.Serialize( a );

		//---	Write Flag Bit Defines
		for( int i = 0 ; i < MOTION_FLAG_BITS ; i++ )
		{
			a.WriteTaggedString( IE_TAG_DOC_FLAGDEFINE, m_FlagDefine[i] ) ;
			a.WriteTaggedString( IE_TAG_DOC_FLAGDESCRIPTION, m_FlagDescription[i] ) ;
		}

		a.WriteTaggedString( IE_TAG_DOC_CHARACTER_INCLUDE, m_CharacterIncludeFile ) ;

		//---	Close Archive
		a.WriteTag(IE_TYPE_END,IE_TAG_END) ;

		//---	Write to Disk File
		s32 Length = mf.GetLength() ;
		u8 *Buffer = mf.Detach() ;
		ar.Write (Buffer, Length) ;
		free (Buffer) ;
	}
	else
	{
		u32	Version = 0 ;
		s32 Length = ar.GetFile()->GetLength() ;
		u8	*Buffer = new u8[Length] ;
		ASSERT (Buffer) ;
		ar.Read (Buffer, Length) ;
		CMemFile	mf(Buffer, Length) ;
		CieArchive	a(&mf, !!ar.IsStoring(), this) ;

		BOOL done = FALSE ;
		int			iFlagBit1 = 0 ;
		int			iFlagBit2 = 0 ;
		while (!done)
		{
			CString		String ;
			a.ReadTag() ;
			switch (a.m_rTag)
			{
///////////////////////////////////////////////////////////////////////////
//	SYSTEM ENTRIES
			case IE_TAG_ARCHIVE_TYPE:
				a.ReadString (String) ;
				break ;
			case IE_TAG_ARCHIVE_NAME:
				a.ReadString (String) ;
				break ;
			case IE_TAG_ARCHIVE_DATE:
				a.ReadString (String) ;
				break ;
			case IE_TAG_ARCHIVE_VERSION:
				a.Readu32 (Version) ;
				break ;

			case IE_TAG_END:
				done = TRUE ;
				break ;
///////////////////////////////////////////////////////////////////////////
//	DOCUMENT ENTRIES

			case IE_TAG_CHARACTERLIST:
				m_CharacterList.Serialize(a) ;
				break ;
			case IE_TAG_MESHLIST:
				m_MeshList.Serialize(a) ;
				break ;
			case IE_TAG_MOVIELIST:
				m_MovieList.Serialize(a) ;
				break ;
			case IE_TAG_EVENTLIST:
				m_ProjectEventList.Serialize( a );
				break;

			case IE_TAG_DOC_FLAGDEFINE:
				a.ReadString( m_FlagDefine[iFlagBit1++] ) ;
				break ;
			case IE_TAG_DOC_FLAGDESCRIPTION:
				a.ReadString( m_FlagDescription[iFlagBit2++] ) ;
				break ;

			case IE_TAG_DOC_CHARACTER_INCLUDE:
				a.ReadString( m_CharacterIncludeFile ) ;
				break ;

///////////////////////////////////////////////////////////////////////////
//	CLEANUP UNKNOWNS
			default:
				a.ReadSkip() ;
				break ;
			}
		}

		//---	Close Files & Buffers, etc.
		mf.Detach () ;
		delete []Buffer ;

///////////////////////////////////////////////////////////////////////////
//	FIXUP POINTERS
		POSITION Pos1 = m_CharacterList.GetHeadPosition() ;
		while( Pos1 )
		{
			CCharacter *pCharacter = m_CharacterList.GetNext( Pos1 ) ;
			ASSERT( pCharacter );

			//---	loop through the motions checking them for links to be re-established
			POSITION Pos2 = pCharacter->m_MotionList.GetHeadPosition() ;
			while( Pos2 )
			{
				CMotion *pMotion = pCharacter->m_MotionList.GetNext( Pos2 ) ;
				ASSERT( pMotion );

				//---	if there are no pointers to be fixed up, continue to the next one
				if( !pMotion->m_FixupLinkStuff )
					continue;

				//---	relink the character and motion
				pMotion->m_pLinkCharacter = (CCharacter*)m_CharacterList.IndexToPtr( (s32)pMotion->m_pLinkCharacter ) ;
				pMotion->m_pLinkMotion = (CMotion*)pMotion->m_pLinkCharacter->m_MotionList.IndexToPtr( (s32)pMotion->m_pLinkMotion ) ;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCeDDoc diagnostics

#ifdef _DEBUG
void CCeDDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCeDDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCeDDoc commands

void CCeDDoc::OnViewNew() 
{
	// TODO: Add your command handler code here
/*
	CFrameWnd* pFrame = new CChildFrame ;
	ASSERT( pFrame ) ;
	ASSERT_KINDOF( CFrameWnd, pFrame ) ;

	CCreateContext context ;
	context.m_pNewViewClass = RUNTIME_CLASS(CMotionEditView) ;
	context.m_pCurrentDoc = this ;
	context.m_pNewDocTemplate = NULL ;
	context.m_pLastView = NULL ;
	context.m_pCurrentFrame = NULL ;
	pFrame->LoadFrame( IDR_CEDTYPE, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,	NULL, &context ) ;
	pFrame->InitialUpdateFrame( this, TRUE ) ;
*/
}

void CCeDDoc::OnViewGroundPlane() 
{
	// TODO: Add your command handler code here
	m_DrawGroundPlane = !m_DrawGroundPlane ;
	UpdateAllViews( NULL ) ;
}

void CCeDDoc::OnViewConstructionPlane() 
{
	// TODO: Add your command handler code here
	m_DrawConstructionPlane = !m_DrawConstructionPlane ;
	UpdateAllViews( NULL ) ;
}

void CCeDDoc::OnViewProject() 
{
	// TODO: Add your command handler code here

	CProjectView *pView = GetProjectView() ;
	if( pView )
	{
		((CProjectFrame*)pView->GetParent())->MDIDestroy() ;
	}
	else
	{
		CProjectFrame *pProjectFrame = new CProjectFrame ;
		ASSERT( pProjectFrame ) ;
		ASSERT_KINDOF( CFrameWnd, pProjectFrame ) ;

		CCreateContext context ;
		context.m_pNewViewClass = RUNTIME_CLASS(CProjectView) ;
		context.m_pCurrentDoc = this ;
		context.m_pNewDocTemplate = NULL ;
		context.m_pLastView = NULL ;
		context.m_pCurrentFrame = NULL ;
		pProjectFrame->LoadFrame( IDR_CEDTYPE, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, &context ) ;
		pProjectFrame->InitialUpdateFrame( this, TRUE ) ;
	}
}

void CCeDDoc::OnChangedViewList() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CDocument::OnChangedViewList();
}

void CCeDDoc::OnViewMotionedit() 
{
	// TODO: Add your command handler code here

	CreateCharacterView( NULL ) ;
}


void CCeDDoc::OnMovieImport() 
{
	// TODO: Add your command handler code here
	//---	Preserve Current Drive and Directory
	int DirNameLen = GetCurrentDirectory( 0, NULL ) ;
	CString OldCurrentDir ;
	GetCurrentDirectory( DirNameLen, OldCurrentDir.GetBuffer( DirNameLen ) ) ;

	//---	Throw up File Requester to select ICP
	CFileDialog		OpenDialog( TRUE ) ;
	OpenDialog.m_ofn.lpstrTitle = "Select Project for Movie Import" ;
	OpenDialog.m_ofn.lpstrFilter = "CeD Files (*.icp)\000*.icp\000" ;
	if( OpenDialog.DoModal( ) == IDOK )
	{
		//---	Create Temp Document & Call Serialize Function to load it
		CCeDDoc		NewDoc ;
		CFile		ArchiveFile( OpenDialog.GetPathName( ), CFile::modeRead ) ;
		CArchive	Archive( &ArchiveFile, CArchive::load ) ;
		NewDoc.Serialize( Archive ) ;

		//---	Now need to Move Movies from NewDoc to CurrentDoc and do all appropriate character
		//		and motion translations
		POSITION Pos = NewDoc.m_MovieList.GetHeadPosition( ) ;
		while( Pos )
		{
			bool Success = true ;

			//---	Get Movie from List
			CMovie *pMovie = NewDoc.m_MovieList.GetNext( Pos ) ;
			ASSERT( pMovie ) ;
			NewDoc.m_MovieList.Remove( pMovie ) ;

			//---	Fixup movie meshes to link into current project

			POSITION PosMesh = NewDoc.m_MeshList.GetHeadPosition( );
			while ( PosMesh )
			{
				CMesh *pNewMesh = NewDoc.m_MeshList.GetNext( PosMesh );
				ASSERT(pNewMesh);

				CMesh *pOldMesh = m_MeshList.FindMatchingMesh ( pNewMesh );
				// if the current movie does not have a matching mesh, import the
				// mesh into the old movie and then patch up.
				if (!pOldMesh)
				{
					CMesh *pOldMesh = new CMesh;
					ASSERT(pOldMesh);
					bool Success = pOldMesh->CreateFromFile(pNewMesh->GetPathName());
					if (Success)
					{
						m_MeshList.AddTail(pOldMesh);
						SetModifiedFlag();
					}
					else
					{
						CString String;
						String.Format(" Could not find mesh '%s' during import",pNewMesh->GetPathName());
						::MessageBox(NULL,String,"Import Failed",MB_ICONSTOP);
					}
				}
			}

			//---	Fixup Movie to link into Current Project
			POSITION PosActor = pMovie->m_ActorList.GetHeadPosition( ) ;
			while( PosActor )
			{
				//---	Get Actor
				CActor *pActor = pMovie->m_ActorList.GetNext( PosActor ) ;
				ASSERT( pActor ) ;

				//---	Find Matching Character Pointer for actor
				CCharacter *pOldCharacter = pActor->m_pCharacter ;
				if( pOldCharacter )
				{
					CSkeleton *pOldSkeleton = pOldCharacter->GetSkeleton() ;
					CCharacter *pNewCharacter = GetMatchingCharacter( pOldCharacter ) ;

					//---	If Character not Found then create a new character
					if( (pNewCharacter == NULL) && (pOldCharacter != NULL) )
					{
						pNewCharacter = CreateNewCharacter( ) ;
						if( pNewCharacter && pOldSkeleton )
						{
							CharacterImportSkeletonFromFile( pNewCharacter, pOldSkeleton->GetPathName() ) ;
							pNewCharacter->SetName( pActor->m_pCharacter->GetName() ) ;
						}
					}
					pActor->m_pCharacter = pNewCharacter ;
				}
				else if (pActor->m_pMesh)
				{
					CString String;
					String.Format("Linking mesh '%s' to actor failed",pActor->m_pMesh->GetPathName());
					pActor->m_pMesh = m_MeshList.FindMatchingMesh( pActor->m_pMesh );
					if (!pActor->m_pMesh)
					{
						::MessageBox(NULL,String,"Mesh Link Failed",MB_ICONSTOP);
					}
				}

				//---	Fixup All Actor Keys
				POSITION PosKey = pActor->m_Keys.GetHeadPosition( ) ;
				while( PosKey )
				{
					CKey *pKey = pActor->m_Keys.GetNext( PosKey ) ;
					ASSERT( pKey ) ;
					pKey->m_pCharacter = pActor->m_pCharacter ;
					pKey->m_pActor = pActor ;
					if( pKey->m_pCharacter )
					{
						pKey->m_pMotion = pActor->m_pCharacter->GetMatchingMotion( pKey->m_pMotion ) ;
					}
				}
			}

			//---	If Successful then add Movie to Current Doc
//			if( Success )
			{
                bool AddMovie = true;

                POSITION Pos = m_MovieList.FindMovie( pMovie->GetName() );

                if (Pos != NULL)
                {
                    char buffer[256];

                    x_sprintf(buffer, "\"%s\" already exists in current ICP file.\n\nReplace movie with the new one?", pMovie->GetName() );

                    int res;
                    res = ::MessageBox(NULL, buffer, "Replace", MB_YESNO);

                    if (res == IDNO)
                    {
                        AddMovie = false;
                    }
                    else
                    {
                        CMovie *old = (CMovie*)m_MovieList.GetAt(Pos);
                        m_MovieList.RemoveAt(Pos);
                        delete old;
                    }
                }

                if (AddMovie)
                {
				    m_MovieList.AddTail( pMovie ) ;
                }
			}
		}

		//---	Stuff Changed!!!
		SetModifiedFlag( ) ;
		UpdateAllViews( NULL ) ;
	}

	//---	Restore Current Drive and Directory
	SetCurrentDirectory( OldCurrentDir ) ;
}

CMesh *CCeDDoc::CreateNewMesh()
{
	CMesh *pMesh = NULL ;

	CFileDialog fd( TRUE, NULL, NULL ) ;

	fd.m_ofn.lpstrTitle = "Import Mesh" ;
	fd.m_ofn.lpstrFilter = "ASC Files (*.asc)\000*.asc\000" ;

	if( fd.DoModal() == IDOK )
	{
		CString PathName = fd.GetPathName() ;
		
		pMesh = new CMesh ;
		ASSERT( pMesh ) ;
		bool Success = pMesh->CreateFromFile( PathName ) ;
		if( Success )
		{
			m_MeshList.AddTail( pMesh ) ;
			UpdateAllViews( NULL, HINT_MESH_NEW ) ;
			SetModifiedFlag() ;
		}
		else
		{
			CString String ;
			String.Format( "File '%s' is not a valid Mesh", PathName ) ;
			::MessageBox( NULL, String, "Create Failed", MB_ICONSTOP ) ;
		}
	}

	return pMesh ;
}


CEventList* CCeDDoc::GetProjectEventList()
{
	return &m_ProjectEventList;
}

void CCeDDoc::ShowDefinedEventsDlg()
{
	CDefinedEventsDlg DefinedEventsDlg;
	
	DefinedEventsDlg.SetDocument( (CCeDDoc *)this );
	if( DefinedEventsDlg.DoModal() == IDOK )
	{
		SetModifiedFlag();
		UpdateAllViews( NULL );
	}
}

void CCeDDoc::OnEventsDefinedevents() 
{
	ShowDefinedEventsDlg();
}

void CCeDDoc::OnEditFilepaths() 
{
	CDlgFilePaths FilePathsSearchReplace;

	if( FilePathsSearchReplace.DoModal() )
	{
		//---	search through the motions
		m_CharacterList.SearchReplaceFilePaths( FilePathsSearchReplace.m_SearchString,
												FilePathsSearchReplace.m_ReplaceString,
												FilePathsSearchReplace.m_WhereToLook );

		//---	search through the meshes
		m_MeshList.SearchReplaceFilePaths( FilePathsSearchReplace.m_SearchString,
										   FilePathsSearchReplace.m_ReplaceString,
										   FilePathsSearchReplace.m_WhereToLook );

		//---	update the display
		UpdateAllViews( NULL );
	}
}

///////////////////////////////////////////////////////////////////////////////
BOOL CCeDDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
#ifndef NO_TIMESTAMP_CHECKING
	m_TimeStamp = 0;
#endif

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
BOOL CCeDDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
#ifndef NO_TIMESTAMP_CHECKING
	//---	set the time stamp for this document before it is loaded (this protects even if it is changed while loading)
	CFileStatus s;
	CFile::GetStatus( lpszPathName, s );
	m_TimeStamp = MakeTimeStamp( s.m_mtime );
#endif

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// TODO: Add your specialized creation code here
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CCeDDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
#ifndef NO_TIMESTAMP_CHECKING
	m_TimeStamp	= 0;
#endif

	BOOL ret = CDocument::OnSaveDocument(lpszPathName);
	
#ifndef NO_TIMESTAMP_CHECKING
	//---	get the time stamp of the new file
	CFileStatus s;
	CFile::GetStatus( lpszPathName, s );
	m_TimeStamp = MakeTimeStamp( s.m_mtime );
#endif
	
	return ret;
}


///////////////////////////////////////////////////////////////////////////////
//
//	MakeTimeStamp
//
//	stores a 32 bit time stamp of the source file at the time of building
//
DWORD CCeDDoc::MakeTimeStamp( CTime &rTime )
{
#ifndef NO_TIMESTAMP_CHECKING
	//---	build 32 bit time stamp

	//---	year is condenced to save room.  It is given 6 bits( 64 years ) and starts from 1970.
	DWORD Year			= (DWORD) rTime.GetYear() - 1970;
	DWORD YearBits		= (DWORD) ((((Year < 0) ? 0 : Year)	<< 26) & 0xfc000000);
	DWORD MonthBits		= (DWORD) ((rTime.GetMonth()		<< 22) & 0x03c00000);
	DWORD DayBits		= (DWORD) ((rTime.GetDay()			<< 17) & 0x003e0000);
	DWORD HourBits		= (DWORD) ((rTime.GetHour()			<< 12) & 0x0001f000);
	DWORD MinuteBits	= (DWORD) ((rTime.GetMinute()		<< 6)  & 0x00000fc0);
	DWORD SecondBits	= (DWORD) ((rTime.GetSecond()		<< 0)  & 0x0000003f);

	return YearBits | MonthBits | DayBits | HourBits | MinuteBits | SecondBits;
#else
	return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
BOOL CCeDDoc::CheckFileChange( void )
{
#ifndef NO_TIMESTAMP_CHECKING
	//---	do nothing until a file is associated with this file
	if( m_TimeStamp == 0 )
		return FALSE;

	//---	get the source file time stamp
	CFileStatus s;
	CFile::GetStatus( GetPathName(), s );
	DWORD TimeStamp = MakeTimeStamp( s.m_mtime );

	//---	compare the source file time stamp to the loaded time stamp
	if( TimeStamp != m_TimeStamp )
	{
		m_TimeStamp = TimeStamp;
		return TRUE;
	}

	return FALSE;
#else
	return 0;
#endif
}

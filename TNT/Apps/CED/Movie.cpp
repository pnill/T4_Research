// Movie.cpp : implementation file
//

#include "stdafx.h"

#include "CeD.h"
#include "CeDDoc.h"
#include "Movie.h"
#include "ieArchive.h"
#include "Actor.h"
#include "Shot.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovie Implementation

IMPLEMENT_DYNCREATE(CMovie, CObject)

/////////////////////////////////////////////////////////////////////////////
// CMovie Constructors

void CMovie::Initialize()
{
	m_Name = "New Movie" ;
	m_pCurActor = NULL ;
	m_NumFrames = 500 ;
	m_NumLayers = 0;
	m_nFrame = 0 ;
	m_ShotList.m_pMovie = this ;
	m_UseShotList = false ;
    m_PreviewWidth = 640 ;
    m_PreviewHeight = 480 ;

	for( int i=0; i<NUM_EXPORT_TYPES; i++ )
	{
		m_ExportPaths[i]	= "";
		m_ExportActive[i]	= FALSE;
	}
}

CMovie::CMovie()
{
	//---	Reset Data Members
	Initialize () ;
}

CMovie::~CMovie()
{
	//---	Destroy the Texture
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CMovie Implementation

bool CMovie::Create()
{
	return true ;
}

void CMovie::Destroy()
{
	//---	Initialize Data Members
	Initialize () ;
}

void CMovie::Copy (CMovie &Dst, CMovie &Src)
{
	ASSERT(0); // not yet implemented
}



/////////////////////////////////////////////////////////////////////////////
// CMovie Overridden operators

CMovie &CMovie::operator=( CMovie &ptMovie )
{
	Copy (*this, ptMovie) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CMovie serialization

void CMovie::Serialize(CieArchive& a)
{
//	CScrEdDoc *pDoc = (CScrEdDoc*)a.m_pDocument ;
	int i;
	int NumPaths;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_MOVIE) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_MOVIE_NAME, m_Name ) ;
		a.WriteTaggeds32( IE_TAG_MOVIE_NUMFRAMES, m_NumFrames ) ;
		a.WriteTaggeds32( IE_TAG_MOVIE_FRAME, m_nFrame ) ;
		a.WriteTaggedbool( IE_TAG_MOVIE_USESHOTLIST, m_UseShotList ) ;
		a.WriteTaggeds32( IE_TAG_MOVIE_PREVIEWWIDTH, m_PreviewWidth ) ;
		a.WriteTaggeds32( IE_TAG_MOVIE_PREVIEWHEIGHT, m_PreviewHeight ) ;
		m_ActorList.Serialize( a ) ;
		m_ShotList.Serialize( a ) ;

		//---	write the movie layers
		a.WriteTaggeds32( IE_TAG_MOVIE_LAYERS, m_NumLayers );
		for( i=0; i<m_NumLayers; i++ )
			a.WriteTaggedString( IE_TAG_MOVIE_LAYER_NAME, m_LayerNames[i] );

		//---	write the export paths
		a.WriteTaggeds32( IE_TAG_MOVIE_EXPORTPATHS, NUM_EXPORT_TYPES );
		for( i=0; i<NUM_EXPORT_TYPES; i++ )
		{
			a.WriteTaggedString( -1, m_ExportPaths[i] );
			a.WriteTaggedbool( -1, m_ExportActive[i] );
		}

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_MOVIE_END ) ;
	}
	else
	{
		//---	Load
        bool tbool ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_MOVIE_NAME:
				a.ReadString(m_Name) ;
				break ;
			case IE_TAG_MOVIE_NUMFRAMES:
				a.Reads32(m_NumFrames) ;
				break ;
			case IE_TAG_MOVIE_FRAME:
				a.Reads32(m_nFrame) ;
				break ;
			case IE_TAG_MOVIE_USESHOTLIST:
				a.Readbool(tbool) ;
				break ;
			case IE_TAG_MOVIE_PREVIEWWIDTH:
				a.Reads32(m_PreviewWidth) ;
				break ;
			case IE_TAG_MOVIE_PREVIEWHEIGHT:
				a.Reads32(m_PreviewHeight) ;
				break ;

			case IE_TAG_MOVIE_LAYERS:
				a.Reads32( m_NumLayers );
				for( i=0; i<m_NumLayers; i++ )
				{
					a.ReadTag();
					ASSERT(a.m_rTag == IE_TAG_MOVIE_LAYER_NAME);
					a.ReadString( m_LayerNames[i] );
				}
				break;

			case IE_TAG_MOVIE_EXPORTPATHS:
				a.Reads32( (int&)NumPaths );
				for( i=0; i<NumPaths; i++ )
				{
					a.ReadTag();
					a.ReadString( m_ExportPaths[i] );
					a.ReadTag();
					a.Readbool( m_ExportActive[i] );
				}
				break;

			case IE_TAG_ACTORLIST:
				m_ActorList.Serialize( a ) ;
				break ;

			case IE_TAG_SHOTLIST:
				m_ShotList.Serialize( a ) ;
				break ;

			case IE_TAG_MOVIE_END:
				done = TRUE ;
				break ;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMovie diagnostics

#ifdef _DEBUG
void CMovie::AssertValid() const
{
	CObject::AssertValid();
}

void CMovie::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CMovie operations

/////////////////////////////////////////////////////////////////////////////
//	Name Functions

//=================================================================================================
void CMovie::SetName( const char *pName )
{
	m_Name = pName ;
}

//=================================================================================================
const char *CMovie::GetName( )
{
	return m_Name ;
}


/////////////////////////////////////////////////////////////////////////////
//	Actor Functions

//=================================================================================================
CActor *CMovie::GetCurActor( )
{
	return m_pCurActor ;
}

//=================================================================================================
CActor *CMovie::SetCurActor( CActor *pActor )
{
	if( pActor )
	{
		pActor->m_Selected = true ;
	}
	m_pCurActor = pActor ;
	return m_pCurActor ;
}

/////////////////////////////////////////////////////////////////////////////
//	Layer Functions

//=================================================================================================
void CMovie::SetLayer( int Layer, CString& LayerName )
{
	if(( Layer < 0) || ( Layer >= m_NumLayers ))
		return;

	m_LayerNames[ Layer ] = LayerName;
}

//=================================================================================================
int CMovie::AddLayer( CString& LayerName )
{
	if( m_NumLayers >= MAX_MOVIE_LAYERS )
		return -1;

	m_LayerNames[ m_NumLayers++ ] = LayerName;
	return m_NumLayers-1;
}

//=================================================================================================
bool CMovie::RemoveLayer( CString& LayerName )
{
	s32 Layer = FindLayerByName( LayerName );
	if( Layer == -1 )
		return FALSE;

	return RemoveLayer( Layer );
}

//=================================================================================================
bool CMovie::RemoveLayer( int Layer )
{
	if(( Layer < 0) || ( Layer >= m_NumLayers ))
		return FALSE;

	m_NumLayers--;
	for(; Layer<m_NumLayers; Layer++ )
		m_LayerNames[Layer] = m_LayerNames[Layer+1];

	return TRUE;
}

//=================================================================================================
int CMovie::FindLayerByName( CString& LayerName )
{
	for(int i=0; i<m_NumLayers; i++)
		if( m_LayerNames[i] == LayerName )
			return i;

	return -1;
}

//=================================================================================================
CString& CMovie::GetLayer( int Layer )
{
	static CString NullString="";

	if(( Layer < 0 ) || ( Layer >= m_NumLayers ))
		return NullString;

	return m_LayerNames[Layer];
}

/////////////////////////////////////////////////////////////////////////////
//	Frame Functions

//=================================================================================================
int	CMovie::SetFrame( int nFrame )
{
	m_nFrame = nFrame ;
	return m_nFrame ;
}

//=================================================================================================
int	CMovie::GetFrame( )
{
	return m_nFrame ;
}

//=================================================================================================
int	CMovie::SetNumFrames( int NumFrames )
{
	m_NumFrames = NumFrames ;
	return m_NumFrames ;
}

//=================================================================================================
int	CMovie::GetNumFrames( )
{
	return m_NumFrames ;
}

//=================================================================================================
void CMovie::InsertFrames( int AtFrame, int nFrames )
{
	POSITION Pos = m_ActorList.GetHeadPosition() ;
	while( Pos )
	{
		CActor *pActor = m_ActorList.GetNext( Pos ) ;
		ASSERT( pActor ) ;
		pActor->InsertFrames( AtFrame, nFrames ) ;
	}

	Pos = m_ShotList.GetHeadPosition() ;
	while( Pos )
	{
		CShot *pShot = m_ShotList.GetNext( Pos ) ;
		ASSERT( pShot ) ;
		if( pShot->GetFrame() >= AtFrame )
		{
			pShot->SetFrame( pShot->GetFrame() + nFrames ) ;
			if( nFrames < 0 )
				pShot->SetFrame( max( AtFrame, pShot->GetFrame() ) );
		}
	}
}

//=================================================================================================
void CMovie::DeleteFrames( int AtFrame, int nFrames )
{
	POSITION Pos = m_ActorList.GetHeadPosition() ;
	while( Pos )
	{
		CActor *pActor = m_ActorList.GetNext( Pos ) ;
		ASSERT( pActor ) ;
		pActor->DeleteFrames( AtFrame, nFrames ) ;
	}

	Pos = m_ShotList.GetHeadPosition() ;
	while( Pos )
	{
		CShot *pShot = m_ShotList.GetNext( Pos ) ;
		ASSERT( pShot ) ;
		if( pShot->GetFrame() >= AtFrame )
		{
			s32 frame = pShot->GetFrame() - nFrames ;
			pShot->SetFrame( max( AtFrame, frame ) ) ;
		}
	}
}

//=================================================================================================
void CMovie::CharacterDeleted( CCharacter *pCharacter )
{
	//---	Validate all Character Pointers in Actors
	POSITION Pos = m_ActorList.GetHeadPosition() ;
	while( Pos )
	{
		CActor *pActor = m_ActorList.GetNext( Pos ) ;
		ASSERT( pActor ) ;
		if( pActor->m_pCharacter == pCharacter ) pActor->m_pCharacter = NULL ;
	}
}

//=================================================================================================
void CMovie::MotionDeleted( CMotion *pMotion )
{
	//---	Validate all Actors
	POSITION Pos = m_ActorList.GetHeadPosition() ;
	while( Pos )
	{
		CActor *pActor = m_ActorList.GetNext( Pos ) ;
		ASSERT( pActor ) ;
		pActor->MotionDeleted( pMotion ) ;
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Shot Functions

void CMovie::ShotDelete( CShot *pShot )
{
	CShot *pScan = NULL ;
	CShot *pPrev = NULL ;

	POSITION Pos = m_ShotList.GetHeadPosition() ;
	while( Pos )
	{
		POSITION Pos2 = Pos ;
		pPrev = pScan ;
		pScan = m_ShotList.GetNext( Pos ) ;

		if( pScan == pShot )
		{
			m_ShotList.RemoveAt( Pos2 ) ;
			delete pShot ;
			break ;
		}
	}
}

//=================================================================================================
CShot *CMovie::GetCurShot( )
{
	CShot *pShotFound = NULL ;

	POSITION Pos = m_ShotList.GetHeadPosition() ;
	while( Pos )
	{
		CShot *pShot = m_ShotList.GetNext( Pos ) ;
		if( pShot->GetFrame() <= GetFrame() )
			pShotFound = pShot ;
	}

	return pShotFound ;
}

//=================================================================================================
CActor *CMovie::ShotGetEye( int nFrame )
{
	CActor *pActor = NULL ;

	POSITION Pos = m_ShotList.GetHeadPosition() ;
	while( Pos )
	{
		CShot *pShot = m_ShotList.GetNext( Pos ) ;
		if( (pShot->GetFrame() <= nFrame) && pShot->GetEye() )
			pActor = pShot->GetEye() ;
	}

	return pActor ;
}

//=================================================================================================
CActor *CMovie::ShotGetTarget( int nFrame )
{
	CActor *pActor = NULL ;

	POSITION Pos = m_ShotList.GetHeadPosition() ;
	while( Pos )
	{
		CShot *pShot = m_ShotList.GetNext( Pos ) ;
		if( (pShot->GetFrame() <= nFrame) && pShot->GetTarget() )
			pActor = pShot->GetTarget() ;
	}

	return pActor ;
}

//=================================================================================================
f32 CMovie::ShotGetFOV( int nFrame )
{
	CShot*	pShot		= NULL;
	CShot*	pLastShot	= NULL;
	CActor*	pActor		= NULL ;
	radian	fov;

	POSITION Pos = m_ShotList.GetHeadPosition() ;
	while( Pos )
	{
		pLastShot = pShot;
		pShot = m_ShotList.GetNext( Pos ) ;
		if( nFrame < pShot->GetFrame() )
			break;
	}

	if( pShot && (pShot->GetFrame() <= nFrame) )
		fov = (f32)pShot->GetFOV();
	else if( !pLastShot )
		fov = 60.0f;
	else if( !pLastShot->GetInterpolate() )
		fov = (f32)pLastShot->GetFOV();
	else
	{
		ASSERT(pShot);
		f32 FOV0	= (f32)pLastShot->GetFOV();
		f32 FOV1	= (f32)pShot->GetFOV();
		f32 Ratio	= (f32)(nFrame-pLastShot->GetFrame()) / (f32)(pShot->GetFrame() - pLastShot->GetFrame());
		fov = FOV0 + (FOV1-FOV0)*Ratio;
	}

	return fov ;
}

//=================================================================================================
void CMovie::Merge( CMovie *pMovie )
{
	CMapPtrToPtr	Map ;

	//---	Add All Actors to this Movie
	POSITION Pos = pMovie->m_ActorList.GetHeadPosition( ) ;
	while( Pos )
	{
		CActor *pActor = pMovie->m_ActorList.GetNext( Pos ) ;
		if( pActor )
		{
			CActor *pNewActor = new CActor ;
			ASSERT( pNewActor ) ;
			*pNewActor = *pActor ;
			Map.SetAt( pActor, pNewActor ) ;
			m_ActorList.AddTail( pNewActor ) ;
		}
	}

	//---	Add All Shots to this Movie
	Pos = pMovie->m_ShotList.GetHeadPosition( ) ;
	while( Pos )
	{
		CShot *pShot = pMovie->m_ShotList.GetNext( Pos ) ;
		if( pShot )
		{
			CShot *pNewShot = new CShot ;
			ASSERT( pNewShot ) ;
			*pNewShot = *pShot ;
			pNewShot->SetMovie( this ) ;
			void *Result ;
			if( Map.Lookup( pShot->GetEye(), Result ) )
				pNewShot->SetEye( (CActor*)Result ) ;
			else
				pNewShot->SetEye( NULL ) ;
			if( Map.Lookup( pShot->GetTarget(), Result ) )
				pNewShot->SetTarget( (CActor*)Result ) ;
			else
				pNewShot->SetTarget( NULL ) ;
			m_ShotList.InsertSorted( pNewShot ) ;
		}
	}
}

//=================================================================================================
void CMovie::SetPreviewWidth( s32 Width )
{
    m_PreviewWidth = Width ;
}

//=================================================================================================
void CMovie::SetPreviewHeight( s32 Height )
{
    m_PreviewHeight = Height ;
}

//=================================================================================================
s32 CMovie::GetPreviewWidth( )
{
    return m_PreviewWidth ;
}

//=================================================================================================
s32 CMovie::GetPreviewHeight( )
{
    return m_PreviewHeight ;
}

//=================================================================================================

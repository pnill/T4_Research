// Character.cpp : implementation file
//

#include "stdafx.h"

#include "Character.h"
#include "ieArchive.h"
#include "MFCUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCharacter Implementation

IMPLEMENT_DYNCREATE(CCharacter, CObject)

/////////////////////////////////////////////////////////////////////////////
// CCharacter Constructors

void CCharacter::Initialize()
{
	m_FaceDirAngle				= 0 ;
	m_nFrame					= 0 ;
	m_pCurSkelPoint				= NULL ;
	m_pCurMotion				= NULL ;
	m_pSkeleton					= NULL ;
	m_NoExport					= false ;
	m_ExportEnableGAMECUBE		= false ;
	m_ExportEnablePC			= false ;
	m_ExportEnableXBOX			= false ;
	m_ExportEnablePS2			= false ;
	m_ExportOptForceUpper		= false ;
	m_ExportOptCollisionBubbles	= false ;
	m_ExportOptMotionFlags		= false ;
	m_ExportFrontEndSpecific	= false ;

	m_ExportFrameRate			= 30.0f ;
	m_ExportBits				= 12 ;
	m_ExportFramesPerBlock		= 32 ;

	m_Name = "New Character" ;
}

CCharacter::CCharacter()
{
	//---	Reset Data Members
	Initialize () ;
}

CCharacter::~CCharacter()
{
	//---	Destroy the Texture
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CCharacter Implementation

bool CCharacter::Create()
{
	return true ;
}

void CCharacter::Destroy()
{
	if( m_pSkeleton )
		delete m_pSkeleton ;

	m_MotionList.DeleteAllMotions() ;

	//---	Initialize Data Members
	Initialize () ;
}

void CCharacter::Copy (CCharacter &Dst, CCharacter &Src)
{
}

/////////////////////////////////////////////////////////////////////////////
// CCharacter Overridden operators

CCharacter &CCharacter::operator=( CCharacter &ptCharacter )
{
	Copy (*this, ptCharacter) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
bool CCharacter::SearchReplaceFilePaths( CString Search, CString Replace, s32 Method )
{
	return m_MotionList.SearchReplaceFilePaths( Search, Replace, Method );
}

/////////////////////////////////////////////////////////////////////////////
// CCharacter serialization

void CCharacter::Serialize(CieArchive& a)
{
//	CScrEdDoc *pDoc = (CScrEdDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_CHARACTER) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_CHARACTER_NAME, m_Name ) ;
		if( m_pSkeleton )
			m_pSkeleton->Serialize( a ) ;
		m_SkelPointList.Serialize( a ) ;
		m_MotionList.Serialize( a, this ) ;
		m_AttachedEvents.Serialize( a );
		a.WriteTaggedbool( IE_TAG_CHARACTER_NOEXPORT, m_NoExport ) ;
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_ENABLEGAMECUBE, m_ExportEnableGAMECUBE ) ;
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_ENABLEPC, m_ExportEnablePC ) ;
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_ENABLEXBOX, m_ExportEnableXBOX ) ;
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_ENABLEPS2, m_ExportEnablePS2 ) ;
		a.WriteTaggedString( IE_TAG_CHARACTER_EXPORT_FILEGAMECUBE, m_ExportFileGAMECUBE ) ;
		a.WriteTaggedString( IE_TAG_CHARACTER_EXPORT_FILEPC, m_ExportFilePC ) ;
		a.WriteTaggedString( IE_TAG_CHARACTER_EXPORT_FILEXBOX, m_ExportFileXBOX ) ;
		a.WriteTaggedString( IE_TAG_CHARACTER_EXPORT_FILEPS2, m_ExportFilePS2 ) ;
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_OPTFORCEUPPER, m_ExportOptForceUpper ) ;
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_OPTCOLLISIONBUBBLES, m_ExportOptCollisionBubbles ) ;
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_OPTMOTIONFLAGS, m_ExportOptMotionFlags ) ;
		a.WriteTaggedf32( IE_TAG_CHARACTER_EXPORT_FRAMERATE, m_ExportFrameRate );
		a.WriteTaggeds32( IE_TAG_CHARACTER_EXPORT_BITS, m_ExportBits );
		a.WriteTaggeds32( IE_TAG_CHARACTER_EXPORT_FRAMESPERBLOCK, m_ExportFramesPerBlock );

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_CHARACTER_END ) ;
	}
	else
	{
		//---	Load
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_CHARACTER_NAME:
				a.ReadString(m_Name) ;
				break ;

			case IE_TAG_SKELETON:
				m_pSkeleton = new CSkeleton ;
				ASSERT( m_pSkeleton ) ;
				m_pSkeleton->Serialize( a ) ;
				break ;

			case IE_TAG_SKELPOINTLIST:
				m_SkelPointList.Serialize( a ) ;
				break ;

			case IE_TAG_MOTIONLIST:
				m_MotionList.Serialize( a, this ) ;
				break ;

			case IE_TAG_EVENTLIST:
				m_AttachedEvents.Serialize( a );
				break;

			case IE_TAG_CHARACTER_NOEXPORT:
				a.Readbool( m_NoExport ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_ENABLEGAMECUBE:
				a.Readbool( m_ExportEnableGAMECUBE ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_ENABLEPC:
				a.Readbool( m_ExportEnablePC ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_ENABLEXBOX:
				a.Readbool( m_ExportEnableXBOX ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_ENABLEPS2:
				a.Readbool( m_ExportEnablePS2 ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_FILEGAMECUBE:
				a.ReadString( m_ExportFileGAMECUBE ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_FILEPC:
				a.ReadString( m_ExportFilePC ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_FILEXBOX:
				a.ReadString( m_ExportFileXBOX ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_FILEPS2:
				a.ReadString( m_ExportFilePS2 ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_OPTFORCEUPPER:
				a.Readbool( m_ExportOptForceUpper ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_OPTCOLLISIONBUBBLES:
				a.Readbool( m_ExportOptCollisionBubbles ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_OPTMOTIONFLAGS:
				a.Readbool( m_ExportOptMotionFlags ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_FRAMERATE:
				a.Readf32( m_ExportFrameRate ) ;
				break ;

			case IE_TAG_CHARACTER_EXPORT_BITS:
				a.Reads32( m_ExportBits );
				break;

			case IE_TAG_CHARACTER_EXPORT_FRAMESPERBLOCK:
				a.Reads32( m_ExportFramesPerBlock );
				break;

			case IE_TAG_CHARACTER_END:
				done = TRUE ;
				break ;

			default:
				ASSERT(0) ;
				MessageBox( NULL, "File may be corrupted, or it was created with a newer version of the editor.", "Error loading document", MB_ICONERROR );
				a.ReadSkip() ;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCharacter diagnostics

#ifdef _DEBUG
void CCharacter::AssertValid() const
{
	CObject::AssertValid();
}

void CCharacter::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CCharacter operations

void CCharacter::SetName( const char *pName )
{
	m_Name = pName ;
}

const char *CCharacter::GetName( )
{
	return m_Name ;
}

bool CCharacter::AddMotionToList( CMotion *pMotion )
{
	if( pMotion )
	{
		pMotion->m_pCharacter = this ;
		m_MotionList.AddTail( pMotion ) ;
	}
	return true ;
}

bool CCharacter::ImportSkeletonFromFile( const char *pFileName )
{
	bool	Success = false ;
	CSkeleton *pSkeleton = new CSkeleton ;

	if( pSkeleton->CreateFromFile( pFileName ) )
	{
		if( m_pSkeleton )
		{
			delete m_pSkeleton ;
		}

		m_pSkeleton = pSkeleton ;
		Success = true ;
	}
	else
	{
		delete pSkeleton ;
	}

	return Success ;
}

CMotion *CCharacter::ImportMotionFromFile( const char *pFileName )
{
	CMotion *pMotion;

	//---	if this motion file already exists in this character's motion list, do not load it
	pMotion = FindMotionByName( pFileName );
	if( pMotion )
	{
		CString msg;
		msg.Format( "File '%s' already imported", pFileName );
		MessageBox( NULL, msg, "File Already Imported", MB_ICONERROR );
		return pMotion;
	}

	bool	Success = false ;
	pMotion = new CMotion ;
	ASSERT( pMotion ) ;
	ASSERT( m_pSkeleton ) ;
//	pMotion->m_pSkeleton = m_pSkeleton ;
	pMotion->m_pCharacter = this ;

	pMotion->m_PathName = pFileName ;
	pMotion->m_ExportName = MFCU_GetFName( pFileName ) ;

	if( pMotion->CreateFromFile( pFileName ) )
	{
		AddMotionToList( pMotion ) ;
	}
	else
	{
		delete pMotion ;
		pMotion = NULL ;
	}

	return pMotion ;
}

int CCharacter::GetNumMotions()
{
	return m_MotionList.GetCount() ;
}

CMotion *CCharacter::GetMotion( int iMotion )
{
	CMotion	*pMotion = (CMotion*)m_MotionList.IndexToPtr( iMotion ) ;
	return pMotion ;
}

CMotion *CCharacter::SetCurMotion( int iMotion )
{
	m_pCurMotion = (CMotion*)m_MotionList.IndexToPtr( iMotion ) ;
	return m_pCurMotion ;
}

CMotion *CCharacter::SetCurMotion( CMotion *pMotion )
{
	m_pCurMotion = pMotion ;
	AnimGotoStart() ;
	return m_pCurMotion ;
}

CMotion *CCharacter::GetCurMotion()
{
	return m_pCurMotion ;
}

void CCharacter::DeleteMotion( CMotion *pMotion )
{
	CMotion *pScan = NULL ;
	CMotion *pPrevMotion = NULL ;

	POSITION Pos = m_MotionList.GetHeadPosition() ;
	while( Pos )
	{
		POSITION Pos2 = Pos ;
		pPrevMotion = pScan ;
		pScan = m_MotionList.GetNext( Pos ) ;

		if( pScan == pMotion )
		{
			m_MotionList.RemoveAt( Pos2 ) ;
			delete pMotion ;
			if( GetCurMotion() == pMotion )
			{
				CMotion *pNextMotion = NULL ;
				if( Pos )
					pNextMotion = (CMotion*)m_MotionList.GetAt( Pos ) ;
				if( pNextMotion )
					SetCurMotion( pNextMotion ) ;
				else if( pPrevMotion )
					SetCurMotion( pPrevMotion ) ;
				else
					SetCurMotion( NULL ) ;
			}
		}
		else if( pScan->m_pLinkMotion == pMotion )
		{
			pScan->m_pLinkMotion = NULL;
		}
	}
}

CMotion *CCharacter::FindMotionByName( const char *pPathName )
{
	CMotion *pScan = NULL ;
	CMotion	*pFound = NULL ;

	POSITION Pos = m_MotionList.GetHeadPosition() ;
	while( Pos )
	{
		pScan = m_MotionList.GetNext( Pos ) ;

		if( stricmp( pScan->GetPathName(), pPathName ) == 0 )
		{
			pFound = pScan ;
			break ;
		}
	}

	return pFound ;
}

CMotion *CCharacter::GetMatchingMotion( CMotion *pMotion )
{
	CMotion *pFound = NULL ;

	if( pMotion )
	{
		pFound = FindMotionByName( pMotion->GetPathName() ) ;
		if( pFound == NULL )
		{
			pFound = ImportMotionFromFile( pMotion->GetPathName() ) ;
		}
	}

	return pFound ;
}

CSkeleton *CCharacter::GetSkeleton()
{
	return m_pSkeleton ;
}

float CCharacter::GetCurFrame( )
{
	return m_nFrame ;
}

void CCharacter::SetCurFrame( float nFrame )
{
	if( m_pCurMotion )
	{
		if( m_pCurMotion->GetNumFrames() <= 0 )
		{
			m_nFrame = 0 ;
			m_pSkeleton->SetNeutralPose() ;
			return ;
		}

		while( nFrame >= m_pCurMotion->GetNumFrames() )
			nFrame -= m_pCurMotion->GetNumFrames() ;
		while( nFrame < 0 )
			nFrame += m_pCurMotion->GetNumFrames() ;
	}
	else
	{
		nFrame = 0 ;
	}

	m_nFrame = nFrame ;

	if( m_pSkeleton )
		m_pSkeleton->SetPose( m_pCurMotion, (float)m_nFrame ) ;
}

void CCharacter::SetPose( CMotion *pMotion, float nFrame, bool Blend )
{
	if( pMotion )
	{
		if( pMotion->GetNumFrames() <= 0 )
		{
			nFrame = 0 ;
			m_pSkeleton->SetNeutralPose() ;
			return ;
		}

		while( nFrame >= pMotion->GetNumFrames() )
			nFrame -= pMotion->GetNumFrames() ;
		while( nFrame < 0 )
			nFrame += pMotion->GetNumFrames() ;
	}
	else
	{
		nFrame = 0 ;
	}

	if( m_pSkeleton )
		m_pSkeleton->SetPose( pMotion, nFrame, Blend ) ;
}

void CCharacter::AnimGotoStart()
{
	SetCurFrame( 0 ) ;
}

void CCharacter::AnimPreviousFrame()
{
	SetCurFrame( m_nFrame-1 ) ;
}

void CCharacter::AnimNextFrame()
{
	SetCurFrame( m_nFrame+1 ) ;
}

void CCharacter::AnimGotoEnd()
{
	SetCurFrame( (f32)(m_pCurMotion->GetNumFrames()-1) ) ;
}





void CCharacter::SetCurSkelPoint( CSkelPoint *pSkelPoint )
{
	CSkelPoint *pNew = NULL ;

	POSITION Pos = m_SkelPointList.GetHeadPosition() ;
	while( Pos )
	{
		CSkelPoint *pScan = m_SkelPointList.GetNext( Pos ) ;
		if( pScan == pSkelPoint )
		{
			pNew = pSkelPoint ;
			break ;
		}
	}

	m_pCurSkelPoint = pNew ;
}

CSkelPoint *CCharacter::GetCurSkelPoint( )
{
	return m_pCurSkelPoint ;
}

void CCharacter::DeleteSkelPoint( CSkelPoint *pSkelPoint )
{
	CSkelPoint *pScan = NULL ;
	CSkelPoint *pPrevEvent = NULL ;

	int Index = m_SkelPointList.PtrToIndex( pSkelPoint ) ;
	if( Index >= 0 )
	{
		POSITION Pos = m_SkelPointList.GetHeadPosition() ;
		while( Pos )
		{
			POSITION Pos2 = Pos ;
			pPrevEvent = pScan ;
			pScan = m_SkelPointList.GetNext( Pos ) ;

			if( pScan == pSkelPoint )
			{
				m_SkelPointList.RemoveAt( Pos2 ) ;
				delete pSkelPoint ;
				if( GetCurSkelPoint() == pSkelPoint )
					SetCurSkelPoint( NULL ) ;
				break ;
			}
		}

		//---	Send Delete to all Motions
		Pos = m_MotionList.GetHeadPosition( ) ;
		while( Pos )
		{
			CMotion *pMotion = m_MotionList.GetNext( Pos ) ;
			ASSERT( pMotion ) ;
			pMotion->SkelPointDeleteCheck( Index ) ;
		}
	}
}

CSkelPoint *CCharacter::AddSkelPoint( )
{
	CSkelPoint *pSkelPoint = new CSkelPoint() ;
	ASSERT( pSkelPoint ) ;
	m_SkelPointList.AddTail( pSkelPoint ) ;
	SetCurSkelPoint( pSkelPoint ) ;

	return pSkelPoint ;
}

CEventList* CCharacter::GetAttachedEvents()
{
	return &m_AttachedEvents;
}

s32 CCharacter::MotionToExportIndex( CMotion *pMotion )
{
	s32 Index=0;
	POSITION pos = m_MotionList.GetHeadPosition();
	CMotion* pCurMotion;

	if( !pMotion || !pMotion->m_bExport )
		return -1;
	
	while( pos )
	{
		pCurMotion = m_MotionList.GetNext( pos );
		if( pCurMotion == pMotion )
			break;

		if( pCurMotion->m_bExport )
			Index++;
	}

	if( pCurMotion != pMotion )
		return -1;

	return Index;
}

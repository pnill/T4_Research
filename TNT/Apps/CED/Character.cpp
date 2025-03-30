// Character.cpp : implementation file
//

#include "stdafx.h"

#include "Character.h"
#include "ieArchive.h"
#include "MFCUtils.h"
#include "DlgMotion.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
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
	s32 i;

	m_FaceDirAngle				= 0 ;
	m_nFrame					= 0 ;
	m_pCurSkin					= NULL ;
	m_pCurSkelPoint				= NULL ;
	m_pCurMotion				= NULL ;
	m_pSecondMotion				= NULL ;
	m_SecondMotionRatio			= 0.0f ;
	m_pSkeleton					= NULL ;
	m_NoExport					= false ;
	m_ExportEnableGAMECUBE		= false ;
	m_ExportEnablePC			= false ;
	m_ExportEnableXBOX			= false ;
	m_ExportEnablePS2			= false ;
	m_ExportOptForceUpper		= false ;
	m_ExportOptCollisionBubbles	= false ;
	m_ExportOptMotionFlags		= false ;

	m_ExportFrameRate			= 30.0f ;
	m_ExportBits				= 12 ;
	m_ExportFramesPerBlock		= 32 ;

	m_bExportQuaternion			= TRUE;
	m_bExportQuatW				= FALSE;
	m_bExportWorldRots			= FALSE;
	m_bExportGlobalRots			= FALSE;
	m_bExportQuatBlending		= FALSE;
	m_bExportPropQuatBlending	= FALSE;

	m_pLastMotion				= NULL ;
	m_bBlendedPlaybackActive	= FALSE ;

	m_Name						= "New Character" ;

	m_WorldBase.Identity();

	m_FaceDirRecipeCount		= 0;
	m_FaceDirRecipeID			= -1;

	m_bExportDataFile			= TRUE;
	m_bExportHeaderFile			= TRUE;
	m_bExportTextFile			= FALSE;
	m_bExportHeaderFilePath		= TRUE;

	m_bRenderSkinTranslucent	= FALSE;
	m_ViewMotionBlended			= TRUE;

	m_SkeletonPathName			= "";

	m_ExportMotionPrefix		= "ID_";
	m_ExportEventPrefix			= "EV_";

	//---	initialize the prop definitions
	for( i=0; i<MAX_NUM_PROPS; i++ )
	{
		m_PropDefs[i].SetIndex( i );
		m_PropDefs[i].SetCharacter( this );
	}
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
	s32 i;
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
		m_SkinList.Serialize( a );
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
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_DATAFILE, m_bExportDataFile );
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_HEADERFILE, m_bExportHeaderFile );
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_TEXTFILE, m_bExportTextFile );
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_HEADERPATHNAME, m_bExportHeaderFilePath );
		a.WriteTaggedString( IE_TAG_CHARACTER_SKELETONFILE, m_SkeletonPathName );
		a.WriteTaggedString( IE_TAG_CHARACTER_EXPORT_MOTIONPREFIX, m_ExportMotionPrefix );
		a.WriteTaggedString( IE_TAG_CHARACTER_EXPORT_EVENTPREFIX, m_ExportEventPrefix );

		//---	Write out the FaceDir recipe
		a.WriteTaggeds32( IE_TAG_CHARACTER_FACEDIRRECIPE, m_FaceDirRecipeCount );
		for( i=0; i<m_FaceDirRecipeCount; i++ )
		{
			a.WriteTaggedString( IE_TAG_CHARACTER_FACEDIRRECIPE, CString(m_FaceDirRecipe[i].BoneName) );
			a.WriteTaggeds32( IE_TAG_CHARACTER_FACEDIRRECIPE, m_FaceDirRecipe[i].Weight );
		}

		//---	export flags
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_QUATERNION, m_bExportQuaternion );
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_QUATWVALUE, m_bExportQuatW );
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_WORLDROT, m_bExportWorldRots );
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_GLOBALROT, m_bExportGlobalRots );
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_QUATBLEND, m_bExportQuatBlending );
		a.WriteTaggedbool( IE_TAG_CHARACTER_EXPORT_PROPQUATBLEND, m_bExportPropQuatBlending );

		//---	Get Index for LinkCharacter and LinkMotion and Write those
		a.WriteTaggeds32( IE_TAG_CHARACTER_PROPDEFS, MAX_NUM_PROPS );
		for( i=0; i<MAX_NUM_PROPS; i++ )
			m_PropDefs[i].Serialize( a );

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_CHARACTER_END ) ;
	}
	else
	{
		bool	b;
		s32		Count;

		m_ExportEventPrefix = "ID_";	// set this value to the old value for any loaded file initialy.  If the file is a newer one, it will replace this value.

		//---	Load
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_CHARACTER_NAME:
				a.ReadString( m_Name ) ;
				break ;

			case IE_TAG_SKELETON:
				m_pSkeleton = new CSkeleton ;
				ASSERT( m_pSkeleton ) ;
				m_pSkeleton->Serialize( a ) ;
				MapFaceDirRecipeToSkel();
				break ;

			case IE_TAG_SKELPOINTLIST:
				m_SkelPointList.Serialize( a ) ;
				break ;

			case IE_TAG_SKINLIST:
				m_SkinList.Serialize( a ) ;
				if( m_pSkeleton )
					FitSkinsToSkeleton();
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

			case IE_TAG_CHARACTER_SKELETONFILE:
				a.ReadString( m_SkeletonPathName ) ;
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

			case IE_TAG_CHARACTER_FACEDIRRECIPE:
				a.Reads32(m_FaceDirRecipeCount);
				for( i=0; i<m_FaceDirRecipeCount; i++ )
				{
					a.ReadTag();
					a.ReadString( m_FaceDirRecipe[i].BoneName );
					a.ReadTag();
					a.Reads32( m_FaceDirRecipe[i].Weight );
				}

				//---	map the face dir recipe to the current skeleton
				if( m_pSkeleton )
					MapFaceDirRecipeToSkel();

				break ;

			case IE_TAG_CHARACTER_EXPORT_DATAFILE:
				a.Readbool( m_bExportDataFile );
				break ;

			case IE_TAG_CHARACTER_EXPORT_HEADERFILE:
				a.Readbool( m_bExportHeaderFile );
				break ;

			case IE_TAG_CHARACTER_EXPORT_TEXTFILE:
				a.Readbool( m_bExportTextFile );
				break ;

			case IE_TAG_CHARACTER_EXPORT_HEADERPATHNAME:
				a.Readbool( m_bExportHeaderFilePath );
				break ;

			case IE_TAG_CHARACTER_EXPORT_WORLDROT:
				a.Readbool( m_bExportWorldRots );
				break;

			case IE_TAG_CHARACTER_EXPORT_GLOBALROT:
				a.Readbool( m_bExportGlobalRots );
				break;

			case IE_TAG_CHARACTER_EXPORT_QUATERNION:
				a.Readbool( m_bExportQuaternion );
				break;

			case IE_TAG_CHARACTER_EXPORT_QUATWVALUE:
				a.Readbool( m_bExportQuatW );
				break;

			case IE_TAG_CHARACTER_EXPORT_QUATBLEND:
				a.Readbool( m_bExportQuatBlending );
				break;

			case IE_TAG_CHARACTER_EXPORT_PROPQUATBLEND:
				a.Readbool( m_bExportPropQuatBlending );
				break;

			case IE_TAG_CHARACTER_PROPDEFS:
				a.Reads32( Count );
				for( i=0; i<Count; i++ )
				{
					if( i<MAX_NUM_PROPS )
						m_PropDefs[i].Serialize( a );
					else
					{
						CPropDef P;
						P.Serialize( a );
					}
				}
				break;

			// no longer used, but don't remove because the values must remain constant
			case IE_TAG_CHARACTER_EXPORT_QUATBITS_ON:
				a.Readbool( b );
				break;

			// no longer used, but don't remove because the values must remain constant
			case IE_TAG_CHARACTER_EXPORT_QUATBITS:
				a.Reads32( Count );
				break;

			case IE_TAG_CHARACTER_EXPORT_MOTIONPREFIX:
				a.ReadString( m_ExportMotionPrefix );
				break;

			case IE_TAG_CHARACTER_EXPORT_EVENTPREFIX:
				a.ReadString( m_ExportEventPrefix );
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

		//---
		//
		//	Fix up the motion pointers used within motions which are created by blending together other motions
		//
		//---
		POSITION pos = m_MotionList.GetHeadPosition();
		while( pos )
		{
			CMotion* pMotion = m_MotionList.GetNext( pos );
			for( i=0; i<pMotion->m_BlendedMotionCount; i++ )
				pMotion->m_BlendedMotions[i].pMotion = GetMotion( (s32)pMotion->m_BlendedMotions[i].pMotion );
		}

#if 0 // For ASB to remove bone connections for all events, use this when you are importing a new skeleton for the body.
      // It will disconnect all events from their bones and keep them as a position. If the animation changes, the event
      // will no longer follow that bone, but it will at least allow you to still import a new skeleton to the model.
pos = m_MotionList.GetHeadPosition();
CMotion* pMotion;
while( pos )
{
	pMotion = m_MotionList.GetNext( pos );

	POSITION pos2 = pMotion->m_EventList.GetHeadPosition();
	CEvent* pEvent;
	while( pos2 )
	{
		pEvent = pMotion->m_EventList.GetNext( pos2 );
        pEvent->SetAttached( FALSE );
        pEvent->SetBone( -1 );
	}
}
#endif
   
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


////////////////////////////////////////////////////////////////////////////
bool CCharacter::ImportSkinFromFile( const char *pFileName )
{
	CSkin*	pNewSkin;
	xbool	success;

	//---	create space for the new skin
	pNewSkin = new CSkin;

	//---	attempt to load the file
	success = pNewSkin->CreateFromFile( pFileName );
	if( !success )
	{
		delete pNewSkin;
		return FALSE;
	}

	//---	attempt to map the skin to the skeleton, if one has been assigned
	if( m_pSkeleton )
	{
		success = pNewSkin->FitToSkeleton( *m_pSkeleton->GetSkel() );
		if( !success )
		{
			delete pNewSkin;
			return FALSE;
		}
	}

	//---	add the skin to the list
	m_SkinList.AddTail( pNewSkin );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
bool CCharacter::RemoveSkin( void )
{
	s32 Index = m_SkinList.PtrToIndex( (CObject*)GetCurSkin() );
	if( Index != -1 )
	{
		SetCurSkin( -1 );
		return RemoveSkin( Index );
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
bool CCharacter::RemoveSkin( int iSkin )
{
	ASSERT( ( iSkin >= 0 )&&( iSkin < GetNumSkins()) );
	if( ( iSkin < 0 )&&( iSkin >= GetNumSkins() ) )
		return FALSE;

	CSkin* pSkin = (CSkin*)m_SkinList.IndexToPtr( iSkin );
	if( pSkin )
	{
		m_SkinList.RemoveAt( m_SkinList.FindIndex( iSkin ) );
		delete pSkin;
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
CSkin* CCharacter::GetSkin( int iSkin )
{
	if( ( iSkin < 0 )||( iSkin >= GetNumSkins() ) )
		return NULL;

	return (CSkin*)m_SkinList.IndexToPtr( iSkin ) ;
}

////////////////////////////////////////////////////////////////////////////
void CCharacter::SetCurSkin( int iSkin )
{
	CSkin* pSkin = GetSkin( iSkin );

	//---	if a NULL skin is set, no skin will be drawn
	m_pCurSkin = pSkin;
}

////////////////////////////////////////////////////////////////////////////
CSkin* CCharacter::GetCurSkin( void )
{
	return m_pCurSkin;
}

////////////////////////////////////////////////////////////////////////////
int CCharacter::GetNumSkins( void )
{
	return m_SkinList.GetCount();
}

////////////////////////////////////////////////////////////////////////////
bool CCharacter::FitSkinsToSkeleton( void )
{
	POSITION pos;
	CSkin* pSkin;
	bool Success = TRUE;
	
	pos = m_SkinList.GetHeadPosition();
	while( pos )
	{
		pSkin = m_SkinList.GetNext( pos );
		Success = Success && pSkin->FitToSkeleton( *m_pSkeleton->GetSkel() );
	}

	return Success;
}

////////////////////////////////////////////////////////////////////////////
bool CCharacter::AddMotionToList( CMotion *pMotion )
{
	if( pMotion )
	{
		pMotion->m_pCharacter = this ;	
		m_MotionList.AddTail( pMotion ) ;
	}
	return true ;
}

////////////////////////////////////////////////////////////////////////////
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

		//---	fit any loaded skins to this skeleton
		FitSkinsToSkeleton();

		//---	mape the FaceDir recipe to the skeleton
		MapFaceDirRecipeToSkel();
	}
	else
	{
		delete pSkeleton ;
	}

	return Success ;
}

////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CMotion *CCharacter::NewMotion( void )
{
	CMotion *pMotion;

	pMotion = new CMotion ;
	ASSERT( pMotion ) ;

	pMotion->m_pCharacter	= this ;
	pMotion->m_PathName		= CString("") ;
	pMotion->m_ExportName	= CString("NEW_MOTION") ;

	CDlgMotion	Dialog;
	Dialog.m_pMotion			= pMotion;
	Dialog.m_bIsMotionBlended	= TRUE;	// default motion blending on

	if( Dialog.DoModal() == IDOK )
		AddMotionToList( pMotion );

	return pMotion ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharacter::MapFaceDirRecipeToSkel( void )
{
	s32 BoneID;
	s32 i;

	ASSERT(GetSkeleton());

	//---	if there is currently a recipe, try to map the current
	//		recipe to the skeleton.
	if( m_FaceDirRecipeCount != 0 )
	{
		for( i=0; i<m_FaceDirRecipeCount; i++ )
		{
			BoneID = GetSkeleton()->GetSkel()->FindBone( m_FaceDirRecipe[i].BoneName );
			if( BoneID == -1 )
			{
				x_memmove( &m_FaceDirRecipe[i], &m_FaceDirRecipe[i+1], (m_FaceDirRecipeCount-i-1)*sizeof(SBoneWeight) );
				m_FaceDirRecipeCount--;
				i--;
			}
			else
			{
				m_FaceDirRecipe[i].BoneID = BoneID;
			}
		}
	}

	//---	if the recipe length is zero, set it to the root bone
	if( m_FaceDirRecipeCount == 0 )
	{
		m_FaceDirRecipeCount = 1;
		m_FaceDirRecipe[0].BoneID = 0;
		m_FaceDirRecipe[0].Weight = 1;
		m_FaceDirRecipe[0].BoneName.Format( "%s", GetSkeleton()->GetSkel()->GetBoneName(0) );
	}
}

////////////////////////////////////////////////////////////////////////////
int CCharacter::GetNumMotions()
{
	return m_MotionList.GetCount() ;
}

////////////////////////////////////////////////////////////////////////////
CMotion *CCharacter::GetMotion( int iMotion )
{
	CMotion	*pMotion = (CMotion*)m_MotionList.IndexToPtr( iMotion ) ;
	return pMotion ;
}

////////////////////////////////////////////////////////////////////////////
CMotion *CCharacter::SetCurMotion( int iMotion )
{
	m_pCurMotion = (CMotion*)m_MotionList.IndexToPtr( iMotion ) ;
	return m_pCurMotion ;
}

////////////////////////////////////////////////////////////////////////////
CMotion *CCharacter::SetCurMotion( CMotion *pMotion )
{
	return SetCurMotion( pMotion, 0.0f, 0.0f, pMotion ? (f32)pMotion->GetNumFrames() : 0.0f );
}

////////////////////////////////////////////////////////////////////////////
CMotion *CCharacter::SetCurMotion( CMotion *pMotion, f32 BlendFrames, f32 StartFrame, f32 EndFrame, CMotion *pLastMotion, f32 LastFrame )
{
	m_pCurMotion			= pMotion ;
	m_CurMotion_BlendFrames	= BlendFrames ;
	m_CurMotion_StartFrame	= StartFrame ;
	m_CurMotion_EndFrame	= (EndFrame<0.0f) ? pMotion->GetNumFrames()-1 : EndFrame ;
	m_pLastMotion			= pLastMotion ;
	m_LastMotion_EndFrame	= (EndFrame<0.0f) ? pLastMotion->GetNumFrames()-1 : LastFrame ;

	AnimGotoStart() ;
	return m_pCurMotion ;
}

CMotion *CCharacter::GetCurMotion()
{
	return m_pCurMotion ;
}

CMotion *CCharacter::SetSecondMotion( CMotion *pMotion, f32 Ratio )
{
	m_SecondMotionRatio = Ratio;
	return m_pSecondMotion = pMotion;
}

void CCharacter::DeleteMotion( CMotion *pMotion )
{
	CMotion*	pScan		= NULL ;
	CMotion*	pPrevMotion	= NULL ;
	s32			i;

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
		else
		{
			for( i=0; i<MAX_NUM_PROPS; i++ )
			{
				if( pScan->GetProp(i).GetMotion() == pMotion )
				{
					pScan->GetProp(i).SetActive( FALSE );
					pScan->GetProp(i).SetLinkedMotion( NULL );
				}
			}
		}
	}
}

CMotion *CCharacter::FindMotionByName( const char *pPathName )
{
	CMotion *pScan = NULL ;
	CMotion	*pFound = NULL ;

	//---	look for a motion with the same path
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

CMotion *CCharacter::FindMotionByExportName( const char *pExportName )
{
	CMotion *pScan = NULL;
	CMotion *pFound = NULL;

	//---	if no motion was found, look for a motion with the same name
	POSITION Pos = m_MotionList.GetHeadPosition() ;
	while( Pos )
	{
		pScan = m_MotionList.GetNext( Pos ) ;

		if( stricmp( pScan->GetExportName(), pExportName ) == 0 )
		{
			pFound = pScan ;
			break ;
		}
	}

	return pFound;
}

CMotion *CCharacter::GetMatchingMotion( CMotion *pMotion )
{
	CMotion *pFound = NULL ;

	if( pMotion )
	{
		pFound = FindMotionByName( pMotion->GetPathName() ) ;
		if( pFound == NULL )	pFound = FindMotionByExportName( pMotion->GetExportName() );
		if( pFound == NULL )
		{
			pFound = ImportMotionFromFile( pMotion->GetPathName() ) ;
			if( pFound )
			{
				CString CurrentName = pFound->m_ExportName;
				pFound->m_ExportName.Format( "MOV_%s", CurrentName );
			}
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
	if(( !m_pCurMotion ) || ( m_pCurMotion->GetNumFrames() <= 0 ))
	{
		m_nFrame = 0 ;
	}
	else
	{
		while( nFrame >= (f32)m_pCurMotion->GetNumFrames() )
			nFrame -= (f32)m_pCurMotion->GetNumFrames() ;
		while( nFrame < 0 )
			nFrame += (f32)m_pCurMotion->GetNumFrames() ;

		m_nFrame = nFrame;
	}

	SetPose( m_ViewMotionBlended );
}

void CCharacter::SetPose( bool Blend )
{
	if(( !m_pCurMotion ) || ( m_pCurMotion->GetNumFrames() <= 0 ) )
	{
		if( m_pSkeleton )
			m_pSkeleton->SetNeutralPose() ;
		return ;
	}

	f32 nFrame = m_nFrame;
	while( nFrame >= m_pCurMotion->GetNumFrames() )
		nFrame -= m_pCurMotion->GetNumFrames() ;
	while( nFrame < 0 )
		nFrame += m_pCurMotion->GetNumFrames() ;

	f32			BlendRatio = 1.0f;
	bool		MergeMotion = FALSE;
	f32			SecondFrame;
	CMotion*	pSecondMotion;

	//---	if there is a second motion set to be blended to, use the set values
	if( m_pSecondMotion )
	{
		pSecondMotion	= m_pSecondMotion;
		SecondFrame		= m_pSecondMotion->GetNumFrames() * (f32)nFrame / (f32)m_pCurMotion->GetNumFrames();
		BlendRatio		= 1.0f - m_SecondMotionRatio;
		MergeMotion		= TRUE;
	}
	else
	{
		//---	there is no second motion, but there may be a previous motion which is getting blended out of

		pSecondMotion	= m_pLastMotion;
		SecondFrame		= m_LastMotion_EndFrame;

		//---	if the set frame is within the range of blended frames, perform the blending
		if(( nFrame >= m_CurMotion_StartFrame ) && ( nFrame < (m_CurMotion_StartFrame + m_CurMotion_BlendFrames) ))
			BlendRatio	= (nFrame - m_CurMotion_StartFrame + 1) / m_CurMotion_BlendFrames;
	}

	if( m_pSkeleton )
	{
		if( (BlendRatio != 1.0f) && pSecondMotion )
			m_pSkeleton->SetPose2MotionBlend( m_WorldBase, m_pCurMotion, (float)m_nFrame, pSecondMotion, SecondFrame, BlendRatio, MergeMotion, Blend ) ;
		else
			m_pSkeleton->SetPose( m_WorldBase, m_pCurMotion, (float)m_nFrame, Blend ) ;
	}

	return;
}

void CCharacter::SetPose( CMotion *pMotion, float nFrame, bool Blend, bool Mirror )
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
		m_pSkeleton->SetPose( m_WorldBase, pMotion, nFrame, Blend, Mirror ) ;
}

void CCharacter::SetPoseAsProp( const vector3& Pos, const radian3& Rot, CMotion* pMotion, f32 Frame, bool Blend, bool Mirror )
{
#if 0
	vector3	FramePos;
	radian3	FrameRot;
	matrix4	Mat;

	pMotion->GetTranslationBlend( FramePos, Frame );
	pMotion->GetRotationBlend( FrameRot, Frame );

	Mat.Identity();

	Mat.RotateZ( -FrameRot.Z );
	Mat.RotateY( -FrameRot.Y );
	Mat.RotateX( -FrameRot.X );

	Mat.RotateXYZ( Rot );
	Mat.Translate( Pos - FramePos );

	if( m_pSkeleton )
		m_pSkeleton->SetPose( Mat, pMotion, Frame, Blend, Mirror );
#else
	if( m_pSkeleton )
		m_pSkeleton->SetPose( Pos, Rot, pMotion, Frame, Blend, Mirror );
#endif
}

void CCharacter::AnimGotoStart()
{
	SetCurFrame( 0 ) ;
}

void CCharacter::AnimPreviousFrame()
{
	if( m_pSecondMotion && (m_pSecondMotion->GetNumFrames() != m_pCurMotion->GetNumFrames()))
	{
		f32 Inc = (f32)(m_pCurMotion->GetNumFrames()-1) / (f32)(m_pSecondMotion->GetNumFrames()-1);
		Inc = (1.0f-m_SecondMotionRatio + (Inc*m_SecondMotionRatio));
		SetCurFrame( m_nFrame-Inc );
	}
	else
		SetCurFrame( m_nFrame-1 ) ;
}

void CCharacter::AnimNextFrame()
{
	if( m_pSecondMotion && (m_pSecondMotion->GetNumFrames() != m_pCurMotion->GetNumFrames()))
	{
		f32 Inc = (f32)(m_pCurMotion->GetNumFrames()-1) / (f32)(m_pSecondMotion->GetNumFrames()-1);
		Inc = (1.0f-m_SecondMotionRatio + (Inc*m_SecondMotionRatio));
		SetCurFrame( m_nFrame+Inc );
	}
	else
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

/////////////////////////////////////////////////////////////////////////////
CPropDef* CCharacter::AllocateNewPropDef( void )
{
	s32 i;
	for( i=0; i<MAX_NUM_PROPS; i++ )
		if( !GetPropDef(i).IsActive() )
			return &GetPropDef(i);
	
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CPropDef* CCharacter::FindPropByCharacter( CCharacter* pFindCharacter )
{
	s32 i;
	for( i=0; i<MAX_NUM_PROPS; i++ )
		if( GetPropDef(i).GetLinkedCharacter() && (GetPropDef(i).GetLinkedCharacter() == pFindCharacter) )
			return &GetPropDef(i);
	
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void CCharacter::EditProperties( void )
{

}

// Motion.cpp : implementation file
//

#include "stdafx.h"

#include "Motion.h"
#include "ieArchive.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	MOTION_AMC_VERSION	1

#include "CeD.h"
#include "Skeleton.h"
#include "Character.h"
#include "mfcutils.h"
#include "CeDDoc.h"
#include "MotionFlags.h"
#include "DlgMotion.h"
#include "DlgFilePaths.h" // for search and replace methods enumeration

/////////////////////////////////////////////////////////////////////////////
// CMotion Implementation

IMPLEMENT_DYNCREATE(CMotion, CObject)

/////////////////////////////////////////////////////////////////////////////
// CMotion Constructors

CMotion::CMotion()
{
	//---	Reset Data Members
	Initialize () ;
}

CMotion::~CMotion()
{
	//---	Destroy the Texture
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CMotion Implementation

void CMotion::Initialize()
{
	ANIM_InitInstance( &m_Anim ) ;
	m_pCharacter = NULL ;
	m_IsLoaded = false ;
	m_LoadFailed = false ;
	m_IsANM = false ;
	m_nFrames = 0 ;
	m_pFaceDir = NULL ;
	m_pMoveDir = NULL ;
	m_pCurEvent = NULL ;
	m_pLinkCharacter = NULL ;
	m_pLinkMotion = NULL ;
	m_FixupLinkStuff = false ;
	m_bAutoResetMoveDir = TRUE;
	m_bAutoResetFaceDir = TRUE;
	m_bExport = TRUE;
	m_ChangedExport = FALSE;
	m_bExportFrameRate = FALSE;
	m_ExportFrameRate = 30.0f;
	m_bExportBits = FALSE;
	m_ExportBits = 12;
	m_pCompVersion = NULL;
	m_bExportLink = TRUE;
	for( int i = 0 ; i < MOTION_FLAG_BITS ; i++ ) m_FlagBit[i] = false ;
}

void CMotion::Destroy()
{
	//---	Initialize Data Members
	if( m_pFaceDir )
	{
		free( m_pFaceDir ) ;
		m_pFaceDir = NULL ;
	}
	if( m_pMoveDir )
	{
		free( m_pMoveDir ) ;
		m_pMoveDir = NULL ;
	}

	if( m_pCompVersion )
		delete m_pCompVersion;

	ANIM_KillInstance( &m_Anim ) ;
	Initialize () ;
}

void CMotion::Copy (CMotion &Dst, CMotion &Src)
{
	ANIM_CopyAnim( &Dst.m_Anim, &Src.m_Anim );

	Dst.m_pCharacter					= Src.m_pCharacter;
	Dst.m_PathName						= Src.m_PathName;
	Dst.m_ExportName					= Src.m_ExportName;
	Dst.m_IsLoaded						= Src.m_IsLoaded;
	Dst.m_LoadFailed					= Src.m_LoadFailed;
	Dst.m_IsANM							= Src.m_IsANM;
	Dst.m_nFrames						= Src.m_nFrames;
	Dst.m_pFaceDir						= Src.m_pFaceDir;
	Dst.m_pMoveDir						= Src.m_pMoveDir;
	Dst.m_EventList						= Src.m_EventList;
	Dst.m_pCurEvent						= Src.m_pCurEvent;
	Dst.m_pLinkCharacter				= Src.m_pLinkCharacter;
	Dst.m_pLinkMotion					= Src.m_pLinkMotion;
	Dst.m_FixupLinkStuff				= Src.m_FixupLinkStuff;
	Dst.m_bExport						= Src.m_bExport;
	Dst.m_bExportFrameRate				= Src.m_bExportFrameRate;
	Dst.m_ExportFrameRate				= Src.m_ExportFrameRate;
	Dst.m_bExportBits					= Src.m_bExportBits;
	Dst.m_ExportBits					= Src.m_ExportBits;
	Dst.m_bExportLink					= Src.m_bExportLink;

	//---	get the number of frames
	s32 nFrames = Src.GetNumFrames() ;

	//---	copy the face directions
	if (Src.m_pFaceDir)
	{
		Dst.m_pFaceDir = (f32*)malloc( nFrames * sizeof(f32) ) ;
		ASSERT( Dst.m_pFaceDir ) ;
		x_memcpy( Dst.m_pFaceDir, Src.m_pFaceDir, nFrames*sizeof( f32 ) ) ;
	}
	else
		Dst.m_pFaceDir = NULL;

	//---	copy the move directions
	if (Src.m_pMoveDir)
	{
		Dst.m_pMoveDir = (f32*)malloc( nFrames * sizeof(f32) ) ;
		ASSERT( Dst.m_pMoveDir ) ;
		x_memcpy( Dst.m_pMoveDir, Src.m_pMoveDir, nFrames*sizeof( f32 ) ) ;
	}
	else
		Dst.m_pMoveDir = NULL;

	//---	copy the flag bits
	x_memcpy( &Dst.m_FlagBit[0], &Src.m_FlagBit[0], MOTION_FLAG_BITS*sizeof(bool) ) ;

	Dst.m_SkelPointChecks.SetSize( Src.m_SkelPointChecks.GetSize() );
	for( int i=0; i<Src.m_SkelPointChecks.GetSize(); i++ )
		Dst.m_SkelPointChecks[i] = Src.m_SkelPointChecks[i];
}

bool CMotion::Modify( void )
{
	CDlgMotion	Dialog;
	bool		ReloadFile = FALSE;
	bool		bExport = m_bExport;

	Dialog.m_Motion = *this;
	if( Dialog.DoModal() == IDOK )
	{
		//---	determine if the file changed
		if( GetPathName() != Dialog.m_Motion.GetPathName() )
			ReloadFile = TRUE;

		//---	set the new data
		*this = Dialog.m_Motion;
		m_ChangedExport = m_bExport != bExport ? TRUE : FALSE;

		//---	if the file has changed, reload the file
		if( ReloadFile )
			ReloadFromFile();

		return TRUE;
	}

	return FALSE;
}

static radian ComputeRootDirFromXYZ (radian RX,radian RY,radian RZ)
{
    f32 sx,sy,sz;
    f32 cx,cy,cz;
    f32 ZAxisX,ZAxisZ;
    f32 YAxisX,YAxisZ;
    f32 ZAxisL,YAxisL;
    radian ZAxisDir,YAxisDir;
    radian FaceDir;

    ///////////////////////////////////////////////////////////////////////
    //
    //  STRAIGHT FROM AM_PLAY
    //
    // Normal steps:
    // 1) Create RXYZ matrix
    // 2) Transform Z-axis (0,0,1) using matrix to get vector in anim space
    // 3) Find YAW value of horiz (XZ) component of vector.
    //
    // Quick steps:
    // Since column2 is the z-axis in the dest space of the matrix, we don't 
    // need to transform the z vector.  Since we only use the X,Z components
    // of the dest vector to compute the yaw we only need to compute the X,Z
    // components of column2. These are matrix components X=[2][0], Z=[2][2].
    // If the horiz components are too small, use the y-axis the same way. 
    // That would be X=[1][0], Z=[1][2].
    ///////////////////////////////////////////////////////////////////////

    // Get sin, cos of XYZ angles
    x_sincos(RX,&sx,&cx);
    x_sincos(RY,&sy,&cy);
    x_sincos(RZ,&sz,&cz);

    // Compute XZ components of Z-axis in dest space
    ZAxisX   = (cx*cz*sy) + (sx*sz);  // M[2][0]
    ZAxisZ   = (cx*cy);               // M[2][2]
    ZAxisDir = x_atan2(ZAxisX,ZAxisZ);
    ZAxisL   = x_sqrt(ZAxisX*ZAxisX + ZAxisZ*ZAxisZ);

    // Compute XZ components of Y-axis in dest space
    YAxisX   = (sx*sy*cz) - (cx*sz);  // M[1][0]
    YAxisZ   = (sx*cy);               // M[1][2]
    YAxisDir = x_atan2(YAxisX,YAxisZ);
    YAxisL   = x_sqrt(YAxisX*YAxisX + YAxisZ*YAxisZ);

    // Bring angles close to each other
    while ((YAxisDir - ZAxisDir) < -R_180) YAxisDir += R_360;
    while ((YAxisDir - ZAxisDir) >  R_180) YAxisDir -= R_360;

    // Compute a weighted average using axis length as the weight
    ZAxisL *= ZAxisL;
    YAxisL *= YAxisL;
    FaceDir  = ZAxisDir * (ZAxisL / (ZAxisL+YAxisL));
    FaceDir += YAxisDir * (YAxisL / (ZAxisL+YAxisL));
    return FaceDir;
}


///////////////////////////////////////////////////////////////////////////////
bool CMotion::CreateFromAMC( const char *pFileName, bool SupressWarningBoxes, long* pResult )
{
	m_PathName = pFileName ;

	skel *pSkel = m_pCharacter->GetSkeleton()->GetSkel();
	if( pResult ) *pResult = 0;

	//---	attempt to load the animation
	err Error = ANIM_ImportAMC( &m_Anim, pFileName, pSkel, 0, MOTION_AMC_VERSION ) ;

	//---	if the animation did not load propertly, inform the user and get out
	if( Error != ERR_SUCCESS )
    {
		if( !SupressWarningBoxes )
		{
			CString String ;
			String.Format( "Unable to load motion '%s'", pFileName ) ;
			::MessageBox( NULL, String, "ERROR LOADING ANIMATION", MB_ICONWARNING ) ;
		}
		if( pResult ) *pResult |= LOAD_RESULT_FRAMESCHANGED;
		m_LoadFailed = TRUE ;
		return FALSE;
    }

	//---	complete the process
	return CreateFromAnim( SupressWarningBoxes, pResult );
}



///////////////////////////////////////////////////////////////////////////////
bool CMotion::CreateFromAnim( bool SupressWarningBoxes, long* pResult )
{
	skel *pSkel = m_pCharacter->GetSkeleton()->GetSkel();
	if( pResult ) *pResult = 0;

    //---   Check for single frame anim
    if (m_Anim.NFrames==1)
    {
		if( !SupressWarningBoxes )
		{
			CString String ;
			String.Format( "Motion '%s' has only one frame.", m_PathName ) ;
			::MessageBox( NULL, String, "Single Frame Motion", MB_ICONWARNING ) ;
		}
		if( pResult ) *pResult |= LOAD_RESULT_ONEFRAME;
//CJG-FIX-PATCH-HACK            ANIM_DuplicateLastFrame(&m_Anim);
    }

	//---	Set Loaded Flags
	m_IsLoaded = TRUE ;

	//---	Set LoadFailed Flag
	m_LoadFailed = FALSE ;

	//---	Create and Initialize FaceDir & MoveDir Arrays from the loaded data
	s32 nFrames = GetNumFrames() ;
	f32	*pFaceDir = (f32*)malloc( nFrames * sizeof(f32) ) ;
	ASSERT( pFaceDir ) ;
	f32	*pMoveDir = (f32*)malloc( nFrames * sizeof(f32) ) ;
	ASSERT( pMoveDir ) ;

	for( int i = 0 ; i < nFrames ; i++ )
	{
		//---	Generate FaceDir
		radian	Rx, Ry, Rz ;
        Rx = ANIM_GetRotation( GetAnim(), i, 0 );
        Ry = ANIM_GetRotation( GetAnim(), i, 1 );
        Rz = ANIM_GetRotation( GetAnim(), i, 2 );
        pFaceDir[i] = ComputeRootDirFromXYZ( Rx, Ry, Rz ) ;

		//---	Generate MoveDir
		vector3d	TS, TE ;
		ANIM_GetTransValue( GetAnim(), i, &TS ) ;

		if( i >= (nFrames-1) )
		{
			vector3d	Diff ;
			ANIM_GetTransValue( GetAnim(), i-1, &TE ) ;
			V3_Sub( &Diff, &TS, &TE ) ;
			V3_Add( &TE, &TE, &Diff ) ;
			V3_Add( &TE, &TE, &Diff ) ;
		}
		else
			ANIM_GetTransValue( GetAnim(), i+1, &TE ) ;

		V3_Sub( &TE, &TE, &TS ) ;
		pMoveDir[i] = x_atan2( TE.X, TE.Z ) ;
	}

	//---	If Motion already Has a FaceDir Array then merge and truncate
	if( m_pFaceDir  )
	{
		//---	Copy Existing Editted Buffer to New Buffer and Set New Buffer
		x_memcpy( pFaceDir, m_pFaceDir, min( m_nFrames, nFrames ) * sizeof( f32 ) );
		free( m_pFaceDir ) ;
		m_pFaceDir = pFaceDir ;
	}
	else
		m_pFaceDir = pFaceDir ;

	
	//---	If Motion already Has a MoveDir Array then merge and truncate
	if( m_pMoveDir )
	{
		//---	Copy Existing Editted Buffer to New Buffer and Set New Buffer
		x_memcpy( pMoveDir, m_pMoveDir, min( m_nFrames, nFrames ) * sizeof( f32 ) );
		free( m_pMoveDir ) ;
		m_pMoveDir = pMoveDir ;
	}
	else
		m_pMoveDir = pMoveDir ;

	//---	Warn if Number of Frames have changed
	if( (m_nFrames != 0) && (m_nFrames != nFrames))
	{
		if( !SupressWarningBoxes )
		{
			CString String ;
			String.Format( "Motion '%s' has changed number of frames since last loaded. From %d to %d", GetPathName(), m_nFrames, nFrames ) ;
			::MessageBox( NULL, String, "Motion Resized Warning", MB_ICONWARNING ) ;
		}
		if( pResult ) *pResult = LOAD_RESULT_FRAMESCHANGED;
	}

    //---   Warn if events are out of animation range
	for (i=0; i<GetNumEvents(); i++)
	{
		CEvent *pEvent = GetEvent(i) ;
		if (pEvent->GetFrame() >= GetNumFrames())
		{
			if( !SupressWarningBoxes )
			{
				CString String ;
				String.Format( "Motion '%s' has a event(s) past the end of the animation", GetPathName() ) ;
				::MessageBox( NULL, String, "Event Past Animation", MB_ICONWARNING ) ;
				//pEvent->SetFrame( GetNumFrames()-1 );
			}
			if( pResult ) *pResult = LOAD_RESULT_EVENTOUTSIDEANIM;
			break;
		}
	}
    
	//---	Set Number of Frames in each Array
	m_nFrames = nFrames ;

	if( m_bAutoResetFaceDir )
		ResetAllFaceDir();

	if( m_bAutoResetMoveDir )
		ResetAllMoveDir();

	return m_IsLoaded ;
}

bool CMotion::EnsureLoaded( )
{
	if( !m_IsLoaded && !m_LoadFailed )
	{
		CreateFromFile( m_PathName ) ;
	}

	return m_IsLoaded ;
}

bool CMotion::CreateFromFile( const char *pFileName, bool SupressWarningBoxes, long* pResult )
{
	char	Ext[_MAX_EXT] ;

	_splitpath( pFileName, NULL, NULL, NULL, Ext ) ;
	if( stricmp( Ext, ".AMC" ) == 0 )
	{
		CreateFromAMC( pFileName, SupressWarningBoxes, pResult ) ;
	}

	return m_IsLoaded ;
}

bool CMotion::ReloadFromFile( bool SupressWarningBoxes, long* pResult )
{
	//---	if it is currently loaded, remove the current load
	if( m_IsLoaded )
	{
		ANIM_KillInstance( &m_Anim );
		m_IsLoaded = FALSE;
	}

	return CreateFromFile( GetPathName(), SupressWarningBoxes, pResult );
}

const char *CMotion::GetPathName()
{
	return (const char*)m_PathName ;
}

const char *CMotion::GetExportName()
{
	return (const char*)m_ExportName ;
}

bool CMotion::SearchReplacePathName( CString& rSearch, CString& rReplace, s32 Method )
{
	//since the incoming search and replace strings have already been forced to 
	//uppercase, it would be nice if the pathname would as well...
	CString UpperCasePathName	= m_PathName;

	UpperCasePathName.MakeUpper();

	int Loc = UpperCasePathName.Find((LPCTSTR) rSearch );

	//---	if there are conditions on the search, return if they don't apply
	if ((Loc == -1 ) ||
		(( Method == FILEPATHS_LOOKFROMSTART ) && (Loc != 0)) ||
		(( Method == FILEPATHS_LOOKFROMEND ) && (Loc != (m_PathName.GetLength() - rSearch.GetLength()))))
		return FALSE;

	//---	replace the search string in the file path
	CString FirstPart = m_PathName.Left( Loc );
	CString LastPart = m_PathName.Right( m_PathName.GetLength() - (Loc + rSearch.GetLength()) );

	m_PathName = FirstPart + rReplace + LastPart;
	return TRUE;
}

int CMotion::GetNumFrames()
{
	if( EnsureLoaded() )
		return m_Anim.NFrames ;

	return 0 ;
}

anim *CMotion::GetAnim()
{
	if( EnsureLoaded() )
		return &m_Anim ;

	return NULL ;
}

int CMotion::GetStartFrame()
{
	if( EnsureLoaded() )
		return m_Anim.StartFrame ;

	return NULL ;
}

radian CMotion::GetMoveDir( int nFrame )
{
	if( !EnsureLoaded() ) return 0.0f;
	ASSERT( m_pMoveDir ) ;

	return m_pMoveDir[nFrame] ;
}

void CMotion::SetMoveDir( int nFrame, radian MoveDir )
{
	if( !EnsureLoaded() ) return;
	ASSERT( m_pMoveDir ) ;

	while( MoveDir > DEG_TO_RAD( 180) ) MoveDir -= DEG_TO_RAD(360) ;
	while( MoveDir < DEG_TO_RAD(-180) ) MoveDir += DEG_TO_RAD(360) ;
	
	m_pMoveDir[nFrame] = MoveDir ;
}

void CMotion::SetAllMoveDir( radian MoveDir )
{
	if( !EnsureLoaded() ) return;
	ASSERT( m_pMoveDir ) ;

	while( MoveDir > DEG_TO_RAD( 180) ) MoveDir -= DEG_TO_RAD(360) ;
	while( MoveDir < DEG_TO_RAD(-180) ) MoveDir += DEG_TO_RAD(360) ;
	
	for( int i = 0 ; i < m_nFrames ; i++ )
		m_pMoveDir[i] = MoveDir ;
}

radian CMotion::GetFaceDir( int nFrame )
{
	if( !EnsureLoaded() ) return 0.0f;
	ASSERT( m_pFaceDir ) ;

	return m_pFaceDir[nFrame] ;
}

void CMotion::SetFaceDir( int nFrame, radian FaceDir )
{
	if( !EnsureLoaded() ) return;
	ASSERT( m_pFaceDir ) ;

	while( FaceDir > DEG_TO_RAD( 180) ) FaceDir -= DEG_TO_RAD(360) ;
	while( FaceDir < DEG_TO_RAD(-180) ) FaceDir += DEG_TO_RAD(360) ;
	
	m_pFaceDir[nFrame] = FaceDir ;
}

void CMotion::SetAllFaceDir( radian FaceDir )
{
	if( !EnsureLoaded() ) return;
	ASSERT( m_pFaceDir ) ;

	while( FaceDir > DEG_TO_RAD( 180) ) FaceDir -= DEG_TO_RAD(360) ;
	while( FaceDir < DEG_TO_RAD(-180) ) FaceDir += DEG_TO_RAD(360) ;
	
	for( int i = 0 ; i < m_nFrames ; i++ )
		m_pFaceDir[i] = FaceDir ;
}

void CMotion::ResetAllFaceDir( void )
{
	if( !EnsureLoaded() ) return;

	//---	Generate FaceDir
	for( int i = 0 ; i < m_nFrames ; i++ )
    {
	    radian	Rx, Ry, Rz ;
        Rx = ANIM_GetRotation( GetAnim(), i, 0 );
        Ry = ANIM_GetRotation( GetAnim(), i, 1 );
        Rz = ANIM_GetRotation( GetAnim(), i, 2 );
        m_pFaceDir[i] = ComputeRootDirFromXYZ( Rx, Ry, Rz ) ;
    }
}

void CMotion::ResetAllMoveDir( void )
{
	if( !EnsureLoaded() ) return;

	for( int i = 0 ; i < m_nFrames ; i++ )
	{
		//---	Generate MoveDir
		vector3d	TS, TE ;
		ANIM_GetTransValue( GetAnim(), i, &TS ) ;
		if( i < (m_nFrames-1) )
		{
			ANIM_GetTransValue( GetAnim(), i+1, &TE ) ;
		}
		else
		{
			vector3d	Diff ;
			ANIM_GetTransValue( GetAnim(), i-1, &TE ) ;
			V3_Sub( &Diff, &TS, &TE ) ;
			V3_Add( &TE, &TE, &Diff ) ;
			V3_Add( &TE, &TE, &Diff ) ;
		}
		V3_Sub( &TE, &TE, &TS ) ;
		m_pMoveDir[i] = x_atan2( TE.X, TE.Z ) ;
	}
}

void CMotion::SetRemainingMoveDir( int nFrame, radian MoveDir)
{
	if( !EnsureLoaded() ) return;
	ASSERT( m_pMoveDir ) ;

	while( MoveDir > DEG_TO_RAD( 180) ) MoveDir -= DEG_TO_RAD(360) ;
	while( MoveDir < DEG_TO_RAD(-180) ) MoveDir += DEG_TO_RAD(360) ;
	
	for( int i = nFrame; i < m_nFrames ; i++ )
		m_pMoveDir[i] = MoveDir ;
}

void CMotion::SetRemainingFaceDir( int nFrame, radian FaceDir)
{
	if( !EnsureLoaded() ) return;
	ASSERT( m_pFaceDir ) ;

	while( FaceDir > DEG_TO_RAD( 180) ) FaceDir -= DEG_TO_RAD(360) ;
	while( FaceDir < DEG_TO_RAD(-180) ) FaceDir += DEG_TO_RAD(360) ;
	
	for( int i = nFrame ; i < m_nFrames ; i++ )
		m_pFaceDir[i] = FaceDir ;
}


void CMotion::GetTranslation( vector3d *pVector, int nFrame )
{
	ASSERT( pVector ) ;

	anim *pAnim = GetAnim() ;
	if( pAnim )
		ANIM_GetTransValue( GetAnim(), nFrame, pVector ) ;
	else
		V3_Zero( pVector ) ;
}

int CMotion::GetNumEvents( )
{
	return m_EventList.GetCount() ;
}

CEvent *CMotion::GetEvent( int iEvent )
{
	return (CEvent*)m_EventList.IndexToPtr( iEvent ) ;
}

void CMotion::SetCurEvent( CEvent *pEvent )
{
	m_pCurEvent = pEvent ;
}

CEvent *CMotion::GetCurEvent( )
{
	return m_pCurEvent ;
}

void CMotion::DeleteEvent( CEvent *pEvent )
{
	CEvent *pScan = NULL ;
	CEvent *pPrevEvent = NULL ;

	POSITION Pos = m_EventList.GetHeadPosition() ;
	while( Pos )
	{
		POSITION Pos2 = Pos ;
		pPrevEvent = pScan ;
		pScan = m_EventList.GetNext( Pos ) ;

		if( pScan == pEvent )
		{
			m_EventList.RemoveAt( Pos2 ) ;
			delete pEvent ;
			if( GetCurEvent() == pEvent )
				SetCurEvent( NULL ) ;
			break ;
		}
	}
}

CEvent *CMotion::AddEvent( )
{
	CEvent *pEvent = new CEvent() ;
	ASSERT( pEvent ) ;
	pEvent->SetMotion( this ) ;
	m_EventList.AddTail( pEvent ) ;
	SetCurEvent( pEvent ) ;

	return pEvent ;
}

BOOL CMotion::SkelPointGetCheck( int Index )
{
	BOOL	State = FALSE ;
	if( Index < m_SkelPointChecks.GetSize() )
		State = m_SkelPointChecks.GetAt( Index ) ;
	return State ;
}

void CMotion::SkelPointSetCheck( int Index, BOOL State )
{
	m_SkelPointChecks.SetAtGrow( Index, State ? 1 : 0 ) ;
}

void CMotion::SkelPointDeleteCheck( int Index )
{
	if( Index < m_SkelPointChecks.GetSize() )
		m_SkelPointChecks.RemoveAt( Index ) ;
}

/////////////////////////////////////////////////////////////////////////////
// CMotion Overridden operators

CMotion &CMotion::operator=( CMotion &ptMotion )
{
	Copy (*this, ptMotion) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CMotion serialization

void CMotion::Serialize( CieArchive& a, CCharacter *pCharacter )
{
	int		i ;

	CCeDDoc *pDoc = (CCeDDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_MOTION) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_MOTION_PATHNAME, m_PathName ) ;
		a.WriteTaggedString( IE_TAG_MOTION_EXPORTNAME, m_ExportName ) ;
		a.WriteTaggedbool( IE_TAG_MOTION_ISANM, m_IsANM ) ;
		if( m_pFaceDir )
		{
			a.WriteTaggeds32( IE_TAG_MOTION_FACEDIR, m_nFrames ) ;
			for( i = 0 ; i < m_nFrames ; i++ )
			{
				a.Writef32(m_pFaceDir[i]) ;
			}
		}
		if( m_pMoveDir )
		{
			a.WriteTaggeds32( IE_TAG_MOTION_MOVEDIR, m_nFrames ) ;
			for( i = 0 ; i < m_nFrames ; i++ )
			{
				a.Writef32(m_pMoveDir[i]) ;
			}
		}
		m_EventList.Serialize( a ) ;

		//---	Get Index for LinkCharacter and LinkMotion and Write those
		if( m_pLinkCharacter )
		{
			s32 Index = pDoc->m_CharacterList.PtrToIndex( m_pLinkCharacter ) ;
			if( Index != -1 )
				a.WriteTaggeds32( IE_TAG_MOTION_LINKCHARACTER, Index ) ;
		}
		if( m_pLinkMotion )
		{
			s32 Index = m_pLinkCharacter->m_MotionList.PtrToIndex( m_pLinkMotion ) ;
			if( Index != -1 )
				a.WriteTaggeds32( IE_TAG_MOTION_LINKMOTION, Index ) ;
		}

		//---	Write Flag Bits
		for( int i = 0 ; i < MOTION_FLAG_BITS ; i++ )
		{
			a.WriteTaggedbool( IE_TAG_MOTION_FLAGBIT, m_FlagBit[i] ) ; 
		}

		//---	Write SkelPoint Check States
		for( i = 0 ; i < m_SkelPointChecks.GetSize() ; i++ )
		{
			a.WriteTaggedbool( IE_TAG_MOTION_SKELPOINTCHECK, !!SkelPointGetCheck(i) ) ;
		}

		//--- Write Auto Reset Face Direction toggle
		a.WriteTaggedbool( IE_TAG_MOTION_AUTORESET_FACEDIR, (bool)m_bAutoResetFaceDir );

		//--- Write Auto Reset Move Direction toggle
		a.WriteTaggedbool( IE_TAG_MOTION_AUTORESET_MOVEDIR, (bool)m_bAutoResetMoveDir );

		//--- Export
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT, (bool)m_bExport );
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_LINK, (bool)m_bExport );

		//---	Fractional frames information
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_FRAMERATE_ON, m_bExportFrameRate );
		a.WriteTaggedf32( IE_TAG_MOTION_EXPORT_FRAMERATE, m_ExportFrameRate );

		//---	export bits
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_BITS_ON, m_bExportBits );
		a.WriteTaggeds32( IE_TAG_MOTION_EXPORT_BITS, m_ExportBits );

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_MOTION_END ) ;
	}
	else
	{
		//---	Load Motion
		s32		iFlagBit = 0 ;
		s32		SkelPointIndex = 0 ;
		BOOL done = FALSE ;
		while (!done)
		{
			s32		Index ;
			bool	tbool ;
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_MOTION_PATHNAME:
				a.ReadString(m_PathName) ;
				m_ExportName = MFCU_GetFName( m_PathName ) ;
				break ;
			case IE_TAG_MOTION_EXPORTNAME:
				a.ReadString(m_ExportName) ;
				break ;

			case IE_TAG_MOTION_ISANM:
				a.Readbool(m_IsANM) ;
				break ;

			case IE_TAG_MOTION_FACEDIR:
				a.Reads32(m_nFrames) ;
				m_pFaceDir = (f32*)malloc( m_nFrames * sizeof(f32) ) ;
				ASSERT( m_pFaceDir ) ;
				a.m_rType = IE_TYPE_f32 ;
				a.m_rLength = 4 ;
				for( i = 0 ; i < m_nFrames ; i++ )
				{
					a.Readf32(m_pFaceDir[i]) ;
				}
				break ;

			case IE_TAG_MOTION_MOVEDIR:
				a.Reads32(m_nFrames) ;
				m_pMoveDir = (f32*)malloc( m_nFrames * sizeof(f32) ) ;
				ASSERT( m_pMoveDir ) ;
				a.m_rType = IE_TYPE_f32 ;
				a.m_rLength = 4 ;
				for( i = 0 ; i < m_nFrames ; i++ )
				{
					a.Readf32(m_pMoveDir[i]) ;
				}
				break ;

			case IE_TAG_EVENTLIST:
				m_EventList.Serialize( a ) ;
				//---	Set Motion pointers in each event
				{
					POSITION Pos = m_EventList.GetHeadPosition() ;
					while( Pos )
					{
						CEvent *pEvent = m_EventList.GetNext( Pos ) ;
						if( pEvent )
							pEvent->SetMotion( this ) ;
					}
				}
				break ;

			case IE_TAG_MOTION_LINKCHARACTER:
				a.Reads32( Index ) ;
				m_pLinkCharacter = (CCharacter*)Index ;
				m_FixupLinkStuff = true ;
				break ;

			case IE_TAG_MOTION_LINKMOTION:
				a.Reads32( Index ) ;
				m_pLinkMotion = (CMotion*)Index ;
				m_FixupLinkStuff = true ;
				break ;

			case IE_TAG_MOTION_FLAGBIT:
				a.Readbool( m_FlagBit[iFlagBit++] ) ;
				break ;

			case IE_TAG_MOTION_SKELPOINTCHECK:
				a.Readbool( tbool ) ;
				SkelPointSetCheck( SkelPointIndex++, tbool ) ;
				break ;

			case IE_TAG_MOTION_AUTORESET_FACEDIR :
				a.Readbool( m_bAutoResetFaceDir );
				break;

			case IE_TAG_MOTION_AUTORESET_MOVEDIR :
				a.Readbool( m_bAutoResetMoveDir );
				break;

			case IE_TAG_MOTION_EXPORT:
				a.Readbool( m_bExport );
				break;

			case IE_TAG_MOTION_EXPORT_LINK:
				a.Readbool( m_bExportLink );
				break;

			case IE_TAG_MOTION_EXPORT_FRAMERATE_ON:
				a.Readbool( m_bExportFrameRate );
				break;

			case IE_TAG_MOTION_EXPORT_FRAMERATE:
				a.Readf32( m_ExportFrameRate );
				break;

			case IE_TAG_MOTION_EXPORT_BITS_ON:
				a.Readbool( m_bExportBits );
				break;

			case IE_TAG_MOTION_EXPORT_BITS:
				a.Reads32( m_ExportBits );
				break;

			case IE_TAG_MOTION_END:
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
// CMotion diagnostics

#ifdef _DEBUG
void CMotion::AssertValid() const
{
	CObject::AssertValid();
}

void CMotion::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

void CMotion::SetAutoResetFaceDir(bool bReset)
{
	m_bAutoResetFaceDir = bReset;
}

void CMotion::SetAutoResetMoveDir(bool bReset)
{
	m_bAutoResetMoveDir = bReset;
}

bool CMotion::GetAutoResetFaceDir()
{
	return m_bAutoResetFaceDir;
}

bool CMotion::GetAutoResetMoveDir()
{
	return m_bAutoResetMoveDir;
}

f32 CMotion::GetExportFrameRate( void )
{
	f32 ret;

	//---	get the export frame rate
	if( m_bExportFrameRate )
		ret = m_ExportFrameRate;
	else if ( m_pCharacter )
		ret = m_pCharacter->m_ExportFrameRate;
	else
		ret = 60.0f;

	//---	the exported frame rate shouldn't be greater than 60 since the source motion data is given at 60
	if( ret > 60.0f )
		ret = 60.0f;
	else if( ret < 0.0f )
		ret = 0.0f;

	//---	truncate the return value to the exported value ( it must fit in a 6.2 fixed number)
	ret = ((f32)((s32) ((ret*1024.0f)+0.5f)))/1024.0f;

	return ret;
}

s32 CMotion::GetExportBits( void )
{
	s32 ret;

	//---	get the export frame rate
	if( m_bExportBits )
		ret = m_ExportBits;
	else if ( m_pCharacter )
		ret = m_pCharacter->m_ExportBits;
	else
		ret = 12;

	//---	the exported frame rate shouldn't be greater than 60 since the source motion data is given at 60
	if( ret > 16 )
		ret = 16;
	else if( ret < 0 )
		ret = 0;

	return ret;
}



//////////////////////////////////////////////////////////////////////
f32 CMotion::GetResampledAnimation( anim& rResampAnim, f32 FrameRate )
{
    s16*		RawData;
	s32			RawDataSize;
	s32			ResampleFrames;
	s16			Value ;
	s16			NextValue;
	s32			WholeFrame;
	f32			FracFrame;
	f32			nFrame;
	s32			ResampFrame;
	s32			LastFrame;
	s32			dDir;
	s32			NBoneStreams = GetAnim()->NBones*3;
	
	//---	determine the number of resampled frames
	ResampleFrames = (s32)((FrameRate*GetAnim()->NFrames/60.0f)+0.5f);
	if( ResampleFrames < 1 )
		ResampleFrames = 1;

	//---	get a buffer for resampling
	ANIM_InitInstance( &rResampAnim );
	ANIM_CopyAnimHeader( &rResampAnim, GetAnim() );
	rResampAnim.NFrames		= ResampleFrames;
	rResampAnim.Stream		= (s16*) x_malloc( ResampleFrames*rResampAnim.NStreams*sizeof(s16) );

    /////////////////////////////////////////////////////////////
	//---	Loop through all Streams and load into RawData
    /////////////////////////////////////////////////////////////
	//---	get the exported frame rate, round it off the same way that it is exported below
	LastFrame		= GetAnim()->NFrames-1;
	f32 FrameStep	= (f32)(GetAnim()->NFrames-1)/(f32)(ResampleFrames-1);
	RawData = rResampAnim.Stream;
	RawDataSize = 0;
	for( int nStream = 0 ; nStream < GetAnim()->NStreams ; ++nStream )
	{
		//---	Loop through All Frames in this Block and load into RawData
		nFrame = 0.0f;
		for( ResampFrame = 0 ; ResampFrame < ResampleFrames ; ++ResampFrame, nFrame += FrameStep )
		{
			WholeFrame = (s32)nFrame;
			FracFrame = nFrame - (f32)WholeFrame;

			if( WholeFrame >= LastFrame )
			{
				WholeFrame = LastFrame-1;
				FracFrame = 1.0f;
			}

			//---	interpolate this frame
			ANIM_GetStreamValue( GetAnim(), WholeFrame, nStream, &Value );
			ANIM_GetStreamValue( GetAnim(), WholeFrame+1, nStream, &NextValue );
//			Value = Value + (s16)((f32)(NextValue-Value)*FracFrame+0.5f);

			if( nStream < NBoneStreams )
			{
				dDir = NextValue - Value;
				while( dDir >  ANGLE_180 )	dDir -= ANGLE_360;
				while( dDir <= -ANGLE_180 )	dDir += ANGLE_360;
				Value = Value + (s16)((f32)dDir*FracFrame+0.5f);
				Value &= ANGLE_360-1;
			}
			else
				Value = Value + (s16)((f32)(NextValue-Value)*FracFrame+0.5f);


			RawData[RawDataSize++] = Value;
		}
	}

	//---	return the resample rate used
	//		NOTE: The resample rate is not exactly the rate requested.
	//		To keep the first and last frame the rate had to be adjusted.
	return 60.0f*ResampleFrames/GetAnim()->NFrames;
}

//////////////////////////////////////////////////////////////////////
//#define FRAMES_HALVED
#define FRAMES_FRACTIONAL
//#define	ACCURATE_COMPRESSION
#include "acccmp.h"
void CMotion::MakeIntoCompressedVersion( void )
{
#if 0
    s32		i,j;
    anim	Anim;
    anim*	pAnim;
    skel*	pSkel;
	bool	RotationsConvertedToXZY=FALSE;
	s32		TotalAnimRawDataSize=0;

    //---   Get the resampled animation for processing
    GetResampledAnimation( Anim, GetExportFrameRate() );
    pAnim = &Anim;

    //---   Get duplicate of animation for processing
    CSkeleton *pSkeleton = m_pCharacter->GetSkeleton() ;
    pSkel = pSkeleton->GetSkel();

    matrix4 DestM[50];
    matrix4 WorldOrientM;
	f32		ExportAngleConversion = ((f32)(1<<GetExportBits()))/R_360;
	f32		OneOverExportAngleConversion;
	if( ExportAngleConversion == 0.0f )
		OneOverExportAngleConversion = 0.0f;
	else
		OneOverExportAngleConversion = 1.0f/ExportAngleConversion;

	ASSERT(pAnim->NBones<=50);

    //---   Build WorldOrientM
    M4_Identity( &WorldOrientM );

    //---   Build matrices and compute global XZY rotations
    for (i=0; i<pAnim->NFrames; i++)
    {
        // Compute local to world matrix
        SKEL_BuildMatrices (pSkel, pAnim, i, DestM, &WorldOrientM);

        // Pull global XYZ rotations from matrix and convert to global XZY
        for (j=0; j<pAnim->NBones; j++)
        {
            radian3d Rot;
            M4_GetRotationsXYZ(&DestM[j], &Rot);

            // Place new rotation in animation
			ANIM_SetRotation( pAnim, i, j*3+0, ((s32)(Rot.X * ExportAngleConversion))*OneOverExportAngleConversion );
			ANIM_SetRotation( pAnim, i, j*3+1, ((s32)(Rot.Y * ExportAngleConversion))*OneOverExportAngleConversion );
			ANIM_SetRotation( pAnim, i, j*3+2, ((s32)(Rot.Z * ExportAngleConversion))*OneOverExportAngleConversion );
        }
    }

    //---   Minimize deltas in rotation streams for maximum compression
    for (i=0; i<pAnim->NBones*3; i++)
    {
        ANIM_MinimizeAngleDiff( pAnim, i );
    }

    /////////////////////////////////////////////////////////////
	//---	Keep Record of beginning of Data in File
    /////////////////////////////////////////////////////////////

	//---	set the new number of frames into the animation
	GetAnim()->NFrames = pAnim->NFrames;
	x_memcpy( GetAnim()->Stream, pAnim->Stream, pAnim->NFrames*pAnim->NStreams*sizeof(s16) );

	//---	set the export parameters which were active for this motion at the time it was compressed.
	m_ExportBits = GetExportBits();
	m_bExportBits = true;

	m_ExportFrameRate = GetExportFrameRate();
	m_bExportFrameRate = true;

	//---	remove the temporary animation buffer
    ANIM_KillInstance (&Anim);
#else
    s16*	RawData;
    byte*	CompData;
    s16*	DecompData;
    s32		RawDataSize;
    s32		CompDataSize;
    s32		i,j;
    anim	Anim;
    anim*	pAnim;
    skel*	pSkel;
	bool	RotationsConvertedToXZY=FALSE;
	s32		TotalAnimRawDataSize=0;

    //---   Get duplicate of animation for processing
    pAnim = GetAnim() ;
    ANIM_InitInstance (&Anim);
    ANIM_CopyAnim (&Anim, pAnim);
    pAnim = &Anim;
    CSkeleton *pSkeleton = this->m_pCharacter->GetSkeleton() ;
    pSkel = pSkeleton->GetSkel();

//---   Convert bone rotations from local XYZ to global XZY
#ifndef ACCURATE_COMPRESSION
	f32 Rotation;
	f32	ExportAngleConversion = (f32)(1<<GetExportBits())/R_360;
	f32	OneOverExportAngleConversion = R_360/((f32)(1<<GetExportBits()));

//	f32 Anti_Rotation = R_90;//-ANIM_GetRotation( pAnim, 0, 1 );

	for (i=0; i<(pAnim->NBones*3); ++i)
	{
		for (j=0; j<pAnim->NFrames; ++j)
		{
			Rotation = ANIM_GetRotation( pAnim, j, i );
//			if( i==1 )
//			{
//				Rotation += Anti_Rotation;
//			}
			Rotation = (f32)((s32)(Rotation*ExportAngleConversion)) * OneOverExportAngleConversion;
			ANIM_SetRotation( pAnim, j, i, Rotation );
//			Rotation = (f32)((s32)(Rotation*ExportAngleConversion));
//			ANIM_SetRotValue( pAnim, j, i, (s32)Rotation );
		}
	}
//
//	s16 *pStream;
//	f32 X, Z;
//	f32 NX, NZ;
//	f32 S, C;
//	x_sincos( Anti_Rotation, &S, &C );
//	for( i=0; i<pAnim->NFrames; i++ )
//	{
//		pStream = &pAnim->Stream[ i*pAnim->NStreams + pAnim->NBones*3 ];
//		X = ((f32)pStream[0]) / 16.0f;
//		Z = ((f32)pStream[2]) / 16.0f;
//		NX = C*X + S*Z;
//		NZ = C*Z - S*X;
//		pStream[0] = (s16)(NX * 16.0f);
//		pStream[2] = (s16)(NZ * 16.0f);
//	}

#else
    {
        matrix4 DestM[50];
        matrix4 WorldOrientM;
		f32		ExportAngleConversion = ((f32)(1<<GetExportBits()))/R_360;

		ASSERT(pAnim->NBones<=50);

        //---   Build WorldOrientM
        M4_Identity( &WorldOrientM );

        //---   Build matrices and compute global XZY rotations
        for (i=0; i<pAnim->NFrames; i++)
        {
            // Compute local to world matrix
            SKEL_BuildMatrices (pSkel, pAnim, i, DestM, &WorldOrientM);

            // Pull global XYZ rotations from matrix and convert to global XZY
            for (j=0; j<pAnim->NBones; j++)
            {
                radian3d Rot;
                M4_GetRotationsXYZ(&DestM[j], &Rot);
//                x_XYZToXZYRot( &Rot );

                // Place new rotation in animation
				ANIM_SetRotValue( pAnim, i, j*3+0, (s32)(Rot.X * ExportAngleConversion) );
				ANIM_SetRotValue( pAnim, i, j*3+1, (s32)(Rot.Y * ExportAngleConversion) );
				ANIM_SetRotValue( pAnim, i, j*3+2, (s32)(Rot.Z * ExportAngleConversion) );
//				pAnim->Stream[ (pAnim->NFrames*((j*3)+0)) + i ] = (s16)(Rot.X * ExportAngleConversion);
//				pAnim->Stream[ (pAnim->NFrames*((j*3)+1)) + i ] = (s16)(Rot.Y * ExportAngleConversion);
//				pAnim->Stream[ (pAnim->NFrames*((j*3)+2)) + i ] = (s16)(Rot.Z * ExportAngleConversion);
            }
        }

		RotationsConvertedToXZY = true;
    }
#endif

    //---   Minimize deltas in rotation streams for maximum compression
    for (i=0; i<pAnim->NBones*3; i++)
    {
        ANIM_MinimizeAngleDiff( pAnim, i, ANGLE_360 );
    }
    
    //---   Allocate Raw and Comp data blocks
    RawData = (s16*)x_malloc(sizeof(s16)*GetNumFrames()*256);
    CompData = (byte*)x_malloc(sizeof(s16)*GetNumFrames()*256);
	DecompData = (s16*)x_malloc(sizeof(s16)*GetNumFrames()*256);
    ASSERT(RawData && CompData && DecompData);

	//---	Determine First and Last Frame to Export to this Block
	s32	L			= 0 ;
	s32	R			= GetNumFrames()-1 ;

	//---	Export Block of Frames
    RawDataSize  = 0;
    CompDataSize = 0;

    /////////////////////////////////////////////////////////////
	//---	Loop through all Streams and load into RawData
    /////////////////////////////////////////////////////////////
//	GetRawAnimFrameDataBlockForExport( RawData, &RawDataSize, pMotion, L, R )
	{
		#define INSIGNIFICANT_VALUE 0.00001f

		//---	get the exported frame rate, round it off the same way that it is exported below
		f32 FrameStep	= 60.0f/GetExportFrameRate();
		for( int nStream = 0 ; nStream < pAnim->NStreams ; ++nStream )
		{
			s16 Value ;
			s16 NextValue;
			s32 WholeFrame;
			f32 FracFrame;
			f32 AmountPast;
			f32 nFrame;

			//---	Loop through All Frames in this Block and load into RawData
			for( nFrame = (f32)L ; ((f32)R - nFrame) > INSIGNIFICANT_VALUE ; nFrame+=FrameStep )
			{
				WholeFrame = (s32)nFrame;
				FracFrame = nFrame - (f32)WholeFrame;

				//---	interpolate this frame
				ANIM_GetStreamValue( pAnim, WholeFrame, nStream, &Value );
				ANIM_GetStreamValue( pAnim, WholeFrame+1, nStream, &NextValue );
				Value = Value + (s16)((f32)(NextValue-Value)*FracFrame);

				RawData[RawDataSize] = Value;
				RawDataSize++;
			}

			//---	Export a 'projected' last frame if needed
			AmountPast = nFrame - R;

			//---	if we were close, or through rounding error didn't quite make it to the last frame, simply use the last frame
			if( AmountPast < INSIGNIFICANT_VALUE )
				ANIM_GetStreamValue( pAnim, R, nStream, &Value );
			else
			{
				s16 PrevValue;
				ANIM_GetStreamValue( pAnim, R-1, nStream, &PrevValue ) ;
				ANIM_GetStreamValue( pAnim, R  , nStream, &Value ) ;
				Value = Value + (s16)((f32)(Value-PrevValue)*AmountPast);
			}

			RawData[RawDataSize] = Value;
			RawDataSize++;
		}
	}
	ASSERT((RawDataSize % pAnim->NStreams)==0);
#if 0
    /////////////////////////////////////////////////////////////
    //---   Compress RawData into CompData buffer
    /////////////////////////////////////////////////////////////
    CompDataSize = ACCCMP_Encode(CompData,
                                 RawData,
                                 pAnim->NStreams,
                                 (RawDataSize / pAnim->NStreams));

    /////////////////////////////////////////////////////////////
	//---	Decompress the CompData into the DecompData buffer
    /////////////////////////////////////////////////////////////
	ACCCMP_Decode( CompData, &DecompData[TotalAnimRawDataSize], pAnim->NStreams, (s16)(RawDataSize / pAnim->NStreams) );
#else
	x_memcpy( &DecompData[TotalAnimRawDataSize], RawData, RawDataSize*sizeof(s16) );
#endif
    /////////////////////////////////////////////////////////////
	//---	Keep Record of beginning of Data in File
    /////////////////////////////////////////////////////////////
	TotalAnimRawDataSize += RawDataSize;

	x_memcpy( pAnim->Stream, DecompData, TotalAnimRawDataSize*sizeof(s16) );
	x_memcpy( GetAnim()->Stream, pAnim->Stream, GetAnim()->NFrames*GetAnim()->NStreams*sizeof(s16) );

    x_free(RawData);
    x_free(CompData);
	x_free(DecompData);

	//---	set the new number of frames into the animation
	ASSERT((TotalAnimRawDataSize % pAnim->NStreams)==0);
	GetAnim()->NFrames = TotalAnimRawDataSize / (GetAnim()->NStreams);
	pAnim->NFrames = GetAnim()->NFrames;

	//---	set the export parameters which were active for this motion at the time it was compressed.
	m_ExportBits = GetExportBits();
	m_bExportBits = true;

	m_ExportFrameRate = GetExportFrameRate();
	m_bExportFrameRate = true;

#ifndef ACCURATE_COMPRESSION

	//---	convert the compressed/uncompressed data back into the anim buffer standard buffer sizes
/*	f32		ExportAngleReset = R_360/((f32)(1<<GetExportBits()));
	for( i=0; i<GetAnim()->NFrames; i++ )
		for( j=0; j<GetAnim()->NBones*3; j++ )
		{
			f32 Value = (f32)ANIM_GetRotValue( GetAnim(), i, j );
			ANIM_SetRotation( GetAnim(), i, j, Value*ExportAngleReset ); 
		}*/

#else

	//---	if the rotations were converted, convert them back
	if( RotationsConvertedToXZY )
	{
		matrix4 DestM[50];
		matrix4 WorldOrientM;
		f32		ExportAngleConversion = ((f32)(1<<GetExportBits()))/R_360;
		f32		ExportAngleReset = R_360/((f32)(1<<GetExportBits()));

		ASSERT(GetAnim()->NBones<=50);

		//---   Build WorldOrientM
		M4_Identity( &WorldOrientM );

		//---	convert the compressed/uncompressed data back into the anim buffer standard buffer sizes
		for( i=0; i<GetAnim()->NFrames; i++ )
			for( j=0; j<GetAnim()->NBones*3; j++ )
			{
				f32 Value = (f32)ANIM_GetRotValue( GetAnim(), i, j );
				ANIM_SetRotation( GetAnim(), i, j, Value*ExportAngleReset ); 
			}

		//---   Build matrices and compute global XYZ rotations
		//		while doing so, move the data back into the GetAnim() data
		for (i=0; i<GetAnim()->NFrames; i++)
		{
			// Compute local to world matrix
//			SKEL_BuildMatricesXZY (pSkel, GetAnim(), i, DestM, &WorldOrientM);
			SKEL_BuildMatrices (pSkel, GetAnim(), i, DestM, &WorldOrientM);

			// Pull global XZY rotations from matrix and convert to global XYZ
			for (j=0; j<GetAnim()->NBones; j++)
			{
				radian3d Rot;
//				M4_GetRotationsXZY(&DestM[j], &Rot);
//				x_XZYToXYZRot( &Rot );
				M4_GetRotationsXYZ(&DestM[j], &Rot);

				// Place new rotation in animation
				ANIM_SetRotation( GetAnim(), i, j*3+0, Rot.X );
				ANIM_SetRotation( GetAnim(), i, j*3+1, Rot.Y );
				ANIM_SetRotation( GetAnim(), i, j*3+2, Rot.Z );
//				GetAnim()->Stream[ (GetAnim()->NFrames*((j*3)+0)) + i ] = (s16)(Rot.X * ExportAngleConversion);
//				GetAnim()->Stream[ (GetAnim()->NFrames*((j*3)+1)) + i ] = (s16)(Rot.Y * ExportAngleConversion);
//				GetAnim()->Stream[ (GetAnim()->NFrames*((j*3)+2)) + i ] = (s16)(Rot.Z * ExportAngleConversion);
			}
		}
	}
#endif

	//---	remove the temporary animation buffer
    ANIM_KillInstance (&Anim);
#endif
}

///////////////////////////////////////////////////////////////////////////////
CMotion* CMotion::GetCompressedVersion( void )
{
	//---	if there is already a compressed version, make sure that it is the right compressed format and then return it
	if( m_pCompVersion )
	{
		if(( m_pCompVersion->GetExportBits() == GetExportBits() ) &&
		   ( m_pCompVersion->GetExportFrameRate() == GetExportFrameRate() ))
		   return m_pCompVersion;

		//---	it has changed since the last time it was compressed, delete it to be recompressed
		delete m_pCompVersion;
	}

	//---	create a compressed version
	m_pCompVersion = new CMotion;
	*m_pCompVersion = *this;
	m_pCompVersion->MakeIntoCompressedVersion();

	//---	return the compressed version
	return m_pCompVersion;
}

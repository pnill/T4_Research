// Motion.cpp : implementation file
//

#include "stdafx.h"

#include "Motion.h"
#include "ieArchive.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	MOTION_AMC_VERSION	1

#define	INTERPOLATE_ROOT_SAVES

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
	m_nFrames60 = 0 ;
	m_FrameRate	= 0 ;
	m_pRootRotsSave = NULL ;
	m_pFaceDir = NULL ;
	m_pMoveDir = NULL ;
	m_pCurEvent = NULL ;
	m_bAutoResetMoveDir = TRUE;
	m_bAutoResetFaceDir = TRUE;
	m_bExport = TRUE;
	m_ChangedExport = FALSE;
	m_bExportFrameRate = FALSE;
	m_ExportFrameRate = 30.0f;
	m_bExportBits = FALSE;
	m_ExportBits = 12;
	m_pCompVersion = NULL;
	m_FaceDirRecipeUsed = -2; // -2 makes sure that it is different from the first one used
	m_FaceDirArrayLength	= 0;
	m_MoveDirArrayLength	= 0;
	m_bExportDefault = TRUE;
	m_bExportQuaternion = TRUE;
	m_bExportQuatW		= FALSE;
	m_bExportWorldRots = FALSE;
	m_bExportGlobalRots = FALSE;
	m_bExportQuatBlending = FALSE;
	m_bExportPropQuatBlending = FALSE;
	for( int i = 0 ; i < MOTION_FLAG_BITS ; i++ ) m_FlagBit[i] = false ;

	for( i=0; i<MAX_NUM_PROPS; i++ )
	{
		m_Props[i].SetIndex( i );
		m_Props[i].SetMotion( this );
	}

	m_bSupressWarningBoxes		= FALSE;

	m_pLinkCharacter_OldSystem	= NULL;
	m_pLinkMotion_OldSystem		= NULL;
	m_bExportLink_OldSystem		= TRUE;

	m_RuntimeVersion			= 0;
	m_BlendedMotionCount		= 0; // it isn't blended, it is loaded from a file
	for( i=0; i<MAX_NUM_BLENDED_MOTIONS; i++ )
	{
		m_BlendedMotions[i].pMotion						= NULL;
		m_BlendedMotions[i].Ratio						= 0.0f;
		m_BlendedMotions[i].RuntimeVersion				= -1;
	}

	m_bInUpdateMotionVersion	= FALSE;
}

void CMotion::Destroy()
{
	//---	Initialize Data Members
	if( m_pFaceDir )
	{
		x_free( m_pFaceDir ) ;
		m_pFaceDir = NULL ;
	}
	if( m_pMoveDir )
	{
		x_free( m_pMoveDir ) ;
		m_pMoveDir = NULL ;
	}

	if( m_pCompVersion )
	{
		delete m_pCompVersion;
		m_pCompVersion = NULL;
	}

	if( m_pRootRotsSave )
	{
		x_free( m_pRootRotsSave ) ;
		m_pRootRotsSave = NULL;
	}

	ANIM_KillInstance( &m_Anim ) ;
	Initialize () ;
}

//////////////////////////////////////////////////////////////////////////////
void CMotion::Copy (CMotion &Dst, CMotion &Src)
{
	//---	copy everything else
	Dst.m_pCharacter					= Src.m_pCharacter;
	Dst.m_PathName						= Src.m_PathName;
	Dst.m_ExportName					= Src.m_ExportName;
	Dst.m_IsLoaded						= Src.m_IsLoaded;
	Dst.m_LoadFailed					= Src.m_LoadFailed;
	Dst.m_IsANM							= Src.m_IsANM;
	Dst.m_nFrames60						= Src.m_nFrames60;
	Dst.m_FrameRate						= Src.m_FrameRate;
	Dst.m_EventList						= Src.m_EventList;
	Dst.m_pCurEvent						= Src.m_pCurEvent;
	Dst.m_bExport						= Src.m_bExport;
	Dst.m_bExportFrameRate				= Src.m_bExportFrameRate;
	Dst.m_ExportFrameRate				= Src.m_ExportFrameRate;
	Dst.m_bExportBits					= Src.m_bExportBits;
	Dst.m_ExportBits					= Src.m_ExportBits;
	Dst.m_FaceDirArrayLength			= Src.m_FaceDirArrayLength;
	Dst.m_MoveDirArrayLength			= Src.m_MoveDirArrayLength;
	Dst.m_bExportDefault				= Src.m_bExportDefault;
	Dst.m_bExportQuaternion				= Src.m_bExportQuaternion;
	Dst.m_bExportQuatW					= Src.m_bExportQuatW;
	Dst.m_bExportWorldRots				= Src.m_bExportWorldRots;
	Dst.m_bExportGlobalRots				= Src.m_bExportGlobalRots;
	Dst.m_bExportQuatBlending			= Src.m_bExportQuatBlending;
	Dst.m_bExportPropQuatBlending		= Src.m_bExportPropQuatBlending;
	Dst.m_bAutoResetFaceDir				= Src.m_bAutoResetFaceDir;
	Dst.m_bAutoResetMoveDir				= Src.m_bAutoResetMoveDir;

	Dst.m_bExportLink_OldSystem			= Src.m_bExportLink_OldSystem;

	//---	get the number of frames
	s32 nFrames = Src.GetNumFrames() ;
	if( nFrames==0 ) nFrames = 1;	// make sure the mallocs have something to allocate

	//---	if the animation is loaded, copy it over.
	if( Src.m_IsLoaded )
		ANIM_CopyAnim( &Dst.m_Anim, &Src.m_Anim );

	//---	copy the face directions
	if( Src.m_pFaceDir && (nFrames>0) )
	{
		Dst.m_pFaceDir = (f32*)x_malloc( nFrames*sizeof(f32) ) ;
		ASSERT( Dst.m_pFaceDir ) ;
		x_memcpy( Dst.m_pFaceDir, Src.m_pFaceDir, nFrames*sizeof( f32 ) ) ;
	}
	else
		Dst.m_pFaceDir = NULL;

	//---	copy the move directions
	if( Src.m_pMoveDir && (nFrames>0))
	{
		Dst.m_pMoveDir = (f32*)x_malloc( nFrames*sizeof(f32) ) ;
		ASSERT( Dst.m_pMoveDir ) ;
		x_memcpy( Dst.m_pMoveDir, Src.m_pMoveDir, nFrames*sizeof( f32 ) ) ;
	}
	else
		Dst.m_pMoveDir = NULL;

	//---	copy the root rotations save
	if( Src.m_pRootRotsSave && (nFrames>0) )
	{
		Dst.m_pRootRotsSave = (radian3*)x_malloc( nFrames*sizeof(radian3) ) ;
		ASSERT( Dst.m_pRootRotsSave ) ;
		x_memcpy( Dst.m_pRootRotsSave, Src.m_pRootRotsSave, nFrames*sizeof( radian3 ) ) ;
	}
	else
		Dst.m_pRootRotsSave = NULL;

	//---	copy over the prop data
	s32 i;
	for( i=0; i<MAX_NUM_PROPS; i++ )
		Dst.m_Props[i]	= Src.m_Props[i];
	
	//---	copy the flag bits
	x_memcpy( &Dst.m_FlagBit[0], &Src.m_FlagBit[0], MOTION_FLAG_BITS*sizeof(bool) ) ;

	//---	copy the skel point checks
	Dst.m_SkelPointChecks.SetSize( Src.m_SkelPointChecks.GetSize() );
	for( i=0; i<Src.m_SkelPointChecks.GetSize(); i++ )
		Dst.m_SkelPointChecks[i] = Src.m_SkelPointChecks[i];

	//---	copy the blended motion information
	Dst.m_BlendedMotionCount = Src.m_BlendedMotionCount;
	for( i=0; i<Src.m_BlendedMotionCount; i++ )
		Dst.m_BlendedMotions[i] = Src.m_BlendedMotions[i];
}

//////////////////////////////////////////////////////////////////////////////
bool CMotion::Modify( void )
{
	CDlgMotion	Dialog;
	bool		ReloadFile = FALSE;
	bool		bExport = m_bExport ? TRUE : FALSE;

	Dialog.m_pMotion	= this;
	if( Dialog.DoModal() == IDOK )
	{
		//---	determine if the file changed
		if( GetPathName() != Dialog.m_Motion.GetPathName() )
			ReloadFile = TRUE;

		//---	set the new data
		m_ChangedExport = ((m_bExport?TRUE:FALSE) != bExport) ? TRUE : FALSE;

		//---	if the file has changed, reload the file
		if( ReloadFile )
			ReloadFromFile( m_bSupressWarningBoxes );

		return TRUE;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
//	
//
static radian ComputeRootDirFromXYZ (radian RX,radian RY,radian RZ)
{
#if 1 // quaternion method, not only is the code nicer, the results are even more stable.  - Kevin

	quaternion q;

	//---	turn the rotations into a quaterion
	q.Identity();
	q.RotateX( RX );
	q.RotateY( RY );
	q.RotateZ( RZ );
	q.Normalize();

	//---	remove the pitch and roll from the quaternion (our face dir is reletive only to the yaw)
	q.X = 0.0f;
	q.Z = 0.0f;
	q.Normalize();

	//---	rotate a vector which points out the Pitch=0, Yaw=0, Roll=0 direction into place.
	vector3 v(0.0f, 0.0f, 1.0f);
	v = q*v;

	//---	this vector should have a Y component of zero (because we removed the pitch/roll from the quat above)
	//		get the direction pointed to by the X and Z components of the vector, this is our new RootDir
	return x_atan2( v.X, v.Z );


#elif 1 // this new code is much much better than the old code (below), its only weakness is it will create a jump in face dirs
		// of up to 180 degrees as the person rolls over.  This is not too bad because people don't roll over very often, and when
		// they do we don't break the animation during it.  However, for replay, the code can interpolate with a bit of a wable.
		// - Kevin

	vector3 v;
	vector3 n;
	vector3 d;

	//---	get the vector pointing
	v.Set( 0.0f, 0.0f, 1.0f );
	v.Rotate( radian3( RX, RY, RZ ) );

	//---	get the normal to the ZY plane
	n.Set( 1.0f, 0.0f, 0.0f );
	n.Rotate( radian3( RX, RY, RZ ) );

	if( x_abs(n.X) > 0.00001f )
		d.X = -n.Z*v.Z / n.X;
	else if(( -n.Z*v.Z >= 0.0f ) && ( n.X >= 0.0f ))
		d.X = 1.0f;
	else
		d.X = -1.0f;
		
	d.Y = 0.0f;
	d.Z = v.Z;

	f32 FaceDir = x_atan2( d.X, d.Z );

	//---	The FaceDir can now face one of two directions, either the way we want, or
	//		180 degrees the opposite.  To select the correct of these two directions,
	//		lets chose the one which has an acute angle to the anim unit Z direction.
	//		However, if the player is tipped upside down, lets make it the one closest
	//		to the -Z direction so that it always points the direction that the player
	//		is facing if his feet are rotated to the ground along the shortest root.
	//
	vector3	vFaceDirSelectionVector;
	vector3 y;
	y.Set(0.0f, 1.0f, 0.0f);
	y.Rotate( radian3( RX, RY, RZ ) );
	if( y.Y < 0.0f )
		vFaceDirSelectionVector = -v;
	else
		vFaceDirSelectionVector = v;

	//---	if the angle between the FaceDir and the root vector is greater than
	//		R_90, reverse the direction of the FaceDir.
	radian	AngDiff = x_abs(x_acos(vFaceDirSelectionVector.Dot( d ) / (vFaceDirSelectionVector.Length()*d.Length())));
	if( AngDiff > R_90 )
		FaceDir += R_180;

	return FaceDir;

#else

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
    x_sincos(RX,sx,cx);
    x_sincos(RY,sy,cy);
    x_sincos(RZ,sz,cz);

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

#endif
}


//========================================================================================
//
//	Debugging function to display root rotation data from loading to export for multiple animations.
//	this is useful to check the export data.
//
void DisplayAnimMB( CMotion* pMotion, anim* pAnim, s32 Mode, const char* ModeName )
{
	static	const	s32		NumAnims = 2;
	static	const	s32		NumModes = 4;
	static			char*	Anims[NumAnims] = { "QBK_1MN_3012", "BAL_QBK_1MN_3012" };
	static			radian3	AnimRots[NumAnims][NumModes];
	static			CString	ModeNames[NumModes];
	static			radian	YRot=0.0f;
					bool	bDisplayInfoBox;
					s32		i, j;

	bDisplayInfoBox = (Mode&0x8000) ? TRUE : FALSE;
	Mode &= 0x7fff;

	ASSERT((Mode>=0)&&(Mode<=NumModes));

	//---	assign the mode name	
	if(( Mode < NumModes )&&( ModeNames[Mode].GetLength()==0 ))
		ModeNames[Mode].Format( "%s", ModeName );

	//---	find which motion this is
	for( i=0; i<NumAnims; i++ )
	{
		if( strncmp( Anims[i], pMotion->GetExportName(), strlen( Anims[i] ) ) == 0 )
			break;
	}

	//---	if it wasn't found, exit now
	if( i == NumAnims )
		return;

	if( Mode < NumModes )
	{
		radian3& rRot = AnimRots[i][Mode];
		if( Mode == NumModes-1 )
		{
			rRot.X = ANIM_GetRotation( pAnim, 0, pAnim->NBones*3+5 + 0 );
			rRot.Y = ANIM_GetRotation( pAnim, 0, pAnim->NBones*3+5 + 1 );
			rRot.Z = ANIM_GetRotation( pAnim, 0, pAnim->NBones*3+5 + 2 );
		}
		else
		{
			rRot.X = ANIM_GetRotation( pAnim, 0, 0 );
			rRot.Y = ANIM_GetRotation( pAnim, 0, 1 );
			rRot.Z = ANIM_GetRotation( pAnim, 0, 2 );
		}

		CString ModeStr;
		if( bDisplayInfoBox )
		{
			ModeStr.Format( "Mode(%d):  %s  (%8.5f, %8.5f, %8.5f)\n", Mode, ModeNames[Mode], rRot.X, rRot.Y, rRot.Z );
			MessageBox( NULL, ModeStr, pMotion->GetExportName(), MB_OK );
		}
	}
	else
	{
		CString message;
		CString ModeStr;

		for( j=0; j<NumModes; j++ )
		{
			radian3& rRot = AnimRots[i][j];
			ModeStr.Format( "Mode(%d):  %s  (%8.5f, %8.5f, %8.5f)\n", j, ModeNames[j], rRot.X, rRot.Y, rRot.Z );
			message += ModeStr;
		}

		MessageBox( NULL, message, pMotion->GetExportName(), MB_OK );
	}
}

///////////////////////////////////////////////////////////////////////////////
bool CMotion::CreateFromAMC( const char *pFileName, bool SupressWarningBoxes, long* pResult )
{
	m_PathName = pFileName ;

	//---	Get the skeleton and make sure the ASF file was able to be loaded
	CSkel *pSkel = m_pCharacter->GetSkeleton()->GetSkel();
	if( pSkel->m_pBone == NULL )
	{
		static xbool bMessageDisplayed = FALSE;
		if( !bMessageDisplayed )
		{
			bMessageDisplayed = TRUE;
			::MessageBox( NULL, "Motion cannot be loaded without a skeleton.", "ERROR LOADING ANIMATION", MB_ICONWARNING );
		}
		return FALSE;
	}

	if( pResult ) *pResult = 0;

	//---	attempt to load the animation
	err Error = ANIM_ImportAMC( &m_Anim, pFileName, pSkel, MOTION_AMC_VERSION ) ;

	//---	if the animation did not load propertly, inform the user and get out
	if( (Error != ERR_SUCCESS) || (m_Anim.NFrames == 0) )
    {
		if( !SupressWarningBoxes )
		{
			CString String ;
			if( m_Anim.NFrames == 0 )
				String.Format( "Motion has no frames of data '%s'", pFileName );
			else
				String.Format( "Unable to load motion '%s'", pFileName );
			::MessageBox( NULL, String, "ERROR LOADING ANIMATION", MB_ICONWARNING );
		}
		if( pResult ) *pResult |= LOAD_RESULT_ERROR;
		m_LoadFailed = TRUE ;
		return FALSE;
    }

	//---	complete the process
	bool ret = CreateFromAnim( SupressWarningBoxes, pResult );

//DisplayAnimMB( this, GetAnim(), 0x8000|0, "ON LOAD");

	return ret;
}


///////////////////////////////////////////////////////////////////////////////
bool CMotion::CreateFromAnim( bool SupressWarningBoxes, long* pResult )
{
	s32 i;

	CSkel *pSkel = m_pCharacter->GetSkeleton()->GetSkel();
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
    }

	//---	Set Loaded Flags
	m_IsLoaded = TRUE ;

	//---	Set LoadFailed Flag
	m_LoadFailed = FALSE ;

	//---	Create and Initialize FaceDir & MoveDir Arrays from the loaded data
	s32 nFrames = GetNumFrames() ;

	//---	Warn if Number of Frames have changed
	if( (m_FaceDirArrayLength != 0) && (m_FaceDirArrayLength != nFrames))
	{
		if( !SupressWarningBoxes )
		{
			CString String ;
			String.Format( "Motion '%s' has changed number of frames since last loaded. From %d to %d", GetPathName(), m_FaceDirArrayLength, nFrames ) ;
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

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	EnsureFaceDirs();
	EnsureMoveDirs();
    
	//---	Set Number of Frames in each Array
	m_nFrames60	= nFrames ;
	m_FrameRate	= 1.0f ;	// assume that the animation loaded was a 60fps animation

	return m_IsLoaded ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::InvalidateFaceDirs( void )
{
	if( m_pFaceDir && m_bAutoResetFaceDir )
	{
		x_free( m_pFaceDir );
		m_pFaceDir = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::InvalidateMoveDirs( void )
{
	if( m_pMoveDir && m_bAutoResetMoveDir )
	{
		x_free( m_pMoveDir );
		m_pMoveDir = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::EnsureMoveDirs( void )
{
	s32 nFrames = GetNumFrames();
	if( nFrames == 0 ) 
		return;

	if( m_pMoveDir && (m_MoveDirArrayLength == nFrames) )
		return;

	f32	*pMoveDir = m_pMoveDir;
	m_pMoveDir = (f32*)x_malloc( nFrames * sizeof(f32) ) ;
	ASSERT( m_pMoveDir ) ;

	for( int i = 0 ; i < nFrames ; i++ )
	{
		//---	Generate MoveDir
		vector3 	TS, TE ;
		ANIM_GetTransValue( GetAnim(), i, &TS ) ;

		if( i >= (nFrames-1) )
		{
			vector3 	Diff ;
			ANIM_GetTransValue( GetAnim(), i-1, &TE ) ;
			Diff = TS - TE;
			TE += 2*Diff;
		}
		else
			ANIM_GetTransValue( GetAnim(), i+1, &TE ) ;

		TE -= TS;
		m_pMoveDir[i] = x_atan2( TE.X, TE.Z ) ;
	}
	
	//---	If Motion already Has a MoveDir Array then merge and truncate
	if( pMoveDir )
	{
		//---	Copy Existing Editted Buffer to New Buffer and Set New Buffer
		if( !m_bAutoResetMoveDir )
			x_memcpy( m_pMoveDir, pMoveDir, min( m_MoveDirArrayLength, nFrames ) * sizeof( f32 ) );

		x_free( pMoveDir ) ;
	}

	m_MoveDirArrayLength	= nFrames;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::EnsureFaceDirs( void )
{
	s32	Count;
	s32 i;

	s32 nFrames = GetNumFrames();

	//---	no frames means no motion
	if( nFrames == 0 )
		return;

	//---	if the FaceDir's are automatically reset, and the the recipe has changed, re-calculate the face dirs.
	if( m_pFaceDir &&
		m_pRootRotsSave &&
		(m_FaceDirArrayLength == nFrames) &&
		(!m_bAutoResetFaceDir || (m_FaceDirRecipeUsed == m_pCharacter->m_FaceDirRecipeID )) )
		return;

	f32 *pFaceDir = m_pFaceDir;
	m_pFaceDir = (f32*)x_malloc( nFrames * sizeof(f32) ) ;
	ASSERT( m_pFaceDir ) ;

	//---	save the root rotation values
	ASSERTS(!m_pRootRotsSave || (m_FaceDirArrayLength == nFrames), "Root rots save should have been removed before changing the number of frames");
	RestoreRootRots();	// if they had been saved before, put them back (otherwise we assume that they haven't been changed yet and leave them alone)
	SaveRootRots();		// save them before changing them

	//---	Generate FaceDirs from the motion itself
	for( i=0; i<nFrames; i++ )
		ResetFaceDir( i );

	//---	If Motion already Has a FaceDir Array 
	Count = 0;
	if( pFaceDir )
	{
		if( !m_bAutoResetFaceDir )
		{
			//---	Set the face directions for all frames using the SetFaceDir() function call so that the root rotations
			//		are updated correctly
			Count = min( m_FaceDirArrayLength, nFrames );
			for( i=0; i<Count; i++ )
				ChangeFaceDir( i, pFaceDir[i] );
		}

		//---	free the face direction buffer
		x_free( pFaceDir ) ;
	}

	//---	store the new number of frames of save face directions
	m_FaceDirArrayLength	= nFrames;

	//---	store the new recipe ID
	m_FaceDirRecipeUsed = m_pCharacter->m_FaceDirRecipeID;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::RestoreRootRots( void )
{
	s32 i;

	if( m_pRootRotsSave == NULL )
		return;

	for( i = 0 ; i < GetNumFrames(); i++ )
	{
		//---	store the root rotation values (msut be set here because ResetFaceDir()
		//		called below will access this data.
		ANIM_SetRotation( GetAnim(), i, 0, m_pRootRotsSave[i].X );
		ANIM_SetRotation( GetAnim(), i, 1, m_pRootRotsSave[i].Y );
		ANIM_SetRotation( GetAnim(), i, 2, m_pRootRotsSave[i].Z );
	}

	x_free( m_pRootRotsSave );
	m_pRootRotsSave = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::SaveRootRots( void )
{
	s32 i;

	if( m_pRootRotsSave != NULL )
		return;

	//---	no frames means no motion
	if( GetNumFrames()==0 )
		return;

	//---	get storage for the saved root rotation array.  These rotations are
	//		stored because the root rotations are going to be converted to
	//		frame local rotations based on the face direction of the animation
	//		on the given frame.
	m_pRootRotsSave	= (radian3*)x_malloc( GetNumFrames()*sizeof(radian3) );
	ASSERT(m_pRootRotsSave);
	for( i = 0 ; i < GetNumFrames(); i++ )
	{
		//---	store the root rotation values (must be set here because ResetFaceDir()
		//		called below will access this data.
		m_pRootRotsSave[i].X = ANIM_GetRotation( GetAnim(), i, 0 );
		m_pRootRotsSave[i].Y = ANIM_GetRotation( GetAnim(), i, 1 );
		m_pRootRotsSave[i].Z = ANIM_GetRotation( GetAnim(), i, 2 );
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::FrameAdjustRootRots( void )
{
	s32 i;

	//---	no frames means no motion
	if( GetNumFrames() == 0 )
		return;

	EnsureLoaded();
	SaveRootRots();

	if( m_bAutoResetFaceDir || !m_pFaceDir )
	{
		//---	if there is no face dir array, allocate one to be filled
		if( !m_pFaceDir )
		{
			m_pFaceDir				= (f32*)x_malloc( GetNumFrames()*sizeof(f32) );
			m_FaceDirArrayLength	= GetNumFrames();
		}

		//---	reset the face directions for each frame
		for( i=0; i<GetNumFrames(); i++ )
			ResetFaceDir( i );
	}
	else
	{
		//---	get the face directions from the current list of face dirs
		for( i=0; i<GetNumFrames(); i++ )
			SetFaceDir( i, m_pFaceDir[i] );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMotion::EnsureLoaded( long* pResult )
{
	if( !this )
		return FALSE;

	//---	if it is a blended motion, attempt to update it without loading
	if( IsBlendedMotion() )
	{
		//---	while updating the motion version, assume that the motion is loaded so that
		//		many needed functions will work.
		if( m_bInUpdateMotionVersion )
			return TRUE;

		CheckUpdateMotionVersion();
		return m_IsLoaded;
	}

	if( !m_IsLoaded && !m_LoadFailed )
		CreateFromFile( m_PathName, m_bSupressWarningBoxes, pResult );

	return m_IsLoaded ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMotion::CreateFromFile( const char *pFileName, bool SupressWarningBoxes, long* pResult )
{
	char	Ext[_MAX_EXT] ;

	_splitpath( pFileName, NULL, NULL, NULL, Ext ) ;
	if( stricmp( Ext, ".AMC" ) == 0 )
		CreateFromAMC( pFileName, SupressWarningBoxes, pResult ) ;

	//---	immediately after loaded from a file increment the runtime version so that any motions which use this one to
	//		create a new one will rebuild themselves.
	IncRuntimeVersion();

	return m_IsLoaded ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	CString ThisPathName = m_PathName;
	CString SearchPathName = rSearch;
	ThisPathName.MakeUpper();
	SearchPathName.MakeUpper();

	int Loc = ThisPathName.Find((LPCTSTR) rSearch );

	//---	if there are conditions on the search, return if they don't apply
	if ((Loc == -1 ) ||
		(( Method == FILEPATHS_LOOKFROMSTART ) && (Loc != 0)) ||
		(( Method == FILEPATHS_LOOKFROMEND ) && (Loc != (ThisPathName.GetLength() - SearchPathName.GetLength()))))
		return FALSE;

	//---	replace the search string in the file path
	CString FirstPart	= m_PathName.Left( Loc );
	CString LastPart	= m_PathName.Right( m_PathName.GetLength() - (Loc + rSearch.GetLength()) );

	m_PathName = FirstPart + rReplace + LastPart;
	return TRUE;
}

int CMotion::GetNumFrames()
{
	if( EnsureLoaded() )
		return m_Anim.NFrames ;

	return 0 ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
anim *CMotion::GetAnim()
{
	if( EnsureLoaded() )
		return &m_Anim ;

	return NULL ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMotion::GetStartFrame()
{
	if( EnsureLoaded() )
		return m_Anim.StartFrame ;

	return NULL ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
radian CMotion::GetMoveDir( int nFrame )
{
	if( !EnsureLoaded() ) return 0.0f;
	EnsureMoveDirs();
	ASSERT( m_pMoveDir ) ;

	return m_pMoveDir[nFrame] ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::SetMoveDir( int nFrame, radian MoveDir )
{
	if( !EnsureLoaded() ) return;
	EnsureMoveDirs();
	ASSERT( m_pMoveDir ) ;

	while( MoveDir > DEG_TO_RAD( 180) ) MoveDir -= DEG_TO_RAD(360) ;
	while( MoveDir < DEG_TO_RAD(-180) ) MoveDir += DEG_TO_RAD(360) ;
	
	m_pMoveDir[nFrame] = MoveDir ;
	m_bAutoResetMoveDir = FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMotion::SetAllMoveDir( radian MoveDir )
{
	if( !EnsureLoaded() ) return;
	EnsureMoveDirs();
	ASSERT( m_pMoveDir ) ;

	while( MoveDir > DEG_TO_RAD( 180) ) MoveDir -= DEG_TO_RAD(360) ;
	while( MoveDir < DEG_TO_RAD(-180) ) MoveDir += DEG_TO_RAD(360) ;
	
	for( int i = 0 ; i < GetNumFrames() ; i++ )
		m_pMoveDir[i] = MoveDir ;
	m_bAutoResetMoveDir = FALSE;
}

//=============================================================================================
radian CMotion::GetFaceDir( int nFrame )
{
	if( !EnsureLoaded() ) return 0.0f;
	EnsureFaceDirs();
	ASSERT( m_pFaceDir ) ;

	return m_pFaceDir[nFrame] ;
}

//=============================================================================================
radian CMotion::GetFaceDirBlend( float fFrame )
{
	if( !EnsureLoaded() ) return 0.0f;
	EnsureFaceDirs();
	ASSERT( m_pFaceDir ) ;

	s32 iframe	= (s32)fFrame;
	f32	Frac	= fFrame - (f32)iframe;

	if( iframe >= GetNumFrames()-1 )
		return m_pFaceDir[GetNumFrames()-1];

	f32	f0 = m_pFaceDir[iframe];
	f32 f1 = m_pFaceDir[iframe+1];
	f32	d = f1-f0;
	while(d>=R_180)	d -= R_360;
	while(d<-R_180)	d += R_360;
	return f0 + Frac*d;;
}

//=============================================================================================
void CMotion::GetBoneRots( s32 nFrame, s32 BoneID, f32& Rx, f32& Ry, f32& Rz )
{
	SSkelBone* pBone = m_pCharacter->GetSkeleton()->GetSkel()->m_pBone;

	vector3 v(0.0f, 0.0f, 1.0f);
	radian3 r;
	matrix4 m;

	m.Identity();
	while( BoneID != -1 )
	{
		r.X = ANIM_GetRotation( GetAnim(), nFrame, 3*BoneID + 0 );
		r.Y = ANIM_GetRotation( GetAnim(), nFrame, 3*BoneID + 1 );
		r.Z = ANIM_GetRotation( GetAnim(), nFrame, 3*BoneID + 2 );

		m.RotateXYZ( r );
		BoneID = pBone[BoneID].ParentID;
	}

	r = m.GetRotationXYZ();
	Rx = r.X;
	Ry = r.Y;
	Rz = r.Z;

	while( Rx>R_180 ) Rx -= R_360;
	while( Rx<-R_180 ) Rx += R_360;
	while( Ry>R_180 ) Ry -= R_360;
	while( Ry<-R_180 ) Ry += R_360;
	while( Rz>R_180 ) Rz -= R_360;
	while( Rz<-R_180 ) Rz += R_360;
}

//=============================================================================================
void CMotion::ChangeFaceDir( s32 iFrame, f32 FaceDir )
{
	//---	If the root node is being changed, the runtime version has changed to require any motions created
	//		using this motion to update because blended motions are re-alligned based on their first frame.
	if( (iFrame == 0) && (m_pFaceDir[iFrame] != FaceDir) )
		IncRuntimeVersion();

	//---	save the new face direction
	m_pFaceDir[iFrame]	= FaceDir;

	//---	pull the new face direction out of the frame's original root rotations to put the
	//		animation rotations into frame space
	matrix4 m;
	radian3 r;
	SaveRootRots();
	ASSERT(m_pRootRotsSave);
	m.SetRotationXYZ( m_pRootRotsSave[iFrame] );
	m.RotateY( -FaceDir );
	r = m.GetRotationXYZ();
	ANIM_SetRotation( GetAnim(), iFrame, 0, r.X );
	ANIM_SetRotation( GetAnim(), iFrame, 1, r.Y );
	ANIM_SetRotation( GetAnim(), iFrame, 2, r.Z );
}

//=============================================================================================
void CMotion::ResetFaceDir( s32 nFrame )
{
	if( !EnsureLoaded() ) return;
	ASSERT(m_pFaceDir);

	radian	FaceDir,FDir;
	f32		vX, vZ;
    radian	Rx, Ry, Rz ;
	f32		Weight;
	f32		TotalWeight;
	s32		i;

	//---	save the root rotation values
	SaveRootRots();

	//---	put the old root rotation back for this frame temporarily
	ANIM_SetRotation( GetAnim(), nFrame, 0, m_pRootRotsSave[nFrame].X );
	ANIM_SetRotation( GetAnim(), nFrame, 1, m_pRootRotsSave[nFrame].Y );
	ANIM_SetRotation( GetAnim(), nFrame, 2, m_pRootRotsSave[nFrame].Z );

	//---	get the recipe of bone/weights used to calculate the face direction
	s32 RecipeLen			= m_pCharacter->GetFaceDirRecipeLength();
	SBoneWeight* pRecipe	= (SBoneWeight*)&m_pCharacter->GetFaceDirRecipe( 0 );

	//---	if there is only one weight, handle it as quickly as possible
	if( RecipeLen == 1 )
	{
		GetBoneRots( nFrame, pRecipe[0].BoneID, Rx, Ry, Rz );
        FaceDir = ComputeRootDirFromXYZ( Rx, Ry, Rz ) ;
	}
	else
	{
		vX = vZ = 0.0f;
		TotalWeight	= 0.0f;

		//---	for each bone/weight in the recipe, calculate its FaceDir and add
		//		it to the running total (by adding vectors).
		for( i=0; i<RecipeLen; i++ )
		{
			//---	get the bone's rotation
			GetBoneRots( nFrame, pRecipe[i].BoneID, Rx, Ry, Rz );

			//---	calculate the bone's FaceDir
			FDir = ComputeRootDirFromXYZ( Rx, Ry, Rz ) ;

			//---	add this face dir to the running total (use vector addition)
			Weight = (f32)pRecipe[i].Weight;
			TotalWeight += Weight;
			vX += Weight*x_sin( FDir );
			vZ += Weight*x_cos( FDir );
		}

		//---	calculate the final face dir.
		vX /= TotalWeight;
		vZ /= TotalWeight;
		FaceDir = x_atan2( vX, vZ );
	}

	//---	save the new face direction
	ChangeFaceDir( nFrame, FaceDir );
}

//=============================================================================================
void CMotion::SetFaceDir( int nFrame, radian FaceDir )
{
	if( !EnsureLoaded() ) return;
	ASSERT( m_pFaceDir ) ;
	ASSERT(!m_bAutoResetFaceDir) ;
	m_bAutoResetFaceDir = FALSE;
	m_FaceDirRecipeUsed = -2;

	while( FaceDir > DEG_TO_RAD( 180) ) FaceDir -= DEG_TO_RAD(360) ;
	while( FaceDir < DEG_TO_RAD(-180) ) FaceDir += DEG_TO_RAD(360) ;

	//---	save the new face direction
	ChangeFaceDir( nFrame, FaceDir );
}

//=============================================================================================
void CMotion::SetAllFaceDir( radian FaceDir )
{
	if( !EnsureLoaded() ) return;
	ASSERT( m_pFaceDir ) ;
	ASSERT(!m_bAutoResetFaceDir) ;
	m_bAutoResetFaceDir = FALSE;
	m_FaceDirRecipeUsed = -2;

	while( FaceDir > DEG_TO_RAD( 180) ) FaceDir -= DEG_TO_RAD(360) ;
	while( FaceDir < DEG_TO_RAD(-180) ) FaceDir += DEG_TO_RAD(360) ;
	
	for( int i = 0 ; i < GetNumFrames() ; i++ )
		ChangeFaceDir( i, FaceDir );
}

//=============================================================================================
void CMotion::ResetAllFaceDir( xbool bForce )
{
	if( !EnsureLoaded() ) return;

	//---	if the recipe hasn't changed since the last update, there is no reason to
	//		recalculate the face dirs
	if( !bForce && (m_FaceDirRecipeUsed == m_pCharacter->m_FaceDirRecipeID) )
		return;

	//---	reset all of the face directions
	for( int i = 0 ; i < GetNumFrames() ; i++ )
		ResetFaceDir( i );

	//---	store the new recipe ID
	m_FaceDirRecipeUsed = m_pCharacter->m_FaceDirRecipeID;
}

//=============================================================================================
void CMotion::ResetAllMoveDir( void )
{
	if( !EnsureLoaded() ) return;

	for( int i = 0 ; i < GetNumFrames() ; i++ )
	{
		//---	Generate MoveDir
		vector3 	TS, TE ;
		vector3 	Diff ;

		//---	get the position of the current frame
		ANIM_GetTransValue( GetAnim(), i, &TS ) ;

		//---	we are going to get the difference between this frame and the next frame
		int j=i+1;

		//---	if the next frame doesn't exist, then start with the last frame
		if( j == GetNumFrames() )
			j = i-1;

		ASSERT(j>=0); // no motion can have less than 2 frames
		do
		{
			//---	get the difference between the frames and use that to set the move direction
			ANIM_GetTransValue( GetAnim(), j, &TE ) ;
			Diff = TE - TS;

			if( j < i )
			{
				Diff.X = -Diff.X;
				Diff.Y = -Diff.Y;
				Diff.Z = -Diff.Z;
			}

		} while((--j>=0) && ( Diff.X == 0.0f )&&( Diff.Z == 0.0f ));

		m_pMoveDir[i] = x_atan2( Diff.X, Diff.Z ) ;
	}
}

//=========================================================================
void CMotion::GetTranslation( vector3& rVector, s32 nFrame )
{
	anim *pAnim = GetAnim() ;
	if( pAnim )
		ANIM_GetTransValue( GetAnim(), nFrame, &rVector ) ;
	else
		rVector.Zero();
}

//=========================================================================
void CMotion::GetTranslationBlend( vector3& rVector, f32 nFrame )
{
	anim *pAnim = GetAnim() ;
	if( !pAnim )
	{
		rVector.Zero();
		return;
	}

	vector3	Translation;
	vector3	Translation1;
	int iFrame = (int)nFrame;

	//---	For the motion to wrap correctly, nFrame should not be passed in >= (NumFrames-1),
	//		however, when viewing the motions with the scroll bar, the motion may get set to
	//		NFrames-1.  In that case we would want Frame1 to be set to the same value as Frame0
	//		rather than wrapping it to the beginning.
	if( iFrame >= (GetNumFrames()-1) )
	{
		ANIM_GetTransValue( GetAnim(), iFrame, &rVector ) ;
		return;
	}

	ANIM_GetTransValue( GetAnim(), iFrame, &Translation ) ;
	ANIM_GetTransValue( GetAnim(), iFrame+1, &Translation1 ) ;

	float Frac = nFrame - (f32)iFrame;
	Translation.X += (Translation1.X-Translation.X)*Frac;
	Translation.Y += (Translation1.Y-Translation.Y)*Frac;
	Translation.Z += (Translation1.Z-Translation.Z)*Frac;

	rVector = Translation;
}

//=========================================================================
void CMotion::GetRotation( radian3& rRotation, int nFrame )
{
	rRotation	= m_pRootRotsSave[nFrame];
}

//=========================================================================
void CMotion::GetRotationBlend( radian3& rRotation, f32 nFrame )
{
	anim *pAnim = GetAnim() ;
	if( !pAnim )
	{
		rRotation.Zero();
		return;
	}

	radian3	Rot;
	radian3	Rot1;
	int iFrame = (int)nFrame;

	//---	For the motion to wrap correctly, nFrame should not be passed in >= (NumFrames-1),
	//		however, when viewing the motions with the scroll bar, the motion may get set to
	//		NFrames-1.  In that case we would want Frame1 to be set to the same value as Frame0
	//		rather than wrapping it to the beginning.
	if( iFrame >= (GetNumFrames()-1) )
	{
		GetRotation( rRotation, iFrame ) ;
		return;
	}

	GetRotation( Rot, iFrame );
	GetRotation( Rot1, iFrame+1 );

	float Frac = nFrame - (f32)iFrame;
	float dRot;
	
	dRot = Rot1.X - Rot.X;
	while( dRot > R_180 )	dRot -= R_360;
	while( dRot < -R_180 )	dRot += R_360;
	Rot.X = Rot.X + dRot*Frac;
	
	dRot = Rot1.Y - Rot.Y;
	while( dRot > R_180 )	dRot -= R_360;
	while( dRot < -R_180 )	dRot += R_360;
	Rot.Y = Rot.Y + dRot*Frac;
	
	dRot = Rot1.Z - Rot.Z;
	while( dRot > R_180 )	dRot -= R_360;
	while( dRot < -R_180 )	dRot += R_360;
	Rot.Z = Rot.Z + dRot*Frac;

	rRotation = Rot;
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
	Copy( *this, ptMotion );

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

		//---	NOTE:	As an optimization hack to speed up saving, the
		//				m_Anim.NFrames is assumed to be set correctly because
		//				it has been loaded into during the IsLoading() option
		//				below.  Since the m_pFaceDir and m_pMoveDir arrays are
		//				allocated based on the m_Anim.NFrames, the motion must
		//				have been loaded at one time for the arrays to exist.
		//				this prevents us from having to load the motion while
		//				saving out.
		if( m_pFaceDir )
		{
			s32 i = 0;
			a.WriteTaggeds32( IE_TAG_MOTION_FACEDIR, m_Anim.NFrames ) ;
			for( i = 0 ; i < m_Anim.NFrames; i++ )
			{
				a.Writef32(m_pFaceDir[i]) ;
			}
		}
		if( m_pMoveDir )
		{
			s32 i = 0;
			a.WriteTaggeds32( IE_TAG_MOTION_MOVEDIR, m_Anim.NFrames ) ;
			for( i = 0 ; i < m_Anim.NFrames; i++ )
			{
				a.Writef32(m_pMoveDir[i]) ;
			}
		}
		m_EventList.Serialize( a ) ;

		//---	Get Index for LinkCharacter and LinkMotion and Write those
		a.WriteTaggeds32( IE_TAG_MOTION_PROPS, MAX_NUM_PROPS );
		for( i=0; i<MAX_NUM_PROPS; i++ )
			m_Props[i].Serialize( a );

		//---	Write Flag Bits
		for( i=0 ; i<MOTION_FLAG_BITS; i++ )
			a.WriteTaggedbool( IE_TAG_MOTION_FLAGBIT, m_FlagBit[i] ) ; 

		//---	Write SkelPoint Check States
		for( i=0; i<m_SkelPointChecks.GetSize(); i++ )
			a.WriteTaggedbool( IE_TAG_MOTION_SKELPOINTCHECK, !!SkelPointGetCheck(i) ) ;

		//--- Write Auto Reset Face Direction toggle
		a.WriteTaggedbool( IE_TAG_MOTION_AUTORESET_FACEDIR, (bool)m_bAutoResetFaceDir );

		//--- Write Auto Reset Move Direction toggle
		a.WriteTaggedbool( IE_TAG_MOTION_AUTORESET_MOVEDIR, (bool)m_bAutoResetMoveDir );

		//--- Export
		a.WriteTaggeds32( IE_TAG_MOTION_EXPORT_FLAG, m_bExport );

		//---	Fractional frames information
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_FRAMERATE_ON, m_bExportFrameRate );
		a.WriteTaggedf32( IE_TAG_MOTION_EXPORT_FRAMERATE, m_ExportFrameRate );

		//---	export bits
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_BITS_ON, m_bExportBits );
		a.WriteTaggeds32( IE_TAG_MOTION_EXPORT_BITS, m_ExportBits );

		//---	export flags
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_DEFAULT, m_bExportDefault );
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_QUATERNION, m_bExportQuaternion );
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_QUATWVALUE, m_bExportQuatW );
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_WORLDROTS, m_bExportWorldRots );
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_GLOBALROTS, m_bExportGlobalRots );
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_QUATBLEND, m_bExportQuatBlending );
		a.WriteTaggedbool( IE_TAG_MOTION_EXPORT_PROPQUATBLEND, m_bExportPropQuatBlending );

		//---	blended motion data
		a.WriteTaggeds32( IE_TAG_MOTION_BLEND_COUNT, m_BlendedMotionCount );
		a.Writes32( 1 ); // version of the structure being saved
		for( i=0; i<m_BlendedMotionCount; i++ )
		{
			a.Writes32( m_pCharacter->MotionToExportIndex( m_BlendedMotions[i].pMotion ) );
			a.Writef32( m_BlendedMotions[i].Ratio );
		}

		a.WriteTaggedbool( IE_TAG_MOTION_BLEND_EXPORTFORMULA, m_bExportBlendFormula );

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_MOTION_END ) ;
	}
	else
	{
		//---	Load Motion
		s32		iFlagBit = 0 ;
		s32		BlendStructVersion;
		s32		SkelPointIndex = 0 ;
		s32		nFrames;
		s32		Count;
		BOOL	done = FALSE ;
		bool	b;

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
				a.Reads32(nFrames) ;
				m_Anim.NFrames = nFrames;
				m_FaceDirArrayLength = nFrames;	// grab this as well
				if( nFrames > 0 )
				{
					m_pFaceDir = (f32*)x_malloc( nFrames * sizeof(f32) ) ;
					ASSERT( m_pFaceDir ) ;
					a.m_rType = IE_TYPE_f32 ;
					a.m_rLength = 4 ;
					for( i = 0 ; i < nFrames ; i++ )
						a.Readf32(m_pFaceDir[i]) ;
				}
				else
					m_pFaceDir = NULL;
				break ;

			case IE_TAG_MOTION_MOVEDIR:
				a.Reads32(nFrames) ;
				m_Anim.NFrames = nFrames;
				m_FaceDirArrayLength = nFrames;	// grab this as well
				if( nFrames > 0 )
				{
					m_pMoveDir = (f32*)x_malloc( nFrames * sizeof(f32) ) ;
					ASSERT( m_pMoveDir ) ;
					a.m_rType = IE_TYPE_f32 ;
					a.m_rLength = 4 ;
					for( i = 0 ; i < nFrames ; i++ )
						a.Readf32(m_pMoveDir[i]) ;
				}
				else
					m_pMoveDir = NULL;
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
				m_pLinkCharacter_OldSystem = (CCharacter*)(Index+1) ;
				break ;

			case IE_TAG_MOTION_LINKMOTION:
				a.Reads32( Index ) ;
				m_pLinkMotion_OldSystem = (CMotion*)(Index+1) ;
				break ;

			case IE_TAG_MOTION_EXPORT_LINK:
				a.Readbool( m_bExportLink_OldSystem );
				break;

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
				a.Readbool( b );
				if( b )
					m_bExport = 1;
				else
					m_bExport = 0;
				break;

			case IE_TAG_MOTION_EXPORT_FLAG:
				a.Reads32( m_bExport );
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

			case IE_TAG_MOTION_EXPORT_DEFAULT:
				a.Readbool( m_bExportDefault );
				break;

			case IE_TAG_MOTION_EXPORT_WORLDROTS:
				a.Readbool( m_bExportWorldRots );
				break;

			case IE_TAG_MOTION_EXPORT_GLOBALROTS:
				a.Readbool( m_bExportGlobalRots );
				break;

			case IE_TAG_MOTION_EXPORT_QUATERNION:
				a.Readbool( m_bExportQuaternion );
				break;

			case IE_TAG_MOTION_EXPORT_QUATBLEND:
				a.Readbool( m_bExportQuatBlending );
				break;

			case IE_TAG_MOTION_EXPORT_PROPQUATBLEND:
				a.Readbool( m_bExportPropQuatBlending );
				break;

			case IE_TAG_MOTION_PROPS:
				a.Reads32( Count );
				for( i=0; i<Count; i++ )
				{
					if( i<MAX_NUM_PROPS )
						m_Props[i].Serialize( a );
					else
					{
						CPropData P;
						P.Serialize( a );
					}
				}
				break;

			case IE_TAG_MOTION_BLEND_COUNT:
				a.Reads32( m_BlendedMotionCount );
				a.Reads32( BlendStructVersion );
				for( i=0; i<m_BlendedMotionCount; i++ )
				{
					a.Reads32( *((s32*)&m_BlendedMotions[i].pMotion) );
					a.Readf32( m_BlendedMotions[i].Ratio );
				}
				break;

			case IE_TAG_MOTION_BLEND_EXPORTFORMULA:
				a.Readbool( m_bExportBlendFormula );
				break;

			case IE_TAG_MOTION_EXPORT_QUATWVALUE:
				a.Readbool( m_bExportQuatW );
				break;

			// no longer used, but don't remove because the values must remain constant
			case IE_TAG_MOTION_EXPORT_QUATBITS_ON:
				a.Readbool( b );
				break;

			// no longer used, but don't remove because the values must remain constant
			case IE_TAG_MOTION_EXPORT_QUATBITS:
				a.Reads32( Count );
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

//==========================================================================
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

//==========================================================================
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

//==========================================================================
bool CMotion::GetExportQuaternion( void )
{
	if( m_bExportDefault )
		return m_pCharacter->m_bExportQuaternion;

	return m_bExportQuaternion;
}

//==========================================================================
bool CMotion::GetExportQuatW( void )
{
	if( m_bExportDefault )
		return (m_pCharacter->m_bExportQuaternion && m_pCharacter->m_bExportQuatW) ? TRUE : FALSE;

	return (m_bExportQuaternion && m_bExportQuatW) ? TRUE : FALSE;
}

//==========================================================================
bool CMotion::GetExportWorldRots( void )
{
	if( m_bExportDefault )
		return m_pCharacter->m_bExportWorldRots;

	return m_bExportWorldRots;
}

//==========================================================================
bool CMotion::GetExportGlobalRots( void )
{
	if( m_bExportDefault )
		return m_pCharacter->m_bExportGlobalRots;

	return m_bExportGlobalRots;
}

//==========================================================================
bool CMotion::GetExportQuatBlend( void )
{
	if( m_bExportDefault )
		return m_pCharacter->m_bExportQuatBlending;

	return m_bExportQuatBlending;
}

//==========================================================================
bool CMotion::GetExportPropQuatBlend( void )
{
	if( m_bExportDefault )
		return m_pCharacter->m_bExportPropQuatBlending;

	return m_bExportPropQuatBlending;
}

//==========================================================================
radian3 BlendXYZUsingQuaternions( const radian3& FromXYZ, const radian3& ToXYZ, f32 t )
{
    quaternion  Q1;
    quaternion  Q2;
    quaternion  Quat;
    matrix4     M4;
    radian3     result;

    ASSERT( (t >= 0.0f) && (t <= 1.0f) );

	//---	I have seen a radian3 which when converted to a quaternion using SetupRotateXYZ() and then converted back
	//		using BuildMatrix() and matrix4::GetRotationXYZ() was changed to a different angle.  I found another way around it
	//		for now, however this should be addressed at some point.  --- Kevin.

	Q1.SetupRotateXYZ( FromXYZ );
	Q2.SetupRotateXYZ( ToXYZ );

	Quat = BlendLinear( Q1, Q2, t );

    //---   get the angles out of the quaternion
	Quat.BuildMatrix( M4 );
	result = M4.GetRotationXYZ();

    return result;
}

//==========================================================================
f32 CMotion::ResampleAnimation( anim& rResampAnim, s32 NewNumFrames )
{
    s16*		RawData;
	s32			RawDataSize;
	s16			Value ;
	s16			NextValue;
	s32			WholeFrame;
	f32			FracFrame;
	f32			nFrame;
	s32			ResampFrame;
	s32			LastFrame;
	s32			dDir;
	s32			NBoneStreams = GetAnim()->NBones*3;
	radian3		RotA;
	radian3		RotB;
	radian3		RotI;

	//---	get a buffer for resampling
	ANIM_InitInstance( &rResampAnim );
	ANIM_CopyAnimHeader( &rResampAnim, GetAnim() );
	ASSERT(NewNumFrames > 0);
	rResampAnim.NFrames		= NewNumFrames;
	rResampAnim.Stream		= (s16*) x_malloc( NewNumFrames*rResampAnim.NStreams*sizeof(s16) );

    /////////////////////////////////////////////////////////////
	//---	Loop through all Streams and load into RawData
    /////////////////////////////////////////////////////////////
	//---	get the exported frame rate, round it off the same way that it is exported below
	s32 NStreams = GetAnim()->NStreams;
	s32 CurResampFrame;
	LastFrame		= GetAnim()->NFrames-1;
	f32 FrameStep	= (f32)(GetAnim()->NFrames-1)/(f32)(NewNumFrames-1);
	RawData = rResampAnim.Stream;
	RawDataSize = 0;
	for( int cStream = 0 ; cStream < NBoneStreams ; cStream+=3 )
	{
		//---	Loop through All Frames in this Block and load into RawData
		nFrame = 0.0f;
		CurResampFrame = 0;
		for( ResampFrame = 0 ; ResampFrame < NewNumFrames ; ++ResampFrame, nFrame += FrameStep )
		{
			WholeFrame = (s32)nFrame;
			FracFrame = nFrame - (f32)WholeFrame;

			if( WholeFrame >= LastFrame )
			{
				WholeFrame = LastFrame-1;
				FracFrame = 1.0f;
			}

			//---	interpolate this frame
			RotA.Pitch	= ANIM_GetRotation( GetAnim(), WholeFrame, cStream );
			RotA.Yaw	= ANIM_GetRotation( GetAnim(), WholeFrame, cStream+1 );
			RotA.Roll	= ANIM_GetRotation( GetAnim(), WholeFrame, cStream+2 );
			RotB.Pitch	= ANIM_GetRotation( GetAnim(), WholeFrame+1, cStream );
			RotB.Yaw	= ANIM_GetRotation( GetAnim(), WholeFrame+1, cStream+1 );
			RotB.Roll	= ANIM_GetRotation( GetAnim(), WholeFrame+1, cStream+2 );

			RotI = BlendXYZUsingQuaternions( RotA, RotB, FracFrame );

			ANIM_SetRotation( &rResampAnim, CurResampFrame, cStream+0, RotI.Pitch );
			ANIM_SetRotation( &rResampAnim, CurResampFrame, cStream+1, RotI.Yaw );
			ANIM_SetRotation( &rResampAnim, CurResampFrame, cStream+2, RotI.Roll );

			RawDataSize+=3;
			CurResampFrame++;
		}
	}

	//---	handle the rest of the streams
	for( cStream = NBoneStreams ; cStream < NStreams ; cStream++ )
	{
		nFrame = 0.0f;
		for( ResampFrame = 0 ; ResampFrame < NewNumFrames ; ++ResampFrame, nFrame += FrameStep )
		{
			WholeFrame = (s32)nFrame;
			FracFrame = nFrame - (f32)WholeFrame;

			if( WholeFrame >= LastFrame )
			{
				WholeFrame = LastFrame-1;
				FracFrame = 1.0f;
			}

			if( ANIM_StreamIsRotation( GetAnim(), cStream ) )
			{
				ANIM_GetStreamValue( GetAnim(), WholeFrame, cStream, &Value );
				ANIM_GetStreamValue( GetAnim(), WholeFrame+1, cStream, &NextValue );

				dDir = NextValue - Value;
				while( dDir >  ANGLE_180 )	dDir -= ANGLE_360;
				while( dDir <= -ANGLE_180 )	dDir += ANGLE_360;
				Value = Value + (s16)((f32)dDir*FracFrame+0.5f);
				Value &= ANGLE_360-1;
			}
			else
			{
				ANIM_GetStreamValue( GetAnim(), WholeFrame, cStream, &Value );
				ANIM_GetStreamValue( GetAnim(), WholeFrame+1, cStream, &NextValue );
				Value = Value + (s16)(FracFrame*(NextValue - Value));
			}

			RawData[RawDataSize++] = Value;
		}
	}

	//---	return the resample rate used
	//		NOTE: The resample rate is not exactly the rate requested.
	//		To keep the first and last frame the rate had to be adjusted.
	m_FrameRate = 60.0f*(NewNumFrames-1)/(m_nFrames60-1);
	return m_FrameRate;
}

//////////////////////////////////////////////////////////////////////
f32 CMotion::ResampleAnimation( anim& rResampAnim, f32 FrameRate )
{
	s32 ResampleFrames;

	//---	determine the number of resampled frames
	ResampleFrames = (s32)((FrameRate*GetAnim()->NFrames/60.0f)+0.5f);
	if( ResampleFrames < 2 )
		ResampleFrames = 2;

	//---	if the number of resampled frames is equal to the current number of frames,
	//		return now.
	if( ResampleFrames == GetAnim()->NFrames )
	{
		ANIM_CopyAnim( &rResampAnim, GetAnim() );
		m_FrameRate = 60.0f*(ResampleFrames-1)/(m_nFrames60-1);
		return m_FrameRate;
	}

	return ResampleAnimation( rResampAnim, ResampleFrames );
}

//////////////////////////////////////////////////////////////////////
void CMotion::MimicFinalExportQuantizationResult( anim &Anim, s32 ExportBits )
{
    s32			i, j;
	radian3		Rot;
	vector3		Pos;
	quaternion	Quat;
	matrix4		Mat;

    //---   Get duplicate of animation for processing
    CSkeleton *pSkeleton = m_pCharacter->GetSkeleton() ;

    matrix4 DestM[50];
    matrix4 WorldOrientM;
	f32		ExportAngleConversion	= ((f32)(1<<ExportBits))/R_360;
	f32		ExportQuatConversion	= (f32)( (1L<<ExportBits)/2 - 1 );
	f32		ExportQuatWConversion	= (f32)( (1L<<ExportBits) - 1 );
	f32		OneOverExportQuatConversion;
	f32		OneOverExportAngleConversion;
	f32		OneOverExportQuatWConversion;

	if( ExportAngleConversion == 0.0f )	OneOverExportAngleConversion	= 0.0f;
	else								OneOverExportAngleConversion	= 1.0f/ExportAngleConversion;
	if( ExportQuatConversion == 0.0f )	OneOverExportQuatConversion		= 0.0f;
	else								OneOverExportQuatConversion		= 1.0f/ExportQuatConversion;
	if( ExportQuatWConversion == 0.0f )	OneOverExportQuatWConversion	= 0.0f;
	else								OneOverExportQuatWConversion	= 1.0f/ExportQuatWConversion;

	ASSERT(Anim.NBones<=50);

    //---   Build WorldOrientM
    WorldOrientM.Identity();

    //---   Build matrices and compute global XZY rotations
    for (i=0; i<Anim.NFrames; i++)
    {
        // Pull global XYZ rotations from matrix and convert to global XZY
        for (j=0; j<Anim.NBones; j++)
        {
            // Place new rotation in animation
			Rot.X = ANIM_GetRotation( &Anim, i, j*3+0 );
			Rot.Y = ANIM_GetRotation( &Anim, i, j*3+1 );
			Rot.Z = ANIM_GetRotation( &Anim, i, j*3+2 );

			if( GetExportQuaternion() )
			{
				Quat.SetupRotateXYZ( Rot );

				if( Quat.W < 0.0f )
					Quat.Negate();

				Quat.Normalize();

				//---	mimic compression on the quaternion.
				Quat.X = (((f32)((s32)((Quat.X*ExportQuatConversion)+ExportQuatConversion)))-ExportQuatConversion)*OneOverExportQuatConversion;
				Quat.Y = (((f32)((s32)((Quat.Y*ExportQuatConversion)+ExportQuatConversion)))-ExportQuatConversion)*OneOverExportQuatConversion;
				Quat.Z = (((f32)((s32)((Quat.Z*ExportQuatConversion)+ExportQuatConversion)))-ExportQuatConversion)*OneOverExportQuatConversion;

				//---	recalculate W based on the X, Y, Z values so that the quat's length will still be 1.0 even with quantization
				Quat.W = (f32)sqrt( 1.0f - (Quat.X*Quat.X + Quat.Y*Quat.Y + Quat.Z*Quat.Z) );

				//---	if the W value is going to be exported, quantize it now
				if( this->GetExportQuatW() )
					Quat.W = ((f32)((s32)(Quat.W*ExportQuatWConversion)))*OneOverExportQuatWConversion;

				Quat.BuildMatrix( Mat );
				Rot = Mat.GetRotationXYZ();
			}
			else
			{
				//---	shift the values by the bit compression
				Rot.X = ((s32)(Rot.X * ExportAngleConversion))*OneOverExportAngleConversion;
				Rot.Y = ((s32)(Rot.Y * ExportAngleConversion))*OneOverExportAngleConversion;
				Rot.Z = ((s32)(Rot.Z * ExportAngleConversion))*OneOverExportAngleConversion;
			}

            // Place new rotation in animation
			ANIM_SetRotation( &Anim, i, j*3+0, Rot.X );
			ANIM_SetRotation( &Anim, i, j*3+1, Rot.Y );
			ANIM_SetRotation( &Anim, i, j*3+2, Rot.Z );
        }

		// quantize the position value (should have very little affect)
		ANIM_GetTransValue( &Anim, i, &Pos );
		Pos.Scale( 16.0f );
		Pos.X = (f32)((s16)Pos.X);
		Pos.Y = (f32)((s16)Pos.Y);
		Pos.Z = (f32)((s16)Pos.Z);
		Pos.Scale( 1.0f / 16.0f );
		ANIM_SetTransValue( &Anim, i, &Pos );
    }

    //---   Minimize deltas in rotation streams for maximum compression
    for (i=0; i<Anim.NBones*3; i++)
    {
        ANIM_MinimizeAngleDiff( &Anim, i, ANGLE_360 );
    }
}


//////////////////////////////////////////////////////////////////////

void CMotion::MakeIntoCompressedVersion( xbool bMimicBitCompression )
{
    anim	Anim;
	bool	RotationsConvertedToXZY=FALSE;
	s32		TotalAnimRawDataSize=0;
	s32		i;

#ifndef INTERPOLATE_ROOT_SAVES
	//---	lets put the original root rotations back into the animation before compressing it.
	for( i=0; i<m_Anim.NFrames; i++ )
	{
		ANIM_SetRotation( &m_Anim, i, 0, m_pRootRotsSave[i].X );
		ANIM_SetRotation( &m_Anim, i, 1, m_pRootRotsSave[i].Y );
		ANIM_SetRotation( &m_Anim, i, 2, m_pRootRotsSave[i].Z );
	}
#endif

    //---   Get the resampled animation for processing
    ResampleAnimation( Anim, GetExportFrameRate() );

	//---	mimic the final
	if( bMimicBitCompression )
		MimicFinalExportQuantizationResult( Anim, GetExportBits() );

    /////////////////////////////////////////////////////////////
	//---	Keep Record of beginning of Data in File
    /////////////////////////////////////////////////////////////

	//---	resample any prop data to match the new number of frames of the motion
	for( i=0; i<MAX_NUM_PROPS; i++ )
		m_Props[i].ResampleProp( GetAnim()->NFrames, Anim.NFrames );

	//---	set the new number of frames into the animation
	GetAnim()->NFrames = Anim.NFrames;
	s16* t = GetAnim()->Stream;
	GetAnim()->Stream = Anim.Stream;
	Anim.Stream = t;

	//---	make sure the new motion's face/move dirs fit
	ResetFaceMoveDirsForNewMotion();

	EnsureFaceDirs();
	EnsureMoveDirs();
	ASSERT(m_pFaceDir);
	ASSERT(m_pMoveDir);
	if( bMimicBitCompression )
	{
		//---	process each of the frame's FaceDir/MoveDir to
		//		make them reflect the bit compression
		f32		ExportAngleConversion = ((f32)(1<<GetExportBits()))/R_360;
		f32		OneOverExportAngleConversion;
		if( ExportAngleConversion == 0.0f )
			OneOverExportAngleConversion = 0.0f;
		else
			OneOverExportAngleConversion = 1.0f/ExportAngleConversion;

		for( i=0; i<GetNumFrames(); i++ )
		{
			m_pFaceDir[i] = ((s32)(m_pFaceDir[i] * ExportAngleConversion))*OneOverExportAngleConversion;
			m_pMoveDir[i] = ((s32)(m_pMoveDir[i] * ExportAngleConversion))*OneOverExportAngleConversion;
		}

		//---	mimic the prop stream's export quantization
		for( i=0; i<MAX_NUM_PROPS; i++ )
			m_Props[i].MimicFinalExportQuantizationResult();
	}

	//---	set the export parameters which were active for this motion at the time it was compressed.
	m_ExportBits = GetExportBits();
	m_bExportBits = true;

	m_ExportFrameRate = GetExportFrameRate();
	m_bExportFrameRate = true;

	//---	store the number of frames for this animation so that when the next one is laoded, the new animation
	//		will know how many of the MoveDir/FaceDir values should be saved.
	m_FaceDirArrayLength = GetNumFrames();

	//---	remove the temporary animation buffer
    ANIM_KillInstance (&Anim);
}


///////////////////////////////////////////////////////////////////////////////
void CMotion::ResetFaceMoveDirsForNewMotion( void )
{
#ifndef INTERPOLATE_ROOT_SAVES
	//---	cause the m_pFaceDir, m_pMoveDir, m_pRootRotsSave arrays to be updated.
	x_free( m_pRootRotsSave );
	m_pRootRotsSave = NULL;
	m_FaceDirRecipeUsed = -2;

	//---	force the update right now, why not?
	GetFaceDir(0);
#endif

	s32	NewNumFrames = m_Anim.NFrames;

	if( NewNumFrames == m_FaceDirArrayLength )
		return;

#ifdef INTERPOLATE_ROOT_SAVES
	ASSERT(NewNumFrames>0);
	radian3* pRootRotsSave = (radian3*) x_malloc( NewNumFrames*sizeof(radian3) );
#endif
	f32*	pFaceDirs = (f32*) x_malloc( NewNumFrames*sizeof(f32) );
	f32*	pMoveDirs = (f32*) x_malloc( NewNumFrames*sizeof(f32) );

	f32		fStep = (m_FaceDirArrayLength-1) / (f32)(NewNumFrames-1);
	f32		fFrame;
	s32		WholeFrame, i;
	f32		FracFrame;
	f32		v0, v1, d;
	for( i=0, fFrame=0; i<NewNumFrames; i++, fFrame+=fStep )
	{
		WholeFrame	= (s32)fFrame;
		FracFrame	= fFrame - WholeFrame;

		if( WholeFrame >= (m_FaceDirArrayLength-1) )
		{
#ifdef INTERPOLATE_ROOT_SAVES
			pRootRotsSave[i] = m_pRootRotsSave[WholeFrame];
#endif
			pFaceDirs[i] = m_pFaceDir[WholeFrame];
			pMoveDirs[i] = m_pMoveDir[WholeFrame];
		}
		else
		{
#ifdef INTERPOLATE_ROOT_SAVES
			//---	interpolate the root saves values.  (this is better than re-calculating them because it keeps the animation faced foreward)
			radian3 r0 = m_pRootRotsSave[WholeFrame];
			radian3 r1 = m_pRootRotsSave[WholeFrame+1];
			pRootRotsSave[i] = BlendXYZUsingQuaternions( r0, r1, FracFrame );
#endif

			//---	interpolate face dir
			v0	= m_pFaceDir[WholeFrame];
			v1	= m_pFaceDir[WholeFrame+1];
			d = v1-v0;
			while( d>R_180 ) d-=R_360;
			while( d<-R_180) d+=R_360;
			pFaceDirs[i] = v0 + FracFrame*d;

			//---	interpolate move dir
			v0	= m_pMoveDir[WholeFrame];
			v1	= m_pMoveDir[WholeFrame+1];
			d = v1-v0;
			while( d>R_180 ) d-=R_360;
			while( d<-R_180) d+=R_360;
			pMoveDirs[i] = v0 + FracFrame*d;
		}
	}

#ifdef INTERPOLATE_ROOT_SAVES
	x_free( m_pRootRotsSave );
	m_pRootRotsSave = pRootRotsSave;
#endif

	//---
	//
	//	Set the newly interpolated face dirs
	//
	//---
	x_free( m_pFaceDir );
	m_pFaceDir = pFaceDirs;

	x_free( m_pMoveDir );
	m_pMoveDir = pMoveDirs;

	//---
	//
	//	invalidate the face/move dirs of this motion, if they are provided by the user they will be kept as the interpolated ones,
	//	if they are automatic they will be reset to be recalculated when needed
	//
	//---
	InvalidateFaceDirs();
	InvalidateMoveDirs();

	//---	store the number of frames for this animation so that when the next one is laoded, the new animation
	//		will know how many of the MoveDir/FaceDir values should be saved.
	m_FaceDirArrayLength = GetNumFrames();
}

///////////////////////////////////////////////////////////////////////////////
CMotion* CMotion::GetCompressedVersion( bool bMimicBitCompression )
{
	s32 i;

	//---	make sure that if any changes have occured, the motion has been updated
	CheckUpdateMotionVersion();

	//---	if there is already a compressed version, make sure that it is the right compressed format and then return it
	//		otherwise it will have to be regenerated
	if( IsBlendedMotion() && IsExportBlendFormula() )
	{
		if( m_pCompVersion )
		{
			for( i=0; i<m_BlendedMotionCount; i++ )
				if( !m_BlendedMotions[i].pMotion->CheckCompressedVersion() )
					break;

			if( i==m_BlendedMotionCount )
				return m_pCompVersion;
		}
	}
	else if( m_pCompVersion && CheckCompressedVersion() )
	   return m_pCompVersion;

	//---	it has changed since the last time it was compressed, delete it to be recompressed
	delete m_pCompVersion;
	m_pCompVersion = new CMotion;
	*m_pCompVersion = *this;

	if( IsExportBlendFormula() )
		UpdateCompressedMotionVersion( bMimicBitCompression );
	else
		m_pCompVersion->MakeIntoCompressedVersion( bMimicBitCompression );

	//---	return the compressed version
	return m_pCompVersion;
}

///////////////////////////////////////////////////////////////////////////////
void CMotion::ForceCompressedVersionUpdate( void )
{
	if( m_pCompVersion )
		delete m_pCompVersion;

	m_pCompVersion = NULL;
}

///////////////////////////////////////////////////////////////////////////////
s32 CMotion::CountNumExportedStreams( s32 PropsUsed )
{
	s32 i;

	if( !GetExportQuaternion() )
		return m_Anim.NStreams;

	s32 nStreams = m_Anim.NStreams;

	//---	one extra stream per bone
	nStreams += m_Anim.NBones;

	//---	one extra stream per attached prop
	for( i=0; i<MAX_NUM_PROPS; i++ )
		if( PropsUsed&(1<<i) )
			nStreams++;

	return nStreams;
}

///////////////////////////////////////////////////////////////////////////////
void CMotion::UpdateMotionVersion( void )
{
	bool bBlended;
	s32 i;

	//---	if there are no blended motions, there is nothing to do so get out fast.
	if( m_BlendedMotionCount == 0 )
	{
		m_LastUpdateRuntimeVersion = m_RuntimeVersion;
		return;
	}

	//---	flag that we are building the motion
	m_bInUpdateMotionVersion	= TRUE;

	//---	loop through the rest of the blended motions and blend them in as it goes.
	f32	TotalBlend	= 0.0f;
	for( i=0; i<m_BlendedMotionCount; i++ )
	{
		TotalBlend += m_BlendedMotions[i].Ratio;

		//---	only blend if there is something to blend
		if( m_BlendedMotions[i].Ratio > 0.0f )
		{
			bBlended = BlendMotion( m_BlendedMotions[i].pMotion,
									m_BlendedMotions[i].Ratio / TotalBlend );
		}

		//---	the only reason that a motion couldn't be blended is if it couldn't be loaded,
		//		so return early
		if( !bBlended )
		{
			m_IsLoaded = FALSE;
			return;
		}
	}

	//---	reset the runtime versions of the motions which were used.
	for( i=0; i<m_BlendedMotionCount; i++ )
	{
		if( m_BlendedMotions[i].pMotion )
			m_BlendedMotions[i].RuntimeVersion = m_BlendedMotions[i].pMotion->GetRuntimeVersion();
		else
			m_BlendedMotions[i].RuntimeVersion = -1;
	}

	//---	adjust all of the root rotations so that they are relative to the face directions
	FrameAdjustRootRots();

	//---	reset the runtime version
	m_LastUpdateRuntimeVersion = ++m_RuntimeVersion;

	m_IsLoaded = TRUE;

	//---	flag that we are done building the motion
	m_bInUpdateMotionVersion	= FALSE;
}

///////////////////////////////////////////////////////////////////////////////
void CMotion::UpdateCompressedMotionVersion( bool bMimicBitCompression )
{
	bool bBlended;
	s32 i;

	//---	if there are no blended motions, there is nothing to do so get out fast.
	if( m_BlendedMotionCount == 0 )
	{
		m_LastUpdateRuntimeVersion = m_RuntimeVersion;
		return;
	}

	//---	flag that we are building the motion
	m_pCompVersion->m_bInUpdateMotionVersion	= TRUE;

	//---	loop through the rest of the blended motions and blend them in as it goes.
	f32	TotalBlend	= 0.0f;
	for( i=0; i<m_BlendedMotionCount; i++ )
	{
		TotalBlend += m_BlendedMotions[i].Ratio;
		if( m_BlendedMotions[i].Ratio > 0.0f )
		{
			bBlended = m_pCompVersion->BlendMotion(
							m_BlendedMotions[i].pMotion->GetCompressedVersion( bMimicBitCompression ),
							m_BlendedMotions[i].Ratio / TotalBlend );
		}

		//---	the only reason that a motion couldn't be blended is if it couldn't be loaded,
		//		so return early
		if( !bBlended )
		{
			m_IsLoaded = FALSE;
			return;
		}
	}

	//---	flag that we are done building the motion
	m_pCompVersion->m_bInUpdateMotionVersion	= FALSE;
}

///////////////////////////////////////////////////////////////////////////////
bool CMotion::BlendMotion( CMotion* pMotion, f32 fRatio )
{
	//---	if the animation is loaded, copy it over.
	if( !pMotion->EnsureLoaded() )
		return FALSE;

	ASSERT( pMotion->m_IsLoaded );

	//---	if the ratio is 1.0, copy the motion and return
	if( fRatio == 1.0f )
	{
		//---	restore the root rotations
		RestoreRootRots();
		pMotion->RestoreRootRots();

		ANIM_CopyAnim( &m_Anim, &pMotion->m_Anim );
		
		//---	copy CMotion variables which must be duplicated
		m_nFrames60 = pMotion->m_nFrames60;
		m_FrameRate = pMotion->m_FrameRate;

		//---	invalidate the face/move dirs of this motion
		InvalidateFaceDirs();
		InvalidateMoveDirs();

		pMotion->SaveRootRots();
	}
	else
	{
		anim	RotatedAnim;
		anim*	pBlendAnim;

		//---	Because often animations are facing different directions for no real reason, all motions which are merged into the first
		//		motion will be rotated so that they start the same direction as the original motion.  This is for convineince.  Calculate
		//		the yaw difference to be passed into a blended animation
		f32 dYaw = GetFaceDir( 0 ) - pMotion->GetFaceDir( 0 );
		while( dYaw > R_180 )	dYaw -= R_360;
		while( dYaw < -R_180 )	dYaw += R_360;

		if( x_abs( dYaw ) > 0.001f )
		{
			ANIM_InitInstance( &RotatedAnim );
			ANIM_CopyAnim( &RotatedAnim, &pMotion->m_Anim );
			RotateAnimation( RotatedAnim, dYaw );
			pBlendAnim	= &RotatedAnim;
		}
		else
			pBlendAnim	= &pMotion->m_Anim;

		//---	restore the root rotations
		RestoreRootRots();
		pMotion->RestoreRootRots();

		BlendAnimation( m_Anim, *pBlendAnim, fRatio );

		if( pBlendAnim == &RotatedAnim )
			ANIM_KillInstance( &RotatedAnim );

		//---	invalidate the face/move dirs of this motion
		InvalidateFaceDirs();
		InvalidateMoveDirs();

		pMotion->SaveRootRots();
	}
	
	//---	blend the prop data
	for( s32 i=0; i<MAX_NUM_PROPS; i++ )
		m_Props[i].MergeProp( pMotion->m_Props[i], fRatio );

	//---	if there was a compressed version, force it to update itself now
	ForceCompressedVersionUpdate();

	//---	recalculate the new number of fremes that the motion has when played back at 60 fraems per second
	//		this number is related to the original number of frames that the motions had when loaded from the
	//		drive and doesn't change from that number even when motions are compressed to fewer actual data frames.
	if( m_nFrames60 != pMotion->m_nFrames60 )
		m_nFrames60 = (s32) ((((1.0f-fRatio)*(f32)m_nFrames60) + (fRatio*pMotion->m_nFrames60)) + 0.5f);

	//---	calculate the new frame rate required to get the motion playing at the intended rate
	m_FrameRate = 60.0f*(m_Anim.NFrames-1)/(m_nFrames60-1);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
s32 CMotion::BlendAnimation( anim& rAnimDst, anim& rAnimSrc, f32 fRatio )
{
	f32		fRatio0			= 1.0f - fRatio;
	f32		fRatio1			= fRatio;
	s32		NewNumFrames;
	anim	Anim0;
	anim	Anim1;
	s16		Value;
	s16		Value0;
	s16		Value1;
	radian3	RotA;
	radian3	RotB;
	radian3	RotI;
	s32		dDir;

	ANIM_InitInstance( &Anim0 );
	ANIM_InitInstance( &Anim1 );

	//---
	//
	//	Copy the two animations before processing them.
	//
	//---
	ANIM_CopyAnim( &Anim0, &rAnimDst );
	ANIM_CopyAnim( &Anim1, &rAnimSrc );

	//---
	//
	//	If the number of frames between the two motions is different, calculate a new number of frames for the motions
	//
	//---
	if( rAnimDst.NFrames != rAnimSrc.NFrames )
	{
		NewNumFrames	= (s32) (((f32)rAnimDst.NFrames*fRatio0) + ((f32)rAnimSrc.NFrames*fRatio1));

		if( Anim0.NFrames != NewNumFrames )
			ResampleAnimation( Anim0, NewNumFrames );

		if( Anim1.NFrames != NewNumFrames )
			ResampleAnimation( Anim1, NewNumFrames );
	}
	else
		NewNumFrames	= Anim0.NFrames;

	//---	make sure that the destination animation has room for the new number of frames
	if( rAnimDst.NFrames != NewNumFrames )
		ANIM_ChangeNumFrames( &rAnimDst, NewNumFrames );

	//---
	//
	//	now that we are sure to have two motions which are the same length, blend them together into a new motion
	//
	//---
	for( int cStream=0; cStream<Anim0.NBones*3; cStream+=3 )
	{
		for( int cFrame=0; cFrame<NewNumFrames; ++cFrame )
		{
			//---	interpolate this frame
			RotA.Pitch	= ANIM_GetRotation( &Anim0, cFrame, cStream+0 );
			RotA.Yaw	= ANIM_GetRotation( &Anim0, cFrame, cStream+1 );
			RotA.Roll	= ANIM_GetRotation( &Anim0, cFrame, cStream+2 );
			RotB.Pitch	= ANIM_GetRotation( &Anim1, cFrame, cStream+0 );
			RotB.Yaw	= ANIM_GetRotation( &Anim1, cFrame, cStream+1 );
			RotB.Roll	= ANIM_GetRotation( &Anim1, cFrame, cStream+2 );

			RotI = BlendXYZUsingQuaternions( RotA, RotB, fRatio1 );

			ANIM_SetRotation( &rAnimDst, cFrame, cStream+0, RotI.Pitch );
			ANIM_SetRotation( &rAnimDst, cFrame, cStream+1, RotI.Yaw );
			ANIM_SetRotation( &rAnimDst, cFrame, cStream+2, RotI.Roll );
		}
	}

	//---	handle the rest of the streams
	for( cStream=Anim0.NBones; cStream<Anim0.NStreams; cStream++ )
	{
		for( int cFrame=0; cFrame<NewNumFrames; ++cFrame )
		{
			//---	if this is a rotation stream, we must make sure that we wrap properly
			//		otherwise, interpolate normally
			if( ANIM_StreamIsRotation( &Anim0, cStream ) )
			{
				ANIM_GetStreamValue( &Anim0, cFrame, cStream, &Value0 );
				ANIM_GetStreamValue( &Anim1, cFrame, cStream, &Value1 );

				dDir = Value1 - Value0;
				while( dDir >  ANGLE_180 )	dDir -= ANGLE_360;
				while( dDir <= -ANGLE_180 )	dDir += ANGLE_360;
				Value = Value0 + (s16)((f32)dDir*fRatio1+0.5f);
				Value &= ANGLE_360-1;
			}
			else
			{
				ANIM_GetStreamValue( &Anim0, cFrame, cStream, &Value0 );
				ANIM_GetStreamValue( &Anim1, cFrame, cStream, &Value1 );
				Value = Value0 + (s16)(fRatio1*(Value1 - Value0));
			}

			ANIM_SetStreamValue( &rAnimDst, cFrame, cStream, Value );
		}
	}

	ANIM_KillInstance( &Anim0 );
	ANIM_KillInstance( &Anim1 );

	return NewNumFrames;
}

///////////////////////////////////////////////////////////////////////////
void CMotion::RotateAnimation( anim& rRotatedAnim, radian dYaw )
{
	radian3	Rot;
	vector3	Pos;
	s16		PosX;
	s16		PosZ;

	//---
	//
	//	now that we are sure to have two motions which are the same length, blend them together into a new motion
	//
	//---
	for( int cStream=0; cStream<rRotatedAnim.NBones; cStream+=3 )
	{
		for( int cFrame=0; cFrame<rRotatedAnim.NFrames; ++cFrame )
		{
			//---	interpolate this frame
			Rot.Pitch	= ANIM_GetRotation( &rRotatedAnim, cFrame, cStream+0 );
			Rot.Yaw		= ANIM_GetRotation( &rRotatedAnim, cFrame, cStream+1 );
			Rot.Roll	= ANIM_GetRotation( &rRotatedAnim, cFrame, cStream+2 );
			Rot.XYZ_to_XZY();
			Rot.Yaw += dYaw;
			Rot.XZY_to_XYZ();
			ANIM_SetRotation( &rRotatedAnim, cFrame, cStream+0, Rot.Pitch );
			ANIM_SetRotation( &rRotatedAnim, cFrame, cStream+1, Rot.Yaw );
			ANIM_SetRotation( &rRotatedAnim, cFrame, cStream+2, Rot.Roll );
		}
	}

	//---
	//
	//	now rotate the position stream (cStream should be on it now)
	//
	//---
	for( int cFrame=0; cFrame<rRotatedAnim.NFrames; ++cFrame )
	{
		ANIM_GetStreamValue( &rRotatedAnim, cFrame, cStream+0, &PosX );
		ANIM_GetStreamValue( &rRotatedAnim, cFrame, cStream+2, &PosZ );

		Pos.X = (f32)PosX;
		Pos.Z = (f32)PosZ;
		Pos.RotateY( dYaw );
		PosX = (s16)Pos.X;
		PosZ = (s16)Pos.Z;

		ANIM_SetStreamValue( &rRotatedAnim, cFrame, cStream+0, PosX );
		ANIM_SetStreamValue( &rRotatedAnim, cFrame, cStream+2, PosZ );
	}
}

///////////////////////////////////////////////////////////////////////////
void CMotion::AdjustBlendedWeights( void )
{
	s32 i;

	//---	calculate the total weight
	f32 TotalWeight=0;
	for( i=0; i<MAX_NUM_BLENDED_MOTIONS; i++ )
	{
		if( m_BlendedMotions[i].Ratio < 0.0f )
			m_BlendedMotions[i].Ratio = 0.0f;

		if( !m_BlendedMotions[i].pMotion )
			m_BlendedMotions[i].Ratio = 0.0f;

		TotalWeight += m_BlendedMotions[i].Ratio;
	}

	//---	adjust all weights involved
	for( i=0; i<MAX_NUM_BLENDED_MOTIONS; i++ )
	{
		m_BlendedMotions[i].Ratio = m_BlendedMotions[i].Ratio / TotalWeight;
	}
};
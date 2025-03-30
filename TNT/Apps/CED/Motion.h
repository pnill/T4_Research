#ifndef _Motion_h_
#define	_Motion_h_
// Motion.h : header file
//

#include "ieArchive.h"
#include "anim.h"
#include "EventList.h"
#include "FileTags.h"
#include "Prop.h"

class CSkeleton ;
class CCharacter ;

#define	MAX_NUM_PROPS			8
#define	MAX_NUM_BLENDED_MOTIONS	4


#define	EXPORTBITS_TO_QUATBITS(a)	((s32)(((f32)(a))*3.0f/4.0f + 0.5f))


/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_MOTION_END = IE_TAG_MOTION+1,

	IE_TAG_MOTION_PATHNAME,
	IE_TAG_MOTION_ISANM,
	IE_TAG_MOTION_FACEDIR,
	IE_TAG_MOTION_MOVEDIR,
	IE_TAG_MOTION_LINKCHARACTER,
	IE_TAG_MOTION_LINKMOTION,
	IE_TAG_MOTION_EXPORTNAME,

	IE_TAG_MOTION_FLAGBIT,

	IE_TAG_MOTION_SKELPOINTCHECK,
	IE_TAG_MOTION_AUTORESET_FACEDIR,
	IE_TAG_MOTION_AUTORESET_MOVEDIR,
	IE_TAG_MOTION_EXPORT,				// flag replaced by IE_TAG_MOTION_EXPORT_FLAG in new save, but kept for backward compatability

	IE_TAG_MOTION_EXPORT_FRAMERATE_ON,
	IE_TAG_MOTION_EXPORT_FRAMERATE,

	IE_TAG_MOTION_EXPORT_BITS_ON,
	IE_TAG_MOTION_EXPORT_BITS,

	IE_TAG_MOTION_EXPORT_LINK,

	IE_TAG_MOTION_EXPORT_WORLDROTS,
	IE_TAG_MOTION_EXPORT_QUATERNION,
	IE_TAG_MOTION_EXPORT_QUATBLEND,
	IE_TAG_MOTION_EXPORT_DEFAULT,

	IE_TAG_MOTION_EXPORT_QUATBITS_ON,	// no longer used, but don't remove because the values must remain constant
	IE_TAG_MOTION_EXPORT_QUATBITS,		// no longer used, but don't remove because the values must remain constant

	IE_TAG_MOTION_EXPORT_GLOBALROTS,

	IE_TAG_MOTION_EXPORT_FLAG,

	IE_TAG_MOTION_PROPS,

	IE_TAG_MOTION_BLEND_COUNT,
	IE_TAG_MOTION_BLEND_EXPORTFORMULA,

	IE_TAG_MOTION_EXPORT_PROPQUATBLEND,

	IE_TAG_MOTION_EXPORT_QUATWVALUE,
} ;



/////////////////////////////////////////////////////////////////////////////
// CMotion

#define	MOTION_FLAG_BITS	16

class CMotion : public CObject
{
	DECLARE_DYNCREATE(CMotion)

// Attributes
public:

	enum ELoadResultFlags
	{
		LOAD_RESULT_ERROR				= (1<<0),
		LOAD_RESULT_ONEFRAME			= (1<<1),
		LOAD_RESULT_FRAMESCHANGED		= (1<<2),
		LOAD_RESULT_EVENTOUTSIDEANIM	= (1<<3),
	};

	//---	structure to hold information for blending motions
	struct SMotionBlend
	{
		CMotion*	pMotion;					// motion being blended to create a new motion
		f32			Ratio;						// amount of blending from 0.0 to 1.0
		s32			RuntimeVersion;				// version of the motion last referenced (if this number doesn't match the
												// pMotion->GetRuntimeVersion() value, this motion must be recalculated)
	};

public:
	CCharacter	*m_pCharacter ;														// Character that owns Motion
																					
	CString		m_PathName ;														// Path to File
	CString		m_ExportName ;														// Export Name
																					
	bool		m_IsLoaded ;														// Anim has been loaded
	bool		m_LoadFailed ;														// Load Failed
	bool		m_IsANM ;															// Loaded from ANM
																					
	anim		m_Anim ;															// Anim which goes with this Motion
	radian3*	m_pRootRotsSave ;													// save the original root rotations (these will be modified for actual motion).
																					
	s32			m_nFrames60 ;														// Num Frames when played at 60 frames per second (with interpolation)
	f32			m_FrameRate ;														// frame rate to play animation at to get a 60 fps animation
																					
	f32			*m_pFaceDir ;														// FaceDir in Radians
	f32			*m_pMoveDir ;														// MoveDir in Radians
																					
	s32			m_FaceDirArrayLength ;												// used to tell if the number of frames of the face direction array needs to be changed to 
																					// match a new number of frames either because the newly loaded motion was changed, or the motion is blended differently.
	s32			m_MoveDirArrayLength ;												// same as for face direction
																					
	bool		m_bExportFrameRate;													// T/F, does this animation specify an export frame rate
	f32			m_ExportFrameRate;													// frame rate in frames/second
																					
	bool		m_bExportBits;														// Export bits given
	s32			m_ExportBits;														// Export bits;
																					
	bool		m_bExportDefault;													// use the default character specifications
	bool		m_bExportQuaternion;												// export quaternions rather that euler
	bool		m_bExportQuatW;														// export the quaternion W value rather than generating it on the fly
	bool		m_bExportWorldRots;													// export world rather than frame roations (frame rotations must have the frame face dir added to the yaw).
	bool		m_bExportGlobalRots;												// export global rather than local rotations (local must be combined with parent bone's rots)
	bool		m_bExportQuatBlending;												// force quat blending for this animation (N/A to quaternion export)
	bool		m_bExportPropQuatBlending;											// force quat blending for this animation's props (N/A to quaterion export)
																					
	CEventList	m_EventList ;														// List of Events
	CEvent		*m_pCurEvent ;														// Current Event
																					
	CPropData	m_Props[MAX_NUM_PROPS] ;											// array of prop data
																					
	bool		m_FlagBit[MOTION_FLAG_BITS] ;										// Flag Bits
																					
	CByteArray	m_SkelPointChecks ;													// Skel Point Checks
																					
	s32			m_bExport ;															// T/F, should this motion be exported ( a value of 2 means that it is intended to be removed, but is still needed at export).
																					
	bool		m_ChangedExport ;													
																					
	s32			m_FaceDirRecipeUsed ;												// Set to the ID of the recipe used to calculate the face dirs.
																					// if the recipe changes, the next request for a face dir from
																					// this animation will cause it to re-calculate the face dirs.
																					
	CMotion		*m_pCompVersion ;													// used to show what this motion will look like compressed.
																					
	bool		m_bSupressWarningBoxes ;											// T/F should warning boxes be supressed.  Only TRUE during export.
	bool		m_bInUpdateMotionVersion ;											// T/F should the EnsureLoaded() function be turned off?  Set to TRUE while building a blended motion.

	//---	these members are kept to aid in loading up old versions of the file.
	//		they are not used after the file is loaded and saved in the new version.
	CCharacter*	m_pLinkCharacter_OldSystem;
	CMotion*	m_pLinkMotion_OldSystem;
	bool		m_bExportLink_OldSystem;											// T/F, should the linked character/motion be exported
																					
	//---	Multi-Motion Blend data													
	bool			m_bExportBlendFormula;											// T/F, should the motion's blend formula be exported rather than the actual blended motion?
	s32				m_LastUpdateRuntimeVersion;										// version of this motion (this value increments whenever a change is made, this will cause the motion to be re-loaded/re-created)
	s32				m_RuntimeVersion;												// version of this motion (this value increments whenever a change is made, this will cause the motion to be re-loaded/re-created)
	s32				m_BlendedMotionCount;											// number of motions blended to create this one, if zero, this motion is loaded from a file.
	SMotionBlend	m_BlendedMotions[MAX_NUM_BLENDED_MOTIONS];						// blend information for the blended motions

	bool		m_bAutoResetMoveDir;
	bool		m_bAutoResetFaceDir;

// Constructors
public:
	CMotion();
	~CMotion();

// Operations
public:
	s32			GetExportBits			( void );
	f32			GetExportFrameRate		( void );
	bool		GetExportQuaternion		( void );
	bool		GetExportQuatW			( void );
	bool		GetExportWorldRots		( void );
	bool		GetExportGlobalRots		( void );
	bool		GetExportQuatBlend		( void );
	bool		GetExportPropQuatBlend	( void );

	s32			CountNumExportedStreams( s32 PropsUsed );

	bool		GetAutoResetMoveDir		( void );
	bool		GetAutoResetFaceDir		( void );
	void		SetAutoResetMoveDir		( bool bReset );
	void		SetAutoResetFaceDir		( bool bReset );
	virtual void Serialize( CieArchive& a, CCharacter *pCharacter ) ;
	CMotion &operator=( CMotion & ) ;												// Right side is the argument.
																					
	bool		CreateFromFile( const char *pFileName,								
								bool		SupressWarningBoxes=FALSE,				
								long*		pResult=NULL ) ;						// Create Motion from File
	bool		CreateFromAMC(	const char *pFileName,								
								bool		SupressWarningBoxes=FALSE,				
								long*		pResult=NULL ) ;						// Create Motion from AMC
	bool		CreateFromAnim( bool SupressWarningBoxes, long* pResult );			
	bool		ReloadFromFile( bool		SupressWarningBoxes=FALSE,				
								long*		pResult=NULL ) ;						// Reload the motion file
	bool		EnsureLoaded(	long*		pResult=NULL ) ;						// Ensure Motion Loaded
	void		EnsureFaceDirs( void );												
	void		EnsureMoveDirs( void );												
																					
																					
	const char	*GetPathName() ;													// Get Name of Motion
	const char	*GetExportName() ;													// Get Name of Export
																					
	anim		*GetAnim() ;														// Get Pointer to anim
	int			GetStartFrame() ;													// Get First Frame with Motion Data
	int			GetNumFrames() ;													// Get Number of Frames in Anim
	int			GetNumRealFrames() ;												// Actuall frames in the anim data
																					
	void		GetTranslation( vector3& rVector, int nFrame ) ;					// Get Translation from Anim
	void		GetRotation( radian3& rRotation, int nFrame ) ;						
																					
	void		GetTranslationBlend( vector3& rVector, f32 nFrame ) ;				// Get Translation from Anim
	void		GetRotationBlend( radian3& rRotation, f32 nFrame ) ;				// Get the rotation from the animation
																					
	radian		GetFaceDir( int nFrame ) ;											// Get FaceDir at Frame
	void		SetFaceDir( int nFrame, radian FaceDir ) ;							// Set FaceDir at Frame
	void		ResetFaceDir( s32 nFrame );											// Calculate the FaceDir based on the skeleton pose
	void		SetAllMoveDir( radian MoveDir ) ;									// Set All FaceDir values
	void		ResetAllMoveDir( void ) ;			        						// Set All FaceDir values
	radian		GetMoveDir( int nFrame ) ;											// Get MoveDir at Frame
	void		SetMoveDir( int nFrame, radian MoveDir ) ;							// Set MoveDir at Frame
	void		ResetAllFaceDir( xbool bForce=FALSE ) ;        						// Set All FaceDir values
	void		SetAllFaceDir( radian FaceDir ) ;									// Set All MoveDir values
	radian		GetFaceDirBlend( float fFrame ) ;									
																					
	void		SaveRootRots( void );												// saves all of the root rotation values
	void		RestoreRootRots( void );											
	void		FrameAdjustRootRots( void );										// adjust the root rotations so that they are relative to the frame


	void		GetBoneRots( s32 nFrame, s32 BoneID, f32& Rx, f32& Ry, f32& Rz );

	CPropData&	GetProp( s32 i );

	int			GetNumEvents( ) ;													// Get Number of Events
	CEvent		*GetEvent( int iEvent ) ;											// Get Event from Index
	void		SetCurEvent( CEvent *pEvent ) ;										// Set Current Event
	CEvent		*GetCurEvent( ) ;													// Get Current Event
	void		DeleteEvent( CEvent *pEvent ) ;										// Delete Event
	CEvent		*AddEvent( ) ;														// Add New Event
																					
																					
	//---	Blend motion functions													
	bool		IsExportBlendFormula( void );										// return TRUE if the blend formula should be exported rather than the motion itself.  Exporting the formula saves export data, but is slower at run-time.
	void		SetExportBlendformula( bool bSetting );								// sets the export flag to determine if the formula or the motion gets exported.
	bool		IsBlendedMotion( void );											// returns TRUE if this motion is created by blending other motions
	s32			GetRuntimeVersion( void );											// get the current version number of this motion.  whenever anything changes this number is incremented to inform any other motions based on this one to reset.
	void		IncRuntimeVersion( void );											// increment the runtime version number.  This will reset this motion to be recalculated when anything requests information from it.
	bool		CheckMotionVersion( void );											// quick function intended to rebuild the motion if any changes have been made to it or any of its blended motions.
	void		UpdateMotionVersion( void );										// function to rebuild the motion if any changes have been made to it or any of its blended motions.
	void		CheckUpdateMotionVersion( void );									// quick function intended to rebuild the motion if any changes have been made to it or any of its blended motions.
	bool		CheckCompressedMotionVersion( void );								// quick function intended to rebuild the motion if any changes have been made to it or any of its blended motions.
	void		UpdateCompressedMotionVersion( bool bMimicBitCompression=TRUE );	// function to rebuild the motion if any changes have been made to it or any of its blended motions.
	void		CheckUpdateCompressedMotionVersion( void );							// quick function intended to rebuild the motion if any changes have been made to it or any of its blended motions.
	bool		BlendMotion( CMotion* pMotion, f32 fRatio );						// merges another motion into this one at a particular ratio
	void		AdjustBlendedWeights( void );										// ensures that all ratios for blended motions total 1.0f
																					
																					
	BOOL		SkelPointGetCheck( int Index ) ;									// Get Check State
	void		SkelPointSetCheck( int Index, BOOL State ) ;						// Set Check State
	void		SkelPointDeleteCheck( int Index ) ;									// Delete Check
																					
	bool		Modify( void ) ;													// Displays a dialog to allow modification
	bool		SearchReplacePathName( CString& rSearch,							
									   CString& rReplace,							
									   s32 Method );								
																					
	bool		CheckCompressedVersion( void );										
	CMotion*	GetCompressedVersion( bool bMimicBitCompression=TRUE );				// returns the compressed version of this motion
																					
	void		MakeIntoCompressedVersion( xbool bMimicBitCompression );			// changes this motion data into its compressed version
	f32			ResampleAnimation( anim& rResampAnim, f32 FrameRate );				// changes this motion data to reflect a new frame rate
	f32			ResampleAnimation( anim& rResampAnim, s32 NewNumFrames );			// changes this motion data to reflect a new number of frames
	s32			BlendAnimation( anim& rAnimDst, anim& rAnimSrc, f32 ratio );		// blends two animations together returns the new number of frames
	void		RotateAnimation( anim& rRotatedAnim, radian dYaw );					// rotates an animation
	void		MimicFinalExportQuantizationResult( anim& Anim, s32 ExportBits );	// changes this motion data to reflect what the compressed version would look like.
	void		ResetFaceMoveDirsForNewMotion( void );								// after resampling the animation, this function makes sure that
																					// the face dirs/move dirs are reset properly
	void		ForceCompressedVersionUpdate( void );

	void		InvalidateFaceDirs( void );
	void		InvalidateMoveDirs( void );

// Implementation
protected:
	void		Initialize();														// Initialize Data
	void		Destroy();															// Destroy Motion
	void		Copy_Essentials( CMotion& Dst, CMotion& Src );								
	void		Copy( CMotion &Dst, CMotion &Src );									// Copy
				
	void		ChangeFaceDir( s32 iFrame, f32 FaceDir );

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
inline bool CMotion::IsExportBlendFormula( void )
{
	return m_bExportBlendFormula;
}

/////////////////////////////////////////////////////////////////////////////
inline bool CMotion::IsBlendedMotion( void )
{
	return (m_BlendedMotionCount > 0) ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////
inline void CMotion::SetExportBlendformula( bool bSetting )
{
	m_bExportBlendFormula = bSetting;

	if( IsBlendedMotion() )
		ForceCompressedVersionUpdate();
}

/////////////////////////////////////////////////////////////////////////////
inline void CMotion::IncRuntimeVersion( void )
{
	m_RuntimeVersion++;
}

/////////////////////////////////////////////////////////////////////////////
inline s32 CMotion::GetRuntimeVersion( void )
{
	return m_RuntimeVersion;
}

///////////////////////////////////////////////////////////////////////////////
inline bool CMotion::CheckMotionVersion( void )
{
	s32 i;

	if( GetRuntimeVersion() != m_LastUpdateRuntimeVersion )
		return FALSE;

	//---	check blended motions for changes
	for( i=0; i<m_BlendedMotionCount; i++ )
	{
		ASSERT(m_BlendedMotions[i].pMotion);
		if( m_BlendedMotions[i].pMotion->GetRuntimeVersion() != m_BlendedMotions[i].RuntimeVersion )
			return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
inline void CMotion::CheckUpdateMotionVersion( void )
{
	if( CheckMotionVersion() )
		return;

	UpdateMotionVersion();
}

/////////////////////////////////////////////////////////////////////////////
inline bool CMotion::CheckCompressedVersion( void )
{
	if( !CheckMotionVersion() )
		return FALSE;

	if(( m_pCompVersion->GetExportBits() == GetExportBits() )  &&
	   ( m_pCompVersion->GetExportFrameRate() == GetExportFrameRate() ) &&
	   ( m_pCompVersion->GetExportQuatW() == GetExportQuatW() ))
	   return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
inline CPropData& CMotion::GetProp( s32 i )
{
	ASSERT((i>=0)&&(i<MAX_NUM_PROPS));
	return m_Props[i];
}

/////////////////////////////////////////////////////////////////////////////
#endif	//_Motion_h_

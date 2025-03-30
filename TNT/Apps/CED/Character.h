#ifndef _Character_h_
#define	_Character_h_
// Character.h : header file
//

#include "ieArchive.h"
#include "Skeleton.h"
#include "MotionList.h"
#include "SkelPointList.h"
#include "SkinList.h"

#include "FileTags.h"
#include "EventList.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_CHARACTER_END = IE_TAG_CHARACTER+1,

	IE_TAG_CHARACTER_NAME,
	IE_TAG_CHARACTER_NOEXPORT,

	IE_TAG_CHARACTER_EXPORT_ENABLEGAMECUBE,
	IE_TAG_CHARACTER_EXPORT_ENABLEPC,
	IE_TAG_CHARACTER_EXPORT_FILEGAMECUBE,
	IE_TAG_CHARACTER_EXPORT_FILEPC,
	IE_TAG_CHARACTER_EXPORT_OPTFORCEUPPER,
	IE_TAG_CHARACTER_EXPORT_OPTCOLLISIONBUBBLES,
	IE_TAG_CHARACTER_EXPORT_OPTMOTIONFLAGS,

	IE_TAG_CHARACTER_EXPORT_ENABLEXBOX,
	IE_TAG_CHARACTER_EXPORT_FILEXBOX,
	IE_TAG_CHARACTER_EXPORT_ENABLEPS2,
	IE_TAG_CHARACTER_EXPORT_FILEPS2,

	IE_TAG_CHARACTER_EXPORT_FRAMERATE,
	IE_TAG_CHARACTER_EXPORT_BITS,
	IE_TAG_CHARACTER_EXPORT_FRAMESPERBLOCK,

	IE_TAG_CHARACTER_FACEDIRRECIPE,

	IE_TAG_CHARACTER_EXPORT_DATAFILE,
	IE_TAG_CHARACTER_EXPORT_HEADERFILE,
	IE_TAG_CHARACTER_EXPORT_TEXTFILE,
	IE_TAG_CHARACTER_EXPORT_HEADERPATHNAME,

	IE_TAG_CHARACTER_EXPORT_QUATERNION,
	IE_TAG_CHARACTER_EXPORT_WORLDROT,
	IE_TAG_CHARACTER_EXPORT_QUATBLEND,
	IE_TAG_CHARACTER_EXPORT_QUATBITS_ON,		// no longer used, but don't remove because the values must remain constant
	IE_TAG_CHARACTER_EXPORT_QUATBITS,			// no longer used, but don't remove because the values must remain constant
	IE_TAG_CHARACTER_EXPORT_GLOBALROT,

	IE_TAG_CHARACTER_SKELETONFILE,

	IE_TAG_CHARACTER_PROPDEFS,
	IE_TAG_CHARACTER_EXPORT_PROPQUATBLEND,

	IE_TAG_CHARACTER_EXPORT_QUATWVALUE,

	IE_TAG_CHARACTER_EXPORT_MOTIONPREFIX,
	IE_TAG_CHARACTER_EXPORT_EVENTPREFIX,
} ;

class CCeDDoc ;

/////////////////////////////////////////////////////////////////////////////

//---	structure for holding FaceDir recipe bone weights
#define	MAX_FACEDIR_RECIPE_LENGTH		6
struct SBoneWeight
{
	CString			BoneName;
	s32				BoneID;
	s32				Weight;
};


/////////////////////////////////////////////////////////////////////////////
// CCharacter

class CCharacter : public CObject
{
	DECLARE_DYNCREATE(CCharacter)

// Attributes
public:
	CCeDDoc			*m_pDoc ;								// Pointer to Document

	radian			m_FaceDirAngle ;						// Face Direction for Current Frame

	CString			m_Name ;								// Character Name

	CSkeleton		*m_pSkeleton ;							// Skeleton
	CString			m_SkeletonPathName ;					// remembered skeleton file path

	CSkelPointList	m_SkelPointList ;						// List of Skeleton Points
	CSkelPoint		*m_pCurSkelPoint ;						// Pointer to Current Skel Point

	CSkinList		m_SkinList ;							// List of Skins used by this character
	CSkin			*m_pCurSkin ;							// Pointer to the Current Skin

	CMotionList		m_MotionList ;							// List of Motions
	CMotion			*m_pCurMotion ;							// Pointer to Current Motion

	CMotion			*m_pSecondMotion ;						// bi-linear blend view motion
	f32				m_SecondMotionRatio;

	bool			m_bRenderSkinTranslucent;				// render the skin transucent

	float			m_nFrame ;								// Frame of Motion
	bool			m_NoExport ;							// No Export Flag

	bool			m_ExportEnableGAMECUBE ;				// Enable GAMECUBE Export of this Character
	bool			m_ExportEnablePC ;						// Enable PC Export of this Character
	bool			m_ExportEnableXBOX ;					// Enable XBOX Export of this Character
	bool			m_ExportEnablePS2 ;						// Enable PS2 Export of this Character
	CString			m_ExportFileGAMECUBE ;					// GAMECUBE Export File
	CString			m_ExportFilePC ;						// PC Export File
	CString			m_ExportFileXBOX ;						// XBOX Export File
	CString			m_ExportFilePS2 ;						// PS2 Export File
	bool			m_ExportOptForceUpper ;					// Force Uppercase Define Export
	bool			m_ExportOptCollisionBubbles ;			// Export Collision Bubbles
	bool			m_ExportOptMotionFlags ;				// Export Motion Flags

	f32				m_ExportFrameRate ;						// number of frames/second in the export
	s32				m_ExportBits ;							// number of bits to encode the stream data
	s32				m_ExportFramesPerBlock ;				// number of frames to export in each block
	
	bool			m_bExportQuaternion ;					// export quaternions rather that euler
	bool			m_bExportQuatW ;						// export the quaterion W value rather than calculate it on the fly
	bool			m_bExportWorldRots ;					// export world rather than frame rotations (which must have the face dir added to them)
	bool			m_bExportGlobalRots ;					// export global rotations rather than local rotations (which must be combined with parent rotations)
	bool			m_bExportQuatBlending ;					// force quat blending for this animation (N/A to quaternion export)
	bool			m_bExportPropQuatBlending ;				// force quat blending for this animation's props (N/A to quaternion export)

	bool			m_ViewMotionBlended ;

	CString			m_ExportMotionPrefix ;
	CString			m_ExportEventPrefix ;

	matrix4			m_WorldBase ; // base offset of all displayed motions

	//---	prop definitions
	CPropDef		m_PropDefs[MAX_NUM_PROPS] ;				// array of prop definitions

	//---	special motion playback data
	bool			m_bBlendedPlaybackActive ;
	CMotion*		m_pLastMotion ;
	f32				m_LastMotion_StartFrame ;
	f32				m_LastMotion_EndFrame ;
	f32				m_CurMotion_BlendFrames ;
	f32				m_CurMotion_StartFrame ;
	f32				m_CurMotion_EndFrame ;

	//---	FaceDir calculation is a weight of many bones
	SBoneWeight		m_FaceDirRecipe[MAX_FACEDIR_RECIPE_LENGTH];
	s32				m_FaceDirRecipeCount;
	s32				m_FaceDirRecipeID;

	//---	export options flags
	bool			m_bExportDataFile;
	bool			m_bExportHeaderFile;
	bool			m_bExportTextFile;
	bool			m_bExportHeaderFilePath;

// Constructors
public:
	CCharacter();
	~CCharacter();

// Operations
public:
	CEventList* GetAttachedEvents();
	virtual void Serialize(CieArchive& a) ;
	CCharacter &operator=( CCharacter & ) ;								// Right side is the argument.

	bool		Create					( ) ;							// Create Character

	void		SetName					( const char *pName ) ;			// Set Name of Character
	const char	*GetName				( ) ;							// Get Name of Character

	bool		ImportSkeletonFromFile	( const char *pFileName ) ;		// Import a Skeleton from an ASF file
	CSkeleton	*GetSkeleton			( ) ;							// Get Pointer to Skeleton

	bool		ImportSkinFromFile		( const char *pFileName ) ;		// Loads a skin from a skin file
	bool		RemoveSkin				( void ) ;						// Removes a loaded skin
	bool		RemoveSkin				( int iSkin ) ;					// Removes a loaded skin
	CSkin*		GetSkin					( int iSkin ) ;					// Returns a pointer to a specific skin
	void		SetCurSkin				( int iSkin ) ;					// Sets the current skin
	CSkin*		GetCurSkin				( void ) ;						// Returns a pointer to the current skin
	int			GetNumSkins				( void ) ;						// Returns the number of skins in the file
	bool		FitSkinsToSkeleton		( void ) ;						// fits the skins to the skeleton (called when a new skeleton is set)

	bool		AddMotionToList			( CMotion *pMotion ) ;			// Add a Motion to the List manually
	CMotion*	ImportMotionFromFile	( const char *pFileName ) ;		// Import a Motion from an AMC file
	CMotion*	NewMotion				( void );						// creates a new motion


	int			GetNumMotions			( ) ;							// Get Number of Motions
	CMotion		*GetMotion				( int iMotion ) ;				// Get Motion from Index

	CMotion		*GetCurMotion			( ) ;							// Get Pointer to Current Motion
	CMotion		*SetCurMotion			( CMotion *pMotion ) ;			// Set Current Motion to specified Motion
	CMotion		*SetCurMotion			( CMotion *pMotion, f32 BlendFrames, f32 StartFrame, f32 EndFrame, CMotion *pLastMotion=NULL, f32 LastFrame=0 );
	CMotion		*SetCurMotion			( int iMotion ) ;				// Set Current Motion from List

	//---	bi-directional blending
	CMotion		*SetSecondMotion		( CMotion *pMotion, f32 Ratio ) ;// Set a second motion to be blended to and the ratio of that motion to be used.

	void		DeleteMotion			( CMotion *pMotion ) ;			// Delete Motion
	CMotion		*FindMotionByName		( const char *pPathName ) ;		// Find Motion given PathName
	CMotion		*FindMotionByExportName	( const char *pExportName ) ;	// Find Motion given ExportName
	CMotion		*GetMatchingMotion		( CMotion *pMotion ) ;			// Get Matching Motion given Motion

	bool		RenderSkinTranslucent	( bool bTranslucent ) ;
	bool		RenderSkinTranslucent	( void );

	CPropDef&	GetPropDef				( s32 i );						// get the prop definition.
	CPropDef*	AllocateNewPropDef		( void );
	CPropDef*	FindPropByCharacter		( CCharacter* pFindCharacter );

	float		GetCurFrame				( ) ;							// Get Anim Frame
	void		SetCurFrame				( float nFrame ) ;				// Set Anim Frame
	void		SetPose					( bool Blend=false ) ;			// Set Skeleton Pose
//	void		SetPose					( float nFrame,
//										  bool Blend=false ) ;			// Set Skeleton Pose
	void		SetPose					( CMotion *pMotion,
										  float nFrame,
										  bool Blend=false,
										  bool Mirror=false) ;			// Set Pose
	void		SetPoseAsProp			( const vector3& Pos,
										  const radian3& Rot,
										  CMotion* pMotion,
										  f32 Frame,
										  bool Blend = FALSE,
										  bool Mirror = FALSE);
	void		AnimGotoStart			( ) ;							// Goto Start of Anim
	void		AnimPreviousFrame		( ) ;							// Goto Previous Frame
	void		AnimNextFrame			( ) ;							// Goto Next Frame
	void		AnimGotoEnd				( ) ;							// Goto End of Anim

	void		SetCurSkelPoint			( CSkelPoint *pSkelPoint ) ;	// Set Current SkelPoint
	CSkelPoint	*GetCurSkelPoint		( ) ;							// Get Current SkelPoint
	void		DeleteSkelPoint			( CSkelPoint *pSkelPoint ) ;	// Delete SkelPoint
	CSkelPoint	*AddSkelPoint			( ) ;							// Add new SkelPoint
	bool		SearchReplaceFilePaths	( CString Search, CString Replace, s32 Method );

	void		EditProperties			( void );

	s32			MotionToExportIndex		( CMotion *pMotion );			// determines the exported index of a motion

	void		MapFaceDirRecipeToSkel	( void ) ;
	s32					GetFaceDirRecipeLength	( void )		{ return m_FaceDirRecipeCount; }
	const SBoneWeight&	GetFaceDirRecipe		( s32 Index )	{ ASSERT(Index>=0&&Index<m_FaceDirRecipeCount); return m_FaceDirRecipe[Index]; }

// Implementation
protected:
	CEventList m_AttachedEvents;
	void Initialize() ;										// Initialize Data
	void Destroy () ;										// Destroy Character
	void Copy (CCharacter &Dst, CCharacter &Src) ;			// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

inline bool CCharacter::RenderSkinTranslucent( bool bTranslucent )
{
	return m_bRenderSkinTranslucent = bTranslucent;
}

inline bool CCharacter::RenderSkinTranslucent( void )
{
	return m_bRenderSkinTranslucent;
}

/////////////////////////////////////////////////////////////////////////////
inline CPropDef& CCharacter::GetPropDef( s32 i )
{
	ASSERT((i>=0)&&(i<MAX_NUM_PROPS));
	return m_PropDefs[i];
}


/////////////////////////////////////////////////////////////////////////////
#endif	//_Character_h_

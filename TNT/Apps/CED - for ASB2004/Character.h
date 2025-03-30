#ifndef _Character_h_
#define	_Character_h_
// Character.h : header file
//

#include "ieArchive.h"
#include "Skeleton.h"
#include "MotionList.h"
#include "SkelPointList.h"

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
} ;

/////////////////////////////////////////////////////////////////////////////
// CCharacter

class CCeDDoc ;

class CCharacter : public CObject
{
	DECLARE_DYNCREATE(CCharacter)

// Attributes
public:
	CCeDDoc			*m_pDoc ;								// Pointer to Document

	radian			m_FaceDirAngle ;						// Face Direction for Current Frame

	CString			m_Name ;								// Character Name

	CSkeleton		*m_pSkeleton ;							// Skeleton

	CSkelPointList	m_SkelPointList ;						// List of Skeleton Points
	CSkelPoint		*m_pCurSkelPoint ;						// Pointer to Current Skel Point

	CMotionList		m_MotionList ;							// List of Motions
	CMotion			*m_pCurMotion ;							// Pointer to Current Motion

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
	bool			m_ExportFrontEndSpecific;				// Export Front End Specific

	f32				m_ExportFrameRate ;						// number of frames/second in the export
	s32				m_ExportBits ;							// number of bits to encode the stream data
	s32				m_ExportFramesPerBlock ;				// number of frames to export in each block

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

	bool		AddMotionToList			( CMotion *pMotion ) ;			// Add a Motion to the List manually
	CMotion		*ImportMotionFromFile	( const char *pFileName ) ;		// Import a Motion from an AMC file

	int			GetNumMotions			( ) ;							// Get Number of Motions
	CMotion		*GetMotion				( int iMotion ) ;				// Get Motion from Index

	CMotion		*GetCurMotion			( ) ;							// Get Pointer to Current Motion
	CMotion		*SetCurMotion			( CMotion *pMotion ) ;			// Set Current Motion to specified Motion
	CMotion		*SetCurMotion			( int iMotion ) ;				// Set Current Motion from List
	void		DeleteMotion			( CMotion *pMotion ) ;			// Delete Motion
	CMotion		*FindMotionByName		( const char *pPathName ) ;		// Find Motion given PathName
	CMotion		*GetMatchingMotion		( CMotion *pMotion ) ;			// Get Matching Motion given Motion

	float		GetCurFrame				( ) ;							// Get Anim Frame
	void		SetCurFrame				( float nFrame ) ;				// Set Anim Frame
	void		SetPose					( float nFrame,
										  bool Blend=false ) ;			// Set Skeleton Pose
	void		SetPose					( CMotion *pMotion,
										  float nFrame,
										  bool Blend=false) ;			// Set Pose
	void		AnimGotoStart			( ) ;							// Goto Start of Anim
	void		AnimPreviousFrame		( ) ;							// Goto Previous Frame
	void		AnimNextFrame			( ) ;							// Goto Next Frame
	void		AnimGotoEnd				( ) ;							// Goto End of Anim

	void		SetCurSkelPoint			( CSkelPoint *pSkelPoint ) ;	// Set Current SkelPoint
	CSkelPoint	*GetCurSkelPoint		( ) ;							// Get Current SkelPoint
	void		DeleteSkelPoint			( CSkelPoint *pSkelPoint ) ;	// Delete SkelPoint
	CSkelPoint	*AddSkelPoint			( ) ;							// Add new SkelPoint
	bool		SearchReplaceFilePaths	( CString Search, CString Replace, s32 Method );

	s32			MotionToExportIndex		( CMotion *pMotion );			// determines the exported index of a motion

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

/////////////////////////////////////////////////////////////////////////////
#endif	//_Character_h_

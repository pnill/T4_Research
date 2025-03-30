#ifndef _Motion_h_
#define	_Motion_h_
// Motion.h : header file
//

#include "ieArchive.h"
#include "anim.h"
#include "EventList.h"
#include "FileTags.h"

class CSkeleton ;
class CCharacter ;

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
	IE_TAG_MOTION_EXPORT,

	IE_TAG_MOTION_EXPORT_FRAMERATE_ON,
	IE_TAG_MOTION_EXPORT_FRAMERATE,

	IE_TAG_MOTION_EXPORT_BITS_ON,
	IE_TAG_MOTION_EXPORT_BITS,

	IE_TAG_MOTION_EXPORT_LINK,
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

public:
	CCharacter	*m_pCharacter ;													// Character that owns Motion

	CString		m_PathName ;													// Path to File
	CString		m_ExportName ;													// Export Name

	anim		m_Anim ;														// Anim which goes with this Motion

	bool		m_IsLoaded ;													// Anim has been loaded
	bool		m_LoadFailed ;													// Load Failed
	bool		m_IsANM ;														// Loaded from ANM

	s32			m_nFrames ;														// Num Frames
	f32			*m_pFaceDir ;													// FaceDir in Radians
	f32			*m_pMoveDir ;													// MoveDir in Radians

	bool		m_bExportFrameRate;												// T/F, does this animation specify an export frame rate
	f32			m_ExportFrameRate;												// frame rate in frames/second

	bool		m_bExportBits;
	s32			m_ExportBits;													// Export bits;

	CEventList	m_EventList ;													// List of Events
	CEvent		*m_pCurEvent ;													// Current Event

	CCharacter	*m_pLinkCharacter ;												// Linked Character
	CMotion		*m_pLinkMotion ;												// Linked Motion
	bool		m_FixupLinkStuff ;												// Linked pointers need Fixup

	bool		m_FlagBit[MOTION_FLAG_BITS] ;									// Flag Bits

	CByteArray	m_SkelPointChecks ;												// Skel Point Checks

	bool		m_bExport ;														// T/F, should this motion be exported
	bool		m_bExportLink ;													// T/F, should the linked character/motion be exported

	bool		m_ChangedExport ;


	CMotion		*m_pCompVersion ;												// used to show what this motion will look like compressed.

// Constructors
public:
	CMotion();
	~CMotion();

// Operations
public:
	s32	GetExportBits( void );
	f32	GetExportFrameRate( void );

	bool GetAutoResetMoveDir();
	bool GetAutoResetFaceDir();
	void SetAutoResetMoveDir( bool bReset );
	void SetAutoResetFaceDir( bool bReset );
	virtual void Serialize( CieArchive& a, CCharacter *pCharacter ) ;
	CMotion &operator=( CMotion & ) ;											// Right side is the argument.

	bool		CreateFromFile( const char *pFileName,
								bool		SupressWarningBoxes=FALSE,
								long*		pResult=NULL ) ;					// Create Motion from File
	bool		CreateFromAMC(	const char *pFileName,
								bool		SupressWarningBoxes=FALSE,
								long*		pResult=NULL ) ;					// Create Motion from AMC
	bool		CreateFromAnim( bool SupressWarningBoxes, long* pResult );
	bool		ReloadFromFile( bool		SupressWarningBoxes=FALSE,
								long*		pResult=NULL ) ;					// Reload the motion file
	bool		EnsureLoaded( ) ;												// Ensure Motion Loaded

	const char	*GetPathName() ;												// Get Name of Motion
	const char	*GetExportName() ;												// Get Name of Export

	anim		*GetAnim() ;													// Get Pointer to anim
	int			GetNumFrames() ;												// Get Number of Frames in Anim
	int			GetStartFrame() ;												// Get First Frame with Motion Data

	void		GetTranslation( vector3d *pVector, int nFrame ) ;				// Get Translation from Anim

	radian		GetFaceDir( int nFrame ) ;										// Get FaceDir at Frame
	void		SetFaceDir( int nFrame, radian FaceDir ) ;						// Set FaceDir at Frame
	void		SetAllMoveDir( radian MoveDir ) ;								// Set All FaceDir values
	void		ResetAllMoveDir( void ) ;			        					// Set All FaceDir values
	radian		GetMoveDir( int nFrame ) ;										// Get MoveDir at Frame
	void		SetMoveDir( int nFrame, radian MoveDir ) ;						// Set MoveDir at Frame
	void		ResetAllFaceDir( void ) ;			        					// Set All FaceDir values
	void		SetAllFaceDir( radian FaceDir ) ;								// Set All MoveDir values
	void		SetRemainingMoveDir( int nFrame, radian MoveDir);				
	void		SetRemainingFaceDir( int nFrame, radian FaceDir);				

	int			GetNumEvents( ) ;												// Get Number of Events
	CEvent		*GetEvent( int iEvent ) ;										// Get Event from Index
	void		SetCurEvent( CEvent *pEvent ) ;									// Set Current Event
	CEvent		*GetCurEvent( ) ;												// Get Current Event
	void		DeleteEvent( CEvent *pEvent ) ;									// Delete Event
	CEvent		*AddEvent( ) ;													// Add New Event

	BOOL		SkelPointGetCheck( int Index ) ;								// Get Check State
	void		SkelPointSetCheck( int Index, BOOL State ) ;					// Set Check State
	void		SkelPointDeleteCheck( int Index ) ;								// Delete Check

	bool		Modify( void ) ;												// Displays a dialog to allow modification
	bool		SearchReplacePathName( CString& rSearch,
									   CString& rReplace,
									   s32 Method );

	CMotion*	GetCompressedVersion( void );									// returns the compressed version of this motion

	void		MakeIntoCompressedVersion( void );								// changes this motion data into its compressed version
	f32			GetResampledAnimation( anim& rResampAnim, f32 FrameRate );		// changes this motion data to reflect a new frame rate

// Implementation
protected:
	bool m_bAutoResetMoveDir;
	bool m_bAutoResetFaceDir;
	void Initialize() ;															// Initialize Data
	void Destroy () ;															// Destroy Motion
	void Copy (CMotion &Dst, CMotion &Src) ;									// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_Motion_h_

#ifndef _Actor_h_
#define	_Actor_h_
// Actor.h : header file
//

#include "ieArchive.h"
#include "FileTags.h"
#include "Key.h"
#include "KeyList.h"

class CRenderContext ;
class CCharacter ;
class CMotion ;
class CMesh ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_ACTOR_END = IE_TAG_ACTOR+1,

	IE_TAG_ACTOR_NAME,
	IE_TAG_ACTOR_CHARACTER,
	IE_TAG_ACTOR_TYPE,
	IE_TAG_ACTOR_DISPLAYPATH,
	IE_TAG_ACTOR_MESH,
	IE_TAG_ACTOR_SCALE,									// No longer used but kept for backward compatibility
	IE_TAG_ACTOR_LAYER,

} ;

/////////////////////////////////////////////////////////////////////////////
//	ACTOR TYPES
enum
{
	ACTOR_CHARACTER		= 0,
	ACTOR_CAMERA,
	ACTOR_LIGHT,
	ACTOR_SCENERY,
	ACTOR_SHOT,
} ;

/////////////////////////////////////////////////////////////////////////////
// CActor

class CActor : public CObject
{
	DECLARE_DYNCREATE(CActor)

// Attributes
public:
	CString				m_Name ;									// Actor Name
	s32					m_Type ;									// Actor Type
	CCharacter			*m_pCharacter ;								// Character to play
	CMesh				*m_pMesh ;									// Mesh to display
	CKeyList			m_Keys ;									// Keyframes
	CKey				*m_pCurKey ;								// Current Keyframe

	s32					m_nCache ;									// Number of Points Cached
	vector3d			*m_pCache ;									// Cache of Points
	s32					m_CacheFirstFrame ;							// First Frame in Cache

	s32					m_Layer ;									// editing/displaying layer

	bool				m_DisplayPath ;								// Display Path in Movie View

	bool				m_Selected ;								// Actor is Selected

// Constructors
public:
	CActor();
	~CActor();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;
	CActor &operator=( CActor & ) ;										// Right side is the argument.

	bool		Create					( ) ;							// Create Actor

	void			SetName				( const char *pName ) ;			// Set Name of Actor
	const char		*GetName			( ) ;							// Get Name of Actor

	void			SetLayer			( int Layer ) ;					// set the actor's layer
	int				GetLayer			( ) ;							// get the actor's layer

	void			SetCharacter		( CCharacter *pCharacter ) ;	// Set Character
	CCharacter		*GetCharacter		( ) ;							// Get Character
	void			SetMesh				( CMesh *pMesh ) ;				// Set Mesh
	CMesh			*GetMesh			( ) ;							// Get Mesh

	CKey			*SetCurKey			( CKey *pKey ) ;				// Set Current Key
	CKey			*GetCurKey			( ) ;							// Get Current Key

	CKey			*KeyNew				( int Frame ) ;					// New Key
	void			KeyDelete			( CKey *pKey ) ;				// Delete Key
	void			KeySelectAll		( bool Select ) ;				// Select All
	void			KeyDrag				( vector3d *pDelta ) ;			// Drag Selected Ones
	bool			KeySyncMotion		( CKey* pKey );					// syncronizes this key's motion with any
																		// neighboring keys with the same motion
	bool			KeySyncDirection	( CKey* pKey );					// syncronizes this key's direction with
																		// neighboring keys which have their direction synced

	void			CalcSpline			( vector3d *p1, vector3d *p2,
										  vector3d *v1, vector3d *v2,
										  int num, vector3d *points) ;	// Calc Spline Segment
	void			CalculateSplineCache( ) ;							// Calculate Cache of Points on Spline
	void			DrawPath			( CRenderContext *pRC,
										  bool Selected ) ;				// Draw path of Points / Keys

	CKey			*GetKeyAtTime		( int t ) ;						// Get Key Active at Time
	CKey			*GetMotionKeyAtTime	( int t ) ;						// Get Motion at Time
	void			GetRotationAtTime	( vector3d *pRot, int t ) ;		// Get Rotation at Time
	f32				GetScaleAtTime		( int t );						// Get scale at time
	void			PoseSkeleton		( int t ) ;						// Pose Skeleton at Time
	void			GetPositionAtTime	( vector3d *pv, int t ) ;		// Get Position at Time
	void			GetDirectionAtTime	( vector3d *pv, int t ) ;		// Get Direction at Time
	void			GetOrientationAtTime( radian *pYRot, int t ) ;		// Get Orientation at Time
	void			GetMotionInfoAtTime	( CMotion **ppMotion,
										  int *pnFrame,
										  vector3d *pDeltaMove,
										  radian *pFaceDir,
										  radian *pMoveDir,
										  int t ) ;						// Get Motion at Time t

	void			InsertFrames		( int t, int nFrames ) ;		// Insert nFrames at Time t
	void			DeleteFrames		( int t, int nFrames ) ;		// Delete nFrames at Time t

	s32				SetType				( int Type ) ;					// Set Type
	s32				GetType				( ) ;							// Get Type

	void			MotionDeleted		( CMotion *pMotion ) ;			// Motion was Deleted

// Implementation
protected:
	void Initialize() ;													// Initialize Data
	void Destroy () ;													// Destroy Actor
	void Copy (CActor &Dst, CActor &Src) ;								// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_Actor_h_

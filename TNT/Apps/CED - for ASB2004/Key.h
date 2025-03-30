#ifndef _KeyPosition_h_
#define	_KeyPosition_h_
// KeyPosition.h : header file
//

#include "ieArchive.h"
#include "FileTags.h"
#include "Character.h"
#include "x_math.h"

class CActor ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_KEY_END = IE_TAG_KEY+1,

	IE_TAG_KEY_NAME,
	IE_TAG_KEY_FRAME,
	IE_TAG_KEY_POS,
	IE_TAG_KEY_VECTOR,
	IE_TAG_KEY_AMCBASED,
	IE_TAG_KEY_AUTOLINKPOS,

	IE_TAG_KEY_MOTION,
	IE_TAG_KEY_STARTFRAME,
	IE_TAG_KEY_CHARACTER,
	IE_TAG_KEY_EVENT,

	IE_TAG_KEY_ROTATION,
	IE_TAG_KEY_SCALE,
	IE_TAG_KEY_VISIBLE,
	IE_TAG_KEY_CAMERALOCK,

	IE_TAG_KEY_FRAMERATE,
	IE_TAG_KEY_BLENDFRAMES,
	IE_TAG_KEY_MIRROR,

	IE_TAG_KEY_AUTOLINKDIR

	// ADD NEW ONES TO THE END ALWAYS!!!
} ;

/////////////////////////////////////////////////////////////////////////////
// CKeyPosition

class CKey : public CObject
{
	DECLARE_DYNCREATE(CKey)

// Attributes
public:
	CActor			*m_pActor ;											// Pointer to Actor
	CCharacter		*m_pCharacter ;										// Pointer to Character

	CString			m_Name ;											// Key Name
	vector3d		m_Position ;										// Position
	vector3d		m_Vector ;											// Vector at Position
	vector3d		m_Rotation ;										// Rotation at Position
	s32				m_Frame ;											// Frame of KeyFrame
	bool			m_Selected ;										// Is Selected

	bool			m_AMCbased ;										// Position is AMC based
	bool			m_AutoLinkPos ;										// AutoLink the position
	bool			m_AutoLinkDir ;										// AutoLink the direction
	bool			m_Visible ;											// Is this scenery object visible?
	bool			m_CameraLock;										// Is this scenery object camera relative?

	CMotion			*m_pMotion ;										// Motion Pointer
	s32				m_StartFrame ;										// Start Frame in Motion
	f32				m_FrameRate ;										// Frame rate to play motion
	f32				m_BlendFrames ;										// Number of frames to blend motion
	bool			m_Mirror ;											// Should animation be mirrored

	u16				m_Event ;											// Event Number
	f32				m_Scale ;											// Scale for scenery object key


// Constructors
public:
	CKey();
	~CKey();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;
	CKey &operator=( CKey & ) ;											// Right side is the argument.

	void		SetName					( const char *pName ) ;
	const char	*GetName				( ) ;

	void		SetActor				( CActor *pActor ) ;
	CActor		*GetActor				( ) ;

	void		SetPosition				( vector3d *pv ) ;
	void		GetPosition				( vector3d *pv ) ;

	void		SetVector				( vector3d *pv ) ;
	void		GetVector				( vector3d *pv ) ;
	
	void		SetRotation				( vector3d *pv ) ;
	void		GetRotation				( vector3d *pv ) ;
	
	void		SetFrame				( int Frame ) ;
	int			GetFrame				( ) ;
	
	void		SetCharacter			( CCharacter *pCharacter ) ;
	CCharacter	*GetCharacter			( ) ;
	
	void		SetMotion				( CMotion *pMotion ) ;
	CMotion		*GetMotion				( ) ;
	
	void		SetStartFrame			( int StartFrame ) ;
	int			GetStartFrame			( ) ;

	void		SetFrameRate			( float FrameRate ) ;
	float		GetFrameRate			( ) ;

	void		SetBlendFrames			( float BlendFrames ) ;
	float		GetBlendFrames			( ) ;

	void		SetMirror				( bool Mirror ) ;
	bool		GetMirror				( ) ;

	void		SetEvent				( u16 Event ) ;
	u16			GetEvent				( ) ;

	void		SetScale				( f32 scale );
	f32			GetScale				( ) ;

// Implementation
protected:
	void Initialize() ;													// Initialize Data
	void Destroy () ;													// Destroy Key
	void Copy (CKey &Dst, CKey &Src) ;									// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_Key_h_

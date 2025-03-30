#ifndef _Shot_h_
#define	_Shot_h_
// Shot.h : header file
//

#include "ieArchive.h"
#include "FileTags.h"
#include "x_math.h"

class CActor ;
class CMovie ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_SHOT_END = IE_TAG_SHOT+1,

	IE_TAG_SHOT_NAME,
	IE_TAG_SHOT_FRAME,
	IE_TAG_SHOT_EYE,
	IE_TAG_SHOT_TARGET,
	IE_TAG_SHOT_FOV,
	IE_TAG_SHOT_SPEED,
} ;

/////////////////////////////////////////////////////////////////////////////
// CShot

class CShot : public CObject
{
	DECLARE_DYNCREATE(CShot)

// Attributes
public:
	CString			m_Name ;											// Shot Name

	s32				m_Frame ;											// Frame
	s32				m_FOV;												// Field of view in degrees
	s32				m_Speed ;											// Percent Speed of playback on this shot
	CActor			*m_pEye ;											// Eye Actor
	CActor			*m_pTarget ;										// Target Actor

	CMovie			*m_pMovie ;											// Movie Shot belongs to

// Constructors
public:
	CShot();
	~CShot();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;
	CShot &operator=( CShot & ) ;											// Right side is the argument.

	void		SetName					( const char *pName ) ;
	const char	*GetName				( ) ;

	void		SetFrame				( s32 Frame ) ;
	s32			GetFrame				( ) ;

	void		SetFOV					( s32 Degrees );
	s32			GetFOV					( );

	void		SetSpeed				( s32 Speed );
	s32			GetSpeed				( );

	void		SetEye					( CActor *pActor ) ;
	CActor		*GetEye					( ) ;

	void		SetTarget				( CActor *pActor ) ;
	CActor		*GetTarget				( ) ;

	void		SetMovie				( CMovie *pMovie ) ;
	CMovie		*GetMovie				( ) ;
// Implementation
protected:
	void Initialize() ;													// Initialize Data
	void Destroy () ;													// Destroy Shot
	void Copy (CShot &Dst, CShot &Src) ;									// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_Shot_h_

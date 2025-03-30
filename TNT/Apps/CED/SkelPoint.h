#ifndef _SkelPoint_h_
#define	_SkelPoint_h_
// SkelPoint.h : header file
//

#include "x_color.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_memory.hpp"

#include "ieArchive.h"

#include "FileTags.h"

class CMotion ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_SKELPOINT_END = IE_TAG_SKELPOINT+1,

	IE_TAG_SKELPOINT_NAME,
	IE_TAG_SKELPOINT_POSITION,
	IE_TAG_SKELPOINT_RADIUS,
	IE_TAG_SKELPOINT_BONE,
} ;

/////////////////////////////////////////////////////////////////////////////
// CSkelPoint

class CSkelPoint : public CObject
{
	DECLARE_DYNCREATE(CSkelPoint)

// Attributes
public:

private:
	CString			m_Name ;								// SkelPoint Name

	vector3 		m_Position ;							// Position of SkelPoint
	s32				m_iBone ;								// Relative to Bone
	f32				m_Radius ;								// SkelPoint Radius

// Constructors
public:
	CSkelPoint();
	~CSkelPoint();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;
	CSkelPoint &operator=( CSkelPoint & ) ;							// Right side is the argument.

	bool		Create () ;											// Create SkelPoint

	void		SetName( const char *pName ) ;						// Set Name of SkelPoint
	const char	*GetName( ) ;										// Get Name of SkelPoint

	void		GetPosition( vector3  *pVector ) ;					// Get the Position
	void		SetPosition( vector3  *pVector ) ;					// Get the Position

	int			GetBone( ) ;										// Get Bone to be relative to
	void		SetBone( int iBone ) ;								// Set Bone to be relative to

	f32			GetRadius( ) ;										// Get Radius
	void		SetRadius( f32 Radius ) ;							// Set Radius

// Implementation
protected:
	void Initialize() ;												// Initialize Data
	void Destroy () ;												// Destroy SkelPoint
	void Copy (CSkelPoint &Dst, CSkelPoint &Src) ;					// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_SkelPoint_h_

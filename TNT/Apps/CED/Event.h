#ifndef _Event_h_
#define	_Event_h_
// Event.h : header file
//

#include "x_color.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_memory.hpp"

#include "ieArchive.h"

#include "FileTags.h"

class CMotion ;
class CSkeleton ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_EVENT_END = IE_TAG_EVENT+1,

	IE_TAG_EVENT_NAME,
	IE_TAG_EVENT_POSITION,
	IE_TAG_EVENT_RADIUS,
	IE_TAG_EVENT_FRAME,

	//---	attached to bone information
	IE_TAG_EVENT_BONE,
	IE_TAG_EVENT_DIST,
	IE_TAG_EVENT_ROT,
	IE_TAG_EVENT_FORCEX,
	IE_TAG_EVENT_FORCEY,
	IE_TAG_EVENT_FORCEZ,
	IE_TAG_EVENT_FORCEPOS,
	IE_TAG_EVENT_FORCERELJOINT,
	IE_TAG_EVENT_BONENAME,
	IE_TAG_EVENT_ATTACHED,
	IE_TAG_EVENT_LINK
} ;

/////////////////////////////////////////////////////////////////////////////
// CEvent

class CEvent : public CObject
{
	DECLARE_DYNCREATE(CEvent)

// Attributes
public:

private:
	CString			m_Name ;								// Event Name

	CMotion			*m_pMotion ;							// Pointer to Motion
	s32				m_iFrame ;								// Frame Index
	vector3 		m_Position ;							// Position of Event
	f32				m_Radius ;								// Event Radius

	//---	attached to bone information
	bool			m_Attached ;							// Event is attached to a bone
	s32				m_Link ;								// 0 - person, 1..8 which linked motion (prop) is it attached to.
	CString			m_Bone ;								// use name unless equal to "<no name>" otherwise use m_iBone for backward compatability
	s32				m_iBone ;								// Relative to Bone
	f32				m_Distance ;							// Distance from attached bone
	vector3 		m_Rot ;									// Rotation relative to attached bone
	bool			m_bForceX ;								// T/F is the X position forced
	bool			m_bForceY ;								// T/F is the Y position forced
	bool			m_bForceZ ;								// T/F is the Z position forced
	vector3 		m_ForcePos ;							// Forced positions
	bool			m_bForcePosRel ;						// T/F is the forced position relative to the joint (FALSE - Relative to origin)

	//---	non-saved values used at run-time
	bool			m_bModified ;							// T/F has this event been modified
	CString			m_OldName ;								// Name of the event before it was changed

// Constructors
public:
	CEvent();
	~CEvent();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;
	CEvent &operator=( CEvent & ) ;															// copy
	CEvent &operator<=( CEvent & ) ;														// copy event specific
																							
	bool			Create () ;																// Create Event
																							
	void			SetName( const char *pName ) ;											// Set Name of Event
	const char		*GetName( ) ;															// Get Name of Event
																							
	CMotion			*GetMotion( ) ;															// Get Motion Pointer
	void			SetMotion( CMotion *pMotion ) ;											// Set Motion Pointer
																							
	int				GetFrame( ) ;															// Get Frame
	void			SetFrame( int iFrame ) ;												// Set Frame
																							
	void			GetPosition( vector3  *pVector ) ;										// Get the Position
	void			SetPosition( vector3  *pVector ) ;										// Get the Position

	bool			GetAttached( )					{ return m_Attached; }					// T/F is the event attached to a bone
	void			SetAttached( bool Set );
	s32				GetLink()						{ return m_Link; }						// if this event is linked to a character, the value will be a non-zero index
	void			SetLink( s32 Link );
	CString			GetBoneName( )					{ return m_Bone; }						// Get the attached bone name
	void			SetBoneName( CString& Bone );
	int				GetBone( void );
	void			SetBone( int iBone );
													
	f32				GetRadius( ) 					{ return m_Radius; }					// Get Radius
	void			SetRadius( f32 Radius ) 		{ m_Radius = Radius; SetModified(); }	// Set Radius
																							
	const vector3 &	GetAttRot( ) 					{ return m_Rot; }						// Get Attached Rotation
	void			SetAttRot( vector3 & Rot )		{ m_Rot = Rot; SetModified(); }			// Set Attached Rotation
																							
	f32				GetAttDist( ) 					{ return m_Distance; }					// Get Attached Distance
	void			SetAttDist( f32 Dist )			{ m_Distance = Dist; SetModified(); }	// Set Attached Distance
																							
	bool			GetAttForceX( )					{ return m_bForceX; }					// Get Attached Force Position X
	void			SetAttForceX( bool Set )		{ m_bForceX = Set; SetModified(); }		// Set Attached Force Position X
																							
	bool			GetAttForceY( )					{ return m_bForceY; }					// Get Attached Force Position Y
	void			SetAttForceY( bool Set )		{ m_bForceY = Set; SetModified(); }		// Set Attached Force Position Y
																							
	bool			GetAttForceZ( )					{ return m_bForceZ; }					// Get Attached Force Position Z
	void			SetAttForceZ( bool Set )		{ m_bForceZ = Set; SetModified(); }		// Set Attached Force Position Z
																							
	const vector3 &	GetAttForcePos( )				{ return m_ForcePos; }					// Get Attached Force Position
	void			SetAttForcePos( vector3 & rPos ){ m_ForcePos = rPos; SetModified(); }	// Set Attached Force Position

	bool			GetAttForceRelToJoint( )		{ return m_bForcePosRel; }				// Get Attached Force Position Relative to Joint
	void			SetAttForceRelToJoint( bool Set ){ m_bForcePosRel = Set; SetModified();}// Set Attached Force Position Relative to Joint

	bool			UpdateEventPosition( void );
//	bool			ProjectEventPosition( vector3 & rPos,
//										  CSkeleton *pSkeleton,
//										  CMotion *pMotion );

	bool			GetModified(  )					{ return m_bModified; }
	void			SetModified( bool Set=TRUE )	{ m_bModified = Set; }

	CString&		GetOldName( )					{ return m_OldName; }
	void			SetOldName( CString OldName )	{ m_OldName = OldName; }

// Implementation
protected:
	void Initialize() ;												// Initialize Data
	void Destroy () ;												// Destroy Event
	void Copy (CEvent &Dst, CEvent &Src) ;							// Copy
	void CopyEventSpecific (CEvent &Dst, CEvent &Src) ;				// Copy data specific to the event and not the motion that it is tied to

	void ApplyEventMatrix( matrix4* DstMatrices, matrix4* BoneMatrix);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_Event_h_

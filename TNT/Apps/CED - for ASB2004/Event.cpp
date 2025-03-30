// Event.cpp : implementation file
//

#include "stdafx.h"

#include "Event.h"
#include "ieArchive.h"
#include "Motion.h"
#include "Skeleton.h"
#include "Character.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEvent Implementation

IMPLEMENT_DYNCREATE(CEvent, CObject)

/////////////////////////////////////////////////////////////////////////////
// CEvent Constructors

void CEvent::Initialize()
{
	m_Name = "New Event" ;
	V3_Set( &m_Position, 0, 0, 0 ) ;
	m_Attached = FALSE;
	m_Bone = "<no name>";
	m_iBone = -1 ;
	m_iFrame = 0 ;
	m_Radius = 10 ;
	m_pMotion = NULL ;
	m_Distance = 0.0f ;
	m_Rot.X = m_Rot.Y = m_Rot.Z = 0.0f ;
	m_bForceX = m_bForceY =	m_bForceZ = FALSE ;
	m_ForcePos.X = m_ForcePos.Y = m_ForcePos.Z = 0.0f ; 
}

CEvent::CEvent()
{
	//---	Reset Data Members
	Initialize () ;
}

CEvent::~CEvent()
{
	//---	Destroy the Texture
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CEvent Implementation

bool CEvent::Create()
{
	return true ;
}

void CEvent::Destroy()
{
	//---	Initialize Data Members
	Initialize () ;
}

void CEvent::Copy (CEvent &Dst, CEvent &Src)
{
	CopyEventSpecific( Dst, Src );

	Dst.m_pMotion = Src.m_pMotion;
	Dst.m_iFrame = Src.m_iFrame;
	V3_Copy( &Dst.m_Position, &Src.m_Position );
}

void CEvent::CopyEventSpecific (CEvent &Dst, CEvent &Src)
{
	Dst.m_Name = Src.m_Name;
	Dst.m_Attached = Src.m_Attached;
	Dst.m_Bone = Src.m_Bone;
	Dst.m_iBone = Src.m_iBone;
	Dst.m_Radius = Src.m_Radius;
	Dst.m_Rot = Src.m_Rot;
	Dst.m_Distance = Src.m_Distance;
	Dst.m_bForceX = Src.m_bForceX;
	Dst.m_bForceY = Src.m_bForceY;
	Dst.m_bForceZ = Src.m_bForceZ;
	Dst.m_ForcePos = Src.m_ForcePos;
	Dst.m_bForcePosRel = Src.m_bForcePosRel;
	Dst.m_Position = Src.m_Position;
}

/////////////////////////////////////////////////////////////////////////////
// CEvent Overridden operators

CEvent &CEvent::operator=( CEvent &ptEvent )
{
	Copy (*this, ptEvent) ;

    return *this;  // Assignment operator returns left side.
}

CEvent &CEvent::operator<=( CEvent &ptEvent )
{
	CopyEventSpecific (*this, ptEvent) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CEvent serialization

void CEvent::Serialize(CieArchive& a)
{
//	CScrEdDoc *pDoc = (CScrEdDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_EVENT) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_EVENT_NAME, m_Name ) ;
		a.WriteTaggedvector3d( IE_TAG_EVENT_POSITION, m_Position ) ;
		a.WriteTaggedf32( IE_TAG_EVENT_RADIUS, m_Radius ) ;
		a.WriteTaggeds32( IE_TAG_EVENT_FRAME, m_iFrame ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_ATTACHED, m_Attached ) ;
		a.WriteTaggedString( IE_TAG_EVENT_BONENAME, m_Bone ) ;
		a.WriteTaggeds32( IE_TAG_EVENT_BONE, m_iBone ) ;
		a.WriteTaggedf32( IE_TAG_EVENT_DIST, m_Distance ) ;
		a.WriteTaggedvector3d( IE_TAG_EVENT_ROT, m_Rot ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_FORCEX, m_bForceX ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_FORCEY, m_bForceY ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_FORCEZ, m_bForceZ ) ;
		a.WriteTaggedvector3d( IE_TAG_EVENT_FORCEPOS, m_ForcePos ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_FORCERELJOINT, m_bForcePosRel ) ;

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_EVENT_END ) ;
	}
	else
	{
		bool AttachedRead=FALSE;
		//---	Load
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_EVENT_NAME:			a.ReadString(m_Name) ;			break ;
			case IE_TAG_EVENT_POSITION:		a.Readvector3d(m_Position) ;	break ;
			case IE_TAG_EVENT_RADIUS:		a.Readf32(m_Radius) ;			break ;
			case IE_TAG_EVENT_FRAME:		a.Reads32(m_iFrame) ;			break ;
			case IE_TAG_EVENT_ATTACHED:		a.Readbool(m_Attached) ;AttachedRead=TRUE;	break ;
			case IE_TAG_EVENT_BONENAME:		a.ReadString(m_Bone) ;			break ;
			case IE_TAG_EVENT_BONE:			a.Reads32(m_iBone) ;			break ;
			case IE_TAG_EVENT_DIST:			a.Readf32(m_Distance) ;			break ;
			case IE_TAG_EVENT_ROT:			a.Readvector3d(m_Rot) ;			break ;
			case IE_TAG_EVENT_FORCEX:		a.Readbool(m_bForceX) ;			break ;
			case IE_TAG_EVENT_FORCEY:		a.Readbool(m_bForceY) ;			break ;
			case IE_TAG_EVENT_FORCEZ:		a.Readbool(m_bForceZ) ;			break ;
			case IE_TAG_EVENT_FORCEPOS:		a.Readvector3d(m_ForcePos) ;	break ;
			case IE_TAG_EVENT_FORCERELJOINT:a.Readbool(m_bForcePosRel) ;	break ;

			case IE_TAG_EVENT_END:			done = TRUE ;					break ;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}

		//---	if the bone identifier is set, it is attached (m_iBone is kept for backward compatability)
		if( !AttachedRead )
			m_Attached = ( m_iBone != -1 ) ? TRUE : FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEvent diagnostics

#ifdef _DEBUG
void CEvent::AssertValid() const
{
	CObject::AssertValid();
}

void CEvent::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CEvent operations

void CEvent::SetName( const char *pName )
{
	m_Name = pName ;
}

const char *CEvent::GetName( )
{
	return m_Name ;
}

CMotion	*CEvent::GetMotion( )
{
	return m_pMotion ;
}

void CEvent::SetMotion( CMotion *pMotion )
{
	m_pMotion = pMotion ;
}

int CEvent::GetFrame( )
{
	int iFrame = m_iFrame ; 
	return iFrame ;
}

void CEvent::SetFrame( int iFrame )
{
	m_iFrame = iFrame ;
}

void CEvent::GetPosition( vector3d *pVector )
{
	V3_Copy( pVector, &m_Position ) ;
}

void CEvent::SetPosition( vector3d *pVector )
{
	V3_Copy( &m_Position, pVector ) ;
}

bool CEvent::UpdateEventPosition( void )
{
	//---	if this event is not attached, return now
	if( !GetAttached() )
		return FALSE;

	//---	look for the attached bone
	CSkeleton* pSkeleton = GetMotion()->m_pCharacter->m_pSkeleton;
	s32 iBone = GetBone();
	if(( iBone == -1 ) && ( GetBoneName() != "<no bone>" ))
		iBone = pSkeleton->FindBoneFromName( GetBoneName() );

	//---	if the bone couldn't be found, exit now
	if( iBone == -1 )
		return FALSE;

	//---	get the event's projected location from the bone
	vector3d Pos;
	matrix4* pBoneMatrix;
	matrix4 EventMatrix;
	pSkeleton->SetPose( GetMotion(), (float)GetFrame() );
	pBoneMatrix = pSkeleton->GetPosedBoneMatrix( iBone );
	ApplyEventMatrix( &EventMatrix, pBoneMatrix );
	M4_GetTranslation( &EventMatrix, &Pos ) ;

	//---	apply any forced positions now
	if( GetAttForceRelToJoint() )
	{
		vector3d BonePos;
		M4_GetTranslation( pBoneMatrix, &BonePos );
		if( GetAttForceX() )	Pos.X = BonePos.X + GetAttForcePos().X;
		if( GetAttForceY() )	Pos.Y = BonePos.Y + GetAttForcePos().Y;
		if( GetAttForceZ() )	Pos.Z = BonePos.Z + GetAttForcePos().Z;
	}
	else
	{
		if( GetAttForceX() )	Pos.X = GetAttForcePos().X;
		if( GetAttForceY() )	Pos.Y = GetAttForcePos().Y;
		if( GetAttForceZ() )	Pos.Z = GetAttForcePos().Z;
	}

	//---	set the event's position
	SetPosition( &Pos );

	return TRUE;
}

void CEvent::ApplyEventMatrix( matrix4* DstMatrices, matrix4* BoneMatrix )
{
    radian3d LR;
    vector3d LT;

    // Get local rotation and translation info
    LT.X = LT.Y = 0.0f;
	LT.Z = GetAttDist();

	LR.X = GetAttRot().X;
	LR.Y = GetAttRot().Y;
	LR.Z = GetAttRot().Z;

    // Build matrix local to world matrix bone
    M4_Identity         (DstMatrices);
    M4_SetRotationsXYZ  (DstMatrices,&LR);
    M4_TranslateOn      (DstMatrices,&LT);
    M4_PreMultOn        (DstMatrices,BoneMatrix);
}

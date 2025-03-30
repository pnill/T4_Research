// Event.cpp : implementation file
//

#include "stdafx.h"

#include "Event.h"
#include "ieArchive.h"
#include "Motion.h"
#include "Skeleton.h"
#include "Character.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	NO_BONE_NAME	"<no name>"

/////////////////////////////////////////////////////////////////////////////
// CEvent Implementation

IMPLEMENT_DYNCREATE(CEvent, CObject)

/////////////////////////////////////////////////////////////////////////////
// CEvent Constructors

void CEvent::Initialize()
{
	m_Name = "New Event" ;
	m_Position.Zero();
//	V3_Set( &m_Position, 0, 0, 0 ) ;
	m_Attached	= FALSE;
	m_Link		= 0;
	m_Bone		= NO_BONE_NAME;
	m_iBone		= -1 ;
	m_iFrame	= 0 ;
	m_Radius	= 10 ;
	m_pMotion	= NULL ;
	m_Distance	= 0.0f ;
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
	Dst.m_Position = Src.m_Position;
//	V3_Copy( &Dst.m_Position, &Src.m_Position );
}

void CEvent::CopyEventSpecific (CEvent &Dst, CEvent &Src)
{
	Dst.m_Name			= Src.m_Name;
	Dst.m_Attached		= Src.m_Attached;
	Dst.m_Link			= Src.m_Link;
	Dst.m_Bone			= Src.m_Bone;
	Dst.m_iBone			= Src.m_iBone;
	Dst.m_Radius		= Src.m_Radius;
	Dst.m_Rot			= Src.m_Rot;
	Dst.m_Distance		= Src.m_Distance;
	Dst.m_bForceX		= Src.m_bForceX;
	Dst.m_bForceY		= Src.m_bForceY;
	Dst.m_bForceZ		= Src.m_bForceZ;
	Dst.m_ForcePos		= Src.m_ForcePos;
	Dst.m_bForcePosRel	= Src.m_bForcePosRel;
	Dst.m_Position		= Src.m_Position;
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
		a.WriteTaggedvector3( IE_TAG_EVENT_POSITION, m_Position ) ;
		a.WriteTaggedf32( IE_TAG_EVENT_RADIUS, m_Radius ) ;
		a.WriteTaggeds32( IE_TAG_EVENT_FRAME, m_iFrame ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_ATTACHED, m_Attached ) ;
		a.WriteTaggedString( IE_TAG_EVENT_BONENAME, m_Bone ) ;
		a.WriteTaggeds32( IE_TAG_EVENT_BONE, m_iBone ) ;
		a.WriteTaggedf32( IE_TAG_EVENT_DIST, m_Distance ) ;
		a.WriteTaggedvector3( IE_TAG_EVENT_ROT, m_Rot ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_FORCEX, m_bForceX ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_FORCEY, m_bForceY ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_FORCEZ, m_bForceZ ) ;
		a.WriteTaggedvector3( IE_TAG_EVENT_FORCEPOS, m_ForcePos ) ;
		a.WriteTaggedbool( IE_TAG_EVENT_FORCERELJOINT, m_bForcePosRel ) ;
		a.WriteTaggeds32( IE_TAG_EVENT_LINK, m_Link );

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
			case IE_TAG_EVENT_POSITION:		a.Readvector3(m_Position) ;		break ;
			case IE_TAG_EVENT_RADIUS:		a.Readf32(m_Radius) ;			break ;
			case IE_TAG_EVENT_FRAME:		a.Reads32(m_iFrame) ;			break ;
			case IE_TAG_EVENT_ATTACHED:		a.Readbool(m_Attached) ;AttachedRead=TRUE;	break ;
			case IE_TAG_EVENT_BONENAME:		a.ReadString(m_Bone) ;			break ;
			case IE_TAG_EVENT_BONE:			a.Reads32(m_iBone) ;			break ;
			case IE_TAG_EVENT_DIST:			a.Readf32(m_Distance) ;			break ;
			case IE_TAG_EVENT_ROT:			a.Readvector3(m_Rot) ;			break ;
			case IE_TAG_EVENT_FORCEX:		a.Readbool(m_bForceX) ;			break ;
			case IE_TAG_EVENT_FORCEY:		a.Readbool(m_bForceY) ;			break ;
			case IE_TAG_EVENT_FORCEZ:		a.Readbool(m_bForceZ) ;			break ;
			case IE_TAG_EVENT_FORCEPOS:		a.Readvector3(m_ForcePos) ;		break ;
			case IE_TAG_EVENT_FORCERELJOINT:a.Readbool(m_bForcePosRel) ;	break ;
			case IE_TAG_EVENT_LINK:			a.Reads32(m_Link) ;				break ;

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

	UpdateEventPosition();
}

void CEvent::GetPosition( vector3 *pVector )
{
	*pVector  = m_Position;
}

void CEvent::SetPosition( vector3 *pVector )
{
	m_Position = *pVector;
}

void CEvent::SetAttached( bool Set )
{
	m_Attached = Set;
	SetModified();
}

void CEvent::SetLink( s32 Link )
{
	m_Link = Link;
	m_iBone	= -1;
	SetModified();
}

void CEvent::SetBoneName( CString& Bone )
{
	m_Bone = Bone;
	m_iBone	= -1;
	SetModified();
}

void CEvent::SetBone( int iBone )
{
	CSkeleton*	pSkeleton = NULL;

	m_iBone = iBone;
	SetModified();

	if( m_iBone == -1 )
	{
		m_Bone = NO_BONE_NAME;
		return;
	}
	
	//---	look for the attached bone
	if( GetLink() )
	{
		const CCharacter	*pLinkCharacter = GetMotion()->GetProp(GetLink()-1).GetLinkedCharacter();
		if( pLinkCharacter )
			pSkeleton	= pLinkCharacter->m_pSkeleton;
	}
	else
		pSkeleton	= GetMotion()->m_pCharacter->m_pSkeleton;

	if( pSkeleton )
	{
		SSkelBone* pBone = pSkeleton->GetBoneFromIndex( m_iBone );

		if( pBone )
			m_Bone.Format( "%s", pBone->BoneName );
		else
			m_Bone.Format( "%s", NO_BONE_NAME );
		m_iBone = -1;
	}
}

int CEvent::GetBone( void )
{
	//---	if the bone hasn't yet been found, find it using the name
	if( (m_iBone == -1) && GetMotion() )
	{
		CSkeleton*	pSkeleton = NULL;
		
		//---	look for the attached bone
		if( GetLink() )
		{
			const CCharacter	*pLinkCharacter = GetMotion()->GetProp(GetLink()-1).GetLinkedCharacter();
			if( pLinkCharacter )
				pSkeleton	= pLinkCharacter->m_pSkeleton;
		}
		else
		{
			if( GetMotion()->m_pCharacter )
				pSkeleton	= GetMotion()->m_pCharacter->m_pSkeleton;
		}

		if( pSkeleton )
			m_iBone = pSkeleton->FindBoneFromName( m_Bone );
	}

	return m_iBone;
}


bool CEvent::UpdateEventPosition( void )
{
	CSkeleton*	pSkeleton;
	CMotion*	pMotion;
	s32			iBone;
	vector3		Pos;
	matrix4*	pBoneMatrix;
	matrix4		EventMatrix;
	matrix4		Identity;

	//---	if this event is not attached, return now
	if( !GetAttached() )
		return FALSE;

	//---	look for the attached bone
	if( GetLink() )
	{
		const CCharacter *pLinkCharacter = GetMotion()->GetProp(GetLink()-1).GetLinkedCharacter();
		if( pLinkCharacter )
		{
			pSkeleton	= pLinkCharacter->m_pSkeleton;
			pMotion		= (CMotion*)GetMotion()->GetProp(GetLink()-1).GetLinkedMotion();
		}
	}
	else
	{
		pSkeleton	= GetMotion()->m_pCharacter->m_pSkeleton;
		pMotion		= GetMotion();
	}

	//---	if the prop linked to has a motion and a skeleton, move the event there
	//		otherwise, put it on the prop at this point in its stream.
	if( pSkeleton && pMotion )
	{
		iBone = GetBone();
		if(( iBone == -1 ) && ( GetBoneName() != "<no bone>" ))
			iBone = pSkeleton->FindBoneFromName( GetBoneName() );

		//---	if the bone couldn't be found, exit now
		if( iBone == -1 )
		{
			Pos.Set( 0.0f, 0.0f, 0.0f );
			SetPosition( &Pos );
			return FALSE;
		}

		//---	get the event's projected location from the bone
		Identity.Identity();
		pSkeleton->SetPose( Identity, pMotion, (float)GetFrame() );
		pBoneMatrix = pSkeleton->GetPosedBoneMatrix( iBone );
		ApplyEventMatrix( &EventMatrix, pBoneMatrix );
		Pos = EventMatrix.GetTranslation();
	}
	else
	{
		ASSERTS(0, "Not yet implemented");
	}

	//---	apply any forced positions now
	if( GetAttForceRelToJoint() )
	{
		vector3 BonePos;
		BonePos = pBoneMatrix->GetTranslation();
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
    radian3 LR;
    vector3 LT;

    // Get local rotation and translation info
    LT.X = LT.Y = 0.0f;
	LT.Z = GetAttDist();

	LR.Pitch	= GetAttRot().X;
	LR.Yaw		= GetAttRot().Y;
	LR.Roll		= GetAttRot().Z;

    // Build matrix local to world matrix bone
	DstMatrices->Identity();
	DstMatrices->SetRotationXYZ(LR);
	DstMatrices->Translate(LT);
	DstMatrices->Transform(*BoneMatrix); // The new Transform() function matches the old M4_PreMultOn() function
//    M4_Identity         (DstMatrices);
//    M4_SetRotationsXYZ  (DstMatrices,&LR);
//    M4_TranslateOn      (DstMatrices,&LT);
//    M4_PreMultOn        (DstMatrices,BoneMatrix);
}

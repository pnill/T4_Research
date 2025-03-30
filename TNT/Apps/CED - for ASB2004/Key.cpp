// Key.cpp : implementation file
//

#include "stdafx.h"

#include "Key.h"
#include "ieArchive.h"
#include "CeDDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyPosition Implementation

IMPLEMENT_DYNCREATE(CKey, CObject)

/////////////////////////////////////////////////////////////////////////////
// CKeyPosition Constructors

void CKey::Initialize()
{
	m_pCharacter	= NULL ;
	m_pActor		= NULL ;
	m_Name			= "Key" ;
	m_Selected		= false ;
	m_AMCbased		= false ;
	m_AutoLinkPos	= false ;
	m_AutoLinkDir	= false ;
	m_pMotion		= NULL ;
	m_StartFrame	= 0 ;
	m_FrameRate		= 1.0f ;
	m_BlendFrames	= 0.0f ;
	m_Mirror		= false ;
	m_Event			= 0 ;
	m_Scale			= 1.0f;
	m_Visible		= true;
	m_CameraLock	= false;
	V3_Zero( &m_Position ) ;
	V3_Zero( &m_Vector ) ;
	V3_Zero( &m_Rotation ) ;
}

CKey::CKey()
{
	//---	Reset Data Members
	Initialize () ;
}

CKey::~CKey()
{
	//---	Destroy
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CKey Implementation

void CKey::Destroy()
{
	//---	Initialize Data Members
	Initialize () ;
}

void CKey::Copy (CKey &Dst, CKey &Src)
{
	Dst.m_pActor		= Src.m_pActor ;
	Dst.m_Name			= Src.m_Name ;
	Dst.m_Position		= Src.m_Position ;
	Dst.m_Vector		= Src.m_Vector ;
	Dst.m_Frame			= Src.m_Frame ;
	Dst.m_Selected		= false ;
	Dst.m_AMCbased		= Src.m_AMCbased ;
	Dst.m_AutoLinkPos	= Src.m_AutoLinkPos ;
	Dst.m_AutoLinkDir	= Src.m_AutoLinkDir ;
	Dst.m_pCharacter	= Src.m_pCharacter ;
	Dst.m_pMotion		= Src.m_pMotion ;
	Dst.m_StartFrame	= Src.m_StartFrame ;
	Dst.m_FrameRate		= Src.m_FrameRate ;
	Dst.m_BlendFrames	= Src.m_BlendFrames ;
	Dst.m_Mirror		= Src.m_Mirror ;
	Dst.m_Scale			= Src.m_Scale;
	Dst.m_Visible		= Src.m_Visible;
	Dst.m_CameraLock	= Src.m_CameraLock;
}

/////////////////////////////////////////////////////////////////////////////
// CKey Overridden operators

CKey &CKey::operator=( CKey &ptKey )
{
	Copy (*this, ptKey) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CKey serialization

void CKey::Serialize(CieArchive& a)
{
	CCeDDoc *pDoc = (CCeDDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_KEY) ;

		//---	Write Details
		a.WriteTaggedString		( IE_TAG_KEY_NAME,			m_Name ) ;
		a.WriteTaggeds32		( IE_TAG_KEY_FRAME,			m_Frame ) ;
		a.WriteTaggedvector3d	( IE_TAG_KEY_POS,			m_Position ) ;
		a.WriteTaggedvector3d	( IE_TAG_KEY_VECTOR,		m_Vector ) ;
		a.WriteTaggedvector3d	( IE_TAG_KEY_ROTATION,		m_Rotation ) ;
		a.WriteTaggedbool		( IE_TAG_KEY_AMCBASED,		m_AMCbased ) ;
		a.WriteTaggedbool		( IE_TAG_KEY_AUTOLINKPOS,	m_AutoLinkPos ) ;
		a.WriteTaggedbool		( IE_TAG_KEY_AUTOLINKDIR,	m_AutoLinkDir ) ;
		a.WriteTaggedu16		( IE_TAG_KEY_EVENT,			m_Event ) ;
		a.WriteTaggedf32		( IE_TAG_KEY_SCALE,			m_Scale );
		a.WriteTaggedbool		( IE_TAG_KEY_VISIBLE,		m_Visible );
		a.WriteTaggedbool		( IE_TAG_KEY_CAMERALOCK,	m_CameraLock);

		if( m_pCharacter )
		{
			s32	Index = pDoc->m_CharacterList.PtrToIndex(	m_pCharacter ) ;
			a.WriteTaggeds32( IE_TAG_KEY_CHARACTER, Index ) ;

			if( m_pMotion )
			{
				Index = m_pCharacter->m_MotionList.PtrToIndex( m_pMotion ) ;
				a.WriteTaggeds32( IE_TAG_KEY_MOTION, Index ) ;
			}
		}

		a.WriteTaggeds32		( IE_TAG_KEY_STARTFRAME,	m_StartFrame ) ;
		a.WriteTaggedf32		( IE_TAG_KEY_FRAMERATE,		m_FrameRate ) ;
		a.WriteTaggedf32		( IE_TAG_KEY_BLENDFRAMES,	m_BlendFrames ) ;
		a.WriteTaggedbool		( IE_TAG_KEY_MIRROR,		m_Mirror ) ;

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_KEY_END ) ;
	}
	else
	{
		//---	Load
		s32	Index ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_KEY_NAME:
				a.ReadString(m_Name) ;
				break ;
			case IE_TAG_KEY_FRAME:
				a.Reads32( m_Frame ) ;
				break ;
			case IE_TAG_KEY_POS:
				a.Readvector3d( m_Position ) ;
				break ;
			case IE_TAG_KEY_VECTOR:
				a.Readvector3d( m_Vector ) ;
				break ;
			case IE_TAG_KEY_ROTATION:
				a.Readvector3d( m_Rotation ) ;
				break ;
			case IE_TAG_KEY_AMCBASED:
				a.Readbool( m_AMCbased ) ;
				break ;
			case IE_TAG_KEY_AUTOLINKPOS:
				a.Readbool( m_AutoLinkPos ) ;
				break ;
			case IE_TAG_KEY_AUTOLINKDIR:
				a.Readbool( m_AutoLinkDir ) ;
				break ;
			case IE_TAG_KEY_CHARACTER:
				a.Reads32( Index ) ;
				m_pCharacter = (CCharacter*)pDoc->m_CharacterList.IndexToPtr( Index ) ;
				break ;
			case IE_TAG_KEY_MOTION:
				a.Reads32( Index ) ;
				if( m_pCharacter )
				{
					m_pMotion = (CMotion*)m_pCharacter->m_MotionList.IndexToPtr( Index ) ;
				}
				break ;
			case IE_TAG_KEY_STARTFRAME:
				a.Reads32( m_StartFrame ) ;
				break ;
			case IE_TAG_KEY_FRAMERATE:
				a.Readf32( m_FrameRate ) ;
				break ;
			case IE_TAG_KEY_BLENDFRAMES:
				a.Readf32( m_BlendFrames ) ;
				break ;
			case IE_TAG_KEY_MIRROR:
				a.Readbool( m_Mirror ) ;
				break ;
			case IE_TAG_KEY_EVENT:
				a.Readu16( m_Event ) ;
				break ;
			case IE_TAG_KEY_SCALE:
				a.Readf32( m_Scale );
				break;
			case IE_TAG_KEY_VISIBLE:
				a.Readbool( m_Visible );
				break;
			case IE_TAG_KEY_CAMERALOCK:
				a.Readbool( m_CameraLock );
				break;
			case IE_TAG_KEY_END:
				done = TRUE ;
				break ;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CKey diagnostics

#ifdef _DEBUG
void CKey::AssertValid() const
{
	CObject::AssertValid();
}

void CKey::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CKey operations

void CKey::SetName( const char *pName )
{
	m_Name = pName ;
}

const char *CKey::GetName( )
{
	return m_Name ;
}

void CKey::SetActor( CActor *pActor )
{
	m_pActor = pActor ;
}

CActor *CKey::GetActor( )
{
	return m_pActor ;
}

void CKey::SetPosition( vector3d *pv )
{
	V3_Copy( &m_Position, pv ) ;
}

void CKey::GetPosition( vector3d *pv )
{
	V3_Copy( pv, &m_Position ) ;
}

void CKey::SetVector( vector3d *pv )
{
	V3_Copy( &m_Vector, pv ) ;
}

void CKey::GetVector( vector3d *pv )
{
	V3_Copy( pv, &m_Vector ) ;
}

void CKey::SetRotation( vector3d *pv )
{
	V3_Copy( &m_Rotation, pv ) ;
}

void CKey::GetRotation( vector3d *pv )
{
	V3_Copy( pv, &m_Rotation ) ;
}

void CKey::SetFrame( int Frame )
{
	m_Frame = Frame ;
}

int CKey::GetFrame( )
{
	return m_Frame ;
}

void CKey::SetCharacter( CCharacter *pCharacter )
{
	m_pCharacter = pCharacter ;
}

CCharacter *CKey::GetCharacter( )
{
	return m_pCharacter ;
}
	
void CKey::SetMotion( CMotion *pMotion )
{
	m_pMotion = pMotion ;
}

CMotion *CKey::GetMotion( )
{
	return m_pMotion ;
}
	
void CKey::SetStartFrame( int StartFrame )
{
	m_StartFrame = StartFrame ;
}

int CKey::GetStartFrame( )
{
	return m_StartFrame ;
}
	
void CKey::SetFrameRate( float FrameRate )
{
	m_FrameRate = FrameRate ;
}

float CKey::GetFrameRate( )
{
	return m_FrameRate ;
}

void CKey::SetBlendFrames( float BlendFrames )
{
	m_BlendFrames = BlendFrames ;
}

float CKey::GetBlendFrames( )
{
	return m_BlendFrames ;
}

void CKey::SetMirror( bool Mirror )
{
	m_Mirror = Mirror ;
}

bool CKey::GetMirror( )
{
	return m_Mirror ;
}

void CKey::SetEvent( u16 Event )
{
	m_Event = Event ;
}

u16 CKey::GetEvent( )
{
	return m_Event ;
}

void CKey::SetScale( f32 scale )
{
	m_Scale = scale ;
}

f32 CKey::GetScale (void)
{
	return m_Scale;
}
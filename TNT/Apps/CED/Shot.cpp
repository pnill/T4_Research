// Shot.cpp : implementation file
//

#include "stdafx.h"

#include "Shot.h"
#include "ieArchive.h"
#include "CeDDoc.h"
#include "Actor.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShot Implementation

IMPLEMENT_DYNCREATE(CShot, CObject)

/////////////////////////////////////////////////////////////////////////////
// CShot Constructors

void CShot::Initialize()
{
	m_Name			= "Shot" ;
	m_Frame			= 0 ;
	m_pEye			= NULL ;
	m_pTarget		= NULL ;
	m_FOV			= 60;
	m_Speed			= 100 ;
	m_bInterpolate	= FALSE ;
}

CShot::CShot()
{
	//---	Reset Data Members
	Initialize () ;
}

CShot::~CShot()
{
	//---	Destroy
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CShot Implementation

void CShot::Destroy()
{
	//---	Initialize Data Members
	Initialize () ;
}

void CShot::Copy (CShot &Dst, CShot &Src)
{
	Dst.m_Name			= Src.m_Name ;
	Dst.m_Frame			= Src.m_Frame ;
	Dst.m_pEye			= Src.m_pEye ;
	Dst.m_pTarget		= Src.m_pTarget ;
	Dst.m_FOV			= Src.m_FOV ;
	Dst.m_Speed			= Src.m_Speed ;
	Dst.m_bInterpolate	= Src.m_bInterpolate ;
}

/////////////////////////////////////////////////////////////////////////////
// CShot Overridden operators

CShot &CShot::operator=( CShot &ptShot )
{
	Copy (*this, ptShot) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CShot serialization

void CShot::Serialize(CieArchive& a)
{
	CCeDDoc *pDoc = (CCeDDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_SHOT) ;

		//---	Write Details
		a.WriteTaggedString		( IE_TAG_SHOT_NAME,		m_Name ) ;
		a.WriteTaggeds32		( IE_TAG_SHOT_FRAME,	m_Frame ) ;
		a.WriteTaggeds32		( IE_TAG_SHOT_FOV,		m_FOV );
		a.WriteTaggeds32		( IE_TAG_SHOT_SPEED,	m_Speed );
		a.WriteTaggedbool		( IE_TAG_SHOT_INTERP,	m_bInterpolate );

		if( m_pEye && m_pMovie )
		{
			s32	Index = m_pMovie->m_ActorList.PtrToIndex( m_pEye ) ;
			a.WriteTaggeds32( IE_TAG_SHOT_EYE, Index ) ;
		}
		if( m_pTarget && m_pMovie )
		{
			s32	Index = m_pMovie->m_ActorList.PtrToIndex( m_pTarget ) ;
			a.WriteTaggeds32( IE_TAG_SHOT_TARGET, Index ) ;
		}

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_SHOT_END ) ;
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
			case IE_TAG_SHOT_NAME:
				a.ReadString(m_Name) ;
				break ;
			case IE_TAG_SHOT_FRAME:
				a.Reads32( m_Frame ) ;
				break ;
			case IE_TAG_SHOT_EYE:
				a.Reads32( Index ) ;
				if( m_pMovie )
					m_pEye = (CActor*)m_pMovie->m_ActorList.IndexToPtr( Index ) ;
				break ;
			case IE_TAG_SHOT_TARGET:
				a.Reads32( Index ) ;
				if( m_pMovie )
					m_pTarget = (CActor*)m_pMovie->m_ActorList.IndexToPtr( Index ) ;
				break ;
			case IE_TAG_SHOT_FOV:
				a.Reads32( m_FOV );
				break;
			case IE_TAG_SHOT_SPEED:
				a.Reads32( m_Speed );
				break;

			case IE_TAG_SHOT_END:
				done = TRUE ;
				break ;

			case IE_TAG_SHOT_INTERP:
				a.Readbool( m_bInterpolate );
				break ;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CShot diagnostics

#ifdef _DEBUG
void CShot::AssertValid() const
{
	CObject::AssertValid();
}

void CShot::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CShot operations

void CShot::SetName( const char *pName )
{
	m_Name = pName ;
}

const char *CShot::GetName( )
{
	return m_Name ;
}

void CShot::SetFrame( s32 Frame )
{
	m_Frame = Frame ;
}

s32 CShot::GetFrame( )
{
	return m_Frame ;
}

void CShot::SetFOV( s32 degrees )
{
	m_FOV = degrees ;
}

s32 CShot::GetFOV( )
{
	if ( (m_FOV < 5) || (m_FOV > 90) )
		return 60;							// Default FOV
	else
		return m_FOV ;
}

void CShot::SetSpeed( s32 Speed )
{
	Speed = max( Speed, 1 ) ;
	Speed = min( Speed, 200 ) ;

	m_Speed = Speed ;
}

s32 CShot::GetSpeed( )
{
	return m_Speed ;
}

void CShot::SetEye( CActor *pEye )
{
	m_pEye = pEye ;
}

CActor *CShot::GetEye( )
{
	return m_pEye ;
}

void CShot::SetTarget( CActor *pTarget )
{
	m_pTarget = pTarget ;
}

CActor *CShot::GetTarget( )
{
	return m_pTarget ;
}

void CShot::SetMovie( CMovie *pMovie )
{
	m_pMovie = pMovie ;
}

CMovie *CShot::GetMovie( )
{
	return m_pMovie ;
}

// SkelPoint.cpp : implementation file
//

#include "stdafx.h"

#include "SkelPoint.h"
#include "ieArchive.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSkelPoint Implementation

IMPLEMENT_DYNCREATE(CSkelPoint, CObject)

/////////////////////////////////////////////////////////////////////////////
// CSkelPoint Constructors

void CSkelPoint::Initialize()
{
	m_Name = "New Point" ;
	m_Position.Zero();
//	V3_Set( &m_Position, 0, 0, 0 ) ;
	m_iBone = 0 ;
	m_Radius = 10 ;
}

CSkelPoint::CSkelPoint()
{
	//---	Reset Data Members
	Initialize () ;
}

CSkelPoint::~CSkelPoint()
{
	//---	Destroy the Texture
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CSkelPoint Implementation

bool CSkelPoint::Create()
{
	return true ;
}

void CSkelPoint::Destroy()
{
	//---	Initialize Data Members
	Initialize () ;
}

void CSkelPoint::Copy (CSkelPoint &Dst, CSkelPoint &Src)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSkelPoint Overridden operators

CSkelPoint &CSkelPoint::operator=( CSkelPoint &ptSkelPoint )
{
	Copy (*this, ptSkelPoint) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CSkelPoint serialization

void CSkelPoint::Serialize(CieArchive& a)
{
//	CScrEdDoc *pDoc = (CScrEdDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_SKELPOINT) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_SKELPOINT_NAME, m_Name ) ;
		a.WriteTaggedvector3 ( IE_TAG_SKELPOINT_POSITION, m_Position ) ;
		a.WriteTaggedf32( IE_TAG_SKELPOINT_RADIUS, m_Radius ) ;
		a.WriteTaggeds32( IE_TAG_SKELPOINT_BONE, m_iBone ) ;

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_SKELPOINT_END ) ;
	}
	else
	{
		//---	Load
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_SKELPOINT_NAME:
				a.ReadString(m_Name) ;
				break ;
			case IE_TAG_SKELPOINT_POSITION:
				a.Readvector3 (m_Position) ;
				break ;
			case IE_TAG_SKELPOINT_RADIUS:
				a.Readf32(m_Radius) ;
				break ;
			case IE_TAG_SKELPOINT_BONE:
				a.Reads32(m_iBone) ;
				break ;
			case IE_TAG_SKELPOINT_END:
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
// CSkelPoint diagnostics

#ifdef _DEBUG
void CSkelPoint::AssertValid() const
{
	CObject::AssertValid();
}

void CSkelPoint::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CSkelPoint operations

void CSkelPoint::SetName( const char *pName )
{
	m_Name = pName ;
}

const char *CSkelPoint::GetName( )
{
	return m_Name ;
}

void CSkelPoint::GetPosition( vector3  *pVector )
{
	*pVector = m_Position;
//	V3_Copy( pVector, &m_Position ) ;
}

void CSkelPoint::SetPosition( vector3  *pVector )
{
	m_Position = *pVector;
//	V3_Copy( &m_Position, pVector ) ;
}

int CSkelPoint::GetBone( )
{
	return m_iBone ;
}

void CSkelPoint::SetBone( int iBone )
{
	m_iBone = iBone ;
}

f32 CSkelPoint::GetRadius( )
{
	return m_Radius ;
}

void CSkelPoint::SetRadius( f32 Radius )
{
	m_Radius = Radius ;
}

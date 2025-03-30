// SkelPointList.cpp : implementation file
//

#include "stdafx.h"

#include "SkelPointList.h"
#include "ieArchive.h"
#include "FileTags.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSkelPointList Implementation

IMPLEMENT_DYNCREATE(CSkelPointList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CSkelPointList Constructors

CSkelPointList::CSkelPointList()
{
}

CSkelPointList::~CSkelPointList()
{
	//---	Destory each Point in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		CSkelPoint *pSkelPoint = (CSkelPoint*)GetNext (Pos) ;
		delete pSkelPoint ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSkelPointList Implementation

/////////////////////////////////////////////////////////////////////////////
// CSkelPointList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CSkelPointList serialization

void CSkelPointList::Serialize(CieArchive& a)
{
	if (a.IsStoring())
	{
		//---	Start Point List
		a.WriteTag(IE_TYPE_START,IE_TAG_SKELPOINTLIST) ;

		//---	Serialize each Point
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CSkelPoint *pSkelPoint = GetNext(Pos) ;
			pSkelPoint->Serialize(a) ;
		}

		//---	Terminate Point List
		a.WriteTag(IE_TYPE_END,IE_TAG_SKELPOINTLIST_END) ;
	}
	else
	{
		//---	Load Point List
		CSkelPoint *pSkelPoint ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_SKELPOINT:
				pSkelPoint = new CSkelPoint ;
				ASSERT( pSkelPoint ) ;
				pSkelPoint->Serialize( a ) ;
				AddTail( pSkelPoint ) ;
				break ;
			case IE_TAG_SKELPOINTLIST_END:
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
// CSkelPointList diagnostics

#ifdef _DEBUG
void CSkelPointList::AssertValid() const
{
	CObject::AssertValid();
}

void CSkelPointList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

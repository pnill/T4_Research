// EventList.cpp : implementation file
//

#include "stdafx.h"

#include "EventList.h"
#include "ieArchive.h"
#include "FileTags.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEventList Implementation

IMPLEMENT_DYNCREATE(CEventList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CEventList Constructors

CEventList::CEventList()
{
}

CEventList::~CEventList()
{
	DestroyList();
}

/////////////////////////////////////////////////////////////////////////////
// CEventList Implementation

/////////////////////////////////////////////////////////////////////////////
// CEventList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CEventList serialization

void CEventList::Serialize(CieArchive& a)
{
	if (a.IsStoring())
	{
		//---	Start Event List
		a.WriteTag(IE_TYPE_START,IE_TAG_EVENTLIST) ;

		//---	Serialize each Event
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CEvent *pEvent = GetNext(Pos) ;
			pEvent->Serialize(a) ;
		}

		//---	Terminate Event List
		a.WriteTag(IE_TYPE_END,IE_TAG_EVENTLIST_END) ;
	}
	else
	{
		//---	Load Event List
		CEvent *pEvent ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_EVENT:
				pEvent = new CEvent ;
				ASSERT( pEvent ) ;
				pEvent->Serialize( a ) ;
				AddTail( pEvent ) ;
				break ;
			case IE_TAG_EVENTLIST_END:
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
// CEventList diagnostics

#ifdef _DEBUG
void CEventList::AssertValid() const
{
	CObject::AssertValid();
}

void CEventList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

POSITION CEventList::GetPositionbyName(CString EventName)
{
	POSITION Position;

	Position = GetHeadPosition();
	while( Position )
	{
		CEvent* pEvent;
		CString CurrentName;
		
		pEvent = (CEvent *)GetAt( Position );
		if( pEvent )
		{
			CurrentName = pEvent->GetName();
			if( CurrentName == EventName )
			{
				break;
			}
		}
		GetNext( Position );
	}

	return Position;
}

void CEventList::DestroyList()
{
	//---	Destory each Event in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		CEvent *pEvent = (CEvent*)GetNext (Pos) ;
		delete pEvent ;
		pEvent = NULL;
	}
	RemoveAll();
}

CEventList& CEventList::operator=( CEventList& EventList )
{
	CEvent *pEventSrc;
	CEvent *pEventDst;

	DestroyList();

	POSITION Pos = EventList.GetHeadPosition();
	while (Pos != NULL)
	{
		pEventDst = new CEvent;
		ASSERT( pEventDst );
		pEventSrc = (CEvent *)EventList.GetNext(Pos);
		*pEventDst = *pEventSrc;
		AddTail( pEventDst );
	}

	return *this;
}

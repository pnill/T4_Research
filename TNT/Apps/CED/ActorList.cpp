// ActorList.cpp : implementation file
//

#include "stdafx.h"

#include "ActorList.h"
#include "ieArchive.h"
#include "FileTags.h"
#include "Actor.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CActorList Implementation

IMPLEMENT_DYNCREATE(CActorList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CActorList Constructors

CActorList::CActorList()
{
}

CActorList::~CActorList()
{
	DeleteAllActors() ;
}

/////////////////////////////////////////////////////////////////////////////
// CActorList Implementation

void CActorList::DeleteAllActors()
{
	//---	Destory each Actor in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		POSITION Pos2 = Pos ;
		CActor *pActor = (CActor*)GetNext (Pos) ;
		RemoveAt( Pos2 ) ;
		delete pActor ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CActorList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CActorList serialization

void CActorList::Serialize(CieArchive& a )
{
	if (a.IsStoring())
	{
		//---	Start Motion List
		a.WriteTag(IE_TYPE_START,IE_TAG_ACTORLIST) ;

		//---	Serialize each Motion
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CActor *pActor = GetNext(Pos) ;
			pActor->Serialize( a ) ;
		}

		//---	Terminate Motion List
		a.WriteTag(IE_TYPE_END,IE_TAG_ACTORLIST_END) ;
	}
	else
	{
		//---	Load Motion List
		CActor *pActor ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_ACTOR:
				pActor = new CActor ;
				pActor->Serialize( a ) ;
				AddTail( pActor ) ;
				break ;
			case IE_TAG_ACTORLIST_END:
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
// CActorList diagnostics

#ifdef _DEBUG
void CActorList::AssertValid() const
{
	CObject::AssertValid();
}

void CActorList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

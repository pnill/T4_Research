// ShotList.cpp : implementation file
//

#include "stdafx.h"

#include "ShotList.h"
#include "ieArchive.h"
#include "FileTags.h"

#include "Shot.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShotList Implementation

IMPLEMENT_DYNCREATE(CShotList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CShotList Constructors

CShotList::CShotList()
{
	m_pMovie = NULL ;
}

CShotList::~CShotList()
{
	DeleteAll() ;
}

/////////////////////////////////////////////////////////////////////////////
// CShotList Implementation

void CShotList::DeleteAll()
{
	//---	Destory each Actor in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		POSITION Pos2 = Pos ;
		CShot *pShot = (CShot*)GetNext (Pos) ;
		RemoveAt( Pos2 ) ;
		delete pShot ;
	}
}

void CShotList::InsertSorted( CShot *pShot )
{
	bool	done = false ;
	CShot *pLast = NULL ;
	POSITION Pos = GetHeadPosition() ;
	while( Pos )
	{
		POSITION Pos2 = Pos ;
		pLast = GetNext( Pos ) ;
		if( pLast->GetFrame() > pShot->GetFrame() )
		{
			InsertBefore( Pos2, pShot ) ;
			done = true ;
			break ;
		}
	}

	if( !done )
	{
		AddTail( pShot ) ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CShotList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CShotList serialization

void CShotList::Serialize(CieArchive& a )
{
	if (a.IsStoring())
	{
		//---	Start Motion List
		a.WriteTag(IE_TYPE_START,IE_TAG_SHOTLIST) ;

		//---	Serialize each Motion
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CShot *pShot = GetNext(Pos) ;
			pShot->Serialize( a ) ;
		}

		//---	Terminate Motion List
		a.WriteTag(IE_TYPE_END,IE_TAG_SHOTLIST_END) ;
	}
	else
	{
		//---	Load Shot List
		CShot *pShot ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_SHOT:
				pShot = new CShot ;
				pShot->SetMovie( m_pMovie ) ;
				pShot->Serialize( a ) ;
				AddTail( pShot ) ;
				break ;
			case IE_TAG_SHOTLIST_END:
				done = TRUE ;
				break ;
			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}


CShotList &CShotList::operator=( CShotList &ptObject )
{
	Copy (*this, ptObject) ;
    return *this ;
}

void CShotList::Copy (CShotList &Dst, CShotList &Src)
{
	POSITION Pos = Src.GetHeadPosition( ) ;
	while( Pos )
	{
		CShot *pShotSrc = Src.GetNext( Pos ) ;
		ASSERT( pShotSrc ) ;
		CShot *pShotDst = new CShot ;
		ASSERT( pShotDst ) ;
		*pShotDst = *pShotSrc ;
		Dst.AddTail( pShotDst ) ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CShotList diagnostics

#ifdef _DEBUG
void CShotList::AssertValid() const
{
	CObject::AssertValid();
}

void CShotList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

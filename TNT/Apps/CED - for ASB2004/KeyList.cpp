// KeyList.cpp : implementation file
//

#include "stdafx.h"

#include "KeyList.h"
#include "ieArchive.h"
#include "FileTags.h"

#include "Key.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyList Implementation

IMPLEMENT_DYNCREATE(CKeyList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CKeyList Constructors

CKeyList::CKeyList()
{
}

CKeyList::~CKeyList()
{
	DeleteAll() ;
}

/////////////////////////////////////////////////////////////////////////////
// CKeyList Implementation

void CKeyList::DeleteAll()
{
	//---	Destory each Actor in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		POSITION Pos2 = Pos ;
		CKey *pKey = (CKey*)GetNext (Pos) ;
		RemoveAt( Pos2 ) ;
		delete pKey ;
	}
}

void CKeyList::InsertSorted( CKey *pKey )
{
	bool	done = false ;
	CKey *pLast = NULL ;
	POSITION Pos = GetHeadPosition() ;
	while( Pos )
	{
		POSITION Pos2 = Pos ;
		pLast = GetNext( Pos ) ;
		if( pLast->GetFrame() > pKey->GetFrame() )
		{
			InsertBefore( Pos2, pKey ) ;
			done = true ;
			break ;
		}
	}

	if( !done )
	{
		AddTail( pKey ) ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CKeyList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CKeyList serialization

void CKeyList::Serialize(CieArchive& a )
{
	if (a.IsStoring())
	{
		//---	Start Motion List
		a.WriteTag(IE_TYPE_START,IE_TAG_KEYLIST) ;

		//---	Serialize each Motion
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CKey *pKey = GetNext(Pos) ;
			pKey->Serialize( a ) ;
		}

		//---	Terminate Motion List
		a.WriteTag(IE_TYPE_END,IE_TAG_KEYLIST_END) ;
	}
	else
	{
		//---	Load Motion List
		CKey *pKey ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_KEY:
				pKey = new CKey ;
				pKey->Serialize( a ) ;
				AddTail( pKey ) ;
				break ;
			case IE_TAG_KEYLIST_END:
				done = TRUE ;
				break ;
			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}


CKeyList &CKeyList::operator=( CKeyList &ptObject )
{
	Copy (*this, ptObject) ;
    return *this ;
}

void CKeyList::Copy (CKeyList &Dst, CKeyList &Src)
{
	POSITION Pos = Src.GetHeadPosition( ) ;
	while( Pos )
	{
		CKey *pKeySrc = Src.GetNext( Pos ) ;
		ASSERT( pKeySrc ) ;
		CKey *pKeyDst = new CKey ;
		ASSERT( pKeyDst ) ;
		*pKeyDst = *pKeySrc ;
		Dst.AddTail( pKeyDst ) ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CKeyList diagnostics

#ifdef _DEBUG
void CKeyList::AssertValid() const
{
	CObject::AssertValid();
}

void CKeyList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

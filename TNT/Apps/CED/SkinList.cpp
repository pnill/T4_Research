// SkinList.cpp : implementation file
//

#include "stdafx.h"

#include "SkinList.h"
#include "ieArchive.h"
#include "FileTags.h"
#include "Character.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSkinList Implementation

IMPLEMENT_DYNCREATE(CSkinList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CSkinList Constructors

CSkinList::CSkinList()
{
}

CSkinList::~CSkinList()
{
	DeleteAllSkins() ;
}

/////////////////////////////////////////////////////////////////////////////
// CSkinList Implementation

void CSkinList::DeleteAllSkins()
{
	//---	Destory each Skin in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		POSITION Pos2 = Pos ;
		CSkin *pSkin = (CSkin*)GetNext (Pos) ;
		RemoveAt( Pos2 ) ;
		delete pSkin ;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CSkinList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CSkinList serialization

void CSkinList::Serialize( CieArchive& a )
{
	if (a.IsStoring())
	{
		//---	Start Skin List
		a.WriteTag(IE_TYPE_START,IE_TAG_SKINLIST) ;

		//---	Serialize each Skin
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CSkin *pSkin = GetNext(Pos) ;
			pSkin->Serialize( a ) ;
		}

		//---	Terminate Skin List
		a.WriteTag(IE_TYPE_END,IE_TAG_SKINLIST_END) ;
	}
	else
	{
		//---	Load Skin List
		CSkin *pSkin ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_SKIN:
				pSkin = new CSkin ;
				pSkin->Serialize( a ) ;
				AddTail (pSkin) ;
				break ;
			case IE_TAG_SKINLIST_END:
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
// CSkinList diagnostics

#ifdef _DEBUG
void CSkinList::AssertValid() const
{
	CObject::AssertValid();
}

void CSkinList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

// CharacterList.cpp : implementation file
//

#include "stdafx.h"

#include "CharacterList.h"
#include "ieArchive.h"
#include "FileTags.h"
#include "CeDDoc.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCharacterList Implementation

IMPLEMENT_DYNCREATE(CCharacterList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CCharacterList Constructors

CCharacterList::CCharacterList()
{
}

CCharacterList::~CCharacterList()
{
	//---	Destory each Character in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		CCharacter *pCharacter = (CCharacter*)GetNext (Pos) ;
		delete pCharacter ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCharacterList Implementation
bool CCharacterList::SearchReplaceFilePaths( CString Search, CString Replace, s32 Method )
{
	POSITION Pos = GetHeadPosition();
	CCharacter* pCharacter;
	bool ReplacedSome;

	while( Pos )
	{
		pCharacter = (CCharacter*) GetNext( Pos );
		ASSERT(pCharacter);

		if( pCharacter->SearchReplaceFilePaths( Search, Replace, Method ) )
			ReplacedSome = TRUE;
	}

	return ReplacedSome;
}

/////////////////////////////////////////////////////////////////////////////
// CCharacterList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CCharacterList serialization

void CCharacterList::Serialize(CieArchive& a)
{
	if (a.IsStoring())
	{
		//---	Start Character List
		a.WriteTag(IE_TYPE_START,IE_TAG_CHARACTERLIST) ;

		//---	Serialize each Character
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CCharacter *pCharacter = GetNext(Pos) ;
			pCharacter->Serialize(a) ;
		}

		//---	Terminate Character List
		a.WriteTag(IE_TYPE_END,IE_TAG_CHARACTERLIST_END) ;
	}
	else
	{
		//---	Load Character List
		CCharacter *pCharacter ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_CHARACTER:
				pCharacter = new CCharacter ;
				ASSERT( pCharacter ) ;
				pCharacter->m_pDoc = (CCeDDoc*)a.m_pDocument ;
				pCharacter->Serialize(a) ;
				AddTail (pCharacter) ;
				break ;
			case IE_TAG_CHARACTERLIST_END:
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
// CCharacterList diagnostics

#ifdef _DEBUG
void CCharacterList::AssertValid() const
{
	CObject::AssertValid();
}

void CCharacterList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

// MotionList.cpp : implementation file
//

#include "stdafx.h"

#include "MotionList.h"
#include "ieArchive.h"
#include "FileTags.h"
#include "Character.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMotionList Implementation

IMPLEMENT_DYNCREATE(CMotionList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CMotionList Constructors

CMotionList::CMotionList()
{
}

CMotionList::~CMotionList()
{
	DeleteAllMotions() ;
}

/////////////////////////////////////////////////////////////////////////////
// CMotionList Implementation

void CMotionList::DeleteAllMotions()
{
	//---	Destory each Motion in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		POSITION Pos2 = Pos ;
		CMotion *pMotion = (CMotion*)GetNext (Pos) ;
		RemoveAt( Pos2 ) ;
		delete pMotion ;
	}
}

bool CMotionList::SearchReplaceFilePaths( CString Search, CString Replace, s32 Method )
{
	POSITION Pos = GetHeadPosition();
	CMotion* pMotion;
	bool ReplacedSome;

	Search.MakeUpper();
	Replace.MakeUpper();

	while( Pos )
	{
		pMotion = (CMotion*) GetNext( Pos );
		ASSERT(pMotion);

		if( pMotion->SearchReplacePathName( Search, Replace, Method ) )
			ReplacedSome = TRUE;
	}

	return ReplacedSome;
}

/////////////////////////////////////////////////////////////////////////////
// CMotionList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CMotionList serialization

void CMotionList::Serialize(CieArchive& a, CCharacter *pCharacter )
{
	if (a.IsStoring())
	{
		//---	Start Motion List
		a.WriteTag(IE_TYPE_START,IE_TAG_MOTIONLIST) ;

		//---	Serialize each Motion
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CMotion *pMotion = GetNext(Pos) ;
			pMotion->Serialize( a, pCharacter ) ;
		}

		//---	Terminate Motion List
		a.WriteTag(IE_TYPE_END,IE_TAG_MOTIONLIST_END) ;
	}
	else
	{
		//---	Load Motion List
		CMotion *pMotion ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_MOTION:
				pMotion = new CMotion ;
				pMotion->m_pCharacter = pCharacter ;
				pMotion->Serialize( a, pCharacter ) ;
				AddTail (pMotion) ;
				break ;

			case IE_TAG_MOTIONLIST_END:
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
// CMotionList diagnostics

#ifdef _DEBUG
void CMotionList::AssertValid() const
{
	CObject::AssertValid();
}

void CMotionList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

// MovieList.cpp : implementation file
//

#include "stdafx.h"

#include "MovieList.h"
#include "ieArchive.h"
#include "FileTags.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovieList Implementation

IMPLEMENT_DYNCREATE(CMovieList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CMovieList Constructors

CMovieList::CMovieList()
{
}

CMovieList::~CMovieList()
{
	//---	Destory each Movie in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		CMovie *pMovie = (CMovie*)GetNext (Pos) ;
		delete pMovie ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMovieList Implementation

/////////////////////////////////////////////////////////////////////////////
// CMovieList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CMovieList serialization

void CMovieList::Serialize(CieArchive& a)
{
	if (a.IsStoring())
	{
		//---	Start Character List
		a.WriteTag(IE_TYPE_START,IE_TAG_MOVIELIST) ;

		//---	Serialize each Character
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CMovie *pMovie = GetNext(Pos) ;
			pMovie->Serialize(a) ;
		}

		//---	Terminate Character List
		a.WriteTag(IE_TYPE_END,IE_TAG_MOVIELIST_END) ;
	}
	else
	{
		//---	Load Character List
		CMovie *pMovie ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_MOVIE:
				pMovie = new CMovie ;
				ASSERT( pMovie ) ;
				pMovie->Serialize(a) ;
				AddTail (pMovie) ;
				break ;
			case IE_TAG_MOVIELIST_END:
				done = TRUE ;
				break ;
			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

POSITION CMovieList::FindMovie(const char* movie)
{
    POSITION Pos = GetHeadPosition();
    while (Pos != NULL)
    {
        CMovie *pMovie = (CMovie*)GetAt(Pos);

        if (pMovie->m_Name  == movie)
        {
            break;
        }

        GetNext(Pos);
    }

    return Pos;
}

/////////////////////////////////////////////////////////////////////////////
// CMovieList diagnostics

#ifdef _DEBUG
void CMovieList::AssertValid() const
{
	CObject::AssertValid();
}

void CMovieList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

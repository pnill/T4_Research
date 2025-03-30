#ifndef _MovieList_h_
#define	_MovieList_h_
// MovieList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "Movie.h"

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_MOVIELIST_END = IE_TAG_MOVIELIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CMovieList

class CMovieList : public CieList
{
	DECLARE_DYNCREATE(CMovieList)

// Attributes
public:

// Constructors
public:
	CMovieList();
	~CMovieList();

// Operations
public:

    POSITION FindMovie(const char*);

	virtual void Serialize(CieArchive& a) ;

	CMovie *GetNext (POSITION &rPosition) {	return (CMovie*)CieList::GetNext(rPosition) ; }

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_MovieList_h_

#ifndef _ActorList_h_
#define	_ActorList_h_
// ActorList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "ieArchive.h"

class CActor ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_ACTORLIST_END = IE_TAG_ACTORLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CActorList

class CActorList : public CieList
{
	DECLARE_DYNCREATE(CActorList)

// Attributes
public:

// Constructors
public:
	CActorList();
	~CActorList();

// Operations
public:
	virtual void Serialize( CieArchive& a ) ;

	CActor *GetNext (POSITION &rPosition) { return (CActor*)CieList::GetNext(rPosition) ; }
	void	DeleteAllActors() ;

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_ActorList_h_

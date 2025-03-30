#ifndef _EventList_h_
#define	_EventList_h_
// EventList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "Event.h"

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_EVENTLIST_END = IE_TAG_EVENTLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CEventList

class CEventList : public CieList
{
	DECLARE_DYNCREATE(CEventList)

// Attributes
public:

// Constructors
public:
	CEventList();
	~CEventList();

// Operations
public:
	POSITION GetPositionbyName( CString EventName );
	virtual void Serialize(CieArchive& a) ;
	CEventList& operator=( CEventList& EventList );

	CEvent *GetNext (POSITION &rPosition) {	return (CEvent*)CieList::GetNext(rPosition) ; }

// Implementation
protected:
	void DestroyList();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_EventList_h_

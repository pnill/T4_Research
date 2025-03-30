#ifndef _SkelPointList_h_
#define	_SkelPointList_h_
// SkelPointList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "SkelPoint.h"

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_SKELPOINTLIST_END = IE_TAG_SKELPOINTLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CSkelPointList

class CSkelPointList : public CieList
{
	DECLARE_DYNCREATE(CSkelPointList)

// Attributes
public:

// Constructors
public:
	CSkelPointList();
	~CSkelPointList();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;

	CSkelPoint *GetNext (POSITION &rPosition) {	return (CSkelPoint*)CieList::GetNext(rPosition) ; }

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_SkelPointList_h_

#ifndef _KeyList_h_
#define	_KeyList_h_
// KeyList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "ieArchive.h"

class CKey ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_KEYLIST_END = IE_TAG_KEYLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CKeyList

class CKeyList : public CieList
{
	DECLARE_DYNCREATE(CKeyList)

// Attributes
public:

// Constructors
public:
	CKeyList();
	~CKeyList();

// Operations
public:
	virtual void Serialize( CieArchive& a ) ;
	virtual void Copy (CKeyList &Dst, CKeyList &Src) ;
	virtual CKeyList &operator=( CKeyList & ) ;

	CKey			*GetNext (POSITION &rPosition) { return (CKey*)CieList::GetNext(rPosition) ; }
	void			DeleteAll() ;
	void			InsertSorted( CKey *pKey ) ;

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_KeyList_h_

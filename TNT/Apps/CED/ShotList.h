#ifndef _ShotList_h_
#define	_ShotList_h_
// ShotList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "ieArchive.h"

class CShot ;
class CMovie ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_SHOTLIST_END = IE_TAG_SHOTLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CShotList

class CShotList : public CieList
{
	DECLARE_DYNCREATE(CShotList)

// Attributes
public:
	CMovie			*m_pMovie ;

// Constructors
public:
	CShotList();
	~CShotList();

// Operations
public:
	virtual void Serialize( CieArchive& a ) ;
	virtual void Copy (CShotList &Dst, CShotList &Src) ;
	virtual CShotList &operator=( CShotList & ) ;

	CShot			*GetNext (POSITION &rPosition) { return (CShot*)CieList::GetNext(rPosition) ; }
	void			DeleteAll() ;
	void			InsertSorted( CShot *pShot ) ;

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_ShotList_h_

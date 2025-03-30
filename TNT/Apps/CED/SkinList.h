#ifndef _SkinList_h_
#define	_SkinList_h_
// SkinList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "Skin.h"

class CCharacter ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_SKINLIST_END = IE_TAG_SKINLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CSkinList

class CSkinList : public CieList
{
	DECLARE_DYNCREATE(CSkinList)

// Attributes
public:

// Constructors
public:
	CSkinList();
	~CSkinList();

// Operations
public:
	virtual void Serialize( CieArchive& a ) ;

	CSkin*	GetNext (POSITION &rPosition) { return (CSkin*)CieList::GetNext(rPosition) ; }
	void	DeleteAllSkins() ;

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_SkinList_h_

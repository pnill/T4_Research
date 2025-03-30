#ifndef _CharacterList_h_
#define	_CharacterList_h_
// CharacterList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "Character.h"

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_CHARACTERLIST_END = IE_TAG_CHARACTERLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CCharacterList

class CCharacterList : public CieList
{
	DECLARE_DYNCREATE(CCharacterList)

// Attributes
public:

// Constructors
public:
	CCharacterList();
	~CCharacterList();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;

	CCharacter *GetNext (POSITION &rPosition) {	return (CCharacter*)CieList::GetNext(rPosition) ; }
	bool	SearchReplaceFilePaths( CString Search, CString Replace, s32 Method );

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_CharacterList_h_

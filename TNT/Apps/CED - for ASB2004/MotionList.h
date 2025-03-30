#ifndef _MotionList_h_
#define	_MotionList_h_
// MotionList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "Motion.h"

class CCharacter ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_MOTIONLIST_END = IE_TAG_MOTIONLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CMotionList

class CMotionList : public CieList
{
	DECLARE_DYNCREATE(CMotionList)

// Attributes
public:

// Constructors
public:
	CMotionList();
	~CMotionList();

// Operations
public:
	virtual void Serialize( CieArchive& a, CCharacter *pCharacter ) ;

	CMotion *GetNext (POSITION &rPosition) { return (CMotion*)CieList::GetNext(rPosition) ; }
	void	DeleteAllMotions() ;
	bool	SearchReplaceFilePaths( CString Search, CString Replace, s32 Method );

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_MotionList_h_

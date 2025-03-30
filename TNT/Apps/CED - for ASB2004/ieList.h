#ifndef _ieList_h_
#define	_ieList_h_
// ieList.h : header file
//

#include "x_types.h"

/////////////////////////////////////////////////////////////////////////////
// CieList

class CieList : public CPtrList
{
	DECLARE_DYNCREATE(CieList)

// Attributes
public:

// Constructors
public:
	CieList();
	~CieList();

// Operations
public:
	CObject		*IndexToPtr (s32 Index) ;				// Convert 0based Index into Pointer
	s32			PtrToIndex (CObject *pObject) ;			// Convert pointer to 0based Index
	CObject		*Remove( CObject *pObject ) ;			// Remove Object from List

// Overridden Operators	
public:
	virtual CieList &operator=( CieList & ) ;			// Right side is the argument.

	// Implementation
public:
	virtual void Copy (CieList &Dst, CieList &Src) ; // Copy
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_ieList_h_

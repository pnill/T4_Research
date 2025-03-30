// ieList.cpp : implementation file
//

#include "stdafx.h"

#include "ieList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CieList Implementation

IMPLEMENT_DYNCREATE(CieList, CPtrList)

/////////////////////////////////////////////////////////////////////////////
// CieList Constructors

CieList::CieList()
{
	//---	Reset Data Members
}

CieList::~CieList()
{
	//---	Destroy All List Elements
}

void CieList::Copy (CieList &Dst, CieList &Src)
{
}

/////////////////////////////////////////////////////////////////////////////
// CieList Implementation

//---	Convert 0based Index to Pointer
CObject *CieList::IndexToPtr (s32 Index)
{
	CObject	*pFoundObject = NULL ;

	//---	Return NULL if we passed the Number of elements
	if( (Index >= 0) && (Index < GetCount()) )
	{
		//---	Loop through elements until Name found
		POSITION Pos = GetHeadPosition() ;
		while ((Index-- >= 0) && (Pos != NULL))
		{
			//---	Get Pointer to Next Element
			pFoundObject = (CObject*)GetNext(Pos) ;
		}
	}

	//---	Return Pointer to Object Found
	return pFoundObject ;
}

//---	Convert Pointer to 0based Index
s32 CieList::PtrToIndex (CObject *pObject)
{
	s32			Index = 0 ;
	s32			FoundIndex = -1 ;

	//---	Loop through elements until Name found
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		//---	Get Pointer to Next Element
		CObject *pTestObject = (CObject*)GetNext(Pos) ;
		if (pObject == pTestObject)
		{
			FoundIndex = Index ;
			break ;
		}
		Index++ ;
	}

	//---	Return Pointer to Object Found
	return FoundIndex ;
}

CObject *CieList::Remove( CObject *pObject )
{
	CObject *pFound = NULL ;
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		POSITION Pos2 = Pos ;
		CObject *pScan = (CObject*)GetNext(Pos) ;

		if (pObject == pScan)
		{
			pFound = pScan ;
			RemoveAt( Pos2 ) ;
		}
	}
	return pFound ;
}

/////////////////////////////////////////////////////////////////////////////
// CieList Overridden operators

CieList &CieList::operator=( CieList &ptObject )
{
	Copy (*this, ptObject) ;
    return *this ;
}


/////////////////////////////////////////////////////////////////////////////
// CieList diagnostics

#ifdef _DEBUG
void CieList::AssertValid() const
{
	CObject::AssertValid();
}

void CieList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

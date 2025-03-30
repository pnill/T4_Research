// FlagList.h: interface for the CFlagList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLAGLIST_H__86A11B21_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_)
#define AFX_FLAGLIST_H__86A11B21_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_

#include <afxtempl.h>
#include "Flag.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum
{
	ANIMFLAG_CYCLING,
	ANIMFLAG_RATEANALOG,
	ANIMFLAG_NOMOVEDIR,
	ANIMFLAG_RATERAMPED,
	ANIMFLAG_READVELOCITY,
	ANIMFLAG_LOCKFACEDIR,
	ANIMFLAG_NODISPLACEMENT,
	ANIMFLAG_USEDESTSPEED,

	NUM_DEFAULT_FLAGS
};

class CFlagList : public CObject  
{
public:
	//----------------------------------------------------------------------
	//	Constructor/Destructor
	//----------------------------------------------------------------------

	CFlagList();
	virtual ~CFlagList();

	//----------------------------------------------------------------------
	//	Export functions
	//----------------------------------------------------------------------

	void		ExportMacros		( FILE *pFile, int Shift );
	void		ExportDefines		( FILE *pFile, int Shift );

	//----------------------------------------------------------------------
	//	operation functions
	//----------------------------------------------------------------------

	void		ShiftFlags			( class CAnimTreeNode* pNode, int Operation, int Flag );
	void		Serialize			( CArchive &ar );
	int			GetMask				( int StartBit );
	int			GetFlagBit			( CFlag *pFlag );
	CFlag*		FindFlag			( CString &Name, CFlag *pSkipFlag=NULL );
	int			GetNumFlags			( void );
	CFlagList&	operator=			( CFlagList &Other );
	void		MakeEmpty			( void );
	CFlag*		GetFlag				( int Index );
	void		RemoveAll			( void );
	int			RemoveFlag			( int Index );
	int			RemoveFlag			( CString &Name );
	int			RemoveFlag			( CFlag *pFlag );
	int			GetTotalFlagBits	( void );
	void		AddFlag				( CString &Name=CString("NEW_FLAG"), int Index=-1 );

private:
	CTypedPtrList<CObList,CFlag*> m_FlagList;
};

#endif // !defined(AFX_FLAGLIST_H__86A11B21_DD90_11D2_8D9C_00A0CC23CE22__INCLUDED_)

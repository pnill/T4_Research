// Methods.h: interface for the CMethods class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_METHODS_H__A4F7C707_E046_11D2_9248_00105A29F84B__INCLUDED_)
#define AFX_METHODS_H__A4F7C707_E046_11D2_9248_00105A29F84B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


enum
{
	ANIMMETHOD_NORMAL,
	ANIMMETHOD_NETDISPLACEMENT,
	ANIMMETHOD_FINALMOVEDIR,
	ANIMMETHOD_FINALFACEDIR,
	ANIMMETHOD_FRAMEFACING,

	NUM_DEFAULT_METHODS
};

class CMethods : public CObject  
{
public:
	//----------------------------------------------------------------------
	//	Constructor/Destructor
	//----------------------------------------------------------------------

	CMethods();
	virtual ~CMethods();

	//----------------------------------------------------------------------
	//	Export functions
	//----------------------------------------------------------------------

	void		ExportMacros		( FILE *pFile, int Shift);
	void		ExportDefines		( FILE *pFile, int Shift);

	//----------------------------------------------------------------------
	//	operation functions
	//----------------------------------------------------------------------

	void		ShiftMethods		( class CAnimTreeNode *pNode, int Operation, int Method );
	void		Serialize			( CArchive &ar );
	CMethods&	operator=			( CMethods &Other );
	int			GetMask				( int StartBit );
	void		AddMethod			( CString &Name=CString("NEW_METHOD"), int Index = -1 );
	int			CountNumBits		( int Value );
	CString&	GetMethod			( int Index );
	int			FindMethod			( CString &Name );
	void		RemoveAll			( void );
	void		RemoveMethod		( int Index );
	void		MakeEmpty			( void );
	int			GetTotalMethodBits	( void );
	int			GetCount			( void );

private:
	CStringList m_Methods;

};

#endif // !defined(AFX_METHODS_H__A4F7C707_E046_11D2_9248_00105A29F84B__INCLUDED_)

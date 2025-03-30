// AnimNode.h: interface for the CAnimNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMNODE_H__2DE9B942_6F29_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMNODE_H__2DE9B942_6F29_11D2_9248_00105A29F8F3__INCLUDED_

#include "EquivNode.h"	// Added by ClassView
#include "AnimTransList.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AnimTreeNode.h"
#include "LocoAnimTable.h"
class CScanSourceCodeDlg;

class CAnimNode : public CAnimTreeNode  
{
public:
	CAnimNode(class CAnimTblEdDoc *pDoc, class CAnimTreeNode *pParent, const char *lpszAnimID = NULL, const char *lpszAnimName = NULL);
	virtual ~CAnimNode();

	void					FillCallbackStringArray( CStringArray& rStringArray );

	//---	callback related functions
	void					SetCallback( CString Callback )					{ m_Callback = Callback; }
	CString					GetCallback( void )								{ return m_Callback; }
	CString					GetParentCallback( void );
	void					SetChildrenDefaultCallback( CString& Callback, CString& NewCallback );
	int						GetCallbackExport( void );

	//---	cut table related functions
	void					SetCutTable( class CLocoAnimTable* pCutTable )	{ m_pCutTable = pCutTable; }
	class CLocoAnimTable*	GetCutTable( void )								{ return m_pCutTable; }
	class CLocoAnimTable*	GetParentCutTable( void );
	void					SetChildrenDefaultCutTable( class CLocoAnimTable* pCutTable, CLocoAnimTable* pNewCutTable );
	CString					GetCutTableExport( void );

	//---	method related functions
	void					BS_SetMethod( int Value );
	int						BS_GetMethod( void );
	int						BS_GetGroupMethod( void );
	void					ShiftMethods( int Operation, int Method );
	void					SetChildrenDefaultMethod( int CurValue, int NewValue );

	//---	flag related functions
	void					BS_SetFlags( int Value );
	int						BS_GetFlags( void );
	int						BS_GetGroupFlags( void );
	void					ShiftFlags( int Operation, int Flag );
	void					SetChildrenFlag( int Flag, int Value );

	//---	state related functions
	void					BS_SetStates( int Value );
	int						BS_GetStates( void );
	int						BS_GetGroupStates( void );
	void					ShiftStateClasses( int Operation, int StateClass, int State, int Bit, int Num );
	void					SetChildrenDefaultState( int StateClass, int CurValue, int NewValue );

	//---	priority related functions
	void					BS_SetPriority( int Value );
	int						BS_GetPriority( void );
	int						BS_GetGroupPriority( void );
	void					ShiftPriorities( int Operation, int Priority );
	void					SetChildrenDefaultPriority( int CurValue, int NewValue );

	//---	end state related functions
	void					BS_SetEndStates(int Value);
	int						BS_GetEndStates(void);

	//---	specific state functions
	void					BS_SetState(int StateClass, int Value);
	int						BS_GetState(int StateClass);

	//---	bitset export function
	CString&				BS_GetExportValue(CString &String);

	//---	dependancies (transitions and prerequisites)
	int						CountAllDependancies		( void );
	int						GetNumDependanciesWithLink	( bool Recount=FALSE, bool CountDisabledOnes=FALSE, bool CountInheritedOnes=TRUE );
	int						GetNumDependancies			( bool Recount=FALSE, bool CountDisabledOnes=FALSE, bool CountInheritedOnes=TRUE );
	int						GetNumParentDependancies	( bool Recount=FALSE, bool CountDisabledOnes=FALSE );
	CAnimTrans*				GetFirstDependancy			( void );
	CAnimTrans*				GetNextDependancy			( void );
	void					ClearDependancies			( void );
	void					GetDependanciesWithLink		( CAnimTransList& rAnimTransList );
	void					GetDependancies				( CAnimTransList& rAnimTransList );
	void					SetDependancies				( CAnimTransList& rAnimTransList );

	//---	prerequisites
	int						GetNumPrerequisites			( bool Recount=FALSE, bool CountDisabledOnes=FALSE, bool CountInheritedOnes=TRUE );
	int						GetNumParentPrerequisites	( bool Recount=FALSE, bool CountDisabledOnes=FALSE );
	CAnimTrans*				GetFirstPrerequisite		( void );
	CAnimTrans*				GetNextPrerequisite			( void );
	void					GetPrerequisites			( CAnimTransList& rAnimTransList, CString TagString="" );
	void					GetParentPrerequisites		( CAnimTransList& rAnimTransList, CString TagString="" );

	//---	transitions
	int						GetNumTransitions			( bool Recount=FALSE, bool CountDisabledOnes=FALSE, bool CountInheritedOnes=TRUE );
	int						GetNumParentTransitions		( bool Recount=FALSE, bool CountDisabledOnes=FALSE );
	CAnimTrans*				GetFirstTransition			( void );
	CAnimTrans*				GetNextTransition			( void );
	void					GetTransitions				( CAnimTransList& rAnimTransList, CString TagString=""  );
	void					GetParentTransitions		( CAnimTransList& rAnimTransList, CString TagString=""  );

	void SwapEquivalency( void );
	void SwitchCutTablePointer( void );
	virtual int GetState();
	virtual int IsExported();
	CString& GetVPString( int *pDisplayOrder );
	virtual void AddToListBox(CListBox *pListBox, int nFlags);
	CString GetFrameRateExportString(CString Buffer);
	int GetNumAnimsWithExtras();
	int GetNumAnimsWithDependancies( bool Recount, bool CountDisabledOnes );
	int GetTotalNumDependancies( bool Recount, bool CountDisabledOnes );
	int GetNumAnimsWithDependanciesWithLink( bool Recount, bool CountDisabledOnes );
	int GetTotalNumDependanciesWithLink( bool Recount, bool CountDisabledOnes );
	BOOL RateAnalogControlled(void);
	BOOL HasExtraInfo(void);
	CAnimTreeNode * Clone();
	int GetNumAnims();
	int GetNumExportedAnims();
	int CanHaveChildren();
	void Copy(CAnimTreeNode& AnimTreeNode);
	int IsTransition(void);
	
	class CGroupNode*	GetParent( void ){ return (class CGroupNode*)CAnimTreeNode::GetParent(); }
	int GetNumEquivs();
	void Export(FILE *pFile);
	int GetDeleteOptionEnabled();
	int GetEditOptionEnabled();
	int GetAddTransOptionEnabled();
	int GetAddAnimOptionEnabled();
	int GetAddGroupOptionEnabled();
    int GetScanSourceCodeOptionEnabled();
	void Remove(class CIDList &IDList);
//	CAnimTreeNode* GetNextNode();
	void Serialize(CArchive &ar);
	bool HasChildren();
	bool Edit();
	virtual int GetSelectedImage();
	virtual int GetImage();
	virtual int GetStateImage();
	int GetMouseMenuIndex();
//	const char * GetName() {return LPCTSTR( m_AnimName );};
	const CString& GetName() {return m_AnimName;}
		
//DEL 	const char * GetID()
//DEL 		{return LPCTSTR( m_AnimID );};
	void SetName(const char *lpszAnimName)
		{m_AnimName.Format( "%s", lpszAnimName );};

    CString m_Comment;
	CString m_FrameRate;

	CString m_LinkAnim;
	BOOL	m_bLinkMethod;
	BOOL	m_bLinkPriority;
	BOOL	m_bLinkFrameRate;
	BOOL	m_bLinkFlags;
	BOOL	m_bLinkStates;
	BOOL	m_bLinkTransitions;
	int		m_LinkEnterFrame;

	int m_NumPrerequisites;
	int m_NumTransitions;

	CEquivNode m_RLIDs;

private:
//	CIDList	m_IDList;	// The project file identifiers for this animation.
	int m_Method;
	int m_Flags;
	int m_States;
	int m_EndStates;
	int m_Priority;

	CLocoAnimTable* m_pCutTable;
	CString m_Callback;

	CString m_AnimName;	// The output enumeration identifier.

};

#endif // !defined(AFX_ANIMNODE_H__2DE9B942_6F29_11D2_9248_00105A29F8F3__INCLUDED_)


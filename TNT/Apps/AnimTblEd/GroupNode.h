// GroupNode.h: interface for the CGroupNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GROUPNODE_H__2DE9B943_6F29_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_GROUPNODE_H__2DE9B943_6F29_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AnimTblEdDefs.h"
#include "AnimTreeNode.h"
#include "LocoAnimTable.h"

class CGroupNode : public CAnimTreeNode  
{
public:

	CGroupNode();
	CGroupNode(class CAnimTblEdDoc *pDoc, CAnimTreeNode *pParent, int type = ANIMTREENODE_TYPE_GROUP, const char *lpszName = NULL);

	virtual ~CGroupNode();


	//---	callback related functions
	CString					GetCallback	( void )			{ return m_Callback; }
	void					SetCallback( CString Callback );
	CString					GetParentCallback( void )		{ if (GetParent()) return GetParent()->GetCallback(); return ""; };
	void					SetChildrenDefaultCallback( CString& Callback, CString& NewCallback );
	void					FillCallbackStringArray( CStringArray& rStringArray );

	//---	select callback related functions
	CString					GetSelectCallback( void )		{ return m_SelectCallback; }
	void					SetSelectCallback( CString Callback );
	CString					GetParentSelectCallback( void )		{ if (GetParent()) return GetParent()->GetSelectCallback(); return ""; };
	void					SetChildrenDefaultSelectCallback( CString& Callback, CString& NewCallback );
	void					FillSelectCallbackStringArray( CStringArray& rStringArray );

	//---	cut table related functions
	class CLocoAnimTable*	GetCutTable( void )				{ return m_pCutTable; }
	void					SetCutTable( class CLocoAnimTable* pCutTable );
	class CLocoAnimTable*	GetParentCutTable( void )		{ if (GetParent()) return GetParent()->GetCutTable(); return NULL; };
	void					SetChildrenDefaultCutTable( class CLocoAnimTable* pCutTable, CLocoAnimTable* pNewCutTable );

	//---	method related functions
	int						BS_GetMethod( void );
	void					BS_SetMethod( int Valu );
	int						BS_GetGroupMethod( void );
	void					ShiftMethods( int Operation, int Method );
	void					SetChildrenDefaultMethod( int CurValue, int NewValue );

	//---	flag related functions
	int						BS_GetFlags( void );
	void					BS_SetFlags( int Value );
	int						BS_GetGroupFlags( void );
	void					ShiftFlags( int Operation, int Flag );
	void					SetChildrenFlag( int Flag, int Value );

	//---	state related functions
	int						BS_GetStates( void );
	void					BS_SetStates( int Value );
	int						BS_GetGroupStates( void );
	void					ShiftStateClasses( int Operation, int StateClass, int State, int Bit, int Num );
	void					SetChildrenDefaultState( int StateClass, int CurValue, int NewValue );

	//---	priority related functions
	int						BS_GetPriority( void );
	void					BS_SetPriority( int Value );
	int						BS_GetGroupPriority( void );
	void					ShiftPriorities( int Operation, int Priority );
	void					SetChildrenDefaultPriority( int CurValue, int NewValue );

	//---	dependancies
	int						CountAllDependancies		( void );
	int						GetNumDependancies			( bool Recount=FALSE, bool CountDisabledOnes=FALSE, bool CountInheritedOnes=TRUE );
	int						GetNumParentDependancies	( bool Recount=FALSE, bool CountDisabledOnes=FALSE );
	CAnimTrans*				GetFirstDependancy			( void );
	CAnimTrans*				GetNextDependancy			( void );
	void					ClearDependancies			( void );
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

	void					SwitchCutTablePointer( void );
	CGroupNode*				GetParent( void )			{ return (CGroupNode*)CAnimTreeNode::GetParent(); }
	virtual int				GetState( void );
	virtual CAnimTreeNode*	FindTreeNodeByName( CString& Name );
	CString&				GetVPString( int *pDisplayOrder );
	void					AddToListBox( CListBox *pListBox, int nFlags );
	int						GetNumAnimsWithExtras( void );
	int						GetNumAnimsWithDependancies( bool Recount=FALSE, bool CountDisabledOnes=FALSE );
	int						GetTotalNumDependancies( bool Recount=FALSE, bool CountDisabledOnes=FALSE );
	int						GetNumAnimsWithDependanciesWithLink( bool Recount=FALSE, bool CountDisabledOnes=FALSE );
	int						GetTotalNumDependanciesWithLink( bool Recount=FALSE, bool CountDisabledOnes=FALSE );
	CAnimTreeNode*			Clone();
	int						GetNumAnims();
	int						GetNumExportedAnims();
	CAnimTreeNode*			GetPrevChild(CAnimTreeNode *pNodeAfter);
	int						CanHaveChildren();
	void					Copy(CAnimTreeNode &AnimTreeNode);
	int						GetGroupCount();
	int						GetGroupExportCount();

	void					Export(FILE *pFile);
	int						GetDeleteOptionEnabled();
	int						GetEditOptionEnabled();
	int						GetAddAnimOptionEnabled();
	int						GetAddGroupOptionEnabled();
    int                     GetScanSourceCodeOptionEnabled();
	void					SetFirstChild();
	void					Serialize(CArchive &ar);
	bool					HasChildren();
	void					Remove(class CIDList &IDList);
	int						GetSelectedImage();
	int						GetImage();
	virtual bool			Edit();
	int						GetMouseMenuIndex();
	void					SetName(const char *lpszName)	{m_Name.Format( "%s", lpszName );};
	const CString&			GetName()						{return m_Name;}
//	const char*				GetName()						{return LPCTSTR( m_Name );};

	CString m_Name;
	CString m_Comment;

	int m_NumPrerequisites;
	int m_NumTransitions;

	CAnimTransList m_AnimTransList;

private:
	CString			m_SelectCallback;
	CString			m_Callback;
	CLocoAnimTable*	m_pCutTable;

	int m_Method;
	int m_Flags;
	int m_States;
	int m_Priority;
};

#endif // !defined(AFX_GROUPNODE_H__2DE9B943_6F29_11D2_9248_00105A29F8F3__INCLUDED_)

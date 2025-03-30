// eNode.h: interface for the CeNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENODE_H__A6066AA0_DEAA_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_ENODE_H__A6066AA0_DEAA_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "x_math.hpp"

class CeNode ;
class CRenderContext ;

///////////////////////////////////////////////////////////////////////////////
//	NodeLink Class

struct CeNodeLink
{
	CeNodeLink	*m_pNext ;			// Link to Next NodeLink
	CeNodeLink	*m_pPrev ;			// Link to Prev NodeLink
	
	CeNode		*m_pNode ;			// Pointer to the actual Node
} ;

typedef struct CeNodeLink * NODEPOS ;

///////////////////////////////////////////////////////////////////////////////
//	NodeList Class

class CeNodeList
{
public:
	CeNodeList();
	~CeNodeList();

//---	Attributes
public:
	CeNodeLink	*m_pHead ;
	CeNodeLink	*m_pTail ;
	int			m_Count ;

//---	Functions
public:
	//---	Head / Tail Access
	CeNode		*GetHead( ) ;
	CeNode		*GetTail( ) ;

	//---	Operations
	CeNode		*RemoveHead( ) ;
	CeNode		*RemoveTail( ) ;
	NODEPOS		AddHead( CeNode *pNode ) ;
	NODEPOS		AddTail( CeNode *pNode ) ;
	void		RemoveAll( ) ;

	//---	Iteration
	NODEPOS		GetHeadPosition( ) ;
	NODEPOS		GetTailPosition( ) ;
	CeNode		*GetNext( NODEPOS &Pos ) ;
	CeNode		*GetPrev( NODEPOS &Pos ) ;

	//---	Retreival / Modification
	CeNode		*GetAt( NODEPOS Pos ) ;
	void		SetAt( NODEPOS Pos, CeNode *pNode ) ;
	CeNode		*RemoveAt( NODEPOS Pos ) ;
	CeNode		*Remove( CeNode *pNode ) ;

	//---	Insertion
	NODEPOS		InsertBefore( NODEPOS Pos, CeNode *pNode ) ;
	NODEPOS		InsertAfter( NODEPOS Pos, CeNode *pNode ) ;

	//---	Searching
	NODEPOS		Find( CeNode *pSearchNode, NODEPOS StartAfter = NULL ) ;
	NODEPOS		IndexToPosition( int Index ) ;
	int			PositionToIndex( NODEPOS Pos ) ;

	//---	Status
	int			GetCount( ) ;
	bool		IsEmpty( ) ;
} ;

///////////////////////////////////////////////////////////////////////////////
//	Node Class

#define		NCLASS_NULL			0x0000					// No Class
#define		NCLASS_RENDER		0x0001					// Render Node
#define		NCLASS_MANIPULATOR	0x0002					// Node Manipulator
#define		NCLASS_GUI			0x0003					// GUI Node, ie. Dialog

class CeNode
{
public:
	CeNode();
	virtual ~CeNode();

//---	Attributes
protected:
	u32					m_CompositeClass ;				// Node Class:SubClass (16:16)

	CeNodeList			m_Parents ;						// List of Parents
	CeNodeList			m_Children ;					// List of Children
	CeNodeList			m_Referers;						// List of Referers
	CeNodeList			m_References ;					// List of References

	CString				m_Name ;						// Node Name

	matrix4				m_Matrix ;						// Matrix for Position / Orientation
	vector3 			m_BoundMin ;					// Min XYZ of Bounding Box in Local Space
	vector3 			m_BoundMax ;					// Max XYZ of Bounding Box in Local Space

	bool				m_Hilited ;						// Is Hilited
	bool				m_Selected ;					// Is Selected
	bool				m_Modified ;					// Node has been Modified since loading

//---	Functions
public:
	//---	Scene Graph heirarchy
	void				AddParent( CeNode *pNode ) ;
	void				RemoveParent( CeNode *pNode ) ;
	void				AddChild( CeNode *pNode ) ;
	void				RemoveChild( CeNode *pNode ) ;

	//---	Reference Tracking
	void				AddReferer( CeNode *pNode ) ;
	void				RemoveReferer( CeNode *pNode ) ;
	void				AddReference( CeNode *pNode ) ;
	void				RemoveReference( CeNode *pNode ) ;

	//---	Class
	u16					GetClass( ) ;
	u16					GetSubClass( ) ;
	void				SetClass( u16 Class ) ;
	void				SetSubClass( u16 SubClass ) ;

	//---	Node State
	virtual bool		SetSelected( bool State ) ;
	virtual bool		SetHilited( bool State ) ;
	virtual bool		SetModified( bool State ) ;
	virtual bool		IsHilited( ) ;
	virtual bool		IsSelected( ) ;
	virtual bool		IsModified( ) ;

	//---	Rendering
	virtual void		Render( CRenderContext *pRC ) ;
//	virtual void		GraphRender( CGraphRenderContext *pRC ) ;
//	virtual bool		HitTest( CeHitTest *pHitTest ) ;
//	virtual bool		SendMessage( CeMsg *pMessage ) ;
};

#endif // !defined(AFX_ENODE_H__A6066AA0_DEAA_11D1_AEC0_00A024569FF3__INCLUDED_)

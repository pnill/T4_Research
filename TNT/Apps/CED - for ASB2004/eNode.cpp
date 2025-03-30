// eNode.cpp: implementation of the CeNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "eNode.h"
#include "RenderContext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
//	NodeList Class

CeNodeList::CeNodeList()
{
	m_pHead = NULL ;
	m_pTail = NULL ;
	m_Count = 0 ;
}

CeNodeList::~CeNodeList()
{
	RemoveAll( ) ;
}

//---	Head / Tail Access
CeNode *CeNodeList::GetHead( )
{
	CeNode *pNode = NULL ;
	if( m_pHead )
		pNode = m_pHead->m_pNode ;
	return pNode ;
}

CeNode *CeNodeList::GetTail( )
{
	CeNode *pNode = NULL ;
	if( m_pTail )
		pNode = m_pTail->m_pNode ;
	return pNode ;
}

//---	Operations
CeNode *CeNodeList::RemoveHead( )
{
	return RemoveAt( m_pHead ) ;
}

CeNode *CeNodeList::RemoveTail( )
{
	return RemoveAt( m_pTail ) ;
}

NODEPOS CeNodeList::AddHead( CeNode *pNode )
{
	CeNodeLink *pLink = new CeNodeLink ;
	ASSERT( pLink ) ;
	if( pLink )
	{
		pLink->m_pNode = pNode ;
		pLink->m_pNext = m_pHead ;
		pLink->m_pPrev = NULL ;
		m_pHead = pLink ;
		m_Count++ ;
	}
	return pLink ;
}

NODEPOS	CeNodeList::AddTail( CeNode *pNode )
{
	CeNodeLink *pLink = new CeNodeLink ;
	ASSERT( pLink ) ;
	if( pLink )
	{
		pLink->m_pNode = pNode ;
		pLink->m_pNext = NULL ;
		pLink->m_pPrev = m_pTail ;
		m_pTail = pLink ;
		m_Count++ ;
	}
	return pLink ;
}

void CeNodeList::RemoveAll( )
{
	while( m_pHead != NULL )
	{
		RemoveAt( m_pHead ) ;
	}
}

//---	Iteration
NODEPOS CeNodeList::GetHeadPosition( )
{
	return m_pHead ;
}

NODEPOS	CeNodeList::GetTailPosition( )
{
	return m_pTail ;
}

CeNode *CeNodeList::GetNext( NODEPOS &Pos )
{
	CeNode *pNode = Pos->m_pNode ;
	Pos = Pos->m_pNext ;
	return pNode ;
}

CeNode *CeNodeList::GetPrev( NODEPOS &Pos )
{
	CeNode *pNode = Pos->m_pNode ;
	Pos = Pos->m_pPrev ;
	return pNode ;
}

//---	Retreival / Modification
CeNode *CeNodeList::GetAt( NODEPOS Pos )
{
	return Pos->m_pNode ;
}

void CeNodeList::SetAt( NODEPOS Pos, CeNode *pNode )
{
	Pos->m_pNode = pNode ;
}

CeNode *CeNodeList::RemoveAt( NODEPOS Pos )
{
	//---	Set Prev Node or Head Pointer
	if( Pos->m_pPrev )
		Pos->m_pPrev->m_pNext = Pos->m_pNext ;
	else
		m_pHead = Pos->m_pNext ;

	//---	Set Next Node or Tail Pointer
	if( Pos->m_pNext )
		Pos->m_pNext->m_pPrev = Pos->m_pPrev ;
	else
		m_pTail = Pos->m_pPrev ;

	CeNode *pNode = Pos->m_pNode ;
	delete Pos ;
	m_Count-- ;

	return pNode ;
}

CeNode *CeNodeList::Remove( CeNode *pNode )
{
	NODEPOS Pos = Find( pNode ) ;
	ASSERT( Pos ) ;
	if( Pos )
		RemoveAt( Pos ) ;
	return pNode ;
}

//---	Insertion
NODEPOS CeNodeList::InsertBefore( NODEPOS Pos, CeNode *pNode )
{
	ASSERT( Pos ) ;
	CeNodeLink *pLink = new CeNodeLink ;
	ASSERT( pLink ) ;
	if( pLink )
	{
		pLink->m_pNode = pNode ;
		pLink->m_pNext = Pos ;
		pLink->m_pPrev = Pos->m_pPrev ;
		Pos->m_pPrev = pLink ;
		if( pLink->m_pPrev )
			pLink->m_pPrev->m_pNext = pLink ;
		else
			m_pHead = pLink ;
		m_Count++ ;
	}
	return pLink ;
}

NODEPOS CeNodeList::InsertAfter( NODEPOS Pos, CeNode *pNode )
{
	ASSERT( Pos ) ;
	CeNodeLink *pLink = new CeNodeLink ;
	ASSERT( pLink ) ;
	if( pLink )
	{
		pLink->m_pNode = pNode ;
		pLink->m_pNext = Pos->m_pNext ;
		pLink->m_pPrev = Pos ;
		Pos->m_pNext = pLink ;
		if( pLink->m_pNext )
			pLink->m_pNext->m_pPrev = pLink ;
		else
			m_pTail = pLink ;
		m_Count++ ;
	}
	return pLink ;
}

//---	Searching
NODEPOS	CeNodeList::Find( CeNode *pSearchNode, NODEPOS StartAfter )
{
	NODEPOS Pos = m_pHead ;

	if( StartAfter )
		Pos = StartAfter ;

	while( Pos )
	{
		if( Pos->m_pNode == pSearchNode ) break ;
		Pos = Pos->m_pNext ;
	}

	return Pos ;
}

NODEPOS	CeNodeList::IndexToPosition( int Index )
{
	NODEPOS Pos = m_pHead ;
	while( Index-- )
	{
		Pos = Pos->m_pNext ;
	}
	return Pos ;
}

int CeNodeList::PositionToIndex( NODEPOS Pos )
{
	int		Index = 0 ;
	NODEPOS ScanPos = m_pHead ;
	while( ScanPos )
	{
		if( ScanPos == Pos ) break ;
		ScanPos = ScanPos->m_pNext ;
		Index++ ;
	}
	if( ScanPos == NULL ) Index = -1 ;
	return Index ;
}

//---	Status
int	CeNodeList::GetCount( )
{
	return m_Count ;
}

bool CeNodeList::IsEmpty( )
{
	return (m_Count == 0) ;
}

///////////////////////////////////////////////////////////////////////////////
//	Node Class

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CeNode::CeNode()
{
	m_CompositeClass = 0 ;
	m_Selected = false ;
	m_Hilited = false ;
	m_Modified = false ;
	V3_Zero( &m_BoundMin ) ;
	V3_Zero( &m_BoundMax ) ;
	M4_Identity( &m_Matrix ) ;
}

CeNode::~CeNode()
{

}

//////////////////////////////////////////////////////////////////////
// Operations

//////////////////////////////////////////////////////////////////////
// Scene Graph Operations

void CeNode::AddParent( CeNode *pNode )
{
	m_Parents.AddTail( pNode ) ;
}

void CeNode::RemoveParent( CeNode *pNode )
{
	m_Parents.Remove( pNode ) ;
}

void CeNode::AddChild( CeNode *pNode )
{
	m_Children.AddTail( pNode ) ;
}

void CeNode::RemoveChild( CeNode *pNode )
{
	m_Children.Remove( pNode ) ;
}

//////////////////////////////////////////////////////////////////////
// Reference Operations

void CeNode::AddReferer( CeNode *pNode )
{
	m_Referers.AddTail( pNode ) ;
}

void CeNode::RemoveReferer( CeNode *pNode )
{
	m_Referers.Remove( pNode ) ;
}

void CeNode::AddReference( CeNode *pNode )
{
	m_References.AddTail( pNode ) ;
}

void CeNode::RemoveReference( CeNode *pNode )
{
	m_References.Remove( pNode ) ;
}

//////////////////////////////////////////////////////////////////////
// Class Operations

u16 CeNode::GetClass( )
{
	return (u16)(m_CompositeClass >> 16) ;
}

u16 CeNode::GetSubClass( )
{
	return (u16)(m_CompositeClass & 0xffff ) ;
}

void CeNode::SetClass( u16 Class )
{
	m_CompositeClass &= 0x0000ffff ;
	m_CompositeClass |= ((u32)Class) << 16 ;
}

void CeNode::SetSubClass( u16 SubClass )
{
	m_CompositeClass &= 0xffff0000 ;
	m_CompositeClass |= (u32)SubClass ;
}

//////////////////////////////////////////////////////////////////////
// Flag / State Operations

bool CeNode::SetSelected( bool State )
{
	m_Selected = State ;
	return m_Selected ;
}

bool CeNode::SetHilited( bool State )
{
	m_Hilited = State ;
	return m_Hilited ;
}

bool CeNode::SetModified( bool State )
{
	m_Modified = State ;
	return m_Modified ;
}

bool CeNode::IsSelected( )
{
	return m_Selected ;
}

bool CeNode::IsHilited( )
{
	return m_Hilited ;
}

bool CeNode::IsModified( )
{
	return m_Modified ;
}

//////////////////////////////////////////////////////////////////////
// Render Operations

void CeNode::Render( CRenderContext *pRC )
{
	//---	Add this Node to the Stack, Render all Children and Pop the Node
	pRC->PushNode( this ) ;
	pRC->PushMatrix( &m_Matrix ) ;
	NODEPOS Pos = m_Children.GetHeadPosition() ;
	while( Pos )
	{
		CeNode *pNode = m_Children.GetNext( Pos ) ;
		if( pNode )
		{
			pNode->Render( pRC ) ;
		}
	}
	pRC->PopMatrix( ) ;
	pRC->PopNode( ) ;
}

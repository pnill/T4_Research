// EquivNode.cpp: implementation of the CEquivNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDefs.h"
#include "EquivNode.h"
#include "IDList.h"
#include "EquivDlg.h"

extern int CurrentExportType;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//---	These defines are only needed in this module.
#define	LEFTANIM_MIRRORED	0x00000001
#define LEFTANIM_REVERSED	0x00000002
#define RIGHTANIM_MIRRORED	0x00000004
#define RIGHTANIM_REVERSED	0x00000008

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEquivNode::CEquivNode(class CAnimTblEdDoc *pDoc, CAnimTreeNode *pParent):CAnimTreeNode( pDoc, pParent )
{
	//---	Set the type into the base structure.
	m_Type = ANIMTREENODE_TYPE_EQUIV;
	m_Flags = 0;
	SetLeftMirrored( TRUE );
	SetRightMirrored( FALSE );
}

CEquivNode::CEquivNode()
{
	//---	Set the type into the base structure.
	m_Type = ANIMTREENODE_TYPE_EQUIV;
	m_Flags = 0;
	SetLeftMirrored( TRUE );
	SetRightMirrored( FALSE );
}

CEquivNode::~CEquivNode()
{

}

void CEquivNode::SetLeftID(const char *lpszLeftID, CIDList *pIDList)
{
	//---	never remove an ID
	if (strlen(lpszLeftID) == 0)
		return;

	if( !m_pParent || !m_pParent->GetIgnore() )
		pIDList->DecrementUses( m_LeftID );

	m_LeftID.Format( "%s", lpszLeftID );

	if( !m_pParent || !m_pParent->GetIgnore() )
		pIDList->IncrementUses( m_LeftID );

	//---	When the left ID is set and the Right one is NULL then the right one will be set to a default value.
	if ( m_RightID.IsEmpty() && pIDList)
	{
		SetRightID( lpszLeftID, pIDList );
		SetRightMirrored( !GetLeftMirrored() );
	}

	//---	Rebuild the proper name for this animation.
	SetDefaultName();
}


void CEquivNode::SetRightID(const char *lpszRightID, CIDList *pIDList)
{
	//---	never remove an ID
	if (strlen(lpszRightID) == 0)
		return;

	if( !m_pParent || !m_pParent->GetIgnore() )
		pIDList->DecrementUses( m_RightID );
	
	m_RightID.Format( "%s", lpszRightID );

	if( !m_pParent || !m_pParent->GetIgnore() )
		pIDList->IncrementUses( m_RightID );

	//---	When the left ID is set and the Right one is NULL then the right one will be set to a default value.
	if ( m_LeftID.IsEmpty() && pIDList)
	{
		SetLeftID( lpszRightID, pIDList );
		SetLeftMirrored( !GetRightMirrored() );
	}

	//---	Rebuild the proper name for this animation.
	SetDefaultName();
}


const CString& CEquivNode::GetLeftID()
{
	return m_LeftID;
}

const CString& CEquivNode::GetRightID()
{
	return m_RightID;
}

void CEquivNode::SetLeftMirrored(bool Set)
{
	if (Set)
		m_Flags |= LEFTANIM_MIRRORED;
	else
		m_Flags &= ~LEFTANIM_MIRRORED;

	//---	Rebuild the proper name for this animation.
	SetDefaultName();
}

void CEquivNode::SetLeftReversed(bool Set)
{
	if (Set)
		m_Flags |= LEFTANIM_REVERSED;
	else
		m_Flags &= ~LEFTANIM_REVERSED;

	//---	Rebuild the proper name for this animation.
	SetDefaultName();
}

bool CEquivNode::GetLeftMirrored()
{
	return (m_Flags & LEFTANIM_MIRRORED) ? TRUE : FALSE;
}

bool CEquivNode::GetLeftReversed()
{
	return (m_Flags & LEFTANIM_REVERSED) ? TRUE : FALSE;
}

void CEquivNode::SetRightMirrored(bool Set)
{
	if (Set)
		m_Flags |= RIGHTANIM_MIRRORED;
	else
		m_Flags &= ~RIGHTANIM_MIRRORED;

	//---	Rebuild the proper name for this animation.
	SetDefaultName();
}

void CEquivNode::SetRightReversed(bool Set)
{
	if (Set)
		m_Flags |= RIGHTANIM_REVERSED;
	else
		m_Flags &= ~RIGHTANIM_REVERSED;

	//---	Rebuild the proper name for this animation.
	SetDefaultName();
}

bool CEquivNode::GetRightMirrored()
{
	return (m_Flags & RIGHTANIM_MIRRORED) ? TRUE : FALSE;
}

bool CEquivNode::GetRightReversed()
{
	return (m_Flags & RIGHTANIM_REVERSED) ? TRUE : FALSE;
}

int CEquivNode::GetImage()
{
	return TREEIMAGE_EQUIV;
}

int CEquivNode::GetSelectedImage()
{
	return TREEIMAGE_EQUIV_SELECT;
}

int CEquivNode::GetMouseMenuIndex()
{
	return 2;
}

bool CEquivNode::HasChildren()
{
	return FALSE;
}

int CEquivNode::GetAddGroupOptionEnabled()
{
	return MF_GRAYED;
}

int CEquivNode::GetAddAnimOptionEnabled()
{
	return MF_GRAYED;
}

int CEquivNode::GetAddTransOptionEnabled()
{
	return MF_ENABLED;
}

int CEquivNode::GetEditOptionEnabled()
{
	return MF_ENABLED;
}

int CEquivNode::GetDeleteOptionEnabled()
{
	return MF_ENABLED;
}

void CEquivNode::Remove(CIDList &IDList)
{
	IDList.DecrementUses( GetLeftID() );
	IDList.DecrementUses( GetRightID() );
}


bool CEquivNode::Edit()
{
	CEquivDlg	EquivEditDlg;

	if (m_Type == ANIMTREENODE_TYPE_EQUIV)
	{
		EquivEditDlg.m_pEquivNode = this;
		EquivEditDlg.m_pIDList = &this->GetDocument()->m_IDList;
		if (EquivEditDlg.DoModal() == IDOK)
			return TRUE;	
	}

	return FALSE;
}

void CEquivNode::SetDefaultName()
{
	m_Name.Empty();
	
	//---	Build the LeftID portion of the name.
	if (GetLeftReversed())
		m_Name += "-";
	else
		m_Name += "+";

	if (GetLeftMirrored())
		m_Name += "~";

	m_Name += m_LeftID;

	//---	Add separator
	m_Name += "  |  ";

	//---	Build the RightID portion of the name.
	if (GetRightReversed())
		m_Name += "-";
	else
		m_Name += "+";

	if (GetRightMirrored())
		m_Name += "~";

	m_Name += m_RightID;
}

void CEquivNode::Serialize(CArchive &ar)
{
	short int	version;

	if (ar.IsStoring())
	{
		//---	Usual version stuff.
		version = 1;
		ar.Write( &version, 2 );

		//---	Equiv specific stuff.
		ar.WriteString( m_LeftID );
		ar.WriteString( "\xd\xa" );

		ar.WriteString( m_RightID );
		ar.WriteString( "\xd\xa" );

		ar << m_Flags;
	}
	else
	{
		ar.Read( &version, 2 );
		switch(version)
		{
		case 1:
			ar.ReadString( m_LeftID );
			ar.ReadString( m_RightID );
			ar >> m_Flags;

			SetDefaultName();
			break;
		}
	}
}

void CEquivNode::Export(FILE *pFile)
{
//	if (CurrentExportType == EXPORTTYPE_ANIMOBJECTS)
	if (CurrentExportType == EXPORTTYPE_OBJECTLIST)
	{
		fprintf( pFile, "{{" );
		
		//---	Right animation.
		if (GetRightMirrored())
			fprintf( pFile, "ANM_MIR_BIT|" );

		if (GetRightReversed())
			fprintf( pFile, "ANM_REV_BIT|" );

		fprintf( pFile, "%s/* Right */,", GetRightID() );

		//---	Left animation.
		if (GetLeftMirrored())
			fprintf( pFile, "ANM_MIR_BIT|" );

		if (GetLeftReversed())
			fprintf( pFile, "ANM_REV_BIT|" );

		fprintf( pFile, "%s/* Left */ ", GetLeftID() );


		fprintf( pFile, "}}," );
	}
}

void CEquivNode::Copy(CAnimTreeNode &AnimTreeNode)
{
	CEquivNode	*pEquivNode = (CEquivNode*)&AnimTreeNode;

	//---	Copy the base node.
	pEquivNode->CopyBaseNode( AnimTreeNode );

	//---	Copy the Equiv fields.
	m_LeftID.Format( "%s", pEquivNode->m_LeftID );
	m_RightID.Format( "%s", pEquivNode->m_RightID );
	m_Flags = pEquivNode->m_Flags;
	this->SetDefaultName();
}

int CEquivNode::CanHaveChildren()
{
	return FALSE;
}

CAnimTreeNode * CEquivNode::Clone()
{
	CEquivNode *pEquivNode = new CEquivNode( m_pDoc, m_pParent );

	pEquivNode->Copy( *this );

	return pEquivNode;
}

void CEquivNode::Swap()
{
	CString Temp = m_LeftID;
	m_LeftID = m_RightID;
	m_RightID = Temp;

	bool LeftMirrored = (m_Flags&LEFTANIM_MIRRORED) ? TRUE : FALSE;
	bool LeftReversed = (m_Flags&LEFTANIM_REVERSED) ? TRUE : FALSE;
	bool RightMirrored = (m_Flags&RIGHTANIM_MIRRORED) ? TRUE : FALSE;
	bool RightReversed = (m_Flags&RIGHTANIM_REVERSED) ? TRUE : FALSE;

	m_Flags &= ~(LEFTANIM_MIRRORED|LEFTANIM_REVERSED|RIGHTANIM_MIRRORED|RIGHTANIM_REVERSED);
	m_Flags |=	((LeftMirrored ? RIGHTANIM_MIRRORED : 0) |
				 (LeftReversed ? RIGHTANIM_REVERSED : 0) |
				 (RightMirrored ? LEFTANIM_MIRRORED : 0) |
				 (RightReversed ? LEFTANIM_REVERSED : 0));
}

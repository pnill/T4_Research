// LocoAnimNSR.cpp: implementation of the CLocoAnimNSR class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "LocoAnimNSR.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "AnimTreeNode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLocoAnimNSR::CLocoAnimNSR()
{
	m_NSR = 1.0f;
	m_Anim = -1;
}

CLocoAnimNSR::~CLocoAnimNSR()
{

}

void CLocoAnimNSR::Serialize(CArchive& ar)
{
	int version = 1;

	if (ar.IsStoring())
	{
		ar << version;

		ar << m_NSR;
		ar << m_Anim;
	}
	else
	{
		ar >> version;
		ar >> m_NSR;
		ar >> m_Anim;
	}
}

void CLocoAnimNSR::Export( FILE* pFile, class CAnimTblEdDoc* pDoc, CString& rParentName, int Index, CStringArray& rAnimationList, float& rLastNSR, float OverlapNSR )
{
	CString Anim;
	float MinNSR, MaxNSR;
	CAnimTreeNode* pNode;

	//---	find this animation
	if (m_Anim == -1) pNode = NULL;
	else pNode = pDoc->m_AnimTree.FindTreeNodeByName( rAnimationList[ m_Anim ] );

	if (pNode)	Anim = pNode->GetName();
	else		Anim = "NONE";

	//---	get the NSR range
	if (OverlapNSR >= 0.0f)
	{
		MinNSR = rLastNSR;
		MaxNSR = m_NSR + OverlapNSR;
	}
	else
	{
		MinNSR = rLastNSR + OverlapNSR;
		MaxNSR = m_NSR;
	}

	//---	print the header information
	fprintf( pFile, "\t/* Zone %2d */ { ANIM_%s, %6.4ff, %6.4ff },\t// { Anim, MinNSR, MaxNSR }\n", Index, Anim, MinNSR, MaxNSR );

	rLastNSR = m_NSR;
}

CLocoAnimNSR& CLocoAnimNSR::operator =(CLocoAnimNSR &Other)
{
	m_Anim = Other.m_Anim;
	m_NSR = Other.m_NSR;

	return *this;
}

int CLocoAnimNSR::operator ==(CLocoAnimNSR &Other)
{
	if (m_Anim != Other.m_Anim)	return FALSE;
	if (m_NSR != Other.m_NSR)	return FALSE;

	return TRUE;
}


void CLocoAnimNSR::RemoveAnim( int Index )
{
	if (m_Anim == Index)		m_Anim = -1;
	else if (m_Anim > Index)	m_Anim--;
}

void CLocoAnimNSR::InsertAnim( int Index )
{
	if (m_Anim >= Index)
		m_Anim++;
}

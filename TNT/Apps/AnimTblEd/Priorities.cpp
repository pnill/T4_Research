// Priorities.cpp: implementation of the CPriorities class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "Priorities.h"
#include "AnimTreeNode.h"
#include "AnimNode.h"
#include "GroupNode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPriorities::CPriorities()
{
	AddPriority(CString("DEFAULT"));
}

CPriorities::~CPriorities()
{
	MakeEmpty();
}

void CPriorities::AddPriority(CString &Name, int Index)
{
	if (Index >= 0)
	{
		POSITION pos = m_Priorities.FindIndex(Index);
		if (pos)
		{
			m_Priorities.InsertBefore(pos, Name);
			return;
		}
	}

	m_Priorities.AddTail(Name);
}

int CPriorities::GetCount()
{
	return m_Priorities.GetCount();
}

int CPriorities::GetTotalPriorityBits()
{
	return CountNumBits(GetCount());
}

void CPriorities::MakeEmpty()
{
	while (m_Priorities.GetHeadPosition())
		m_Priorities.RemoveHead();
}

CPriorities& CPriorities::operator =(CPriorities &Other)
{
	CString Priority;
	int i;

	MakeEmpty();
	for (i=0; i<Other.GetCount(); i++)
		AddPriority(Other.GetPriority(i));

	return *this;
}

void CPriorities::RemovePriority(int Index)
{
	POSITION Pos = m_Priorities.FindIndex(Index);
	m_Priorities.RemoveAt(Pos);
}

int CPriorities::FindPriority(CString &Name)
{
	POSITION Pos;
	CString Priority;
	int i;

	Pos = m_Priorities.GetHeadPosition();
	i = 0;
	while (Pos)
	{
		Priority = m_Priorities.GetNext(Pos);
		if (Priority == Name)
			return i;
		i++;
	}

	return -1;
}

CString& CPriorities::GetPriority(int Index)
{
	static CString NullName="";

	POSITION Pos = m_Priorities.FindIndex(Index);

	if (Pos == NULL)
		return NullName;

	return m_Priorities.GetAt(Pos);
}

int CPriorities::CountNumBits(int Value)
{
	int s=Value;
	int n=0;

	if (s == 0)
		return 0;

	s--;
	do
	{
		s >>= 1;
		n++;
	}
	while (s > 0);

	return n;
}

int CPriorities::GetMask(int StartBit)
{
	int bits = GetTotalPriorityBits();
	int mask;

	if (bits == 0)
		return 0;

	mask = 1;
	do
	{
		mask *= 2;
	}
	while (--bits);

	return (mask - 1) << StartBit;
}

void CPriorities::Serialize(CArchive &ar)
{
	int version;
	int i, count;
	CString Name;

	if (ar.IsStoring())
	{
		version = 1;
		ar << version;

		//---	write out each of the priorities
		count = GetCount();
		ar << count;
		for (i=0; i<count; i++)
		{
			ar.WriteString(GetPriority(i));
			ar.WriteString("\xd\xa");
		}
	}
	else
	{
		ar >> version;

		//---	laod in each of the priorities
		MakeEmpty();
		ar >> count;
		for (i=0; i<count; i++)
		{
			ar.ReadString(Name);
			AddPriority(Name);
		}
	}
}

void CPriorities::ShiftPriorities(CAnimTreeNode *pNode, int Operation, int Priority)
{
	CAnimTreeNode *pChild;

	if (!pNode)
		return;

	if (pNode->m_Type == ANIMTREENODE_TYPE_GROUP)
		((CGroupNode*) pNode)->ShiftPriorities(Operation, Priority);
	else if (pNode->m_Type == ANIMTREENODE_TYPE_ANIM)
		((CAnimNode*) pNode)->ShiftPriorities(Operation, Priority);

	if (!pNode->HasChildren())
		return;

	pChild = pNode->GetFirstChild();
	while (pChild)
	{
		ShiftPriorities(pChild, Operation, Priority);
		pChild = pNode->GetNextChild();
	}
}

void CPriorities::ExportDefines(FILE *pFile, int Shift)
{
	int i;

	fprintf(pFile, "\n//---   Animation priority defines\n");
	fprintf(pFile, "#define\tANIMBF_PRIORITYMASK\t\t0x%x\n", ((1 << GetTotalPriorityBits()) - 1) << Shift);
	fprintf(pFile, "#define\tANIMBF_PRIORITYSHIFT\t\t%d\n", Shift);

	fprintf(pFile, "\n//---   Animation priorities\n");
	for (i=0; i<this->GetCount(); i++)
		fprintf(pFile, "#define\tANIMPRIORITY_%s\t\t%d\n", GetPriority(i), i);
}

void CPriorities::ExportMacros(FILE *pFile, int Shift)
{
	fprintf(pFile, "\n//---   Animation priority macros\n");
	fprintf(pFile, "#define\tANIMBF_GETPRIORITY(v)\t\t(((v) & ANIMBF_PRIORITYMASK) >> ANIMBF_PRIORITYSHIFT)\n");
	fprintf(pFile, "#define\tANIM_GETPRIORITY(a)\t\tANIMBF_GETPRIORITY(AnimTable[a].BitField)\n");
	fprintf(pFile, "#define\t_PRIORITY_(v)\t((v) << ANIMBF_PRIORITYSHIFT)\n" );
}

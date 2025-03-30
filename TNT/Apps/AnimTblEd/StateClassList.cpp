// StateClassList.cpp: implementation of the CStateClassList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "AnimTreeNode.h"
#include "StateClassList.h"
#include "GroupNode.h"
#include "AnimNode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStateClassList::CStateClassList()
{

}

CStateClassList::~CStateClassList()
{
	MakeEmpty();
}

int CStateClassList::GetTotalStateBits()
{
	POSITION pos;
	CStateClass *pStateClass;
	int count=0;

	pos = m_StateClassList.GetHeadPosition();
	while (pos)
	{
		pStateClass = m_StateClassList.GetNext(pos);
		count += pStateClass->GetNumBits();
	}

	return count;
}

int CStateClassList::AddStateClass(CString &Name)
{
	CStateClass *pNewClass = new CStateClass(this, Name);

	m_StateClassList.AddTail(pNewClass);
	return m_StateClassList.GetCount() - 1;
}

int CStateClassList::RemoveStateClass(int Index)
{
	POSITION pos = m_StateClassList.FindIndex(Index);
	
	if (!pos)
		return FALSE;

	m_StateClassList.RemoveAt(pos);
	return TRUE;
}

int CStateClassList::RemoveStateClass(CString &Name)
{
	CStateClass *pFoundStateClass;
	POSITION pos;
	int i;

	for (i=0; i<GetCount(); i++)
	{
		pos = m_StateClassList.FindIndex(i);
		if (pos)
		{
			pFoundStateClass = m_StateClassList.GetAt(pos);
			if (pFoundStateClass->GetName() == Name)
			{
				m_StateClassList.RemoveAt(pos);
				delete pFoundStateClass;
				return TRUE;
			}
		}
	}

	return FALSE;
}

int CStateClassList::RemoveStateClass(CStateClass *pStateClass)
{
	CStateClass *pFoundStateClass;
	POSITION pos;
	int i;

	for (i=0; i<GetCount(); i++)
	{
		pos = m_StateClassList.FindIndex(i);
		if (pos)
		{
			pFoundStateClass = m_StateClassList.GetAt(pos);
			if (pFoundStateClass == pStateClass)
			{
				m_StateClassList.RemoveAt(pos);
				delete pFoundStateClass;
				return TRUE;
			}
		}
	}

	return FALSE;
}

CStateClass* CStateClassList::GetStateClass(int Index)
{
	POSITION pos = m_StateClassList.FindIndex(Index);

	if (!pos)
		return NULL;

	return m_StateClassList.GetAt(pos);
}

int CStateClassList::GetCount()
{
	return m_StateClassList.GetCount();
}

CStateClassList& CStateClassList::operator=(CStateClassList &NewList)
{
	int i;
	CStateClass *pStateClass;
	CStateClass *pNewClass;

	//---	empty the old list
	MakeEmpty();

	for (i=0; i<NewList.GetCount(); i++)
	{
		pStateClass = NewList.GetStateClass(i);
		pNewClass = new CStateClass(this);
		*pNewClass = *pStateClass;
		m_StateClassList.AddTail(pNewClass);
	}

	return *this;
}

void CStateClassList::MakeEmpty()
{
	int i=0;

	for (i=GetCount()-1; i>=0; i--)
		RemoveStateClass(i);
}

int CStateClassList::GetStateClassStartBit(int StateClass)
{
	int i;
	int c=0;

	//---	make sure that that many states exists
	if (GetCount() <= StateClass)
		return -1;

	//---	count the bits from all of the states before this one
	for (i=0; i<StateClass; i++)
		c += GetStateClass(i)->GetNumBits();

	return c;
}


int CStateClassList::GetStateClassStartBit(CStateClass *pClass)
{
	int i;
	int c=0;
	CStateClass *pStepClass;

	//---	count the bits from all of the states before this one
	for (i=0; i<GetCount(); i++)
	{
		pStepClass = GetStateClass(i);
		if (pStepClass == pClass)
			break;

		c += pStepClass->GetNumBits();
	}

	//---	if this state class does not exist in the list, return -1
	if (i == GetCount())
		return -1;

	return c;
}

void CStateClassList::SetStateClassName(int Index, CString &Name)
{
	CStateClass *pClass;
	
	pClass = GetStateClass(Index);

	if (pClass)
		pClass->SetName(Name);
}

CStateClass* CStateClassList::FindStateClass(CString &Name, CStateClass *pSkipStateClass)
{
	CStateClass *pFoundStateClass;
	POSITION pos;
	int i;

	for (i=0; i<GetCount(); i++)
	{
		pos = m_StateClassList.FindIndex(i);
		if (pos)
		{
			pFoundStateClass = m_StateClassList.GetAt(pos);
			if ((pFoundStateClass != pSkipStateClass) && (pFoundStateClass->GetName() == Name))
				return pFoundStateClass;
		}
	}

	return NULL;
}

int CStateClassList::GetMask(int StartBit)
{
	int bits = GetTotalStateBits();
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

void CStateClassList::Serialize(CArchive &ar)
{
	int version;
	int i;
	int count;

	if (ar.IsStoring())
	{
		version = 1;
		ar << version;

		//---	store the state classes
		ar << GetCount();
		for (i=0; i<GetCount(); i++)
			GetStateClass(i)->Serialize(ar);
	}
	else
	{
		ar >> version;

		//---	load the state classes
		ar >> count;
		for (i=0; i<count; i++)
		{
			AddStateClass();
			GetStateClass(i)->Serialize(ar);
		}
	}
}

void CStateClassList::ShiftStateClasses(CAnimTreeNode *pNode, int Operation, int StateClass, int State, int Bit, int Num)
{
	CAnimTreeNode* pChild;

	//---	return on trivial case
	if (!pNode)
		return;

	//---	shift the value for this item
	if (pNode->m_Type == ANIMTREENODE_TYPE_GROUP)
		((CGroupNode*) pNode)->ShiftStateClasses(Operation, StateClass, State, Bit, Num);
	else if (pNode->m_Type == ANIMTREENODE_TYPE_ANIM)
		((CAnimNode*) pNode)->ShiftStateClasses(Operation, StateClass, State, Bit, Num);

	//---	if this node has no children, return
	if (!pNode->HasChildren())
		return;

	//---	shift the value for the children
	pChild = pNode->GetFirstChild();
	while (pChild)
	{
		ShiftStateClasses(pChild, Operation, StateClass, State, Bit, Num);
		pChild = pNode->GetNextChild();
	}
}

void CStateClassList::ExportDefines(FILE *pFile, int Shift)
{
	CStateClass *pClass;
	int i, j;

	fprintf(pFile, "\n//---   Animation state defines\n");
	fprintf(pFile, "#define\tANIM_NUMSTATES\t\t\t%d\n", (1 << GetTotalStateBits()));
	fprintf(pFile, "#define\tANIMBF_STATEMASK\t\t0x%x\n", ((1 << GetTotalStateBits()) - 1) << Shift);
	fprintf(pFile, "#define\tANIMBF_STATESHIFT\t\t%d\n", Shift);

	for (i=0; i<this->GetCount(); i++)
	{
		pClass = GetStateClass(i);

		fprintf(pFile, "\n//---	Animation State '%s'\n", pClass->GetName());
		fprintf(pFile, "#define\tANIMBF_STATE_%s_MASK\t\t0x%x\n", pClass->GetName(), ((1 << pClass->GetNumBits()) - 1) << (pClass->GetStartBit() + Shift));
		fprintf(pFile, "#define\tANIMBF_STATE_%s_SHIFT\t\t%d\n", pClass->GetName(), (pClass->GetStartBit() + Shift));
		for (j=0; j<pClass->GetNumStates(); j++)
			fprintf(pFile, "#define\tANIMSTATE_%s_%s\t\t%d\n", pClass->GetName(), pClass->GetState(j)->GetName(), j);
		fprintf(pFile, "\n");
	}
}

void CStateClassList::ExportMacros(FILE *pFile, int Shift)
{
	CStateClass *pClass;
	int i;

	fprintf(pFile, "\n//---   Animation state macros\n");
	fprintf(pFile, "#define\tANIMBF_GETSTATES(v)\t(((v)&ANIMBF_STATEMASK) >> ANIMBF_STATESHIFT)\n");
	fprintf(pFile, "#define\tANIM_GETSTATES(a)\tANIMBF_GETSTATES(AnimTable[a].BitField)\n\n");
	fprintf( pFile, "#define\t_STATE_(v)\t((v) << ANIMBF_STATESHIFT)\n" );

	for (i=0; i<this->GetCount(); i++)
	{
		pClass = GetStateClass(i);

		fprintf(pFile, "#define\tANIMBF_GETSTATE_%s(v)\t(((v)&ANIMBF_STATE_%s_MASK) >> ANIMBF_STATE_%s_SHIFT)\n", pClass->GetName(), pClass->GetName(), pClass->GetName());
		fprintf(pFile, "#define\tANIM_GETSTATE_%s(a)\tANIMBF_GETSTATE_%s(AnimTable[a].BitField)\n", pClass->GetName(), pClass->GetName());
	}
}

void CStateClassList::ExportStateTransTable(FILE *pFile, CAnimTblEdDoc *pDoc)
{
	int NumStates = 1 << GetTotalStateBits();
	int i;
	CString *StateTransLists = new CString[NumStates];
	int ListIndex=0;

	fprintf( pFile, "\n//---	Animation State Transition Lists\n");

	fprintf( pFile, "\n");
	ExportStateTransList(pFile, pDoc, 0, 0, StateTransLists, ListIndex, CString("//---   "));

	fprintf( pFile, "\n//---	Animation State Transition Table\n");
	fprintf( pFile, "const s16 *StateTransTable[ANIM_NUMSTATES] =\n", NumStates);
	fprintf( pFile, "{\n");

	for (i=0; i<NumStates; i++)
	{
		if (StateTransLists[i].IsEmpty())
			fprintf( pFile, "\tNULL,\n");
		else
			fprintf( pFile, "\t%s,\n", StateTransLists[i]);
	}

	fprintf( pFile, "};\n");
}

void CStateClassList::ExportStateTransList(FILE *pFile, CAnimTblEdDoc *pDoc, int BitSet, int StateClass, CString *StateTransLists, int &ListIndex, CString Comment)
{
	CStateClass *pClass;
	int i;

	pClass = GetStateClass(StateClass);

	//---	if there is a state class then we must continue to the next level and add this class to the current value
	if (pClass)
	{
		//---	loop through each of the class states
		for (i=0; i<pClass->GetNumStates(); i++)
		{
			CString MoreComment;
			MoreComment.Format("ANIMSTATE_%s_%s | ", pClass->GetName(), pClass->GetState(i)->GetName());
			pClass->SetSetting(BitSet, i);
			ExportStateTransList(pFile, pDoc, BitSet, StateClass+1, StateTransLists, ListIndex, Comment + MoreComment);
		}

		return;
	}

	ASSERT(ListIndex < (1<<GetTotalStateBits()));

	//---	we are at the bottom of the state class tree, now we can cap off our comment which we have been building and export the list
	if (FindStateTransAnim(BitSet, &pDoc->m_AnimTree))
	{
		//---	close off the comment and export it
		Comment = Comment.Left(Comment.GetLength() - 3);
		fprintf( pFile, "\n%s\n", Comment);

		//---	export the list
		StateTransLists[ListIndex].Format("StateTransList_%x", BitSet);
		fprintf( pFile, "s16 *%s = {", StateTransLists[ListIndex]);
		ExportStateTransAnims(pFile, BitSet, &pDoc->m_AnimTree);
		fseek(pFile, -4, SEEK_END);
		fprintf( pFile, "};\n");
	}

	//---	increment the list index
	ListIndex++;
}

void CStateClassList::ExportStateTransAnims(FILE *pFile, int BitSet, CAnimTreeNode *pNode)
{
	if (pNode->m_Type == ANIMTREENODE_TYPE_ANIM)
	{
		CAnimNode *pAnim = (CAnimNode*) pNode;
		if (pAnim->IsTransition() && (pAnim->BS_GetEndStates() == BitSet))
			fprintf( pFile, "ANIM_%s, ", ((CAnimNode*) pNode)->GetName());
	}
	else
	{
		CAnimTreeNode *pNode2 = pNode->GetFirstChild();
		while (pNode2)
		{
			ExportStateTransAnims(pFile, BitSet, pNode2);
			pNode2 = pNode->GetNextChild();
		}
	}
}

int CStateClassList::FindStateTransAnim(int Bitset, CAnimTreeNode *pNode)
{
	if (pNode->m_Type == ANIMTREENODE_TYPE_ANIM)
	{
		CAnimNode *pAnim = (CAnimNode*) pNode;
		if (pAnim->IsTransition() && (pAnim->BS_GetEndStates() == Bitset))
			return TRUE;
	}
	else
	{
		CAnimTreeNode *pNode2 = pNode->GetFirstChild();
		while (pNode2)
		{
			if (FindStateTransAnim(Bitset, pNode2))
				return TRUE;

			pNode2 = pNode->GetNextChild();
		}
	}

	return FALSE;
}

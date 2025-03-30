// StateClass.cpp: implementation of the CStateClass class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "StateClass.h"
#include "StateClassList.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStateClass::CStateClass(class CStateClassList *pParent, CString &Name, int StartBit)
{
	m_Name			= Name;
	m_NumBits		= 0;
	m_pParentList	= pParent;
}

CStateClass::~CStateClass()
{
	MakeEmpty();
}

int CStateClass::GetNumStates()
{
	return m_States.GetCount();
}

int CStateClass::GetNumBits()
{
	return m_NumBits;
}

int CStateClass::GetStartBit()
{
	return m_pParentList->GetStateClassStartBit(this);
}

void CStateClass::AddState(CString &Name, int Index)
{
	CState *pNewState = new CState(Name);
	POSITION pos=NULL;

	if (Index >= 0)
	{
		pos = m_States.FindIndex(Index);
		if (pos)
			m_States.InsertBefore(pos, pNewState);
	}

	if (pos == NULL)
		m_States.AddTail(pNewState);

	//---	count the new number of state bits
	m_NumBits = CountNumBits(GetNumStates());
}

int CStateClass::SetName(CString &Name)
{
	if (m_pParentList->FindStateClass(Name))
		return FALSE;

	int i;
	m_Name = Name;
	m_Name.MakeUpper();
	i = m_Name.FindOneOf(" ");
	while (i >= 0)
	{
		m_Name = m_Name.Left(i) + "_" + m_Name.Right(m_Name.GetLength() - i - 1);
		i = m_Name.FindOneOf(" ");
	}

	return TRUE;
}

void CStateClass::RemoveState(int Index)
{
	POSITION pos;
	CState *pState;

	pos = m_States.FindIndex(Index);
	if (pos)
	{
		pState = m_States.GetAt(pos);
		delete pState;
		m_States.RemoveAt(pos);
	}

	//---	count the new number of state bits
	m_NumBits = CountNumBits(GetNumStates());
}

void CStateClass::RemoveState(CString &Name)
{
	CState *pState;
	POSITION pos;
	int i;

	for (i=0; i<GetNumStates(); i++)
	{
		pos = m_States.FindIndex(i);
		if (pos)
		{
			pState = m_States.GetAt(pos);
			if (pState->GetName() == Name)
			{
				m_States.RemoveAt(pos);
				delete pState;

				//---	count the new number of state bits
				m_NumBits = CountNumBits(GetNumStates());

				return;
			}
		}
	}
}

void CStateClass::RemoveState(CState *pState)
{
	CState *pFoundState;
	POSITION pos;
	int i;

	for (i=0; i<GetNumStates(); i++)
	{
		pos = m_States.FindIndex(i);
		if (pos)
		{
			pFoundState = m_States.GetAt(pos);
			if (pFoundState == pState)
			{
				m_States.RemoveAt(pos);
				delete pFoundState;

				//---	count the new number of state bits
				m_NumBits = CountNumBits(GetNumStates());

				return;
			}
		}
	}
}

CString & CStateClass::GetName()
{
	return m_Name;
}


CStateClass& CStateClass::operator =(CStateClass &Other)
{
	int i;
	CState *pNewState;
	CState *pState;

	//---	remove the old states
	MakeEmpty();	

	//---	add the new states
	for (i=0; i<Other.GetNumStates(); i++)
	{
		pState = Other.GetState(i);
		if (pState)
		{
			pNewState = new CState;
			*pNewState = *pState;
			m_States.AddTail(pNewState);
		}
	}

	m_Name = Other.GetName();
	m_NumBits = Other.GetNumBits();

	return *this;
}

void CStateClass::MakeEmpty()
{
	int i=0;

	for (i=GetNumStates()-1; i>=0; i--)
		RemoveState(i);
}

CState* CStateClass::GetState(int Index)
{
	POSITION pos;

	pos = m_States.FindIndex(Index);

	if (!pos)
		return NULL;

	return m_States.GetAt(pos);
}

int CStateClass::CountNumBits(int Value)
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

CState* CStateClass::FindState(CString &Name, CState *pSkip)
{
	CState *pState;
	POSITION pos;
	int i;

	for (i=0; i<GetNumStates(); i++)
	{
		pos = m_States.FindIndex(i);
		if (pos)
		{
			pState = m_States.GetAt(pos);
			if ((pState != pSkip) && (pState->GetName() == Name))
				return pState;
		}
	}

	return NULL;
}

int CStateClass::GetMask(void)
{
	int bits = GetNumBits();
	int mask;

	if (bits == 0)
		return 0;

	mask = 1;
	do
	{
		mask *= 2;
	}
	while (--bits);

	return (mask - 1) << GetStartBit();
}

int CStateClass::GetSetting(int BitSet)
{
	return ((BitSet & GetMask()) >> GetStartBit());
}

void CStateClass::SetSetting(int &BitSet, int Value)
{
	BitSet &= ~GetMask();
	BitSet |= (Value << GetStartBit()) & GetMask();
}

void CStateClass::Serialize(CArchive &ar)
{
	int version;
	int i;
	int count;

	if (ar.IsStoring())
	{
		version = 0;
		ar << version;

		ar.WriteString( m_Name );
		ar.WriteString( "\xd\xa" );
		ar << m_NumBits;

		//---	serialize the states
		count = GetNumStates();
		ar << count;
		for (i=0; i<count; i++)
			GetState(i)->Serialize(ar);
	}
	else
	{
		ar >> version;

		ar.ReadString( m_Name );
		ar >> m_NumBits;

		//---	laod the states
		ar >> count;
		for (i=0; i<count; i++)
		{
			AddState();
			GetState(i)->Serialize(ar);
		}
	}
}

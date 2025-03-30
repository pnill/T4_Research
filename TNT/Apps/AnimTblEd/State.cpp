// State.cpp: implementation of the CState class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "State.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CState::CState(CString &Name)
{
	SetName(Name);
}

CState::~CState()
{

}

void CState::SetName(CString &Name)
{
	int i;
	m_Name = Name;
	m_Name.MakeUpper();
	i = m_Name.FindOneOf(" ");
	while (i >= 0)
	{
		m_Name = m_Name.Left(i) + "_" + m_Name.Right(m_Name.GetLength() - i - 1);
		i = m_Name.FindOneOf(" ");
	}
}

CString& CState::GetName()
{
	return m_Name;
}

CState& CState::operator =(CState &Other)
{
	this->m_Name = Other.GetName();

	return *this;
}

void CState::Serialize(CArchive &ar)
{
	if (ar.IsStoring())
	{
		ar.WriteString(m_Name);
		ar.WriteString("\xd\xa");
	}
	else
	{
		ar.ReadString(m_Name);
	}
}

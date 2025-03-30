// Flag.cpp: implementation of the CFlag class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "Flag.h"
#include "FlagList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFlag::CFlag(CFlagList *pParent, CString &Name)
{
	m_Name = Name;
	m_pParent = pParent;
}

CFlag::~CFlag()
{

}

CString& CFlag::GetName()
{
	return m_Name;
}

void CFlag::SetName(CString &Name)
{
	m_Name = Name;
}

CFlag& CFlag::operator= (CFlag &Other)
{
	m_Name = Other.GetName();

	return *this;
}

int CFlag::GetBit()
{
	return m_pParent->GetFlagBit(this);
}


int CFlag::GetMask()
{
	return 1 << GetBit();
}

void CFlag::Serialize(CArchive &ar)
{
	int version;

	if (ar.IsStoring())
	{
		version = 1;
		ar << version;

		ar.WriteString(m_Name);
		ar.WriteString("\xd\xa");
	}
	else
	{
		ar >> version;

		ar.ReadString(m_Name);
	}
}

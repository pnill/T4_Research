// AnimID1.cpp: implementation of the CAnimID class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimID1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAnimID::CAnimID()
{
	m_AnimID.Format( "%s", "" );
	m_nOccurances = 0;
}

CAnimID::CAnimID(const char *lpszAnimID)
{
	m_AnimID.Format( "%s", lpszAnimID );
	m_nOccurances = 0;
}

CAnimID::~CAnimID()
{

}

int CAnimID::IncrementUses()
{
	//---	if the animation was not found, its uses count down from zero, otherwise it counts up.
	if (m_nOccurances >= 0)
		m_nOccurances++;
	else
		m_nOccurances--;

   	return m_nOccurances;
}

int CAnimID::DecrementUses()
{
	ASSERT(m_nOccurances != 0);

	if (m_nOccurances > 0)
		--m_nOccurances;
	else
		++m_nOccurances;

	return m_nOccurances;
}


void CAnimID::MarkUnfound()
{
	if (m_nOccurances > 0)
		m_nOccurances = -m_nOccurances;
}

void CAnimID::MarkFound()
{
	if (m_nOccurances < 0)
		m_nOccurances = -m_nOccurances;
}

bool CAnimID::IsFound()
{
	return (m_nOccurances >= 0);
}

int CAnimID::GetUses()
{
	if (m_nOccurances >= 0)
		return m_nOccurances;
	else
		return -m_nOccurances;
}

// LocoTable.cpp: implementation of the CLocoTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "LocoTable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLocoAnimTable::CLocoAnimTable()
{

}

CLocoAnimTable::~CLocoAnimTable()
{
	while( m_ArcDataList.GetCount() )
	{
		delete m_ArcDataList.GetHead();
		m_ArcDataList.RemoveHead();
	}
}

void CLocoAnimTable::Serialize(CArchive& ar)
{
	int version = 1;
	POSITION pos;
	CLocoArcData* pArcData;
	int count;

	if (ar.IsStoring())
	{
		ar << version;

		ar << m_CenterNSR;
		ar << m_NSR;
		ar << m_PhaseShift;

		ar.WriteString( m_Anim );
		ar.WriteString( "\xd\xa" );

		//---	store the ArcData list
		ar << m_ArcDataList.GetCount();
		pos = m_ArcDataList.GetHeadPosition();
		while( pos )
		{
			pArcData = m_ArcDataList.GetNext( pos );
			pArcData->Serialize( ar );
		}
	}
	else
	{
		ar >> version;

		ar >> m_CenterNSR;
		ar >> m_NSR;
		ar >> m_PhaseShift;

		ar.ReadString( m_Anim );

		//---	load the ArcData list
		ar >> count;
		while( count-- )
		{
			pArcData = new CLocoArcData;
			pArcData->Serialize( ar );
			m_ArcDataList.AddTail( pArcData );
		}
	}
}

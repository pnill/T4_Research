// LocoArcData.cpp: implementation of the CLocoArcData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "LocoArcData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLocoArcData::CLocoArcData()
{
	m_Angle = 2*3.14159265358979f;

	CLocoAnimNSR* pAnimNSR = new CLocoAnimNSR;
	m_AnimNSRList.AddHead( pAnimNSR );
}

CLocoArcData::~CLocoArcData()
{
	EmptyList();
}

void CLocoArcData::Serialize(CArchive& ar)
{
	int version = 1;
	POSITION pos;
	CLocoAnimNSR* pAnimNSR;
	int count;

	if (ar.IsStoring())
	{
		ar << version;

		ar << m_Angle;

		//---	store the Anim/NSR list
		ar << m_AnimNSRList.GetCount();
		pos = m_AnimNSRList.GetHeadPosition();
		while( pos )
		{
			pAnimNSR = m_AnimNSRList.GetNext( pos );
			pAnimNSR->Serialize( ar );
		}
	}
	else
	{
		ar >> version;

		ar >> m_Angle;

		//---	empty the old list
		EmptyList();

		//---	load the Anim/NSR list
		ar >> count;
		while( count-- )
		{
			pAnimNSR = new CLocoAnimNSR;
			pAnimNSR->Serialize( ar );
			m_AnimNSRList.AddTail( pAnimNSR );
		}
	}
}

CLocoArcData& CLocoArcData::operator =(CLocoArcData &Other)
{
	CLocoAnimNSR* pNSR;
	CLocoAnimNSR* pNewNSR;
	POSITION pos;

	//---	copy member variables
	m_Angle = Other.m_Angle;

	//---	copy the list
	EmptyList();
	pos = Other.m_AnimNSRList.GetHeadPosition();
	while( pos )
	{
		pNSR = Other.m_AnimNSRList.GetNext( pos );
		pNewNSR = new CLocoAnimNSR;
		*pNewNSR = *pNSR;
		m_AnimNSRList.AddTail( pNewNSR );
	}

	return *this;
}

int CLocoArcData::operator ==(CLocoArcData &Other)
{
	POSITION pos, pos2;

	//---	check member variables
	if (m_Angle != Other.m_Angle) return FALSE;

	//---	check the list
	EmptyList();
	pos = m_AnimNSRList.GetHeadPosition();
	pos2 = Other.m_AnimNSRList.GetHeadPosition();
	while( pos && pos2 )
		if (*m_AnimNSRList.GetNext( pos ) != *Other.m_AnimNSRList.GetNext( pos2 )) return FALSE;

	if (pos != pos2)
		return FALSE;

	return TRUE;
}

void CLocoArcData::EmptyList()
{
	while( m_AnimNSRList.GetCount() )
	{
		delete m_AnimNSRList.GetHead();
		m_AnimNSRList.RemoveHead();
	}
}

void CLocoArcData::Export( FILE* pFile, class CAnimTblEdDoc* pDoc, int Phase, CString& rParentName, int Index, CStringArray& rAnimationList, float CenterNSR, float OverlapNSR )
{
	POSITION pos;
	CString Name;
	float Overlap;
	
	if (Phase == 0)	return;

	if (m_AnimNSRList.GetCount() == 0) return;
	Name = GetExportName( rParentName, Index );

	if (Phase == 1)
	{
		//---	print the header information
		fprintf( pFile, "\n//---   Locomotion Table Arc Data %s - Anim/NSR range data\n", Name );
		fprintf( pFile, "t_SLOCTBL_AnimNSR	%s_AnimNSRData[%d]=\n{\n", Name, m_AnimNSRList.GetCount() );

		//---	add the children to the list
		pos = m_AnimNSRList.GetHeadPosition();
		int i=0;
		float LastNSR = CenterNSR;
		while( pos )
		{
			if ((( i == 0 ) && ( OverlapNSR < 0.0f )) ||
				(( i == (m_AnimNSRList.GetCount() - 1 )) && ( OverlapNSR > 0.0f )))
				Overlap = 0.0f;
			else
				Overlap = OverlapNSR;

			m_AnimNSRList.GetNext( pos )->Export( pFile, pDoc, Name, i++, rAnimationList, LastNSR, Overlap );
		}
		fprintf( pFile, "};\n");

		return;
	}

	//---	print the header information
	fprintf( pFile, "\t/* Arc %2d */ { ", Index );
	fprintf( pFile, "%6.4ff, ", m_Angle );
	fprintf( pFile, "%2d, ", m_AnimNSRList.GetCount() );
	fprintf( pFile, "%s_AnimNSRData },", Name );
	fprintf( pFile, "\t// {Angle, NumZones, ZoneList}\n" );
}

void CLocoArcData::RemoveAnim( int Index )
{
	POSITION pos = m_AnimNSRList.GetHeadPosition();
	while( pos ) m_AnimNSRList.GetNext( pos )->RemoveAnim( Index );
}

void CLocoArcData::InsertAnim( int Index )
{
	POSITION pos = m_AnimNSRList.GetHeadPosition();
	while( pos ) m_AnimNSRList.GetNext( pos )->InsertAnim( Index );
}

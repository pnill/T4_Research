// LocomotionTables.cpp: implementation of the CLocomotionTables class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "LocomotionTables.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLocomotionTables::CLocomotionTables()
{

}

CLocomotionTables::~CLocomotionTables()
{
	while( m_TableList.GetCount() )
	{
		delete m_TableList.GetHead();
		m_TableList.RemoveHead();
	}
}

void CLocomotionTables::Serialize(CArchive &ar)
{
	int version = 1;
	POSITION pos;
	CLocoAnimTable* pLocoTable;
	int count;

	if (ar.IsStoring())
	{
		ar << version;

		//---	store the ArcData list
		ar << m_TableList.GetCount();
		pos = m_TableList.GetHeadPosition();
		while( pos )
		{
			pLocoTable = m_TableList.GetNext( pos );
			pLocoTable->Serialize( ar );
		}
	}
	else
	{
		ar >> version;

		//---	load the ArcData list
		ar >> count;
		while( count-- )
		{
			pLocoTable = new CLocoAnimTable;
			pLocoTable->Serialize( ar );
			m_TableList.AddTail( pLocoTable );
		}
	}
}

void CLocomotionTables::Export( FILE* pFile, class CAnimTblEdDoc* pDoc, bool DefinesOnly )
{
	POSITION pos;

	//---	if we are only exporting defines, do it here
	if (DefinesOnly)
	{
		fprintf( pFile, "\n" );
		fprintf( pFile, "\n" );
		fprintf( pFile, "//===   Locomotion Animation Tables Enumeration\n" );
		fprintf( pFile, "\n" );

		//---	create defines for accessing the tables
		fprintf( pFile, "enum\n{\n" );
		pos = m_TableList.GetHeadPosition();
		while( pos )
		{
			CLocoAnimTable* pLocTable = m_TableList.GetNext( pos );
			if( pLocTable->m_bDoNotExport )
				fprintf( pFile, "//" );
			fprintf( pFile, "\t%s,\n", pLocTable->GetExportDefine() );
		}
		fprintf( pFile, "\n\tNUM_LOCOANIMTABLES\n};\n" );

		return;
	}

	if (m_TableList.GetCount() == 0) return;

	//---	export the children first
	pos = m_TableList.GetHeadPosition();
	while( pos )
		m_TableList.GetNext( pos )->Export( pFile, pDoc, 1 );

	//---	print the header information
	fprintf( pFile, "\n" );
	fprintf( pFile, "t_SLOCTBL_Table   LocoAnimationTables[NUM_LOCOANIMTABLES]=\n{\t// {Type, Flags, NumArcs, CenterAnim, ReorientAnim, PhaseShift, CenterNSR, MaxNSR, fpCallback, ArcDataList}\n" );

	//---	add the children to the list
	pos = m_TableList.GetHeadPosition();
	while( pos )
		m_TableList.GetNext( pos )->Export( pFile, pDoc, 2 );
	fprintf( pFile, "\n};\n");
/*
	//---	export this structure
	fprintf( pFile, "\n" );
	fprintf( pFile, "t_SLOCTBL_AnimTables   LocoAnimTables=\n" );
	fprintf( pFile, "{\n" );
	fprintf( pFile, "\tNUM_LOCOANIMTABLES,\t// NumAnimTables\n" );
	fprintf( pFile, "\tLocoAnimationTables\t\t// pAnimTable\n" );
	fprintf( pFile, "};\n" );*/
}

void CLocomotionTables::EmptyList()
{
	while( m_TableList.GetCount() )
	{
		delete m_TableList.GetHead();
		m_TableList.RemoveHead();
	}
}

CLocoAnimTable* CLocomotionTables::GetTable( int Index )
{
	POSITION pos = m_TableList.FindIndex( Index );

	if (pos == NULL) return NULL;
	return m_TableList.GetAt( pos );
}

CLocoAnimTable* CLocomotionTables::GetTable( CString Name )
{
	POSITION pos = m_TableList.GetHeadPosition();

	while( pos )
	{
		if( m_TableList.GetAt( pos )->m_Name == Name )
			break;

		m_TableList.GetNext( pos );
	}

	if (pos == NULL)
		return NULL;

	return m_TableList.GetAt( pos );
}

void CLocomotionTables::Delete(int Index)
{
	POSITION pos = m_TableList.FindIndex( Index );
	if (pos == NULL) return;

	delete m_TableList.GetAt( pos );
	m_TableList.RemoveAt( pos );
}

void CLocomotionTables::Delete(CString Name)
{
	POSITION pos = m_TableList.GetHeadPosition();

	while( pos )
	{
		if( m_TableList.GetAt( pos )->m_Name == Name )
			break;

		m_TableList.GetNext( pos );
	}

	if (pos)
	{
		delete m_TableList.GetAt( pos );
		m_TableList.RemoveAt( pos );
	}
}

void CLocomotionTables::AddTable(CLocoAnimTable *pTable)
{
	ASSERT(pTable);
	this->m_TableList.AddTail( pTable );
}

CLocomotionTables& CLocomotionTables::operator =(CLocomotionTables &Other)
{
	CLocoAnimTable* pAnimTable;
	CLocoAnimTable* pNewAnimTable;
	POSITION pos;

	//---	copy the list
	EmptyList();
	pos = Other.m_TableList.GetHeadPosition();
	while( pos )
	{
		pAnimTable = Other.m_TableList.GetNext( pos );
		pNewAnimTable = new CLocoAnimTable;
		*pNewAnimTable = *pAnimTable;
		m_TableList.AddTail( pNewAnimTable );
	}

	return *this;
}

void CLocomotionTables::FillCallbackStringArray( CStringArray& rStringArray )
{
	POSITION pos;

	rStringArray.RemoveAll();
	pos = m_TableList.GetHeadPosition();
	while( pos )
		m_TableList.GetNext( pos )->FillCallbackStringArray( rStringArray );
}

void CLocomotionTables::RecordPointers( void )
{
	POSITION pos;
	CLocoAnimTable* pTable;

	pos = m_TableList.GetHeadPosition();
	while( pos )
	{
		pTable = m_TableList.GetNext( pos );
		pTable->m_pOldSelf = pTable;
	}
}

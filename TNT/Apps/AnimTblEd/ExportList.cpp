// ExportList.cpp: implementation of the CExportList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "ExportList.h"

#include "ExportDef.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExportList::CExportList()
{
	m_nExportDefs = 0;
}

CExportList::~CExportList()
{

}

int CExportList::Add(CExportDef *pExport)
{
	m_ExportList.AddTail( pExport );
	m_nExportDefs++;

	return m_nExportDefs;
}

void CExportList::Remove(int nIndex)
{
	CExportDef	*pExportDef;
	POSITION	ExportListPos;

	ExportListPos = m_ExportList.GetHeadPosition();
	for (int i = 0; i < nIndex; i++)
		m_ExportList.GetNext( ExportListPos );

	pExportDef = m_ExportList.GetAt( ExportListPos );
	m_ExportList.RemoveAt( ExportListPos );

	m_nExportDefs--;

	delete pExportDef;
}

void CExportList::Delete()
{
	if ( !m_ExportList.IsEmpty() )
		m_ExportList.RemoveAll();
}

CExportDef * CExportList::GetFirst()
{
	if (m_ExportList.IsEmpty())
		return NULL;

	m_ExportListPos = m_ExportList.GetHeadPosition();

	return m_ExportList.GetAt( m_ExportListPos );
}

CExportDef * CExportList::GetNext()
{
	if (m_ExportListPos)
	{
		m_ExportList.GetNext( m_ExportListPos );

		if (m_ExportListPos)
			return m_ExportList.GetAt( m_ExportListPos );
	}

	return NULL;
}

CExportDef * CExportList::GetNth(int n)
{
	CExportDef *pExportDef;

	pExportDef = this->GetFirst();
	for (; n > 0; n-- )
	{
		pExportDef = this->GetNext();
		if (pExportDef == NULL)
			break;
	}

	return pExportDef;
}

void CExportList::Remove(CExportDef *pExportDef)
{
	int index;

	index = this->GetIndex( pExportDef );

	if (index != -1)
		this->Remove( index );
}

int CExportList::GetIndex(CExportDef *pExportDef)
{
	CExportDef	*pFind;
	int n;
	
	pFind = this->GetFirst();
	for (n = 0; pFind && (pFind != pExportDef); n++)
		pFind = this->GetNext();

	if (pFind)
		return n;
	
	return -1;
}

int CExportList::GetCount()
{
	return m_nExportDefs;
}

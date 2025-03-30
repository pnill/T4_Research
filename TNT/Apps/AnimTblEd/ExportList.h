// ExportList.h: interface for the CExportList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXPORTLIST_H__6919B421_EDC8_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_EXPORTLIST_H__6919B421_EDC8_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

#include "exportdef.h"

class CExportList  
{
public:
	int GetCount();
	int GetIndex(CExportDef *pExportDef);
	void Remove(CExportDef *pExportDef);
	CExportDef * GetNth(int n);
	CExportDef * GetNext();
	CExportDef * GetFirst();
	int m_nExportDefs;
	POSITION m_ExportListPos;
	void Delete();
	void Remove(int nIndex);
	int Add(class CExportDef *pExport);
	CTypedPtrList<CObList, class CExportDef*> m_ExportList;
	CExportList();
	virtual ~CExportList();

};

#endif // !defined(AFX_EXPORTLIST_H__6919B421_EDC8_11D2_9248_00105A29F8F3__INCLUDED_)

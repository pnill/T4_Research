// ExportDef.h: interface for the CExportDef class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXPORTDEF_H__6919B420_EDC8_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_EXPORTDEF_H__6919B420_EDC8_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define EXPORT_FLAG_ENABLE	(1<<0)

class CExportDef : public CObject  
{
private:
	CString m_Path;
	CString m_Name;
	int m_Flags;
//	int m_Enabled;
	int m_ExportDataSeparately;

public:
	int ExportDataSeparately();
	void ExportDataSeparately( int NewValue );
	CString GetDataPath();
	int IsValid();
	int Edit();
	void Copy(CExportDef &ExportDefCopy);
	void Serialize(CArchive &ar);
	int GetFlags();
	int SetFlags(int NewFlags);
	int SetEnabled(int Enable);
	int GetEnabled();
	void SetPath(const char *lpszPath);
	CString& GetPath();
	void SetName(const char *lpszName);
	CString& GetName();
	CExportDef();
	virtual ~CExportDef();

};

#endif // !defined(AFX_EXPORTDEF_H__6919B420_EDC8_11D2_9248_00105A29F8F3__INCLUDED_)

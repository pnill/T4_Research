// AnimTblEdDoc.h : interface of the CAnimTblEdDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMTBLEDDOC_H__CE6EAFED_57C5_11D2_ABF7_00A024569875__INCLUDED_)
#define AFX_ANIMTBLEDDOC_H__CE6EAFED_57C5_11D2_ABF7_00A024569875__INCLUDED_

#include "ExportList.h"	// Added by ClassView
#include "UpdateView.h"	// Added by ClassView
#include "ProjFile.h"
#include <afxtempl.h>
#include "AnimTreeNode.h"
#include "AnimNode.h"
#include "GroupNode.h"
#include "EquivNode.h"
#include "MetricsDlg.h"

#include "AnimTree.h"
#include "UnAnimDlg.h"	// Added by ClassView
#include "AnimID1.h"
#include "IDList.h"

#include "StateClassList.h"
#include "Method.h"
#include "Priorities.h"
#include "FlagList.h"
#include "LocomotionTables.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define	MAX_NUM_CUTTABLES		255
#define	MAX_NUM_ANIMCALLBACKS	255


class CAnimTblEdDoc : public CDocument
{
public:
	//---	Source File List
	CTypedPtrList<CObList, CProjFile*> m_ProjFileList;
	CGroupNode m_AnimTree;
	CIDList m_IDList;

protected: // create from serialization only
	CAnimTblEdDoc();
	DECLARE_DYNCREATE(CAnimTblEdDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimTblEdDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	void MessageBoxPrimaryView(CString Message, CString Caption, UINT nType = MB_OK);
	CAnimNode* FindAnimByName( CString& Name, CAnimTreeNode *pGroupNode=(CAnimTreeNode*)0xffffffff );
	void ExportData( FILE* pFile );
	void ExportCallbackFunctionTables( FILE* pFile );
	void ExportCallbackFunctionPrototypes( FILE* pFile );
	void CallExport();
	int SizeOfAnimTableEntry();
	int GetExtraInfoIndexExportSize();
	int GetTransAnimIndexExportSize();
	int GetBitFieldExportSize();
	void FillAnimComboBox(CComboBox* pBox, CAnimTreeNode *pNode=(CAnimTreeNode*)0xffffffff);
	void DoExport(FILE *pFile, FILE *pDataFile, CString &ExportPath, CString &ExportName);
	int MoveAnimTreeNode(CAnimTreeNode *pItem, CAnimTreeNode *pTarget, int InsertFlags);
	void ListIDsFromTree(CAnimTreeNode *pGroupNode);
	void ReadProjFiles(CArchive &ar);
	void WriteProjFiles(CArchive &ar);
	void BuildIDList();
	void RemoveAllSourceProjFiles();
	void FillCallbackStringArrays( void );
	CProjFile * AddSourceProjFile(const char *lpszFileName);
    void ScanSourceCode(CAnimTreeNode *pItem);

	DWORD MakeTimeStamp( CTime &rTime );
	BOOL CheckFileChange( void );

	virtual ~CAnimTblEdDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	float m_BaseAnimRate;
	CMethods m_Methods;
	CPriorities m_Priorities;
	CStateClassList m_StateClassList;
	CFlagList m_FlagList;
	CIDList m_AnimList;
	CLocomotionTables m_LocomotionTables;
	CStringArray m_CallbackFunctions;

	CUpdateView m_Update;

	CString m_ExportName[3];
	CString m_ExportPath[3];
	int	m_ExportFlags[3];
	int m_nProjFiles;
	CExportList m_ExportList;
	CStringArray	m_CallbackStrings;				// only used during export to store the condensed (non-duplicate) list of callback functions
	CStringArray	m_SelectCallbackStrings;		// only used during export to store the condensed (non-duplicate) list of callback functions
	CStringArray	m_LocomotionCallbackStrings;	// only used during export to store the condensed (non-duplicate) list of callback functions
	int m_AllCaps;
	int m_MaxBitsInBitfield;

	unsigned int m_TimeStamp;		// time stamp of the loaded file
	bool m_bOldFileVersion;			// used to allow the CAnimTreeNode's to start saving out data since they were not set up to originally

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAnimTblEdDoc)
	afx_msg void OnDialogUnassigned();
	afx_msg void OnUpdateDialogUnassigned(CCmdUI* pCmdUI);
	afx_msg void OnProjectExport();
	afx_msg void OnProjectSettings();
	afx_msg void OnProjectMetrics();
	afx_msg void OnProjectExportproperties();
	afx_msg void OnProjectLocomotiontables();
	afx_msg void OnEditSearch();
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintSetup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMTBLEDDOC_H__CE6EAFED_57C5_11D2_ABF7_00A024569875__INCLUDED_)

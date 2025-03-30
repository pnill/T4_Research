#if !defined(AFX_SCANSOURCECODEDLG_H__A095D50A_3F2E_4558_906A_33BB413CA1FB__INCLUDED_)
#define AFX_SCANSOURCECODEDLG_H__A095D50A_3F2E_4558_906A_33BB413CA1FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScanSourceCodeDlg.h : header file
//

#pragma warning( disable: 4786 ) // Warning: identifier truncated to 255 characters in the debug information

#include "AnimTreeNode.h"
#include <list>
#include <map>

/////////////////////////////////////////////////////////////////////////////
// CScanSourceCodeDlg dialog

class CScanSourceCodeDlg : public CDialog
{
// Construction
public:
	CScanSourceCodeDlg(CWnd* pParent = NULL);   // standard constructor
    ~CScanSourceCodeDlg();

// Dialog Data
	//{{AFX_DATA(CScanSourceCodeDlg)
	enum { IDD = IDD_SCANSOURCECODE };
	BOOL	m_ScanRecursive;
	BOOL	m_IgnoreComments;
	BOOL	m_NodeIsAnim;
	CString	m_AnimName;
	BOOL	m_NodeIsGroup;
	BOOL	m_NodeRecurse;
	//}}AFX_DATA

    // Types
    typedef std::list< CString > StringList;
    typedef std::list< int     > IntList;
    typedef std::map<CString, IntList> LineNumberMap;   //Key = filename, T = linked list of line numbers keyword occurs within file
    typedef std::map<CString, LineNumberMap> KeywordMap;//Key = keyword,  T = LineNumberMap

    struct SearchResults
    {
        KeywordMap  FoundKeywords; //Keywords that were found including file and line numbers
        StringList  KeywordList; //Keywords that were searched for
        StringList  FileList; //Files that were searched
        UINT        LinesSearched;
    };

    // Accessors
    BOOL                    IsInitialized() const;
    const SearchResults&    GetSearchResults() const;

    // Mutators
    void                    Initialize(CAnimTreeNode *node);
    void                    ClearSearchResults();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScanSourceCodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

    void LoadSettings();
    void SaveSettings();

    BOOL AddFolder(const CString&);
    BOOL RemoveFolder(const CString&);
    void ResetFolderListBox();

    void BuildFileList();
    CString ExtractKeyword(const CString &) const;
    void AddKeyword(CAnimTreeNode *node);
    void BuildKeywordList();
    void SearchForKeywords();
    void DisplaySearchResults();

    CListBox* GetFolderListBox();

    //Parser helpers for loading .ini file
    void ParseError(CString&);
    BOOL ParseKeyword(CString&);
    BOOL GetKeyword(CString&);
    BOOL GetInteger(int&);
    BOOL GetString(CString&);

    StringList      m_FolderList;
    CAnimTreeNode  *m_Node;

    SearchResults   m_SearchResults;

	// Generated message map functions
	//{{AFX_MSG(CScanSourceCodeDlg)
	afx_msg void OnScanAddfolder();
	afx_msg void OnScanRemovefolder();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline BOOL CScanSourceCodeDlg::IsInitialized() const
{
    return m_Node != NULL;
};

inline const CScanSourceCodeDlg::SearchResults& CScanSourceCodeDlg::GetSearchResults() const
{
    return m_SearchResults;
}

inline void CScanSourceCodeDlg::ClearSearchResults()
{
    m_SearchResults.FileList.clear();
    m_SearchResults.KeywordList.clear();
    m_SearchResults.FoundKeywords.clear();
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCANSOURCECODEDLG_H__A095D50A_3F2E_4558_906A_33BB413CA1FB__INCLUDED_)

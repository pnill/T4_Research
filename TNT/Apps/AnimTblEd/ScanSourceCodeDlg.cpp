// ScanSourceCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "ScanSourceCodeDlg.h"
#include "AnimTblEdDefs.h"
#include <algorithm>
#include "SourceParser.h"
#include "SourceScanResults.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScanSourceCodeDlg dialog


CScanSourceCodeDlg::CScanSourceCodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanSourceCodeDlg::IDD, pParent)
    , m_Node( NULL )
{
	//{{AFX_DATA_INIT(CScanSourceCodeDlg)
	m_ScanRecursive = FALSE;
	m_IgnoreComments = FALSE;
	m_NodeIsAnim = FALSE;
	m_AnimName = _T("");
	m_NodeIsGroup = FALSE;
	m_NodeRecurse = FALSE;
	//}}AFX_DATA_INIT
}


void CScanSourceCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScanSourceCodeDlg)
	DDX_Check(pDX, IDC_SCAN_RECURSIVE, m_ScanRecursive);
	DDX_Check(pDX, IDC_SCAN_COMMENTS, m_IgnoreComments);
	DDX_Check(pDX, IDC_NODE_IS_ANIM, m_NodeIsAnim);
	DDX_Text(pDX, IDC_ANIM_NAME, m_AnimName);
	DDX_Check(pDX, IDC_NODE_IS_GROUP, m_NodeIsGroup);
	DDX_Check(pDX, IDC_SCAN_NODERECURSE, m_NodeRecurse);
	//}}AFX_DATA_MAP
}

CScanSourceCodeDlg::~CScanSourceCodeDlg()
{
}

BEGIN_MESSAGE_MAP(CScanSourceCodeDlg, CDialog)
	//{{AFX_MSG_MAP(CScanSourceCodeDlg)
	ON_BN_CLICKED(IDC_SCAN_ADDFOLDER, OnScanAddfolder)
	ON_BN_CLICKED(IDC_SCAN_REMOVEFOLDER, OnScanRemovefolder)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CScanSourceCodeDlg::Initialize(CAnimTreeNode *node)
{
    ASSERT( node );

    m_Node = node;

    m_Node->AttachScanSourceDlg( this );

    LoadSettings();

    ASSERT( IsInitialized() );
}

void CScanSourceCodeDlg::ParseError(CString& msg)
{
    char buffer[MAX_PATH*2];

    sprintf(buffer, "%s\n\n%s\nLine %d", (const char*)msg, ParserGetFileName(), ParserGetLineNumber());
    
    MessageBox(buffer, "Syntax Error");
}

BOOL CScanSourceCodeDlg::ParseKeyword(CString& keyword)
{
    CString iExp("Integer Expected");
    CString qsExp("Quoted String Expected");

    if (keyword == "RECURSE_DIR")
    {
        if (!GetInteger( m_ScanRecursive ))
            ParseError(iExp);

        if (m_ScanRecursive) m_ScanRecursive = TRUE;
    }
    else if (keyword == "RECURSE_NODE")
    {
        if (!GetInteger( m_NodeRecurse ))
            ParseError(iExp);

        if (m_NodeRecurse) m_NodeRecurse = TRUE;
    }
    else if (keyword == "IGNORE_COMMENTS")
    {
        if (!GetInteger( m_IgnoreComments ))
            ParseError(iExp);

        if (m_IgnoreComments) m_IgnoreComments = TRUE;
    }
    else if (keyword == "FOLDER")
    {
        CString folder;
        if (!GetString(folder))
            ParseError(qsExp);

        AddFolder(folder);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}         

void CScanSourceCodeDlg::SaveSettings()
{
    FILE *fout;

    fout = fopen("ScanSource.ini", "w");
    ASSERT( fout );
    
    fprintf(fout, "/////////////////////////////////////////\n");
    fprintf(fout, "// CScanSourceCodeDlg Settings File\n");
    fprintf(fout, "//\n\n");
    fprintf(fout, "RECURSE_DIR     = %d  //Do you search recursively within each folder?\n", m_ScanRecursive);
    fprintf(fout, "RECURSE_NODE    = %d  //Do you search for all child animations as well?\n", m_NodeRecurse);
    fprintf(fout, "IGNORE_COMMENTS = %d  //Do you ignore animations found within comments?\n", m_IgnoreComments);
    fprintf(fout, "\n");
    fprintf(fout, "/////////////////////////////////////////\n");
    fprintf(fout, "// Folders to search -- Must be quoted strings.\n");
    fprintf(fout, "//\n\n");


    StringList::iterator it;

    for( it = m_FolderList.begin(); it != m_FolderList.end(); ++it )
    {
        fprintf(fout, "FOLDER = \"%s\"\n", (*it));
    }

    fclose( fout );
}

void CScanSourceCodeDlg::LoadSettings()
{
    FILE *fin;
    fin = fopen("ScanSource.ini", "r");

    if (!fin)
        return;

    ParserPrepareForReading( fin, "ScanSource.ini");
    EParserToken tokDesired, tokRead;
    UValue val;
    CString str;

    BOOL eofFound( FALSE );
    
    while (!eofFound)
    {
        tokDesired = TOKEN_WILDCARD; //Match the next token no matter what it is
        tokRead = ParserGrabToken(&tokDesired, &val);

        ASSERT( tokDesired != TOKEN_MISMATCH );
        ASSERT( tokRead == tokDesired );

        switch (tokDesired)
        {
            case (TOKEN_EOF):
                eofFound = TRUE;
                break;
            
            case (TOKEN_KEYWORD):
                str = val.sValue;
                if (!ParseKeyword( str ))
                {
                    str = "Unknown keyword: ";
                    str += val.sValue;
                    ParseError(str);
                }
                break;
            
            case (TOKEN_COMMENT_C_STYLE):
            case (TOKEN_COMMENT_CPP_STYLE):
            case (TOKEN_OPEN_CURLY):
            case (TOKEN_CLOSED_CURLY):
                break; //Ignore this things

            default:
                str = "Keyword Expected But Not Found.";
                ParseError(str);
        };
    }

    ParserFinishReading( fin );
    fclose( fin );
}

BOOL CScanSourceCodeDlg::GetKeyword(CString& str)
{
    EParserToken tok;
    UValue val;

    tok = TOKEN_KEYWORD;
    ParserGrabToken(&tok, &val);

    if (tok == TOKEN_MISMATCH)
    {
        return FALSE;
    }

    str = val.sValue;

    return TRUE;
}

BOOL CScanSourceCodeDlg::GetInteger(int& i)
{
    EParserToken tok;
    UValue val;

    tok = TOKEN_INTEGER;
    ParserGrabToken(&tok, &val);

    if (tok == TOKEN_MISMATCH)
    {
        return FALSE;
    }

    i = val.iValue;

    return TRUE;
}

BOOL CScanSourceCodeDlg::GetString(CString& str)
{
    EParserToken tok;
    UValue val;

    tok = TOKEN_STRING;
    ParserGrabToken(&tok, &val);

    if (tok == TOKEN_MISMATCH)
    {
        return FALSE;
    }

    str = val.sValue;
    str.Remove('"');

    return TRUE;
}

BOOL CScanSourceCodeDlg::AddFolder(const CString& str)
{
    StringList::iterator it;

    it = std::find(m_FolderList.begin(), m_FolderList.end(), str);

    if (it == m_FolderList.end())
    {
        //Folder not in list already
        //TODO: make sure it is a valid folder name

        m_FolderList.push_back(str);
        return TRUE;
    }

    return FALSE;
}

BOOL CScanSourceCodeDlg::RemoveFolder(const CString& str)
{
    StringList::iterator it;

    it = std::find(m_FolderList.begin(), m_FolderList.end(), str);

    if (it != m_FolderList.end())
    {
        //Folder found
        m_FolderList.erase(it);
        return TRUE;
    }

    return FALSE;
}

void CScanSourceCodeDlg::ResetFolderListBox()
{
    CListBox *lBox = GetFolderListBox();

    lBox->ResetContent();

    StringList::iterator it;

    for( it = m_FolderList.begin(); it != m_FolderList.end(); ++it )
    {
        lBox->AddString(*it);
    }
}


BOOL CScanSourceCodeDlg::OnInitDialog()
{
    ASSERT( IsInitialized() ); //Initialize() must be called prior to invoking the dialog

    m_NodeIsAnim    = FALSE;
    m_NodeIsGroup   = FALSE;

    switch (m_Node->m_Type)
    {
    case ANIMTREENODE_TYPE_GROUP:
        m_NodeIsGroup = TRUE;
        break;

    case ANIMTREENODE_TYPE_ANIM:
        m_NodeIsAnim = TRUE;
        break;
    };

    m_AnimName = m_Node->m_VPString;

    ResetFolderListBox();

    return CDialog::OnInitDialog();
};

void CScanSourceCodeDlg::OnOK()
{
    CWaitCursor wait;

    UpdateData();

    SaveSettings();
    BuildFileList();
    BuildKeywordList();
    SearchForKeywords();
    DisplaySearchResults();

    //CDialog::OnOK();
}

void CScanSourceCodeDlg::OnCancel()
{
    CDialog::OnCancel();
    
    DestroyWindow();
};

CListBox* CScanSourceCodeDlg::GetFolderListBox()
{
    CListBox *lBox = (CListBox*)GetDlgItem(IDC_SCAN_FOLDERS);
    ASSERT( lBox );

    return lBox;
}

void CScanSourceCodeDlg::BuildFileList()
{
    StringList DirectoryList( m_FolderList );
    const char* filters[] = {"*.cpp", "*.c", "*.hpp", "*.h", 0};

    StringList::iterator it;
    HANDLE hFind;
    WIN32_FIND_DATA ffd;
    CString fileFilter;
    CString tempString;
    BOOL success;

    //Alias m_FileList
    StringList &m_FileList = m_SearchResults.FileList;

    m_FileList.clear();

    for (it=DirectoryList.begin(); it!=DirectoryList.end(); ++it)
    {
        int filterIndex = 0;

        while (filters[filterIndex])
        {
            fileFilter = *it + "\\" + filters[filterIndex];
            
            hFind = FindFirstFile( fileFilter, &ffd );
            while (hFind != INVALID_HANDLE_VALUE)
            {
                tempString = *it + "\\" + ffd.cFileName;
                m_FileList.push_back( tempString );
                success = FindNextFile( hFind, &ffd );
                if (!success) break;
            }
            FindClose(hFind);
            
            ++filterIndex;
        }

        if (m_ScanRecursive)
        {
            fileFilter = *it + "\\*.";
            hFind = FindFirstFile( fileFilter, &ffd );
            while (hFind != INVALID_HANDLE_VALUE)
            {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    StringList::iterator it2;
                    CString newDirectory( ffd.cFileName );

                    if (newDirectory != "." && newDirectory != "..")
                    {
                        newDirectory = *it + "\\" + newDirectory;

                        it2 = std::find(DirectoryList.begin(), DirectoryList.end(), newDirectory);
                        if (it2 == DirectoryList.end())
                        {
                            DirectoryList.push_back( newDirectory );
                        }
                    }
                }
                success = FindNextFile( hFind, &ffd );
                if (!success) break;
            }
            FindClose(hFind);
        }
    }
}

CString CScanSourceCodeDlg::ExtractKeyword(const CString &string) const
{
    return string.SpanExcluding(" \t,;:.-");
}

void CScanSourceCodeDlg::AddKeyword(CAnimTreeNode *node)
{
    CString tempString;
    
    switch (node->m_Type)
    {
    case ANIMTREENODE_TYPE_ANIM:
        tempString = "ANIM_" + node->m_VPString;
        m_SearchResults.KeywordList.push_back( ExtractKeyword(tempString) );
        break;

    case ANIMTREENODE_TYPE_GROUP:
        tempString = "ANIMGROUP_" + node->m_VPString;
        m_SearchResults.KeywordList.push_back( ExtractKeyword(tempString) );
        break;
    };
}

void CScanSourceCodeDlg::BuildKeywordList()
{
    typedef std::list< CAnimTreeNode* > NodeList;

    NodeList nList;

    nList.push_back(m_Node);

    m_SearchResults.KeywordList.clear();

    NodeList::iterator it;

    for (it=nList.begin(); it!=nList.end(); ++it)
    {
        CAnimTreeNode *node = *it;
        
        if (!node->GetIgnore())
        {
            AddKeyword( node );
        
            if (m_NodeRecurse)
            {
                CAnimTreeNode *parent =  node;
                CAnimTreeNode *child;
                
                for (child = parent->GetFirstChild(); child != NULL; child = parent->GetNextChild())
                {
                    nList.push_back( child );
                }
            }        
        }    
    }
}

void CScanSourceCodeDlg::SearchForKeywords()
{
    StringList::iterator fileIt;
    StringList::iterator keyIt;
    int searchComments = (m_IgnoreComments ? 0 : 1);

    StringList &m_KeywordList = m_SearchResults.KeywordList;
    StringList &m_FileList = m_SearchResults.FileList;
    KeywordMap &foundKeywords = m_SearchResults.FoundKeywords;

    foundKeywords.clear();
    m_SearchResults.LinesSearched = 0;

    int numKeywords = m_KeywordList.size();

    const int MAX_KEYWORD_LENGTH = 64;
    //typedef char[MAX_KEYWORD_LENGTH] KeywordBuffer;

    const char** keywords = new const char*[ numKeywords ];
    ASSERT( keywords );

    int index = 0;
    for (keyIt = m_KeywordList.begin(); keyIt != m_KeywordList.end(); ++keyIt, ++index)
    {
        keywords[index] = (const char*)(*keyIt);
    }

    for (fileIt = m_FileList.begin(); fileIt != m_FileList.end(); ++fileIt)
    {
#ifdef _DEBUG
        CString File = *fileIt;
#endif
        FILE *fin;
        
        fin = fopen( *fileIt, "r" );
        if (!fin)
        {
            char buffer[MAX_PATH + 64];
            sprintf(buffer, "Warning: Couldn't open file (\"%s\").", *fileIt);
            MessageBox(buffer, "Warning", MB_OK);
            continue;
        }
        
        ParserPrepareForReading(fin, *fileIt);
        
        while ((index = ParserFindKeywords(keywords, numKeywords, searchComments)) >= 0)
        {
            LineNumberMap &lMap = foundKeywords[ keywords[index] ];
            IntList &iList = lMap[ *fileIt ];
            iList.push_back( ParserGetLineNumber() );
        }
        
        m_SearchResults.LinesSearched += ParserGetLineNumber();

        ParserFinishReading( fin );
        fclose( fin );
    }

    delete [] keywords;
}

void CScanSourceCodeDlg::DisplaySearchResults()
{
    CSourceScanResults *resDlg = new CSourceScanResults(this);
    ASSERT( resDlg );

    resDlg->Initialize( this );
    resDlg->Create(CSourceScanResults::IDD, this);
    resDlg->ShowWindow(SW_SHOW);
}

/////////////////////////////////////////////////////////////////////////////
// CScanSourceCodeDlg message handlers

void CScanSourceCodeDlg::OnScanAddfolder() 
{
    char filename[MAX_PATH];
    BROWSEINFO lpb;
    LPITEMIDLIST lpItem;
    memset(&lpb, 0, sizeof( lpb ));

    lpb.pszDisplayName = filename;
    lpItem = SHBrowseForFolder( &lpb );

    if (lpItem)
    {
        HRESULT result;
        LPMALLOC pMalloc;
        
        result = SHGetMalloc( &pMalloc );
        ASSERT( result == NOERROR );

        SHGetPathFromIDList(lpItem, filename);
        pMalloc->Free( lpItem );

        CString newFolder(filename);

        if (newFolder.IsEmpty())
            return;

        if (AddFolder(newFolder))
        {
            GetFolderListBox()->AddString(newFolder);
        }
    }
}

void CScanSourceCodeDlg::OnScanRemovefolder() 
{
    CListBox *lBox = GetFolderListBox();
    int sel;

    sel = lBox->GetCurSel();

    if (sel == LB_ERR)
        return;

    CString folder;
    lBox->GetText(sel, folder);

    if (RemoveFolder(folder))
    {
        lBox->DeleteString(sel);
        lBox->SetCurSel(sel);
    }
}

void CScanSourceCodeDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if (m_Node)
    {
        m_Node->DetachScanSourceDlg();	
    }
}

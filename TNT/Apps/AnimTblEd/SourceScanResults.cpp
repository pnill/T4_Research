// SourceScanResults.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "SourceScanResults.h"
#include "ScanSourceCodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSourceScanResults dialog


CSourceScanResults::CSourceScanResults(CWnd* pParent /*=NULL*/)
	: CDialog(CSourceScanResults::IDD, pParent)
    , m_ScanDlg( NULL )
{
	//{{AFX_DATA_INIT(CSourceScanResults)
	m_NumFiles = 0;
	m_NumKeywords = 0;
	m_NumLOC = 0;
	m_NumKeywordsFound = 0;
	//}}AFX_DATA_INIT
}


void CSourceScanResults::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSourceScanResults)
	DDX_Text(pDX, IDC_NUMFILES, m_NumFiles);
	DDX_Text(pDX, IDC_NUMKEYWORDS, m_NumKeywords);
	DDX_Text(pDX, IDC_NUMLINES, m_NumLOC);
	DDX_Text(pDX, IDC_NUMKEYWORDSFOUND, m_NumKeywordsFound);
	//}}AFX_DATA_MAP
}

void CSourceScanResults::Initialize(const CScanSourceCodeDlg *scanDlg)
{
    ASSERT( scanDlg );

    m_ScanDlg = scanDlg;
}

BOOL CSourceScanResults::OnInitDialog()
{
    CListBox *lBox = (CListBox*)GetDlgItem(IDC_SCANRESULTS_LB);
    CListBox *lBox2= (CListBox*)GetDlgItem(IDC_NOMATCH);
    ASSERT( lBox && lBox2 && m_ScanDlg );

    char buffer[MAX_PATH*2];

    const CScanSourceCodeDlg::SearchResults &searchResults = m_ScanDlg->GetSearchResults();
    const CScanSourceCodeDlg::KeywordMap &foundKeywords = searchResults.FoundKeywords;
    const CScanSourceCodeDlg::StringList &keywords = searchResults.KeywordList;

    m_NumFiles = searchResults.FileList.size();
    m_NumLOC = searchResults.LinesSearched;
    m_NumKeywords = keywords.size();
    m_NumKeywordsFound = foundKeywords.size();


    ///Populate Listbox for Matches Found
    CScanSourceCodeDlg::KeywordMap::const_iterator resIt;

    for( resIt = foundKeywords.begin(); resIt!= foundKeywords.end(); ++resIt)
    {
        const CScanSourceCodeDlg::LineNumberMap &lMap = (*resIt).second;
        CScanSourceCodeDlg::LineNumberMap::const_iterator lIt;
        for (lIt = lMap.begin(); lIt != lMap.end(); ++lIt)
        {
            const CScanSourceCodeDlg::IntList &iList = (*lIt).second;
            CScanSourceCodeDlg::IntList::const_iterator it;

            for (it=iList.begin(); it != iList.end(); ++it)
            {
                sprintf(buffer, "%s    %s    Line %d", (*resIt).first, (*lIt).first, (*it));
                lBox->AddString(buffer);
            }
        }
    }

    //Populate Listbox for no matches

    CScanSourceCodeDlg::StringList::const_iterator keyIt;

    for ( keyIt = keywords.begin(); keyIt != keywords.end(); ++keyIt )
    {
        resIt = foundKeywords.find( *keyIt );

        if (resIt == foundKeywords.end())
        {
            lBox2->AddString( *keyIt );
        }
    }

    return CDialog::OnInitDialog();
}

void CSourceScanResults::OnOK()
{
    CDialog::OnOK();

    DestroyWindow();
}

BEGIN_MESSAGE_MAP(CSourceScanResults, CDialog)
	//{{AFX_MSG_MAP(CSourceScanResults)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CSourceScanResults::OnDestroy() 
{
	CDialog::OnDestroy();
	
	delete this;
}


void CSourceScanResults::OnClose() 
{
	CDialog::OnClose();
    
    DestroyWindow();
}

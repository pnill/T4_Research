// SourceFilesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "SourceFilesDlg.h"
#include "AnimTblEdGlobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSourceFilesDlg dialog


CSourceFilesDlg::CSourceFilesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSourceFilesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSourceFilesDlg)
	//}}AFX_DATA_INIT
}


void CSourceFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSourceFilesDlg)
	DDX_Control(pDX, IDC_LIST, m_FileList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSourceFilesDlg, CDialog)
	//{{AFX_MSG_MAP(CSourceFilesDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSourceFilesDlg message handlers

BOOL CSourceFilesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	//---	Clear the m_Modified flag.
	m_Modified = FALSE;

	//---	Add the Files to the List Box.
	while (!m_FilesString.IsEmpty())
	{
		CString FileName = m_FilesString.SpanExcluding("|");
		if (!FileName.IsEmpty())
		{
			m_FileList.AddString(FileName);
			m_FilesString.Delete(0, FileName.GetLength()+1);
		}
		m_FilesString.TrimLeft();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSourceFilesDlg::OnAdd() 
{
	CString			sFilter = "C Header Files (*.h)|*.h|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog		FileDlg( TRUE, "*.h", NULL, 0, sFilter, this );

	FileDlg.m_ofn.Flags |= OFN_ALLOWMULTISELECT;
	if (FileDlg.DoModal() == IDOK)
	{
		POSITION	FilePos = FileDlg.GetStartPosition();
		CString		RootPath;
		CString		FilePath;

		while (FilePos)
		{
			FilePath = FileDlg.GetNextPathName( FilePos );

			//---	If the file does not already exist in the list then add it.
			if (!FileInList( FilePath ))
			{
				m_FileList.AddString( FilePath );
				m_Modified = TRUE;
			}
			else
				MessageBox( "The file '" + FilePath + "' is already a project source file and will not be added again.", "WARNING", MB_ICONWARNING );
		}
	}
}

void CSourceFilesDlg::OnDelete() 
{
	int	nSelected, i;
	int	SelectedArray[256];

	nSelected = m_FileList.GetSelItems( 256, &SelectedArray[0] );
	for (i = nSelected-1; i >= 0; i--)
	{
		m_FileList.DeleteString(SelectedArray[i]);		
		m_Modified = TRUE;
	}
}

void CSourceFilesDlg::OnOK() 
{
	CString	FileName;
//	CString	TempString;
	int		nFiles;
	int		nBytes;

	//---	Passing data into and out of our list box via a string w/ files separated by a space character.
	m_FilesString = "";
	if (m_Modified)
	{
		nFiles = m_FileList.GetCount();
		for (int i = 0; i < nFiles; i++)
		{
			nBytes = m_FileList.GetTextLen( i );
			m_FileList.GetText( i, FileName.GetBuffer( nBytes ) );
			FileName.ReleaseBuffer();
			m_FilesString += FileName + "|";
		}
	}

	//---	Mark the document as having changed.
	GBL_SetModifiedFlag( TRUE );
	
	CDialog::OnOK();
}

bool CSourceFilesDlg::FileInList(const char *lpszFile)
{
	return (m_FileList.FindStringExact( 0, lpszFile ) != LB_ERR);
}

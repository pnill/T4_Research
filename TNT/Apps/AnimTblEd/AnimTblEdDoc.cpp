// AnimTblEdDoc.cpp : implementation of the CAnimTblEdDoc class
//

//#include "afxdlgs.h"
#include "stdafx.h"
#include "AnimTblEd.h"

#include "ProjFile.h"

#include "AnimTblEdDoc.h"

#include "AnimTblEdDefs.h"

#include "ExportDlg.h"
#include "ProjectSettings.h"
#include "Method.h"
#include "LocomotionTablesDlg.h"
#include "ScanSourceCodeDlg.h"

#include "AnimationEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CAnimTblEdDoc *GBL_pGlobalDoc;

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdDoc

IMPLEMENT_DYNCREATE(CAnimTblEdDoc, CDocument)

BEGIN_MESSAGE_MAP(CAnimTblEdDoc, CDocument)
	//{{AFX_MSG_MAP(CAnimTblEdDoc)
	ON_COMMAND(ID_DIALOG_UNASSIGNED, OnDialogUnassigned)
	ON_UPDATE_COMMAND_UI(ID_DIALOG_UNASSIGNED, OnUpdateDialogUnassigned)
	ON_COMMAND(ID_PROJECT_EXPORT, OnProjectExport)
	ON_COMMAND(ID_PROJECT_SETTINGS, OnProjectSettings)
	ON_COMMAND(ID_PROJECT_METRICS, OnProjectMetrics)
	ON_COMMAND(ID_PROJECT_EXPORTPROPERTIES, OnProjectExportproperties)
	ON_COMMAND(ID_PROJECT_LOCOMOTIONTABLES, OnProjectLocomotiontables)
	ON_COMMAND(ID_EDIT_SEARCH, OnEditSearch)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_SETUP, OnFilePrintSetup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdDoc construction/destruction

CAnimTblEdDoc::CAnimTblEdDoc()
{
	GBL_pGlobalDoc = this;
	m_BaseAnimRate = 0.06f;
	m_MaxBitsInBitfield = 32;
	m_TimeStamp = 0;
	m_bOldFileVersion = FALSE;
}

CAnimTblEdDoc::~CAnimTblEdDoc()
{
	//---	Make sure to get free allocated space for the project .h files.
	RemoveAllSourceProjFiles();

	//---	Free space for anim IDs.
	m_IDList.Delete();
	m_AnimList.Delete();

	//---	Free space for the export definitions.
	m_ExportList.Delete();

	//---	Free space for the animation table tree.
	m_AnimTree.Remove( m_IDList );
}

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdDoc serialization

void CAnimTblEdDoc::Serialize(CArchive& ar)
{
	int version;
	int	nExports;
	CExportDef *pExportDef;
	int i;
	CString str;

	if (ar.IsStoring())
	{
		version = 112;

		ar << version;
		WriteProjFiles( ar );

		//---	Write export information. Hard set to 2 for now.
		nExports = this->m_ExportList.GetCount();
		ar << nExports;

		//---	Write individual export definitions..
		pExportDef = this->m_ExportList.GetFirst();
		while (pExportDef)
		{
			pExportDef->Serialize( ar );

			pExportDef = this->m_ExportList.GetNext();
		}

		//---	BaseAnimRate
		ar << this->m_BaseAnimRate;
		ar << m_MaxBitsInBitfield;

		m_IDList.Serialize( ar );

		//---	serialize the main view only (for now)
		ar << 1L; // number of views
		POSITION pos = this->GetFirstViewPosition();
		if( pos != 0 )
			this->GetNextView( pos )->Serialize( ar );

		//---	serialize the callback functions
		int NumCallbacks = m_CallbackFunctions.GetSize();
		ar << NumCallbacks;
		for( i=0; i<NumCallbacks; i++ )
		{
			str = m_CallbackFunctions[i];
			ar.WriteString( str );
			ar.WriteString( "\xd\xa" );
		}
	}
	else
	{
		m_bOldFileVersion = TRUE;
		version;

		ar >> version;

		if( version >= 112 )
			m_bOldFileVersion = FALSE;

		if (version < 103)
		{
			ReadProjFiles( ar );
			ar.ReadString( m_ExportPath[0] );
			m_ExportPath[1].Format( "" );
			m_ExportPath[2].Format( "" );

			if (version >= 102)
				m_IDList.Serialize( ar );
		}
		else if (version >= 105)
		{
			ReadProjFiles( ar );

			//---	Read export information.
			ar >> nExports;
			while (nExports)
			{
				pExportDef = new CExportDef;

				pExportDef->Serialize( ar );

				m_ExportList.Add( pExportDef );

				nExports--;
			}

			if (version >= 104)
				ar >> m_BaseAnimRate;

			if (version >= 111)
				ar >> m_MaxBitsInBitfield;

			if (version >= 102)
				m_IDList.Serialize( ar );

			if (version >= 106)
			{
				int Count;
				ar >> Count; // assumed to be one at this point
				POSITION pos = this->GetFirstViewPosition();
				if( pos != 0 )
					this->GetNextView( pos )->Serialize( ar );
			}

			//--- verions 107, 108 stuff is below

			if (version >= 109)
			{
				//---	serialize the callback functions
				int NumCallbacks;
				ar >> NumCallbacks;
				for( i=0; i<NumCallbacks; i++ )
				{
					ar.ReadString( str );
					m_CallbackFunctions.Add( str );
				}
			}
		}
		else
		{
			int		Number;
			CString String;


			ReadProjFiles( ar );

			//---	Read export information. Hard set to 2 for now
			ar >> nExports;

			while(nExports)
			{
				pExportDef = new CExportDef;

				ar >> Number;
				pExportDef->SetFlags( Number );

				ar.ReadString( String );
				pExportDef->SetName( String );

				ar.ReadString( String );
				pExportDef->SetPath( String );

				m_ExportList.Add( pExportDef );

				nExports--;
			}

			if (version >= 104)
				ar >> m_BaseAnimRate;

			if (version >= 102)
				m_IDList.Serialize( ar );
		}
	}

	//---	must load the lomocotion tables before the tree so that the tree can relink to it
	if (version >= 110)
		m_LocomotionTables.Serialize(ar);

	m_AnimTree.Serialize( ar );

	//---	serialize the state/flags/priority bitset.
	if ((version >= 108) && (version <= 109))
		m_LocomotionTables.Serialize(ar);

	if (version >= 107)
		m_Methods.Serialize(ar);

	if (version > 100)
		m_StateClassList.Serialize(ar);

	if (version > 100)
		m_FlagList.Serialize(ar);

	if (version > 100)
		m_Priorities.Serialize(ar);

	if (ar.IsLoading())
		BuildIDList();

	//---	When saving mark the document as saved.
	if (ar.IsStoring())
		this->SetModifiedFlag( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdDoc diagnostics

#ifdef _DEBUG
void CAnimTblEdDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAnimTblEdDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdDoc commands

CProjFile * CAnimTblEdDoc::AddSourceProjFile(const char *lpszFileName)
{
	CProjFile	*pNewProjFile = new CProjFile(lpszFileName);

//	pNewProjFile->Open();

	m_nProjFiles++;

	m_ProjFileList.AddTail(pNewProjFile);

	return pNewProjFile;
}

void CAnimTblEdDoc::RemoveAllSourceProjFiles()
{
	CProjFile	*pProjFile;

	while (!m_ProjFileList.IsEmpty())
	{
		pProjFile = m_ProjFileList.RemoveTail();
		if( pProjFile->IsOpen() )
			pProjFile->Close();
		delete pProjFile;
	}
	m_nProjFiles = 0;
}

void CAnimTblEdDoc::BuildIDList()
{
	POSITION	ProjFilePos;
	CProjFile	*pProjFile;
	char		IDBuff[SIZE_ID];
	CIDList		TempIDList;
	CAnimID		*pAnimID;
	CAnimID		*pFoundID;
	int			IDType;

	//---	Start with an empty list.
	m_IDList.Clear();

	ProjFilePos = m_ProjFileList.GetHeadPosition();
	while (ProjFilePos)
	{
		//---	Get a pointer to the CProjFile.
		pProjFile = m_ProjFileList.GetNext( ProjFilePos );
		pProjFile->Open();

		if (pProjFile->IsOpen())
		{
			//---	Reset the file pointer to the top of the list.
			pProjFile->Reset();

			//---	find the first animation
			do
			{
				IDType = pProjFile->GetNextIdentifier(IDBuff, SIZE_ID - 1);
			}
			while(( IDType != IDTYPE_ANIM ) && ( IDType != IDTYPE_EOF ));

			//---	get all of the animation IDs from the file
			if ( IDType == IDTYPE_ANIM )
			{
				do
				{
					TempIDList.Add( IDBuff );

					IDType = pProjFile->GetNextIdentifier(IDBuff, SIZE_ID - 1);
				}
				while ( IDType == IDTYPE_ANIM );
			}

			pProjFile->Close();
		}

		//---	Mark the file as processed
		pProjFile->SetProcessed( TRUE );
	}

	//---	Add items currently in the tree to the ID list.
	CAnimTblEdDoc::ListIDsFromTree( &m_AnimTree );

	//---	Check items in the ID list to the items in the source files.
	pAnimID = m_IDList.GetHead() ;
	while (pAnimID)
	{
		pFoundID = TempIDList.Remove( pAnimID->m_AnimID );

		if (pFoundID)
			delete pFoundID;		// free the memory
		else
			pAnimID->MarkUnfound();	// Mark the item as not being in the list.

		pAnimID = m_IDList.GetNext();
	}

	//---	Add extra elements from the source files into the ID list.
	pAnimID = TempIDList.GetHead();
	while (pAnimID)
	{
		m_IDList.Add( pAnimID->m_AnimID );

		pAnimID = TempIDList.GetNext();
	}

	TempIDList.Delete();
}

void CAnimTblEdDoc::OnDialogUnassigned() 
{
//	m_UnassignedAnimsDlg.m_Visible = !m_UnassignedAnimsDlg.m_Visible;

//	if (m_UnassignedAnimsDlg.m_Visible)
//		m_UnassignedAnimsDlg.ShowWindow( SW_SHOWNA );
//	else
//		m_UnassignedAnimsDlg.ShowWindow( SW_HIDE );
}

void CAnimTblEdDoc::OnUpdateDialogUnassigned(CCmdUI* pCmdUI) 
{
//	pCmdUI->SetCheck( m_UnassignedAnimsDlg.m_Visible );
}

void CAnimTblEdDoc::WriteProjFiles(CArchive &ar)
{
	POSITION listPos;
	CProjFile *pProjFile;

	ar << m_nProjFiles;
	
	listPos = m_ProjFileList.GetHeadPosition();
	while (listPos)
	{
		 pProjFile = m_ProjFileList.GetNext( listPos );
		 pProjFile->Serialize( ar );
	}
}

void CAnimTblEdDoc::ReadProjFiles(CArchive &ar)
{
	CProjFile *pProjFile;
	int	n;
	CString	string;

	ar >> m_nProjFiles;
	n = m_nProjFiles;
	
	while (n--)
	{
		pProjFile = new CProjFile();
		pProjFile->Serialize( ar );
		m_ProjFileList.AddTail( pProjFile );
	}
}

CAnimNode* CAnimTblEdDoc::FindAnimByName( CString& Name, CAnimTreeNode *pGroupNode )
{
	CAnimNode* pRet=NULL;

	if( pGroupNode == (CAnimTreeNode*)0xffffffff )
	{
		if( Name.GetLength() == 0 )
			return NULL;

		return FindAnimByName( Name, &m_AnimTree );
	}

	CAnimTreeNode	*pAnimTreeNode;
	pGroupNode->PushChildStep();
	pAnimTreeNode = pGroupNode->GetFirstChild();

	while( pAnimTreeNode && (pRet==NULL))
	{
//		if( !pAnimTreeNode->GetIgnore() )
		{
			//---	Record the IDs used in each equivalency in the m_IDList.
			if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ANIM)
			{
				if( ((CAnimNode*)pAnimTreeNode)->GetName() == Name )
					pRet = (CAnimNode*)pAnimTreeNode;
			}
			else
				pRet = (CAnimNode*)FindAnimByName( Name, (CGroupNode*)pAnimTreeNode );
		}

		pAnimTreeNode = pGroupNode->GetNextChild();
	}
	pGroupNode->PopChildStep();

	return pRet;
}

void CAnimTblEdDoc::ListIDsFromTree(CAnimTreeNode *pGroupNode)
{
	CAnimTreeNode	*pAnimTreeNode;

	pAnimTreeNode = pGroupNode->GetFirstChild();	
	
	while (pAnimTreeNode)
	{
		if( !pAnimTreeNode->GetIgnore() )
		{
			//---	Record the IDs used in each equivalency in the m_IDList.
			if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ANIM)
			{
				CAnimID	*pAnimID;

				pAnimID = m_IDList.Add( ((CAnimNode*)pAnimTreeNode)->m_RLIDs.GetLeftID() );
				if (pAnimID)
					pAnimID->IncrementUses();

				pAnimID = m_IDList.Add( ((CAnimNode*)pAnimTreeNode)->m_RLIDs.GetRightID() );
				if (pAnimID)
					pAnimID->IncrementUses();

				//---	Also record the Animation IDs in the m_AnimList.
				m_AnimList.AddInc( pAnimTreeNode->GetName() );
			}
			else
				ListIDsFromTree( (CGroupNode*)pAnimTreeNode );
		}

		pAnimTreeNode = pGroupNode->GetNextChild();
	}
}

void CAnimTblEdDoc::OnProjectExport()
{
	CExportDef	*pExportDef;
	FILE		*pFile;
	FILE		*pDataFile;

	pExportDef = this->m_ExportList.GetFirst();

	while (pExportDef)
	{
		//---	Export the first file.
		if (pExportDef->IsValid())
		{
			//---	make a temporary path name to export to
			CString Path = pExportDef->GetPath();
			s32 bs	= Path.ReverseFind( '\\' );
			s32 s	= Path.ReverseFind( '/' );
			s32 c	= Path.ReverseFind( ':' );
			s32 i	= (bs > s) ? bs : s;
				i	= (i > c) ? i : c;
			if( i >= 0 )
				Path = Path.Left( i+1 );
			else
				Path = "";
			CString NewFile = Path + "atb.tmp";

			//---	export the header file to the temporary file
			pFile = fopen( NewFile, "wt" );
			if ( !pFile )
			{
				CString		String;
				POSITION	ViewPos;
				String.Format( "Could not open file for export:%s", pExportDef->GetPath() );
				ViewPos = this->GetFirstViewPosition();
				this->GetNextView( ViewPos )->MessageBox( String, NULL, MB_ICONERROR );
			}

			if( pExportDef->ExportDataSeparately() )
			{
				pDataFile = fopen( pExportDef->GetDataPath(), "wt" );
				if( !pDataFile )
				{
					CString		String;
					POSITION	ViewPos;
					String.Format( "Could not open file for export:%s", pExportDef->GetDataPath() );
					ViewPos = this->GetFirstViewPosition();
					this->GetNextView( ViewPos )->MessageBox( String, NULL, MB_ICONERROR );
				}
			}
			else
				pDataFile = pFile;

			if( pFile && pDataFile )
				this->DoExport( pFile, pDataFile, pExportDef->GetPath(), pExportDef->GetName() );

			if( pFile )
				fclose( pFile );

			if(( pDataFile != pFile ) && pDataFile )
				fclose( pDataFile );

			//---	determine if the two header files are different.  If they are, clobber the old file with the temp file.
			bool bFilesDiffer = TRUE;
			FILE* pNewFile = fopen( NewFile, "rb" );
			fseek( pNewFile, 0, SEEK_END );
			s32 NewFileSize = ftell( pFile );
			u8* pNewData = (u8*) malloc( NewFileSize );
			fread( pNewData, NewFileSize, 1, pNewFile );
			fclose( pNewFile );

			FILE* pOldFile = fopen( pExportDef->GetPath(), "rb" );
			if( pOldFile )
			{
				fseek( pOldFile, 0, SEEK_END );
				s32 OldFileSize = ftell( pOldFile );
				if( OldFileSize )
				{
					u8* pOldData = (u8*) malloc( OldFileSize );
					fread( pOldData, OldFileSize, 1, pOldFile );

					if( NewFileSize == OldFileSize )
					{
						if( memcmp( pNewData, pOldData, NewFileSize ) == 0 )
							bFilesDiffer = FALSE;
					}

					free( pOldData );
				}

				fclose( pOldFile );
			}

			//---	delete the old one and rename the new one to the correct value
			if( bFilesDiffer )
			{
				CFileStatus s;
				if( CFile::GetStatus( pExportDef->GetPath(), s ))
					CFile::Remove( pExportDef->GetPath() );
				CFile::Rename( NewFile, pExportDef->GetPath() ) ;
			}
			else
				CFile::Remove( NewFile );


			free( pNewData );
		}

		pExportDef = this->m_ExportList.GetNext();
	}
}

void CAnimTblEdDoc::OnProjectExportproperties() 
{
	CString		FileName;

	CExportDlg	ExportDlg;
	
	ExportDlg.m_pDoc = this;		

	ExportDlg.DoModal();
}

int CAnimTblEdDoc::MoveAnimTreeNode(CAnimTreeNode *pItem, CAnimTreeNode *pTarget, int InsertFlags)
{
	CAnimTreeNode	*pParent;
	CAnimTreeNode	*pOldParent;
	int				ParentTypeReq;
	int				ParentAltTypeReq;		// don't like this hack.

	ASSERT(pItem);

	//---	Do not allow for items to be placed within themselves.
	if (pTarget->IsAncestor( pItem ))
		return FALSE;

	//---	Can't just move every type of object.
	if ((pItem->m_Type == ANIMTREENODE_TYPE_PROJECT)
		|| (pItem->m_Type == ANIMTREENODE_TYPE_ENUM)
		|| (pItem->m_Type == ANIMTREENODE_TYPE_UNUSED)
		|| (pItem->m_Type == ANIMTREENODE_TYPE_UNDEF))
	{
		MessageBox( GetDesktopWindow(), "Cannot drag this item.", "Error!", MB_OK );
		return FALSE;
	}

	//---	Determine what type of parent is required.
	switch (pItem->m_Type)
	{
	case ANIMTREENODE_TYPE_TRANS:
		ParentTypeReq = ANIMTREENODE_TYPE_ANIM;
		ParentAltTypeReq = ANIMTREENODE_TYPE_GROUP;
		break;
	default:
		ParentTypeReq = ANIMTREENODE_TYPE_GROUP;
		ParentAltTypeReq = ANIMTREENODE_TYPE_ENUM;		
		break;
	}

	//---	Find the new parent.
	if (InsertFlags & DROPTARGET_INSERTINSIDE)
		pParent = pTarget;
	else
		pParent = pTarget->GetParent();

	while (pParent && (pParent->m_Type != ParentTypeReq) && (pParent->m_Type != ParentAltTypeReq))
	{
		pParent = pParent->GetParent();

		//---	Check for end condition of running out of valid parents.
		if (!pParent
			|| (pParent->m_Type == ANIMTREENODE_TYPE_PROJECT)
			|| (pParent->m_Type == ANIMTREENODE_TYPE_UNUSED)
			|| (pParent->m_Type == ANIMTREENODE_TYPE_UNDEF))
		{
			MessageBox( GetDesktopWindow(), "Cannot drop here.", "Error!", MB_OK );
			return FALSE;
		}
	}

	//---	Mark the document as being changed.
	this->SetModifiedFlag();

	//---	Detatch the node from its parent's child list.
	if (InsertFlags & DROPTARGET_INSERTCOPY)
	{
		pItem = pItem->Clone();
	}
	else
	{
		pOldParent = pItem->GetParent();
		pOldParent->SeverChild( pItem );
	}

	//---	Now insert the item to the new location.
	pParent->AddChild( pItem, pTarget, !(InsertFlags & DROPTARGET_INSERTBEFORE) );

	//---	Update the views.
	if (!(InsertFlags & DROPTARGET_INSERTCOPY))
	{
		m_Update.SetUpdateAction( UPDATEVIEW_ACTION_DELETEITEM, pItem, pOldParent );
		this->UpdateAllViews( NULL );
	}
	m_Update.SetUpdateAction( UPDATEVIEW_ACTION_ADDITEM, pItem, pParent, pParent->GetPrevChild( pItem ) );
	this->UpdateAllViews( NULL );

	return TRUE;
}

void CAnimTblEdDoc::DoExport(FILE *pFile, FILE *pDataFile, CString &ExportPath, CString &ExportName)
{
//	CString	FileName;
	CString	FileDefineString;
	CString Include;
	int NumLocoAnims;

//	FileName.Format( "%s", ExportPath );
//	FileName.Delete( 0, FileName.ReverseFind( '\\' ) + 1 );
//	FileName.Replace( ".", "_" );
	m_AnimTree.CountAllDependancies();

	FileDefineString.Format( "%s_%s", this->GetTitle(), ExportName );
	FileDefineString.Replace( ".", "_" );

	fprintf( pFile, "#ifndef __%s__\n", FileDefineString );
	fprintf( pFile, "#define __%s__\n", FileDefineString );

	fprintf( pFile, "\n\n" );
	Include.Format( "%cAnimTable.h%c", '"', '"' );
	fprintf( pFile, "#include %s // include the structures used in this data", Include );
	fprintf( pFile, "\n\n" );
	fprintf( pFile, "#define\tBASE_ANIM_RATE            %10.8ff\n", m_BaseAnimRate );
	fprintf( pFile, "\n");
	fprintf( pFile, "#define\tANALOG_ANIM_RATE          BASE_ANIM_RATE // this value is unimportant since it will be overwritten with the correct value\n");
	fprintf( pFile, "#define\tPREVIOUS_ANIM_RATE        0.0f           // this value specifies that this animation uses whatever anim rate was previously in the anim rate variable\n");
	fprintf( pFile, "#define\tDESTINATION_ANIM_RATE     100000.0f      // this value specifies that this animation should be played back at the rate of the destination animation\n");
	fprintf( pFile, "#define\tLINK_RATE_TO_ANIM(a)      ((f32) (-(a)))\n");
	fprintf( pFile, "#define\tSET_ANIM_RATE(r)          ((f32) (r) * BASE_ANIM_RATE)\n");

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "// The following structures should be provided in the AnimTable.h file included.\n" );
	fprintf( pFile, "// They are expected to be in the following format...\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "/*" );

	fprintf( pFile, "\n" );
	fprintf( pFile, "typedef struct STransAnim\n" );
	fprintf( pFile, "{\n" );
	fprintf( pFile, "\ts16      DestAnim;		  // an animation which the player may want to go to from this animation, but is not allowed to go to directly, (ANIM_NONE means all/any anims)\n" );
	fprintf( pFile, "\ts16      NextAnim;		  // the animation which the player must go into on his way into the destination animation above. (ANIM_NONE means all/any anims)\n" );
	fprintf( pFile, "} t_STransAnim;\n" );

#if 0
	fprintf( pFile, "\n\n" );
	fprintf( pFile, "typedef struct SExtraAnimInfo\n" );
	fprintf( pFile, "{\n");
	fprintf( pFile, "\tf32      AnimRate;         // playback rate for the animation\n");
	fprintf( pFile, "} t_SExtraAnimInfo;\n" );
#endif

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "typedef struct SLR\n" );
	fprintf( pFile, "{\n" );
	fprintf( pFile, "\tu16      Left;             // Animation used when the player is in ""left"" mode (ball is in the right hand).\n" );
	fprintf( pFile, "\tu16      Right;            // Animation used when the player is in ""right"" mode (ball is in the right hand).\n" );
	fprintf( pFile, "} t_SLR;\n" );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "typedef union UAnimLR\n" );
	fprintf( pFile, "{\n" );
	fprintf( pFile, "\tt_LR     Anim;              // Individually named Left/RightAnimations.\n" );
	fprintf( pFile, "\tu16      Array[2];          // Array holding the Left/Right animations.\n" );      
	fprintf( pFile, "} t_UAnimLR;\n" );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "typedef struct SAnimTable\n" );
	fprintf( pFile, "{\n" );
	fprintf( pFile, "\tt_AnimLR     LR;             // The Left and Right version of the animations.\n" );
	fprintf( pFile, "\tu32          BitField;       // holds the Priority/Flags/States for the animation.\n" );//, GetBitFieldExportSize());
	fprintf( pFile, "\tu16          TransAnim;      // pointer to the list of transitions and dependancies\n" );//, GetTransAnimIndexExportSize() );
	fprintf( pFile, "\tf32          FrameRate;      // pointer to a structure of extra animation information which might be needed for some anims.\n"/*, GetExtraInfoIndexExportSize() */);
	fprintf( pFile, "\tu16          Callback;       // index into the array of callback function pointers\n" );
	fprintf( pFile, "\tu16          CutTable;       // ID of the cut table associated with this animation (0xffffffff means none)\n" );
	fprintf( pFile, "} t_SAnimTable;\n\n" );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "typedef struct SAnimTableExport\n" );
	fprintf( pFile, "{" );
	fprintf( pFile, "\tf32                  BaseAnimRate;           // the base rate for all animations\n" );
	fprintf( pFile, "\ts32                  AnimTableLength;        // number of animation table entries\n" );
	fprintf( pFile, "\ts32					LocoAnimTableLength;    // number of locomotion animation tables\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tu32                  MethodMask;             // mask for the Mode of the animation\n" );
	fprintf( pFile, "\tu32                  MethodShift;            // shift for the Mode of the animation\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tu32                  FlagMask;               // mask for the Flag of the animation\n" );
	fprintf( pFile, "\tu32                  FlagShift;              // shift for the Flag of the animation\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tu32                  StateMask;              // mask for the state value of the aniamtion\n" );
	fprintf( pFile, "\tu32                  StateShift;             // shift for the state value of the animation\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tu32                  PriorityMask;           // mask for the priority value of the aniamtion\n" );
	fprintf( pFile, "\tu32                  PriorityShift;          // shift for the priority value of the animation\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tt_SAnimTable*        pAnimTable;             // animation table\n" );
	fprintf( pFile, "\ts16*                 pAnimTransTable;        // transition table\n" );
	fprintf( pFile, "\ts16*                 pStateTransTable;       // state transition table\n" );
	fprintf( pFile, "\tf32*                 pLocoAnimRateTable;     // locomotion animation rate table (frames per inch)\n" );
	fprintf( pFile, "\tt_SLOCTBL_AnimTable*	pLocoAnimTables;        // locomotion animation tables\n" );
	fprintf( pFile, "} t_SAnimTableExport;\n\n" );

	fprintf( pFile, "\n*/\n\n" );

	//---	count the locomotion animations
	NumLocoAnims = m_AnimTree.CountLocomotionAnimations();

	fprintf( pFile, "\n\n");
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//   BIT SET DEFINES (Methods, Flags, States, Priorities)" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_StateClassList.ExportDefines( pFile, 0 );
	m_Methods.ExportDefines( pFile, m_StateClassList.GetTotalStateBits() );
	m_FlagList.ExportDefines( pFile, m_StateClassList.GetTotalStateBits() + m_Methods.GetTotalMethodBits() );
	m_Priorities.ExportDefines( pFile, m_StateClassList.GetTotalStateBits() + m_Methods.GetTotalMethodBits() + m_FlagList.GetTotalFlagBits() );

	fprintf( pFile, "\n\n");
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//   CALLBACK FUNCTION PROTOTYPES (must be defined externally)" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	ExportCallbackFunctionPrototypes( pFile );


	fprintf( pFile, "\n\n");
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//   LOCOMOTION ANIMATION TABLE EXPORT" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_LocomotionTables.Export( pFile, this, TRUE );


	fprintf( pFile, "\n\n" );

	m_AnimTree.DoExport( pFile, EXPORTTYPE_ANIMLIST );

	fprintf( pFile, "\n\n" );

	m_AnimTree.DoExport( pFile, EXPORTTYPE_NUMANIMS );

	fprintf( pFile, "\n\n" );

	m_AnimTree.DoExport( pFile, EXPORTTYPE_ANIMLISTGROUP );

	fprintf( pFile, "\n\n");
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  MACROS\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );

	m_StateClassList.ExportMacros( pFile, 0 );
	m_Methods.ExportMacros( pFile, m_StateClassList.GetTotalStateBits() );
	m_FlagList.ExportMacros( pFile, m_StateClassList.GetTotalStateBits() + m_Methods.GetTotalMethodBits() );
	m_Priorities.ExportMacros( pFile, m_StateClassList.GetTotalStateBits() + m_Methods.GetTotalMethodBits() + m_FlagList.GetTotalFlagBits() );

	m_AnimTree.DoExport( pFile, EXPORTTYPE_MACROS );

	fprintf( pFile, "\n\n" );

	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  EXTERNS\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );

	if( pFile == pDataFile )
	{
		fprintf( pFile, "#ifdef	DEFINE_ANIMATION_STORAGE\n" );

		ExportData( pDataFile );

		fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
		fprintf( pFile, "//  DATA\n" );
		fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );

		fprintf( pFile, "\n#else //__DEFINE_ANIMATION_STORAGE__\n" );

		fprintf( pFile, "extern t_SAnimTableExport AnimTableExport;\n\n" );

		fprintf( pFile, "\n\n#endif //DEFINE_ANIMATION_STORAGE\n" );
	}
	else
	{
		fprintf( pFile, "extern t_SAnimTableExport AnimTableExport;\n\n" );

		CString FileDefineString;
		FileDefineString.Format( "%s_%s_DATA_", this->GetTitle(), ExportName );
		FileDefineString.Replace( ".", "_" );

		fprintf( pDataFile, "#ifndef __%s__\n", FileDefineString );
		fprintf( pDataFile, "#define __%s__\n", FileDefineString );
		fprintf( pDataFile, "\n" );

		fprintf( pDataFile, "// NOTE: The exported .H file must be included before this data file.\n" );
		fprintf( pDataFile, "\n" );

		ExportData( pDataFile );

		fprintf( pDataFile, "\n\n#endif //__%s__\n", FileDefineString );
	}

	fprintf( pFile, "\n\n" );

	fprintf( pFile, "\n\n#endif //__%s__\n", FileDefineString );

	fprintf( pFile, "\n\n" );
}

void CAnimTblEdDoc::ExportData( FILE* pFile )
{
	fprintf( pFile, "\n" );

#if 0
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  ANIMATION STRUCTURES\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_AnimTree.DoExport( pFile, EXPORTTYPE_ANIMOBJECTS );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  EXTRA ANIMATION INFORMATION\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_AnimTree.DoExport( pFile, EXPORTTYPE_EXTRAINFO );
#endif

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  ANIMATION DEPENDANCIES\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_AnimTree.DoExport( pFile, EXPORTTYPE_DEPENDANCIES );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  ANIMATION TABLE\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_AnimTree.DoExport( pFile, EXPORTTYPE_OBJECTLIST );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  ANIMATION GROUP TABLE\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_AnimTree.DoExport( pFile, EXPORTTYPE_OBJECTLISTGROUP );

	fprintf( pFile, "\n\n");
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//   CALLBACK FUNCTION ARRAY" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	ExportCallbackFunctionTables( pFile );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  LOCOMOTION ANIMATION RATES TABLE\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "volatile f32 LocoAnimRateTable[] =\n" );
	fprintf( pFile, "{\n" );
	m_AnimTree.DoExport( pFile, EXPORTTYPE_LOCOMOTIONANIMARRAY );
	fprintf( pFile, "};\n" );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  STATE TRANSITION TABLE\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_StateClassList.ExportStateTransTable( pFile, this );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  LOCOMOTION ANIMATION TABLE EXPORT\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	m_LocomotionTables.Export( pFile, this, FALSE );

	fprintf( pFile, "\n\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "//  ANIMATION TABLE EXPORT STRUCTURE (All exported data is accessable through this)\n" );
	fprintf( pFile, "////////////////////////////////////////////////////////////////////////////////////\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "t_SAnimTableExport AnimTableExport =\n" );
	fprintf( pFile, "{\n" );
	fprintf( pFile, "\tBASE_ANIM_RATE,        // the base rate for all animations\n" );
	fprintf( pFile, "\tNUM_ALL_ANIMS,         // number of animation table entries\n" );
	fprintf( pFile, "\tNUM_LOCOANIMTABLES,    // number of locomotion animation tables\n" );
	fprintf( pFile, "\n" );	
	fprintf( pFile, "\tANIMBF_METHODMASK,     // mask for the Method of the animation\n" );
	fprintf( pFile, "\tANIMBF_METHODSHIFT,    // shift for the Method of the animation\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tANIMBF_FLAGSMASK,      // mask for the Flag of the animation\n" );
	fprintf( pFile, "\tANIMBF_FLAGSSHIFT,     // shift for the Flag of the animation\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tANIMBF_STATEMASK,      // mask for the state value of the aniamtion\n" );
	fprintf( pFile, "\tANIMBF_STATESHIFT,     // shift for the state value of the animation\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tANIMBF_PRIORITYMASK,   // mask for the priority value of the aniamtion\n" );
	fprintf( pFile, "\tANIMBF_PRIORITYSHIFT,  // shift for the priority value of the animation\n" );
	fprintf( pFile, "\n" );
	fprintf( pFile, "\tAnimTable,             // animation table\n" );
	fprintf( pFile, "\tAnimGroupTable,        // animation group table\n" );
	fprintf( pFile, "\tAnimTransTable,        // transition table\n" );
	fprintf( pFile, "\tStateTransTable,       // state transition table\n" );
	fprintf( pFile, "\tAnimationCallbacks,    // animation callback functions\n" );
	fprintf( pFile, "\tLocoAnimRateTable,     // locomotion animation rate table table\n" );
	fprintf( pFile, "\tLocoAnimationTables,   // locomotion animation tables\n" );
	fprintf( pFile, "};\n" );
}

void CAnimTblEdDoc::OnProjectSettings() 
{
	CProjectSettings Settings;

	Settings.m_BaseAnimRate = m_BaseAnimRate;
	Settings.m_BitFieldSize = m_MaxBitsInBitfield;

	if (Settings.DoModal() == IDOK)
	{
		m_BaseAnimRate = Settings.m_BaseAnimRate;
		m_MaxBitsInBitfield = Settings.m_BitFieldSize;
	}
}

void CAnimTblEdDoc::FillAnimComboBox(CComboBox *pBox, CAnimTreeNode *pNode)
{
	//---	handle trivial case
	if (pNode == NULL)
		return;

	//---	initialize the loop
	if (pNode == (CAnimTreeNode*) 0xffffffff)
		pNode = &m_AnimTree;

	if (pNode->m_Type == ANIMTREENODE_TYPE_TRANS)
		return;
	else if (pNode->m_Type == ANIMTREENODE_TYPE_ANIM)
		pBox->AddString(((CAnimNode*)pNode)->GetName());
	else 
	{
		pNode->PushChildStep();
		CAnimTreeNode* pChild = pNode->GetFirstChild();
		while (pChild)
		{
			FillAnimComboBox( pBox, pChild );
			pChild = pNode->GetNextChild();
		}
		pNode->PopChildStep();
	}
}

int CAnimTblEdDoc::GetBitFieldExportSize()
{
	int StateBits = this->m_StateClassList.GetTotalStateBits();
	int FlagBits = this->m_FlagList.GetTotalFlagBits();
	int PriorityBits = this->m_Priorities.GetTotalPriorityBits();
	int	TotalBits = StateBits + FlagBits + PriorityBits;

	if (TotalBits <= 8)
		return 8;
	if (TotalBits <= 16)
		return 16;
	if (TotalBits <= 32)
		return 32;
	if (TotalBits <= 64)
		return 64;

	ASSERT(0);
	return 128;
}

int CAnimTblEdDoc::GetTransAnimIndexExportSize()
{
	int NumDependancies = this->m_AnimTree.GetTotalNumDependancies( FALSE );
	int	GetNumAnimsWithDependancies = this->m_AnimTree.GetNumAnimsWithDependancies( FALSE );
	int	TotalIndicies = NumDependancies + GetNumAnimsWithDependancies;

	TotalIndicies += 1;  //0 index is for anims that have no transition.

	if (TotalIndicies < (1<<8))
		return 8;
	if (TotalIndicies < (1<<16))
		return 16;
	if (TotalIndicies < (1<<32))
		return 32;
	if (TotalIndicies < (1<<64))
		return 64;

	ASSERT(0);
	return 128;
}

int CAnimTblEdDoc::GetExtraInfoIndexExportSize()
{
	ASSERT(0);
	return 128;
}

void CAnimTblEdDoc::OnProjectMetrics() 
{
	CMetricsDlg	MetricsDlg;

	MetricsDlg.pDoc = this;
	
	MetricsDlg.DoModal();
}

int CAnimTblEdDoc::SizeOfAnimTableEntry()
{
	int nBytes=0;
	int	nVariable = 0;

#if VARIABLESTRUCTSIZE
	//---	The BitField portion.
	nVariable += this->GetBitFieldExportSize();

	//---	The TransAnimIndexExportSize.
	nVariable += this->GetTransAnimIndexExportSize();

	if (nVariable <= 32)
		nVariable = 32;
	else if (nVariable <= 64)
		nVariable = 64;
	else
		nVariable = 128;

	//---	Set the initial size by the Variable.
	nBytes = nVariable / 8;
#else
	//---	Bit field
	nBytes += 4;

	//---	trans table index
	nBytes += 2;

	//---	callback function
	nBytes += 1;

	//---	cut table index
	nBytes += 1;

	//---	animation link
	nBytes += 2;

	//---	animataion link data
	nBytes += 2;
#endif

	//---	The Left/Right anim portion is 4 bytes.
	nBytes += 4;

	//---	The Extra Info right now consists only of a floating point AnimRate value.
	nBytes += 4;

	return nBytes;
}


void CAnimTblEdDoc::CallExport()
{
	this->OnProjectExport();
}

void CAnimTblEdDoc::OnProjectLocomotiontables() 
{
	CLocomotionTablesDlg LocoDialog( this );
	
	m_LocomotionTables.RecordPointers();
	LocoDialog.m_LocomotionTables = m_LocomotionTables;

	if (LocoDialog.DoModal() == IDOK)
	{
		m_LocomotionTables = LocoDialog.m_LocomotionTables;
		m_AnimTree.SwitchCutTablePointer();
		SetModifiedFlag();
	}
}

void CAnimTblEdDoc::FillCallbackStringArrays( void )
{
	m_CallbackStrings.RemoveAll();
	m_AnimTree.FillCallbackStringArray( m_CallbackStrings );

	if( m_CallbackStrings.GetSize() > MAX_NUM_ANIMCALLBACKS )
	{
		CString str;
		str.Format( "Too many animation callback functions. There are %d, %d is the current limit. Extra ones will not be exported", m_CallbackStrings.GetSize(), MAX_NUM_ANIMCALLBACKS );
		MessageBox( NULL, str, "Export Error", MB_ICONERROR );
		m_CallbackStrings.SetSize( MAX_NUM_ANIMCALLBACKS );
	}

	m_SelectCallbackStrings.RemoveAll();
	m_AnimTree.FillSelectCallbackStringArray( m_SelectCallbackStrings );

	m_LocomotionTables.FillCallbackStringArray( m_LocomotionCallbackStrings );
}

void CAnimTblEdDoc::ExportCallbackFunctionPrototypes( FILE* pFile )
{
	int i;

	FillCallbackStringArrays();

	//---	export the group select callback prototypes
	fprintf( pFile, "//---\tThe group select callback functions\n" );
	for( i=0; i<m_SelectCallbackStrings.GetSize(); i++ )
		fprintf( pFile, "extern s32 GROUPCB_%s( class QActor* This, s32 GroupIndex );\n", m_SelectCallbackStrings[i] );
	fprintf( pFile, "\n" );

	//---	export the animation callback function prototypes
	fprintf( pFile, "//---\tThe animation callback functions\n" );
	for( i=0; i<m_CallbackStrings.GetSize(); i++ )
		fprintf( pFile, "extern s32 ANIMCB_%s( class QActor* This, u32 Flags, u32 Param );\n", m_CallbackStrings[i] );
	fprintf( pFile, "\n" );

	//---	export the locomotion callback function prototypes
	fprintf( pFile, "//---\tThe locomotion table callback functions\n" );
	for( i=0; i<m_LocomotionCallbackStrings.GetSize(); i++ )
		fprintf( pFile, "extern xbool LOCCB_%s( class QActor* This, s32* pAnim, radian* pRelDir, f32* pNSR, f32* pMaxNSR, f32* pMinNSR, s32 ModeFlags );\n", m_LocomotionCallbackStrings[i] );
	fprintf( pFile, "\n" );

}


void CAnimTblEdDoc::ExportCallbackFunctionTables( FILE* pFile )
{
	int i;

	//===	THIS CODE ASSUMES THAT THE STRING ARRAYS HAVE ALREADY BEEN FILLED WITH A CALL TO FillCallbackStringArrays()
	//		probably through a call to ExportCallbackFunctionPrototypes() since the prototypes would exist in the file before these tables.

	//---	group callbacks are not exported in a table because they are directly pointed to by the group data
	//		this may changes some day, but for now we don't need to make a table.

	//---	now lets export the animation callback functions
	fprintf( pFile, "//---\tThe animation callback functions table\n" );
	fprintf( pFile, "t_fpAnimCallback	AnimationCallbacks[] =\n" );
	fprintf( pFile, "{\n" );
	fprintf( pFile, "\tNULL,\n" );
	for( i=0; i<m_CallbackStrings.GetSize(); i++ )
		fprintf( pFile, "\tANIMCB_%s,\n", m_CallbackStrings[i] );
	fprintf( pFile, "};\n" );
	fprintf( pFile, "\n" );

}

///////////////////////////////////////////////////////////////////////////////
BOOL CAnimTblEdDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	m_nProjFiles = 0;

	//---	Export Options
	m_AllCaps = FALSE;

	//---	Create the two necessary groups to start the tree: the primary enum group and the unused group.
	m_AnimTree.SetName( "PROJECT" );
	m_AnimTree.m_Type = ANIMTREENODE_TYPE_PROJECT;

	CGroupNode	*pEnumGroup = new CGroupNode( this, NULL, ANIMTREENODE_TYPE_ENUM, "ENUMERATION" );
//	CGroupNode	*pUnusedGroup = new CGroupNode( this, NULL, ANIMTREENODE_TYPE_UNUSED, "UNUSED" );

	m_AnimTree.AddChild( pEnumGroup );
//	m_AnimTree.AddChild( pUnusedGroup );

#ifndef NO_TIMESTAMP_CHECKING
	m_TimeStamp = 0;
#endif

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CAnimTblEdDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
#ifndef NO_TIMESTAMP_CHECKING
	//---	set the time stamp for this document before it is loaded (this protects even if it is changed while loading)
	CFileStatus s;
	CFile::GetStatus( lpszPathName, s );
	m_TimeStamp = MakeTimeStamp( s.m_mtime );
#endif

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// TODO: Add your specialized creation code here
	m_AnimTree.m_Type = ANIMTREENODE_TYPE_PROJECT;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CAnimTblEdDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
#ifndef NO_TIMESTAMP_CHECKING
	m_TimeStamp	= 0;
#endif

	BOOL ret = CDocument::OnSaveDocument(lpszPathName);
	
#ifndef NO_TIMESTAMP_CHECKING
	//---	get the time stamp of the new file
	CFileStatus s;
	CFile::GetStatus( lpszPathName, s );
	m_TimeStamp = MakeTimeStamp( s.m_mtime );
#endif
	
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
void CAnimTblEdDoc::OnCloseDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	CDocument::OnCloseDocument();
}

///////////////////////////////////////////////////////////////////////////////
//
//	MakeTimeStamp
//
//	stores a 32 bit time stamp of the source file at the time of building
//
DWORD CAnimTblEdDoc::MakeTimeStamp( CTime &rTime )
{
#ifndef NO_TIMESTAMP_CHECKING
	//---	build 32 bit time stamp

	//---	year is condenced to save room.  It is given 6 bits( 64 years ) and starts from 1970.
	DWORD Year			= (DWORD) rTime.GetYear() - 1970;
	DWORD YearBits		= (DWORD) ((((Year < 0) ? 0 : Year)	<< 26) & 0xfc000000);
	DWORD MonthBits		= (DWORD) ((rTime.GetMonth()		<< 22) & 0x03c00000);
	DWORD DayBits		= (DWORD) ((rTime.GetDay()			<< 17) & 0x003e0000);
	DWORD HourBits		= (DWORD) ((rTime.GetHour()			<< 12) & 0x0001f000);
	DWORD MinuteBits	= (DWORD) ((rTime.GetMinute()		<< 6)  & 0x00000fc0);
	DWORD SecondBits	= (DWORD) ((rTime.GetSecond()		<< 0)  & 0x0000003f);

	return YearBits | MonthBits | DayBits | HourBits | MinuteBits | SecondBits;
#else
	return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
BOOL CAnimTblEdDoc::CheckFileChange( void )
{
#ifndef NO_TIMESTAMP_CHECKING
	//---	do nothing until a file is associated with this file
	if( m_TimeStamp == 0 )
		return FALSE;

	//---	get the source file time stamp
	CFileStatus s;
	CFile::GetStatus( GetPathName(), s );
	DWORD TimeStamp = MakeTimeStamp( s.m_mtime );

	//---	compare the source file time stamp to the loaded time stamp
	if( TimeStamp != m_TimeStamp )
	{
		m_TimeStamp = TimeStamp;
		return TRUE;
	}

	return FALSE;
#else
	return 0;
#endif
}

void CAnimTblEdDoc::ScanSourceCode(CAnimTreeNode *pItem)
{
    ASSERT( pItem );

    CScanSourceCodeDlg *srcDlg = pItem->GetAttachedScanSourceDlg();
    
    if (srcDlg == NULL)
    {
        srcDlg = new CScanSourceCodeDlg;
        ASSERT( srcDlg );

        srcDlg->Initialize(pItem);

        srcDlg->Create(CScanSourceCodeDlg::IDD);
    }

    srcDlg->ShowWindow(SW_SHOW);

}	

void CAnimTblEdDoc::OnEditSearch() 
{
	CAnimNode			SearchAnim( this, NULL );
	CAnimationEditDlg	AnimSearchDlg( &SearchAnim );
	int					nFound;
	CString				Message;

	AnimSearchDlg.m_bSearchMode	= TRUE;

	if( AnimSearchDlg.DoModal() == IDOK )
	{
		if ( AnimSearchDlg.m_NodeSearchMap.IsSearchable() )
		{
			nFound = AnimSearchDlg.m_NodeSearchMap.DoSearch( this );

			Message.Format( "%d Found", nFound );
			MessageBoxPrimaryView( Message, "# of Searches Found", MB_ICONINFORMATION | MB_OK );
		}
	}
}

void CAnimTblEdDoc::MessageBoxPrimaryView(CString Message, CString Caption, UINT nType)
{

	POSITION	pos = GetFirstViewPosition();
	CView		*pCurrentView = NULL;

	pCurrentView = GetNextView(pos);
	pCurrentView->GetActiveWindow()->MessageBox( Message, Caption, nType );
//	AfxMessageBox(Message);
}

void CAnimTblEdDoc::OnFilePrint() 
{
	// TODO: Add your command handler code here
	CPrintDialog PrintDlg( FALSE );

	if (PrintDlg.DoModal() == IDOK)
	{
	}

#if 0
	if (!PrintDlg.GetDefaults())
	   AfxMessageBox(_T("You have no default printer!"));
	else
	{
	   CString strDescription;

	   strDescription.Format(_T("Your default printer is %s on %s using %s."),
		  (LPCTSTR) PrintDlg.GetDeviceName(),
		  (LPCTSTR) PrintDlg.GetPortName(),
		  (LPCTSTR) PrintDlg.GetDriverName());

	   AfxMessageBox(strDescription);
	}
#endif
}

void CAnimTblEdDoc::OnFilePrintSetup() 
{
	// TODO: Add your command handler code here
	CPrintDialog PrintDlg( TRUE );

	if (PrintDlg.DoModal() == IDOK)
	{
	}
}

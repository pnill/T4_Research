// MeshList.cpp : implementation file
//

#include "stdafx.h"

#include "MeshList.h"
#include "ieArchive.h"
#include "FileTags.h"
#include "Mesh.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMeshList Implementation

IMPLEMENT_DYNCREATE(CMeshList, CieList)

/////////////////////////////////////////////////////////////////////////////
// CMeshList Constructors

CMeshList::CMeshList()
{
}

CMeshList::~CMeshList()
{
	DeleteAllMeshs() ;
}

/////////////////////////////////////////////////////////////////////////////
// CMeshList Implementation

void CMeshList::DeleteAllMeshs()
{
	//---	Destory each Mesh in turn
	POSITION Pos = GetHeadPosition() ;
	while (Pos != NULL)
	{
		POSITION Pos2 = Pos ;
		CMesh *pMesh = (CMesh*)GetNext (Pos) ;
		RemoveAt( Pos2 ) ;
		delete pMesh ;
	}
}

CMesh	*CMeshList::FindMatchingMesh( CMesh *pMesh )
{
	CMesh	*pResult = NULL;
	POSITION Pos = GetHeadPosition();

	while ( (Pos != NULL) && (pResult == NULL) )
	{
		CMesh *pCompMesh = (CMesh *)GetNext(Pos);
		if ( _stricmp(pCompMesh->GetPathName(),pMesh->GetPathName())==0 )
		{
			pResult = pCompMesh;
		}
	}
	return pResult;
}

bool CMeshList::SearchReplaceFilePaths( CString Search, CString Replace, s32 Method )
{
	POSITION Pos = GetHeadPosition();
	CMesh* pMesh;
	bool ReplacedSome = FALSE;

	Search.MakeUpper();
	Replace.MakeUpper();

	while( Pos )
	{
		pMesh = (CMesh*) GetNext( Pos );
		ASSERT(pMesh);

		if( pMesh->SearchReplacePathName( Search, Replace, Method ) )
			ReplacedSome = TRUE;
	}

	return ReplacedSome;
}


/////////////////////////////////////////////////////////////////////////////
// CMeshList Overridden operators

/////////////////////////////////////////////////////////////////////////////
// CMeshList serialization

void CMeshList::Serialize(CieArchive& a )
{
	if (a.IsStoring())
	{
		//---	Start Motion List
		a.WriteTag(IE_TYPE_START,IE_TAG_MESHLIST) ;

		//---	Serialize each Motion
		POSITION Pos = GetHeadPosition() ;
		while (Pos != NULL)
		{
			CMesh *pMesh = GetNext(Pos) ;
			pMesh->Serialize( a ) ;
		}

		//---	Terminate Motion List
		a.WriteTag(IE_TYPE_END,IE_TAG_MESHLIST_END) ;
	}
	else
	{
		//---	Load Motion List
		CMesh *pMesh ;
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_MESH:
				pMesh = new CMesh ;
				pMesh->Serialize( a ) ;
				AddTail( pMesh ) ;
				break ;
			case IE_TAG_MESHLIST_END:
				done = TRUE ;
				break ;
			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMeshList diagnostics

#ifdef _DEBUG
void CMeshList::AssertValid() const
{
	CObject::AssertValid();
}

void CMeshList::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

// Mesh.cpp: implementation of the CMesh class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "CeDDoc.h"
#include "Mesh.h"
#include "RenderContext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMesh Implementation

IMPLEMENT_DYNCREATE(CMesh, CObject)

/////////////////////////////////////////////////////////////////////////////
// CMesh Constructors

void CMesh::Initialize()
{
	QRAW_InitInstance( &m_QRaw ) ;
	m_IsLoaded = false ;
	m_LoadFailed = false ;
	m_pVerts = NULL ;
	m_ptVerts = NULL ;
	m_pLines1 = NULL ;
	m_pLines2 = NULL ;
	m_nVerts = 0 ;
	m_nLines = 0 ;
}

CMesh::CMesh()
{
	//---	Reset Data Members
	Initialize () ;
}

CMesh::~CMesh()
{
	//---	Destroy the Texture
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CMesh Implementation

void CMesh::Destroy()
{
	//---	Delete Allocated Data
	if( m_pVerts ) delete []m_pVerts ;
	if( m_ptVerts ) delete []m_ptVerts ;
	if( m_pLines1 ) delete []m_pLines1 ;
	if( m_pLines2 ) delete []m_pLines2 ;

	//---	Initialize Data Members
	Initialize () ;
}

void CMesh::Copy (CMesh &Dst, CMesh &Src)
{
}

bool CMesh::CreateFromFile( const char *pFileName )
{
	char	Ext[_MAX_EXT] ;

	_splitpath( pFileName, NULL, NULL, NULL, Ext ) ;
	if( stricmp( Ext, ".ASC" ) == 0 )
	{
		m_IsLoaded = CreateFromASC( pFileName ) ;
		m_LoadFailed = !m_IsLoaded ;
	}

	return m_IsLoaded ;
}

bool CMesh::CreateFromASC( const char *pFileName )
{
	bool	Success = false ;

	CString String = pFileName ;
	char *pString = String.GetBuffer( 0 ) ;
	err Error = QRAW_LoadQRaw( &m_QRaw, pString ) ;
	String.ReleaseBuffer( ) ;
	if( Error == ERR_SUCCESS )
	{
		m_PathName = pFileName ;

		Success = true ;
	}

	return Success ;
}

bool CMesh::EnsureLoaded( )
{
	if( !m_IsLoaded && !m_LoadFailed )
	{
		CreateFromFile( m_PathName ) ;
	}

	return m_IsLoaded ;
}

const char *CMesh::GetPathName( )
{
	return m_PathName ;
}

const char *CMesh::GetFileName( )
{
	static char	FName[_MAX_FNAME] ;

	_splitpath( GetPathName(), NULL, NULL, FName, NULL ) ;

	return FName ;
}

bool CMesh::SearchReplacePathName( CString& rSearch, CString& rReplace, s32 Method )
{
	CString ThisPathName = m_PathName;
	CString SearchPathName = rSearch;
	ThisPathName.MakeUpper();
	SearchPathName.MakeUpper();

	int Loc = ThisPathName.Find( SearchPathName );

	//---	if there are conditions on the search, return if they don't apply
	if ((Loc == -1 ) ||
		(( Method == FILEPATHS_LOOKFROMSTART ) && (Loc != 0)) ||
		(( Method == FILEPATHS_LOOKFROMEND ) && (Loc != (ThisPathName.GetLength() - SearchPathName.GetLength()))))
		return FALSE;

	//---	replace the search string in the file path
	CString FirstPart = m_PathName.Left( Loc );
	CString LastPart = m_PathName.Right( m_PathName.GetLength() - (Loc + rSearch.GetLength()) );

	m_PathName = FirstPart + rReplace + LastPart;

	return TRUE;
}

qraw *CMesh::GetQRaw( )
{
	return &m_QRaw ;
}

s32 CMesh::AddVertex( vector3  *pVertex )
{
	//---	Scan for Match
	for( int i = 0 ; i < m_nVerts ; i++ )
	{
		vector3 vDiff = *pVertex - m_pVerts[i];
		if( vDiff.Length() == 0.0f )
//		if( V3_Distance( pVertex, &m_pVerts[i] ) == 0.0 )
			return i ;
	}

	//---	Add New Point
	m_pVerts[m_nVerts] = *pVertex;
//	V3_Copy( &m_pVerts[m_nVerts] ,pVertex ) ;
	m_nVerts++ ;
	return m_nVerts-1 ;
}

void CMesh::AddLine( vector3  *pV1, vector3  *pV2 )
{
	//---	Add Vertices
	s32 v1 = AddVertex( pV1 ) ;
	s32 v2 = AddVertex( pV2 ) ;

	//---	Make Sure of Order
	if( v1 > v2 )
	{
		s32 t = v1 ;
		v1 = v2 ;
		v2 = t ;
	}

	//---	Scan for Match
	for( int i = 0 ; i < m_nLines ; i++ )
	{
		if( (v1 == m_pLines1[i]) && (v2 == m_pLines2[i]) )
			return ;
	}

	//---	Add New Line
	m_pLines1[m_nLines] = v1 ;
	m_pLines2[m_nLines] = v2 ;
	m_nLines++ ;
}

void CMesh::Render( CRenderContext *pRC, COLORREF Color )
{
	//---	Make Surce it is loaded
	if( EnsureLoaded( ) )
	{
		//---	Get Pointer to QRaw
		qraw *pR = GetQRaw( ) ;
		ASSERT( pR ) ;

		//---	Make Sure Render Attributes are cached
		if( (m_pVerts == NULL) && (pR->NQTris != 0) )
		{
			//---	Allocate Maximum Possible Storage
			m_pVerts = new vector3 [pR->NQTris * 3] ;
			m_ptVerts = new vector3 [pR->NQTris * 3] ;
			m_pLines1 = new s32[pR->NQTris * 3] ;
			m_pLines2 = new s32[pR->NQTris * 3] ;

			//---	Build Arrays up
			for( int i = 0 ; i < pR->NQTris ; i++ )
			{
				vector3  *p = &pR->QTri[i].XYZ[0] ;
				AddLine( &p[0], &p[1] ) ;
				AddLine( &p[1], &p[2] ) ;
				AddLine( &p[2], &p[0] ) ;
			}
		}

		//---	Transform all Points
		pRC->m_pCamera->m_Matrix.Transform( m_ptVerts, m_pVerts, m_nVerts );
//		M4_TransformVerts( &pRC->m_pCamera->m_Matrix, m_ptVerts, m_pVerts, m_nVerts ) ;

		//---	Setup Color to Draw in
		CPen Pen( PS_SOLID, 1, Color ) ;
		CPen *pOldPen = pRC->SelectObject( &Pen ) ;

		//---	Render all Lines
		for( int i = 0 ; i < m_nLines ; i++ )
		{
			pRC->RENDER_ClipProjLine3d( &m_ptVerts[m_pLines1[i]], &m_ptVerts[m_pLines2[i]] ) ;
		}

		//---	Restore Color
		pRC->SelectObject( pOldPen ) ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMesh Overridden operators

CMesh &CMesh::operator=( CMesh &ptMesh )
{
	Copy (*this, ptMesh) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CMesh serialization

void CMesh::Serialize(CieArchive& a)
{
//	CScrEdDoc *pDoc = (CScrEdDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_MESH) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_MESH_PATHNAME, m_PathName ) ;

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_MESH_END ) ;
	}
	else
	{
		//---	Load Mesh
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_MESH_PATHNAME:
				a.ReadString(m_PathName) ;
				break ;

			case IE_TAG_MESH_END:
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
// CMesh diagnostics

#ifdef _DEBUG
void CMesh::AssertValid() const
{
	CObject::AssertValid();
}

void CMesh::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG

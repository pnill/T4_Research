// Mesh.h: interface for the CMesh class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESH_H__B8064083_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_MESH_H__B8064083_E009_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ieArchive.h"

#include "QRaw.h"
#include "FileTags.h"

class CRenderContext ;


/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_MESH_END = IE_TAG_MESH,

	IE_TAG_MESH_PATHNAME,
} ;

/////////////////////////////////////////////////////////////////////////////
// CMesh

class CMesh : public CObject
{
	DECLARE_DYNCREATE(CMesh)

// Attributes
public:

private:
	CString		m_PathName ;										// PathName to Mesh
	bool		m_IsLoaded ;										// Anim has been loaded
	bool		m_LoadFailed ;										// Load Failed

	qraw		m_QRaw ;											// Quagmire QRaw Structure

	vector3 	*m_pVerts ;											// Vertex Array
	vector3 	*m_ptVerts ;										// Vertex Array
	s32			*m_pLines1 ;										// Line Array
	s32			*m_pLines2 ;										// Line Array
	s32			m_nVerts ;
	s32			m_nLines ;

// Constructors
public:
	CMesh();
	~CMesh();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;
	CMesh &operator=( CMesh & ) ;									// Right side is the argument.

	bool		CreateFromFile	( const char *pFileName ) ;			// Create Mesh from File
	bool		CreateFromASC	( const char *pFileName ) ;			// Create Mesh from ASC File

	bool		EnsureLoaded	( )	;								// Ensure Object Loaded
	const char	*GetPathName	( ) ;								// Get PathName
	const char	*GetFileName	( ) ;								// Get FileName
	bool		SearchReplacePathName	( CString& rSearch, CString& rReplace, s32 Method );

	qraw		*GetQRaw		( ) ;								// Get Pointer to QRaw

	s32			AddVertex		( vector3  *pVertex ) ;
	void		AddLine			( vector3  *pV1, vector3  *pV2 ) ;
	void		Render			( CRenderContext *pRC,
								  COLORREF Color = RGB(0,0,0) ) ;	// Draw Mesh

// Implementation
protected:
	void Initialize				( ) ;								// Initialize Data
	void Destroy				( ) ;								// Destroy
	void Copy					( CMesh &Dst, CMesh &Src ) ;		// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

#endif // !defined(AFX_MESH_H__B8064083_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
